/** @file kanji.cpp

	HITACHI BASIC MASTER LEVEL3 Mark5 / MB-S1 Emulator 'EmuB-6892/EmuB-S1'
	Skelton for retropc emulator

	@author Sasaji
	@date   2011.10.19 -

	@brief [ kanji ]
*/

#include "kanji.h"
#include "../vm.h"
#include "../../logging.h"
#include "../../fileio.h"
#include "../../config.h"

#define EN_JIS2REG 0x02
#define EN_JIS2KAN 0x10

const uint8_t KANJI::krom[32] = {
	0x00,0x00,0x7f,0xfc,0x60,0x0c,0x50,0x14,0x48,0x24,0x44,0x44,0x42,0x84,0x41,0x04,
	0x42,0x84,0x44,0x44,0x48,0x24,0x50,0x14,0x60,0x0c,0x7f,0xfc,0x00,0x00,0x00,0x00
};

void KANJI::init_kanji_rom(uint8_t *rom, size_t size)
{
	for(size_t i=0; i<size; i++) {
		rom[i] = krom[i & 0x1f];
	}
}

void KANJI::initialize()
{
	init_kanji_rom(font, sizeof(font));
	font_enable = 0;
#ifdef USE_KANJI_JIS2
	init_kanji_rom(font2, sizeof(font2));
	font2_enable = 0;
	en_jis2 = 0;
#endif

	// load rom images
	const _TCHAR *app_path, *rom_path[2];

	rom_path[0] = pConfig->rom_path.Get();
	rom_path[1] = vm->application_path();

	for(int i=0; i<2; i++) {
		app_path = rom_path[i];

		if (!font_enable) {
			font_enable = vm->load_data_from_file(app_path, _T("KANJI.ROM"), font, sizeof(font));
		}
#ifdef USE_KANJI_JIS2
		if (!font2_enable) {
			font2_enable = vm->load_data_from_file(app_path, _T("KANJI2.ROM"), font2, sizeof(font2));
		}
#endif
	}

	if (!font_enable) {
		logging->out_logf_x(LOG_WARN, CMsg::VSTR_couldn_t_be_loaded, _T("KANJI.ROM"));
	}
#ifdef USE_KANJI_JIS2
	if (!font2_enable) {
		logging->out_logf_x(LOG_WARN, CMsg::VSTR_couldn_t_be_loaded, _T("KANJI2.ROM"));
	}
#endif

	code = 0;
	offset = 0;
}

void KANJI::reset()
{
	code = 0;
	offset = 0;
#ifdef USE_KANJI_JIS2
	en_jis2 = 0;
#endif
}

void KANJI::write_signal(int id, uint32_t data, uint32_t mask)
{
}

void KANJI::write_io8(uint32_t addr, uint32_t data)
{
	switch (addr & 0x0f) {
#ifdef USE_KANJI_JIS2
		case 2:
			code &= 0x00ff;
			code |= ((data & 0xff) << 8);
			break;
		case 3:
			code &= 0xff00;
			code |= (data & 0xf0);
			offset = (data & 0x0f);
			break;
		case 4:
			en_jis2 = (data & 0x1f);
			break;
#endif
		case 5:
			code &= 0x00ff;
			code |= ((data & 0xff) << 8);
			break;
		case 6:
			code &= 0xff00;
			code |= (data & 0xf0);
			offset = (data & 0x0f);
			break;
	}
}

uint32_t KANJI::read_io8(uint32_t addr)
{
	uint32_t data = 0xff;
	switch (addr & 0x0f) {
#ifdef USE_KANJI_JIS2
		case 2:
			if ((en_jis2 & 0x0f) == EN_JIS2REG) {
				if (en_jis2 & EN_JIS2KAN) {
					data = font2[(code + offset) << 1];
				} else {
					data = font[(code + offset) << 1];
				}
			}
			break;
		case 3:
			if ((en_jis2 & 0x0f) == EN_JIS2REG) {
				if (en_jis2 & EN_JIS2KAN) {
					data = font2[((code + offset) << 1) + 1];
				} else {
					data = font[((code + offset) << 1) + 1];
				}
			}
			break;
		case 4:
			if ((en_jis2 & 0x0f) == EN_JIS2REG) {
				data = ((en_jis2 & 0xf0) | (~en_jis2 & 0x0f) | 0xe0);
			}
			break;
#endif
		case 5:
			data = font[(code + offset) << 1];
			break;
		case 6:
			data = font[((code + offset) << 1) + 1];
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
#ifdef USE_KANJI_JIS2
	vm_state.en_jis2 = en_jis2;
#endif

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
#ifdef USE_KANJI_JIS2
	en_jis2 = vm_state.en_jis2;
#endif

	return true;
}

// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
uint32_t KANJI::debug_read_io8(uint32_t addr)
{
	return read_io8(addr);
}
#endif

