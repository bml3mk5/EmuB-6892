/** @file mc6809.h

	Skelton for retropc emulator

	@par Origin
	MAME 0.142
	@author Takeda.Toshiya
	@date   2011.05.06-

	@note
	Modified by Sasaji at 2012.06.20

	@brief [ MC6809 ]
*/

#ifndef MC6809_H
#define MC6809_H

#include "vm_defs.h"
#include "device.h"
#include "mc6809dasm.h"
#ifdef USE_DEBUGGER
#include "debugger.h"
#endif

#define ILLEGAL_PC_MAX 14

class EMU;

/**
	@brief MC6809 - CPU
*/
class MC6809 : public DEVICE
{
private:
	// context
	DEVICE *d_mem;
#ifdef USE_DEBUGGER
	//for dis-assemble
	MC6809DASM dasm;
	DEBUGGER *d_debugger;
	DEVICE *d_mem_stored;
	uint32_t ea_old;
	uint16_t npc;
#endif

	// registers
	pair32_t pc; 	/* Program counter */
	pair32_t ppc;	/* Previous program counter */
	pair32_t acc;	/* Accumulator a and b */
	pair32_t dp;	/* Direct Page register (page in MSB) */
	pair32_t u, s;	/* Stack pointers */
	pair32_t x, y;	/* Index registers */
	uint8_t cc;
	pair32_t ea;	/* effective address */

	uint32_t int_state;
	uint32_t int_released;
#ifdef USE_DEBUGGER
	uint32_t int_state_debug;
	uint32_t int_flags_debug[MC6809_IDX_END];
#endif
	/* When USE_CPU_REAL_MACHINE_CYCLE is on, icount is increased. */
	int icount;
	int iaccum;
#ifdef USE_CPU_REAL_MACHINE_CYCLE
	int accum_cycle;
#endif
	uint32_t cpu_clock;

	uint16_t illegal_pc[ILLEGAL_PC_MAX];
	uint16_t illegal_pc_idx;

	//for resume
#pragma pack(1)
	struct vm_state_st {
		pair32_t pc;
		pair32_t ppc;
		pair32_t acc;
		pair32_t dp;
		pair32_t u, s;
		pair32_t x, y;
		uint8_t cc;
		pair32_t ea;

		union {
			struct {
				uint8_t int_state;
				uint8_t now_reset;
				char reserved[3];
			} v1;
			struct {
				char reserved;
				uint32_t int_state;
			} v2;
		};

		char reserved[6];
	};
#pragma pack()

	inline uint32_t RM16(uint32_t Addr);
	inline void WM16(uint32_t Addr, pair32_t *p);

