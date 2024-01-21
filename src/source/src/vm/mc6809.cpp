/** @file mc6809.cpp

	Skelton for retropc emulator

	@par Origin
	MAME 0.142
	@author Takeda.Toshiya
	@date   2011.05.06-

	@note
	Fixed IRQ/FIRQ by Mr.Sasaji at 2011.06.17
	@note
	Modified by Sasaji at 2012.06.20

	@brief [ MC6809 ]
*/

#include "mc6809.h"
//#include "../emu.h"
#include "../depend.h"
#include "../config.h"
#include "../fileio.h"
#include "../logging.h"
#include "../utility.h"
#include "mc6809_consts.h"

/****************************************************************************/
/* memory                                                                   */
/****************************************************************************/

#ifdef USE_CPU_REAL_MACHINE_CYCLE

#define SUB_ICOUNT(Count) icount += (Count * accum_cycle)
#define DEC_ICOUNT icount+=accum_cycle

#define SUBALL_ICOUNT(Count)

#define RM(Addr)	d_mem->read_data8w(Addr, &icount); DEC_ICOUNT
#define RM_ARG(Addr)	d_mem->read_data8w(Addr, &icount)
#define WM(Addr,Value)	d_mem->write_data8w(Addr, Value, &icount); DEC_ICOUNT

#define ROP(Addr)	d_mem->read_data8w(Addr, &icount); DEC_ICOUNT
#define ROP_ARG(Addr)	d_mem->read_data8w(Addr, &icount)

#define LA(Addr)	d_mem->latch_address(Addr, &icount); DEC_ICOUNT
#define LAP(Addr)	d_mem->latch_address(Addr, &icount); DEC_ICOUNT

#define WS(Sig,Value,Mask)	d_mem->write_signal(Sig, Value, Mask)
#ifdef _MBS1
#define WS_BABS(Value)	WS(SIG_CPU_BABS, Value, 0x11)
#else
#define WS_BABS(Value)
#endif

/* macros to access memory */
#define IMMBYTE(b)	b = ROP_ARG(PCD); DEC_ICOUNT; PC++
#define IMMWORD(w)	w.d = (ROP_ARG(PCD) << 8); DEC_ICOUNT; w.d |= ROP_ARG((PCD + 1) & 0xffff); DEC_ICOUNT; PC += 2

#else /* !USE_CPU_REAL_MACHINE_CYCLE */

#define SUB_ICOUNT(Count)
#define DEC_ICOUNT

#define SUBALL_ICOUNT(Count) icount -= Count

#define RM(Addr)	d_mem->read_data8(Addr)
#define RM_ARG(Addr)	d_mem->read_data8(Addr)
#define WM(Addr,Value)	d_mem->write_data8(Addr, Value)

#define ROP(Addr)	d_mem->read_data8(Addr)
#define ROP_ARG(Addr)	d_mem->read_data8(Addr)

#define LA(Addr)
#define LAP(Addr)	d_mem->read_data8(Addr)

#define WS(Sig,Value,Mask)	d_mem->write_signal(Sig, Value, Mask)
#ifdef _MBS1
#define WS_BABS(Value)	WS(SIG_CPU_BABS, Value, 0x11)
#else
#define WS_BABS(Value)
#endif

/* macros to access memory */
#define IMMBYTE(b)	b = ROP_ARG(PCD); PC++
#define IMMWORD(w)	w.d = (ROP_ARG(PCD) << 8) | ROP_ARG((PCD + 1) & 0xffff); PC += 2

#endif /* USE_CPU_REAL_MACHINE_CYCLE */

#ifdef USE_DEBUGGER
#define SET_NPC npc = PC
#else
#define SET_NPC
#endif

#define PUSHBYTE(b)	--S; WM(SD,b)
#define PUSHWORD(w)	--S; WM(SD, w.b.l); --S; WM(SD, w.b.h)
#define PULLBYTE(b)	b = RM(SD); S++
#define PULLWORD(w)	w = RM_ARG(SD) << 8; DEC_ICOUNT; S++; w |= RM_ARG(SD); DEC_ICOUNT; S++

#define PSHUBYTE(b)	--U; WM(UD, b);
#define PSHUWORD(w)	--U; WM(UD, w.b.l); --U; WM(UD, w.b.h)
#define PULUBYTE(b)	b = RM(UD); U++
#define PULUWORD(w)	w = RM_ARG(UD) << 8; DEC_ICOUNT; U++; w |= RM_ARG(UD); DEC_ICOUNT; U++

#define CLR_HNZVC	CC &= ~(CC_H | CC_N | CC_Z | CC_V | CC_C)
#define CLR_NZV 	CC &= ~(CC_N | CC_Z | CC_V)
#define CLR_NZ		CC &= ~(CC_N | CC_Z)
#define CLR_HNZC	CC &= ~(CC_H | CC_N | CC_Z | CC_C)
#define CLR_NZVC	CC &= ~(CC_N | CC_Z | CC_V | CC_C)
#define CLR_Z		CC &= ~(CC_Z)
#define CLR_NZC 	CC &= ~(CC_N | CC_Z | CC_C)
#define CLR_ZC		CC &= ~(CC_Z | CC_C)

/* macros for CC -- CC bits affected should be reset before calling */
#define SET_Z(a)		if(!a) SEZ
#define SET_Z8(a)		SET_Z((uint8_t)a)
#define SET_Z16(a)		SET_Z((uint16_t)a)
#define SET_N8(a)		CC |= ((a & 0x80) >> 4)
#define SET_N16(a)		CC |= ((a & 0x8000) >> 12)
#define SET_H(a,b,r)		CC |= (((a ^ b ^ r) & 0x10) << 1)
#define SET_C8(a)		CC |= ((a & 0x100) >> 8)
#define SET_C16(a)		CC |= ((a & 0x10000) >> 16)
#define SET_V8(a,b,r)		CC |= (((a ^ b ^ r ^ (r >> 1)) & 0x80) >> 6)
#define SET_V16(a,b,r)		CC |= (((a ^ b ^ r ^ (r >> 1)) & 0x8000) >> 14)

#define SET_FLAGS8I(a)		{CC |= flags8i[(a) & 0xff];}
#define SET_FLAGS8D(a)		{CC |= flags8d[(a) & 0xff];}

/* combos */
#define SET_NZ8(a)		{SET_N8(a); SET_Z(a);}
#define SET_NZ16(a)		{SET_N16(a); SET_Z(a);}
#define SET_FLAGS8(a,b,r)	{SET_N8(r); SET_Z8(r); SET_V8(a, b, r); SET_C8(r);}
#define SET_FLAGS16(a,b,r)	{SET_N16(r); SET_Z16(r); SET_V16(a, b, r); SET_C16(r);}

#define NXORV		((CC & CC_N) ^ ((CC & CC_V) << 2))

/* for treating an unsigned byte as a signed word */
#define SIGNED(b)	((uint16_t)((b & 0x80) ? (b | 0xff00) : b))

/* macros for addressing modes (postbytes have their own code) */
#define DIRECT		EAD = DPD; IMMBYTE(ea.b.l); DEC_ICOUNT
#define IMM8		EAD = PCD; PC++
#define IMM16		EAD = PCD; PC += 2
#define EXTENDED	IMMWORD(EAP); DEC_ICOUNT

/* macros to set status flags */
#define SEC		CC |= CC_C
#define CLC		CC &= ~CC_C
#define SEZ		CC |= CC_Z
#define CLZ		CC &= ~CC_Z
#define SEN		CC |= CC_N
#define CLN		CC &= ~CC_N
#define SEV		CC |= CC_V
#define CLV		CC &= ~CC_V
#define SEH		CC |= CC_H
#define CLH		CC &= ~CC_H

/* macros for convenience */
#define DIRBYTE(b)	{DIRECT;   b   = RM(EAD);  }
#define DIRWORD(w)	{DIRECT;   w.d = RM16(EAD);}
#define EXTBYTE(b)	{EXTENDED; b   = RM(EAD);  }
#define EXTWORD(w)	{EXTENDED; w.d = RM16(EAD);}

/* macros for branch instructions */
#define BRANCH(f) { \
	uint8_t t; \
	IMMBYTE(t); \
	DEC_ICOUNT; \
	if(f) { \
		SET_NPC; \
		PC += SIGNED(t); \
	} \
	DASM_SET_REL(t); \
}

#define LBRANCH(f) { \
	pair32_t t; \
	IMMWORD(t); \
	DEC_ICOUNT; \
	if(f) { \
		SUBALL_ICOUNT(1); \
		DEC_ICOUNT; \
		SET_NPC; \
		PC += t.w.l; \
	} \
	DASM_SET_REL(t.w.l); \
}

/* macros for setting/getting registers in TFR/EXG instructions */

inline uint32_t MC6809::RM16(uint32_t Addr)
{
	uint32_t result = RM_ARG(Addr) << 8;
	DEC_ICOUNT;
	result |= RM_ARG((Addr + 1) & 0xffff);
	DEC_ICOUNT;
	return result;
}

inline void MC6809::WM16(uint32_t Addr, pair32_t *p)
{
	WM(Addr, p->b.h);
	WM((Addr + 1) & 0xffff, p->b.l);
}

/* increment */
static const uint8_t flags8i[256] = {
	CC_Z,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	CC_N|CC_V,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
	CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
	CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
	CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
	CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
	CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
	CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
	CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N
};

/* decrement */
static const uint8_t flags8d[256] = {
	CC_Z,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,CC_V,
	CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
	CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
	CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
	CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
	CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
	CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
	CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
	CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N
};

#ifndef USE_CPU_REAL_MACHINE_CYCLE
/* FIXME: Cycles differ slighly from hd6309 emulation */
static const uint8_t index_cycle_em[256] = {
	/* Index Loopup cycle counts */
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	2, 3, 2, 3, 0, 1, 1, 0, 1, 4, 0, 4, 1, 5, 0, 5,
	5, 6, 5, 6, 3, 4, 4, 0, 4, 7, 0, 7, 4, 8, 0, 5,
	2, 3, 2, 3, 0, 1, 1, 0, 1, 4, 0, 4, 1, 5, 0, 5,
	5, 6, 5, 6, 3, 4, 4, 0, 4, 7, 0, 7, 4, 8, 0, 5,
	2, 3, 2, 3, 0, 1, 1, 0, 1, 4, 0, 4, 1, 5, 0, 5,
	5, 6, 5, 6, 3, 4, 4, 0, 4, 7, 0, 7, 4, 8, 0, 5,
	2, 3, 2, 3, 0, 1, 1, 0, 1, 4, 0, 4, 1, 5, 0, 5,
	4, 6, 5, 6, 3, 4, 4, 0, 4, 7, 0, 7, 4, 8, 0, 5
};

/* timings for 1-byte opcodes */
static const uint8_t cycles1[] =
{
	6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 3, 6,
	0, 0, 2, 4, 2, 2, 5, 9, 3, 2, 3, 2, 3, 2, 8, 6,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	4, 4, 4, 4, 5, 5, 5, 5, 2, 5, 3, 6,20,11, 2,19,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 3, 6,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 4, 7,
	2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 2, 4, 7, 3, 2,
	4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 4, 6, 7, 5, 5,
	4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 4, 6, 7, 5, 5,
	5, 5, 5, 7, 5, 5, 5, 5, 5, 5, 5, 5, 7, 8, 6, 6,
	2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 3, 3,
	4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5,
	4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5,
	5, 5, 5, 7, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6
};
#endif

MC6809::MC6809(VM* parent_vm, EMU* parent_emu, const char* identifier)
	: DEVICE(parent_vm, parent_emu, identifier)
{
	set_class_name("MC6809");

	cpu_clock = 1;
}

void MC6809::initialize()
{
#ifdef USE_DEBUGGER
	d_mem_stored = d_mem;
	d_debugger->set_name(_T("MC6809"));
	d_debugger->rerate_to_cpu(true);
	d_debugger->set_context_mem(d_mem);
	memset(int_flags_debug, 0, sizeof(int_flags_debug));
#endif
	int_state = 0;
#ifdef USE_CPU_REAL_MACHINE_CYCLE
	accum_cycle = 1;
#endif

	memset(illegal_pc, 0, sizeof(illegal_pc));
	illegal_pc_idx = 0;
}

void MC6809::reset()
{
	icount = 0;
	/*
	 Clear HALT bit when trap (internal HALT) is occurring.
	 */
	if (int_state & MC6809_INSN_HALT) {
		int_state &= ~(MC6809_HALT_BIT | MC6809_INSN_HALT);
	}
	/*
	 Clear SYNC/CWAI/LDS(Arm NMI) flags
	 */
	int_state &= ~(MC6809_CWAI | MC6809_SYNC | MC6809_CWAI_IN | MC6809_CWAI_OUT | MC6809_SYNC_IN | MC6809_SYNC_OUT | MC6809_LDS);
	/*
	 Initial state is set RESET signal,
	 so need release RESET signal form other devices.
	 */
	int_state |= MC6809_RESET_BIT;
	int_released = 0;
#ifdef USE_DEBUGGER
	int_state_debug = int_state;
#endif
#ifdef USE_CPU_REAL_MACHINE_CYCLE
	accum_cycle = 1;
#endif

	DPD = 0;	/* Reset direct page register */

	CC |= CC_II;	/* IRQ disabled */
	CC |= CC_IF;	/* FIRQ disabled */

	memset(illegal_pc, 0, sizeof(illegal_pc));
	illegal_pc_idx = 0;

	/* Clear other registers */
	UD = FLG_CLEAR_CPUREG ? 0 : 0xcdcdcdcd;
	SD = FLG_CLEAR_CPUREG ? 0 : 0xcdcdcdcd;
	XD = FLG_CLEAR_CPUREG ? 0 : 0xcdcdcdcd;
	YD = FLG_CLEAR_CPUREG ? 0 : 0xcdcdcdcd;
	DD = FLG_CLEAR_CPUREG ? 0 : 0xcdcdcdcd;

	WS_BABS(0x01);
	PCD = RM16(0xfffe);
	WS_BABS(0x00);
}

void MC6809::write_signal(int id, uint32_t data, uint32_t mask)
{
#ifdef USE_DEBUGGER
//	uint32_t int_state_prev = int_state_debug;
	int int_flags_idx = -1;
	uint32_t int_flags_next = (data & mask);
#endif

	if(id == SIG_CPU_IRQ) {
#ifdef USE_DEBUGGER
		int_flags_idx = MC6809_IDX_IRQ;
#endif
		if(data & mask) {
			int_state |= MC6809_IRQ_BIT;
#ifdef USE_DEBUGGER
			int_state_debug |= MC6809_IRQ_BIT;
#endif
		}
		else {
			int_state &= ~MC6809_IRQ_BIT;
#ifdef USE_DEBUGGER
			int_state_debug &= ~MC6809_IRQ_BIT;
#endif
		}
	}
	else if(id == SIG_CPU_FIRQ) {
#ifdef USE_DEBUGGER
		int_flags_idx = MC6809_IDX_FIRQ;
#endif
		if(data & mask) {
			int_state |= MC6809_FIRQ_BIT;
#ifdef USE_DEBUGGER
			int_state_debug |= MC6809_FIRQ_BIT;
#endif
		}
		else {
			int_state &= ~MC6809_FIRQ_BIT;
#ifdef USE_DEBUGGER
			int_state_debug &= ~MC6809_FIRQ_BIT;
#endif
		}
	}
	else if(id == SIG_CPU_NMI) {
#ifdef USE_DEBUGGER
		int_flags_idx = MC6809_IDX_NMI;
#endif
		if(data & mask) {
			int_state |= MC6809_NMI_BIT;
#ifdef USE_DEBUGGER
			int_state_debug |= MC6809_NMI_BIT;
#endif
		}
		else {
			// reserve NMI flag until HALT is off 
			if (!(int_state & MC6809_HALT_BIT)) {
				int_state &= ~MC6809_NMI_BIT;
#ifdef USE_DEBUGGER
				int_state_debug &= ~MC6809_NMI_BIT;
#endif
			}
		}
	}
	else if(id == SIG_CPU_RESET) {
#ifdef USE_DEBUGGER
		int_flags_idx = MC6809_IDX_RESET;
#endif
		if(data & mask) {
			int_state |= MC6809_RESET_BIT;
#ifdef USE_DEBUGGER
			int_state_debug |= MC6809_RESET_BIT;
#endif
			now_reset = true;
		}
		else {
			if (now_reset) {
				reset();
			}
			int_state &= ~MC6809_RESET_BIT;
#ifdef USE_DEBUGGER
			int_state_debug &= ~MC6809_RESET_BIT;
#endif
			now_reset = false;
		}
	}
	else if(id == SIG_CPU_HALT) {
#ifdef USE_DEBUGGER
		int_flags_idx = MC6809_IDX_HALT;
#endif
		if(data & mask) {
			int_state |= MC6809_HALT_BIT;
			int_state &= ~MC6809_INSN_HALT;
			int_released &= ~MC6809_HALT_BIT;
#ifdef USE_DEBUGGER
			int_state_debug |= MC6809_HALT_BIT;
			int_state_debug &= ~MC6809_INSN_HALT;
#endif
		}
		else {
			if (int_state & MC6809_HALT_BIT) int_released |= MC6809_HALT_BIT;
			int_state &= ~MC6809_HALT_BIT;
#ifdef USE_DEBUGGER
			int_state_debug &= ~MC6809_HALT_BIT;
#endif
		}
	}

#ifdef USE_DEBUGGER
//	if(d_debugger->now_debugging() && int_state_prev != int_state_debug) {
//		d_debugger->check_intr_break_points((uint32_t)id, (data & mask) ? 0 : 1);
//	}
	if (int_flags_idx >= 0) {
		if(d_debugger->now_debugging() && int_flags_next != int_flags_debug[int_flags_idx]) {
			d_debugger->check_intr_break_points((uint32_t)id, int_flags_next != 0 ? 1 : 0);
		}
		int_flags_debug[int_flags_idx] = int_flags_next;
		dasm.set_signal(int_state_debug, int_flags_idx, int_flags_next);
	}
#endif
}

/// @param[in] clock : -1: process one code / >0: continue processing codes until spend given clock
/// @return : spent clock for processing code
int MC6809::run(int clock)
{
	iaccum = 0;

	// run cpu
	if(clock == -1) {
		// run only one opcode
		icount = 0;
#if defined(USE_DEBUGGER)
#if defined(USE_SUSPEND_IN_CPU)
		bool now_debugging = d_debugger->now_debugging();
		if(now_debugging) {
			d_debugger->check_break_points(PC);
			if(d_debugger->now_suspended()) {
				emu->mute_sound(true);
				while(d_debugger->now_suspend()) {
					emu->sleep(10);
				}
			}
			if(d_debugger->now_debugging()) {
				d_mem = d_debugger;
			} else {
				now_debugging = false;
			}

			run_one_opecode();

			if(now_debugging) {
				if(!d_debugger->now_going()) {
					d_debugger->now_suspended(true);
				}
				d_mem = d_mem_stored;
			}
		} else
#else
		if (d_debugger->now_debugging()) {
			d_mem = d_debugger;
			if (d_debugger->now_suspended()) {
				return 0;
			}
		} else {
			d_mem = d_mem_stored;
		}
#endif
#endif
		{
			run_one_opecode();
		}
		return -icount;
	}
	else {
		// run cpu while given clocks
		icount += clock;
		int first_icount = icount;

		while(icount > 0) {
#if defined(USE_DEBUGGER)
#if defined(USE_SUSPEND_IN_CPU)
			bool now_debugging = d_debugger->now_debugging();
			if(now_debugging) {
				d_debugger->check_break_points(PC);
				if(d_debugger->now_suspended()) {
					emu->mute_sound(true);
					while(d_debugger->now_suspend()) {
						emu->sleep(10);
					}
				}
				if(d_debugger->now_debugging()) {
					d_mem = d_debugger;
				} else {
					now_debugging = false;
				}

				run_one_opecode();

				if(now_debugging) {
					if(!d_debugger->now_going()) {
						d_debugger->now_suspended(true);
					}
					d_mem = d_mem_stored;
				}
			} else
#else
			if (d_debugger->now_debugging()) {
				d_mem = d_debugger;
				if (d_debugger->now_suspended()) {
					return 0;
				}
			} else {
				d_mem = d_mem_stored;
			}
#endif
#endif
			{
				run_one_opecode();
			}
		}
		return first_icount - icount;
	}
}

#ifdef USE_CPU_REAL_MACHINE_CYCLE
/// @param[in] clock : -1: process one code / >0: continue processing codes until spend given clock
/// @param[in] accum : current accumulated clock
/// @param[in] cycle : spent clock is multiplied by cycle
/// @return : spent clock for processing code
int MC6809::run(int clock, int accum, int cycle)
{
	// run only one opcode
	iaccum = accum;
	icount = accum;
	accum_cycle = cycle;
#if defined(USE_DEBUGGER)
#if defined(USE_SUSPEND_IN_CPU)
	bool now_debugging = d_debugger->now_debugging();
	if(now_debugging) {
		d_debugger->check_break_points(PC);
		if(d_debugger->now_suspended()) {
			emu->mute_sound(true);
			while(d_debugger->now_suspend()) {
				emu->sleep(10);
			}
		}
		if(d_debugger->now_debugging()) {
			d_mem = d_debugger;
		} else {
			now_debugging = false;
		}

		run_one_opecode();

		if(now_debugging) {
			if(!d_debugger->now_going()) {
				d_debugger->now_suspended(true);
			}
			d_mem = d_mem_stored;
		}
	} else
#else
	if (d_debugger->now_debugging()) {
		d_mem = d_debugger;
		if (d_debugger->now_suspended()) {
			return 0;
		}
	} else {
		d_mem = d_mem_stored;
	}
#endif
#endif
	{
		run_one_opecode();
	}
	return icount-iaccum;
}
#endif

