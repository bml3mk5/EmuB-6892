/** @file win_input.cpp

	Skelton for retropc emulator

	@author Takeda.Toshiya
	@date   2006.08.18 -

	@note
	Modified for BML3MK5/MBS1 by Sasaji at 2011.06.17

	@brief [ win32 input ]
*/

#include "win_emu.h"
#include "win_main.h"
#include "../../vm/vm.h"
#include "../../config.h"
#include "../../fileio.h"
#include "../emu_input.h"

void EMU_OSD::EMU_INPUT()
{
#ifdef USE_DIRECTINPUT
	// initialize direct input
	lpdi = NULL;
	lpdikey = NULL;
#endif
}

void EMU_OSD::initialize_input()
{
	EMU::initialize_input();

#ifdef USE_DIRECTINPUT
//	if(config.use_direct_input) {
#if DIRECTINPUT_VERSION >= 0x0800
		if(SUCCEEDED(DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&lpdi, NULL))) {
#else
		if(SUCCEEDED(DirectInputCreate(hInstance, DIRECTINPUT_VERSION, &lpdi, NULL))) {
#endif
			if(SUCCEEDED(lpdi->CreateDevice(GUID_SysKeyboard, &lpdikey, NULL))) {
				if(SUCCEEDED(lpdikey->SetDataFormat(&c_dfDIKeyboard))) {
					if(SUCCEEDED(lpdikey->SetCooperativeLevel(hMainWindow, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE))) {
						config.use_direct_input |= 4;
						memset(key_dik_prev, 0, sizeof(key_dik_prev));
					}
				}
			}
		}
//	}
#endif
}

void EMU_OSD::initialize_joystick()
{
#ifdef USE_JOYSTICK
	memset(joyid_map, 0xff, sizeof(joyid_map));
	memset(&dummy_key, 0, sizeof(dummy_key));
	dummy_key.type = INPUT_KEYBOARD;
	dummy_key.ki.wVk = 0x8f;
	dummy_key.ki.wScan = MapVirtualKey(0x8f, 0);
	dummy_key.ki.dwFlags = KEYEVENTF_EXTENDEDKEY;

	send_dummy_key = 0;
#endif

	EMU::initialize_joystick();
}

void EMU_OSD::reset_joystick()
{
#ifdef USE_JOYSTICK
	if (!use_joystick) return;

	int i = 0;
	joy_num = joyGetNumDevs();
	for(int joyid = 0; joyid < joy_num && joyid < 16 && i < 2; joyid++) {
		JOYINFO joyinfo;
		if(joyGetPos(joyid, &joyinfo) == JOYERR_NOERROR) {
			joyid_map[i] = joyid;

			JOYCAPS joycaps;
			joyGetDevCaps(joyid, &joycaps, sizeof(JOYCAPS));
			uint32_t x = (joycaps.wXmin + joycaps.wXmax) / 2;
			joy_xmin[i] = (joycaps.wXmin + x) / 2;
			joy_xmax[i] = (joycaps.wXmax + x) / 2;
			uint32_t y = (joycaps.wYmin + joycaps.wYmax) / 2;
			joy_ymin[i] = (joycaps.wYmin + y) / 2;
			joy_ymax[i] = (joycaps.wYmax + y) / 2;

			joy_enabled[i] = true;

			i++;
		}
	}
#endif
}

void EMU_OSD::release_input()
{
	EMU::release_input();
}

void EMU_OSD::release_joystick()
{
	EMU::release_joystick();

#ifdef USE_JOYSTICK
	// release joystick
	memset(joyid_map, 0xff, sizeof(joyid_map));
#endif
}

