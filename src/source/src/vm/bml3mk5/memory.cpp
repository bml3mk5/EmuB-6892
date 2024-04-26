/** @file memory.cpp

	HITACHI BASIC MASTER LEVEL3 Mark5 Emulator 'EmuB-6892'
	Skelton for retropc emulator

	@author Sasaji
	@date   2011.06.08 -

	@brief [ memory ]
*/

#include "memory.h"
#include <stdlib.h>
#include <time.h>
//#include "../../emu.h"
#include "../vm.h"
#include "../../config.h"
#include "../hd46505.h"
#include "sound.h"
#include "display.h"
#include "comm.h"
#include "cmt.h"
#include "../pia.h"
#include "../via.h"
#include "../../fileio.h"
#include "../../logging.h"
#include "../../utility.h"
#ifdef USE_DEBUGGER
#include "l3basic.h"
#include "../../osd/debugger_console.h"
#endif

//#define _DEBUG_RAM
//#define _DEBUG_CRAM

#define TRACE_PC_IDLE		0xffffffff
#define TRACE_PC_FETCH		0xfffffffe
#define TRACE_PC_WAITING	0xfffffffd
#define TRACE_PC_FIRE		0xfffffffc
#define TRACE_PC_FIRED		0xfffffffb

MEMORY::MEMORY(VM* parent_vm, EMU* parent_emu, const char* identifier)
	: DEVICE(parent_vm, parent_emu, identifier)
{
	set_class_name("MEMORY");

	d_cpu = NULL;

#ifdef USE_DEBUGGER
	dc  = NULL;
	bas = NULL;
#endif
}

MEMORY::~MEMORY()
{
#ifdef USE_DEBUGGER
	delete bas;
#endif
}

// type : w/r
void MEMORY::SET_BANK(int s, int e, uint8_t* w, uint8_t* r, uint8_t type) {
	int sb = (s) >> BANK_SIZE, eb = (e) >> BANK_SIZE;
	for(int i = sb; i <= eb; i++) {
		if((w) == wdmy) {
			wbank[i] = wdmy;
			banktype[i] &= 0x0f;
		} else if ((w) != NULL) {
			wbank[i] = (w) + (1 << BANK_SIZE) * (i - sb);
			banktype[i] = (banktype[i] & 0x0f) | (type & 0xf0);
		}
		if((r) == rdmy) {
			rbank[i] = rdmy;
			banktype[i] &= 0xf0;
		} else if ((r) != NULL) {
			rbank[i] = (r) + (1 << BANK_SIZE) * (i - sb);
			banktype[i] = (banktype[i] & 0xf0) | (type & 0x0f);
		}
	}
}

void MEMORY::initialize()
{
	rom_loaded = 0;
	rom1802_loaded = 0;
	rom1805_loaded = 0;
	rom1806_loaded = 0;
	rom_loaded_at_first = false;

	memset(rom, 0xff, sizeof(rom));
	memset(rdmy, 0xff, sizeof(rdmy));
	memset(rom1802, 0xff, sizeof(rom1802));
	memset(rom1805, 0xff, sizeof(rom1805));
	memset(rom1806, 0xff, sizeof(rom1806));

	// read rom image from file
	load_rom_files();

	// init memory map
	reset_membank();

	// clear trace counter
	trace_counter_id = -1;

	update_config();

#ifdef USE_DEBUGGER
	bas = new L3Basic(this);
#endif
}

void MEMORY::reset()
{
	// read rom image from file
	load_rom_files();

	memset(ram , 0xff, sizeof(ram));
	memset(ram2, 0xff, sizeof(ram2));

	memset(&ram[0xff00], 0xff, 16 * 15);

	// clear color ram
	srand((unsigned int)time(NULL));
	int co = (rand() % 3) * 3 + 1;	// 1 4 7
//	int cg = (rand() % 5);
	memset(color_ram, 0x3f, sizeof(color_ram));
	color_reg = co + (co == 7 ? 24 : 0);

	int cg = (rand() % 5);
	write_data8(0xffd0, (cg == 0 ? co : 0));

	// clear ig ram
	reset_igram();

	// init memory map
	reset_membank();

	// stop trace counter
	reset_tracecounter();

#ifdef USE_DEBUGGER
	if (bas) bas->Reset(0);
#endif
}