#define DUMMY_FETCH_OP { \
	uint8_t ireg = ROP(PCD); \
	if (ireg == 0x10 || ireg == 0x11) { \
		ireg = ROP(PCD+1); \
	} \
}

void MC6809::run_one_opecode()
{
	if(int_released & MC6809_HALT_BIT) {
		/* release HALT */
		DEC_ICOUNT;
		int_released &= ~MC6809_HALT_BIT;
	}

	if(int_state & (MC6809_HALT_BIT | MC6809_RESET_BIT)) {
		/* reserve interrupt */
	}
	else if(int_state & MC6809_NMI_BIT) {
// clear NMI logic
		int_state &= ~MC6809_NMI_BIT;
		int_state &= ~MC6809_SYNC; /* clear SYNC flag */
		if(int_state & MC6809_CWAI) {
			int_state &= ~MC6809_CWAI;
			SUBALL_ICOUNT(7); /* subtract +7 cycles next time */
		}
		else {
			DUMMY_FETCH_OP;	/* fetch next opcode */
			DEC_ICOUNT;
			CC |= CC_E; /* save entire state */
			PUSHWORD(pPC);
			PUSHWORD(pU);
			PUSHWORD(pY);
			PUSHWORD(pX);
			PUSHBYTE(DP);
			PUSHBYTE(B);
			PUSHBYTE(A);
			PUSHBYTE(CC);
			DEC_ICOUNT;
			SUBALL_ICOUNT(19); /* subtract +19 cycles next time */
		}
		CC |= CC_IF | CC_II; /* inhibit FIRQ and IRQ */
		WS_BABS(0x01);
		SET_NPC;
		PCD = RM16(0xfffc);
		WS_BABS(0x00);
		DEC_ICOUNT;
	}
	else if(int_state & (MC6809_FIRQ_BIT | MC6809_IRQ_BIT)) {
		int_state &= ~MC6809_SYNC; /* clear SYNC flag */
		if((int_state & MC6809_FIRQ_BIT) && !(CC & CC_IF)) {
			/* fast IRQ */
// keep on a flag because interrupt signal is controlled by outer devices
//			int_state &= ~MC6809_FIRQ_BIT;
			if(int_state & MC6809_CWAI) {
				int_state &= ~MC6809_CWAI; /* clear CWAI */
				SUBALL_ICOUNT(7); /* subtract +7 cycles */
			}
			else {
				DUMMY_FETCH_OP;	/* fetch next opcode */
				DEC_ICOUNT;
				CC &= ~CC_E; /* save 'short' state */
				PUSHWORD(pPC);
				PUSHBYTE(CC);
				DEC_ICOUNT;
				SUBALL_ICOUNT(10); /* subtract +10 cycles */
			}
			CC |= CC_IF | CC_II; /* inhibit FIRQ and IRQ */
			WS_BABS(0x01);
			SET_NPC;
			PCD = RM16(0xfff6);
			WS_BABS(0x00);
			DEC_ICOUNT;
		}
		else if((int_state & MC6809_IRQ_BIT) && !(CC & CC_II)) {
			/* standard IRQ */
// keep on a flag because interrupt signal is controlled by outer devices
//			int_state &= ~MC6809_IRQ_BIT;
			if(int_state & MC6809_CWAI) {
				int_state &= ~MC6809_CWAI; /* clear CWAI flag */
				SUBALL_ICOUNT(7); /* subtract +7 cycles */
			}
			else {
				DUMMY_FETCH_OP;	/* fetch next opcode */
				DEC_ICOUNT;
				CC |= CC_E; /* save entire state */
				PUSHWORD(pPC);
				PUSHWORD(pU);
				PUSHWORD(pY);
				PUSHWORD(pX);
				PUSHBYTE(DP);
				PUSHBYTE(B);
				PUSHBYTE(A);
				PUSHBYTE(CC);
				DEC_ICOUNT;
				SUBALL_ICOUNT(19); /* subtract +19 cycles */
			}
			CC |= CC_II; /* inhibit IRQ */
			WS_BABS(0x01);
			SET_NPC;
			PCD = RM16(0xfff8);
			WS_BABS(0x00);
			DEC_ICOUNT;
		}
	}
	if (int_state & (MC6809_HALT_BIT | MC6809_RESET_BIT | MC6809_CWAI | MC6809_SYNC)) {
		if (int_state & MC6809_HALT_BIT) WS_BABS(0x11);
		else if (int_state & MC6809_SYNC) WS_BABS(0x10);
		else WS_BABS(0x00);
		SUBALL_ICOUNT(1);
		DEC_ICOUNT;
	}
	else {
		SET_NPC;
		pPPC = pPC;
		uint8_t ireg = ROP(PCD);
#ifdef USE_DEBUGGER
		dasm.ini_pc(PC, ireg);
#endif
		PC++;
		op(ireg);
#ifndef USE_CPU_REAL_MACHINE_CYCLE
		icount -= cycles1[ireg];
#ifdef USE_DEBUGGER
		dasm.set_regs(iaccum,iaccum-icount,CC,DP,A,B,X,Y,S,U);
//		dasm.print(iaccum, iaccum-icount);
#endif
#else
#ifdef USE_DEBUGGER
		dasm.set_regs(iaccum,icount-iaccum,CC,DP,A,B,X,Y,S,U);
//		dasm.print(iaccum, icount-iaccum);
#endif
#endif
	}
}

void MC6809::op(uint8_t ireg)
{
	switch(ireg) {
	case 0x00: neg_di(); break;
	case 0x01: err_neg_di(); break;
	case 0x02: err_ngc_di(); break;
	case 0x03: com_di(); break;
	case 0x04: lsr_di(); break;
	case 0x05: err_lsr_di(); break;
	case 0x06: ror_di(); break;
	case 0x07: asr_di(); break;
	case 0x08: asl_di(); break;
	case 0x09: rol_di(); break;
	case 0x0a: dec_di(); break;
	case 0x0b: err_dcc_di(); break;
	case 0x0c: inc_di(); break;
	case 0x0d: tst_di(); break;
	case 0x0e: jmp_di(); break;
	case 0x0f: clr_di(); break;
	case 0x10: pref10(); break;
	case 0x11: pref11(); break;
	case 0x12: nop(); break;
	case 0x13: sync(); break;
	case 0x14: err_halt(ireg); break;
	case 0x15: err_halt(ireg); break;
	case 0x16: lbra(); break;
	case 0x17: lbsr(); break;
	case 0x18: err_slcc(); break;
	case 0x19: daa(); break;
	case 0x1a: orcc(); break;
	case 0x1b: err_nop(); break;
	case 0x1c: andcc(); break;
	case 0x1d: sex(); break;
	case 0x1e: exg(); break;
	case 0x1f: tfr(); break;
	case 0x20: bra(); break;
	case 0x21: brn(); break;
	case 0x22: bhi(); break;
	case 0x23: bls(); break;
	case 0x24: bcc(); break;
	case 0x25: bcs(); break;
	case 0x26: bne(); break;
	case 0x27: beq(); break;
	case 0x28: bvc(); break;
	case 0x29: bvs(); break;
	case 0x2a: bpl(); break;
	case 0x2b: bmi(); break;
	case 0x2c: bge(); break;
	case 0x2d: blt(); break;
	case 0x2e: bgt(); break;
	case 0x2f: ble(); break;
	case 0x30: leax(); break;
	case 0x31: leay(); break;
	case 0x32: leas(); break;
	case 0x33: leau(); break;
	case 0x34: pshs(); break;
	case 0x35: puls(); break;
	case 0x36: pshu(); break;
	case 0x37: pulu(); break;
	case 0x38: err_cwai(); break;
	case 0x39: rts(); break;
	case 0x3a: abx(); break;
	case 0x3b: rti(); break;
	case 0x3c: cwai(); break;
	case 0x3d: mul(); break;
	case 0x3e: err_reset(); break;
	case 0x3f: swi(); break;
	case 0x40: nega(); break;
	case 0x41: err_nega(); break;
	case 0x42: err_ngca(); break;
	case 0x43: coma(); break;
	case 0x44: lsra(); break;
	case 0x45: err_lsra(); break;
	case 0x46: rora(); break;
	case 0x47: asra(); break;
	case 0x48: asla(); break;
	case 0x49: rola(); break;
	case 0x4a: deca(); break;
	case 0x4b: err_dcca(); break;
	case 0x4c: inca(); break;
	case 0x4d: tsta(); break;
	case 0x4e: err_clca(); break;
	case 0x4f: clra(); break;
	case 0x50: negb(); break;
	case 0x51: err_negb(); break;
	case 0x52: err_ngcb(); break;
	case 0x53: comb(); break;
	case 0x54: lsrb(); break;
	case 0x55: err_lsrb(); break;
	case 0x56: rorb(); break;
	case 0x57: asrb(); break;
	case 0x58: aslb(); break;
	case 0x59: rolb(); break;
	case 0x5a: decb(); break;
	case 0x5b: err_dccb(); break;
	case 0x5c: incb(); break;
	case 0x5d: tstb(); break;
	case 0x5e: err_clcb(); break;
	case 0x5f: clrb(); break;
	case 0x60: neg_ix(); break;
	case 0x61: err_neg_ix(); break;
	case 0x62: err_ngc_ix(); break;
	case 0x63: com_ix(); break;
	case 0x64: lsr_ix(); break;
	case 0x65: err_lsr_ix(); break;
	case 0x66: ror_ix(); break;
	case 0x67: asr_ix(); break;
	case 0x68: asl_ix(); break;
	case 0x69: rol_ix(); break;
	case 0x6a: dec_ix(); break;
	case 0x6b: err_dcc_ix(); break;
	case 0x6c: inc_ix(); break;
	case 0x6d: tst_ix(); break;
	case 0x6e: jmp_ix(); break;
	case 0x6f: clr_ix(); break;
	case 0x70: neg_ex(); break;
	case 0x71: err_neg_ex(); break;
	case 0x72: err_ngc_ex(); break;
	case 0x73: com_ex(); break;
	case 0x74: lsr_ex(); break;
	case 0x75: err_lsr_ex(); break;
	case 0x76: ror_ex(); break;
	case 0x77: asr_ex(); break;
	case 0x78: asl_ex(); break;
	case 0x79: rol_ex(); break;
	case 0x7a: dec_ex(); break;
	case 0x7b: err_dcc_ex(); break;
	case 0x7c: inc_ex(); break;
	case 0x7d: tst_ex(); break;
	case 0x7e: jmp_ex(); break;
	case 0x7f: clr_ex(); break;
	case 0x80: suba_im(); break;
	case 0x81: cmpa_im(); break;
	case 0x82: sbca_im(); break;
	case 0x83: subd_im(); break;
	case 0x84: anda_im(); break;
	case 0x85: bita_im(); break;
	case 0x86: lda_im(); break;
	case 0x87: err_sta_im(); break;
	case 0x88: eora_im(); break;
	case 0x89: adca_im(); break;
	case 0x8a: ora_im(); break;
	case 0x8b: adda_im(); break;
	case 0x8c: cmpx_im(); break;
	case 0x8d: bsr(); break;
	case 0x8e: ldx_im(); break;
	case 0x8f: err_stx_im(); break;
	case 0x90: suba_di(); break;
	case 0x91: cmpa_di(); break;
	case 0x92: sbca_di(); break;
	case 0x93: subd_di(); break;
	case 0x94: anda_di(); break;
	case 0x95: bita_di(); break;
	case 0x96: lda_di(); break;
	case 0x97: sta_di(); break;
	case 0x98: eora_di(); break;
	case 0x99: adca_di(); break;
	case 0x9a: ora_di(); break;
	case 0x9b: adda_di(); break;
	case 0x9c: cmpx_di(); break;
	case 0x9d: jsr_di(); break;
	case 0x9e: ldx_di(); break;
	case 0x9f: stx_di(); break;
	case 0xa0: suba_ix(); break;
	case 0xa1: cmpa_ix(); break;
	case 0xa2: sbca_ix(); break;
	case 0xa3: subd_ix(); break;
	case 0xa4: anda_ix(); break;
	case 0xa5: bita_ix(); break;
	case 0xa6: lda_ix(); break;
	case 0xa7: sta_ix(); break;
	case 0xa8: eora_ix(); break;
	case 0xa9: adca_ix(); break;
	case 0xaa: ora_ix(); break;
	case 0xab: adda_ix(); break;
	case 0xac: cmpx_ix(); break;
	case 0xad: jsr_ix(); break;
	case 0xae: ldx_ix(); break;
	case 0xaf: stx_ix(); break;
	case 0xb0: suba_ex(); break;
	case 0xb1: cmpa_ex(); break;
	case 0xb2: sbca_ex(); break;
	case 0xb3: subd_ex(); break;
	case 0xb4: anda_ex(); break;
	case 0xb5: bita_ex(); break;
	case 0xb6: lda_ex(); break;
	case 0xb7: sta_ex(); break;
	case 0xb8: eora_ex(); break;
	case 0xb9: adca_ex(); break;
	case 0xba: ora_ex(); break;
	case 0xbb: adda_ex(); break;
	case 0xbc: cmpx_ex(); break;
	case 0xbd: jsr_ex(); break;
	case 0xbe: ldx_ex(); break;
	case 0xbf: stx_ex(); break;
	case 0xc0: subb_im(); break;
	case 0xc1: cmpb_im(); break;
	case 0xc2: sbcb_im(); break;
	case 0xc3: addd_im(); break;
	case 0xc4: andb_im(); break;
	case 0xc5: bitb_im(); break;
	case 0xc6: ldb_im(); break;
	case 0xc7: err_stb_im(); break;
	case 0xc8: eorb_im(); break;
	case 0xc9: adcb_im(); break;
	case 0xca: orb_im(); break;
	case 0xcb: addb_im(); break;
	case 0xcc: ldd_im(); break;
	case 0xcd: err_halt(ireg); break;
	case 0xce: ldu_im(); break;
	case 0xcf: err_stu_im(); break;
	case 0xd0: subb_di(); break;
	case 0xd1: cmpb_di(); break;
	case 0xd2: sbcb_di(); break;
	case 0xd3: addd_di(); break;
	case 0xd4: andb_di(); break;
	case 0xd5: bitb_di(); break;
	case 0xd6: ldb_di(); break;
	case 0xd7: stb_di(); break;
	case 0xd8: eorb_di(); break;
	case 0xd9: adcb_di(); break;
	case 0xda: orb_di(); break;
	case 0xdb: addb_di(); break;
	case 0xdc: ldd_di(); break;
	case 0xdd: std_di(); break;
	case 0xde: ldu_di(); break;
	case 0xdf: stu_di(); break;
	case 0xe0: subb_ix(); break;
	case 0xe1: cmpb_ix(); break;
	case 0xe2: sbcb_ix(); break;
	case 0xe3: addd_ix(); break;
	case 0xe4: andb_ix(); break;
	case 0xe5: bitb_ix(); break;
	case 0xe6: ldb_ix(); break;
	case 0xe7: stb_ix(); break;
	case 0xe8: eorb_ix(); break;
	case 0xe9: adcb_ix(); break;
	case 0xea: orb_ix(); break;
	case 0xeb: addb_ix(); break;
	case 0xec: ldd_ix(); break;
	case 0xed: std_ix(); break;
	case 0xee: ldu_ix(); break;
	case 0xef: stu_ix(); break;
	case 0xf0: subb_ex(); break;
	case 0xf1: cmpb_ex(); break;
	case 0xf2: sbcb_ex(); break;
	case 0xf3: addd_ex(); break;
	case 0xf4: andb_ex(); break;
	case 0xf5: bitb_ex(); break;
	case 0xf6: ldb_ex(); break;
	case 0xf7: stb_ex(); break;
	case 0xf8: eorb_ex(); break;
	case 0xf9: adcb_ex(); break;
	case 0xfa: orb_ex(); break;
	case 0xfb: addb_ex(); break;
	case 0xfc: ldd_ex(); break;
	case 0xfd: std_ex(); break;
	case 0xfe: ldu_ex(); break;
	case 0xff: stu_ex(); break;
	}
};