void EMU_OSD::update_input()
{
#ifdef USE_DIRECTINPUT
	if(config.use_direct_input == 5) {
		// direct input
		static uint8_t key_dik[256];
		lpdikey->Acquire();
		lpdikey->GetDeviceState(256, key_dik);

#if DIRECTINPUT_VERSION < 0x0800
		// DIK_RSHIFT is not detected on Vista or later
		if(vista_or_later) {
			key_dik[DIK_RSHIFT] = (GetAsyncKeyState(VK_RSHIFT) & 0x8000) ? 0x80 : 0;
		}
#endif
#ifdef USE_SHIFT_NUMPAD_KEY
		// XXX: don't release shift key while numpad key is pressed
		uint8_t numpad_keys;
		numpad_keys  = key_dik[DIK_NUMPAD0];
		numpad_keys |= key_dik[DIK_NUMPAD1];
		numpad_keys |= key_dik[DIK_NUMPAD2];
		numpad_keys |= key_dik[DIK_NUMPAD3];
		numpad_keys |= key_dik[DIK_NUMPAD4];
		numpad_keys |= key_dik[DIK_NUMPAD5];
		numpad_keys |= key_dik[DIK_NUMPAD6];
		numpad_keys |= key_dik[DIK_NUMPAD7];
		numpad_keys |= key_dik[DIK_NUMPAD8];
		numpad_keys |= key_dik[DIK_NUMPAD9];
		if(numpad_keys & 0x80) {
			key_dik[DIK_LSHIFT] |= key_dik_prev[DIK_LSHIFT];
			key_dik[DIK_RSHIFT] |= key_dik_prev[DIK_RSHIFT];
		}
#endif
//		key_dik[DIK_CIRCUMFLEX] |= key_dik[DIK_EQUALS     ];
//		key_dik[DIK_COLON     ] |= key_dik[DIK_APOSTROPHE ];
//		key_dik[DIK_YEN       ] |= key_dik[DIK_GRAVE      ];
//#ifndef USE_NUMPAD_ENTER
//		key_dik[DIK_RETURN    ] |= key_dik[DIK_NUMPADENTER];
//#endif

		for(int vk = 0; vk < 256; vk++) {
			if(dikey2keycode[vk] != 0 && ((key_dik[vk] ^ key_dik_prev[vk]) & 0x80) != 0) {
				key_down_up((1 - (key_dik[vk] >> 7)) | 2, vk, 0x1ff0000);
			}
		}
		memcpy(key_dik_prev, key_dik, sizeof(key_dik_prev));
	} else {
#endif
#ifdef USE_SHIFT_NUMPAD_KEY
		// update numpad key status
		if(key_shift_pressed && !key_shift_released) {
			if(key_status[VK_SHIFT] == 0) {
				// shift key is newly pressed
				key_status[VK_SHIFT] = 0x80;
#ifdef NOTIFY_KEY_DOWN
				vm->key_down(VK_SHIFT, false);
#endif
			}
		}
		else if(!key_shift_pressed && key_shift_released) {
			if(key_status[VK_SHIFT] != 0) {
				// shift key is newly released
				key_status[VK_SHIFT] = 0;
#ifdef NOTIFY_KEY_DOWN
				vm->key_up(VK_SHIFT);
#endif
				// check l/r shift
				if(!(GetAsyncKeyState(VK_LSHIFT) & 0x8000)) key_status[VK_LSHIFT] &= 0x7f;
				if(!(GetAsyncKeyState(VK_RSHIFT) & 0x8000)) key_status[VK_RSHIFT] &= 0x7f;
			}
		}
		key_shift_pressed = key_shift_released = false;
#endif
#ifdef USE_DIRECTINPUT
	}
#endif

	// release keys
#ifdef USE_AUTO_KEY
	if(lost_focus && autokey_phase == 0) {
#else
	if(lost_focus) {
#endif
		// we lost key focus so release all pressed keys
		for(int i = 0; i < KEY_STATUS_SIZE; i++) {
			if(key_status[i] & 0x80) {
				key_status[i] &= 0x7f;
#ifdef NOTIFY_KEY_DOWN
				if(!key_status[i]) {
					vm->key_up(i);
				}
#endif
			}
		}
	}
	else {
		for(int i = 0; i < KEY_STATUS_SIZE; i++) {
			if(key_status[i] & 0x7f) {
				key_status[i] = (key_status[i] & 0x80) | ((key_status[i] & 0x7f) - 1);
#ifdef NOTIFY_KEY_DOWN
				if(!key_status[i]) {
					vm->key_up(i);
				}
#endif
			}
		}
	}
	lost_focus = false;

#ifdef USE_JOYSTICK
	update_joystick();
#endif
#ifdef USE_KEY_TO_JOY
	// emulate joystick #1 with keyboard
	if(key_status[0x26]) joy_status[0] |= 0x01;	// up
	if(key_status[0x28]) joy_status[0] |= 0x02;	// down
	if(key_status[0x25]) joy_status[0] |= 0x04;	// left
	if(key_status[0x27]) joy_status[0] |= 0x08;	// right
#endif

	// update mouse status
	update_mouse();
}

void EMU_OSD::update_mouse()
{
	memset(mouse_status, 0, sizeof(mouse_status));
#ifdef USE_MOUSE
	if(!mouse_disabled) {
		// get current status
#ifndef USE_MOUSE_ABSOLUTE
		POINT pt;
		GetCursorPos(&pt);
		ScreenToClient(hMainWindow, &pt);
		mouse_status[0]  = pt.x - display_size.w / 2;
		mouse_status[1]  = pt.y - display_size.h / 2;
#else
		POINT pt;
		GetCursorPos(&pt);
		ScreenToClient(hMainWindow, &pt);
		conv_mouse_position(&pt);
		mouse_status[0] = (pt.x & 0x1fff) | (pt.x >= mouse_position.x ? 0 : 0x8000);
		mouse_status[1] = (pt.y & 0x1fff) | (pt.y >= mouse_position.y ? 0 : 0x8000);
		mouse_position.x = pt.x;
		mouse_position.y = pt.y;
#endif
		mouse_status[2]  = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) ? 1 : 0;
		mouse_status[2] |= (GetAsyncKeyState(VK_RBUTTON) & 0x8000) ? 2 : 0;
		mouse_status[2] |= (GetAsyncKeyState(VK_MBUTTON) & 0x8000) ? 4 : 0;
#ifndef USE_MOUSE_ABSOLUTE
		// move mouse cursor to the center of window
		if(!(mouse_status[0] == 0 && mouse_status[1] == 0)) {
			pt.x = display_size.w / 2;
			pt.y = display_size.h / 2;
			ClientToScreen(hMainWindow, &pt);
			SetCursorPos(pt.x, pt.y);
		}
#endif
	}
#endif
#ifdef USE_LIGHTPEN
	if(FLG_USELIGHTPEN) {
		// get current status
		POINT pt;
		GetCursorPos(&pt);
		ScreenToClient(hMainWindow, &pt);

		if (0 <= pt.x && pt.x < display_size.w && 0 <= pt.y && pt.y < display_size.h) {
			// adjust point
			pt.x = mixed_size.w * (pt.x - stretched_dest_real.x) / stretched_size.w;
			pt.y = mixed_size.h * (pt.y - stretched_dest_real.y) / stretched_size.h;

			mouse_status[0] = pt.x;
			mouse_status[1] = pt.y;
			mouse_status[2] = (GetAsyncKeyState(VK_LBUTTON) & 0x8000 ? 1 : 0) | (GetAsyncKeyState(VK_RBUTTON) & 0x8000 ? 2 : 0);
		}
	}
#endif
}