void MEMORY::load_rom_files()
{
	// load rom
	const _TCHAR *app_path, *rom_path[2];

	rom_path[0] = pConfig->rom_path.Get();
	rom_path[1] = vm->application_path();

	for(int i=0; i<2; i++) {
		app_path = rom_path[i];

		if (!rom_loaded) {
			rom_loaded = vm->load_data_from_file(app_path, _T("L3BAS.ROM"), rom, 0x6000, (const uint8_t *)"\xc4\x5c", 2);
		}
		if (!rom_loaded) {
			// try to load another main rom.
			rom_loaded = vm->load_data_from_file(app_path, _T("ROM1.ROM"), rom, 0x5f00, (const uint8_t *)"\xc4\x5c", 2);
			if (rom_loaded) {
				rom_loaded = vm->load_data_from_file(app_path, _T("ROM2.ROM"), &rom[0x5ff0], 16, (const uint8_t *)"\x00\x00\x01\x00", 4);
			}
		}
		if (!rom1802_loaded) {
			rom1802_loaded = vm->load_data_from_file(app_path, _T("MP1802.ROM"), rom1802, 0x800);
		}
		if (!rom1805_loaded) {
			rom1805_loaded = vm->load_data_from_file(app_path, _T("MP1805.ROM"), rom1805, 0x800);
		}
		if (!rom1806_loaded) {
			rom1806_loaded = vm->load_data_from_file(app_path, _T("MP1806.ROM"), rom1806, 0x800);
		}
	}

	if (!rom_loaded_at_first) {
		if (!rom_loaded) {
			logging->out_logf_x(LOG_WARN, CMsg::VSTR_couldn_t_be_loaded, _T("L3BAS.ROM"));
			logging->out_logf_x(LOG_WARN, CMsg::VSTR_couldn_t_be_loaded, _T("ROM1.ROM"));
			logging->out_logf_x(LOG_WARN, CMsg::VSTR_couldn_t_be_loaded, _T("ROM2.ROM"));
		}
		if (!rom1802_loaded) {
			logging->out_logf_x(LOG_WARN, CMsg::VSTR_couldn_t_be_loaded, _T("MP1802.ROM"));
		}
		if (!rom1805_loaded) {
			logging->out_logf_x(LOG_WARN, CMsg::VSTR_couldn_t_be_loaded, _T("MP1805.ROM"));
		}
		if (!rom1806_loaded) {
			logging->out_logf_x(LOG_WARN, CMsg::VSTR_couldn_t_be_loaded, _T("MP1806.ROM"));
		}
		rom_loaded_at_first = true;
	}
}

void MEMORY::reset_igram()
{
	// clear ig ram data
	for(int co=0; co<3; co++) {
		for(int ch=0; ch<256; ch++) {
			for(int i=0; i<8; i+=2) {
				ig_ram[(co*256+ch)*8+i]  =0x00;
				ig_ram[(co*256+ch)*8+i+1]=0xff;
				if ((ch % 3) == co) {
					switch(i) {
						case 0:
							if ((ch >> 4) < 8) ig_ram[(co*256+ch)*8+i]=(1 << (ch >> 4));
							break;
						case 2:
							if ((ch >> 4) < 16) ig_ram[(co*256+ch)*8+i]=(1 << ((ch >> 4) - 8));
							break;
						case 4:
							if ((ch & 15) < 8) ig_ram[(co*256+ch)*8+i]=(1 << (ch & 15));
							break;
						case 6:
							if ((ch & 15) < 16) ig_ram[(co*256+ch)*8+i]=(1 << ((ch & 15) - 8));
							break;
					}
				}
			}
		}
	}
	ig_enable = 0;
	ig_enreg = 0;
}

void MEMORY::reset_membank()
{
	// init memory map
	mem_bank_reg1 = 0x0e;
	mem_bank_reg2 = 0;
	mem_vram_sel = true;
//	vram_sel = 0;
	change_l3_memory_bank();
}

void MEMORY::reset_tracecounter()
{
	// stop trace counter
	if (trace_counter_id != -1) {
		cancel_event(this, trace_counter_id);
	}
	trace_counter_id = -1;
	d_board->write_signal(SIG_CPU_NMI, 0, SIG_NMI_TRACE_MASK);
	pc = 0;
	pc_prev = TRACE_PC_IDLE;
}

void MEMORY::set_igmode(uint32_t data)
{
	ig_enable = data & 0x01;
	d_disp->write_io8(0xffe9, data);
}

