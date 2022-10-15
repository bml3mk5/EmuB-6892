/** @file psgc.cpp

	HITACHI BASIC MASTER LEVEL3 Mark5 Emulator 'EmuB-6892'
	Skelton for retropc emulator

	@author Sasaji
	@date   2012.06.08 -

	@brief [ psg control ]
*/

#include "psgc.h"
//#include "../../emu.h"
#include "../../fileio.h"
#include "../pia.h"

void PSGC::initialize()
{
}

void PSGC::reset()
{
	pa = 0;
	pb = 0;
	sel = 0;
}

void PSGC::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch(id) {
		case SIG_PSG_PIA_PA:
			pa = (data & mask & 0xff);
			break;
		case SIG_PSG_PIA_PB:
			pb = (data & mask & 0x07);
			sel = (pb & 4) >> 2;
			break;
	}
	switch(pb & 3) {
		case 1:
			// read from psg
			pa = 0;
			if (sel == 0) pa |= d_psg[0]->read_io8(1);
			if (sel == 1) pa |= d_psg[1]->read_io8(1);
			// write to pia port A
			d_pia->write_signal(PIA::SIG_PIA_PA, pa, 0xff);
			break;
		case 2:
			// write to psg
			if (sel == 0) d_psg[0]->write_io8(1, pa);
			if (sel == 1) d_psg[1]->write_io8(1, pa);
			break;
		case 3:
			// latch address
			if (sel == 0) d_psg[0]->write_io8(0, pa);
			if (sel == 1) d_psg[1]->write_io8(0, pa);
			break;
	}
}

// ----------------------------------------------------------------------------

void PSGC::save_state(FILEIO *fio)
{
	struct vm_state_st vm_state;

	//
	vm_state_ident.version = Uint16_LE(1);
	vm_state_ident.size = Uint32_LE(sizeof(vm_state_ident) + sizeof(vm_state));

	// copy values
	memset(&vm_state, 0, sizeof(vm_state));
	vm_state.pa = pa;
	vm_state.pb = pb;
	vm_state.sel = sel;

	fio->Fwrite(&vm_state_ident, sizeof(vm_state_ident), 1);
	fio->Fwrite(&vm_state, sizeof(vm_state), 1);
}

bool PSGC::load_state(FILEIO *fio)
{
	vm_state_ident_t vm_state_i;
	struct vm_state_st vm_state;

	READ_STATE_CHUNK(fio, vm_state_i, vm_state);

	pa = vm_state.pa;
	pb = vm_state.pb;
	sel = vm_state.sel;

	return true;
}