void EMU_OSD::update_joystick()
{
#ifdef USE_JOYSTICK
	memset(joy_status, 0, sizeof(joy_status));

	if (!use_joystick) return;

	// update joystick status
	for(int i = 0; i < 2; i++) {
		JOYINFOEX joyinfoex;
		joyinfoex.dwSize = sizeof(JOYINFOEX);
		joyinfoex.dwFlags = JOY_RETURNALL;
		if(joy_enabled[i] == true && joyid_map[i] != -1) {
			if (joyGetPosEx(joyid_map[i], &joyinfoex) == JOYERR_NOERROR) {
				if(joyinfoex.dwYpos < joy_ymin[i]) joy_status[i] |= 0x01;
				if(joyinfoex.dwYpos > joy_ymax[i]) joy_status[i] |= 0x02;
				if(joyinfoex.dwXpos < joy_xmin[i]) joy_status[i] |= 0x04;
				if(joyinfoex.dwXpos > joy_xmax[i]) joy_status[i] |= 0x08;
				joy_status[i] |= ((joyinfoex.dwButtons & 0x0fffffff) << 4);
			} else {
				joy_enabled[i] = false;
			}
		}
		if (joy_status[i] && send_dummy_key <= 0) {
			// send dummy key event, because delay start time of screen saver.
			SendInput(1, &dummy_key, sizeof(INPUT));
			send_dummy_key = 60;
		} else {
			send_dummy_key -= (send_dummy_key > 0) ? 1 : 0;
		}
	}
#endif
}

