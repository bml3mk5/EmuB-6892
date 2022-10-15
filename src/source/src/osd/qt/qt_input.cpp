/** @file qt_input.cpp

	Skelton for retropc emulator
	Qt edition

	@author Sasaji
	@date   2017.02.21

	@brief [ Qt input ]

	@note
	This code is based on win32_input.cpp of the Common Source Code Project.
	Author : Takeda.Toshiya
*/

#include "qt_emu.h"
#include "../../vm/vm.h"
#include "../../fifo.h"
#include "../../fileio.h"
#include "../../config.h"
#include "qt_input.h"

#define KEY_KEEP_FRAMES 3

void EMU_OSD::EMU_INPUT()
{
#ifdef USE_JOYSTICK
	joymgr = nullptr;
	for(int i=0; i<2; i++) {
		joy[i] = nullptr;
		joy_enabled[i] = false;
	}
#endif

	pressed_global_key = false;
}

void EMU_OSD::initialize_input()
{
	EMU::initialize_input();
}

void EMU_OSD::initialize_joystick()
{
#ifdef USE_JOYSTICK
	joymgr = QGamepadManager::instance();
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

	const QList<int> pads = joymgr->connectedGamepads();
	for(int i = 0; i < 2 && i < pads.length(); i++) {
		delete joy[i];
		joy[i] = new QGamepad(pads.at(i));
		joy_enabled[i] = true;
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
		delete joy[i];
		joy[i] = nullptr;
		joy_enabled[i] = false;
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
#if 0
	if (!initialized) return;

	// update mouse status
	memset(mouse_status, 0, sizeof(mouse_status));
	if(!mouse_disabled) {
		// get current status
		VmPoint pt;

//		Uint8_t mstat = SDL_GetMouseState(&(pt.x),&(pt.y));

#ifndef USE_SDL2
		mouse_status[0]  = pt.x - display_size.w / 2;
		mouse_status[1]  = pt.y - display_size.h / 2;
#else
		mouse_status[0]  = pt.x - screen_size.w / 2;
		mouse_status[1]  = pt.y - screen_size.h / 2;
#endif
//		mouse_status[2] =  (mstat & SDL_BUTTON(SDL_BUTTON_LEFT) ? 1 : 0);
//		mouse_status[2] |= (mstat & SDL_BUTTON(SDL_BUTTON_RIGHT) ? 2 : 0);
//		mouse_status[2] |= (mstat & SDL_BUTTON(SDL_BUTTON_MIDDLE) ? 4 : 0);
		// move mouse cursor to the center of window
		if(!(mouse_status[0] == 0 && mouse_status[1] == 0)) {
			pt.x = display_width / 2;
			pt.y = display_height / 2;
//			SDL_WarpMouse(pt.x, pt.y);
		}
	}
#endif

#if 0 //def USE_LIGHTPEN
	if(FLG_USELIGHTPEN) {
		// get current status
		VmPoint pt;
		Uint8_t mstat = SDL_GetMouseState(&(pt.x),&(pt.y));
		pt.x = mixed_width  * (pt.x - stretched_dest_real_x) / stretched_width;
		pt.y = mixed_height * (pt.y - stretched_dest_real_y) / stretched_height;

		mouse_status[0] = pt.x;
		mouse_status[1] = pt.y;
		mouse_status[2] = (mstat & SDL_BUTTON(SDL_BUTTON_LEFT) ? 1 : 0) | (mstat & SDL_BUTTON(SDL_BUTTON_RIGHT) ? 2 : 0);
	}
#endif
}

void EMU_OSD::mouse_down_up(uint8_t UNUSED_PARAM(type), int buttons, int x, int y)
{
	if (!initialized) return;

	// update mouse status
	mouse_status[2] = (buttons & 7);

#ifdef USE_LIGHTPEN
	if(FLG_USELIGHTPEN) {
		// get current status
		x = mixed_size.w * (x - stretched_dest_real.x) / stretched_size.w;
		y = mixed_size.h * (y - stretched_dest_real.y) / stretched_size.h;

		mouse_status[0] = x;
		mouse_status[1] = y;
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
		if(joy_enabled[i] == true) {
			QGamepad *pad = joy[i];

			if(pad->buttonUp()) joy_status[i] |= 0x01;
			if(pad->buttonDown()) joy_status[i] |= 0x02;
			if(pad->buttonRight()) joy_status[i] |= 0x04;
			if(pad->buttonLeft()) joy_status[i] |= 0x08;
			if(pad->buttonA()) joy_status[i] |= 0x10;
			if(pad->buttonB()) joy_status[i] |= 0x20;
			if(pad->buttonX()) joy_status[i] |= 0x40;
			if(pad->buttonY()) joy_status[i] |= 0x80;
			if(pad->buttonSelect()) joy_status[i] |= 0x100;
			if(pad->buttonStart()) joy_status[i] |= 0x200;
			if(pad->buttonL1()) joy_status[i] |= 0x400;
			if(pad->buttonL2() != 0.0) joy_status[i] |= 0x800;
			if(pad->buttonR1()) joy_status[i] |= 0x1000;
			if(pad->buttonR2() != 0.0) joy_status[i] |= 0x2000;
		}
	}
#endif
}

/// @param[in] type   : 0:down 1:up
/// @param[in] code   : Qt key code
/// @param[in] vk_key : vk key
/// @param[in] scan_code : scan code
/// @param[in] mod : modifier
int EMU_OSD::key_down_up_(uint8_t type, int code, uint32_t vk_key, uint32_t scan_code, uint32_t mod)
{
	bool keep_frames = false;
#ifdef USE_SDL2
	// use scancode because sym code is over 512
	code = scan_code;
#endif
	// translate keycode
	// type change UP to DOWN when capslock key in macosx
	if (!translate_keysym_(type, code, vk_key, scan_code, mod, &code, &keep_frames)) {
#ifdef LOG_MEASURE
		out_debugf(_T("KEYDOWN: scancode:%02x code:%02x key_mod:%04x")
			,scan_code, code, key_mod);
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
#ifdef LOG_MEASURE
		out_debugf(_T("KEYUP: scancode:%02x code:%02x key_mod:%04x")
		   ,scan_code, code, key_mod);
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

int EMU_OSD::key_down(int code, bool keep_frames)
{
	key_status[code] = keep_frames ? KEY_KEEP_FRAMES * FRAME_SPLIT_NUM : 0x80;

#ifdef NOTIFY_KEY_DOWN_TO_GUI
	gui->KeyDown(code, mod);
#endif
#ifdef NOTIFY_KEY_DOWN
	vm->key_down(code);
#endif
	return code;
}

void EMU_OSD::key_up(int code, bool UNUSED_PARAM(keep_frames))
{
	if(key_status[code]) {
		key_status[code] &= 0x7f;
#ifdef NOTIFY_KEY_DOWN_TO_GUI
		gui->KeyUp(code);
#endif
#ifdef NOTIFY_KEY_DOWN
		if(!key_status[code]) {
			vm->key_up(code);
		}
#endif
	}
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
		// hide mouse cursor
		// move mouse cursor to the center of window
		VmPoint pt;
		pt.x = display_size.w / 2;
		pt.y = display_size.h / 2;

	}
}

void EMU_OSD::disable_mouse(int mode)
{
	// disable mouse emulation
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