#ifdef USE_CPU_REAL_MACHINE_CYCLE
void MEMORY::write_data8w(uint32_t addr, uint32_t data, int *wait)
#else
void MEMORY::write_data8(uint32_t addr, uint32_t data)
#endif
{
	addr &= 0xffff;

#ifdef _DEBUG_RAM
	if (addr >= 0xff70 && addr <= 0xff7f) {
		uint8_t ch = (data < 0x20 || data > 0x7e) ? '.' : data;
		logging->out_debugf("mw %04x=%02x->%02x %c",addr,ram[addr],data,ch);
	}
#endif

	// vram area
	if (mem_vram_sel && L3_ADDR_VRAM_START <= addr && addr < L3_ADDR_VRAM_END) {
#ifdef _DEBUG_CRAM
		{
			uint8_t ch=data;
			ch = (ch < 0x20 || ch > 0x7e) ? '.' : ch;
			logging->out_debugf("mw %04x=%02x->%02x %c creg:%02x cram:%02x -> %02x inv:%02x",addr,ram[addr],data,ch,color_reg,color_ram[addr - L3_ADDR_VRAM_START],color_reg,color_reg_inv);
		}
#endif
		color_ram[addr - L3_ADDR_VRAM_START] = (color_reg & 0x3f);
	}

	// ig_area
	if (ig_enable) {
		if (ADDR_IGRAM_START <= addr && addr < ADDR_IGRAM_END) {
			for(int i=0; i<3; i++) {
				if (ig_enreg & (1 << i)) {
					ig_ram[addr-ADDR_IGRAM_START+(ADDR_IGRAM_END-ADDR_IGRAM_START)*i] = data & 0xff;
				}
			}
			return;
		}
	}

	if (0xff00 <= addr && addr <= 0xffef) {
		ram[addr] = data;
	} else {
		wbank[addr >> BANK_SIZE][addr & ((1 << BANK_SIZE) - 1)] = data;
	}

#define WRITE_IO8 write_io8
#include "memory_writeio.h"

#ifdef USE_DEBUGGER
	bas->SetTraceBack(addr);
#endif
}

#ifdef USE_CPU_REAL_MACHINE_CYCLE
uint32_t MEMORY::read_data8w(uint32_t addr, int *wait)
#else
uint32_t MEMORY::read_data8(uint32_t addr)
#endif
{
	uint32_t data;
	addr &= 0xffff;

	if (pc_prev != TRACE_PC_IDLE) {
		if (pc_prev == TRACE_PC_FETCH) {
			// trace counter start
			set_trace_counter();
		} else if (pc_prev == TRACE_PC_FIRE) {
			pc_prev = TRACE_PC_FIRED;
		} else if (pc_prev == TRACE_PC_FIRED) {
			// trace counter reset
			clear_tracecounter();
		}
	}

	// vram area
	if (mem_vram_sel && L3_ADDR_VRAM_START <= addr && addr < L3_ADDR_VRAM_END) {
		// 6bit
		// if MK bit is set, do not read from color ram.
		if ((color_reg & 0x80) == 0) {
#ifdef _DEBUG_CRAM
			{
				uint8_t ch=ram[addr];
				ch = (ch < 0x20 || ch > 0x7e) ? '.' : ch;
				logging->out_debugf("mr %04x=%02x %c creg:%02x -> %02x",addr,ram[addr],ch,color_reg,color_ram[addr - L3_ADDR_VRAM_START]);
			}
#endif
			color_reg = (color_ram[addr - L3_ADDR_VRAM_START] & 0x3f);
		}
#ifdef _DEBUG_CRAM
		else {
			{
				uint8_t ch=ram[addr];
				ch = (ch < 0x20 || ch > 0x7e) ? '.' : ch;
				logging->out_debugf("mr %04x=%02x %c (creg:%02x cram:%02x)",addr,ram[addr],ch,color_reg,color_ram[addr - L3_ADDR_VRAM_START]);
			}
		}
#endif
	}

	if (0xff00 <= addr && addr <= 0xffef) {
//		data = ram[addr];
		data = 0xff;
	} else {
		data = rbank[addr >> BANK_SIZE][addr & ((1 << BANK_SIZE) - 1)];
	}

	// memory mapped i/o
#define READ_IO8 read_io8
#include "memory_readio.h"

#ifdef _DEBUG_RAM
	if (addr >= 0xff70 && addr <= 0xff7f) {
		uint8_t ch = (data < 0x20 || data > 0x7f) ? 0x20 : data;
		logging->out_debugf("mr %04x=%02x %c",addr,data,ch);
	}
#endif

	return data;
}

#ifdef USE_CPU_REAL_MACHINE_CYCLE
void MEMORY::latch_address(uint32_t addr, int *wait)
{

}
#endif

void MEMORY::write_io8(uint32_t addr, uint32_t data)
{

}

