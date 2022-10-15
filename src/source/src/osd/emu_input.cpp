/** @file emu_input.cpp

	Skelton for retropc emulator

	@author Takeda.Toshiya
	@date   2006.08.18 -

	@note
	Modified for BML3MK5 by Sasaji at 2011.06.17

	@brief [ emu input ]
*/

#include "../emu.h"
#include "../vm/vm.h"
#include "../config.h"
#include <stdlib.h>
#include "../fifo.h"
#include "../fileio.h"
#include "emu_input.h"

void EMU::EMU_INPUT()
{
	vm_key_status = NULL;
	vm_key_status_size = 0;

	// initialize status
	memset(key_status, 0, sizeof(key_status));
	memset(mouse_status, 0, sizeof(mouse_status));
#ifdef USE_MOUSE_ABSOLUTE
	mouse_position.x = mouse_position.y = 0;
#endif

#ifdef USE_JOYSTICK
	joy_num = 0;
	memset(joy_status, 0, sizeof(joy_status));
	for(int i=0; i<2; i++) {
		joy_enabled[i] = false;
	}
#endif

#ifdef USE_AUTO_KEY
	// initialize autokey
	autokey_buffer = NULL;
	autokey_phase = 0;
	autokey_shift = 0;
	autokey_enabled = false;
#endif
}

void EMU::initialize_input()
{
	logging->out_debug(_T("EMU::initialize_input"));

#ifdef USE_JOYSTICK
	use_joystick = FLG_USEJOYSTICK_ALL ? true : false;
#endif
	// initialize joysticks
	initialize_joystick();

	// mouse emulation is disabled
	mouse_disabled = 1;
	initialize_mouse(FLG_USEMOUSE ? true : false);

#ifdef USE_SHIFT_NUMPAD_KEY
	// initialize shift+numpad conversion
	memset(key_converted, 0, sizeof(key_converted));
	key_shift_pressed = key_shift_released = false;
#endif
#ifdef USE_AUTO_KEY
	// initialize autokey
	autokey_buffer = new FIFOINT(65536);
//	autokey_buffer->clear();
	autokey_phase = 31;	// wait a few sec.
	autokey_shift = 0;
	autokey_enabled = false;
#endif
	lost_focus = false;
	key_mod = 0;
}

void EMU::initialize_joystick()
{
#ifdef USE_JOYSTICK
	for(int i=0; i<2; i++) {
		joy_enabled[i] = false;
	}
	reset_joystick();
#endif
}

void EMU::reset_joystick()
{
}

void EMU::release_input()
{
	// release mouse
	if(!mouse_disabled) {
		disable_mouse(0);
	}

#ifdef USE_AUTO_KEY
	// release autokey buffer
	if(autokey_buffer) {
		autokey_buffer->release();
		delete autokey_buffer;
	}
#endif
}

void EMU::release_joystick()
{
#ifdef USE_JOYSTICK
	// release joystick
	for(int i = 0; i < 2; i++) {
		joy_enabled[i] = false;
	}
	memset(joy_status, 0, sizeof(joy_status));
#endif
}

void EMU::update_input()
{
}

void EMU::update_mouse()
{
}