/// @param[in] type bit0:0:down 1:up  bit1:0:WM_KEYDOWN/UP 1:direct_input
/// @param[in] code   : VK key code
/// @param[in] status : lParam (scan code etc.)
int EMU_OSD::key_down_up(uint8_t type, int code, long status)
{
//#ifdef USE_DIRECTINPUT
//	if (dinput_key_available && status != 0x1ff0000) {
//		return 1;
//	}
//#endif
	bool keep_frames = false;
	if (!translate_keysym(type, code, status, &code, &keep_frames)) {
		// key down
		if (key_mod & KEY_MOD_ALT_KEY) {
			code = translate_global_key(code);
			system_key_down(code);
			execute_global_keys(code, 0);
			return 0;
		} else {
//			bool repeat = ((HIWORD(lParam) & 0x4000) != 0);
//			key_down(LOBYTE(wParam), repeat);
			key_down(code, keep_frames);
		}
	} else {
		// key up
		if (key_mod & KEY_MOD_ALT_KEY) {
			code = translate_global_key(code);
			system_key_up(code);
			if (release_global_keys(code, 0)) return 0;
		}
//		emu->key_up(LOBYTE(wParam));
		key_up(code, keep_frames);
	}
	return 1;
}

//void EMU::key_down(int code, bool repeat)
int EMU_OSD::key_down(int code, bool keep_frames)
{
#ifdef USE_ORIGINAL_KEYINPUT
	int  orig_code = code;
	bool repeat = ((HIWORD(status) & 0x4000) != 0);
	int  scan_code = (status & 0x1ff0000) >> 16;

	if(code == VK_SHIFT) {
		if(GetAsyncKeyState(VK_LSHIFT) & 0x8000) key_status[VK_LSHIFT] = 0x80;
		if(GetAsyncKeyState(VK_RSHIFT) & 0x8000) key_status[VK_RSHIFT] = 0x80;
		if(!(key_status[VK_LSHIFT] || key_status[VK_RSHIFT])) key_status[VK_LSHIFT] = 0x80;
	}
	else if(code == VK_CONTROL) {
		if(GetAsyncKeyState(VK_LCONTROL) & 0x8000) key_status[VK_LCONTROL] = 0x80;
		if(GetAsyncKeyState(VK_RCONTROL) & 0x8000) key_status[VK_RCONTROL] = 0x80;
		if(!(key_status[VK_LCONTROL] || key_status[VK_RCONTROL])) key_status[VK_LCONTROL] = 0x80;
	}
	else if(code == VK_MENU) {
		if(GetAsyncKeyState(VK_LMENU) & 0x8000) key_status[VK_LMENU] = 0x80;
		if(GetAsyncKeyState(VK_RMENU) & 0x8000) key_status[VK_RMENU] = 0x80;
		if(!(key_status[VK_LMENU] || key_status[VK_RMENU])) key_status[VK_LMENU] = 0x80;
	}
	else if(code == 0xf0) {
		code = VK_CAPITAL;
		keep_frames = true;
	}
#ifdef USE_EMU_INHERENT_SPEC
	else if(code == 0xf1) {
		code = VK_NONCONVERT;
//		keep_frames = true;
	}
#endif
	else if(code == 0xf2) {
		code = VK_KANA;
		keep_frames = true;
	}
	else if(code == 0xf3 || code == 0xf4) {
		code = VK_KANJI;
//		keep_frames = true;
	}

#ifdef USE_EMU_INHERENT_SPEC
	// convert numpad keys
	if (scan_code >= 0x47 && scan_code <= 0x53) {
		if (scancode2vkey[scan_code - 0x47] != 0) code = scancode2vkey[scan_code - 0x47];
	}
#endif

#ifdef USE_SHIFT_NUMPAD_KEY
	if(code == VK_SHIFT) {
		key_shift_pressed = true;
		return;
	}
	else if(numpad_table[code] != 0) {
		if(key_shift_pressed || key_shift_released) {
			key_converted[code] = 1;
			key_shift_pressed = true;
			code = numpad_table[code];
		}
	}
#endif
#ifdef DONT_KEEEP_KEY_PRESSED
		if(!(code == VK_SHIFT || code == VK_CONTROL || code == VK_MENU)) {
			key_status[code] = KEY_KEEP_FRAMES;
		}
		else
#endif
#endif
	key_status[code] = keep_frames ? (KEY_KEEP_FRAMES * FRAME_SPLIT_NUM) : 0x80;
#ifdef NOTIFY_KEY_DOWN
	if(keep_frames) {
		repeat = false;
	}
	vm->key_down(code, repeat);
#endif
	return code;
}