void MEMORY::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch(id) {
		case SIG_MEMORY_PIA_PA:
//			if (!FLG_USEPIAJOYSTICK) {
				// from PIA portA
				mem_bank_reg1 = data & 0xce;
				change_l3_memory_bank();
//			}
			break;
		case SIG_CPU_RESET:
			now_reset = (data & mask) ? true : false;
			if (now_reset == true) {
				// now on reset signal

				// reset igmode
				set_igmode(0);
				ig_enreg = 0;
			} else {
				// turn off reset signal

				// init memory map
				reset_membank();
				// stop trace counter
				reset_tracecounter();
			}
			break;
	}
}

void MEMORY::change_l3_memory_bank()
{
//	mem_bank_reg1;	// from PIA port A
//	mem_bank_reg2;	// from 0xffe8
	uint8_t *wexram00 = wdmy;
	uint8_t *rexram00 = rdmy;
	uint8_t *wexram04 = wdmy;
	uint8_t *rexram04 = rdmy;
	uint8_t *wexram80 = wdmy;
	uint8_t *rexram80 = rdmy;

	// have extended ram
	if (pConfig->exram_size_num == 1) {
		wexram00 = ram2 + 0x0000;
		rexram00 = ram2 + 0x0000;
		wexram04 = ram2 + 0x0400;
		rexram04 = ram2 + 0x0400;
		wexram80 = ram2 + 0x8000;
		rexram80 = ram2 + 0x8000;
	}

	memset(banktype, 0, sizeof(banktype));

	// 0x0000 - 0x7fff
	if (mem_bank_reg2 & 0x08) {
		if (mem_bank_reg2 & 0x20) {
			// change to the extended ram both r/w
			SET_BANK(0x0000, 0x7fff, wexram00, rexram00, 0x22);
		} else {
			// change to the extended ram only write
			SET_BANK(0x0000, 0x7fff, wexram00, ram + 0x0000, 0x21);
		}
	} else {
		// change to standard
		SET_BANK(0x0000, 0x7fff, ram + 0x0000, ram + 0x0000, 0x11);
	}

	// 0x0400 - 0x43ff (vram area)
	// exchange vram area to/from extended ram
	if ((mem_bank_reg2 & 0x0c) == 0x04 || (mem_bank_reg2 & 0x0c) == 0x08) {
		// select extend ram
		mem_vram_sel = false;
		if (mem_bank_reg2 & 0x20) {
			// read/write
			SET_BANK(0x0400, 0x43ff, wexram04, rexram04, 0x22);
		} else {
			// write only
			SET_BANK(0x0400, 0x43ff, wexram04, ram + 0x0400, 0x21);
		}
	} else {
		// select standard ram
		mem_vram_sel = true;
		SET_BANK(0x0400, 0x43ff, ram + 0x0400, ram + 0x0400, 0x11);
	}

	// 0x8000 - 0xffff
	if ((mem_bank_reg2 & 0x50) == 0x50) {
		// change to the extended ram both r/w
		SET_BANK(0x8000, 0xffff, wexram80, rexram80, 0x22);
	} else {
		//
		// decide the area to write
		if (mem_bank_reg2 & 0x10) {
			// change to the extended ram only write
			SET_BANK(0x8000, 0xffff, wexram80, NULL, 0x20);
		} else {
			// change to the standard
			SET_BANK(0x8000, 0xffff, ram + 0x8000, NULL, 0x10);
		}

		//
		// decide the area to read
		SET_BANK(0x8000, 0x9fff, NULL, ram + 0x8000, 0x01);
		SET_BANK(0xa000, 0xffff, NULL, rdmy, 0x00);
		if (rom_loaded) {
			SET_BANK(0xa000, 0xffff, NULL, rom + 0x0000, 0x08);
		}
		if (IOPORT_USE_3FDD && rom1805_loaded) {
			// for 3inch compact floppy
			SET_BANK(0xf800, 0xffff, NULL, rom1805, 0x09);
		}
		if (IOPORT_USE_5FDD) {
			if (pConfig->fdd_type == FDD_TYPE_5FDD) {
				// for 5.25inch mini floppy
				if (rom1802_loaded) {
					SET_BANK(0xf800, 0xffff, NULL, rom1802, 0x0a);
				} else if (rom1806_loaded) {
					SET_BANK(0xf800, 0xffff, NULL, rom1806, 0x0b);
				}
			} else if (pConfig->fdd_type == FDD_TYPE_58FDD) {
				// for 8inch standard floppy
				if (rom1806_loaded) {
					SET_BANK(0xf800, 0xffff, NULL, rom1806, 0x0b);
				} else if (rom1802_loaded) {
					SET_BANK(0xf800, 0xffff, NULL, rom1802, 0x0a);
				}
			}
		}
		if (!(mem_bank_reg1 & 0x02)) {
			if (mem_bank_reg1 & 0x40) {
				// r/w to ram (this means cannot read rom data !)
				SET_BANK(0xa000, 0xbfff, NULL, ram + 0xa000, 0x01);
			}
		}
		if (!(mem_bank_reg1 & 0x04)) {
			if (mem_bank_reg1 & 0x80) {
				// r/w to ram (this means cannot read rom data !)
				SET_BANK(0xc000, 0xdfff, NULL, ram + 0xc000, 0x01);
			}
		}
		if (!(mem_bank_reg1 & 0x08)) {
			if (mem_bank_reg1 & 0x80) {
				// r/w to ram (this means cannot read rom data !)
				SET_BANK(0xe000, 0xefff, NULL, ram + 0xe000, 0x01);
			}
		}

		if (mem_bank_reg2 & 0x01) {
			// read from ram (this means cannot read rom data !)
			SET_BANK(0xf000, 0xfeff, NULL, ram + 0xf000, 0x01);
		} else {
			// read from rom , write to ram
			SET_BANK(0xf000, 0xfeff, NULL, rdmy, 0x00);
			if (rom_loaded) {
				SET_BANK(0xf000, 0xfeff, NULL, rom + 0x5000, 0x08);
			}
			if (IOPORT_USE_3FDD && rom1805_loaded) {
				// for 3inch compact floppy
				SET_BANK(0xf800, 0xfeff, NULL, rom1805, 0x09);
			}
			if (IOPORT_USE_5FDD) {
				if (pConfig->fdd_type == FDD_TYPE_5FDD) {
					// for 5.25inch mini floppy
					if (rom1802_loaded) {
						SET_BANK(0xf800, 0xfeff, NULL, rom1802, 0x0a);
					} else if (rom1806_loaded) {
						SET_BANK(0xf800, 0xfeff, NULL, rom1806, 0x0b);
					}
				} else if (pConfig->fdd_type == FDD_TYPE_58FDD) {
					// for 8inch standard floppy
					if (rom1806_loaded) {
						SET_BANK(0xf800, 0xfeff, NULL, rom1806, 0x0b);
					} else if (rom1802_loaded) {
						SET_BANK(0xf800, 0xfeff, NULL, rom1802, 0x0a);
					}
				}
			}		}
		if (mem_bank_reg2 & 0x02) {
			// r/w to ram (this means cannot read rom data !)
			SET_BANK(0xfff0, 0xffff, NULL, ram + 0xfff0, 0x01);
		} else {
			// read from rom , write to ram
			SET_BANK(0xfff0, 0xffff, NULL, rdmy, 0x00);
			if (rom_loaded) {
				SET_BANK(0xfff0, 0xffff, NULL, rom + 0x5ff0, 0x08);
			}
		}
	}
}

