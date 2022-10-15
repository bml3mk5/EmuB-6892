/** @file mc6809dasm.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.08.29 -

	@brief [ mc6809 disassembler ]
*/

#include "mc6809dasm.h"
#include "device.h"
#include "mc6809_consts.h"

#ifdef USE_DEBUGGER

#include "debugger.h"
#include "../utility.h"

static const MC6809DASM::opcode_t opcodes0[] =
{
{ 0x00, _T("NEG"), MC6809DASM::DIR1 },
{ 0x01, _T("NEG??"), MC6809DASM::DIR1 },
{ 0x02, _T("NGC??"), MC6809DASM::DIR1 },
{ 0x03, _T("COM"), MC6809DASM::DIR1 },
{ 0x04, _T("LSR"), MC6809DASM::DIR1 },
{ 0x05, _T("LSR??"), MC6809DASM::DIR1 },
{ 0x06, _T("ROR"), MC6809DASM::DIR1 },
{ 0x07, _T("ASR"), MC6809DASM::DIR1 },
{ 0x08, _T("ASL"), MC6809DASM::DIR1 },
{ 0x09, _T("ROL"), MC6809DASM::DIR1 },
{ 0x0A, _T("DEC"), MC6809DASM::DIR1 },
{ 0x0B, _T("DCC??"), MC6809DASM::DIR1 },
{ 0x0C, _T("INC"), MC6809DASM::DIR1 },
{ 0x0D, _T("TST"), MC6809DASM::DIR1 },
{ 0x0E, _T("JMP"), MC6809DASM::DIR1 },
{ 0x0F, _T("CLR"), MC6809DASM::DIR1 },

{ 0x10, _T("page1"), MC6809DASM::NONE },
{ 0x11, _T("page2"), MC6809DASM::NONE },
{ 0x12, _T("NOP"), MC6809DASM::INH },
{ 0x13, _T("SYNC"), MC6809DASM::INH },
{ 0x14, _T("HCF??"), MC6809DASM::INH },
{ 0x15, _T("HCF??"), MC6809DASM::INH },
{ 0x16, _T("LBRA"), MC6809DASM::REL2 },
{ 0x17, _T("LBSR"), MC6809DASM::REL2 },
{ 0x18, _T("SLCC?"), MC6809DASM::INH },
{ 0x19, _T("DAA"), MC6809DASM::INH },
{ 0x1A, _T("ORCC"), MC6809DASM::IMM1 },
{ 0x1B, _T("NOP??"), MC6809DASM::INH },
{ 0x1C, _T("ANDCC"), MC6809DASM::IMM1 },
{ 0x1D, _T("SEX"), MC6809DASM::INH },
{ 0x1E, _T("EXG"), MC6809DASM::TFR },
{ 0x1F, _T("TFR"), MC6809DASM::TFR },

{ 0x20, _T("BRA"), MC6809DASM::REL1 },
{ 0x21, _T("BRN"), MC6809DASM::REL1 },
{ 0x22, _T("BHI"), MC6809DASM::REL1 },
{ 0x23, _T("BLS"), MC6809DASM::REL1 },
{ 0x24, _T("BCC"), MC6809DASM::REL1 },
{ 0x25, _T("BCS"), MC6809DASM::REL1 },
{ 0x26, _T("BNE"), MC6809DASM::REL1 },
{ 0x27, _T("BEQ"), MC6809DASM::REL1 },
{ 0x28, _T("BVC"), MC6809DASM::REL1 },
{ 0x29, _T("BVS"), MC6809DASM::REL1 },
{ 0x2A, _T("BPL"), MC6809DASM::REL1 },
{ 0x2B, _T("BMI"), MC6809DASM::REL1 },
{ 0x2C, _T("BGE"), MC6809DASM::REL1 },
{ 0x2D, _T("BLT"), MC6809DASM::REL1 },
{ 0x2E, _T("BGT"), MC6809DASM::REL1 },
{ 0x2F, _T("BLE"), MC6809DASM::REL1 },

{ 0x30, _T("LEAX"), MC6809DASM::IDX },
{ 0x31, _T("LEAY"), MC6809DASM::IDX },
{ 0x32, _T("LEAS"), MC6809DASM::IDX },
{ 0x33, _T("LEAU"), MC6809DASM::IDX },
{ 0x34, _T("PSHS"), MC6809DASM::PSH },
{ 0x35, _T("PULS"), MC6809DASM::PSH },
{ 0x36, _T("PSHU"), MC6809DASM::PSH },
{ 0x37, _T("PULU"), MC6809DASM::PSH },
{ 0x38, _T("CWAI?"), MC6809DASM::IMM1 },
{ 0x39, _T("RTS"), MC6809DASM::INH },
{ 0x3A, _T("ABX"), MC6809DASM::INH },
{ 0x3B, _T("RTI"), MC6809DASM::INH },
{ 0x3C, _T("CWAI"), MC6809DASM::IMM1 },
{ 0x3D, _T("MUL"), MC6809DASM::INH },
{ 0x3E, _T("RES??"), MC6809DASM::INH },
{ 0x3F, _T("SWI"), MC6809DASM::INH },

{ 0x40, _T("NEGA"), MC6809DASM::INH },
{ 0x41, _T("NEGA?"), MC6809DASM::INH },
{ 0x42, _T("NGCA?"), MC6809DASM::INH },
{ 0x43, _T("COMA"), MC6809DASM::INH },
{ 0x44, _T("LSRA"), MC6809DASM::INH },
{ 0x45, _T("LSRA?"), MC6809DASM::INH },
{ 0x46, _T("RORA"), MC6809DASM::INH },
{ 0x47, _T("ASRA"), MC6809DASM::INH },
{ 0x48, _T("ASLA"), MC6809DASM::INH },
{ 0x49, _T("ROLA"), MC6809DASM::INH },
{ 0x4A, _T("DECA"), MC6809DASM::INH },
{ 0x4B, _T("DCCA?"), MC6809DASM::INH },
{ 0x4C, _T("INCA"), MC6809DASM::INH },
{ 0x4D, _T("TSTA"), MC6809DASM::INH },
{ 0x4E, _T("CLCA?"), MC6809DASM::INH },
{ 0x4F, _T("CLRA"), MC6809DASM::INH },

{ 0x50, _T("NEGB"), MC6809DASM::INH },
{ 0x51, _T("NEGB?"), MC6809DASM::INH },
{ 0x52, _T("NGCB?"), MC6809DASM::INH },
{ 0x53, _T("COMB"), MC6809DASM::INH },
{ 0x54, _T("LSRB"), MC6809DASM::INH },
{ 0x55, _T("LSRB?"), MC6809DASM::INH },
{ 0x56, _T("RORB"), MC6809DASM::INH },
{ 0x57, _T("ASRB"), MC6809DASM::INH },
{ 0x58, _T("ASLB"), MC6809DASM::INH },
{ 0x59, _T("ROLB"), MC6809DASM::INH },
{ 0x5A, _T("DECB"), MC6809DASM::INH },
{ 0x5B, _T("DCCB?"), MC6809DASM::INH },
{ 0x5C, _T("INCB"), MC6809DASM::INH },
{ 0x5D, _T("TSTB"), MC6809DASM::INH },
{ 0x5E, _T("CLCB?"), MC6809DASM::INH },
{ 0x5F, _T("CLRB"), MC6809DASM::INH },

{ 0x60, _T("NEG"), MC6809DASM::IDX },
{ 0x61, _T("NEG??"), MC6809DASM::IDX },
{ 0x62, _T("NGC??"), MC6809DASM::IDX },
{ 0x63, _T("COM"), MC6809DASM::IDX },
{ 0x64, _T("LSR"), MC6809DASM::IDX },
{ 0x65, _T("LSR??"), MC6809DASM::IDX },
{ 0x66, _T("ROR"), MC6809DASM::IDX },
{ 0x67, _T("ASR"), MC6809DASM::IDX },
{ 0x68, _T("ASL"), MC6809DASM::IDX },
{ 0x69, _T("ROL"), MC6809DASM::IDX },
{ 0x6A, _T("DEC"), MC6809DASM::IDX },
{ 0x6B, _T("DCC??"), MC6809DASM::IDX },
{ 0x6C, _T("INC"), MC6809DASM::IDX },
{ 0x6D, _T("TST"), MC6809DASM::IDX },
{ 0x6E, _T("JMP"), MC6809DASM::IDX },
{ 0x6F, _T("CLR"), MC6809DASM::IDX },

{ 0x70, _T("NEG"), MC6809DASM::EXT },
{ 0x71, _T("NEG??"), MC6809DASM::EXT },
{ 0x72, _T("NGC??"), MC6809DASM::EXT },
{ 0x73, _T("COM"), MC6809DASM::EXT },
{ 0x74, _T("LSR"), MC6809DASM::EXT },
{ 0x75, _T("LSR??"), MC6809DASM::EXT },
{ 0x76, _T("ROR"), MC6809DASM::EXT },
{ 0x77, _T("ASR"), MC6809DASM::EXT },
{ 0x78, _T("ASL"), MC6809DASM::EXT },
{ 0x79, _T("ROL"), MC6809DASM::EXT },
{ 0x7A, _T("DEC"), MC6809DASM::EXT },
{ 0x7B, _T("DCC??"), MC6809DASM::EXT },
{ 0x7C, _T("INC"), MC6809DASM::EXT },
{ 0x7D, _T("TST"), MC6809DASM::EXT },
{ 0x7E, _T("JMP"), MC6809DASM::JMP },
{ 0x7F, _T("CLR"), MC6809DASM::EXT },

{ 0x80, _T("SUBA"), MC6809DASM::IMM1 },
{ 0x81, _T("CMPA"), MC6809DASM::IMM1 },
{ 0x82, _T("SBCA"), MC6809DASM::IMM1 },
{ 0x83, _T("SUBD"), MC6809DASM::IMM2 },
{ 0x84, _T("ANDA"), MC6809DASM::IMM1 },
{ 0x85, _T("BITA"), MC6809DASM::IMM1 },
{ 0x86, _T("LDA"), MC6809DASM::IMM1 },
{ 0x87, _T("STA??"), MC6809DASM::IMM1 },
{ 0x88, _T("EORA"), MC6809DASM::IMM1 },
{ 0x89, _T("ADCA"), MC6809DASM::IMM1 },
{ 0x8A, _T("ORA"), MC6809DASM::IMM1 },
{ 0x8B, _T("ADDA"), MC6809DASM::IMM1 },
{ 0x8C, _T("CMPX"), MC6809DASM::IMM2 },
{ 0x8D, _T("BSR"), MC6809DASM::REL1 },
{ 0x8E, _T("LDX"), MC6809DASM::IMM2 },
{ 0x8F, _T("STX??"), MC6809DASM::IMM2 },

{ 0x90, _T("SUBA"), MC6809DASM::DIR1 },
{ 0x91, _T("CMPA"), MC6809DASM::DIR1 },
{ 0x92, _T("SBCA"), MC6809DASM::DIR1 },
{ 0x93, _T("SUBD"), MC6809DASM::DIR1 },
{ 0x94, _T("ANDA"), MC6809DASM::DIR1 },
{ 0x95, _T("BITA"), MC6809DASM::DIR1 },
{ 0x96, _T("LDA"), MC6809DASM::DIR1 },
{ 0x97, _T("STA"), MC6809DASM::DIR1 },
{ 0x98, _T("EORA"), MC6809DASM::DIR1 },
{ 0x99, _T("ADCA"), MC6809DASM::DIR1 },
{ 0x9A, _T("ORA"), MC6809DASM::DIR1 },
{ 0x9B, _T("ADDA"), MC6809DASM::DIR1 },
{ 0x9C, _T("CMPX"), MC6809DASM::DIR1 },
{ 0x9D, _T("JSR"), MC6809DASM::DIR1 },
{ 0x9E, _T("LDX"), MC6809DASM::DIR1 },
{ 0x9F, _T("STX"), MC6809DASM::DIR1 },

{ 0xA0, _T("SUBA"), MC6809DASM::IDX },
{ 0xA1, _T("CMPA"), MC6809DASM::IDX },
{ 0xA2, _T("SBCA"), MC6809DASM::IDX },
{ 0xA3, _T("SUBD"), MC6809DASM::IDX },
{ 0xA4, _T("ANDA"), MC6809DASM::IDX },
{ 0xA5, _T("BITA"), MC6809DASM::IDX },
{ 0xA6, _T("LDA"), MC6809DASM::IDX },
{ 0xA7, _T("STA"), MC6809DASM::IDX },
{ 0xA8, _T("EORA"), MC6809DASM::IDX },
{ 0xA9, _T("ADCA"), MC6809DASM::IDX },
{ 0xAA, _T("ORA"), MC6809DASM::IDX },
{ 0xAB, _T("ADDA"), MC6809DASM::IDX },
{ 0xAC, _T("CMPX"), MC6809DASM::IDX },
{ 0xAD, _T("JSR"), MC6809DASM::IDX },
{ 0xAE, _T("LDX"), MC6809DASM::IDX },
{ 0xAF, _T("STX"), MC6809DASM::IDX },

{ 0xB0, _T("SUBA"), MC6809DASM::EXT },
{ 0xB1, _T("CMPA"), MC6809DASM::EXT },
{ 0xB2, _T("SBCA"), MC6809DASM::EXT },
{ 0xB3, _T("SUBD"), MC6809DASM::EXT },
{ 0xB4, _T("ANDA"), MC6809DASM::EXT },
{ 0xB5, _T("BITA"), MC6809DASM::EXT },
{ 0xB6, _T("LDA"), MC6809DASM::EXT },
{ 0xB7, _T("STA"), MC6809DASM::EXT },
{ 0xB8, _T("EORA"), MC6809DASM::EXT },
{ 0xB9, _T("ADCA"), MC6809DASM::EXT },
{ 0xBA, _T("ORA"), MC6809DASM::EXT },
{ 0xBB, _T("ADDA"), MC6809DASM::EXT },
{ 0xBC, _T("CMPX"), MC6809DASM::EXT },
{ 0xBD, _T("JSR"), MC6809DASM::JMP },
{ 0xBE, _T("LDX"), MC6809DASM::EXT },
{ 0xBF, _T("STX"), MC6809DASM::EXT },

{ 0xC0, _T("SUBB"), MC6809DASM::IMM1 },
{ 0xC1, _T("CMPB"), MC6809DASM::IMM1 },
{ 0xC2, _T("SBCB"), MC6809DASM::IMM1 },
{ 0xC3, _T("ADDD"), MC6809DASM::IMM2 },
{ 0xC4, _T("ANDB"), MC6809DASM::IMM1 },
{ 0xC5, _T("BITB"), MC6809DASM::IMM1 },
{ 0xC6, _T("LDB"), MC6809DASM::IMM1 },
{ 0xC7, _T("STB??"), MC6809DASM::IMM1 },
{ 0xC8, _T("EORB"), MC6809DASM::IMM1 },
{ 0xC9, _T("ADCB"), MC6809DASM::IMM1 },
{ 0xCA, _T("ORB"), MC6809DASM::IMM1 },
{ 0xCB, _T("ADDB"), MC6809DASM::IMM1 },
{ 0xCC, _T("LDD"), MC6809DASM::IMM2 },
{ 0xCD, _T("HCF??"), MC6809DASM::INH },
{ 0xCE, _T("LDU"), MC6809DASM::IMM2 },
{ 0xCF, _T("STU??"), MC6809DASM::IMM2 },

{ 0xD0, _T("SUBB"), MC6809DASM::DIR1 },
{ 0xD1, _T("CMPB"), MC6809DASM::DIR1 },
{ 0xD2, _T("SBCB"), MC6809DASM::DIR1 },
{ 0xD3, _T("ADDD"), MC6809DASM::DIR1 },
{ 0xD4, _T("ANDB"), MC6809DASM::DIR1 },
{ 0xD5, _T("BITB"), MC6809DASM::DIR1 },
{ 0xD6, _T("LDB"), MC6809DASM::DIR1 },
{ 0xD7, _T("STB"), MC6809DASM::DIR1 },
{ 0xD8, _T("EORB"), MC6809DASM::DIR1 },
{ 0xD9, _T("ADCB"), MC6809DASM::DIR1 },
{ 0xDA, _T("ORB"), MC6809DASM::DIR1 },
{ 0xDB, _T("ADDB"), MC6809DASM::DIR1 },
{ 0xDC, _T("LDD"), MC6809DASM::DIR1 },
{ 0xDD, _T("STD"), MC6809DASM::DIR1 },
{ 0xDE, _T("LDU"), MC6809DASM::DIR1 },
{ 0xDF, _T("STU"), MC6809DASM::DIR1 },

{ 0xE0, _T("SUBB"), MC6809DASM::IDX },
{ 0xE1, _T("CMPB"), MC6809DASM::IDX },
{ 0xE2, _T("SBCB"), MC6809DASM::IDX },
{ 0xE3, _T("ADDD"), MC6809DASM::IDX },
{ 0xE4, _T("ANDB"), MC6809DASM::IDX },
{ 0xE5, _T("BITB"), MC6809DASM::IDX },
{ 0xE6, _T("LDB"), MC6809DASM::IDX },
{ 0xE7, _T("STB"), MC6809DASM::IDX },
{ 0xE8, _T("EORB"), MC6809DASM::IDX },
{ 0xE9, _T("ADCB"), MC6809DASM::IDX },
{ 0xEA, _T("ORB"), MC6809DASM::IDX },
{ 0xEB, _T("ADDB"), MC6809DASM::IDX },
{ 0xEC, _T("LDD"), MC6809DASM::IDX },
{ 0xED, _T("STD"), MC6809DASM::IDX },
{ 0xEE, _T("LDU"), MC6809DASM::IDX },
{ 0xEF, _T("STU"), MC6809DASM::IDX },

{ 0xF0, _T("SUBB"), MC6809DASM::EXT },
{ 0xF1, _T("CMPB"), MC6809DASM::EXT },
{ 0xF2, _T("SBCB"), MC6809DASM::EXT },
{ 0xF3, _T("ADDD"), MC6809DASM::EXT },
{ 0xF4, _T("ANDB"), MC6809DASM::EXT },
{ 0xF5, _T("BITB"), MC6809DASM::EXT },
{ 0xF6, _T("LDB"), MC6809DASM::EXT },
{ 0xF7, _T("STB"), MC6809DASM::EXT },
{ 0xF8, _T("EORB"), MC6809DASM::EXT },
{ 0xF9, _T("ADCB"), MC6809DASM::EXT },
{ 0xFA, _T("ORB"), MC6809DASM::EXT },
{ 0xFB, _T("ADDB"), MC6809DASM::EXT },
{ 0xFC, _T("LDD"), MC6809DASM::EXT },
{ 0xFD, _T("STD"), MC6809DASM::EXT },
{ 0xFE, _T("LDU"), MC6809DASM::EXT },
{ 0xFF, _T("STU"), MC6809DASM::EXT },
{ 0x00, NULL, MC6809DASM::NONE }
};