//void EMU::key_up(int code)
void EMU_OSD::key_up(int code, bool keep_frames)
{
#ifdef USE_ORIGINAL_KEYINPUT
	int  orig_code = code;
	int  scan_code = (status & 0x1ff0000) >> 16;

	if(code == VK_SHIFT) {
#ifndef USE_SHIFT_NUMPAD_KEY
		if(!(GetAsyncKeyState(VK_LSHIFT) & 0x8000)) key_status[VK_LSHIFT] &= 0x7f;
		if(!(GetAsyncKeyState(VK_RSHIFT) & 0x8000)) key_status[VK_RSHIFT] &= 0x7f;
#endif
	}
	else if(code == VK_CONTROL) {
		if(!(GetAsyncKeyState(VK_LCONTROL) & 0x8000)) key_status[VK_LCONTROL] &= 0x7f;
		if(!(GetAsyncKeyState(VK_RCONTROL) & 0x8000)) key_status[VK_RCONTROL] &= 0x7f;
	}
	else if(code == VK_MENU) {
		if(!(GetAsyncKeyState(VK_LMENU) & 0x8000)) key_status[VK_LMENU] &= 0x7f;
		if(!(GetAsyncKeyState(VK_RMENU) & 0x8000)) key_status[VK_RMENU] &= 0x7f;
	}
	else if(code == 0xf0) {
		code = VK_CAPITAL;
	}
#ifdef USE_EMU_INHERENT_SPEC
	else if(code == 0xf1) {
		code = VK_NONCONVERT;
	}
#endif
	else if(code == 0xf2) {
		code = VK_KANA;
	}
	else if(code == 0xf3 || code == 0xf4) {
		code = VK_KANJI;
	}

#ifdef USE_EMU_INHERENT_SPEC
	// convert numpad keys
	if (scan_code >= 0x47 && scan_code <= 0x53) {
		if (scancode2vkey[scan_code - 0x47] != 0) code = scancode2vkey[scan_code - 0x47];
	}
#endif

#ifdef USE_SHIFT_NUMPAD_KEY
	if(code == VK_SHIFT) {
		key_shift_pressed = false;
		key_shift_released = true;
		return;
	}
	else if(key_converted[code] != 0) {
		key_converted[code] = 0;
		code = numpad_table[code];
	}
#endif
#endif
//	if(key_status[code]) {
		key_status[code] &= 0x7f;
#ifdef NOTIFY_KEY_DOWN
		if(!key_status[code]) {
			vm->key_up(code);
		}
#endif
//	}
}

