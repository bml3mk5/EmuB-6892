/** @file mc6809dasm.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.08.29 -

	@brief [ mc6809 disassembler ]
*/

#ifndef MC6809DASM_H
#define MC6809DASM_H

#ifndef USE_DEBUGGER
#define DASM_SET_INH()
#define DASM_SET_IMM(a1)
#define DASM_SET_DIR1(a1,a2,a3)
#define DASM_SET_DIR2(a1,a2,a3,a4)
#define DASM_SET_IDX1(a1)
#define DASM_SET_IDX2(a1)
#define DASM_SET_EXT1(a1,a2)
#define DASM_SET_EXT2(a1,a2,a3)
#define DASM_SET_JMP(a1)
#define DASM_SET_JMP_DIR(a1,a2)
#define DASM_SET_REL(a1)
#define DASM_SET_TFR(a1)
#define DASM_SET_PSH(a1)
#define DASM_SET_SWI()
#define DASM_SET_SWI2()
#define DASM_SET_SWI3()
#define DASM_SET_ERR()
#define DASM_SET_CODE2(a1)
#define DASM_SET_MEM1(a1,a2)
#define DASM_SET_MEM2(a1,a2,a3)

#else /* USE_DEBUGGER*/
#define DASM_SET_INH()				dasm.set_inh()
#define DASM_SET_IMM(a1)			dasm.set_imm(a1)
#define DASM_SET_DIR1(a1,a2,a3)		dasm.set_dir(a1, a2, a3)
#define DASM_SET_DIR2(a1,a2,a3,a4)	dasm.set_dir(a1, a2, a3, a4)
#define DASM_SET_IDX1(a1)			dasm.set_idx1(a1)
#define DASM_SET_IDX2(a1)			dasm.set_idx2(a1)
#define DASM_SET_EXT1(a1,a2)		dasm.set_ext(a1, a2)
#define DASM_SET_EXT2(a1,a2,a3)		dasm.set_ext(a1, a2, a3)
#define DASM_SET_JMP(a1)			dasm.set_jmp(a1)
#define DASM_SET_JMP_DIR(a1,a2)		dasm.set_dir_addr(a1, a2)
#define DASM_SET_REL(a1)			dasm.set_rel(a1)
#define DASM_SET_TFR(a1)			dasm.set_tfr(a1)
#define DASM_SET_PSH(a1)			dasm.set_psh(a1)
#define DASM_SET_SWI()				dasm.set_swi()
#define DASM_SET_SWI2()				dasm.set_swi2()
#define DASM_SET_SWI3()				dasm.set_swi3()
#define DASM_SET_ERR()				dasm.set_err()
#define DASM_SET_CODE2(a1)			dasm.set_code2(a1)
#define DASM_SET_MEM1(a1,a2)		dasm.set_mem(a1, a2)
#define DASM_SET_MEM2(a1,a2,a3)		dasm.set_mem(a1, a2, a3)


#include "../emu.h"
#include "vm.h"
#include "../common.h"

class DEBUGGER;

#define MC6809DASM_PCSTACK_COUNT 20

/// signals on MC6809
enum mc6809_signal_index {
	MC6809_IDX_RESET = 0,
	MC6809_IDX_NMI,
	MC6809_IDX_IRQ,
	MC6809_IDX_FIRQ,
	MC6809_IDX_HALT,
	MC6809_IDX_END
};

/// store operating status on MC6809
typedef struct mc6809dasm_regs_st {
	uint32_t phyaddr;
	uint16_t pc;
	uint16_t s;
	uint16_t u;
	uint16_t x;
	uint16_t y;
	uint8_t  dp;
	uint8_t  a;

	uint8_t  b;
	uint8_t  cc;
	uint8_t  code[5];
	uint8_t  flags;	// bit0:valid data  bit1:data store=1  bit2:write data=1  bit3:2bytes data=1 

	uint32_t state;
	uint32_t int_flags[MC6809_IDX_END];

	uint32_t rw_phyaddr;
	uint16_t rw_addr;

	uint16_t rw_data;
	uint16_t cycles;
} mc6809dasm_regs_t;

#define MC6809DASM_PCSTACK_NUM 1000
#define MC6809DASM_CMDLINE_LEN	128

