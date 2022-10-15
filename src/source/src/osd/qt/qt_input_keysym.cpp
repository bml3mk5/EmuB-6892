/** @file qt_input_keysym.cpp

	Skelton for retropc emulator
	SDL edition

	@author Sasaji
	@date   2012.03.08

	@brief [ sdl input keysym ]
*/

#include "qt_emu.h"
#include "../../vm/vm.h"
#include "../../gui/gui.h"
#include "qt_input.h"
#include <QString>
#include <QKeyEvent>

#if 0
/// notify accel key
int EMU_OSD::system_key_down(int code)
{
	if (vm) {
		code = translate_global_key(code);
		vm->system_key_down(code);
	}
	return code;
}
void EMU_OSD::system_key_up(int code)
{
	if (vm) {
		code = translate_global_key(code);
		vm->system_key_up(code);
	}
}
#endif

//uint8_t EMU_OSD::translate_keysym(uint8_t type, int code, long status, int *new_code, bool *new_keep_frames)
//{
//	short scan_code = (short)((status & 0x1ff0000) >> 16);
//    return translate_keysym(type, code, scan_code, 0, new_code, new_keep_frames);
//}

uint8_t EMU_OSD::translate_keysym_(uint8_t type, int code, uint32_t vk_key, uint32_t scan_code, uint32_t mod, int *new_code, bool *new_keep_frames)
{
	int n_code = 0;
	bool n_keep_frames = false;

#ifdef USE_NATIVE_KEYINPUT
#if defined(Q_OS_WIN32)
	// windows
    if(vk_key == 0xf0) {
		n_code = KEYCODE_CAPSLOCK;
		n_keep_frames = true;
    } else if(vk_key == 0xf2) {
		n_code = KEYCODE_KATAHIRA;
		n_keep_frames = true;
	} else {
		if (scan_code < 128) {
			n_code = nativecode2keycode[scan_code];
		} else {
			for(const nc2kc_t *p = nativecode2keycode2; p->nc >= 0; p++) {
				if (p->nc == static_cast<const short>(scan_code)) {
					n_code = p->kc;
					break;
				}
			}
		}
	}
#elif defined(Q_OS_MACX)
    // MacOS X
    if (scan_code) {
        // vk key exist
        if (vk_key < 128) {
            n_code = vkcode2keycode[vk_key];
        }
    }
#elif defined(Q_OS_UNIX)
    // linu etc. (use X11)
    if (scan_code < 144) {
        n_code = scancode2keycode[scan_code];
    }
#endif
#endif
    if (n_code == 0) {
		uint32_t code_h = static_cast<uint32_t>(code) & 0xff000000;
        if (code < 128) {
            n_code = qtkey2keycode[code];
        } else if (code_h == 0x01000000) {
            n_code = qtkey2keycode2[code & 0x7f];
        }
    }
	if (n_code == 0) {
		n_code = code | 0x100;
	}
	if (new_code) *new_code = n_code;
	if (new_keep_frames) *new_keep_frames = n_keep_frames;
	return type;
}

#ifdef USE_OLD_KEYINPUT
/// Translate key codes
void EMU_OSD::translate_qt_keysym(QKeyEvent *e, int *new_key, uint32_t *new_scancode)
{
	translate_qt_keysym(e->key(), e->nativeScanCode(), e->text(), new_key, new_scancode);
}

/// Translate key codes
void EMU_OSD::translate_qt_keysym(int e_key, uint32_t e_scancode, const QString &e_text, int *new_key, uint32_t *new_scancode)
{
	int n_key = 0;
	uint32_t n_scancode = e_scancode;

	if (n_scancode < 128) {
		n_key = nativecode2keycode[n_scancode];
	} else {
		for(int i=0; nativecode2keycode2[i].nc >= 0; i++) {
			if (nativecode2keycode2[i].nc == n_scancode) {
				n_key = nativecode2keycode2[i].kc;
				break;
			}
		}
	}
	if (n_key == 0) {
		n_key = e_key;
		if (n_key & 0xff000000) {
			if ((n_key & 0xff00) == 0x1100) {
				// Japanese Henkan key
				n_key += 0x80;
			}
			n_key = ((n_key >> 16) | (n_key & 0xff)) & 0x1ff;
		} else {
			n_key &= 0x1ff;
		}
	}
#if defined(_WIN32)
	//
	// for windows
	//
#ifdef DEBUG_KEY_TRANS
	out_debugf(_T("[win] translate_qt_keysym: key:%08x -> %03x scancode:%04x -> %04x")
		,e_key, n_key
		,e_scancode, n_scancode
		);
#endif
#endif

	if (new_key != NULL) *new_key = n_key;
	if (new_scancode != NULL) *new_scancode = n_scancode;
}

bool EMU_OSD::translate_key_down(int code, uint16_t mod)
{
#if defined(_WIN32) || (defined(__APPLE__) && defined(__MACH__))
	if(code == KEYCODE_CAPSLOCK) {
		key_status[code] = KEY_KEEP_FRAMES * FRAME_SPLIT_NUM;
		return true;
	}
//	else if(code == SDLK_WORLD_0) {
//		key_status[code] = KEY_KEEP_FRAMES * FRAME_SPLIT_NUM;
//	}
#endif

	return false;
}
#endif /* USE_OLD_KEYINPUT */

#if 0
/// execute accel key
bool EMU_OSD::execute_global_keys(int code, uint32_t status)
{
	if (gui) {
		code = translate_global_key(code);
		return gui->ExecuteGlobalKeys(code, status);
	}
	return false;
}

/// release accel key
bool EMU_OSD::release_global_keys(int code, uint32_t status)
{
	if (gui) {
		code = translate_global_key(code);
		return gui->ReleaseGlobalKeys(code, status);
	}
	return false;
}

//
int EMU_OSD::translate_global_key(int code)
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