// return vram area
uint8_t* MEMORY::get_vram() {
// vram is always the standard ram
//	return ((mem_bank_reg2 & 0x04) ? ram2 : ram );
	return ram;
}

void MEMORY::fetch_trace_counter() {
	pc = 0;
	pc_prev = TRACE_PC_FETCH;
//	logging->out_debugf(_T("trace_fetch: clk:%lld"), get_current_clock() % 900000);
}

void MEMORY::set_trace_counter() {
	// send NMI interrupt after few clocks (16 at least)
	register_event_by_clock(this, SIG_TRACE_COUNTER, 16, false, &trace_counter_id);
	pc = 0;
	pc_prev = TRACE_PC_WAITING;
//	logging->out_debugf(_T("trace_start: clk:%lld"), get_current_clock() % 900000);
}

void MEMORY::clear_tracecounter()
{
	// clear NMI
	d_board->write_signal(SIG_CPU_NMI, 0, SIG_NMI_TRACE_MASK);
//	logging->out_debugf(_T("trace NMI off: clk:%lld"), get_current_clock() % 900000);
	pc_prev = TRACE_PC_IDLE;
}

// ----------------------------------------------------------------------------
// event handler
// ----------------------------------------------------------------------------

void MEMORY::event_callback(int event_id, int err)
{
	if (event_id == SIG_TRACE_COUNTER) {
		// event stop
//		cancel_event(this, trace_counter_id);
		trace_counter_id = -1;

		pc = d_cpu->get_pc();

#if 0
		if (pc == pc_prev || pc_prev == TRACE_PC_WAITING) {
			// inhibit the NMI interrupt until next opcode is executed.
			pc_prev = pc;
			// next event
			register_event(this, SIG_TRACE_COUNTER, 2, false, &trace_counter_id);
//			logging->out_debugf(_T("trace_fire: clk:%lld pc:%04x next"), get_current_clock() % 900000, pc);
		} else
#endif
		{
			// send NMI interrupt
			d_board->write_signal(SIG_CPU_NMI, SIG_NMI_TRACE_MASK, SIG_NMI_TRACE_MASK);
//			logging->out_debugf(_T("trace_fire: clk:%lld pc:%04x NMI on"), get_current_clock() % 900000, pc);
			pc_prev = TRACE_PC_FIRED;
		}
	}
}