void EMU::update_autokey()
{
#ifdef USE_AUTO_KEY
	// auto key
	switch(autokey_phase) {
	case 1:
		if(autokey_buffer && !autokey_buffer->empty()) {
#ifdef USE_EMU_INHERENT_SPEC
			// update graph key status
			int graph = autokey_buffer->read_not_remove(0) & AUTO_KEY_GRAPH_MASK;
			if(graph && !autokey_shift) {
				vm_key_down(AUTO_KEY_GRAPH);
				logging->out_debugf(_T("autokey phase:% 2d: graph down"), autokey_phase);
				autokey_shift = graph;
				autokey_phase++;
				break;
			}
#endif
			// update shift key status
			int shift = autokey_buffer->read_not_remove(0) & AUTO_KEY_SHIFT_MASK;
			if(shift && !autokey_shift) {
				vm_key_down(AUTO_KEY_SHIFT);
				logging->out_debugf(_T("autokey phase:% 2d: shift down"), autokey_phase);
				autokey_shift = shift;
				autokey_phase++;
				break;
			}
			autokey_shift = 0;
		}
		autokey_phase++;
		break;
	case 2:
		if(autokey_buffer && !autokey_buffer->empty()) {
			autokey_code = autokey_buffer->read_not_remove(0) & AUTO_KEY_MASK;
			vm_key_down(autokey_code);
			logging->out_debugf(_T("autokey phase:% 2d: key down %x"), autokey_phase, autokey_code);
		}
		autokey_phase++;
		break;
	case USE_AUTO_KEY:
		if(!autokey_buffer || autokey_buffer->empty()) {
			autokey_phase = 30;
		} else {
			// wait response from vm
			logging->out_debugf(_T("autokey phase:% 2d: wait "), autokey_phase);
		}
		break;
	case USE_AUTO_KEY + 2:
		if(autokey_shift & AUTO_KEY_SHIFT_MASK) {
			vm_key_up(AUTO_KEY_SHIFT);
			logging->out_debugf(_T("autokey phase:% 2d: shift up:"), autokey_phase);
		}
#ifdef USE_EMU_INHERENT_SPEC
		if(autokey_shift & AUTO_KEY_GRAPH_MASK) {
			vm_key_up(AUTO_KEY_GRAPH);
			logging->out_debugf(_T("autokey phase:% 2d: graph up"), autokey_phase);
		}
#endif
		autokey_shift = 0;
		if(autokey_buffer && !autokey_buffer->empty()) {
			vm_key_up(autokey_buffer->read_not_remove(0) & AUTO_KEY_MASK);
		}
		logging->out_debugf(_T("autokey phase:% 2d: key up %x"), autokey_phase, autokey_code);
		autokey_code = 0;
		autokey_phase++;
		break;
	case USE_AUTO_KEY + 3:
		if(!autokey_buffer || autokey_buffer->empty()) {
			autokey_phase = 30;
		} else {
			// wait response from vm
			logging->out_debugf(_T("autokey phase:% 2d: wait "), autokey_phase);
		}
		break;
	case USE_AUTO_KEY + 4:
		if(autokey_buffer && !autokey_buffer->empty()) {
			// wait enough while vm analyzes one line
			int code = autokey_buffer->read();
			logging->out_debugf(_T("autokey phase:% 2d: code %x"), autokey_phase, code);
			if(code == AUTO_KEY_RETURN) {
				autokey_phase++;	// wait 30 frames
				break;
			}
		}
		// through
	case 30:
	case 150:
		if(autokey_buffer && !autokey_buffer->empty()) {
			autokey_phase = 1;
		}
		else {
			stop_auto_key();
			autokey_phase = 0;
		}
		break;
	default:
		if(autokey_phase) {
			autokey_phase++;
		}
		break;
	}
#endif
}

void EMU::recognized_key(uint16_t key_code)
{
#ifdef USE_AUTO_KEY
	switch(autokey_phase) {
	case USE_AUTO_KEY:
		if(autokey_code) {
			logging->out_debugf(_T("recognized_key: phase:%d auto_key:%x vm_key:%x"),autokey_phase,autokey_code,key_code);
			if (autokey_code == key_code) {
				autokey_phase = USE_AUTO_KEY + 1;
			}
		}
		break;
	case USE_AUTO_KEY + 3:
		if(key_code == AUTO_KEY_NONE) {
			logging->out_debugf(_T("recognized_key: phase:%d auto_key:%x vm_key:%x"),autokey_phase,autokey_code,key_code);
			autokey_phase =  USE_AUTO_KEY + 4;
		}
		break;
	case 0:
		break;
	}
#endif
}