// ex1 opcodes (0x10 0x..)
static const MC6809DASM::opcode_t opcodes1[] =
{
{ 0x21, _T("LBRN"), MC6809DASM::REL2 },
{ 0x22, _T("LBHI"), MC6809DASM::REL2 },
{ 0x23, _T("LBLS"), MC6809DASM::REL2 },
{ 0x24, _T("LBCC"), MC6809DASM::REL2 },
{ 0x25, _T("LBCS"), MC6809DASM::REL2 },
{ 0x26, _T("LBNE"), MC6809DASM::REL2 },
{ 0x27, _T("LBEQ"), MC6809DASM::REL2 },
{ 0x28, _T("LBVC"), MC6809DASM::REL2 },
{ 0x29, _T("LBVS"), MC6809DASM::REL2 },
{ 0x2A, _T("LBPL"), MC6809DASM::REL2 },
{ 0x2B, _T("LBMI"), MC6809DASM::REL2 },
{ 0x2C, _T("LBGE"), MC6809DASM::REL2 },
{ 0x2D, _T("LBLT"), MC6809DASM::REL2 },
{ 0x2E, _T("LBGT"), MC6809DASM::REL2 },
{ 0x2F, _T("LBLE"), MC6809DASM::REL2 },
{ 0x3F, _T("SWI2"), MC6809DASM::SWI2 },
{ 0x83, _T("CMPD"), MC6809DASM::IMM2 },
{ 0x8C, _T("CMPY"), MC6809DASM::IMM2 },
{ 0x8E, _T("LDY"), MC6809DASM::IMM2 },
{ 0x8F, _T("STY??"), MC6809DASM::IMM2 },
{ 0x93, _T("CMPD"), MC6809DASM::DIR1 },
{ 0x9C, _T("CMPY"), MC6809DASM::DIR1 },
{ 0x9E, _T("LDY"), MC6809DASM::DIR1 },
{ 0x9F, _T("STY"), MC6809DASM::DIR1 },
{ 0xA3, _T("CMPD"), MC6809DASM::IDX },
{ 0xAC, _T("CMPY"), MC6809DASM::IDX },
{ 0xAE, _T("LDY"), MC6809DASM::IDX },
{ 0xAF, _T("STY"), MC6809DASM::IDX },
{ 0xB3, _T("CMPD"), MC6809DASM::EXT },
{ 0xBC, _T("CMPY"), MC6809DASM::EXT },
{ 0xBE, _T("LDY"), MC6809DASM::EXT },
{ 0xBF, _T("STY"), MC6809DASM::EXT },
{ 0xCE, _T("LDS"), MC6809DASM::IMM2 },
{ 0xCF, _T("STS??"), MC6809DASM::IMM2 },
{ 0xDE, _T("LDS"), MC6809DASM::DIR1 },
{ 0xDF, _T("STS"), MC6809DASM::DIR1 },
{ 0xEE, _T("LDS"), MC6809DASM::IDX },
{ 0xEF, _T("STS"), MC6809DASM::IDX },
{ 0xFE, _T("LDS"), MC6809DASM::EXT },
{ 0xFF, _T("STS"), MC6809DASM::EXT },
{ 0x00, NULL, MC6809DASM::NONE }
};