	// opcodes
	void run_one_opecode();
	void op(uint8_t ireg);
	inline void fetch_effective_address();
	inline void abx();
	inline void adca_di();
	inline void adca_ex();
	inline void adca_im();
	inline void adca_ix();
	inline void adcb_di();
	inline void adcb_ex();
	inline void adcb_im();
	inline void adcb_ix();
	inline void adda_di();
	inline void adda_ex();
	inline void adda_im();
	inline void adda_ix();
	inline void addb_di();
	inline void addb_ex();
	inline void addb_im();
	inline void addb_ix();
	inline void addd_di();
	inline void addd_ex();
	inline void addd_im();
	inline void addd_ix();
	inline void anda_di();
	inline void anda_ex();
	inline void anda_im();
	inline void anda_ix();
	inline void andb_di();
	inline void andb_ex();
	inline void andb_im();
	inline void andb_ix();
	inline void andcc();
	inline void asla();
	inline void aslb();
	inline void asl_di();
	inline void asl_ex();
	inline void asl_ix();
	inline void asra();
	inline void asrb();
	inline void asr_di();
	inline void asr_ex();
	inline void asr_ix();
	inline void bcc();
	inline void bcs();
	inline void beq();
	inline void bge();
	inline void bgt();
	inline void bhi();
	inline void bita_di();
	inline void bita_ex();
	inline void bita_im();
	inline void bita_ix();
	inline void bitb_di();
	inline void bitb_ex();
	inline void bitb_im();
	inline void bitb_ix();
	inline void ble();
	inline void bls();
	inline void blt();
	inline void bmi();
	inline void bne();
	inline void bpl();
	inline void bra();
	inline void brn();
	inline void bsr();
	inline void bvc();
	inline void bvs();
	inline void clra();
	inline void clrb();
	inline void clr_di();
	inline void clr_ex();
	inline void clr_ix();
	inline void cmpa_di();
	inline void cmpa_ex();
	inline void cmpa_im();
	inline void cmpa_ix();
	inline void cmpb_di();
	inline void cmpb_ex();
	inline void cmpb_im();
	inline void cmpb_ix();
	inline void cmpd_di();
	inline void cmpd_ex();
	inline void cmpd_im();
	inline void cmpd_ix();
	inline void cmps_di();
	inline void cmps_ex();
	inline void cmps_im();
	inline void cmps_ix();
	inline void cmpu_di();
	inline void cmpu_ex();
	inline void cmpu_im();
	inline void cmpu_ix();
	inline void cmpx_di();
	inline void cmpx_ex();
	inline void cmpx_im();
	inline void cmpx_ix();
	inline void cmpy_di();
	inline void cmpy_ex();
	inline void cmpy_im();
	inline void cmpy_ix();
	inline void coma();
	inline void comb();
	inline void com_di();
	inline void com_ex();
	inline void com_ix();
	inline void cwai();
	inline void daa();
	inline void deca();
	inline void decb();
	inline void dec_di();
	inline void dec_ex();
	inline void dec_ix();
	inline void eora_di();
	inline void eora_ex();
	inline void eora_im();
	inline void eora_ix();
	inline void eorb_di();
	inline void eorb_ex();
	inline void eorb_im();
	inline void eorb_ix();
	inline void exg();
	void illegal(uint16_t ireg);
	void illegal_ix(uint8_t postbyte);
	inline void inca();
	inline void incb();
	inline void inc_di();
	inline void inc_ex();
	inline void inc_ix();
	inline void jmp_di();
	inline void jmp_ex();
	inline void jmp_ix();
	inline void jsr_di();
	inline void jsr_ex();
	inline void jsr_ix();
	inline void lbcc();
	inline void lbcs();
	inline void lbeq();
	inline void lbge();
	inline void lbgt();
	inline void lbhi();
	inline void lble();
	inline void lbls();
	inline void lblt();
	inline void lbmi();
	inline void lbne();
	inline void lbpl();
	inline void lbra();
	inline void lbrn();
	inline void lbsr();
	inline void lbvc();
	inline void lbvs();
	inline void lda_di();
	inline void lda_ex();
	inline void lda_im();
	inline void lda_ix();
	inline void ldb_di();
	inline void ldb_ex();
	inline void ldb_im();
	inline void ldb_ix();
	inline void ldd_di();
	inline void ldd_ex();
	inline void ldd_im();
	inline void ldd_ix();
	inline void lds_di();
	inline void lds_ex();
	inline void lds_im();
	inline void lds_ix();
	inline void ldu_di();
	inline void ldu_ex();
	inline void ldu_im();
	inline void ldu_ix();
	inline void ldx_di();
	inline void ldx_ex();
	inline void ldx_im();
	inline void ldx_ix();
	inline void ldy_di();
	inline void ldy_ex();
	inline void ldy_im();
	inline void ldy_ix();
	inline void leas();
	inline void leau();
	inline void leax();
	inline void leay();
	inline void lsra();
	inline void lsrb();
	inline void lsr_di();
	inline void lsr_ex();
	inline void lsr_ix();
	inline void mul();
	inline void nega();
	inline void negb();
	inline void neg_di();
	inline void neg_ex();
	inline void neg_ix();
	inline void nop();
	inline void ora_di();
	inline void ora_ex();
	inline void ora_im();
	inline void ora_ix();
	inline void orb_di();
	inline void orb_ex();
	inline void orb_im();
	inline void orb_ix();
	inline void orcc();
	inline void pref10();
	inline void pref11();
	inline void pshs();
	inline void pshu();
	inline void puls();
	inline void pulu();
	inline void rola();
	inline void rolb();
	inline void rol_di();
	inline void rol_ex();
	inline void rol_ix();
	inline void rora();
	inline void rorb();
	inline void ror_di();
	inline void ror_ex();
	inline void ror_ix();
	inline void rti();
	inline void rts();
	inline void sbca_di();
	inline void sbca_ex();
	inline void sbca_im();
	inline void sbca_ix();
	inline void sbcb_di();
	inline void sbcb_ex();
	inline void sbcb_im();
	inline void sbcb_ix();
	inline void sex();
	inline void sta_di();
	inline void sta_ex();
	inline void err_sta_im();
	inline void sta_ix();
	inline void stb_di();
	inline void stb_ex();
	inline void err_stb_im();
	inline void stb_ix();
	inline void std_di();
	inline void std_ex();
//	inline void err_std_im();
	inline void std_ix();
	inline void sts_di();
	inline void sts_ex();
	inline void err_sts_im();
	inline void sts_ix();
	inline void stu_di();
	inline void stu_ex();
	inline void err_stu_im();
	inline void stu_ix();
	inline void stx_di();
	inline void stx_ex();
	inline void err_stx_im();
	inline void stx_ix();
	inline void sty_di();
	inline void sty_ex();
	inline void err_sty_im();
	inline void sty_ix();
	inline void suba_di();
	inline void suba_ex();
	inline void suba_im();
	inline void suba_ix();
	inline void subb_di();
	inline void subb_ex();
	inline void subb_im();
	inline void subb_ix();
	inline void subd_di();
	inline void subd_ex();
	inline void subd_im();
	inline void subd_ix();
	inline void swi2();
	inline void swi3();
	inline void swi();
	inline void sync();
	inline void tfr();
	inline void tsta();
	inline void tstb();
	inline void tst_di();
	inline void tst_ex();
	inline void tst_ix();