// ----------------------------------------------------------------------------

void MEMORY::update_config()
{
}

// ----------------------------------------------------------------------------

void MEMORY::save_state(FILEIO *fio)
{
	struct vm_state_st *vm_state = NULL;

	// save header
	vm_state_ident.version = Uint16_LE(1);
	vm_state_ident.size = Uint32_LE(sizeof(vm_state_ident) + sizeof(struct vm_state_st));
	fio->Fwrite(&vm_state_ident, sizeof(vm_state_ident), 1);

	// save values
	fio->Fwrite(ram,  sizeof(ram), 1);
	fio->Fwrite(ram2, sizeof(ram2), 1);
	fio->Fwrite(color_ram, sizeof(color_ram), 1);
	fio->Fwrite(ig_ram, sizeof(ig_ram), 1);

	fio->FputUint8(mem_bank_reg1);
	fio->FputUint8(mem_bank_reg2);

	fio->FputInt32_LE(trace_counter_id);
	fio->FputUint32_LE(pc);
	fio->FputUint32_LE(pc_prev);

	// save config
	fio->FputUint8(pConfig->dipswitch);
	fio->FputInt32_LE(pConfig->fdd_type);
	fio->FputInt32_LE(pConfig->io_port);

	fio->Fsets(0, sizeof(vm_state->reserved));
}

bool MEMORY::load_state(FILEIO *fio)
{
	vm_state_ident_t vm_state_i;
	struct vm_state_st *vm_state = NULL;

	if (find_state_chunk(fio, &vm_state_i) != true) {
		return true;
	}

	// load values
	fio->Fread(ram, sizeof(ram), 1);
	fio->Fread(ram2, sizeof(ram2), 1);
	fio->Fread(color_ram, sizeof(color_ram), 1);
	fio->Fread(ig_ram, sizeof(ig_ram), 1);

	mem_bank_reg1 = fio->FgetUint8();
	mem_bank_reg2 = fio->FgetUint8();

	trace_counter_id = fio->FgetInt32_LE();
	pc = fio->FgetUint32_LE();
	pc_prev = fio->FgetUint32_LE();

	// load config
	pConfig->dipswitch = fio->FgetUint8();
	pConfig->fdd_type = fio->FgetInt32_LE();
	pConfig->io_port = fio->FgetInt32_LE();

	fio->Fseek(sizeof(vm_state->reserved), FILEIO::SEEKCUR);

	// calc values
//	write_data8(0xffd0, ram[0xffd0]);	// mode sel
	// rs bit on mode_sel is set in COMM device.
	REG_MODE_SEL = ram[0xffd0];
	d_disp->write_io8(0xffd0, REG_MODE_SEL);

	write_data8(0xffd6, ram[0xffd6]);	// interace sel
	write_data8(0xffd8, ram[0xffd8]);	// color reg

	write_data8(0xffea, ram[0xffea]);	// IG en register

	change_l3_memory_bank();

	return true;
}

// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
void MEMORY::set_debugger_console(DebuggerConsole *dc)
{
	this->dc = dc;
	if (bas) bas->SetDebuggerConsole(dc);
}

void MEMORY::debug_write_data8(int type, uint32_t addr, uint32_t data)
{
	switch(type) {
	case 0:
		ram[addr & 0xffff] = data;
		break;
	case 1:
		ram2[addr & 0xffff] = data;
		break;
	case 2:
		color_ram[addr & 0x3fff] = data;
		break;
	case 3:
		ig_ram[addr & 0x7ff] = data;
		break;
	case 4:
		ig_ram[(addr & 0x7ff) + 0x800] = data;
		break;
	case 5:
		ig_ram[(addr & 0x7ff) + 0x1000] = data;
		break;
	default:
		{
#ifdef USE_CPU_REAL_MACHINE_CYCLE
		int wait;
		write_data8w(addr, data, &wait);
#else
		write_data8(addr, data);
#endif
		}
		break;
	}
}

