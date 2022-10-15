/** @file psg9c.cpp

	HITACHI BASIC MASTER LEVEL3 Mark5 Emulator 'EmuB-6892'
	Skelton for retropc emulator

	@author Sasaji
	@date   2012.06.08 -

	@brief [ 9voice psg control ]
*/

#include "psg9c.h"
//#include "../../emu.h"
#include "../../fileio.h"
#include "../via.h"

void PSG9C::initialize()
{
}

void PSG9C::reset()
{
	pa = 0;
	pb = 0;
	sel = 0;
}

void PSG9C::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch(id) {
		case SIG_PSG9_VIA_PA:
			pa = (data & mask & 0xff);
			break;
		case SIG_PSG9_VIA_PB:
			pb = (data & mask & 0x1f);
			sel = (pb & 0x1c) >> 2;
			break;
	}
	switch(pb & 3) {
		case 2:
			// read from psg
			pa = 0;
			if (sel & 1) pa |= d_psg[0]->read_io8(1);
			if (sel & 2) pa |= d_psg[1]->read_io8(1);
			if (sel & 4) pa |= d_psg[2]->read_io8(1);
			// write to pia port A
			d_via->write_signal(VIA::SIG_VIA_PA, pa, 0xff);
			break;
		case 1:
			// write to psg
			if (sel & 1) d_psg[0]->write_io8(1, pa);
			if (sel & 2) d_psg[1]->write_io8(1, pa);
			if (sel & 4) d_psg[2]->write_io8(1, pa);
			break;
		case 3:
			// latch address
			if (sel & 1) d_psg[0]->write_io8(0, pa);
			if (sel & 2) d_psg[1]->write_io8(0, pa);
			if (sel & 4) d_psg[2]->write_io8(0, pa);
			break;
	}
}

// ----------------------------------------------------------------------------

void PSG9C::save_state(FILEIO *fio)
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

bool PSG9C::load_state(FILEIO *fio)
{
	vm_state_ident_t vm_state_i;
	struct vm_state_st vm_state;

	READ_STATE_CHUNK(fio, vm_state_i, vm_state);

	pa = vm_state.pa;
	pb = vm_state.pb;
	sel = vm_state.sel;

	return true;
}