void EMU::update_joystick()
{
}

int EMU::key_down_up(uint8_t type, int code, long status)
{
	return 1;
}

int EMU::key_down(int code, bool keep_frames)
{
	return code;
}

void EMU::key_up(int code, bool keep_frames)
{
}

int EMU::vm_key_down(int code)
{
#ifdef USE_EMU_INHERENT_SPEC
	code -= 0x80;
#endif
	if (vm_key_status && 0 <= code && code < vm_key_status_size) {
		vm_key_status[code] |= 1;
	}
	return code;
}
void EMU::vm_key_up(int code)
{
#ifdef USE_EMU_INHERENT_SPEC
	code -= 0x80;
#endif
	if (vm_key_status && 0 <= code && code < vm_key_status_size) {
		vm_key_status[code] &= ~1;
	}
}

#ifdef USE_BUTTON
void EMU::press_button(int num)
{
	int code = buttons[num].code;

	if(code) {
		key_down(code, false);
		key_status[code] = KEY_KEEP_FRAMES * FRAME_SPLIT_NUM;
	}
	else {
		// code=0: reset virtual machine
		vm->reset();
	}
}
#endif

void EMU::initialize_mouse(bool enable)
{
	if (enable) enable_mouse(0);
}

void EMU::enable_mouse(int mode)
{
}

void EMU::disable_mouse(int mode)
{
}

void EMU::toggle_mouse()
{
	// toggle mouse enable / disable
	if(mouse_disabled & 1) {
		enable_mouse(0);
	} else {
		disable_mouse(0);
	}
}

bool EMU::get_mouse_enabled()
{
	return ((mouse_disabled & 1) == 0);
}

#if 0 // def USE_MOUSE_ABSOLUTE
void EMU::set_mouse_position()
{
}
#endif

void EMU::enable_joystick()
{
}

#ifdef USE_AUTO_KEY
bool EMU::start_auto_key(const char *str)
{
	bool rc = true;

	stop_auto_key();

	autokey_code = 0;
	autokey_buffer->clear();

	int size = (int)strlen(str);
	int prev_code = 0;
	int prev_mod = 0;
	parse_auto_key(str, size, prev_code, prev_mod);
	parsed_auto_key(prev_code, prev_mod);

	update_config();

	return rc;
}

bool EMU::open_auto_key(const _TCHAR *file_path)
{
	FILEIO *fp;
	char buf[1025];
	bool rc = true;

	stop_auto_key();

	autokey_code = 0;
	fp = new FILEIO();
	int prev_mod = 0;
	if (fp->Fopen(file_path, FILEIO::READ_BINARY)) {
		autokey_buffer->clear();
		int prev_code = 0;
		int size = 0;
		while((size = (int)fp->Fread(buf, sizeof(char), 1024)) > 0) {
			parse_auto_key(buf, size, prev_code, prev_mod);
		}
		fp->Fclose();

		parsed_auto_key(prev_code, prev_mod);

		config.opened_autokey_path.Set(file_path, 0);
	} else {
		logging->out_log(LOG_ERROR, _T("Auto key file couldn't be opened."));
		rc = false;
	}
	delete fp;

	config.initial_autokey_path.SetFromPath(file_path);

	update_config();

	return rc;
}