	inline void err_neg_di();
	inline void err_ngc_di();
	inline void err_lsr_di();
	inline void err_dcc_di();
	inline void err_halt(uint16_t);
	inline void err_slcc();
	inline void err_nop();
	inline void err_cwai();
	inline void err_reset();
	inline void err_nega();
	inline void err_ngca();
	inline void err_lsra();
	inline void err_dcca();
	inline void err_clca();
	inline void err_negb();
	inline void err_ngcb();
	inline void err_lsrb();
	inline void err_dccb();
	inline void err_clcb();
	inline void err_neg_ix();
	inline void err_ngc_ix();
	inline void err_lsr_ix();
	inline void err_dcc_ix();
	inline void err_neg_ex();
	inline void err_ngc_ex();
	inline void err_lsr_ex();
	inline void err_dcc_ex();

public:
	MC6809(VM* parent_vm, EMU* parent_emu, const char* identifier);
	~MC6809() {}

	// common functions
	void initialize();
	void reset();
	int run(int clock);
#ifdef USE_CPU_REAL_MACHINE_CYCLE
	int run(int clock, int accum, int cycle);
#endif
	void write_signal(int id, uint32_t data, uint32_t mask);
	uint32_t get_pc() {
		return ppc.w.l;
	}
	uint32_t get_next_pc() {
		return pc.w.l;
	}
	void set_cpu_clock(uint32_t clk) {
		cpu_clock = clk;
	}
	uint32_t get_cpu_clock() const {
		return cpu_clock;
	}

	// unique function
	void set_context_mem(DEVICE* device) {
		d_mem = device;
#ifdef USE_DEBUGGER
		dasm.set_context_mem(device);
#endif /* USE_DEBUGGER */
	}

	void save_state(FILEIO *fio);
	bool load_state(FILEIO *fio);

#ifdef USE_DEBUGGER
	void set_context_debugger(DEBUGGER* device)
	{
		d_debugger = device;
		dasm.set_context_debugger(device);
	}
	DEVICE *get_debugger()
	{
		return d_debugger;
	}
	uint32_t debug_prog_addr_mask()
	{
		return 0xffff;
	}
	uint32_t debug_data_addr_mask()
	{
		return 0xffff;
	}
	uint32_t debug_data_mask()
	{
		return 0xff;
	}
	bool debug_ioport_is_supported() const
	{
		return false;	// because memory mapped
	}
	bool debug_write_reg(const _TCHAR *reg, uint32_t data);
	void debug_regs_info(_TCHAR *buffer, size_t buffer_len);
	bool get_debug_reg_ptr(_TCHAR *reg, size_t regsiz, void * &regptr, int &reglen);
	int debug_dasm(int type, uint32_t pc, _TCHAR *buffer, size_t buffer_len, int flags);
	int debug_dasm_label(int type, uint32_t pc, _TCHAR *buffer, size_t buffer_len);
	int debug_trace_back_regs(int index, _TCHAR *buffer, size_t buffer_len);
	int debug_trace_back(int index, _TCHAR *buffer, size_t buffer_len);
	bool reach_break_point();
//	void go_suspend();
//	bool now_suspend();
	uint32_t get_debug_pc(int type);
	uint32_t get_debug_next_pc(int type);
	uint32_t get_debug_branch_pc(int type);

	bool get_debug_signal_name_index(const _TCHAR *param, uint32_t *num, uint32_t *mask, int *idx, const _TCHAR **name);
	bool get_debug_signal_name_index(uint32_t num, uint32_t *mask, int *idx, const _TCHAR **name);
	void get_debug_signal_names_str(_TCHAR *buffer, size_t buffer_len);
#endif /* USE_DEBUGGER */
};

#endif /* MC6809_H */