uint32_t MEMORY::debug_read_data8(int type, uint32_t addr)
{
	uint32_t data;

	switch(type) {
	case 0:
		data = ram[addr & 0xffff];
		break;
	case 1:
		data = ram2[addr & 0xffff];
		break;
	case 2:
		data = color_ram[addr & 0x3fff];
		break;
	case 3:
		data = ig_ram[addr & 0x7ff];
		break;
	case 4:
		data = ig_ram[(addr & 0x7ff) + 0x800];
		break;
	case 5:
		data = ig_ram[(addr & 0x7ff) + 0x1000];
		break;
	default:
		if (0xff00 <= addr && addr <= 0xffef) {
			data = 0xff;
		} else {
			data = rbank[addr >> BANK_SIZE][addr & ((1 << BANK_SIZE) - 1)];
		}

	// memory mapped i/o
#undef READ_IO8
#undef WRITE_SIGNAL
#undef DEBUG_READ_OK
#define READ_IO8 debug_read_io8
#include "memory_readio.h"
		break;
	}
	return data;
}

void MEMORY::debug_write_data16(int type, uint32_t addr, uint32_t data)
{
	// big endien
	debug_write_data8(type, addr, (data >> 8) & 0xff);
	debug_write_data8(type, addr + 1, data & 0xff);
}

uint32_t MEMORY::debug_read_data16(int type, uint32_t addr)
{
	// big endien
	uint32_t val = debug_read_data8(type, addr) << 8;
	val |= debug_read_data8(type, addr + 1);
	return val;
}

void MEMORY::debug_write_data32(int type, uint32_t addr, uint32_t data)
{
	// big endien
	debug_write_data16(type, addr, (data >> 16) & 0xffff);
	debug_write_data16(type, addr + 2, data & 0xffff);
}

uint32_t MEMORY::debug_read_data32(int type, uint32_t addr)
{
	// big endien
	uint32_t val = debug_read_data16(type,addr) << 16;
	val |= debug_read_data16(type,addr + 2);
	return val;
}

uint32_t MEMORY::debug_physical_addr_mask(int type)
{
	uint32_t data = 0;
	switch(type) {
	case 0:
		data = 0xffff;
		break;
	case 1:
		data = 0xffff;
		break;
	case 2:
		data = 0x3fff;
		break;
	case 3:
	case 4:
	case 5:
		data = 0x7ff;
		break;
	}
	return data;
}

bool MEMORY::debug_physical_addr_type_name(int type, _TCHAR *buffer, size_t buffer_len)
{
	bool exist = true;
	switch(type) {
	case 0:
		UTILITY::tcscpy(buffer, buffer_len, _T("main RAM"));
		break;
	case 1:
		UTILITY::tcscpy(buffer, buffer_len, _T("extended RAM"));
		break;
	case 2:
		UTILITY::tcscpy(buffer, buffer_len, _T("color RAM"));
		break;
	case 3:
		UTILITY::tcscpy(buffer, buffer_len, _T("IG RAM (blue)"));
		break;
	case 4:
		UTILITY::tcscpy(buffer, buffer_len, _T("IG RAM (red)"));
		break;
	case 5:
		UTILITY::tcscpy(buffer, buffer_len, _T("IG RAM (green)"));
		break;
	default:
		exist = false;
		break;
	}
	return exist;
}

uint32_t MEMORY::debug_read_bank(uint32_t addr)
{
	uint32_t data = 0;
	if (ig_enable && ADDR_IGRAM_START <= addr && addr < ADDR_IGRAM_END) {
		data = banktype[addr >> BANK_SIZE];
		data = (data & 0x0f) | 0x30;	// IG

	} else if (0xff00 <= addr && addr <= 0xffef) {
		data = 0xff;	// io
	} else {
		data = banktype[addr >> BANK_SIZE];
	}
	return data;
}