// ex2 opcodes (0x11 0x..)
static const MC6809DASM::opcode_t opcodes2[] =
{
{ 0x3F, _T("SWI3"), MC6809DASM::INH },
{ 0x83, _T("CMPU"), MC6809DASM::IMM2 },
{ 0x8C, _T("CMPS"), MC6809DASM::IMM2 },
{ 0x93, _T("CMPU"), MC6809DASM::DIR1 },
{ 0x9C, _T("CMPS"), MC6809DASM::DIR1 },
{ 0xA3, _T("CMPU"), MC6809DASM::IDX },
{ 0xAC, _T("CMPS"), MC6809DASM::IDX },
{ 0xB3, _T("CMPU"), MC6809DASM::EXT },
{ 0xBC, _T("CMPS"), MC6809DASM::EXT },
{ 0x00, NULL, MC6809DASM::NONE }
};

MC6809DASM::MC6809DASM()
{
	d_mem = NULL;

	// register stack
	memset(regs, 0, sizeof(regs));
	current_reg = &regs[0];
	current_idx = 0;
	codelen = 0;

	swi2_appear = ~0;
}

MC6809DASM::~MC6809DASM()
{
}

void MC6809DASM::ini_pc(uint16_t pc, uint8_t code)
{
	uint32_t inphyaddr = 0;
	if (d_mem) {
		inphyaddr = d_mem->debug_latch_address(pc);
	}
	push_stack_pc(inphyaddr, pc, code);
}

