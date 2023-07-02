/** @file sdl_input.cpp

	Skelton for retropc emulator
	SDL edition

	@author Sasaji
	@date   2012.02.21

	@brief [ sdl input ]

	@note
	This code is based on win32_input.cpp of the Common Source Code Project.
	Author : Takeda.Toshiya
*/

#include "sdl_emu.h"
#include "../../vm/vm.h"
#include "../../fifo.h"
#include "../../fileio.h"
#include "../../config.h"
#include "../emu_input.h"

void EMU_OSD::EMU_INPUT()
{
#ifdef USE_JOYSTICK
	for(int i=0; i<2; i++) {
		joy[i] = NULL;
	}
#endif

	pressed_global_key = false;
}

void EMU_OSD::initialize_input()
{
#ifdef USE_JOYSTICK
	// initialize joysticks
	if (SDL_InitSubSystem(SDL_INIT_JOYSTICK)) {
		logging->out_logf(LOG_WARN, _T("SDL_InitSubSystem(SDL_INIT_JOYSTICK): %s."), SDL_GetError());
	}

	SDL_JoystickEventState(SDL_IGNORE);
#ifdef USE_SDL2
	// enable joystick status on all window
	SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
#endif
#endif

	EMU::initialize_input();
}

void EMU_OSD::initialize_joystick()
{
#ifdef USE_JOYSTICK
	for(int i = 0; i < 2; i++) {
		joy_xmin[i] = -16384;
		joy_xmax[i] = 16384;
		joy_ymin[i] = -16384;
		joy_ymax[i] = 16384;
	}
#endif

	EMU::initialize_joystick();
}