inline void MC6809::fetch_effective_address()
{
	uint8_t postbyte = ROP_ARG(PCD);
	DEC_ICOUNT;
	PC++;

	DASM_SET_IDX1(postbyte);

	switch(postbyte) {
	// 0x00-0x7f 5bit offset ~:+1+1
	case 0x00: EA = X; LA(PCD); DEC_ICOUNT; break;
	case 0x01: EA = X + 1; LA(PCD); DEC_ICOUNT; break;
	case 0x02: EA = X + 2; LA(PCD); DEC_ICOUNT; break;
	case 0x03: EA = X + 3; LA(PCD); DEC_ICOUNT; break;
	case 0x04: EA = X + 4; LA(PCD); DEC_ICOUNT; break;
	case 0x05: EA = X + 5; LA(PCD); DEC_ICOUNT; break;
	case 0x06: EA = X + 6; LA(PCD); DEC_ICOUNT; break;
	case 0x07: EA = X + 7; LA(PCD); DEC_ICOUNT; break;
	case 0x08: EA = X + 8; LA(PCD); DEC_ICOUNT; break;
	case 0x09: EA = X + 9; LA(PCD); DEC_ICOUNT; break;
	case 0x0a: EA = X + 10; LA(PCD); DEC_ICOUNT; break;
	case 0x0b: EA = X + 11; LA(PCD); DEC_ICOUNT; break;
	case 0x0c: EA = X + 12; LA(PCD); DEC_ICOUNT; break;
	case 0x0d: EA = X + 13; LA(PCD); DEC_ICOUNT; break;
	case 0x0e: EA = X + 14; LA(PCD); DEC_ICOUNT; break;
	case 0x0f: EA = X + 15; LA(PCD); DEC_ICOUNT; break;

	case 0x10: EA = X - 16; LA(PCD); DEC_ICOUNT; break;
	case 0x11: EA = X - 15; LA(PCD); DEC_ICOUNT; break;
	case 0x12: EA = X - 14; LA(PCD); DEC_ICOUNT; break;
	case 0x13: EA = X - 13; LA(PCD); DEC_ICOUNT; break;
	case 0x14: EA = X - 12; LA(PCD); DEC_ICOUNT; break;
	case 0x15: EA = X - 11; LA(PCD); DEC_ICOUNT; break;
	case 0x16: EA = X - 10; LA(PCD); DEC_ICOUNT; break;
	case 0x17: EA = X - 9; LA(PCD); DEC_ICOUNT; break;
	case 0x18: EA = X - 8; LA(PCD); DEC_ICOUNT; break;
	case 0x19: EA = X - 7; LA(PCD); DEC_ICOUNT; break;
	case 0x1a: EA = X - 6; LA(PCD); DEC_ICOUNT; break;
	case 0x1b: EA = X - 5; LA(PCD); DEC_ICOUNT; break;
	case 0x1c: EA = X - 4; LA(PCD); DEC_ICOUNT; break;
	case 0x1d: EA = X - 3; LA(PCD); DEC_ICOUNT; break;
	case 0x1e: EA = X - 2; LA(PCD); DEC_ICOUNT; break;
	case 0x1f: EA = X - 1; LA(PCD); DEC_ICOUNT; break;

	case 0x20: EA = Y; LA(PCD); DEC_ICOUNT; break;
	case 0x21: EA = Y + 1; LA(PCD); DEC_ICOUNT; break;
	case 0x22: EA = Y + 2; LA(PCD); DEC_ICOUNT; break;
	case 0x23: EA = Y + 3; LA(PCD); DEC_ICOUNT; break;
	case 0x24: EA = Y + 4; LA(PCD); DEC_ICOUNT; break;
	case 0x25: EA = Y + 5; LA(PCD); DEC_ICOUNT; break;
	case 0x26: EA = Y + 6; LA(PCD); DEC_ICOUNT; break;
	case 0x27: EA = Y + 7; LA(PCD); DEC_ICOUNT; break;
	case 0x28: EA = Y + 8; LA(PCD); DEC_ICOUNT; break;
	case 0x29: EA = Y + 9; LA(PCD); DEC_ICOUNT; break;
	case 0x2a: EA = Y + 10; LA(PCD); DEC_ICOUNT; break;
	case 0x2b: EA = Y + 11; LA(PCD); DEC_ICOUNT; break;
	case 0x2c: EA = Y + 12; LA(PCD); DEC_ICOUNT; break;
	case 0x2d: EA = Y + 13; LA(PCD); DEC_ICOUNT; break;
	case 0x2e: EA = Y + 14; LA(PCD); DEC_ICOUNT; break;
	case 0x2f: EA = Y + 15; LA(PCD); DEC_ICOUNT; break;

	case 0x30: EA = Y - 16; LA(PCD); DEC_ICOUNT; break;
	case 0x31: EA = Y - 15; LA(PCD); DEC_ICOUNT; break;
	case 0x32: EA = Y - 14; LA(PCD); DEC_ICOUNT; break;
	case 0x33: EA = Y - 13; LA(PCD); DEC_ICOUNT; break;
	case 0x34: EA = Y - 12; LA(PCD); DEC_ICOUNT; break;
	case 0x35: EA = Y - 11; LA(PCD); DEC_ICOUNT; break;
	case 0x36: EA = Y - 10; LA(PCD); DEC_ICOUNT; break;
	case 0x37: EA = Y - 9; LA(PCD); DEC_ICOUNT; break;
	case 0x38: EA = Y - 8; LA(PCD); DEC_ICOUNT; break;
	case 0x39: EA = Y - 7; LA(PCD); DEC_ICOUNT; break;
	case 0x3a: EA = Y - 6; LA(PCD); DEC_ICOUNT; break;
	case 0x3b: EA = Y - 5; LA(PCD); DEC_ICOUNT; break;
	case 0x3c: EA = Y - 4; LA(PCD); DEC_ICOUNT; break;
	case 0x3d: EA = Y - 3; LA(PCD); DEC_ICOUNT; break;
	case 0x3e: EA = Y - 2; LA(PCD); DEC_ICOUNT; break;
	case 0x3f: EA = Y - 1; LA(PCD); DEC_ICOUNT; break;

	case 0x40: EA = U; LA(PCD); DEC_ICOUNT; break;
	case 0x41: EA = U + 1; LA(PCD); DEC_ICOUNT; break;
	case 0x42: EA = U + 2; LA(PCD); DEC_ICOUNT; break;
	case 0x43: EA = U + 3; LA(PCD); DEC_ICOUNT; break;
	case 0x44: EA = U + 4; LA(PCD); DEC_ICOUNT; break;
	case 0x45: EA = U + 5; LA(PCD); DEC_ICOUNT; break;
	case 0x46: EA = U + 6; LA(PCD); DEC_ICOUNT; break;
	case 0x47: EA = U + 7; LA(PCD); DEC_ICOUNT; break;
	case 0x48: EA = U + 8; LA(PCD); DEC_ICOUNT; break;
	case 0x49: EA = U + 9; LA(PCD); DEC_ICOUNT; break;
	case 0x4a: EA = U + 10; LA(PCD); DEC_ICOUNT; break;
	case 0x4b: EA = U + 11; LA(PCD); DEC_ICOUNT; break;
	case 0x4c: EA = U + 12; LA(PCD); DEC_ICOUNT; break;
	case 0x4d: EA = U + 13; LA(PCD); DEC_ICOUNT; break;
	case 0x4e: EA = U + 14; LA(PCD); DEC_ICOUNT; break;
	case 0x4f: EA = U + 15; LA(PCD); DEC_ICOUNT; break;

	case 0x50: EA = U - 16; LA(PCD); DEC_ICOUNT; break;
	case 0x51: EA = U - 15; LA(PCD); DEC_ICOUNT; break;
	case 0x52: EA = U - 14; LA(PCD); DEC_ICOUNT; break;
	case 0x53: EA = U - 13; LA(PCD); DEC_ICOUNT; break;
	case 0x54: EA = U - 12; LA(PCD); DEC_ICOUNT; break;
	case 0x55: EA = U - 11; LA(PCD); DEC_ICOUNT; break;
	case 0x56: EA = U - 10; LA(PCD); DEC_ICOUNT; break;
	case 0x57: EA = U - 9; LA(PCD); DEC_ICOUNT; break;
	case 0x58: EA = U - 8; LA(PCD); DEC_ICOUNT; break;
	case 0x59: EA = U - 7; LA(PCD); DEC_ICOUNT; break;
	case 0x5a: EA = U - 6; LA(PCD); DEC_ICOUNT; break;
	case 0x5b: EA = U - 5; LA(PCD); DEC_ICOUNT; break;
	case 0x5c: EA = U - 4; LA(PCD); DEC_ICOUNT; break;
	case 0x5d: EA = U - 3; LA(PCD); DEC_ICOUNT; break;
	case 0x5e: EA = U - 2; LA(PCD); DEC_ICOUNT; break;
	case 0x5f: EA = U - 1; LA(PCD); DEC_ICOUNT; break;

	case 0x60: EA = S; LA(PCD); DEC_ICOUNT; break;
	case 0x61: EA = S + 1; LA(PCD); DEC_ICOUNT; break;
	case 0x62: EA = S + 2; LA(PCD); DEC_ICOUNT; break;
	case 0x63: EA = S + 3; LA(PCD); DEC_ICOUNT; break;
	case 0x64: EA = S + 4; LA(PCD); DEC_ICOUNT; break;
	case 0x65: EA = S + 5; LA(PCD); DEC_ICOUNT; break;
	case 0x66: EA = S + 6; LA(PCD); DEC_ICOUNT; break;
	case 0x67: EA = S + 7; LA(PCD); DEC_ICOUNT; break;
	case 0x68: EA = S + 8; LA(PCD); DEC_ICOUNT; break;
	case 0x69: EA = S + 9; LA(PCD); DEC_ICOUNT; break;
	case 0x6a: EA = S + 10; LA(PCD); DEC_ICOUNT; break;
	case 0x6b: EA = S + 11; LA(PCD); DEC_ICOUNT; break;
	case 0x6c: EA = S + 12; LA(PCD); DEC_ICOUNT; break;
	case 0x6d: EA = S + 13; LA(PCD); DEC_ICOUNT; break;
	case 0x6e: EA = S + 14; LA(PCD); DEC_ICOUNT; break;
	case 0x6f: EA = S + 15; LA(PCD); DEC_ICOUNT; break;

	case 0x70: EA = S - 16; LA(PCD); DEC_ICOUNT; break;
	case 0x71: EA = S - 15; LA(PCD); DEC_ICOUNT; break;
	case 0x72: EA = S - 14; LA(PCD); DEC_ICOUNT; break;
	case 0x73: EA = S - 13; LA(PCD); DEC_ICOUNT; break;
	case 0x74: EA = S - 12; LA(PCD); DEC_ICOUNT; break;
	case 0x75: EA = S - 11; LA(PCD); DEC_ICOUNT; break;
	case 0x76: EA = S - 10; LA(PCD); DEC_ICOUNT; break;
	case 0x77: EA = S - 9; LA(PCD); DEC_ICOUNT; break;
	case 0x78: EA = S - 8; LA(PCD); DEC_ICOUNT; break;
	case 0x79: EA = S - 7; LA(PCD); DEC_ICOUNT; break;
	case 0x7a: EA = S - 6; LA(PCD); DEC_ICOUNT; break;
	case 0x7b: EA = S - 5; LA(PCD); DEC_ICOUNT; break;
	case 0x7c: EA = S - 4; LA(PCD); DEC_ICOUNT; break;
	case 0x7d: EA = S - 3; LA(PCD); DEC_ICOUNT; break;
	case 0x7e: EA = S - 2; LA(PCD); DEC_ICOUNT; break;
	case 0x7f: EA = S - 1; LA(PCD); DEC_ICOUNT; break;

#define INDEXED_AUTOINC_1(Reg16) EA = Reg16; Reg16++; LA(PCD); SUB_ICOUNT(2)
#define INDEXED_AUTOINC_2(Reg16) EA = Reg16; Reg16 += 2; LA(PCD); SUB_ICOUNT(3)
#define INDEXED_AUTODEC_1(Reg16) Reg16--; EA = Reg16; LA(PCD); SUB_ICOUNT(2)
#define INDEXED_AUTODEC_2(Reg16) Reg16 -= 2; EA = Reg16; LA(PCD); SUB_ICOUNT(3)
#define INDEXED_00_OFFSET(Reg16) EA = Reg16; LA(PCD)
#define INDEXED_AC_OFFSET(Reg16, Reg8) EA = Reg16 + SIGNED(Reg8); LA(PCD); DEC_ICOUNT
#define INDEXED_AD_OFFSET(Reg16, RegD) EA = Reg16 + RegD; LA(PCD); LA(PCD+1); LA(PCD+2); SUB_ICOUNT(2)
#define INDEXED_08_OFFSET(Reg16) IMMBYTE(EA); DASM_SET_IDX2((uint8_t)EA); EA = Reg16 + SIGNED(EA); DEC_ICOUNT
#define INDEXED_16_OFFSET(Reg16) IMMWORD(EAP); DASM_SET_IDX2((uint16_t)EA); EA += Reg16; LA(PCD); SUB_ICOUNT(2)
#define INDEXED_08_PCRELATIVE    IMMBYTE(EA); DASM_SET_IDX2((uint8_t)EA); EA = PC + SIGNED(EA); DEC_ICOUNT
#define INDEXED_16_PCRELATIVE    IMMWORD(EAP); DASM_SET_IDX2((uint16_t)EA); EA += PC; LA(PCD); SUB_ICOUNT(3)
#define INDEXED_EXTENDED         IMMWORD(EAP); DASM_SET_IDX2((uint16_t)EA); LA(PCD)
#ifndef USE_DEBUGGER
#define INDEXED_INDIRECT         EAD = RM16(EAD); DEC_ICOUNT
#else
#define INDEXED_INDIRECT         ea_old = EAD; EAD = RM16(EAD); DASM_SET_MEM1((uint16_t)ea_old, (uint16_t)EAD); DEC_ICOUNT
#endif

	case 0x80: INDEXED_AUTOINC_1(X); break;	// ~:+1+2
	case 0x81: INDEXED_AUTOINC_2(X); break;	// ~:+1+3
	case 0x82: INDEXED_AUTODEC_1(X); break;	// ~:+1+2
	case 0x83: INDEXED_AUTODEC_2(X); break;	// ~:+1+3
	case 0x84: INDEXED_00_OFFSET(X); break;	// ~:+1+0
	case 0x85: INDEXED_AC_OFFSET(X,B); break;	// ~:+1+1
	case 0x86: INDEXED_AC_OFFSET(X,A); break;	// ~:+1+1
	case 0x87: illegal_ix(postbyte); EA = 0; break; /* ILLEGAL*/
	case 0x88: INDEXED_08_OFFSET(X); break;	// ~:+1+1
	case 0x89: INDEXED_16_OFFSET(X); break;	//~:+1+4
	case 0x8a: illegal_ix(postbyte); EA = 0; break; /* ILLEGAL*/
	case 0x8b: INDEXED_AD_OFFSET(X,D); break;	// ~:+1+4
	case 0x8c: INDEXED_08_PCRELATIVE; break; // ~:+1+1
	case 0x8d: INDEXED_16_PCRELATIVE; break;	//~:+1+5
	case 0x8e: illegal_ix(postbyte); EA = 0; break; /* ILLEGAL*/
	case 0x8f: illegal_ix(postbyte); INDEXED_EXTENDED; break;

	/* Indirect ,R+ not in my specs */
	case 0x90: illegal_ix(postbyte); INDEXED_AUTOINC_1(X); INDEXED_INDIRECT; break;	 /* ILLEGAL*/ // ~:+1+2+3
	case 0x91: INDEXED_AUTOINC_2(X); INDEXED_INDIRECT; break;	// ~:+1+3+3
	case 0x92: illegal_ix(postbyte); INDEXED_AUTODEC_1(X); INDEXED_INDIRECT; break;	 /* ILLEGAL*/ // ~:+1+2+3
	case 0x93: INDEXED_AUTODEC_2(X); INDEXED_INDIRECT; break;	// ~:+1+3+3
	case 0x94: INDEXED_00_OFFSET(X); INDEXED_INDIRECT; break;	// ~:+1+0+3
	case 0x95: INDEXED_AC_OFFSET(X,B); INDEXED_INDIRECT; break;	//~:+1+1+3
	case 0x96: INDEXED_AC_OFFSET(X,A); INDEXED_INDIRECT; break;	//~:+1+1+3
	case 0x97: illegal_ix(postbyte); EA = 0; break; /* ILLEGAL*/
	case 0x98: INDEXED_08_OFFSET(X); INDEXED_INDIRECT; break;	// ~:+1+1+3
	case 0x99: INDEXED_16_OFFSET(X); INDEXED_INDIRECT; break;	//~:+1+4+3
	case 0x9a: illegal_ix(postbyte); EA = 0; break; /* ILLEGAL*/
	case 0x9b: INDEXED_AD_OFFSET(X,D); INDEXED_INDIRECT; break;	//~:+1+4+3
	case 0x9c: INDEXED_08_PCRELATIVE; INDEXED_INDIRECT; break;	// ~:+1+1+3
	case 0x9d: INDEXED_16_PCRELATIVE; INDEXED_INDIRECT; break;	// ~:+1+5+3
	case 0x9e: illegal_ix(postbyte); EA = 0; break; /* ILLEGAL*/
	case 0x9f: INDEXED_EXTENDED; INDEXED_INDIRECT; break;	// ~:+1+2+3

	case 0xa0: INDEXED_AUTOINC_1(Y); break;	// ~:+1+2
	case 0xa1: INDEXED_AUTOINC_2(Y); break;	// ~:+1+3
	case 0xa2: INDEXED_AUTODEC_1(Y); break;	// ~:+1+2
	case 0xa3: INDEXED_AUTODEC_2(Y); break;	// ~:+1+3
	case 0xa4: INDEXED_00_OFFSET(Y); break;	// ~:+1+0
	case 0xa5: INDEXED_AC_OFFSET(Y,B); break;	// ~:+1+1
	case 0xa6: INDEXED_AC_OFFSET(Y,A); break;	// ~:+1+1
	case 0xa7: illegal_ix(postbyte); EA = 0; break; /* ILLEGAL*/
	case 0xa8: INDEXED_08_OFFSET(Y); break;	// ~:+1+1
	case 0xa9: INDEXED_16_OFFSET(Y); break;	//~:+1+4
	case 0xaa: illegal_ix(postbyte); EA = 0; break; /* ILLEGAL*/
	case 0xab: INDEXED_AD_OFFSET(Y,D); break;	// ~:+1+4
	case 0xac: INDEXED_08_PCRELATIVE; break; // ~:+1+1
	case 0xad: INDEXED_16_PCRELATIVE; break;	//~:+1+5
	case 0xae: illegal_ix(postbyte); EA = 0; break; /* ILLEGAL*/
	case 0xaf: illegal_ix(postbyte); INDEXED_EXTENDED; break;

	case 0xb0: illegal_ix(postbyte); INDEXED_AUTOINC_1(Y); INDEXED_INDIRECT; break;	 /* ILLEGAL*/ // ~:+1+2+3
	case 0xb1: INDEXED_AUTOINC_2(Y); INDEXED_INDIRECT; break;	// ~:+1+3+3
	case 0xb2: illegal_ix(postbyte); INDEXED_AUTODEC_1(Y); INDEXED_INDIRECT; break;	 /* ILLEGAL*/ // ~:+1+2+3
	case 0xb3: INDEXED_AUTODEC_2(Y); INDEXED_INDIRECT; break;	// ~:+1+3+3
	case 0xb4: INDEXED_00_OFFSET(Y); INDEXED_INDIRECT; break;	// ~:+1+0+3
	case 0xb5: INDEXED_AC_OFFSET(Y,B); INDEXED_INDIRECT; break;	//~:+1+1+3
	case 0xb6: INDEXED_AC_OFFSET(Y,A); INDEXED_INDIRECT; break;	//~:+1+1+3
	case 0xb7: illegal_ix(postbyte); EA = 0; break; /* ILLEGAL*/
	case 0xb8: INDEXED_08_OFFSET(Y); INDEXED_INDIRECT; break;	// ~:+1+1+3
	case 0xb9: INDEXED_16_OFFSET(Y); INDEXED_INDIRECT; break;	//~:+1+4+3
	case 0xba: illegal_ix(postbyte); EA = 0; break; /* ILLEGAL*/
	case 0xbb: INDEXED_AD_OFFSET(Y,D); INDEXED_INDIRECT; break;	//~:+1+4+3
	case 0xbc: INDEXED_08_PCRELATIVE; INDEXED_INDIRECT; break;	// ~:+1+1+3
	case 0xbd: INDEXED_16_PCRELATIVE; INDEXED_INDIRECT; break;	// ~:+1+5+3
	case 0xbe: illegal_ix(postbyte); EA = 0; break; /* ILLEGAL*/
	case 0xbf: INDEXED_EXTENDED; INDEXED_INDIRECT; break;

	case 0xc0: INDEXED_AUTOINC_1(U); break;	// ~:+1+2
	case 0xc1: INDEXED_AUTOINC_2(U); break;	// ~:+1+3
	case 0xc2: INDEXED_AUTODEC_1(U); break;	// ~:+1+2
	case 0xc3: INDEXED_AUTODEC_2(U); break;	// ~:+1+3
	case 0xc4: INDEXED_00_OFFSET(U); break;	// ~:+1+0
	case 0xc5: INDEXED_AC_OFFSET(U,B); break;	// ~:+1+1
	case 0xc6: INDEXED_AC_OFFSET(U,A); break;	// ~:+1+11
	case 0xc7: illegal_ix(postbyte); EA = 0; break; /*ILLEGAL*/
	case 0xc8: INDEXED_08_OFFSET(U); break;	// ~:+1+1
	case 0xc9: INDEXED_16_OFFSET(U); break;	//~:+1+4
	case 0xca: illegal_ix(postbyte); EA = 0; break; /*ILLEGAL*/
	case 0xcb: INDEXED_AD_OFFSET(U,D); break;	// ~:+1+4
	case 0xcc: INDEXED_08_PCRELATIVE; break; // ~:+1+1
	case 0xcd: INDEXED_16_PCRELATIVE; break;	//~:+1+5
	case 0xce: illegal_ix(postbyte); EA = 0; break; /*ILLEGAL*/
	case 0xcf: illegal_ix(postbyte); INDEXED_EXTENDED; break;

	case 0xd0: illegal_ix(postbyte); INDEXED_AUTOINC_1(U); INDEXED_INDIRECT; break;	 /* ILLEGAL*/ // ~:+1+2+3
	case 0xd1: INDEXED_AUTOINC_2(U); INDEXED_INDIRECT; break;	// ~:+1+3+3
	case 0xd2: illegal_ix(postbyte); INDEXED_AUTODEC_1(U); INDEXED_INDIRECT; break;	 /* ILLEGAL*/ // ~:+1+2+3
	case 0xd3: INDEXED_AUTODEC_2(U); INDEXED_INDIRECT; break;	// ~:+1+3+3
	case 0xd4: INDEXED_00_OFFSET(U); INDEXED_INDIRECT; break;	// ~:+1+0+3
	case 0xd5: INDEXED_AC_OFFSET(U,B); INDEXED_INDIRECT; break;	//~:+1+1+3
	case 0xd6: INDEXED_AC_OFFSET(U,A); INDEXED_INDIRECT; break;	//~:+1+1+3
	case 0xd7: illegal_ix(postbyte); EA = 0; break; /*ILLEGAL*/
	case 0xd8: INDEXED_08_OFFSET(U); INDEXED_INDIRECT; break;	// ~:+1+1+3
	case 0xd9: INDEXED_16_OFFSET(U); INDEXED_INDIRECT; break;	//~:+1+4+3
	case 0xda: illegal_ix(postbyte); EA = 0; break; /*ILLEGAL*/
	case 0xdb: INDEXED_AD_OFFSET(U,D); INDEXED_INDIRECT; break;	//~:+1+4+3
	case 0xdc: INDEXED_08_PCRELATIVE; INDEXED_INDIRECT; break;	// ~:+1+1+3
	case 0xdd: INDEXED_16_PCRELATIVE; INDEXED_INDIRECT; break;	// ~:+1+5+3
	case 0xde: illegal_ix(postbyte); EA = 0; break; /*ILLEGAL*/
	case 0xdf: INDEXED_EXTENDED; INDEXED_INDIRECT; break;

	case 0xe0: INDEXED_AUTOINC_1(S); break;	// ~:+1+2
	case 0xe1: INDEXED_AUTOINC_2(S); break;	// ~:+1+3
	case 0xe2: INDEXED_AUTODEC_1(S); break;	// ~:+1+2
	case 0xe3: INDEXED_AUTODEC_2(S); break;	// ~:+1+3
	case 0xe4: INDEXED_00_OFFSET(S); break;	// ~:+1+0
	case 0xe5: INDEXED_AC_OFFSET(S,B); break;	// ~:+1+1
	case 0xe6: INDEXED_AC_OFFSET(S,A); break;	// ~:+1+1
	case 0xe7: illegal_ix(postbyte); EA = 0; break; /*ILLEGAL*/
	case 0xe8: INDEXED_08_OFFSET(S); break;	// ~:+1+1
	case 0xe9: INDEXED_16_OFFSET(S); break;	//~:+1+4
	case 0xea: illegal_ix(postbyte); EA = 0; break; /*ILLEGAL*/
	case 0xeb: INDEXED_AD_OFFSET(S,D); break;	// ~:+1+4
	case 0xec: INDEXED_08_PCRELATIVE; break; // ~:+1+1
	case 0xed: INDEXED_16_PCRELATIVE; break;	//~:+1+5
	case 0xee: illegal_ix(postbyte); EA = 0; break; /*ILLEGAL*/
	case 0xef: illegal_ix(postbyte); INDEXED_EXTENDED; break;

	case 0xf0: illegal_ix(postbyte); INDEXED_AUTOINC_1(S); INDEXED_INDIRECT; break;	 /* ILLEGAL*/ // ~:+1+2+3
	case 0xf1: INDEXED_AUTOINC_2(S); INDEXED_INDIRECT; break;	// ~:+1+3+3
	case 0xf2: illegal_ix(postbyte); INDEXED_AUTODEC_1(S); INDEXED_INDIRECT; break;	 /* ILLEGAL*/ // ~:+1+2+3
	case 0xf3: INDEXED_AUTODEC_2(S); INDEXED_INDIRECT; break;	// ~:+1+3+3
	case 0xf4: INDEXED_00_OFFSET(S); INDEXED_INDIRECT; break;	// ~:+1+0+3
	case 0xf5: INDEXED_AC_OFFSET(S,B); INDEXED_INDIRECT; break;	//~:+1+1+3
	case 0xf6: INDEXED_AC_OFFSET(S,A); INDEXED_INDIRECT; break;	//~:+1+1+3
	case 0xf7: illegal_ix(postbyte); EA = 0; break; /*ILLEGAL*/
	case 0xf8: INDEXED_08_OFFSET(S); INDEXED_INDIRECT; break;	// ~:+1+1+3
	case 0xf9: INDEXED_16_OFFSET(S); INDEXED_INDIRECT; break;	//~:+1+4+3
	case 0xfa: illegal_ix(postbyte); EA = 0; break; /*ILLEGAL*/
	case 0xfb: INDEXED_AD_OFFSET(S,D); INDEXED_INDIRECT; break;	//~:+1+4+3
	case 0xfc: INDEXED_08_PCRELATIVE; INDEXED_INDIRECT; break;	// ~:+1+1+3
	case 0xfd: INDEXED_16_PCRELATIVE; INDEXED_INDIRECT; break;	// ~:+1+5+3
	case 0xfe: illegal_ix(postbyte); EA = 0; break; /*ILLEGAL*/
	case 0xff: INDEXED_EXTENDED; INDEXED_INDIRECT; break;
	}
#ifndef USE_CPU_REAL_MACHINE_CYCLE
	icount -= index_cycle_em[postbyte];
#endif
}