void MC6809DASM::push_stack_pc(uint32_t phyaddr, uint16_t pc, uint8_t code)
{
	mc6809dasm_regs_t *prev_reg = current_reg;

	current_idx = (current_idx + 1) % MC6809DASM_PCSTACK_NUM;

	current_reg = &regs[current_idx];

	current_reg->phyaddr = phyaddr;
	current_reg->pc      = pc;
	current_reg->code[0] = code;
	current_reg->state   = (prev_reg->state & 0xffff);
	memcpy(current_reg->int_flags, prev_reg->int_flags, sizeof(current_reg->int_flags));

	codelen = 1;

	current_reg->flags = 1;
}

int MC6809DASM::get_stack_pc(int index, mc6809dasm_regs_t *stack)
{
	if (index < 0) return -2;
	if (index >= MC6809DASM_PCSTACK_NUM) index = MC6809DASM_PCSTACK_NUM - 1;

	int reg_index = (current_idx - index + MC6809DASM_PCSTACK_NUM) % MC6809DASM_PCSTACK_NUM;
	bool exist = false;
	do {
		if (regs[reg_index].flags & 1) {
			exist = true;
			break;
		} 
		reg_index++;
		if (reg_index >= MC6809DASM_PCSTACK_NUM) reg_index = 0;
	} while(reg_index != current_idx);
	if (!exist) return -2;

	if (stack) *stack = regs[reg_index];

	index--;
	return index;
}

void MC6809DASM::set_mem(uint16_t addr, uint8_t data, bool write)
{
	if (d_mem) {
		current_reg->rw_phyaddr = d_mem->debug_latch_address(addr);
	}
	current_reg->rw_addr = addr;
	current_reg->rw_data = data;
	current_reg->flags &= 0xf1;
	current_reg->flags |= 2;
	if (write) current_reg->flags |= 4;
}

void MC6809DASM::set_mem(uint16_t addr, uint16_t data, bool write)
{
	if (d_mem) {
		current_reg->rw_phyaddr = d_mem->debug_latch_address(addr);
	}
	current_reg->rw_addr = addr;
	current_reg->rw_data = data;
	current_reg->flags &= 0xf1;
	current_reg->flags |= 0xa;
	if (write) current_reg->flags |= 4;
}

void MC6809DASM::set_phymem(uint32_t addr, uint8_t data, bool write)
{
	current_reg->rw_phyaddr = addr;
}

void MC6809DASM::set_code2(uint8_t code2)
{
	current_reg->code[codelen] = code2;
	codelen++;
}

void MC6809DASM::set_swi2_param(uint8_t cat, uint8_t idx)
{
	current_reg->code[codelen] = cat;
	codelen++;
	current_reg->code[codelen] = idx;
	codelen++;
}

void MC6809DASM::set_inh()
{
}

void MC6809DASM::set_swi()
{
}

void MC6809DASM::set_swi2()
{
}

void MC6809DASM::set_swi3()
{
}

void MC6809DASM::set_dir_addr(uint8_t dp, uint8_t addrl)
{
	current_reg->code[codelen] = addrl;
	codelen++;
	current_reg->dp = dp;
}

void MC6809DASM::set_dir(uint8_t dp, uint8_t addrl, uint8_t data, bool write)
{
	set_dir_addr(dp, addrl);
	set_mem((dp << 8) | addrl, data, write);
}

void MC6809DASM::set_dir(uint8_t dp, uint8_t addrl, uint16_t data, bool write)
{
	set_dir_addr(dp, addrl);
	set_mem((dp << 8) | addrl, data, write);
}