void EMU_OSD::reset_joystick()
{
#ifdef USE_JOYSTICK
	if (!use_joystick) return;

	joy_num = SDL_NumJoysticks();
	for(int i = 0; i < joy_num && i < 2; i++) {
#ifndef USE_SDL2
		if (!SDL_JoystickOpened(i)) {
#else
		if (!SDL_JoystickGetAttached(joy[i])) {
#endif
			if ((joy[i] = SDL_JoystickOpen(i)) != NULL) {
				joy_enabled[i] = true;
			}
		}
	}
#endif
}

void EMU_OSD::release_input()
{
	// release joystick
	release_joystick();

	EMU::release_input();
}

void EMU_OSD::release_joystick()
{
	EMU::release_joystick();

#ifdef USE_JOYSTICK
	// release joystick
	for(int i = 0; i < 2; i++) {
		if (joy[i] != NULL) {
			SDL_JoystickClose(joy[i]);
		}
		joy[i] = NULL;
	}
#endif
}

void EMU_OSD::update_input()
{
	// release keys
#ifdef USE_AUTO_KEY
	if(lost_focus && !autokey_enabled) {
#else
	if(lost_focus) {
#endif
		// we lost key focus so release all pressed keys
		for(int i = 0; i < KEY_STATUS_SIZE; i++) {
			if(key_status[i] & 0x80) {
				key_status[i] &= 0x7f;
				if (!key_status[i]) {
					vm_key_up(vm_key_map[i], VM_KEY_STATUS_KEYBOARD);
				}
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
				if (!key_status[i]) {
					vm_key_up(vm_key_map[i], VM_KEY_STATUS_KEYBOARD);
				}
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
	if (!initialized) return;

	// update mouse status
	memset(mouse_status, 0, sizeof(mouse_status));
#ifdef USE_MOUSE
	if(!mouse_disabled) {
		// get current status
		VmPoint pt;
		Uint8 mstat = SDL_GetMouseState(&(pt.x),&(pt.y));

#ifndef USE_SDL2
		mouse_status[0]  = pt.x - display_size.w / 2;
		mouse_status[1]  = pt.y - display_size.h / 2;
#else
		mouse_status[0]  = pt.x - screen_size.w / 2;
		mouse_status[1]  = pt.y - screen_size.h / 2;
#endif
		mouse_status[2] =  (mstat & SDL_BUTTON(SDL_BUTTON_LEFT) ? 1 : 0);
		mouse_status[2] |= (mstat & SDL_BUTTON(SDL_BUTTON_RIGHT) ? 2 : 0);
		mouse_status[2] |= (mstat & SDL_BUTTON(SDL_BUTTON_MIDDLE) ? 4 : 0);
		// move mouse cursor to the center of window
		if(!(mouse_status[0] == 0 && mouse_status[1] == 0)) {
#ifndef USE_SDL2
			pt.x = display_size.w / 2;
			pt.y = display_size.h / 2;
			SDL_WarpMouse(pt.x, pt.y);
#else
			pt.x = screen_size.w / 2;
			pt.y = screen_size.h / 2;
			SDL_WarpMouseInWindow(window, pt.x, pt.y);
#endif
		}
	}
#endif
#ifdef USE_LIGHTPEN
	if(FLG_USELIGHTPEN) {
		// get current status
		VmPoint pt;
		Uint8 mstat = SDL_GetMouseState(&(pt.x),&(pt.y));

		if (0 <= pt.x && pt.x < display_size.w && 0 <= pt.y && pt.y < display_size.h) {
			// adjust point
			pt.x = mixed_size.w * (pt.x - stretched_dest_real.x) / stretched_size.w;
			pt.y = mixed_size.h * (pt.y - stretched_dest_real.y) / stretched_size.h;

			mouse_status[0] = pt.x;
			mouse_status[1] = pt.y;
			mouse_status[2] = (mstat & SDL_BUTTON(SDL_BUTTON_LEFT) ? 1 : 0) | (mstat & SDL_BUTTON(SDL_BUTTON_RIGHT) ? 2 : 0);
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
	SDL_JoystickUpdate();
	for(int i = 0; i < 2; i++) {
		if(joy_enabled[i] == true) {
#ifndef USE_SDL2
			if (SDL_JoystickOpened(i)) {
#else
			if (SDL_JoystickGetAttached(joy[i])) {
#endif
				int x = SDL_JoystickGetAxis(joy[i], 0);
				int y = SDL_JoystickGetAxis(joy[i], 1);
				if(y < joy_ymin[i]) joy_status[i] |= 0x01;
				if(y > joy_ymax[i]) joy_status[i] |= 0x02;
				if(x < joy_xmin[i]) joy_status[i] |= 0x04;
				if(x > joy_xmax[i]) joy_status[i] |= 0x08;
				int nbuttons = SDL_JoystickNumButtons(joy[i]);
				for(int n=0; n<nbuttons && n<28; n++) {
					if (SDL_JoystickGetButton(joy[i], n)) {
						joy_status[i] |= (0x10 << n);
					}
				}
			} else {
				joy_enabled[i] = false;
			}
		}
	}
#endif
}

/// @param[in] type   : 0:down 1:up
/// @param[in] code   : SDL key code
/// @param[in] status : scan code
int EMU_OSD::key_down_up(uint8_t type, int code, long status)
{
	bool keep_frames = false;
#ifdef USE_SDL2
	// use scancode because sym code is over 512
	code = (int)status;
#endif
	// translate keycode
	// type change UP to DOWN when capslock key in macosx
	if (!translate_keysym(type, code, (short)status, &code, &keep_frames)) {
		// key down
#ifdef LOG_MEASURE
		logging->out_debugf(_T("SDL_KEYDOWN: code:%08x scancode:%04x")
			,code, scan_code);
#endif
		if (key_mod & KEY_MOD_ALT_KEY) {
			// notify key down
			code = translate_global_key(code);
			system_key_down(code);
			// execute for pressed global key
			execute_global_keys(code, 0);
		} else {
			key_down(code, keep_frames);
		}
	} else {
		// key up
#ifdef LOG_MEASURE
		logging->out_debugf(_T("SDL_KEYUP: code:%08x scancode:%04x")
			,code, scan_code);
#endif
		if (key_mod & KEY_MOD_ALT_KEY) {
			// notify key up
			code = translate_global_key(code);
			system_key_up(code);
			// release global key
			if (release_global_keys(code, 0)) return 0;
		}
		key_up(code, keep_frames);
	}
	return 0;
}

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
		// hide mouse cursor
		SDL_ShowCursor(SDL_DISABLE);
		// move mouse cursor to the center of window
		VmPoint pt;
#ifndef USE_SDL2
		pt.x = display_size.w / 2;
		pt.y = display_size.h / 2;
		SDL_WarpMouse(pt.x, pt.y);
#else
		pt.x = screen_size.w / 2;
		pt.y = screen_size.h / 2;
		SDL_WarpMouseInWindow(window, pt.x, pt.y);
#endif
	}
}

void EMU_OSD::disable_mouse(int mode)
{
	// disable mouse emulation
	if(!mouse_disabled) {
		SDL_ShowCursor(SDL_ENABLE);
	}
	mouse_disabled |= (mode ? 2 : 1);
}

#if 0
void EMU_OSD::toggle_mouse()
{
	// toggle mouse enable / disable
	if(mouse_disabled & 1) {
		enable_mouse(0);
	} else {
		disable_mouse(0);
	}
}

bool EMU_OSD::get_mouse_enabled()
{
	return ((mouse_disabled & 1) == 0);
}
#endif