void MC6809::illegal(uint16_t ireg)
{
	if (FLG_SHOWMSG_UNDEFOP) {
		bool match = false;
		for(int i=0; i<ILLEGAL_PC_MAX; i++) {
			if (illegal_pc[i] == PPC) {
				match = true;
				break;
			}
		}
		if (!match) {
			logging->out_logf(LOG_ERROR,_T("MC6809: $%02X is illegal opcode at $%04x"), ireg, PPC);
			illegal_pc[illegal_pc_idx] = PPC;
			illegal_pc_idx = (illegal_pc_idx + 1) % ILLEGAL_PC_MAX;
		}
	}

	DASM_SET_ERR();
}

void MC6809::illegal_ix(uint8_t postbyte)
{
	if (FLG_SHOWMSG_UNDEFOP) {
		bool match = false;
		for(int i=0; i<ILLEGAL_PC_MAX; i++) {
			if (illegal_pc[i] == (PC - 1)) {
				match = true;
				break;
			}
		}
		if (!match) {
			logging->out_logf(LOG_ERROR,_T("MC6809: $%02X is illegal postbyte at $%04x"), postbyte, (PC - 1));
			illegal_pc[illegal_pc_idx] = (PC - 1);
			illegal_pc_idx = (illegal_pc_idx + 1) % ILLEGAL_PC_MAX;
		}
	}

	DASM_SET_ERR();
}

/* $00 NEG direct ?**** ~:6 */
void MC6809::neg_di()
{
	uint16_t r, t;
	DIRBYTE(t);
	DASM_SET_DIR1(DP, ea.b.l, (uint8_t)t);

	r = -t;
	CLR_NZVC;
	SET_FLAGS8(0, t, r);
	DEC_ICOUNT;

	WM(EAD, r);
}

/* $01 ILLEGAL, same as $00 */
void MC6809::err_neg_di()
{
	illegal(0x01);
	neg_di();
}

/* $02 ILLEGAL, same as $03 */
void MC6809::err_ngc_di()
{
	illegal(0x02);
	if (CC & 1) com_di();
	else neg_di();
}

/* $03 COM direct -**01 ~:6 */
void MC6809::com_di()
{
	uint8_t t;
	DIRBYTE(t);
	DASM_SET_DIR1(DP, ea.b.l, t);

	t = ~t;
	CLR_NZV;
	SET_NZ8(t);
	SEC;
	DEC_ICOUNT;

	WM(EAD, t);
}

/* $04 LSR direct -0*-* ~:6 */
void MC6809::lsr_di()
{
	uint8_t t;
	DIRBYTE(t);
	DASM_SET_DIR1(DP, ea.b.l, t);

	CLR_NZC;
	CC |= (t & CC_C);
	t >>= 1;
	SET_Z8(t);
	DEC_ICOUNT;

	WM(EAD, t);
}

/* $05 ILLEGAL, same as $04 */
void MC6809::err_lsr_di()
{
	illegal(0x05);
	lsr_di();
}

/* $06 ROR direct -**-* ~:6 */
void MC6809::ror_di()
{
	uint8_t t, r;
	DIRBYTE(t);
	DASM_SET_DIR1(DP, ea.b.l, t);

	r =  (CC & CC_C) << 7;
	CLR_NZC;
	CC |= (t & CC_C);
	r |= t >> 1;
	SET_NZ8(r);
	DEC_ICOUNT;

	WM(EAD, r);
}

/* $07 ASR direct ?**-* ~:6 */
void MC6809::asr_di()
{
	uint8_t t;
	DIRBYTE(t);
	DASM_SET_DIR1(DP, ea.b.l, t);

	CLR_NZC;
	CC |= (t & CC_C);
	t = (t & 0x80) | (t >> 1);
	SET_NZ8(t);
	DEC_ICOUNT;

	WM(EAD, t);
}

/* $08 ASL direct ?**** ~:6 */
void MC6809::asl_di()
{
	uint16_t t, r;
	DIRBYTE(t);
	DASM_SET_DIR1(DP, ea.b.l, (uint8_t)t);

	r = t << 1;
	CLR_NZVC;
	SET_FLAGS8(t, t, r);
	DEC_ICOUNT;

	WM(EAD, r);
}

/* $09 ROL direct -**** ~:6 */
void MC6809::rol_di()
{
	uint16_t t, r;
	DIRBYTE(t);
	DASM_SET_DIR1(DP, ea.b.l, (uint8_t)t);

	r = (CC & CC_C) | (t << 1);
	CLR_NZVC;
	SET_FLAGS8(t, t, r);
	DEC_ICOUNT;

	WM(EAD, r);
}

/* $0A DEC direct -***- ~:6 */
void MC6809::dec_di()
{
	uint8_t t;
	DIRBYTE(t);
	DASM_SET_DIR1(DP, ea.b.l, t);

	--t;
	CLR_NZV;
	SET_FLAGS8D(t);
	DEC_ICOUNT;

	WM(EAD, t);
}

/* $0B ILLEGAL, same as $0A */
void MC6809::err_dcc_di()
{
	illegal(0x0b);

	uint8_t t;
	DIRBYTE(t);
	DASM_SET_DIR1(DP, ea.b.l, t);

	--t;
	CLR_NZV;
	SET_FLAGS8D(t);
	if (t) SEC; else CLC;
	DEC_ICOUNT;

	WM(EAD, t);
}

/* $OC INC direct -***- ~:6 */
void MC6809::inc_di()
{
	uint8_t t;
	DIRBYTE(t);
	DASM_SET_DIR1(DP, ea.b.l, t);

	t++;
	CLR_NZV;
	SET_FLAGS8I(t);
	DEC_ICOUNT;

	WM(EAD, t);
}

/* $OD TST direct -**0- ~:6 */
void MC6809::tst_di()
{
	uint8_t t;
	DIRBYTE(t);
	DASM_SET_DIR1(DP, ea.b.l, t);

	CLR_NZV;
	SET_NZ8(t);
	SUB_ICOUNT(2);
}

/* $0E JMP direct ----- ~:3 */
void MC6809::jmp_di()
{
	DIRECT;
	SET_NPC;
	PCD = EAD;

	DASM_SET_JMP_DIR(DP, pc.b.l);
}

/* $0F CLR direct -0100 ~:6 */
void MC6809::clr_di()
{
	DIRECT;
	(void)RM(EAD);
	DASM_SET_DIR1(DP, ea.b.l, (uint8_t)0);

	DEC_ICOUNT;

	WM(EAD, 0);
	CLR_NZVC;
	SEZ;
}

/* $10 FLAG */

/* $11 FLAG */

/* $12 NOP inherent ----- ~:2 */
void MC6809::nop()
{
	LA(PCD);

	DASM_SET_INH();
}

/* $13 SYNC inherent ----- ~:2- */
void MC6809::sync()
{
	/* SYNC stops processing instructions until an interrupt request happens. */
	/* This doesn't require the corresponding interrupt to be enabled: if it  */
	/* is disabled, execution continues with the next instruction.            */
	int_state |= MC6809_SYNC;	 /* HJB 990227 */
	LA(PCD);
	DEC_ICOUNT;	// include 1 dead cycle

	DASM_SET_INH();
}

/* $14 ILLEGAL HALT? */
void MC6809::err_halt(uint16_t ireg)
{
	illegal(ireg);
	write_signal(SIG_CPU_HALT, 1, 1);
	int_state |= MC6809_INSN_HALT;	// internal HALT
}

/* $15 ILLEGAL HALT? */

/* $16 LBRA relative ----- ~:5 */
void MC6809::lbra()
{
	IMMWORD(EAP);
	SUB_ICOUNT(2);
	SET_NPC;
	PC += EA;

	DASM_SET_REL(ea.w.l);
}

/* $17 LBSR relative ----- ~:9 */
void MC6809::lbsr()
{
	IMMWORD(EAP);
	SUB_ICOUNT(2);
	LA(EAD);
	DEC_ICOUNT;
	PUSHWORD(pPC);
	SET_NPC;
	PC += EA;

	DASM_SET_REL(ea.w.l);
}

/* $18 ILLEGAL */
void MC6809::err_slcc()
{
	illegal(0x18);

	DEC_ICOUNT;
	CC &= ~CC_IF;
	if (CC & CC_Z) SEC;
	CC <<= 1;
	LA(PCD);

	DASM_SET_INH();
}

/* $19 DAA inherent (A) -**0* ~:2 */
void MC6809::daa()
{
	uint8_t msn, lsn;
	uint16_t t, cf = 0;
	msn = A & 0xf0; lsn = A & 0x0f;
	if(lsn > 0x09 || (CC & CC_H)) cf |= 0x06;
	if(msn > 0x80 && lsn > 0x09 ) cf |= 0x60;
	if(msn > 0x90 || (CC & CC_C)) cf |= 0x60;
	t = cf + A;
	CLR_NZV; /* keep carry from previous operation */
	SET_NZ8((uint8_t)t); SET_C8(t);
	A = (uint8_t)t;
	LA(PCD);

	DASM_SET_INH();
}

/* $1A ORCC immediate ##### ~:3 */
void MC6809::orcc()
{
	uint8_t t;
	IMMBYTE(t);
	CC |= t;
	LA(PCD);

	DASM_SET_IMM(t);
}

/* $1B ILLEGAL NOP?? ~:2 */
void MC6809::err_nop()
{
	illegal(0x1b);
	nop();
}

/* $1C ANDCC immediate ##### ~:3 */
void MC6809::andcc()
{
	uint8_t t;
	IMMBYTE(t);
	CC &= t;
	LA(PCD);

	DASM_SET_IMM(t);
}

/* $1D SEX inherent -**-- ~:2 */
void MC6809::sex()
{
	uint16_t t;
	t = SIGNED(B);
	D = t;
	//  CLR_NZV;    Tim Lindner 20020905: verified that V flag is not affected
	CLR_NZ;
	SET_NZ16(t);
	LA(PCD);

	DASM_SET_INH();
}

/* $1E EXG inherent ----- ~:8 */
void MC6809::exg()
{
	uint16_t t1, t2;
	uint8_t tb;

	IMMBYTE(tb);
	SET_NPC;
	if((tb ^ (tb >> 4)) & 0x08) {
		/* transfer $ff to both registers */
		t1 = t2 = 0xff;
	}
	else {
		switch(tb >> 4) {
		case  0: t1 = D;  break;
		case  1: t1 = X;  break;
		case  2: t1 = Y;  break;
		case  3: t1 = U;  break;
		case  4: t1 = S;  break;
		case  5: t1 = PC; break;
		case  8: t1 = A;  break;
		case  9: t1 = B;  break;
		case 10: t1 = CC; break;
		case 11: t1 = DP; break;
		default: t1 = 0xff; break;
		}
		switch(tb&15) {
		case  0: t2 = D;  break;
		case  1: t2 = X;  break;
		case  2: t2 = Y;  break;
		case  3: t2 = U;  break;
		case  4: t2 = S;  break;
		case  5: t2 = PC; break;
		case  8: t2 = A;  break;
		case  9: t2 = B;  break;
		case 10: t2 = CC; break;
		case 11: t2 = DP; break;
		default: t2 = 0xff; break;
		}
	}
	switch(tb >> 4) {
	case  0: D = t2;  break;
	case  1: X = t2;  break;
	case  2: Y = t2;  break;
	case  3: U = t2;  break;
	case  4: S = t2;  break;
	case  5: PC = t2; break;
	case  8: A = (uint8_t)t2;  break;
	case  9: B = (uint8_t)t2;  break;
	case 10: CC = (uint8_t)t2; break;
	case 11: DP = (uint8_t)t2; break;
	}
	switch(tb&15) {
	case  0: D = t1;  break;
	case  1: X = t1;  break;
	case  2: Y = t1;  break;
	case  3: U = t1;  break;
	case  4: S = t1;  break;
	case  5: PC = t1; break;
	case  8: A = (uint8_t)t1;  break;
	case  9: B = (uint8_t)t1;  break;
	case 10: CC = (uint8_t)t1; break;
	case 11: DP = (uint8_t)t1; break;
	}
	SUB_ICOUNT(6);

	DASM_SET_TFR(tb);
}

/* $1F TFR inherent ----- ~:6 */
void MC6809::tfr()
{
	uint8_t tb;
	uint16_t t;

	IMMBYTE(tb);
	SET_NPC;
	if((tb ^ (tb >> 4)) & 0x08) {
		/* transfer $ff to register */
		t = 0xff;
	}
	else {
		switch(tb >> 4) {
		case  0: t = D;  break;
		case  1: t = X;  break;
		case  2: t = Y;  break;
		case  3: t = U;  break;
		case  4: t = S;  break;
		case  5: t = PC; break;
		case  8: t = A;  break;
		case  9: t = B;  break;
		case 10: t = CC; break;
		case 11: t = DP; break;
		default: t = 0xff; break;
		}
	}
	switch(tb&15) {
	case  0: D = t;  break;
	case  1: X = t;  break;
	case  2: Y = t;  break;
	case  3: U = t;  break;
	case  4: S = t;  break;
	case  5: PC = t; break;
	case  8: A = (uint8_t)t;  break;
	case  9: B = (uint8_t)t;  break;
	case 10: CC = (uint8_t)t; break;
	case 11: DP = (uint8_t)t; break;
	}
	SUB_ICOUNT(4);

	DASM_SET_TFR(tb);
}

/* $20 BRA relative ----- ~:3 */
void MC6809::bra()
{
	uint8_t t;
	IMMBYTE(t);
	SET_NPC;
	PC += SIGNED(t);
	DEC_ICOUNT;

	DASM_SET_REL(t);
}

/* $21 BRN relative ----- ~:3 */
void MC6809::brn()
{
	uint8_t t;
	IMMBYTE(t);
	DEC_ICOUNT;

	DASM_SET_REL(t);
}

/* $1021 LBRN relative ----- ~:5 */
void MC6809::lbrn()
{
	IMMWORD(EAP);
	DEC_ICOUNT;

	DASM_SET_REL(ea.w.l);
}

/* $22 BHI relative ----- ~:3 */
void MC6809::bhi()
{
	BRANCH(!(CC & (CC_Z | CC_C)));
}

/* $1022 LBHI relative ----- ~:5 or 6 */
void MC6809::lbhi()
{
	LBRANCH(!(CC & (CC_Z | CC_C)));
}

/* $23 BLS relative ----- ~:3 */
void MC6809::bls()
{
	BRANCH((CC & (CC_Z | CC_C)));
}

/* $1023 LBLS relative ----- ~:5 or 6 */
void MC6809::lbls()
{
	LBRANCH((CC & (CC_Z | CC_C)));
}

/* $24 BCC relative ----- ~:3 */
void MC6809::bcc()
{
	BRANCH(!(CC & CC_C));
}

/* $1024 LBCC relative ----- ~:5 or 6 */
void MC6809::lbcc()
{
	LBRANCH(!(CC & CC_C));
}

/* $25 BCS relative ----- ~:3 */
void MC6809::bcs()
{
	BRANCH((CC & CC_C));
}

/* $1025 LBCS relative ----- ~:5 or 6 */
void MC6809::lbcs()
{
	LBRANCH((CC & CC_C));
}

/* $26 BNE relative ----- ~:3 */
void MC6809::bne()
{
	BRANCH(!(CC & CC_Z));
}

/* $1026 LBNE relative ----- ~:5 or 6 */
void MC6809::lbne()
{
	LBRANCH(!(CC & CC_Z));
}

/* $27 BEQ relative ----- ~:3 */
void MC6809::beq()
{
	BRANCH((CC & CC_Z));
}

/* $1027 LBEQ relative ----- ~:5 or 6 */
void MC6809::lbeq()
{
	LBRANCH((CC & CC_Z));
}

/* $28 BVC relative ----- ~:3 */
void MC6809::bvc()
{
	BRANCH(!(CC & CC_V));
}

/* $1028 LBVC relative ----- ~:5 or 6 */
void MC6809::lbvc()
{
	LBRANCH(!(CC & CC_V));
}

/* $29 BVS relative ----- ~:3 */
void MC6809::bvs()
{
	BRANCH((CC & CC_V));
}

/* $1029 LBVS relative ----- ~:5 or 6 */
void MC6809::lbvs()
{
	LBRANCH((CC & CC_V));
}

/* $2A BPL relative ----- ~:3 */
void MC6809::bpl()
{
	BRANCH(!(CC & CC_N));
}

/* $102A LBPL relative ----- ~:5 or 6 */
void MC6809::lbpl()
{
	LBRANCH(!(CC & CC_N));
}

/* $2B BMI relative ----- ~:3 */
void MC6809::bmi()
{
	BRANCH((CC & CC_N));
}

/* $102B LBMI relative ----- ~:5 or 6 */
void MC6809::lbmi()
{
	LBRANCH((CC & CC_N));
}

/* $2C BGE relative ----- ~:3 */
void MC6809::bge()
{
	BRANCH(!NXORV);
}

/* $102C LBGE relative ----- ~:5 or 6 */
void MC6809::lbge()
{
	LBRANCH(!NXORV);
}

/* $2D BLT relative ----- ~:3 */
void MC6809::blt()
{
	BRANCH(NXORV);
}

/* $102D LBLT relative ----- ~:5 or 6 */
void MC6809::lblt()
{
	LBRANCH(NXORV);
}

/* $2E BGT relative ----- ~:3 */
void MC6809::bgt()
{
	BRANCH(!(NXORV || (CC & CC_Z)));
}

/* $102E LBGT relative ----- ~:5 or 6 */
void MC6809::lbgt()
{
	LBRANCH(!(NXORV || (CC & CC_Z)));
}

/* $2F BLE relative ----- ~:3 */
void MC6809::ble()
{
	BRANCH((NXORV || (CC & CC_Z)));
}

/* $102F LBLE relative ----- ~:5 or 6 */
void MC6809::lble()
{
	LBRANCH((NXORV || (CC & CC_Z)));
}

/* $30 LEAX indexed --*-- ~:3+ */
void MC6809::leax()
{
	fetch_effective_address();
	X = EA;
	CLR_Z;
	SET_Z(X);
	DEC_ICOUNT;

//	DASM_SET_MEM1(ea.w.l, X);
}

/* $31 LEAY indexed --*-- ~:3+ */
void MC6809::leay()
{
	fetch_effective_address();
	Y = EA;
	CLR_Z;
	SET_Z(Y);
	DEC_ICOUNT;

//	DASM_SET_MEM1(ea.w.l, Y);
}

/* $32 LEAS indexed ----- ~:3+ */
void MC6809::leas()
{
	fetch_effective_address();
	S = EA;
	int_state |= MC6809_LDS;
	DEC_ICOUNT;

//	DASM_SET_MEM1(ea.w.l, S);
}

/* $33 LEAU indexed ----- ~:3+ */
void MC6809::leau()
{
	fetch_effective_address();
	U = EA;
	DEC_ICOUNT;

//	DASM_SET_MEM1(ea.w.l, U);
}

/* $34 PSHS inherent ----- ~:5+ */
void MC6809::pshs()
{
	uint8_t t;
	IMMBYTE(t);
	SET_NPC;
	SUB_ICOUNT(2);
	LAP(SD);
	if(t & 0x80) { PUSHWORD(pPC); SUBALL_ICOUNT(2); }
	if(t & 0x40) { PUSHWORD(pU);  SUBALL_ICOUNT(2); }
	if(t & 0x20) { PUSHWORD(pY);  SUBALL_ICOUNT(2); }
	if(t & 0x10) { PUSHWORD(pX);  SUBALL_ICOUNT(2); }
	if(t & 0x08) { PUSHBYTE(DP);  SUBALL_ICOUNT(1); }
	if(t & 0x04) { PUSHBYTE(B);   SUBALL_ICOUNT(1); }
	if(t & 0x02) { PUSHBYTE(A);   SUBALL_ICOUNT(1); }
	if(t & 0x01) { PUSHBYTE(CC);  SUBALL_ICOUNT(1); }

	DASM_SET_PSH(t);
}

/* $35 PULS inherent ----- ~:5+ */
void MC6809::puls()
{
	uint8_t t;
	IMMBYTE(t);
	SET_NPC;
	SUB_ICOUNT(2);
	if(t & 0x01) { PULLBYTE(CC);  SUBALL_ICOUNT(1); }
	if(t & 0x02) { PULLBYTE(A);   SUBALL_ICOUNT(1); }
	if(t & 0x04) { PULLBYTE(B);   SUBALL_ICOUNT(1); }
	if(t & 0x08) { PULLBYTE(DP);  SUBALL_ICOUNT(1); }
	if(t & 0x10) { PULLWORD(XD);  SUBALL_ICOUNT(2); }
	if(t & 0x20) { PULLWORD(YD);  SUBALL_ICOUNT(2); }
	if(t & 0x40) { PULLWORD(UD);  SUBALL_ICOUNT(2); }
	if(t & 0x80) { PULLWORD(PCD); SUBALL_ICOUNT(2); }
	LAP(SD);

	DASM_SET_PSH(t);
}