void MC6809DASM::set_imm(uint8_t data)
{
	current_reg->code[codelen] = data;
	codelen++;
}

void MC6809DASM::set_imm(uint16_t data)
{
	current_reg->code[codelen] = ((data & 0xff00) >> 8);
	codelen++;
	current_reg->code[codelen] = (data & 0xff);
	codelen++;
}

void MC6809DASM::set_ext_addr(uint16_t addr)
{
	current_reg->code[codelen] = ((addr & 0xff00) >> 8);
	codelen++;
	current_reg->code[codelen] = (addr & 0xff);
	codelen++;
}

void MC6809DASM::set_ext(uint16_t addr, uint8_t data, bool write)
{
	set_ext_addr(addr);
	set_mem(addr, data, write);
}

void MC6809DASM::set_ext(uint16_t addr, uint16_t data, bool write)
{
	set_ext_addr(addr);
	set_mem(addr, data, write);
}

void MC6809DASM::set_idx1(uint8_t data)
{
	current_reg->code[codelen] = data;
	codelen++;
}

void MC6809DASM::set_idx2(uint8_t data)
{
	current_reg->code[codelen] = data;
	codelen++;
}

void MC6809DASM::set_idx2(uint16_t data)
{
	current_reg->code[codelen] = ((data & 0xff00) >> 8);
	codelen++;
	current_reg->code[codelen] = (data & 0xff);
	codelen++;
}

void MC6809DASM::set_rel(uint8_t data)
{
	current_reg->code[codelen] = data;
	codelen++;
}

void MC6809DASM::set_rel(uint16_t data)
{
	current_reg->code[codelen] = ((data & 0xff00) >> 8);
	codelen++;
	current_reg->code[codelen] = (data & 0xff);
	codelen++;
}

void MC6809DASM::set_jmp(uint16_t data)
{
	current_reg->code[codelen] = ((data & 0xff00) >> 8);
	codelen++;
	current_reg->code[codelen] = (data & 0xff);
	codelen++;
}

void MC6809DASM::set_tfr(uint8_t data)
{
	current_reg->code[codelen] = data;
	codelen++;
}

void MC6809DASM::set_psh(uint8_t data)
{
	current_reg->code[codelen] = data;
	codelen++;
}

void MC6809DASM::set_err()
{
}

void MC6809DASM::set_regs(int accum, int cycles, uint8_t cc, uint8_t dp, uint8_t a, uint8_t b, uint16_t x, uint16_t y, uint16_t s, uint16_t u)
{
//	start_accum = accum;
//	expended_cycles = cycles;
	current_reg->cycles = cycles;
	current_reg->cc = cc;
	current_reg->dp = dp;
	current_reg->a = a;
	current_reg->b = b;
	current_reg->x = x;
	current_reg->y = y;
	current_reg->s = s;
	current_reg->u = u;
}

/// @param [in] phy_addr: physical address converted from pc
/// @param [in] pc      : program counter
/// @param [in] ops     : op code
/// @param [in] opslen  : op code length
/// @param [in] flags   : bit5: 1=calc rel addr using phy_addr  bit4: 1=not use dp
/// @param [in] dp      : direct pointer
int MC6809DASM::print_dasm(uint32_t phy_addr, uint16_t pc, const uint8_t *ops, int opslen, int flags, uint8_t *dp)
{
	en_opmode mode = NONE;
	int opspos = set_cmd_str(phy_addr, ops, &mode);
	uint32_t opdata;
	switch(mode) {
	case INH:
		break;
	case DIR1:
		if (flags & 0x10) {
			UTILITY::sntprintf(cmd, MC6809DASM_CMDLINE_LEN, _T(" <$%02X")
				, ops[opspos]
			);
		} else {
			opdata = (uint32_t)(dp ? *dp : current_reg->dp) << 8 | ops[opspos];
			debugger->cat_value_or_symbol(cmd, MC6809DASM_CMDLINE_LEN, _T(" "), _T("$%04X")
				, opdata
			);
		}
		opspos++;
		break;
	case EXT:
		opdata = (uint32_t)ops[opspos] << 8 | ops[opspos+1];
		debugger->cat_value_or_symbol(cmd, MC6809DASM_CMDLINE_LEN, _T(" "), _T("$%04X")
			, opdata
		);
		opspos+=2;
		break;
	case IMM1:
		UTILITY::sntprintf(cmd, MC6809DASM_CMDLINE_LEN, _T(" #$%02X")
			, ops[opspos]
		);
		opspos++;
		break;
	case IMM2:
		opdata = (uint32_t)ops[opspos] << 8 | ops[opspos+1];
		UTILITY::sntprintf(cmd, MC6809DASM_CMDLINE_LEN, _T(" #$%04X")
			, opdata
		);
		opspos+=2;
		break;
	case REL1:
		opdata = ops[opspos];
		opdata = ((flags & 0x20 ? phy_addr & 0xffff : pc) + opspos + 1 + (int8_t)opdata) & 0xffff;
		debugger->cat_value_or_symbol(cmd, MC6809DASM_CMDLINE_LEN, _T(" "), _T("$%04X")
			, opdata 
		);
		opspos++;
		break;
	case REL2:
		opdata = (uint32_t)ops[opspos] << 8 | ops[opspos+1];
		opdata = ((flags & 0x20 ? phy_addr & 0xffff : pc) + opspos + 2 + (int16_t)opdata) & 0xffff;
		debugger->cat_value_or_symbol(cmd, MC6809DASM_CMDLINE_LEN, _T(" "), _T("$%04X")
			, opdata
		);
		opspos+=2;
		break;
	case JMP:
		opdata = (uint32_t)ops[opspos] << 8 | ops[opspos+1];
		debugger->cat_value_or_symbol(cmd, MC6809DASM_CMDLINE_LEN, _T(" "), _T("$%04X")
			, opdata
		);
		opspos+=2;
		break;
	case IDX:
		opspos = set_idx_str(phy_addr, pc, ops, opspos, flags);
		break;
	case TFR:
		opspos = set_tfr_str(ops, opspos);
		break;
	case PSH:
		opspos = set_psh_str(ops, opspos);
		break;
	case SWI2:
#ifdef _MBS1
		opspos = set_swi2_str(phy_addr, ops, opspos);
#endif
		break;
	case FDB:
		opdata = (uint32_t)ops[opspos] << 8 | ops[opspos+1];
		UTILITY::sntprintf(cmd, MC6809DASM_CMDLINE_LEN, _T(" $%04X")
			,opdata
		);
		opspos+=2;
		break;
	default:
		break;
	}

#ifdef _MBS1
	UTILITY::stprintf(line, sizeof(line) / sizeof(line[0]), _T("%05X %04X"), phy_addr, pc);
#else
	UTILITY::stprintf(line, sizeof(line) / sizeof(line[0]), _T("%04X"), pc);
#endif
	for(int i=0; i < opslen; i++) {
		if (i < opspos) {
			UTILITY::sntprintf(line, sizeof(line) / sizeof(line[0]), _T(" %02X"), ops[i]);
		} else {
			UTILITY::tcscat(line, sizeof(line) / sizeof(line[0]), _T("   "));
		}
	}

	UTILITY::sntprintf(line, sizeof(line) / sizeof(line[0]), _T(" %-18s"), cmd);

	return opspos;
}