#if 0
int EMU_OSD::vm_key_down(int code)
{
#ifdef USE_EMU_INHERENT_SPEC
	code -= 0x80;
#endif
	if (vm_key_status && 0 <= code && code < vm_key_status_size) {
		vm_key_status[code] |= 1;
	}
	return code;
}
void EMU_OSD::vm_key_up(int code)
{
#ifdef USE_EMU_INHERENT_SPEC
	code -= 0x80;
#endif
	if (vm_key_status && 0 <= code && code < vm_key_status_size) {
		vm_key_status[code] &= ~1;
	}
}
#endif

#ifdef USE_BUTTON
void EMU_OSD::press_button(int num)
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

#if 0
void EMU_OSD::initialize_mouse(bool enable)
{
	if (enable) enable_mouse(0);
}
#endif

void EMU_OSD::enable_mouse(int mode)
{
	// enable mouse emulation
	int pmouse_disabled = mouse_disabled;
	mouse_disabled &= (mode ? ~2 : ~1);
	if(pmouse_disabled && !mouse_disabled) {
#ifndef USE_MOUSE_ABSOLUTE
		// hide mouse cursor
		for (int i = 0; ShowCursor(FALSE) >= 0 && i < 10; i++) {}
#endif
		// move mouse cursor to the center of window
#ifndef USE_MOUSE_ABSOLUTE
		POINT pt;
		pt.x = display_size.w / 2;
		pt.y = display_size.h / 2;
		ClientToScreen(hMainWindow, &pt);
		SetCursorPos(pt.x, pt.y);
#else /* USE_MOUSE_ABSOLUTE */
		POINT pt;
		GetCursorPos(&pt);
		ScreenToClient(hMainWindow, &pt);
		conv_mouse_position(&pt);
		vm->set_mouse_position(pt.x, pt.y);
		mouse_position.x = pt.x;
		mouse_position.y = pt.y;
#endif
	}
}

void EMU_OSD::disable_mouse(int mode)
{
	// disable mouse emulation
	if(!mouse_disabled) {
#ifndef USE_MOUSE_ABSOLUTE
		for (int i = 0; ShowCursor(TRUE) < 0 && i < 10; i++) {}
#endif
	}
	mouse_disabled |= (mode ? 2 : 1);
}

#ifdef USE_MOUSE_ABSOLUTE
void EMU_OSD::conv_mouse_position(POINT *pt)
{
	pt->x = (pt->x + 0);
	if (pt->x < 0) pt->x = 0;
	else if (pt->x >= 640) pt->x = 639;
	pt->y = (pt->y - 40);
	if (pt->y < 0) pt->y = 0;
	else if (pt->y >= 400) pt->y = 399;
}
#endif

#if 0 // def USE_MOUSE_ABSOLUTE
void EMU_OSD::set_mouse_position()
{
	POINT pt;
	GetCursorPos(&pt);
//	ScreenToClient(hMainWindow, &pt);
	//
	mouse_position.x = pt.x;
	mouse_position.y = pt.y;
	// convert to vm position
//	mouse_posx = mixed_width  * (mouse_posx + stretched_dest_real.x) / stretched_width - 64;
//	mouse_posy = mixed_height * (mouse_posy + stretched_dest_real.y) / stretched_height - 56;
#if 0
	if (mouse_posx < 0) {
		mouse_posx = 64 * stretched_width / mixed_width - stretched_dest_real.x;
	}
	if (mouse_posx > 639) {
		mouse_posx = (64 + 639) * stretched_width / mixed_width - stretched_dest_real.x;
	}
	if (mouse_posy < 0) {
		mouse_posy = 56 * stretched_height / mixed_height - stretched_dest_real.y;
	}
	if (mouse_posy > 399) {
		mouse_posy = (56 + 399) * stretched_height / mixed_height - stretched_dest_real.y;
	}
#endif
}
#endif

void EMU_OSD::enable_joystick()
{
	// reinitialize
	initialize_joystick();
}