/* $36 PSHU inherent ----- ~:5+ */
void MC6809::pshu()
{
	uint8_t t;
	IMMBYTE(t);
	SET_NPC;
	SUB_ICOUNT(2);
	LAP(UD);
	if(t & 0x80) { PSHUWORD(pPC); SUBALL_ICOUNT(2); }
	if(t & 0x40) { PSHUWORD(pS);  SUBALL_ICOUNT(2); }
	if(t & 0x20) { PSHUWORD(pY);  SUBALL_ICOUNT(2); }
	if(t & 0x10) { PSHUWORD(pX);  SUBALL_ICOUNT(2); }
	if(t & 0x08) { PSHUBYTE(DP);  SUBALL_ICOUNT(1); }
	if(t & 0x04) { PSHUBYTE(B);   SUBALL_ICOUNT(1); }
	if(t & 0x02) { PSHUBYTE(A);   SUBALL_ICOUNT(1); }
	if(t & 0x01) { PSHUBYTE(CC);  SUBALL_ICOUNT(1); }

	DASM_SET_PSH(t);
}

/* $37 PULU inherent ----- ~:5+ */
void MC6809::pulu()
{
	uint8_t t;
	IMMBYTE(t);
	SET_NPC;
	SUB_ICOUNT(2);
	if(t & 0x01) { PULUBYTE(CC);  SUBALL_ICOUNT(1); }
	if(t & 0x02) { PULUBYTE(A);   SUBALL_ICOUNT(1); }
	if(t & 0x04) { PULUBYTE(B);   SUBALL_ICOUNT(1); }
	if(t & 0x08) { PULUBYTE(DP);  SUBALL_ICOUNT(1); }
	if(t & 0x10) { PULUWORD(XD);  SUBALL_ICOUNT(2); }
	if(t & 0x20) { PULUWORD(YD);  SUBALL_ICOUNT(2); }
	if(t & 0x40) { PULUWORD(SD);  SUBALL_ICOUNT(2); }
	if(t & 0x80) { PULUWORD(PCD); SUBALL_ICOUNT(2); }
	LAP(UD);

	DASM_SET_PSH(t);
}

/* $38 ILLEGAL CWAI? */
void MC6809::err_cwai()
{
	illegal(0x38);
	cwai();
}

/* $39 RTS inherent ----- ~:5 */
void MC6809::rts()
{
	LA(PCD);
	SET_NPC;
	PULLWORD(PCD);
	DEC_ICOUNT;

	DASM_SET_INH();
}

/* $3A ABX inherent ----- ~:3 */
void MC6809::abx()
{
	X += B;
	LA(PCD);
	DEC_ICOUNT;

	DASM_SET_INH();
}

/* $3B RTI inherent ##### ~:6 or 15 */
void MC6809::rti()
{
	uint8_t t;
	LA(PCD);
	SET_NPC;
	PULLBYTE(CC);
	t = CC & CC_E;		/* HJB 990225: entire state saved? */
	if(t) {
		SUBALL_ICOUNT(9);
		PULLBYTE(A);
		PULLBYTE(B);
		PULLBYTE(DP);
		PULLWORD(XD);
		PULLWORD(YD);
		PULLWORD(UD);
	}
	PULLWORD(PCD);
	LA(SD);

	DASM_SET_INH();
}

/* $3C CWAI inherent ----1 ~:19+ */
void MC6809::cwai()
{
	uint8_t t;
	IMMBYTE(t);
	SET_NPC;
	CC &= t;
	LA(PCD);
	DEC_ICOUNT;
	/*
	 * CWAI stacks the entire machine state on the hardware stack,
	 * then waits for an interrupt; when the interrupt is taken
	 * later, the state is *not* saved again after CWAI.
	 */
	CC |= CC_E; 		/* HJB 990225: save entire state */
	PUSHWORD(pPC);
	PUSHWORD(pU);
	PUSHWORD(pY);
	PUSHWORD(pX);
	PUSHBYTE(DP);
	PUSHBYTE(B);
	PUSHBYTE(A);
	PUSHBYTE(CC);
	int_state |= MC6809_CWAI;	 /* HJB 990228 */
	DEC_ICOUNT;

	DASM_SET_IMM(t);
}

/* $3D MUL inherent --*-@ ~:11 */
void MC6809::mul()
{
	uint16_t t;
	t = A * B;
	CLR_ZC; SET_Z16(t);
	if(t & 0x80) {
		SEC;
	}
	D = t;
	LA(PCD);
	SUB_ICOUNT(9);

	DASM_SET_INH();
}

/* $3E ILLEGAL RESET?? */
void MC6809::err_reset()
{
	illegal(0x3e);
	now_reset = true;
	write_signal(SIG_CPU_RESET, 0, 1);
}

/* $3F SWI (SWI2 SWI3) absolute indirect ----- ~:19 */
void MC6809::swi()
{
	DASM_SET_SWI();

	LA(PCD);
	DEC_ICOUNT;

	CC |= CC_E; 			/* HJB 980225: save entire state */
	PUSHWORD(pPC);
	PUSHWORD(pU);
	PUSHWORD(pY);
	PUSHWORD(pX);
	PUSHBYTE(DP);
	PUSHBYTE(B);
	PUSHBYTE(A);
	PUSHBYTE(CC);
	CC |= CC_IF | CC_II;	/* inhibit FIRQ and IRQ */
	DEC_ICOUNT;
	WS_BABS(0x01);
	SET_NPC;
	PCD = RM16(0xfffa);
	WS_BABS(0x00);
	DEC_ICOUNT;
}

/* $103F SWI2 absolute indirect ----- ~:20 */
void MC6809::swi2()
{
	DASM_SET_SWI2();

	LA(PCD);
	DEC_ICOUNT;

	CC |= CC_E; 			/* HJB 980225: save entire state */
	PUSHWORD(pPC);
	PUSHWORD(pU);
	PUSHWORD(pY);
	PUSHWORD(pX);
	PUSHBYTE(DP);
	PUSHBYTE(B);
	PUSHBYTE(A);
	PUSHBYTE(CC);
	DEC_ICOUNT;
	WS_BABS(0x01);
	SET_NPC;
	PCD = RM16(0xfff4);
	WS_BABS(0x00);
	DEC_ICOUNT;
}

/* $113F SWI3 absolute indirect ----- ~:20 */
void MC6809::swi3()
{
	DASM_SET_SWI3();

	LA(PCD);
	DEC_ICOUNT;

	CC |= CC_E; 			/* HJB 980225: save entire state */
	PUSHWORD(pPC);
	PUSHWORD(pU);
	PUSHWORD(pY);
	PUSHWORD(pX);
	PUSHBYTE(DP);
	PUSHBYTE(B);
	PUSHBYTE(A);
	PUSHBYTE(CC);
	DEC_ICOUNT;
	WS_BABS(0x01);
	SET_NPC;
	PCD = RM16(0xfff2);
	WS_BABS(0x00);
	DEC_ICOUNT;
}

/* $40 NEGA inherent ?**** ~:2 */
void MC6809::nega()
{
	uint16_t r;
	r = -A;
	CLR_NZVC;
	SET_FLAGS8(0, A, r);
	A = (uint8_t)r;
	LA(PCD);

	DASM_SET_INH();
}

/* $41 ILLEGAL, same as $40 */
void MC6809::err_nega()
{
	illegal(0x41);
	nega();
}

/* $42 ILLEGAL, same as $43 */
void MC6809::err_ngca()
{
	illegal(0x42);
	if (CC & 1) coma();
	else nega();
}

/* $43 COMA inherent -**01 ~:2 */
void MC6809::coma()
{
	A = ~A;
	CLR_NZV;
	SET_NZ8(A);
	SEC;
	LA(PCD);

	DASM_SET_INH();
}

/* $44 LSRA inherent -0*-* ~:2 */
void MC6809::lsra()
{
	CLR_NZC;
	CC |= (A & CC_C);
	A >>= 1;
	SET_Z8(A);
	LA(PCD);

	DASM_SET_INH();
}

/* $45 ILLEGAL, same as $44 */
void MC6809::err_lsra()
{
	illegal(0x45);
	lsra();
}

/* $46 RORA inherent -**-* ~:2 */
void MC6809::rora()
{
	uint8_t r;
	r = (CC & CC_C) << 7;
	CLR_NZC;
	CC |= (A & CC_C);
	r |= A >> 1;
	SET_NZ8(r);
	A = r;
	LA(PCD);

	DASM_SET_INH();
}

/* $47 ASRA inherent ?**-* ~:2 */
void MC6809::asra()
{
	CLR_NZC;
	CC |= (A & CC_C);
	A = (A & 0x80) | (A >> 1);
	SET_NZ8(A);
	LA(PCD);

	DASM_SET_INH();
}

/* $48 ASLA inherent ?**** ~:2 */
void MC6809::asla()
{
	uint16_t r;
	r = A << 1;
	CLR_NZVC;
	SET_FLAGS8(A, A, r);
	A = (uint8_t)r;
	LA(PCD);

	DASM_SET_INH();
}

/* $49 ROLA inherent -**** ~:2 */
void MC6809::rola()
{
	uint16_t t, r;
	t = A;
	r = (CC & CC_C) | (t << 1);
	CLR_NZVC; SET_FLAGS8(t, t, r);
	A = (uint8_t)r;
	LA(PCD);

	DASM_SET_INH();
}

/* $4A DECA inherent -***- ~:2 */
void MC6809::deca()
{
	--A;
	CLR_NZV;
	SET_FLAGS8D(A);
	LA(PCD);

	DASM_SET_INH();
}

/* $4B ILLEGAL, same as $4A */
void MC6809::err_dcca()
{
	illegal(0x4a);

	--A;
	CLR_NZV;
	SET_FLAGS8D(A);
	if (A) SEC; else CLC;
	LA(PCD);

	DASM_SET_INH();
}

/* $4C INCA inherent -***- ~:2 */
void MC6809::inca()
{
	++A;
	CLR_NZV;
	SET_FLAGS8I(A);
	LA(PCD);

	DASM_SET_INH();
}

/* $4D TSTA inherent -**0- ~:2 */
void MC6809::tsta()
{
	CLR_NZV;
	SET_NZ8(A);
	LA(PCD);

	DASM_SET_INH();
}

/* $4E ILLEGAL, same as $4F */
void MC6809::err_clca()
{
	illegal(0x4e);
	A = 0;
	CLR_NZV; SEZ;
	LA(PCD);

	DASM_SET_INH();
}

/* $4F CLRA inherent -0100 ~:2 */
void MC6809::clra()
{
	A = 0;
	CLR_NZVC; SEZ;
	LA(PCD);

	DASM_SET_INH();
}

/* $50 NEGB inherent ?**** ~:2 */
void MC6809::negb()
{
	uint16_t r;
	r = -B;
	CLR_NZVC;
	SET_FLAGS8(0, B, r);
	B = (uint8_t)r;
	LA(PCD);

	DASM_SET_INH();
}

/* $51 ILLEGAL, same as $50 */
void MC6809::err_negb()
{
	illegal(0x51);
	negb();
}

/* $52 ILLEGAL, same as $53 */
void MC6809::err_ngcb()
{
	illegal(0x52);
	if (CC & 1) comb();
	else negb();
}

/* $53 COMB inherent -**01 ~:2 */
void MC6809::comb()
{
	B = ~B;
	CLR_NZV;
	SET_NZ8(B);
	SEC;
	LA(PCD);

	DASM_SET_INH();
}

/* $54 LSRB inherent -0*-* ~:2 */
void MC6809::lsrb()
{
	CLR_NZC;
	CC |= (B & CC_C);
	B >>= 1;
	SET_Z8(B);
	LA(PCD);

	DASM_SET_INH();
}

/* $55 ILLEGAL, same as $54 */
void MC6809::err_lsrb()
{
	illegal(0x55);
	lsrb();
}

/* $56 RORB inherent -**-* ~:2 */
void MC6809::rorb()
{
	uint8_t r;
	r = (CC & CC_C) << 7;
	CLR_NZC;
	CC |= (B & CC_C);
	r |= B >> 1;
	SET_NZ8(r);
	B = r;
	LA(PCD);

	DASM_SET_INH();
}

/* $57 ASRB inherent ?**-* ~:2 */
void MC6809::asrb()
{
	CLR_NZC;
	CC |= (B & CC_C);
	B = (B & 0x80) | (B >> 1);
	SET_NZ8(B);
	LA(PCD);

	DASM_SET_INH();
}

/* $58 ASLB inherent ?**** ~:2 */
void MC6809::aslb()
{
	uint16_t r;
	r = B << 1;
	CLR_NZVC;
	SET_FLAGS8(B, B, r);
	B = (uint8_t)r;
	LA(PCD);

	DASM_SET_INH();
}

/* $59 ROLB inherent -**** ~:2 */
void MC6809::rolb()
{
	uint16_t t, r;
	t = B;
	r = CC & CC_C;
	r |= t << 1;
	CLR_NZVC;
	SET_FLAGS8(t, t, r);
	B = (uint8_t)r;
	LA(PCD);

	DASM_SET_INH();
}

/* $5A DECB inherent -***- ~:2 */
void MC6809::decb()
{
	--B;
	CLR_NZV;
	SET_FLAGS8D(B);
	LA(PCD);

	DASM_SET_INH();
}

/* $5B ILLEGAL, same as $5A */
void MC6809::err_dccb()
{
	illegal(0x5b);

	--B;
	CLR_NZV;
	SET_FLAGS8D(B);
	if (B) SEC; else CLC;
	LA(PCD);

	DASM_SET_INH();
}

/* $5C INCB inherent -***- ~:2 */
void MC6809::incb()
{
	++B;
	CLR_NZV;
	SET_FLAGS8I(B);
	LA(PCD);

	DASM_SET_INH();
}

/* $5D TSTB inherent -**0- ~:2 */
void MC6809::tstb()
{
	CLR_NZV;
	SET_NZ8(B);
	LA(PCD);

	DASM_SET_INH();
}

/* $5E ILLEGAL, same as $5F */
void MC6809::err_clcb()
{
	illegal(0x5e);
	B = 0;
	CLR_NZV; SEZ;
	LA(PCD);

	DASM_SET_INH();
}

/* $5F CLRB inherent -0100 ~:2 */
void MC6809::clrb()
{
	B = 0;
	CLR_NZVC; SEZ;
	LA(PCD);

	DASM_SET_INH();
}

/* $60 NEG indexed ?**** ~:5+ */
void MC6809::neg_ix()
{
	uint16_t r, t;
	fetch_effective_address();
	t = RM(EAD);
	DASM_SET_MEM1(ea.w.l, (uint8_t)t);

	r = -t;
	CLR_NZVC;
	SET_FLAGS8(0, t, r);
	DEC_ICOUNT;

	WM(EAD, r);
}

/* $61 ILLEGAL, same as $60 */
void MC6809::err_neg_ix()
{
	illegal(0x61);
	neg_ix();
}

/* $62 ILLEGAL, same as $63 */
void MC6809::err_ngc_ix()
{
	illegal(0x62);
	if (CC & 1) com_ix();
	else neg_ix();
}

/* $63 COM indexed -**01 ~:5+ */
void MC6809::com_ix()
{
	uint8_t t;
	fetch_effective_address();
	t = RM(EAD);
	DASM_SET_MEM1(ea.w.l, t);

	t = ~t;
	CLR_NZV;
	SET_NZ8(t);
	SEC;
	DEC_ICOUNT;

	WM(EAD, t);
}

/* $64 LSR indexed -0*-* ~:5+ */
void MC6809::lsr_ix()
{
	uint8_t t;
	fetch_effective_address();
	t = RM(EAD);
	DASM_SET_MEM1(ea.w.l, t);

	CLR_NZC;
	CC |= (t & CC_C);
	t >>= 1; SET_Z8(t);
	DEC_ICOUNT;

	WM(EAD, t);
}

/* $65 ILLEGAL, same as $64 */
void MC6809::err_lsr_ix()
{
	illegal(0x65);
	lsr_ix();
}

/* $66 ROR indexed -**-* ~:5+ */
void MC6809::ror_ix()
{
	uint8_t t, r;
	fetch_effective_address();
	t = RM(EAD);
	DASM_SET_MEM1(ea.w.l, t);

	r = (CC & CC_C) << 7;
	CLR_NZC;
	CC |= (t & CC_C);
	r |= t >> 1; SET_NZ8(r);
	DEC_ICOUNT;

	WM(EAD, r);
}

/* $67 ASR indexed ?**-* ~:5+ */
void MC6809::asr_ix()
{
	uint8_t t;
	fetch_effective_address();
	t = RM(EAD);
	DASM_SET_MEM1(ea.w.l, t);

	CLR_NZC;
	CC |= (t & CC_C);
	t = (t & 0x80) | (t >> 1);
	SET_NZ8(t);
	DEC_ICOUNT;

	WM(EAD, t);
}

/* $68 ASL indexed ?**** ~:5+ */
void MC6809::asl_ix()
{
	uint16_t t, r;
	fetch_effective_address();
	t = RM(EAD);
	DASM_SET_MEM1(ea.w.l, (uint8_t)t);

	r = t << 1;
	CLR_NZVC;
	SET_FLAGS8(t, t, r);
	DEC_ICOUNT;

	WM(EAD, r);
}

/* $69 ROL indexed -**** ~:5+ */
void MC6809::rol_ix()
{
	uint16_t t, r;
	fetch_effective_address();
	t = RM(EAD);
	DASM_SET_MEM1(ea.w.l, (uint8_t)t);

	r = CC & CC_C;
	r |= t << 1;
	CLR_NZVC;
	SET_FLAGS8(t, t, r);
	DEC_ICOUNT;

	WM(EAD, r);
}

/* $6A DEC indexed -***- ~:5+ */
void MC6809::dec_ix()
{
	uint8_t t;
	fetch_effective_address();
	t = RM(EAD);
	DASM_SET_MEM1(ea.w.l, t);

	--t;
	CLR_NZV; SET_FLAGS8D(t);
	DEC_ICOUNT;

	WM(EAD, t);
}

/* $6B ILLEGAL, same as $6A */
void MC6809::err_dcc_ix()
{
	illegal(0x6b);

	uint8_t t;
	fetch_effective_address();
	t = RM(EAD);
	DASM_SET_MEM1(ea.w.l, t);

	--t;
	CLR_NZV; SET_FLAGS8D(t);
	if (t) SEC; else CLC;
	DEC_ICOUNT;

	WM(EAD, t);
}

/* $6C INC indexed -***- ~:5+ */
void MC6809::inc_ix()
{
	uint8_t t;
	fetch_effective_address();
	t = RM(EAD);
	DASM_SET_MEM1(ea.w.l, t);

	t++;
	CLR_NZV; SET_FLAGS8I(t);
	DEC_ICOUNT;

	WM(EAD, t);
}

/* $6D TST indexed -**0- ~:5+ */
void MC6809::tst_ix()
{
	uint8_t t;
	fetch_effective_address();
	t = RM(EAD);
	DASM_SET_MEM1(ea.w.l, t);

	CLR_NZV;
	SET_NZ8(t);
	SUB_ICOUNT(2);
}

/* $6E JMP indexed ----- ~:2+ */
void MC6809::jmp_ix()
{
	fetch_effective_address();
	PCD = EAD;
}

/* $6F CLR indexed -0100 ~:5+ */
void MC6809::clr_ix()
{
	fetch_effective_address();
	(void)RM(EAD);
	DEC_ICOUNT;

	WM(EAD, 0);
	CLR_NZVC; SEZ;

	DASM_SET_MEM2(ea.w.l, (uint8_t)0, true);
}

/* $70 NEG extended ?**** ~:7 */
void MC6809::neg_ex()
{
	uint16_t r, t;
	EXTBYTE(t);
	DASM_SET_EXT1(ea.w.l, t);

	r = -t;
	CLR_NZVC; SET_FLAGS8(0, t, r);
	DEC_ICOUNT;

	WM(EAD, r);
}

/* $71 ILLEGAL, same as $70 */
void MC6809::err_neg_ex()
{
	illegal(0x71);
	neg_ex();
}

/* $72 ILLEGAL, same as $73 */
void MC6809::err_ngc_ex()
{
	illegal(0x72);
	if (CC & 1) com_ex();
	else neg_ex();
}

/* $73 COM extended -**01 ~:7 */
void MC6809::com_ex()
{
	uint8_t t;
	EXTBYTE(t);
	DASM_SET_EXT1(ea.w.l, t);

	t = ~t;
	CLR_NZV; SET_NZ8(t); SEC;
	DEC_ICOUNT;

	WM(EAD, t);
}

/* $74 LSR extended -0*-* ~:7 */
void MC6809::lsr_ex()
{
	uint8_t t;
	EXTBYTE(t);
	DASM_SET_EXT1(ea.w.l, t);

	CLR_NZC; CC |= (t & CC_C);
	t >>=1; SET_Z8(t);
	DEC_ICOUNT;

	WM(EAD, t);
}

/* $75 ILLEGAL, same as $74 */
void MC6809::err_lsr_ex()
{
	illegal(0x75);
	lsr_ex();
}

/* $76 ROR extended -**-* ~:7 */
void MC6809::ror_ex()
{
	uint8_t t, r;
	EXTBYTE(t);
	DASM_SET_EXT1(ea.w.l, t);

	r = (CC & CC_C) << 7;
	CLR_NZC; CC |= (t & CC_C);
	r |= t >> 1; SET_NZ8(r);
	DEC_ICOUNT;

	WM(EAD, r);
}

/* $77 ASR extended ?**-* ~:7 */
void MC6809::asr_ex()
{
	uint8_t t;
	EXTBYTE(t);
	DASM_SET_EXT1(ea.w.l, t);

	CLR_NZC; CC |= (t & CC_C);
	t = (t & 0x80) | (t >> 1);
	SET_NZ8(t);
	DEC_ICOUNT;

	WM(EAD, t);
}

/* $78 ASL extended ?**** ~:7 */
void MC6809::asl_ex()
{
	uint16_t t, r;
	EXTBYTE(t);
	DASM_SET_EXT1(ea.w.l, (uint8_t)t);

	r = t << 1;
	CLR_NZVC; SET_FLAGS8(t, t, r);
	DEC_ICOUNT;

	WM(EAD, r);

}

