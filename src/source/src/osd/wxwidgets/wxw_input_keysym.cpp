/** @file wxw_input_keysym.cpp

	Skelton for retropc emulator
	wxWidgets edition

	@author Sasaji
	@date   2012.03.08

	@brief [ wxw input keysym ]
*/

#include <wx/wx.h>
#include "wxw_emu.h"
#include "../../vm/vm.h"
#include "wxw_main.h"
#include "../../gui/gui.h"
#include "wxw_input.h"

#if 0
/// notify accel key
int EMU::system_key_down(int code)
{
	if (vm) {
		code = translate_global_key(code);
		vm->system_key_down(code);
	}
	return code;
}
void EMU::system_key_up(int code)
{
	if (vm) {
		code = translate_global_key(code);
		vm->system_key_up(code);
	}
}
#endif

uint8_t EMU_OSD::translate_keysym(uint8_t type, int code, long status, int *new_code, bool *new_keep_frames)
{
	short scan_code = (short)((status & 0x1ff0000) >> 16);
	return translate_keysym(type, code, scan_code, new_code, new_keep_frames);
}

uint8_t EMU_OSD::translate_keysym(uint8_t type, int code, short scan_code, int *new_code, bool *new_keep_frames)
{
	int n_code = 0;
	bool n_keep_frames = false;
	uint8_t n_type = type;

#if defined(__WXMSW__)
	/* Windows */
	if(code == VK_SHIFT) {
		if(scan_code == 0x36 || GetKeyState(VK_RSHIFT) & 0x8000) n_code = KEYCODE_RSHIFT;
		else n_code = KEYCODE_LSHIFT;
	}
	else if(code == VK_CONTROL) {
		if((scan_code & 0x100) || GetKeyState(VK_RCONTROL) & 0x8000) n_code = KEYCODE_RCTRL;
		else n_code = KEYCODE_LCTRL;
	}
	else if(code == VK_MENU) {
		if((scan_code & 0x100) || GetKeyState(VK_RMENU) & 0x8000) n_code = KEYCODE_RALT;
		else n_code = KEYCODE_LALT;
	}
	else if(code == VK_RETURN) {
		if (scan_code & 0x100) n_code = KEYCODE_KP_ENTER;
		else n_code = KEYCODE_RETURN;
	}
	else if(code == 0xf0) {
		n_code = KEYCODE_CAPSLOCK;
		n_keep_frames = true;
	}
	else if(code == 0xf2) {
		n_code = KEYCODE_KATAHIRA;
		n_keep_frames = true;
	}
	else if (code < 256) {
		n_code = vkkey2keycode[code];
	}

	// convert numpad keys
	if (scan_code >= 0x47 && scan_code <= 0x53) {
		if (scancode2keycode47[scan_code - 0x47] != 0) n_code = scancode2keycode47[scan_code - 0x47];
	}
	else if (scan_code >= 0x70 && scan_code <= 0x7f) {
		if (scancode2keycode70[scan_code - 0x70] != 0) n_code = scancode2keycode70[scan_code - 0x70];
	}
	if (n_code == 0) {
		n_code = code | 0x100;
	}
#elif defined(__WXOSX__)
	/* Apple MacOSX */
	if(code == 0x39) {
		// caps lock
		// toggled on/off when every push this key.
		n_code = KEYCODE_CAPSLOCK;
		n_keep_frames = true;
		n_type = 0;	// always keydown
	} else if (code < 128) {
		n_code = scancode2keycode[code];
#if 0
	} else if (code == 0) {
		for(const ks2kc_t *p = keysym2keycode; p->ks >= 0; p++) {
			if (p->ks == scan_code) {
				n_code = p->kc;
				break;
			}
		}
#endif
	} else if (code == 255) {
		n_type = 1;
	} else {
		n_code = (code & 0x1ff) | 0x100;
	}
#elif defined(__WXGTK__)
	/* gtk */
	if ((code & 0xff00) == 0xff00) {
		n_code = gdkkey2keycodeFF[code & 0xff];
	} else if ((code & 0xff00) == 0) {
		n_code = gdkkey2keycode[code & 0xff];
	}
#endif

	if (new_code) *new_code = n_code;
	if (new_keep_frames) *new_keep_frames = n_keep_frames;

	return n_type;
}

