/** @file kanji.cpp

	HITACHI BASIC MASTER LEVEL3 Mark5 / MB-S1 Emulator 'EmuB-6892/EmuB-S1'
	Skelton for retropc emulator

	@author Sasaji
	@date   2011.10.19 -

	@brief [ kanji ]
*/

#include "kanji.h"
//#include "../../emu.h"
#include "../vm.h"
#include "../../logging.h"
#include "../../fileio.h"
#include "../../config.h"

static const uint16_t krom[16] = {
	0xfffe,0x0440,0x0440,0x0440,0x7ffc,0x4444,0x4444,0x4444,0x4444,0x7ffc,0x4444,0x0440,0x0440,0x0440,0xfffe,0x0000
};

void KANJI::initialize()
{
	memset(font, 0x01, sizeof(font));
	font_enable = false;

	// load rom images
	const _TCHAR *app_path, *rom_path[2];

	rom_path[0] = config.rom_path;
	rom_path[1] = vm->application_path();

	for(int i=0; i<2; i++) {
		app_path = rom_path[i];

		if (!font_enable) {
			font_enable = vm->load_data_from_file(app_path, _T("KANJI.ROM"), font, sizeof(font));
		}
	}

	if (!font_enable) {
		logging->out_logf_x(LOG_WARN, CMsg::VSTR_couldn_t_be_loaded, _T("KANJI.ROM"));
	}

	code = 0;
	offset = 0;
}

void KANJI::reset()
{
	code = 0;
	offset = 0;
}

void KANJI::write_signal(int id, uint32_t data, uint32_t mask)
{
}

void KANJI::write_io8(uint32_t addr, uint32_t data)
{
	switch (addr & 0xffff) {
		case 0:
			code &= 0x00ff;
			code |= ((data & 0xff) << 8);
			break;
		case 1:
			code &= 0xff00;
			code |= (data & 0xf0);
			offset = (data & 0x0f);
			break;
	}
}

uint32_t KANJI::read_io8(uint32_t addr)
{
	uint32_t data = 0;
	switch (addr & 0xffff) {
		case 0:
			if (font_enable) {
				data = font[(code + offset) << 1];
			} else {
				data = (krom[offset] & 0xff00) >> 8;
			}
			break;
		case 1:
			if (font_enable) {
				data = font[((code + offset) << 1) + 1];
			} else {
				data = (krom[offset] & 0x00ff);
			}
			break;
	}
	return data;
}

// ----------------------------------------------------------------------------
// event handler
// ----------------------------------------------------------------------------

void KANJI::event_frame()
{
}

void KANJI::event_callback(int event_id, int err)
{
}

// ----------------------------------------------------------------------------

void KANJI::save_state(FILEIO *fio)
{
	struct vm_state_st vm_state;

	//
	vm_state_ident.version = Uint16_LE(1);
	vm_state_ident.size = Uint32_LE(sizeof(vm_state_ident) + sizeof(vm_state));

	// copy values
	memset(&vm_state, 0, sizeof(vm_state));
	vm_state.code = Uint16_LE(code);
	vm_state.offset = offset;

	fio->Fwrite(&vm_state_ident, sizeof(vm_state_ident), 1);
	fio->Fwrite(&vm_state, sizeof(vm_state), 1);
}

bool KANJI::load_state(FILEIO *fio)
{
	vm_state_ident_t vm_state_i;
	struct vm_state_st vm_state;

	READ_STATE_CHUNK(fio, vm_state_i, vm_state);

	code = Uint16_LE(vm_state.code);
	offset = vm_state.offset;

	return true;
}

// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
uint32_t KANJI::debug_read_io8(uint32_t addr)
{
	return read_io8(addr);
}
#endif