/* $79 ROL extended -**** ~:7 */
void MC6809::rol_ex()
{
	uint16_t t, r;
	EXTBYTE(t);
	DASM_SET_EXT1(ea.w.l, (uint8_t)t);

	r = (CC & CC_C) | (t << 1);
	CLR_NZVC; SET_FLAGS8(t, t, r);
	DEC_ICOUNT;

	WM(EAD, r);
}

/* $7A DEC extended -***- ~:7 */
void MC6809::dec_ex()
{
	uint8_t t;
	EXTBYTE(t);
	DASM_SET_EXT1(ea.w.l, t);

	--t;
	CLR_NZV; SET_FLAGS8D(t);
	DEC_ICOUNT;

	WM(EAD, t);
}

/* $7B ILLEGAL, same as $7A */
void MC6809::err_dcc_ex()
{
	illegal(0x7b);

	uint8_t t;
	EXTBYTE(t);
	DASM_SET_EXT1(ea.w.l, t);

	--t;
	CLR_NZV; SET_FLAGS8D(t);
	if (t) SEC; else CLC;
	DEC_ICOUNT;

	WM(EAD, t);
}

/* $7C INC extended -***- ~:7 */
void MC6809::inc_ex()
{
	uint8_t t;
	EXTBYTE(t);
	DASM_SET_EXT1(ea.w.l, t);

	t++;
	CLR_NZV; SET_FLAGS8I(t);
	DEC_ICOUNT;

	WM(EAD, t);
}

/* $7D TST extended -**0- ~:7 */
void MC6809::tst_ex()
{
	uint8_t t;
	EXTBYTE(t);
	DASM_SET_EXT1(ea.w.l, t);

	CLR_NZV; SET_NZ8(t);
	SUB_ICOUNT(2);
}

/* $7E JMP extended ----- ~:4 */
void MC6809::jmp_ex()
{
	EXTENDED;
	SET_NPC;
	PCD = EAD;

	DASM_SET_JMP(ea.w.l);
}

/* $7F CLR extended -0100 ~:7 */
void MC6809::clr_ex()
{
	EXTENDED;
	(void)RM(EAD);
	DEC_ICOUNT;
	WM(EAD, 0);
	CLR_NZVC; SEZ;

	DASM_SET_EXT2(ea.w.l, (uint8_t)0, true);
}

/* $80 SUBA immediate ?**** ~:2 */
void MC6809::suba_im()
{
	uint16_t t, r;
	IMMBYTE(t);
	r = A - t;
	CLR_NZVC;
	SET_FLAGS8(A, t, r);
	A = (uint8_t)r;

	DASM_SET_IMM((uint8_t)t);
}

/* $81 CMPA immediate ?**** ~:2 */
void MC6809::cmpa_im()
{
	uint16_t t, r;
	IMMBYTE(t);
	r = A - t;
	CLR_NZVC;
	SET_FLAGS8(A, t, r);

	DASM_SET_IMM((uint8_t)t);
}

/* $82 SBCA immediate ?**** ~:2 */
void MC6809::sbca_im()
{
	uint16_t t, r;
	IMMBYTE(t);
	r = A - t - (CC & CC_C);
	CLR_NZVC;
	SET_FLAGS8(A, t, r);
	A = (uint8_t)r;

	DASM_SET_IMM((uint8_t)t);
}

/* $83 SUBD (CMPD CMPU) immediate -**** ~:4 */
void MC6809::subd_im()
{
	uint32_t r, d;
	pair32_t b;
	IMMWORD(b);
	d = D;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
	D = r;
	DEC_ICOUNT;

	DASM_SET_IMM(b.w.l);
}

/* $1083 CMPD immediate -**** ~:5 */
void MC6809::cmpd_im()
{
	uint32_t r, d;
	pair32_t b;
	IMMWORD(b);
	d = D;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
	DEC_ICOUNT;

	DASM_SET_IMM(b.w.l);
}

/* $1183 CMPU immediate -**** ~:5 */
void MC6809::cmpu_im()
{
	uint32_t r, d;
	pair32_t b;
	IMMWORD(b);
	d = U;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
	DEC_ICOUNT;

	DASM_SET_IMM(b.w.l);
}

/* $84 ANDA immediate -**0- ~:2 */
void MC6809::anda_im()
{
	uint8_t t;
	IMMBYTE(t);
	A &= t;
	CLR_NZV;
	SET_NZ8(A);

	DASM_SET_IMM(t);
}

/* $85 BITA immediate -**0- ~:2 */
void MC6809::bita_im()
{
	uint8_t t, r;
	IMMBYTE(t);
	r = A & t;
	CLR_NZV;
	SET_NZ8(r);

	DASM_SET_IMM(t);
}

/* $86 LDA immediate -**0- ~:2 */
void MC6809::lda_im()
{
	IMMBYTE(A);
	CLR_NZV;
	SET_NZ8(A);

	DASM_SET_IMM(A);
}

/* is this a legal instruction? */
/* $87 STA immediate -**0- ~:2 */
void MC6809::err_sta_im()
{
	illegal(0x87);
	CLR_NZV;
	SET_NZ8(A);
	IMM8;
	WM(EAD, A);

	DASM_SET_IMM(A);
}

/* $88 EORA immediate -**0- ~:2 */
void MC6809::eora_im()
{
	uint8_t t;
	IMMBYTE(t);
	A ^= t;
	CLR_NZV;
	SET_NZ8(A);

	DASM_SET_IMM(t);
}

/* $89 ADCA immediate ***** ~:2 */
void MC6809::adca_im()
{
	uint16_t t, r;
	IMMBYTE(t);
	r = A + t + (CC & CC_C);
	CLR_HNZVC;
	SET_FLAGS8(A, t, r);
	SET_H(A, t, r);
	A = (uint8_t)r;

	DASM_SET_IMM((uint8_t)t);
}

/* $8A ORA immediate -**0- ~:2 */
void MC6809::ora_im()
{
	uint8_t t;
	IMMBYTE(t);
	A |= t;
	CLR_NZV;
	SET_NZ8(A);

	DASM_SET_IMM(t);
}

/* $8B ADDA immediate ***** ~:2 */
void MC6809::adda_im()
{
	uint16_t t, r;
	IMMBYTE(t);
	r = A + t;
	CLR_HNZVC;
	SET_FLAGS8(A, t, r);
	SET_H(A, t, r);
	A = (uint8_t)r;

	DASM_SET_IMM((uint8_t)t);
}

/* $8C CMPX (CMPY CMPS) immediate -**** ~:4 */
void MC6809::cmpx_im()
{
	uint32_t r, d;
	pair32_t b;
	IMMWORD(b);
	d = X;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
	DEC_ICOUNT;

	DASM_SET_IMM(b.w.l);
}

/* $108C CMPY immediate -**** ~:5 */
void MC6809::cmpy_im()
{
	uint32_t r, d;
	pair32_t b;
	IMMWORD(b);
	d = Y;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
	DEC_ICOUNT;

	DASM_SET_IMM(b.w.l);
}

/* $118C CMPS immediate -**** ~:5 */
void MC6809::cmps_im()
{
	uint32_t r, d;
	pair32_t b;
	IMMWORD(b);
	d = S;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
	DEC_ICOUNT;

	DASM_SET_IMM(b.w.l);
}

/* $8D BSR ----- ~:7 */
void MC6809::bsr()
{
	uint8_t t;
	uint16_t r;
	IMMBYTE(t);
	DEC_ICOUNT;
	r = SIGNED(t);
	LA(r);
	DEC_ICOUNT;
	PUSHWORD(pPC);
	SET_NPC;
	PC += r;

	DASM_SET_REL(t);
}

/* $8E LDX (LDY) immediate -**0- ~:3 */
void MC6809::ldx_im()
{
	IMMWORD(pX);
	CLR_NZV;
	SET_NZ16(X);

	DASM_SET_IMM(x.w.l);
}

/* $108E LDY immediate -**0- ~:4 */
void MC6809::ldy_im()
{
	IMMWORD(pY);
	CLR_NZV;
	SET_NZ16(Y);

	DASM_SET_IMM(y.w.l);
}

/* is this a legal instruction? */
/* $8F STX (STY) immediate -**0- ~:3 */
void MC6809::err_stx_im()
{
	illegal(0x8f);
	CLR_NZV;
	SET_NZ16(X);
	IMM16;
	WM16(EAD, &pX);

	DASM_SET_IMM(x.w.l);
}

/* is this a legal instruction? */
/* $108F STY immediate -**0- ~:4 */
void MC6809::err_sty_im()
{
	illegal(0x108f);
	CLR_NZV;
	SET_NZ16(Y);
	IMM16;
	WM16(EAD, &pY);

	DASM_SET_IMM(y.w.l);
}

/* $90 SUBA direct ?**** ~:4 */
void MC6809::suba_di()
{
	uint16_t t, r;
	DIRBYTE(t);
	r = A - t;
	CLR_NZVC;
	SET_FLAGS8(A, t, r);
	A = (uint8_t)r;

	DASM_SET_DIR1(DP, ea.b.l, (uint8_t)t);
}

/* $91 CMPA direct ?**** ~:4 */
void MC6809::cmpa_di()
{
	uint16_t t, r;
	DIRBYTE(t);
	r = A - t;
	CLR_NZVC;
	SET_FLAGS8(A, t, r);

	DASM_SET_DIR1(DP, ea.b.l, (uint8_t)t);
}

/* $92 SBCA direct ?**** ~:4 */
void MC6809::sbca_di()
{
	uint16_t t, r;
	DIRBYTE(t);
	r = A - t - (CC & CC_C);
	CLR_NZVC;
	SET_FLAGS8(A, t, r);
	A = (uint8_t)r;

	DASM_SET_DIR1(DP, ea.b.l, (uint8_t)t);
}

/* $93 SUBD (CMPD CMPU) direct -**** ~:6 */
void MC6809::subd_di()
{
	uint32_t r, d;
	pair32_t b;
	DIRWORD(b);
	d = D;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
	D = r;
	DEC_ICOUNT;

	DASM_SET_DIR1(DP, ea.b.l, b.w.l);
}

/* $1093 CMPD direct -**** ~:7 */
void MC6809::cmpd_di()
{
	uint32_t r, d;
	pair32_t b;
	DIRWORD(b);
	d = D;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
	DEC_ICOUNT;

	DASM_SET_DIR1(DP, ea.b.l, b.w.l);
}

/* $1193 CMPU direct -**** ~:7 */
void MC6809::cmpu_di()
{
	uint32_t r, d;
	pair32_t b;
	DIRWORD(b);
	d = U;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(U, b.d, r);
	DEC_ICOUNT;

	DASM_SET_DIR1(DP, ea.b.l, b.w.l);
}

/* $94 ANDA direct -**0- ~:4 */
void MC6809::anda_di()
{
	uint8_t t;
	DIRBYTE(t);
	A &= t;
	CLR_NZV;
	SET_NZ8(A);

	DASM_SET_DIR1(DP, ea.b.l, t);
}

/* $95 BITA direct -**0- ~:4 */
void MC6809::bita_di()
{
	uint8_t t, r;
	DIRBYTE(t);
	r = A & t;
	CLR_NZV;
	SET_NZ8(r);

	DASM_SET_DIR1(DP, ea.b.l, t);
}

/* $96 LDA direct -**0- ~:4 */
void MC6809::lda_di()
{
	DIRBYTE(A);
	CLR_NZV;
	SET_NZ8(A);

	DASM_SET_DIR1(DP, ea.b.l, A);
}

/* $97 STA direct -**0- ~:4 */
void MC6809::sta_di()
{
	CLR_NZV;
	SET_NZ8(A);
	DIRECT;
	WM(EAD, A);

	DASM_SET_DIR2(DP, ea.b.l, A, true);
}

/* $98 EORA direct -**0- ~:4 */
void MC6809::eora_di()
{
	uint8_t t;
	DIRBYTE(t);
	A ^= t;
	CLR_NZV;
	SET_NZ8(A);

	DASM_SET_DIR1(DP, ea.b.l, t);
}

/* $99 ADCA direct ***** ~:4 */
void MC6809::adca_di()
{
	uint16_t t, r;
	DIRBYTE(t);
	r = A + t + (CC & CC_C);
	CLR_HNZVC;
	SET_FLAGS8(A, t, r);
	SET_H(A, t, r);
	A = (uint8_t)r;

	DASM_SET_DIR1(DP, ea.b.l, t);
}

/* $9A ORA direct -**0- ~:4 */
void MC6809::ora_di()
{
	uint8_t t;
	DIRBYTE(t);
	A |= t;
	CLR_NZV;
	SET_NZ8(A);

	DASM_SET_DIR1(DP, ea.b.l, t);
}

/* $9B ADDA direct ***** ~:4 */
void MC6809::adda_di()
{
	uint16_t t, r;
	DIRBYTE(t);
	r = A + t;
	CLR_HNZVC;
	SET_FLAGS8(A, t, r);
	SET_H(A, t, r);
	A = (uint8_t)r;

	DASM_SET_DIR1(DP, ea.b.l, (uint8_t)t);
}

/* $9C CMPX (CMPY CMPS) direct -**** ~:6 */
void MC6809::cmpx_di()
{
	uint32_t r, d;
	pair32_t b;
	DIRWORD(b);
	d = X;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
	DEC_ICOUNT;

	DASM_SET_DIR1(DP, ea.b.l, b.w.l);
}

/* $109C CMPY direct -**** ~:7 */
void MC6809::cmpy_di()
{
	uint32_t r, d;
	pair32_t b;
	DIRWORD(b);
	d = Y;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
	DEC_ICOUNT;

	DASM_SET_DIR1(DP, ea.b.l, b.w.l);
}

/* $119C CMPS direct -**** ~:7 */
void MC6809::cmps_di()
{
	uint32_t r, d;
	pair32_t b;
	DIRWORD(b);
	d = S;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
	DEC_ICOUNT;

	DASM_SET_DIR1(DP, ea.b.l, b.w.l);
}

/* $9D JSR direct ----- ~:7 */
void MC6809::jsr_di()
{
	DIRECT;
	LA(EAD);
	DEC_ICOUNT;

	PUSHWORD(pPC);
	SET_NPC;
	PCD = EAD;

	DASM_SET_JMP_DIR(DP, pc.b.l);
}

/* $9E LDX (LDY) direct -**0- ~:5 */
void MC6809::ldx_di()
{
	DIRWORD(pX);
	CLR_NZV;
	SET_NZ16(X);

	DASM_SET_DIR1(DP, ea.b.l, X);
}

/* $109E LDY direct -**0- ~:6 */
void MC6809::ldy_di()
{
	DIRWORD(pY);
	CLR_NZV;
	SET_NZ16(Y);

	DASM_SET_DIR1(DP, ea.b.l, Y);
}

/* $9F STX (STY) direct -**0- ~:5 */
void MC6809::stx_di()
{
	CLR_NZV;
	SET_NZ16(X);
	DIRECT;
	WM16(EAD, &pX);

	DASM_SET_DIR2(DP, ea.b.l, X, true);
}

/* $109F STY direct -**0- ~:6 */
void MC6809::sty_di()
{
	CLR_NZV;
	SET_NZ16(Y);
	DIRECT;
	WM16(EAD, &pY);

	DASM_SET_DIR2(DP, ea.b.l, Y, true);
}

/* $a0 SUBA indexed ?**** ~:3+ */
void MC6809::suba_ix()
{
	uint16_t t, r;
	fetch_effective_address();
	t = RM(EAD);
	r = A - t;
	CLR_NZVC;
	SET_FLAGS8(A, t, r);
	A = (uint8_t)r;

	DASM_SET_MEM1(ea.w.l, (uint8_t)t);
}

/* $a1 CMPA indexed ?**** ~:3+ */
void MC6809::cmpa_ix()
{
	uint16_t t, r;
	fetch_effective_address();
	t = RM(EAD);
	r = A - t;
	CLR_NZVC;
	SET_FLAGS8(A, t, r);

	DASM_SET_MEM1(ea.w.l, (uint8_t)t);
}

/* $a2 SBCA indexed ?**** ~:3+ */
void MC6809::sbca_ix()
{
	uint16_t t, r;
	fetch_effective_address();
	t = RM(EAD);
	r = A - t - (CC & CC_C);
	CLR_NZVC;
	SET_FLAGS8(A, t, r);
	A = (uint8_t)r;

	DASM_SET_MEM1(ea.w.l, (uint8_t)t);
}

/* $a3 SUBD (CMPD CMPU) indexed -**** ~:5+ */
void MC6809::subd_ix()
{
	uint32_t r, d;
	pair32_t b;
	fetch_effective_address();
	b.d=RM16(EAD);
	d = D;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
	D = r;
	DEC_ICOUNT;

	DASM_SET_MEM1(ea.w.l, b.w.l);
}

/* $10a3 CMPD indexed -**** ~:6+ */
void MC6809::cmpd_ix()
{
	uint32_t r, d;
	pair32_t b;
	fetch_effective_address();
	b.d=RM16(EAD);
	d = D;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
	DEC_ICOUNT;

	DASM_SET_MEM1(ea.w.l, b.w.l);
}

/* $11a3 CMPU indexed -**** ~:6+ */
void MC6809::cmpu_ix()
{
	uint32_t r;
	pair32_t b;
	fetch_effective_address();
	b.d=RM16(EAD);
	r = U - b.d;
	CLR_NZVC;
	SET_FLAGS16(U, b.d, r);
	DEC_ICOUNT;

	DASM_SET_MEM1(ea.w.l, b.w.l);
}

/* $a4 ANDA indexed -**0- ~:3+ */
void MC6809::anda_ix()
{
	uint8_t t;
	fetch_effective_address();
	t = RM(EAD);
	A &= t;
	CLR_NZV;
	SET_NZ8(A);

	DASM_SET_MEM1(ea.w.l, t);
}

/* $a5 BITA indexed -**0- ~:3+ */
void MC6809::bita_ix()
{
	uint8_t r, t;
	fetch_effective_address();
	t = RM(EAD);
	r = A & t;
	CLR_NZV;
	SET_NZ8(r);

	DASM_SET_MEM1(ea.w.l, t);
}

/* $a6 LDA indexed -**0- ~:3+ */
void MC6809::lda_ix()
{
	fetch_effective_address();
	A = RM(EAD);
	CLR_NZV;
	SET_NZ8(A);

	DASM_SET_MEM1(ea.w.l, A);
}

/* $a7 STA indexed -**0- ~:3+ */
void MC6809::sta_ix()
{
	fetch_effective_address();
	CLR_NZV;
	SET_NZ8(A);
	WM(EAD, A);

	DASM_SET_MEM2(ea.w.l, A, true);
}

/* $a8 EORA indexed -**0- ~:3+ */
void MC6809::eora_ix()
{
	uint8_t t;
	fetch_effective_address();
	t = RM(EAD);
	A ^= t;
	CLR_NZV;
	SET_NZ8(A);

	DASM_SET_MEM1(ea.w.l, t);
}

/* $a9 ADCA indexed ***** ~:3+ */
void MC6809::adca_ix()
{
	uint16_t t, r;
	fetch_effective_address();
	t = RM(EAD);
	r = A + t + (CC & CC_C);
	CLR_HNZVC;
	SET_FLAGS8(A, t, r);
	SET_H(A, t, r);
	A = (uint8_t)r;

	DASM_SET_MEM1(ea.w.l, (uint8_t)t);
}

/* $aA ORA indexed -**0- ~:3+ */
void MC6809::ora_ix()
{
	uint8_t t;
	fetch_effective_address();
	t = RM(EAD);
	A |= t;
	CLR_NZV;
	SET_NZ8(A);

	DASM_SET_MEM1(ea.w.l, t);
}

/* $aB ADDA indexed ***** ~:3+ */
void MC6809::adda_ix()
{
	uint16_t t, r;
	fetch_effective_address();
	t = RM(EAD);
	r = A + t;
	CLR_HNZVC;
	SET_FLAGS8(A, t, r);
	SET_H(A, t, r);
	A = (uint8_t)r;

	DASM_SET_MEM1(ea.w.l, (uint8_t)t);
}

/* $aC CMPX (CMPY CMPS) indexed -**** ~:5+ */
void MC6809::cmpx_ix()
{
	uint32_t r, d;
	pair32_t b;
	fetch_effective_address();
	b.d=RM16(EAD);
	d = X;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
	DEC_ICOUNT;

	DASM_SET_MEM1(ea.w.l, b.w.l);
}

/* $10aC CMPY indexed -**** ~:6+ */
void MC6809::cmpy_ix()
{
	uint32_t r, d;
	pair32_t b;
	fetch_effective_address();
	b.d=RM16(EAD);
	d = Y;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
	DEC_ICOUNT;

	DASM_SET_MEM1(ea.w.l, b.w.l);
}

/* $11aC CMPS indexed -**** ~:6+ */
void MC6809::cmps_ix()
{
	uint32_t r, d;
	pair32_t b;
	fetch_effective_address();
	b.d=RM16(EAD);
	d = S;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
	DEC_ICOUNT;

	DASM_SET_MEM1(ea.w.l, b.w.l);
}

/* $aD JSR indexed ----- ~:6+ */
void MC6809::jsr_ix()
{
	fetch_effective_address();
	LA(EAD);
	DEC_ICOUNT;
	PUSHWORD(pPC);
	SET_NPC;
	PCD = EAD;
}

/* $aE LDX (LDY) indexed -**0- ~:4+ */
void MC6809::ldx_ix()
{
	fetch_effective_address();
	X = RM16(EAD);
	CLR_NZV;
	SET_NZ16(X);

	DASM_SET_MEM1(ea.w.l, X);
}