#ifdef USE_OLD_KEYINPUT
/// Translate key code
void EMU::translate_wxw_keysym(int &code, int &unicode, uint32_t &rawcode)
{
	logging->out_debugf(_T("Keycode: code:0x%04x unicode:0x%04x rawcode:0x%04x"),code,unicode,rawcode);

#if defined(__WXMSW__)
	//
	// for windows
	//
	switch(rawcode) {
	case 0x1c:
		// henkan
		code = WXK_SPECIAL1;
		break;
	case 0x1d:
		// muhenkan
		code = WXK_SPECIAL2;
		break;
	case 0xe2:
		// under score
		code = '_';
		break;
	case 0xf0:
		// caps
		code = WXK_CAPITAL;
		break;
	case 0xf2:
		// kata / hira
		code = WXK_SPECIAL3;
		break;
	case 0xf4:
		// zenkaku / hankaku
		code = WXK_SPECIAL4;
		break;
	}

	switch(code) {
	case WXK_NUMPAD_DELETE:
		code = WXK_NUMPAD_DECIMAL;
		break;
	case WXK_NUMPAD_INSERT:
		code = WXK_NUMPAD0;
		break;
	case WXK_NUMPAD_END:
		code = WXK_NUMPAD1;
		break;
	case WXK_NUMPAD_DOWN:
		code = WXK_NUMPAD2;
		break;
	case WXK_NUMPAD_PAGEDOWN:
		code = WXK_NUMPAD3;
		break;
	case WXK_NUMPAD_LEFT:
		code = WXK_NUMPAD4;
		break;
	case WXK_CLEAR:
		code = WXK_NUMPAD5;
		break;
	case WXK_NUMPAD_RIGHT:
		code = WXK_NUMPAD6;
		break;
	case WXK_NUMPAD_HOME:
		code = WXK_NUMPAD7;
		break;
	case WXK_NUMPAD_UP:
		code = WXK_NUMPAD8;
		break;
	case WXK_NUMPAD_PAGEUP:
		code = WXK_NUMPAD9;
		break;
	}
#elif defined(__WXGTK__)
	//
	// for GTK (linux , FreeBSD etc)
	//
	switch(rawcode) {
	case 0x1c:
		// henkan
		code = WXK_SPECIAL1;
		break;
	case 0x1d:
		// muhenkan
		code = WXK_SPECIAL2;
		break;
	case 0xe2:
		// under score
		code = '_';
		break;
	case 0xf0:
		// caps
		code = WXK_CAPITAL;
		break;
	case 0xf2:
		// kata / hira
		code = WXK_SPECIAL3;
		break;
	case 0xf4:
		// zenkaku / hankaku
		code = WXK_SPECIAL4;
		break;
	case 0xffeb:
		// windows key
		code = WXK_WINDOWS_LEFT;
		break;
	}

	switch(code) {
	case WXK_NUMPAD_DELETE:
		code = WXK_NUMPAD_DECIMAL;
		break;
	case WXK_NUMPAD_INSERT:
		code = WXK_NUMPAD0;
		break;
	case WXK_NUMPAD_END:
		code = WXK_NUMPAD1;
		break;
	case WXK_NUMPAD_DOWN:
		code = WXK_NUMPAD2;
		break;
	case WXK_NUMPAD_PAGEDOWN:
		code = WXK_NUMPAD3;
		break;
	case WXK_NUMPAD_LEFT:
		code = WXK_NUMPAD4;
		break;
	case 0x17f:
		code = WXK_NUMPAD5;
		break;
	case WXK_NUMPAD_RIGHT:
		code = WXK_NUMPAD6;
		break;
	case WXK_NUMPAD_HOME:
		code = WXK_NUMPAD7;
		break;
	case WXK_NUMPAD_UP:
		code = WXK_NUMPAD8;
		break;
	case WXK_NUMPAD_PAGEUP:
		code = WXK_NUMPAD9;
		break;
	}
#elif defined(__WXOSX__)
	//
	// for MacOS X
	//
	switch(rawcode) {
	case 0x36:	// rcommand
		code = WXK_SPECIAL5;
		break;
	case 0x37:	// lcommand
		code = WXK_SPECIAL6;
		break;
	}
	if (code >= 0x21 && code <= 0x2b) {
		code += 0x10;
	} else if (code >= 0x3c && code <= 0x3f) {
		code -= 0x10;
	} else if (code >= 0x60 && code <= 0x7f) {
		code -= 0x20;
	} else if (code == 0xa5) {
		code = 0x5c;
	}
#endif
	if (code == WXK_NONE) {
		code = unicode;
	}
	logging->out_debugf(_T("->trans: code:0x%04x unicode:0x%04x rawcode:0x%04x"),code,unicode,rawcode);
}

bool EMU::translate_key_down(int code, uint16_t mod)
{
#if defined(_WIN32) || (defined(__APPLE__) && defined(__MACH__))
	if(code == WXK_CAPITAL || code == WXK_SPECIAL3) {
		key_status[code] = KEY_KEEP_FRAMES * FRAME_SPLIT_NUM;
		return true;
	}
#endif

	return false;
}
#endif /* USE_OLD_KEYINPUT */

#if 0
/// execute accel key
bool EMU::execute_global_keys(int code, uint32_t status)
{
	if (gui) {
		code = translate_global_key(code);
		return gui->ExecuteGlobalKeys(code, status);
	}
	return false;
}

/// release accel key
bool EMU::release_global_keys(int code, uint32_t status)
{
	if (gui) {
		code = translate_global_key(code);
		return gui->ReleaseGlobalKeys(code, status);
	}
	return false;
}

//
int EMU::translate_global_key(int code)
{
	if (code == KEYCODE_RETURN) {
		code = GLOBALKEY_RETURN;
	} else if (code == KEYCODE_LCTRL || code == KEYCODE_RCTRL) {
			code = GLOBALKEY_CONTROL;
	} else if (code == KEYCODE_0) {
		code = GLOBALKEY_0;
	} else if (KEYCODE_1 <= code && code <= KEYCODE_9) {
		code = code + GLOBALKEY_1 - KEYCODE_1;
	} else if (KEYCODE_A <= code && code <= KEYCODE_Z) {
		code = code + GLOBALKEY_A - KEYCODE_A;
	} else if (KEYCODE_F1 <= code && code <= KEYCODE_F12) {
		code = code + GLOBALKEY_F1 - KEYCODE_F1;
	} else if (KEYCODE_F13 <= code && code <= KEYCODE_F15) {
		code = code + GLOBALKEY_F13 - KEYCODE_F13;
	}
	return code;
}
#endif