void EMU::parse_auto_key(const char *buf, int size, int &prev_code, int &prev_mod)
{
	for(int i = 0; i < size; i++) {
		int code = buf[i] & 0xff;
#ifdef USE_EMU_INHERENT_SPEC
		if(prev_code == 0xd && code == 0xa) {
			prev_code = code;
			continue;	// cr-lf
		}
#else
		if((0x81 <= code && code <= 0x9f) || 0xe0 <= code) {
			i++;	// kanji ?
			continue;
		}
		else if(code == 0xa) {
			continue;	// cr-lf
		}
#endif
		prev_code = code;

		if((code = autokey_table[code]) != 0) {
#ifdef USE_EMU_INHERENT_SPEC
			int kana = code & (AUTO_KEY_KATA_MASK | AUTO_KEY_HIRA_MASK);
			int prev_kana = prev_mod & (AUTO_KEY_KATA_MASK | AUTO_KEY_HIRA_MASK);
			if(kana - prev_kana == AUTO_KEY_KATA_MASK) {
				autokey_buffer->write(AUTO_KEY_KANA);
			} else if(kana - prev_kana == AUTO_KEY_HIRA_MASK) {
				autokey_buffer->write(AUTO_KEY_KANA);
				autokey_buffer->write(AUTO_KEY_KANA);
			}
			if(prev_kana - kana == AUTO_KEY_HIRA_MASK) {
				autokey_buffer->write(AUTO_KEY_KANA);
			} else if(prev_kana - kana == AUTO_KEY_KATA_MASK) {
				autokey_buffer->write(AUTO_KEY_KANA);
				autokey_buffer->write(AUTO_KEY_KANA);
			}
#else
			if((prev_mod ^ code) & AUTO_KEY_KANA_MASK) {
				// hankaku katakana
				autokey_buffer->write(AUTO_KEY_KANA);
			}
#endif
			prev_mod = code & ~AUTO_KEY_MASK;

#if defined(USE_AUTO_KEY_NO_CAPS)
			if((code & AUTO_KEY_SHIFT_MASK) && !(code & (AUTO_KEY_UPPER_MASK | AUTO_KEY_LOWER_MASK))) {
#elif defined(USE_AUTO_KEY_CAPS)
			if(code & (AUTO_KEY_SHIFT_MASK | AUTO_KEY_LOWER_MASK)) {
#else
			if(code & (AUTO_KEY_SHIFT_MASK | AUTO_KEY_UPPER_MASK)) {
#endif
				autokey_buffer->write((code & AUTO_KEY_MASK) | AUTO_KEY_SHIFT_MASK);
			}
#ifdef USE_EMU_INHERENT_SPEC
			else if (code & AUTO_KEY_GRAPH_MASK) {
				autokey_buffer->write((code & AUTO_KEY_MASK) | AUTO_KEY_GRAPH_MASK);
			}
#endif
			else {
				autokey_buffer->write(code & AUTO_KEY_MASK);
			}
		}
	}
}

void EMU::parsed_auto_key(int prev_code, int prev_mod)
{
	if(prev_mod & AUTO_KEY_KATA_MASK) {
		autokey_buffer->write(AUTO_KEY_KANA);
		autokey_buffer->write(AUTO_KEY_KANA);
	} else if (prev_mod == AUTO_KEY_HIRA_MASK) {
		autokey_buffer->write(AUTO_KEY_KANA);
	}

	autokey_phase = (autokey_phase > 30 ? autokey_phase : 1);
	autokey_shift = 0;
	autokey_enabled = true;
}

void EMU::stop_auto_key()
{
#ifndef USE_EMU_INHERENT_SPEC
	if(autokey_shift) {
		vm_key_up(AUTO_KEY_SHIFT);
	}
	if (vm_key_status) {
		for (int i=0; i<vm_key_status_size; i++) {
			vm_key_status[i] &= ~1;
		}
	}
#else
	if(autokey_shift) {
		vm_key_up(AUTO_KEY_SHIFT);
	}
	if(autokey_shift) {
		vm_key_up(AUTO_KEY_GRAPH);
	}
	if (vm_key_status) {
		for (int i=0; i<vm_key_status_size; i++) {
			vm_key_status[i] &= ~1;
		}
	}
#endif
	autokey_phase = (autokey_phase > 30 ? autokey_phase : 0);
	autokey_shift = 0;
	autokey_enabled = false;

	config.opened_autokey_path.Clear();

	update_config();
}
#endif /* USE_AUTO_KEY */