/* $10aE LDY indexed -**0- ~:5+ */
void MC6809::ldy_ix()
{
	fetch_effective_address();
	Y = RM16(EAD);
	CLR_NZV;
	SET_NZ16(Y);

	DASM_SET_MEM1(ea.w.l, Y);
}

/* $aF STX (STY) indexed -**0- ~:4+ */
void MC6809::stx_ix()
{
	fetch_effective_address();
	CLR_NZV;
	SET_NZ16(X);
	WM16(EAD, &pX);

	DASM_SET_MEM2(ea.w.l, X, true);
}

/* $10aF STY indexed -**0- ~:5+ */
void MC6809::sty_ix()
{
	fetch_effective_address();
	CLR_NZV;
	SET_NZ16(Y);
	WM16(EAD, &pY);

	DASM_SET_MEM2(ea.w.l, Y, true);
}

/* $b0 SUBA extended ?**** ~:5 */
void MC6809::suba_ex()
{
	uint16_t t, r;
	EXTBYTE(t);
	r = A - t;
	CLR_NZVC;
	SET_FLAGS8(A, t, r);
	A = (uint8_t)r;

	DASM_SET_EXT1(ea.w.l, (uint8_t)t);
}

/* $b1 CMPA extended ?**** ~:5 */
void MC6809::cmpa_ex()
{
	uint16_t t, r;
	EXTBYTE(t);
	r = A - t;
	CLR_NZVC;
	SET_FLAGS8(A, t, r);

	DASM_SET_EXT1(ea.w.l, (uint8_t)t);
}

/* $b2 SBCA extended ?**** ~:5 */
void MC6809::sbca_ex()
{
	uint16_t t, r;
	EXTBYTE(t);
	r = A - t - (CC & CC_C);
	CLR_NZVC;
	SET_FLAGS8(A, t, r);
	A = (uint8_t)r;

	DASM_SET_EXT1(ea.w.l, (uint8_t)t);
}

/* $b3 SUBD (CMPD CMPU) extended -**** ~:7 */
void MC6809::subd_ex()
{
	uint32_t r, d;
	pair32_t b;
	EXTWORD(b);
	d = D;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
	D = r;
	DEC_ICOUNT;

	DASM_SET_EXT1(ea.w.l, b.w.l);
}

/* $10b3 CMPD extended -**** ~:8 */
void MC6809::cmpd_ex()
{
	uint32_t r, d;
	pair32_t b;
	EXTWORD(b);
	d = D;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
	DEC_ICOUNT;

	DASM_SET_EXT1(ea.w.l, b.w.l);
}

/* $11b3 CMPU extended -**** ~:8 */
void MC6809::cmpu_ex()
{
	uint32_t r, d;
	pair32_t b;
	EXTWORD(b);
	d = U;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
	DEC_ICOUNT;

	DASM_SET_EXT1(ea.w.l, b.w.l);
}

/* $b4 ANDA extended -**0- ~:5 */
void MC6809::anda_ex()
{
	uint8_t t;
	EXTBYTE(t);
	A &= t;
	CLR_NZV;
	SET_NZ8(A);

	DASM_SET_EXT1(ea.w.l, t);
}

/* $b5 BITA extended -**0- ~:5 */
void MC6809::bita_ex()
{
	uint8_t t, r;
	EXTBYTE(t);
	r = A & t;
	CLR_NZV; SET_NZ8(r);

	DASM_SET_EXT1(ea.w.l, t);
}

/* $b6 LDA extended -**0- ~:5 */
void MC6809::lda_ex()
{
	EXTBYTE(A);
	CLR_NZV;
	SET_NZ8(A);

	DASM_SET_EXT1(ea.w.l, A);
}

/* $b7 STA extended -**0- ~:5 */
void MC6809::sta_ex()
{
	CLR_NZV;
	SET_NZ8(A);
	EXTENDED;
	WM(EAD, A);

	DASM_SET_EXT2(ea.w.l, A, true);
}

/* $b8 EORA extended -**0- ~:5 */
void MC6809::eora_ex()
{
	uint8_t t;
	EXTBYTE(t);
	A ^= t;
	CLR_NZV;
	SET_NZ8(A);

	DASM_SET_EXT1(ea.w.l, t);
}

/* $b9 ADCA extended ***** ~:5 */
void MC6809::adca_ex()
{
	uint16_t t, r;
	EXTBYTE(t);
	r = A + t + (CC & CC_C);
	CLR_HNZVC;
	SET_FLAGS8(A, t, r);
	SET_H(A, t, r);
	A = (uint8_t)r;

	DASM_SET_EXT1(ea.w.l, (uint8_t)t);
}

/* $bA ORA extended -**0- ~:5 */
void MC6809::ora_ex()
{
	uint8_t t;
	EXTBYTE(t);
	A |= t;
	CLR_NZV;
	SET_NZ8(A);

	DASM_SET_EXT1(ea.w.l, t);
}

/* $bB ADDA extended ***** ~:5 */
void MC6809::adda_ex()
{
	uint16_t t, r;
	EXTBYTE(t);
	r = A + t;
	CLR_HNZVC;
	SET_FLAGS8(A, t, r);
	SET_H(A, t, r);
	A = (uint8_t)r;

	DASM_SET_EXT1(ea.w.l, (uint8_t)t);
}

/* $bC CMPX (CMPY CMPS) extended -**** ~:7 */
void MC6809::cmpx_ex()
{
	uint32_t r, d;
	pair32_t b;
	EXTWORD(b);
	d = X;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
	DEC_ICOUNT;

	DASM_SET_EXT1(ea.w.l, b.w.l);
}

/* $10bC CMPY extended -**** ~:8 */
void MC6809::cmpy_ex()
{
	uint32_t r, d;
	pair32_t b;
	EXTWORD(b);
	d = Y;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
	DEC_ICOUNT;

	DASM_SET_EXT1(ea.w.l, b.w.l);
}

/* $11bC CMPS extended -**** ~:8 */
void MC6809::cmps_ex()
{
	uint32_t r, d;
	pair32_t b;
	EXTWORD(b);
	d = S;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
	DEC_ICOUNT;

	DASM_SET_EXT1(ea.w.l, b.w.l);
}

/* $bD JSR extended ----- ~:8 */
void MC6809::jsr_ex()
{
	EXTENDED;
	LA(EAD);
	DEC_ICOUNT;
	PUSHWORD(pPC);
	SET_NPC;
	PCD = EAD;

	DASM_SET_JMP(ea.w.l);
}

/* $bE LDX (LDY) extended -**0- ~:6 */
void MC6809::ldx_ex()
{
	EXTWORD(pX);
	CLR_NZV;
	SET_NZ16(X);

	DASM_SET_EXT1(ea.w.l, X);
}

/* $10bE LDY extended -**0- ~:7 */
void MC6809::ldy_ex()
{
	EXTWORD(pY);
	CLR_NZV;
	SET_NZ16(Y);

	DASM_SET_EXT1(ea.w.l, Y);
}

/* $bF STX (STY) extended -**0- ~:6 */
void MC6809::stx_ex()
{
	CLR_NZV;
	SET_NZ16(X);
	EXTENDED;
	WM16(EAD, &pX);

	DASM_SET_EXT2(ea.w.l, X, true);
}

/* $10bF STY extended -**0- ~:7 */
void MC6809::sty_ex()
{
	CLR_NZV;
	SET_NZ16(Y);
	EXTENDED;
	WM16(EAD, &pY);

	DASM_SET_EXT2(ea.w.l, Y, true);
}

/* $c0 SUBB immediate ?**** ~:2 */
void MC6809::subb_im()
{
	uint16_t t, r;
	IMMBYTE(t);
	r = B - t;
	CLR_NZVC;
	SET_FLAGS8(B, t, r);
	B = (uint8_t)r;

	DASM_SET_IMM((uint8_t)t);
}

/* $c1 CMPB immediate ?**** ~:2 */
void MC6809::cmpb_im()
{
	uint16_t t, r;
	IMMBYTE(t);
	r = B - t;
	CLR_NZVC; SET_FLAGS8(B, t, r);

	DASM_SET_IMM((uint8_t)t);
}

/* $c2 SBCB immediate ?**** ~:2 */
void MC6809::sbcb_im()
{
	uint16_t t, r;
	IMMBYTE(t);
	r = B - t - (CC & CC_C);
	CLR_NZVC;
	SET_FLAGS8(B, t, r);
	B = (uint8_t)r;

	DASM_SET_IMM((uint8_t)t);
}

/* $c3 ADDD immediate -**** ~:4 */
void MC6809::addd_im()
{
	uint32_t r, d;
	pair32_t b;
	IMMWORD(b);
	d = D;
	r = d + b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
	D = r;
	DEC_ICOUNT;

	DASM_SET_IMM(b.w.l);
}

/* $c4 ANDB immediate -**0- ~:2 */
void MC6809::andb_im()
{
	uint8_t t;
	IMMBYTE(t);
	B &= t;
	CLR_NZV;
	SET_NZ8(B);

	DASM_SET_IMM(t);
}

/* $c5 BITB immediate -**0- ~:2 */
void MC6809::bitb_im()
{
	uint8_t t, r;
	IMMBYTE(t);
	r = B & t;
	CLR_NZV;
	SET_NZ8(r);

	DASM_SET_IMM(t);
}

/* $c6 LDB immediate -**0- ~:2 */
void MC6809::ldb_im()
{
	IMMBYTE(B);
	CLR_NZV;
	SET_NZ8(B);

	DASM_SET_IMM(B);
}

/* is this a legal instruction? */
/* $c7 STB immediate -**0- ~:2 */
void MC6809::err_stb_im()
{
	illegal(0xc7);
	CLR_NZV;
	SET_NZ8(B);
	IMM8;
	WM(EAD, B);

	DASM_SET_IMM(B);
}

/* $c8 EORB immediate -**0- ~:2 */
void MC6809::eorb_im()
{
	uint8_t t;
	IMMBYTE(t);
	B ^= t;
	CLR_NZV;
	SET_NZ8(B);

	DASM_SET_IMM(t);
}

/* $c9 ADCB immediate ***** ~:2 */
void MC6809::adcb_im()
{
	uint16_t t, r;
	IMMBYTE(t);
	r = B + t + (CC & CC_C);
	CLR_HNZVC;
	SET_FLAGS8(B, t, r);
	SET_H(B, t, r);
	B = (uint8_t)r;

	DASM_SET_IMM((uint8_t)t);
}

/* $cA ORB immediate -**0- ~:2 */
void MC6809::orb_im()
{
	uint8_t t;
	IMMBYTE(t);
	B |= t;
	CLR_NZV;
	SET_NZ8(B);

	DASM_SET_IMM(t);
}

/* $cB ADDB immediate ***** ~:2 */
void MC6809::addb_im()
{
	uint16_t t, r;
	IMMBYTE(t);
	r = B + t;
	CLR_HNZVC;
	SET_FLAGS8(B, t, r);
	SET_H(B, t, r);
	B = (uint8_t)r;

	DASM_SET_IMM((uint8_t)t);
}

/* $cC LDD immediate -**0- ~:3 */
void MC6809::ldd_im()
{
	IMMWORD(pD);
	CLR_NZV;
	SET_NZ16(D);

	DASM_SET_IMM(D);
}

/* is this a legal instruction? */
/* $cD STD immediate -**0- ~:3 */
#if 0
void MC6809::err_std_im()
{
	illegal(0xcd);
	CLR_NZV;
	SET_NZ16(D);
	IMM16;
	WM16(EAD, &pD);

	DASM_SET_IMM(D);
}
#endif

/* $cE LDU (LDS) immediate -**0- ~:3 */
void MC6809::ldu_im()
{
	IMMWORD(pU);
	CLR_NZV;
	SET_NZ16(U);

	DASM_SET_IMM(U);
}

/* $10cE LDS immediate -**0- ~:4 */
void MC6809::lds_im()
{
	IMMWORD(pS);
	CLR_NZV;
	SET_NZ16(S);
	int_state |= MC6809_LDS;

	DASM_SET_IMM(S);
}

/* is this a legal instruction? */
/* $cF STU (STS) immediate -**0- ~:3 */
void MC6809::err_stu_im()
{
	illegal(0xcf);
	CLR_NZV;
	SET_NZ16(U);
	IMM16;
	WM16(EAD, &pU);

	DASM_SET_IMM(U);
}

/* is this a legal instruction? */
/* $10cF STS immediate -**0- ~:4 */
void MC6809::err_sts_im()
{
	illegal(0x10cf);
	CLR_NZV;
	SET_NZ16(S);
	IMM16;
	WM16(EAD, &pS);

	DASM_SET_IMM(S);
}

/* $d0 SUBB direct ?**** ~:4 */
void MC6809::subb_di()
{
	uint16_t t, r;
	DIRBYTE(t);
	r = B - t;
	CLR_NZVC;
	SET_FLAGS8(B, t, r);
	B = (uint8_t)r;

	DASM_SET_DIR1(DP, ea.b.l, (uint8_t)t);
}

/* $d1 CMPB direct ?**** ~:4 */
void MC6809::cmpb_di()
{
	uint16_t t, r;
	DIRBYTE(t);
	r = B - t;
	CLR_NZVC;
	SET_FLAGS8(B, t, r);

	DASM_SET_DIR1(DP, ea.b.l, (uint8_t)t);
}

/* $d2 SBCB direct ?**** ~:4 */
void MC6809::sbcb_di()
{
	uint16_t t, r;
	DIRBYTE(t);
	r = B - t - (CC & CC_C);
	CLR_NZVC;
	SET_FLAGS8(B, t, r);
	B = (uint8_t)r;

	DASM_SET_DIR1(DP, ea.b.l, (uint8_t)t);
}

/* $d3 ADDD direct -**** ~:6 */
void MC6809::addd_di()
{
	uint32_t r, d;
	pair32_t b;
	DIRWORD(b);
	d = D;
	r = d + b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
	D = r;
	DEC_ICOUNT;

	DASM_SET_DIR1(DP, ea.b.l, b.w.l);
}

/* $d4 ANDB direct -**0- ~:4 */
void MC6809::andb_di()
{
	uint8_t t;
	DIRBYTE(t);
	B &= t;
	CLR_NZV;
	SET_NZ8(B);

	DASM_SET_DIR1(DP, ea.b.l, t);
}

/* $d5 BITB direct -**0- ~:4 */
void MC6809::bitb_di()
{
	uint8_t t, r;
	DIRBYTE(t);
	r = B & t;
	CLR_NZV;
	SET_NZ8(r);

	DASM_SET_DIR1(DP, ea.b.l, t);
}

/* $d6 LDB direct -**0- ~:4 */
void MC6809::ldb_di()
{
	DIRBYTE(B);
	CLR_NZV;
	SET_NZ8(B);

	DASM_SET_DIR1(DP, ea.b.l, B);
}

/* $d7 STB direct -**0- ~:4 */
void MC6809::stb_di()
{
	CLR_NZV;
	SET_NZ8(B);
	DIRECT;
	WM(EAD, B);

	DASM_SET_DIR2(DP, ea.b.l, B, true);
}

/* $d8 EORB direct -**0- ~:4 */
void MC6809::eorb_di()
{
	uint8_t t;
	DIRBYTE(t);
	B ^= t;
	CLR_NZV;
	SET_NZ8(B);

	DASM_SET_DIR1(DP, ea.b.l, t);
}

/* $d9 ADCB direct ***** ~:4 */
void MC6809::adcb_di()
{
	uint16_t t, r;
	DIRBYTE(t);
	r = B + t + (CC & CC_C);
	CLR_HNZVC;
	SET_FLAGS8(B, t, r);
	SET_H(B, t, r);
	B = (uint8_t)r;

	DASM_SET_DIR1(DP, ea.b.l, (uint8_t)t);
}

/* $dA ORB direct -**0- ~:4 */
void MC6809::orb_di()
{
	uint8_t t;
	DIRBYTE(t);
	B |= t;
	CLR_NZV;
	SET_NZ8(B);

	DASM_SET_DIR1(DP, ea.b.l, t);
}

/* $dB ADDB direct ***** ~:4 */
void MC6809::addb_di()
{
	uint16_t t, r;
	DIRBYTE(t);
	r = B + t;
	CLR_HNZVC;
	SET_FLAGS8(B, t, r);
	SET_H(B, t, r);
	B = (uint8_t)r;

	DASM_SET_DIR1(DP, ea.b.l, (uint8_t)t);
}

/* $dC LDD direct -**0- ~:5 */
void MC6809::ldd_di()
{
	DIRWORD(pD);
	CLR_NZV;
	SET_NZ16(D);

	DASM_SET_DIR1(DP, ea.b.l, D);
}

/* $dD STD direct -**0- ~:5 */
void MC6809::std_di()
{
	CLR_NZV;
	SET_NZ16(D);
	DIRECT;
	WM16(EAD, &pD);

	DASM_SET_DIR2(DP, ea.b.l, D, true);
}

/* $dE LDU (LDS) direct -**0- ~:5 */
void MC6809::ldu_di()
{
	DIRWORD(pU);
	CLR_NZV;
	SET_NZ16(U);

	DASM_SET_DIR1(DP, ea.b.l, U);
}

/* $10dE LDS direct -**0- ~:6 */
void MC6809::lds_di()
{
	DIRWORD(pS);
	CLR_NZV;
	SET_NZ16(S);
	int_state |= MC6809_LDS;

	DASM_SET_DIR1(DP, ea.b.l, S);
}

/* $dF STU (STS) direct -**0- ~:5 */
void MC6809::stu_di()
{
	CLR_NZV;
	SET_NZ16(U);
	DIRECT;
	WM16(EAD, &pU);

	DASM_SET_DIR2(DP, ea.b.l, U, true);
}

/* $10dF STS direct -**0- ~:6 */
void MC6809::sts_di()
{
	CLR_NZV;
	SET_NZ16(S);
	DIRECT;
	WM16(EAD, &pS);

	DASM_SET_DIR2(DP, ea.b.l, S, true);
}

/* $e0 SUBB indexed ?**** ~:3+ */
void MC6809::subb_ix()
{
	uint16_t t, r;
	fetch_effective_address();
	t = RM(EAD);
	r = B - t;
	CLR_NZVC;
	SET_FLAGS8(B, t, r);
	B = (uint8_t)r;

	DASM_SET_MEM1(ea.w.l, (uint8_t)t);
}

/* $e1 CMPB indexed ?**** ~:3+ */
void MC6809::cmpb_ix()
{
	uint16_t t, r;
	fetch_effective_address();
	t = RM(EAD);
	r = B - t;
	CLR_NZVC;
	SET_FLAGS8(B, t, r);

	DASM_SET_MEM1(ea.w.l, (uint8_t)t);
}

/* $e2 SBCB indexed ?**** ~:3+ */
void MC6809::sbcb_ix()
{
	uint16_t t, r;
	fetch_effective_address();
	t = RM(EAD);
	r = B - t - (CC & CC_C);
	CLR_NZVC;
	SET_FLAGS8(B, t, r);
	B = (uint8_t)r;

	DASM_SET_MEM1(ea.w.l, (uint8_t)t);
}

/* $e3 ADDD indexed -**** ~:5+ */
void MC6809::addd_ix()
{
	uint32_t r, d;
	pair32_t b;
	fetch_effective_address();
	b.d=RM16(EAD);
	d = D;
	r = d + b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
	D = r;
	DEC_ICOUNT;

	DASM_SET_MEM1(ea.w.l, b.w.l);
}

/* $e4 ANDB indexed -**0- ~:3+ */
void MC6809::andb_ix()
{
	uint8_t t;
	fetch_effective_address();
	t = RM(EAD);
	B &= t;
	CLR_NZV;
	SET_NZ8(B);

	DASM_SET_MEM1(ea.w.l, t);
}

/* $e5 BITB indexed -**0- ~:3+ */
void MC6809::bitb_ix()
{
	uint8_t r, t;
	fetch_effective_address();
	t = RM(EAD);
	r = B & t;
	CLR_NZV;
	SET_NZ8(r);

	DASM_SET_MEM1(ea.w.l, t);
}

/* $e6 LDB indexed -**0- ~:3+ */
void MC6809::ldb_ix()
{
	fetch_effective_address();
	B = RM(EAD);
	CLR_NZV;
	SET_NZ8(B);

	DASM_SET_MEM1(ea.w.l, B);
}

/* $e7 STB indexed -**0- ~:3+ */
void MC6809::stb_ix()
{
	fetch_effective_address();
	CLR_NZV;
	SET_NZ8(B);
	WM(EAD, B);

	DASM_SET_MEM2(ea.w.l, B, true);
}

/* $e8 EORB indexed -**0- ~:3+ */
void MC6809::eorb_ix()
{
	uint8_t t;
	fetch_effective_address();
	t = RM(EAD);
	B ^= t;
	CLR_NZV;
	SET_NZ8(B);

	DASM_SET_MEM1(ea.w.l, t);
}

/* $e9 ADCB indexed ***** ~:3+ */
void MC6809::adcb_ix()
{
	uint16_t t, r;
	fetch_effective_address();
	t = RM(EAD);
	r = B + t + (CC & CC_C);
	CLR_HNZVC;
	SET_FLAGS8(B, t, r);
	SET_H(B, t, r);
	B = (uint8_t)r;

	DASM_SET_MEM1(ea.w.l, (uint8_t)t);
}

/* $eA ORB indexed -**0- ~:3+ */
void MC6809::orb_ix()
{
	uint8_t t;
	fetch_effective_address();
	t = RM(EAD);
	B |= t;
	CLR_NZV;
	SET_NZ8(B);

	DASM_SET_MEM1(ea.w.l, t);
}

/* $eB ADDB indexed ***** ~:3+ */
void MC6809::addb_ix()
{
	uint16_t t, r;
	fetch_effective_address();
	t = RM(EAD);
	r = B + t;
	CLR_HNZVC;
	SET_FLAGS8(B, t, r);
	SET_H(B, t, r);
	B = (uint8_t)r;

	DASM_SET_MEM1(ea.w.l, (uint8_t)t);
}