int MC6809DASM::print_dasm_label(int type, uint32_t pc)
{
	const _TCHAR *label = debugger->find_symbol_name(pc);
	if (label) {
#ifdef _MBS1
		UTILITY::tcscpy(line, sizeof(line) / sizeof(line[0]), _T("                    "));
#else
		UTILITY::tcscpy(line, sizeof(line) / sizeof(line[0]), _T("              "));
#endif
		UTILITY::tcscat(line, sizeof(line) / sizeof(line[0]), label);
		return 1;
	}
	return 0;
}

int MC6809DASM::print_dasm_preprocess(int type, uint32_t pc, int flags)
{
	// next opcode
	uint8_t ops[5];
	uint32_t phy_addr = 0;
#ifdef _MBS1
	if (type >= 0) {
		phy_addr = pc;
		if (type == 0) {
			pc = d_mem->debug_address_mapping_rev(pc);
		}
		for(int i = 0; i < 5; i++) {
			ops[i] = d_mem->debug_read_data8(type, phy_addr + i);
		}
	} else {
		phy_addr = d_mem->debug_latch_address(pc);
		for(int i = 0; i < 5; i++) {
			ops[i] = d_mem->debug_read_data8(-1, pc + i);
		}
	}
	if ((pc & 0xffff0000) == 0xffff0000) {
		// no mapping
		flags |= 0x20;
	}
#else
	phy_addr = pc;
	for(int i = 0; i < 5; i++) {
		ops[i] = d_mem->debug_read_data8(type, pc + i);
	}
#endif
	line[0]=0;
	int opspos = print_dasm(phy_addr, pc & 0xffff, ops, 5, flags, NULL);
	return opspos;
}

int MC6809DASM::print_dasm_processed(uint16_t pc)
{
	line[0]=0;
	int opspos = print_dasm(current_reg->phyaddr, current_reg->pc, current_reg->code, 5, 0, NULL);
	print_cycles(current_reg->cycles);
	print_memory_data(regs[current_idx]);
	return opspos;
}

int MC6809DASM::print_dasm_traceback(int index)
{
	mc6809dasm_regs_t stack;
	int next = get_stack_pc(index, &stack);
	if (next >= -1) {
		line[0]=0;
		print_dasm(stack.phyaddr, stack.pc & 0xffff, stack.code, 5, 0, &stack.dp);
		print_cycles(stack.cycles);
		print_memory_data(stack);
	}
	return next;
}

void MC6809DASM::print_cycles(int cycles)
{
	// expended machine cycles 
	UTILITY::sntprintf(line, _MAX_PATH, _T(" (%2d)")
		,cycles
	);
}

void MC6809DASM::print_regs(const mc6809dasm_regs_t &regs)
{
	// register
	UTILITY::stprintf(line, _MAX_PATH,
		 _T(
		 "DP:%02X A:%02X B:%02X X:%04X Y:%04X U:%04X S:%04X CC:[%c%c%c%c%c%c%c%c]"
		 )
		,regs.dp
		,regs.a, regs.b
		,regs.x, regs.y, regs.u, regs.s
		,(regs.cc & CC_E) ? _T('E') : _T('-')
		,(regs.cc & CC_IF) ? _T('F') : _T('-')
		,(regs.cc & CC_H) ? _T('H') : _T('-')
		,(regs.cc & CC_II) ? _T('I') : _T('-')
		,(regs.cc & CC_N) ? _T('N') : _T('-')
		,(regs.cc & CC_Z) ? _T('Z') : _T('-')
		,(regs.cc & CC_V) ? _T('V') : _T('-')
		,(regs.cc & CC_C) ? _T('C') : _T('-')
	 );
	size_t len = _tcslen(line);
	if (regs.state & (MC6809_HALT_BIT | MC6809_INSN_HALT | MC6809_HALT_REL)) {
		UTILITY::stprintf(&line[len], sizeof(line) / sizeof(line[0]) - len, _T(" HALT(%04X)"), regs.int_flags[MC6809_IDX_HALT]);
		len = _tcslen(line);
	}
	if (regs.state & MC6809_RESET_BIT) {
		UTILITY::stprintf(&line[len], sizeof(line) / sizeof(line[0]) - len, _T(" RESET(%04X)"), regs.int_flags[MC6809_IDX_RESET]);
		len = _tcslen(line);
	}
	if (regs.state & MC6809_NMI_BIT) {
		UTILITY::stprintf(&line[len], sizeof(line) / sizeof(line[0]) - len, _T(" NMI(%04X)"), regs.int_flags[MC6809_IDX_NMI]);
		len = _tcslen(line);
	}
	if (regs.state & MC6809_IRQ_BIT) {
		UTILITY::stprintf(&line[len], sizeof(line) / sizeof(line[0]) - len, _T(" IRQ(%04X)"), regs.int_flags[MC6809_IDX_IRQ]);
		len = _tcslen(line);
	}
	if (regs.state & MC6809_FIRQ_BIT) {
		UTILITY::stprintf(&line[len], sizeof(line) / sizeof(line[0]) - len, _T(" FIRQ(%04X)"), regs.int_flags[MC6809_IDX_FIRQ]);
		len = _tcslen(line);
	}
}

void MC6809DASM::print_regs_current()
{
	print_regs(*current_reg);
}

void MC6809DASM::print_regs_current(uint16_t pc, uint8_t cc, uint8_t dp, uint8_t a, uint8_t b, uint16_t x, uint16_t y, uint16_t s, uint16_t u, uint32_t state, uint32_t *int_flags)
{
	mc6809dasm_regs_t reg;
	reg.pc = pc;
	reg.cc = cc;
	reg.dp = dp;
	reg.a = a;
	reg.b = b;
	reg.x = x;
	reg.y = y;
	reg.s = s;
	reg.u = u;
	reg.state = (state & 0xffff);
	memcpy(&reg.int_flags, int_flags, sizeof(reg.int_flags));

	print_regs(reg);
}

int MC6809DASM::print_regs_traceback(int index)
{
	mc6809dasm_regs_t stack;
	int next = get_stack_pc(index, &stack);
	if (next >= -1) {
		print_regs(stack);
	}
	return next;
}