void MEMORY::debug_memory_map_info(DebuggerConsole *dc)
{
	uint32_t prev_addr = 0;
	uint8_t  prev_data = 0;
	uint32_t end_addr = 0x10000;
	uint32_t inc_addr = 16;

	for(uint32_t addr=0; addr <= end_addr; addr+=inc_addr) {
		uint8_t data = debug_read_bank(addr);
		if (addr == 0) {
			prev_data = data;
		}
		if (data != prev_data || addr == end_addr) {
			dc->Printf(_T("%04X - %04X : Read:"), prev_addr, addr-1);
			print_memory_bank(prev_data, false, dc->GetBuffer(true), dc->GetBufferSize());
			dc->Out(false);
			dc->Print(_T("  Write:"), false);
			print_memory_bank(prev_data, true, dc->GetBuffer(true), dc->GetBufferSize());
			dc->Out(false);
			dc->Cr();
			prev_addr = addr;
			prev_data = data;
		}
	}
}

void MEMORY::print_memory_bank(uint32_t data, bool w, _TCHAR *buffer, size_t buffer_len)
{
	_TCHAR str[32];
	if (w) {
		data >>= 4;
	}
	switch(data & 0x0f) {
	case 1:
		UTILITY::tcscpy(str, sizeof(str) / sizeof(str[0]), _T("Main RAM"));
		break;
	case 2:
		UTILITY::tcscpy(str, sizeof(str) / sizeof(str[0]), _T("Extend RAM"));
		break;
	case 3:
		UTILITY::tcscpy(str, sizeof(str) / sizeof(str[0]), _T("IG RAM"));
		break;
	case 8:
		UTILITY::tcscpy(str, sizeof(str) / sizeof(str[0]), _T("L3 Basic ROM"));
		break;
	case 9:
		UTILITY::tcscpy(str, sizeof(str) / sizeof(str[0]), _T("MP-1805 ROM"));
		break;
	case 10:
		UTILITY::tcscpy(str, sizeof(str) / sizeof(str[0]), _T("MP-1802 ROM"));
		break;
	case 11:
		UTILITY::tcscpy(str, sizeof(str) / sizeof(str[0]), _T("MP-1806 ROM"));
		break;
	case 15:
		UTILITY::tcscpy(str, sizeof(str) / sizeof(str[0]), _T("I/O Port"));
		break;
	default:
		UTILITY::tcscpy(str, sizeof(str) / sizeof(str[0]), _T("(no assign)"));
		break;
	}
	UTILITY::stprintf(buffer, buffer_len, _T("%-15s"), str); 
}


bool MEMORY::debug_write_reg(uint32_t reg_num, uint32_t data)
{
	return false;
}

void MEMORY::debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
}

int MEMORY::get_debug_graphic_memory_size(int num, int type, int *width, int *height)
{
	return d_disp->get_debug_graphic_memory_size(num, type, width, height);
}

bool MEMORY::debug_graphic_type_name(int type, _TCHAR *buffer, size_t buffer_len)
{
	return d_disp->debug_graphic_type_name(type, buffer, buffer_len);
}

bool MEMORY::debug_draw_graphic(int type, int width, int height, scrntype *buffer)
{
	return d_disp->debug_draw_graphic(type, width, height, buffer);
}

bool MEMORY::debug_dump_graphic(int type, int width, int height, uint16_t *buffer)
{
	return d_disp->debug_dump_graphic(type, width, height, buffer);
}

bool MEMORY::debug_basic_is_supported()
{
	return true;
}

uint32_t MEMORY::debug_basic_get_line_number_ptr()
{
	return bas->GetLineNumberPtr();
}

uint32_t MEMORY::debug_basic_get_line_number()
{
	return bas->GetLineNumber();
}

void MEMORY::debug_basic_variables(DebuggerConsole *dc, int name_cnt, const _TCHAR **names)
{
	bas->PrintVariable(name_cnt, names);
}

void MEMORY::debug_basic_list(DebuggerConsole *dc, int st_line, int ed_line)
{
	if (st_line == -1 || st_line == ed_line) {
		bas->PrintCurrentLine(st_line);
	} else {
		bas->PrintList(st_line, ed_line, 0);
	}
}

void MEMORY::debug_basic_trace_onoff(DebuggerConsole *dc, bool enable)
{
	bas->TraceOnOff(enable);
}

void MEMORY::debug_basic_trace_current()
{
	bas->PrintCurrentTrace();
}

void MEMORY::debug_basic_trace_back(DebuggerConsole *dc, int num)
{
	bas->PrintTraceBack(num);
}

void MEMORY::debug_basic_command(DebuggerConsole *dc)
{
	bas->PrintCommandList();
}

void MEMORY::debug_basic_error(DebuggerConsole *dc, int num)
{
	bas->PrintError(num);
}

bool MEMORY::debug_basic_check_break_point(uint32_t line, int len)
{
	return bas->IsCurrentLine(line, line + len);
}

#endif