/* $eC LDD indexed -**0- ~:4+ */
void MC6809::ldd_ix()
{
	fetch_effective_address();
	D = RM16(EAD);
	CLR_NZV; SET_NZ16(D);

	DASM_SET_MEM1(ea.w.l, D);
}

/* $eD STD indexed -**0- ~:4+ */
void MC6809::std_ix()
{
	fetch_effective_address();
	CLR_NZV;
	SET_NZ16(D);
	WM16(EAD, &pD);

	DASM_SET_MEM2(ea.w.l, D, true);
}

/* $eE LDU (LDS) indexed -**0- ~:4+ */
void MC6809::ldu_ix()
{
	fetch_effective_address();
	U = RM16(EAD);
	CLR_NZV;
	SET_NZ16(U);

	DASM_SET_MEM1(ea.w.l, U);
}

/* $10eE LDS indexed -**0- ~:5+ */
void MC6809::lds_ix()
{
	fetch_effective_address();
	S = RM16(EAD);
	CLR_NZV;
	SET_NZ16(S);
	int_state |= MC6809_LDS;

	DASM_SET_MEM1(ea.w.l, S);
}

/* $eF STU (STS) indexed -**0- ~:4+ */
void MC6809::stu_ix()
{
	fetch_effective_address();
	CLR_NZV;
	SET_NZ16(U);
	WM16(EAD, &pU);

	DASM_SET_MEM2(ea.w.l, U, true);
}

/* $10eF STS indexed -**0- ~:5+ */
void MC6809::sts_ix()
{
	fetch_effective_address();
	CLR_NZV;
	SET_NZ16(S);
	WM16(EAD, &pS);

	DASM_SET_MEM2(ea.w.l, S, true);
}

/* $f0 SUBB extended ?**** ~:5 */
void MC6809::subb_ex()
{
	uint16_t t, r;
	EXTBYTE(t);
	r = B - t;
	CLR_NZVC;
	SET_FLAGS8(B, t, r);
	B = (uint8_t)r;

	DASM_SET_EXT1(ea.w.l, (uint8_t)t);
}

/* $f1 CMPB extended ?**** ~:5 */
void MC6809::cmpb_ex()
{
	uint16_t t, r;
	EXTBYTE(t);
	r = B - t;
	CLR_NZVC;
	SET_FLAGS8(B, t, r);

	DASM_SET_EXT1(ea.w.l, (uint8_t)t);
}

/* $f2 SBCB extended ?**** ~:5 */
void MC6809::sbcb_ex()
{
	uint16_t t, r;
	EXTBYTE(t);
	r = B - t - (CC & CC_C);
	CLR_NZVC;
	SET_FLAGS8(B, t, r);
	B = (uint8_t)r;

	DASM_SET_EXT1(ea.w.l, (uint8_t)t);
}

/* $f3 ADDD extended -**** ~:7 */
void MC6809::addd_ex()
{
	uint32_t r, d;
	pair32_t b;
	EXTWORD(b);
	d = D;
	r = d + b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
	D = r;
	DEC_ICOUNT;

	DASM_SET_EXT1(ea.w.l, b.w.l);
}

/* $f4 ANDB extended -**0- ~:5 */
void MC6809::andb_ex()
{
	uint8_t t;
	EXTBYTE(t);
	B &= t;
	CLR_NZV;
	SET_NZ8(B);

	DASM_SET_EXT1(ea.w.l, t);
}

/* $f5 BITB extended -**0- ~:5 */
void MC6809::bitb_ex()
{
	uint8_t t, r;
	EXTBYTE(t);
	r = B & t;
	CLR_NZV;
	SET_NZ8(r);

	DASM_SET_EXT1(ea.w.l, t);
}

/* $f6 LDB extended -**0- ~:5 */
void MC6809::ldb_ex()
{
	EXTBYTE(B);
	CLR_NZV;
	SET_NZ8(B);

	DASM_SET_EXT1(ea.w.l, B);
}

/* $f7 STB extended -**0- ~:5 */
void MC6809::stb_ex()
{
	CLR_NZV;
	SET_NZ8(B);
	EXTENDED;
	WM(EAD, B);

	DASM_SET_EXT2(ea.w.l, B, true);
}

/* $f8 EORB extended -**0- ~:5 */
void MC6809::eorb_ex()
{
	uint8_t t;
	EXTBYTE(t);
	B ^= t;
	CLR_NZV;
	SET_NZ8(B);

	DASM_SET_EXT1(ea.w.l, t);
}

/* $f9 ADCB extended ***** ~:5 */
void MC6809::adcb_ex()
{
	uint16_t t, r;
	EXTBYTE(t);
	r = B + t + (CC & CC_C);
	CLR_HNZVC;
	SET_FLAGS8(B, t, r);
	SET_H(B, t, r);
	B = (uint8_t)r;

	DASM_SET_EXT1(ea.w.l, (uint8_t)t);
}

/* $fA ORB extended -**0- ~:5 */
void MC6809::orb_ex()
{
	uint8_t t;
	EXTBYTE(t);
	B |= t;
	CLR_NZV;
	SET_NZ8(B);

	DASM_SET_EXT1(ea.w.l, t);
}

/* $fB ADDB extended ***** ~:5 */
void MC6809::addb_ex()
{
	uint16_t t, r;
	EXTBYTE(t);
	r = B + t;
	CLR_HNZVC;
	SET_FLAGS8(B, t, r);
	SET_H(B, t, r);
	B = (uint8_t)r;

	DASM_SET_EXT1(ea.w.l, (uint8_t)t);
}

/* $fC LDD extended -**0- ~:6 */
void MC6809::ldd_ex()
{
	EXTWORD(pD);
	CLR_NZV;
	SET_NZ16(D);

	DASM_SET_EXT1(ea.w.l, D);
}

/* $fD STD extended -**0- ~:6 */
void MC6809::std_ex()
{
	CLR_NZV;
	SET_NZ16(D);
	EXTENDED;
	WM16(EAD, &pD);

	DASM_SET_EXT2(ea.w.l, D, true);
}

/* $fE LDU (LDS) extended -**0- ~:6 */
void MC6809::ldu_ex()
{
	EXTWORD(pU);
	CLR_NZV;
	SET_NZ16(U);

	DASM_SET_EXT1(ea.w.l, U);
}

/* $10fE LDS extended -**0- ~:7 */
void MC6809::lds_ex()
{
	EXTWORD(pS);
	CLR_NZV;
	SET_NZ16(S);
	int_state |= MC6809_LDS;

	DASM_SET_EXT1(ea.w.l, S);
}

/* $fF STU (STS) extended -**0- ~:6 */
void MC6809::stu_ex()
{
	CLR_NZV;
	SET_NZ16(U);
	EXTENDED;
	WM16(EAD, &pU);

	DASM_SET_EXT2(ea.w.l, U, true);
}

/* $10fF STS extended -**0- ~:7 */
void MC6809::sts_ex()
{
	CLR_NZV;
	SET_NZ16(S);
	EXTENDED;
	WM16(EAD, &pS);

	DASM_SET_EXT2(ea.w.l, S, true);
}

/* $10xx opcodes */
void MC6809::pref10()
{
	uint8_t ireg2 = ROP(PCD);
	PC++;

	DASM_SET_CODE2(ireg2);

	switch(ireg2) {
	case 0x21: lbrn(); SUBALL_ICOUNT(5); break;
	case 0x22: lbhi(); SUBALL_ICOUNT(5); break;
	case 0x23: lbls(); SUBALL_ICOUNT(5); break;
	case 0x24: lbcc(); SUBALL_ICOUNT(5); break;
	case 0x25: lbcs(); SUBALL_ICOUNT(5); break;
	case 0x26: lbne(); SUBALL_ICOUNT(5); break;
	case 0x27: lbeq(); SUBALL_ICOUNT(5); break;
	case 0x28: lbvc(); SUBALL_ICOUNT(5); break;
	case 0x29: lbvs(); SUBALL_ICOUNT(5); break;
	case 0x2a: lbpl(); SUBALL_ICOUNT(5); break;
	case 0x2b: lbmi(); SUBALL_ICOUNT(5); break;
	case 0x2c: lbge(); SUBALL_ICOUNT(5); break;
	case 0x2d: lblt(); SUBALL_ICOUNT(5); break;
	case 0x2e: lbgt(); SUBALL_ICOUNT(5); break;
	case 0x2f: lble(); SUBALL_ICOUNT(5); break;
	case 0x3f: swi2(); SUBALL_ICOUNT(20); break;
	case 0x83: cmpd_im(); SUBALL_ICOUNT(5); break;
	case 0x8c: cmpy_im(); SUBALL_ICOUNT(5); break;
	case 0x8e: ldy_im(); SUBALL_ICOUNT(4); break;
	case 0x8f: err_sty_im(); SUBALL_ICOUNT(4); break;
	case 0x93: cmpd_di(); SUBALL_ICOUNT(7); break;
	case 0x9c: cmpy_di(); SUBALL_ICOUNT(7); break;
	case 0x9e: ldy_di(); SUBALL_ICOUNT(6); break;
	case 0x9f: sty_di(); SUBALL_ICOUNT(6); break;
	case 0xa3: cmpd_ix(); SUBALL_ICOUNT(7); break;
	case 0xac: cmpy_ix(); SUBALL_ICOUNT(7); break;
	case 0xae: ldy_ix(); SUBALL_ICOUNT(6); break;
	case 0xaf: sty_ix(); SUBALL_ICOUNT(6); break;
	case 0xb3: cmpd_ex(); SUBALL_ICOUNT(8); break;
	case 0xbc: cmpy_ex(); SUBALL_ICOUNT(8); break;
	case 0xbe: ldy_ex(); SUBALL_ICOUNT(7); break;
	case 0xbf: sty_ex(); SUBALL_ICOUNT(7); break;
	case 0xce: lds_im(); SUBALL_ICOUNT(4); break;
	case 0xcf: err_sts_im(); SUBALL_ICOUNT(4); break;
	case 0xde: lds_di(); SUBALL_ICOUNT(6); break;
	case 0xdf: sts_di(); SUBALL_ICOUNT(6); break;
	case 0xee: lds_ix(); SUBALL_ICOUNT(6); break;
	case 0xef: sts_ix(); SUBALL_ICOUNT(6); break;
	case 0xfe: lds_ex(); SUBALL_ICOUNT(7); break;
	case 0xff: sts_ex(); SUBALL_ICOUNT(7); break;
	default: illegal(ireg2 | 0x1000); break;
	}
}

/* $11xx opcodes */
void MC6809::pref11()
{
	uint8_t ireg2 = ROP(PCD);
	PC++;

	DASM_SET_CODE2(ireg2);

	switch(ireg2) {
	case 0x3f: swi3(); SUBALL_ICOUNT(20); break;
	case 0x83: cmpu_im(); SUBALL_ICOUNT(5); break;
	case 0x8c: cmps_im(); SUBALL_ICOUNT(5); break;
	case 0x93: cmpu_di(); SUBALL_ICOUNT(7); break;
	case 0x9c: cmps_di(); SUBALL_ICOUNT(7); break;
	case 0xa3: cmpu_ix(); SUBALL_ICOUNT(7); break;
	case 0xac: cmps_ix(); SUBALL_ICOUNT(7); break;
	case 0xb3: cmpu_ex(); SUBALL_ICOUNT(8); break;
	case 0xbc: cmps_ex(); SUBALL_ICOUNT(8); break;
	default: illegal(ireg2 | 0x1100); break;
	}
}

// ----------------------------------------------------------------------------

void MC6809::save_state(FILEIO *fio)
{
	struct vm_state_st vm_state;

	//
	vm_state_ident.version = Uint16_LE(2);
	vm_state_ident.size = Uint32_LE(sizeof(vm_state_ident) + sizeof(vm_state));

	// copy values
	memset(&vm_state, 0, sizeof(vm_state));
	vm_state.pc.d = Uint32_LE(pc.d);
	vm_state.ppc.d = Uint32_LE(ppc.d);
	vm_state.acc.d = Uint32_LE(acc.d);
	vm_state.dp.d = Uint32_LE(dp.d);
	vm_state.u.d = Uint32_LE(u.d);
	vm_state.s.d = Uint32_LE(s.d);
	vm_state.x.d = Uint32_LE(x.d);
	vm_state.y.d = Uint32_LE(y.d);
	vm_state.cc = cc;
	vm_state.ea.d = Uint32_LE(ea.d);

	vm_state.v2.int_state = Uint32_LE(int_state);

//	vm_state.now_reset = now_reset ? 1 : 0;

	fio->Fwrite(&vm_state_ident, sizeof(vm_state_ident), 1);
	fio->Fwrite(&vm_state, sizeof(vm_state), 1);
}

bool MC6809::load_state(FILEIO *fio)
{
	vm_state_ident_t vm_state_i;
	struct vm_state_st vm_state;

	READ_STATE_CHUNK(fio, vm_state_i, vm_state);

	// copy values
	pc.d = Uint32_LE(vm_state.pc.d);
	ppc.d = Uint32_LE(vm_state.ppc.d);
	acc.d = Uint32_LE(vm_state.acc.d);
	dp.d = Uint32_LE(vm_state.dp.d);
	u.d = Uint32_LE(vm_state.u.d);
	s.d = Uint32_LE(vm_state.s.d);
	x.d = Uint32_LE(vm_state.x.d);
	y.d = Uint32_LE(vm_state.y.d);
	cc = vm_state.cc;
	ea.d = Uint32_LE(vm_state.ea.d);

	if (Uint16_LE(vm_state_i.version) >= 2) {
		int_state = Uint32_LE(vm_state.v2.int_state);
		now_reset = ((int_state & MC6809_RESET_BIT) != 0);
	} else {
		uint8_t old = vm_state.v1.int_state;
		now_reset = vm_state.v1.now_reset ? true : false;
		// convert
		int_state = (old & (MC6809_IRQ_BIT | MC6809_FIRQ_BIT | MC6809_NMI_BIT));
		if (old & MC6809_CWAI_V1) int_state |= MC6809_CWAI;
		if (old & MC6809_SYNC_V1) int_state |= MC6809_SYNC;
		if (old & MC6809_LDS_V1) int_state |= MC6809_LDS;
		if (old & MC6809_HALT_BIT_V1) int_state |= MC6809_HALT_BIT;
		if (now_reset) int_state |= MC6809_RESET_BIT;
	}
	int_released = 0;
	icount = 0;

	return true;
}

// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
//void MC6809::debug_write_data8(uint32_t addr, uint32_t data)
//{
//	int wait;
//	d_mem_stored->write_data8w(addr, data, &wait);
//}

//uint32_t MC6809::debug_read_data8(uint32_t addr)
//{
//	return d_mem_stored->debug_read_data8(addr);
//}

#define DEBUG_WRITE_REG(reg_name, reg_variable) \
	if(_tcsicmp(reg, _T(reg_name)) == 0) { \
		reg_variable = data; \
		return true; \
	}

bool MC6809::debug_write_reg(const _TCHAR *reg, uint32_t data)
{
	DEBUG_WRITE_REG("PC", PC)
	DEBUG_WRITE_REG("DP", DP)
	DEBUG_WRITE_REG("A", A)
	DEBUG_WRITE_REG("B", B)
	DEBUG_WRITE_REG("D", D)
	DEBUG_WRITE_REG("U", U)
	DEBUG_WRITE_REG("X", X)
	DEBUG_WRITE_REG("Y", Y)
	DEBUG_WRITE_REG("S", S)
	DEBUG_WRITE_REG("CC", CC)
	return false;
}

void MC6809::debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	dasm.print_regs_current(PC, CC, DP, A, B, X, Y, S, U, int_state_debug, int_flags_debug);
	UTILITY::tcscpy(buffer, buffer_len, dasm.get_line());
}

#define SET_DEBUG_REG_PTR(reg_name, reg_variable) \
	if(_tcsicmp(reg, _T(reg_name)) == 0) { \
		UTILITY::tcscpy(reg, regsiz, _T(reg_name)); \
		regptr = reinterpret_cast<void *>(&reg_variable); \
		reglen = static_cast<int>(sizeof(reg_variable)); \
		return true; \
	}

bool MC6809::get_debug_reg_ptr(_TCHAR *reg, size_t regsiz, void * &regptr, int &reglen)
{
	SET_DEBUG_REG_PTR("PC", PC)
	SET_DEBUG_REG_PTR("DP", DP)
	SET_DEBUG_REG_PTR("A", A)
	SET_DEBUG_REG_PTR("B", B)
	SET_DEBUG_REG_PTR("D", D)
	SET_DEBUG_REG_PTR("U", U)
	SET_DEBUG_REG_PTR("X", X)
	SET_DEBUG_REG_PTR("Y", Y)
	SET_DEBUG_REG_PTR("S", S)
	SET_DEBUG_REG_PTR("UH", u.b.h)
	SET_DEBUG_REG_PTR("UL", u.b.l)
	SET_DEBUG_REG_PTR("XH", x.b.h)
	SET_DEBUG_REG_PTR("XL", x.b.l)
	SET_DEBUG_REG_PTR("YH", y.b.h)
	SET_DEBUG_REG_PTR("YL", y.b.l)
	SET_DEBUG_REG_PTR("SH", s.b.h)
	SET_DEBUG_REG_PTR("SL", s.b.l)
	SET_DEBUG_REG_PTR("CC", CC)
	return false;
}

int MC6809::debug_dasm(int type, uint32_t pc, _TCHAR *buffer, size_t buffer_len, int flags)
{
	int opspos;
	if ((flags & 1) == 0) {
		// next opcode
		opspos = dasm.print_dasm_preprocess(type, pc, flags);
		if (buffer) UTILITY::tcscpy(buffer, buffer_len, dasm.get_line());
	} else {
		// current opcode
		opspos = dasm.print_dasm_processed(pc & 0xffff);
		if (buffer) UTILITY::tcscpy(buffer, buffer_len, dasm.get_line());
	}
	return opspos;
}

int MC6809::debug_dasm_label(int type, uint32_t pc, _TCHAR *buffer, size_t buffer_len)
{
	int len = dasm.print_dasm_label(type, pc);
	if (len) {
		UTILITY::tcscpy(buffer, buffer_len, dasm.get_line());
	}
	return len;
}

int MC6809::debug_trace_back_regs(int index, _TCHAR *buffer, size_t buffer_len)
{
	int next = dasm.print_regs_traceback(index);
	if (next >= -1) {
		UTILITY::tcscpy(buffer, buffer_len, dasm.get_line());
	}
	return next;
}

int MC6809::debug_trace_back(int index, _TCHAR *buffer, size_t buffer_len)
{
	int next = dasm.print_dasm_traceback(index);
	if (next >= -1) {
		UTILITY::tcscpy(buffer, buffer_len, dasm.get_line());
	}
	return next;
}

bool MC6809::reach_break_point()
{
	return d_debugger->reach_break_point_at(PC);
}

//void MC6809::go_suspend()
//{
//	d_debugger->go_suspend();
//}

//bool MC6809::now_suspend()
//{
//	return d_debugger->now_suspend();
//}

uint32_t MC6809::get_debug_pc(int type) {
#ifdef _MBS1
	return type ? d_mem_stored->debug_latch_address(ppc.w.l) : ppc.w.l;
#else
	return ppc.w.l;
#endif
}

uint32_t MC6809::get_debug_next_pc(int type) {
#ifdef _MBS1
	return type ? d_mem_stored->debug_latch_address(pc.w.l) : pc.w.l;
#else
	return pc.w.l;
#endif
}

uint32_t MC6809::get_debug_branch_pc(int type) {
	uint16_t addr = npc != ppc.w.l ? npc : pc.w.l;
#ifdef _MBS1
	return type ? d_mem_stored->debug_latch_address(addr) : addr;
#else
	return addr;
#endif
}

/// signal names
static struct {
	const _TCHAR *name;
	uint32_t        num;
} signal_names_map[] = {
	{ _T("RESET"), SIG_CPU_RESET },
	{ _T("NMI"), SIG_CPU_NMI },
	{ _T("IRQ"), SIG_CPU_IRQ },
	{ _T("FIRQ"), SIG_CPU_FIRQ },
	{ _T("HALT"), SIG_CPU_HALT },
	{ NULL, 0 }
};

bool MC6809::get_debug_signal_name_index(const _TCHAR *param, uint32_t *num, uint32_t *mask, int *idx, const _TCHAR **name)
{
	int i = 0; 
	for(; signal_names_map[i].name != NULL; i++) {
		if (_tcsicmp(param, signal_names_map[i].name) == 0) {
			if (num) *num = signal_names_map[i].num;
			if (mask) *mask = d_debugger->get_stored_mask();
			if (idx) *idx = i;
			if (name) *name = signal_names_map[i].name;
			return true;
		}
	}
	return false;
}

bool MC6809::get_debug_signal_name_index(uint32_t num, uint32_t *mask, int *idx, const _TCHAR **name)
{
	int i = 0; 
	for(; signal_names_map[i].name != NULL; i++) {
		if (signal_names_map[i].num == num) {
			if (mask) *mask = d_debugger->get_stored_mask();
			if (idx) *idx = i;
			if (name) *name = signal_names_map[i].name;
			return true;
		}
	}
	return false;
}

void MC6809::get_debug_signal_names_str(_TCHAR *buffer, size_t buffer_len)
{
	int i = 0;
	buffer[0] = _T('\0');
	for(; signal_names_map[i].name != NULL; i++) {
		if (i > 0) UTILITY::tcscat(buffer, buffer_len, _T(","));
		UTILITY::tcscat(buffer, buffer_len, signal_names_map[i].name);
	}
}

#endif /* USE_DEBUGGER */