void MC6809DASM::print_memory_data(mc6809dasm_regs_t &stack)
{
	// memory data
	if ((stack.flags & 2) == 0) return;

	if ((stack.flags & 8) == 0) {
		UTILITY::sntprintf(line, _MAX_PATH
#ifdef _MBS1
			, _T(" (%c %05X %04X:%02X)")
			, (stack.flags & 4) ? _T('W') : _T('R')
			, stack.rw_phyaddr
#else
			, _T(" (%c %04X:%02X)")
			, (stack.flags & 4) ? _T('W') : _T('R')
#endif
			, stack.rw_addr, stack.rw_data & 0xff);
	} else {
		UTILITY::sntprintf(line, _MAX_PATH
#ifdef _MBS1
			, _T(" (%c %05X %04X:%04X)")
			, (stack.flags & 4) ? _T('W') : _T('R')
			, stack.rw_phyaddr
#else
			, _T(" (%c %04X:%04X)")
			, (stack.flags & 4) ? _T('W') : _T('R')
#endif
			, stack.rw_addr, stack.rw_data);
	}
}

void MC6809DASM::set_signal(uint32_t state, int int_flags_idx, uint32_t int_flags)
{
	if ((current_reg->state & (MC6809_HALT_BIT | MC6809_INSN_HALT)) != 0 && (state & (MC6809_HALT_BIT | MC6809_INSN_HALT)) == 0) {
		// release HALT
		state |= ((current_reg->state & 0xffff0000) | MC6809_HALT_REL);
	} else {
		current_reg->int_flags[int_flags_idx] = int_flags;
	}
	current_reg->state = state;
}

int MC6809DASM::set_cmd_str(uint32_t addr, const uint8_t *ops, en_opmode *mod)
{
	uint8_t ch = ops[0];
	uint8_t cl = ops[1];
	int oplen = 0;

	memset(cmd, 0, sizeof(cmd));

	if (swi2_appear == addr) {
		// system call for S1
		if ((ch & 0xf0) == 0x40) {
			UTILITY::stprintf(cmd, MC6809DASM_CMDLINE_LEN, _T("%-5s"), _T("FDB"));
			if (mod) *mod = FDB;
			return oplen;
		}
	}
	swi2_appear = ~0;

	if (ch == 0x10) {
		for(int i=0; opcodes1[i].s != NULL; i++) {
			if (cl == opcodes1[i].c) {
				UTILITY::stprintf(cmd, MC6809DASM_CMDLINE_LEN, _T("%-5s"), opcodes1[i].s);
				if (mod) *mod = opcodes1[i].m;
				oplen = 2;
				break;
			}
		}
		if (oplen != 2) {
			UTILITY::stprintf(cmd, MC6809DASM_CMDLINE_LEN, _T("%-5s"), _T("???"));
			oplen = 1;
		}
	} else if (ch == 0x11) {
		for(int i=0; opcodes2[i].s != NULL; i++) {
			if (cl == opcodes2[i].c) {
				UTILITY::stprintf(cmd, MC6809DASM_CMDLINE_LEN, _T("%-5s"), opcodes2[i].s);
				if (mod) *mod = opcodes2[i].m;
				oplen = 2;
				break;
			}
		}
		if (oplen != 2) {
			UTILITY::stprintf(cmd, MC6809DASM_CMDLINE_LEN, _T("%-5s"), _T("???"));
			oplen = 1;
		}
	} else {
		UTILITY::stprintf(cmd, MC6809DASM_CMDLINE_LEN, _T("%-5s"), opcodes0[ch].s);
		if (mod) *mod = opcodes0[ch].m;
		oplen = 1;
	}
	return oplen;
}