/**
	@brief MC6809 disassembler
*/
class MC6809DASM
{
public:
	enum en_opmode {
		NONE = 0,
		INH,
		DIR1,
		EXT,
		IMM1,
		IMM2,
		IDX,
		REL1,
		REL2,
		JMP,
		TFR,
		PSH,
		SWI2,
		FDB,
		ERR,
	};

	typedef struct opcode_st {
		uint8_t       c;
		const _TCHAR *s;
		en_opmode     m;
	} opcode_t;

private:
	DEVICE *d_mem;
	DEBUGGER *debugger;

	_TCHAR line[_MAX_PATH];

	mc6809dasm_regs_t regs[MC6809DASM_PCSTACK_NUM];
	mc6809dasm_regs_t *current_reg;
	int current_idx;
	int codelen;

	_TCHAR cmd[MC6809DASM_CMDLINE_LEN];

	int set_cmd_str(uint32_t addr, const uint8_t *ops, en_opmode *mod);
	int set_idx_str(uint32_t phyaddr, uint16_t pc, const uint8_t *ops, int opspos, int flags);
	int set_tfr_str(const uint8_t *ops, int opspos);
	int set_psh_str(const uint8_t *ops, int opspos);
	int set_swi2_str(uint32_t addr, const uint8_t *ops, int opspos);
	uint32_t swi2_appear;

	void set_ext_addr(uint16_t addr);

	void push_stack_pc(uint32_t phyaddr, uint16_t pc, uint8_t code);

public:
	MC6809DASM();
	~MC6809DASM();

	void ini_pc(uint16_t pc, uint8_t code);
	void set_code2(uint8_t code2);
	void set_swi2_param(uint8_t cat, uint8_t idx);

	void set_mem(uint16_t addr, uint8_t data, bool write = false);
	void set_mem(uint16_t addr, uint16_t data, bool write = false);
	void set_phymem(uint32_t addr, uint8_t data, bool write = false);

	void set_inh();
	void set_swi();
	void set_swi2();
	void set_swi3();

	void set_dir_addr(uint8_t dp, uint8_t addrl);
	void set_dir(uint8_t dp, uint8_t addrl, uint8_t data, bool write = false);
	void set_dir(uint8_t dp, uint8_t addrl, uint16_t data, bool write = false);

	void set_imm(uint8_t data);
	void set_imm(uint16_t data);

	void set_ext(uint16_t addr, uint8_t data, bool write = false);
	void set_ext(uint16_t addr, uint16_t data, bool write = false);

	void set_idx1(uint8_t data);
	void set_idx2(uint8_t data);
	void set_idx2(uint16_t data);

	void set_rel(uint8_t data);
	void set_rel(uint16_t data);
	void set_jmp(uint16_t data);

	void set_tfr(uint8_t data);

	void set_psh(uint8_t data);

	void set_err();

	void set_signal(uint32_t state, int int_flags_idx, uint32_t int_flags);

	void set_regs(int accum, int cycles, uint8_t cc, uint8_t dp, uint8_t a, uint8_t b, uint16_t x, uint16_t y, uint16_t s, uint16_t u);

	int print_dasm(uint32_t phy_addr, uint16_t pc, const uint8_t *ops, int opslen, int flags, uint8_t *dp);
	int print_dasm_label(int type, uint32_t pc);
	int print_dasm_preprocess(int type, uint32_t pc, int flags);
	int print_dasm_processed(uint16_t pc);
	int print_dasm_traceback(int index);

	void print_cycles(int cycles);
	void print_regs(const mc6809dasm_regs_t &regs);
	void print_regs_current();
	void print_regs_current(uint16_t pc, uint8_t cc, uint8_t dp, uint8_t a, uint8_t b, uint16_t x, uint16_t y, uint16_t s, uint16_t u, uint32_t state, uint32_t *int_flags);
	int print_regs_traceback(int index);

	void print_memory_data(mc6809dasm_regs_t &stack);

	int get_stack_pc(int index, mc6809dasm_regs_t *stack);

	const _TCHAR *get_line() const {
		return line;
	}
	size_t get_line_length() const {
		return _tcslen(line);
	}

	void set_context_mem(DEVICE* device) {
		d_mem = device;
	}
	void set_context_debugger(DEBUGGER *device) {
		debugger = device;
	}
};

#endif /* USE_DEBUGGER */

#endif /* MC6809DASM_H */