int MC6809DASM::set_idx_str(uint32_t phyaddr, uint16_t pc, const uint8_t *ops, int opspos, int flags)
{
	_TCHAR reg = _T('?');

	uint8_t postbyte = ops[opspos];
	uint8_t pdata1 = ops[opspos+1];
	uint16_t pdata2 = ((ops[opspos+1] << 8) | ops[opspos+2]);

	switch(postbyte & 0x60) {
	case 0x00: reg = _T('X'); break;
	case 0x20: reg = _T('Y'); break;
	case 0x40: reg = _T('U'); break;
	case 0x60: reg = _T('S'); break;
	}
	if ((postbyte & 0x80) == 0) {
		// 0x00 - 0x7f
		UTILITY::sntprintf(cmd, MC6809DASM_CMDLINE_LEN, _T(" %d,%c")
		, ((int)postbyte & 0xf) - ((postbyte & 0x10) ? 16 : 0)
		, reg);
		opspos++;
	} else if ((postbyte & 0x8f) == 0x80) {
		if ((postbyte & 0x10) == 0) {
			UTILITY::sntprintf(cmd, MC6809DASM_CMDLINE_LEN, _T(" ,%c+")
			, reg);
		} else {
			// illegal
			UTILITY::sntprintf(cmd, MC6809DASM_CMDLINE_LEN, _T(" [,%c+] ???")
			, reg);
		}
		opspos++;
	} else if ((postbyte & 0x8f) == 0x81) {
		if ((postbyte & 0x10) == 0) {
			UTILITY::sntprintf(cmd, MC6809DASM_CMDLINE_LEN, _T(" ,%c++")
			, reg);
		} else {
			UTILITY::sntprintf(cmd, MC6809DASM_CMDLINE_LEN, _T(" [,%c++]")
			, reg);
		}
		opspos++;
	} else if ((postbyte & 0x8f) == 0x82) {
		if ((postbyte & 0x10) == 0) {
			UTILITY::sntprintf(cmd, MC6809DASM_CMDLINE_LEN, _T(" ,-%c")
			, reg);
		} else {
			// illegal
			UTILITY::sntprintf(cmd, MC6809DASM_CMDLINE_LEN, _T(" [,-%c] ???")
			, reg);
		}
		opspos++;
	} else if ((postbyte & 0x8f) == 0x83) {
		if ((postbyte & 0x10) == 0) {
			UTILITY::sntprintf(cmd, MC6809DASM_CMDLINE_LEN, _T(" ,--%c")
			, reg);
		} else {
			UTILITY::sntprintf(cmd, MC6809DASM_CMDLINE_LEN, _T(" [,--%c]")
			, reg);
		}
		opspos++;
	} else if ((postbyte & 0x8f) == 0x84) {
		if ((postbyte & 0x10) == 0) {
			UTILITY::sntprintf(cmd, MC6809DASM_CMDLINE_LEN, _T(" ,%c")
			, reg);
		} else {
			UTILITY::sntprintf(cmd, MC6809DASM_CMDLINE_LEN, _T(" [,%c]")
			, reg);
		}
		opspos++;
	} else if ((postbyte & 0x8f) == 0x85) {
		if ((postbyte & 0x10) == 0) {
			UTILITY::sntprintf(cmd, MC6809DASM_CMDLINE_LEN, _T(" B,%c")
			, reg);
		} else {
			UTILITY::sntprintf(cmd, MC6809DASM_CMDLINE_LEN, _T(" [B,%c]")
			, reg);
		}
		opspos++;
	} else if ((postbyte & 0x8f) == 0x86) {
		if ((postbyte & 0x10) == 0) {
			UTILITY::sntprintf(cmd, MC6809DASM_CMDLINE_LEN, _T(" A,%c")
			, reg);
		} else {
			UTILITY::sntprintf(cmd, MC6809DASM_CMDLINE_LEN, _T(" [A,%c]")
			, reg);
		}
		opspos++;
	} else if ((postbyte & 0x8f) == 0x87) {
		// illegal
		UTILITY::sntprintf(cmd, MC6809DASM_CMDLINE_LEN, _T(" ,???"));
		opspos++;
	} else if ((postbyte & 0x8f) == 0x88) {
		// 8bit offset
		if ((postbyte & 0x10) == 0) {
			UTILITY::sntprintf(cmd, MC6809DASM_CMDLINE_LEN, _T(" %d,%c")
			, (int)pdata1 - ((pdata1 & 0x80) ? 256 : 0)
			, reg);
		} else {
			UTILITY::sntprintf(cmd, MC6809DASM_CMDLINE_LEN, _T(" [%d,%c]")
			, (int)pdata1 - ((pdata1 & 0x80) ? 256 : 0)
			, reg);
		}
		opspos+=2;
	} else if ((postbyte & 0x8f) == 0x89) {
		// 16bit offset
		if ((postbyte & 0x10) == 0) {
			UTILITY::sntprintf(cmd, MC6809DASM_CMDLINE_LEN, _T(" %d,%c")
			, (int)pdata2 - ((pdata2 & 0x8000) ? 65536 : 0)
			, reg);
		} else {
			UTILITY::sntprintf(cmd, MC6809DASM_CMDLINE_LEN, _T(" [%d,%c]")
			, (int)pdata2 - ((pdata2 & 0x8000) ? 65536 : 0)
			, reg);
		}
		opspos+=3;
	} else if ((postbyte & 0x8f) == 0x8a) {
		// illegal
		UTILITY::sntprintf(cmd, MC6809DASM_CMDLINE_LEN, _T(" ,???"));
		opspos++;
	} else if ((postbyte & 0x8f) == 0x8b) {
		if ((postbyte & 0x10) == 0) {
			UTILITY::sntprintf(cmd, MC6809DASM_CMDLINE_LEN, _T(" D,%c")
			, reg);
		} else {
			UTILITY::sntprintf(cmd, MC6809DASM_CMDLINE_LEN, _T(" [D,%c]")
			, reg);
		}
		opspos++;
	} else if ((postbyte & 0x8f) == 0x8c) {
		// 8bit offset PCR
		int ofs = (opspos + 2 + ((flags & 0x20 ? (phyaddr & 0xffff) : pc) + (int8_t)pdata1)) & 0xffff;
		if ((postbyte & 0x10) == 0) {
			UTILITY::sntprintf(cmd, MC6809DASM_CMDLINE_LEN, _T(" $%04X,PCR")
			, ofs
			);
		} else {
			UTILITY::sntprintf(cmd, MC6809DASM_CMDLINE_LEN, _T(" [$%04X,PCR]")
			, ofs
			);
		}
		opspos+=2;
	} else if ((postbyte & 0x8f) == 0x8d) {
		// 16bit offset PCR
		int ofs = (opspos + 3 + ((flags & 0x20 ? (phyaddr & 0xffff) : pc) + (int16_t)pdata2)) & 0xffff;
		if ((postbyte & 0x10) == 0) {
			UTILITY::sntprintf(cmd, MC6809DASM_CMDLINE_LEN, _T(" $%04X,PCR")
			, ofs
			);
		} else {
			UTILITY::sntprintf(cmd, MC6809DASM_CMDLINE_LEN, _T(" [$%04X,PCR]")
			, ofs
			);
		}
		opspos+=3;
	} else if ((postbyte & 0x8f) == 0x8e) {
		// illegal
		UTILITY::sntprintf(cmd, MC6809DASM_CMDLINE_LEN, _T(" ,???"));
		opspos++;
	} else if ((postbyte & 0x8f) == 0x8f) {
		if ((postbyte & 0x10) == 0) {
			// illegal
			UTILITY::sntprintf(cmd, MC6809DASM_CMDLINE_LEN, _T(" $%04X ???")
			, pdata2 & 0xffff
			);
		} else {
			// extended indirect
			UTILITY::sntprintf(cmd, MC6809DASM_CMDLINE_LEN, _T(" [$%04X]")
			, pdata2 & 0xffff
			);
		}
		opspos+=3;
	}
	return opspos;
}

static const _TCHAR *tfr_regs[] = {
	_T("D"),
	_T("X"),
	_T("Y"),
	_T("U"),
	_T("S"),
	_T("PC"),
	_T("?"),
	_T("?"),

	_T("A"),
	_T("B"),
	_T("CC"),
	_T("DP"),
	_T("?"),
	_T("?"),
	_T("?"),
	_T("?"),
	NULL
};

int MC6809DASM::set_tfr_str(const uint8_t *ops, int opspos)
{
	uint8_t dh = ((ops[opspos] & 0xf0) >> 4);
	uint8_t dl = (ops[opspos] & 0xf);

	UTILITY::sntprintf(cmd, MC6809DASM_CMDLINE_LEN, _T(" %s,%s")
		, tfr_regs[dh], tfr_regs[dl]
	);
	opspos++;
	return opspos;
}

static const _TCHAR *psh_regs[] = {
	_T("CC"),
	_T("A"),
	_T("B"),
	_T("DP"),
	_T("X"),
	_T("Y"),
	_T("S/U"),
	_T("PC"),
	NULL
};

int MC6809DASM::set_psh_str(const uint8_t *ops, int opspos)
{
	uint8_t data = ops[opspos];
	bool is_s = ((ops[0] & 0xf) < 0x06);
	bool is_psh = ((ops[0] & 1) == 0);
	bool one = true;
	int ist = (is_psh ? 7 : 0);
	int ied = (is_psh ? -1 : 8);
	int isp = (is_psh ? -1 : 1);
	for(int i=ist; i != ied; i+=isp) {
		if ((data & (1 << i)) != 0) {
			UTILITY::sntprintf(cmd, MC6809DASM_CMDLINE_LEN, _T("%c%s")
				, one ? _T(' ') : _T(',')
				, (i == 6 ? (is_s ? _T("U") : _T("S")) : psh_regs[i])
			);
			one = false;
		}
	}
	opspos++;
	return opspos;
}

int MC6809DASM::set_swi2_str(uint32_t addr, const uint8_t *ops, int opspos)
{
	if ((ops[opspos] & 0xf0) == 0x40) {
		// system call for S1
		UTILITY::sntprintf(cmd, MC6809DASM_CMDLINE_LEN, _T("  ($%02X $%02X)")
			,ops[opspos]
			,ops[opspos+1]
		);
		swi2_appear = addr + opspos;
	}
	return opspos;
}

#endif /* USE_DEBUGGER */
