/** @file wxw_input.cpp

	Skelton for retropc emulator
	wxWidgets edition

	@author Sasaji
	@date   2012.02.21

	@brief [ wxw input ]

	@note
	This code is based on win32_input.cpp of the Common Source Code Project.
	Author : Takeda.Toshiya
*/

//#include <wx/wx.h>
#ifndef USE_SDL_JOYSTICK
#include <wx/joystick.h>
#endif
#include "wxw_emu.h"
#include "../../vm/vm.h"
#include "../../fifo.h"
#include "../../fileio.h"
#include "../../config.h"
#include "../emu_input.h"

void EMU_OSD::EMU_INPUT()
{
#ifdef USE_JOYSTICK
//	memset(joy_status, 0, sizeof(joy_status));
	for(int i=0; i<2; i++) {
		joy[i] = NULL;
//		joy_enabled[i] = false;
	}
#endif

	pressed_global_key = false;

#ifdef USE_AUTO_KEY
	// initialize autokey
//	autokey_buffer = NULL;
//	autokey_phase = 0;
//	autokey_shift = 0;
//	autokey_enabled = false;
#endif
}

void EMU_OSD::initialize_input()
{
#ifdef USE_JOYSTICK
//	use_joystick = FLG_USEJOYSTICK_ALL ? true : false;
	// initialize joysticks
#ifdef USE_SDL_JOYSTICK
	if (SDL_InitSubSystem(SDL_INIT_JOYSTICK)) {
		logging->out_logf(LOG_WARN, _T("SDL_InitSubSystem(SDL_INIT_JOYSTICK): %s."), SDL_GetError());
	}
#else
	joy_num = wxJoystick::GetNumberJoysticks();
	logging->out_logf(LOG_DEBUG, _T("wxJoystick::GetNumberJoysticks: %d"), joy_num);
#endif
#endif
	EMU::initialize_input();

	// mouse emulation is disabled
//	mouse_disabled = 1;
//	initialize_mouse(FLG_USEMOUSE ? true : false);

#ifdef USE_AUTO_KEY
	// initialize autokey
//	autokey_buffer = new FIFO(65536);
//	autokey_buffer->clear();
//	autokey_phase = 31;	// wait a few sec.
//	autokey_shift = 0;
//	autokey_enabled = false;
#endif
//	lost_focus = false;
//	key_mod = 0;
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
#ifdef USE_SDL_JOYSTICK
	SDL_JoystickEventState(SDL_IGNORE);
#endif
	reset_joystick();
#endif
}

void EMU_OSD::reset_joystick()
{
#ifdef USE_JOYSTICK
	if (!use_joystick) return;

#ifdef USE_SDL_JOYSTICK
	joy_num = SDL_NumJoysticks();
	for(int i = 0; i < joy_num && i < 2; i++) {
#if defined(USE_WX2)
		if (!SDL_JoystickGetAttached(joy[i])) {
#else
		if (!SDL_JoystickOpened(i)) {
#endif
			if ((joy[i] = SDL_JoystickOpen(i)) != NULL) {
				joy_enabled[i] = true;
			}
		}
	}
#else
	joy_num = wxJoystick::GetNumberJoysticks();
	for(int i = 0; i < joy_num && i < 2; i++) {
		delete joy[i];
		joy[i] = new wxJoystick(i);
		if (joy[i]->IsOk()) {
			int offset = (joy[i]->GetXMax() - joy[i]->GetXMin()) / 4;
			joy_xmin[i] = joy[i]->GetXMin() + offset;
			joy_xmax[i] = joy[i]->GetXMax() - offset;
			offset = (joy[i]->GetYMax() - joy[i]->GetYMin()) / 4;
			joy_ymin[i] = joy[i]->GetYMin() + offset;
			joy_ymax[i] = joy[i]->GetYMax() - offset;
			joy_enabled[i] = true;
		}
	}
#endif
#endif
}

void EMU_OSD::release_input()
{
	EMU::release_input();

	// release mouse
//	if(!mouse_disabled) {
//		disable_mouse(0);
//	}
	// release joystick
	release_joystick();

#ifdef USE_AUTO_KEY
	// release autokey buffer
//	if(autokey_buffer) {
//		autokey_buffer->release();
//		delete autokey_buffer;
//	}
#endif
}

void EMU_OSD::release_joystick()
{
	EMU::release_joystick();

#ifdef USE_JOYSTICK
	// release joystick
	for(int i = 0; i < 2; i++) {
#ifdef USE_SDL_JOYSTICK
		if (joy[i] != NULL) {
			SDL_JoystickClose(joy[i]);
		}
#else
		delete joy[i];
#endif
		joy[i] = NULL;
//		joy_enabled[i] = false;
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
//	update_mouse();
}

void EMU_OSD::update_mouse()
{
}

void EMU_OSD::update_mouse_event(wxMouseState &mstat)
{
	if (!initialized) return;

	wxPoint pt;

	// update mouse status
	memset(mouse_status, 0, sizeof(mouse_status));
	if(!mouse_disabled) {
		// get current status
		mouse_status[0]  = mstat.GetX() - display_size.w / 2;
		mouse_status[1]  = mstat.GetY() - display_size.h / 2;
		mouse_status[2] =  (mstat.LeftIsDown() ? 1 : 0);
		mouse_status[2] |= (mstat.RightIsDown() ? 2 : 0);
		mouse_status[2] |= (mstat.MiddleIsDown() ? 4 : 0);
		// move mouse cursor to the center of window
		if(!(mouse_status[0] == 0 && mouse_status[1] == 0)) {
			pt.x = display_size.w / 2;
			pt.y = display_size.h / 2;
			mstat.SetPosition(pt);
		}
	}
#ifdef USE_LIGHTPEN
	if(FLG_USELIGHTPEN) {
		// get current status
		pt.x = mstat.GetX();
		pt.y = mstat.GetY();
		pt.x = screen_size.w * (pt.x - stretched_dest_real.x) / stretched_size.w;
		pt.y = screen_size.h * (pt.y - stretched_dest_real.y) / stretched_size.h;

		mouse_status[0] = pt.x;
		mouse_status[1] = pt.y;
		mouse_status[2] = (mstat.LeftIsDown() ? 1 : 0) | (mstat.RightIsDown() ? 2 : 0);

//		logging->out_debug(_T("mouse: px:%d py:%d 0x%02x"), pt.x, pt.y, mouse_status[2]);
	}
#endif
}

void EMU_OSD::update_joystick()
{
#ifdef USE_JOYSTICK
	memset(joy_status, 0, sizeof(joy_status));

	if (!use_joystick) return;

	// update joystick status
#ifdef USE_SDL_JOYSTICK
	SDL_JoystickUpdate();
	for(int i = 0; i < 2; i++) {
		if(joy_enabled[i] == true) {
#if defined(USE_WX2)
			if (!SDL_JoystickGetAttached(joy[i])) {
#else
			if (SDL_JoystickOpened(i)) {
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
#else
	for(int i = 0; i < 2; i++) {
		if(joy_enabled[i] == true) {
			if (joy[i]->IsOk()) {
				wxPoint xy = joy[i]->GetPosition();
				if(xy.y < joy_ymin[i]) joy_status[i] |= 0x01;
				if(xy.y > joy_ymax[i]) joy_status[i] |= 0x02;
				if(xy.x < joy_xmin[i]) joy_status[i] |= 0x04;
				if(xy.x > joy_xmax[i]) joy_status[i] |= 0x08;
				int nbuttons = joy[i]->GetNumberButtons();
				for(int n=0; n<nbuttons && n<28; n++) {
					if (joy[i]->GetButtonState(n)) {
						joy_status[i] |= (0x10 << n);
					}
				}
			} else {
				joy_enabled[i] = false;
			}
		}
	}
#endif
#endif
}

/// @param[in] type   : 0:down 1:up
/// @param[in] code   : wxWidgets key code
/// @param[in] status : scan code
int EMU_OSD::key_down_up(uint8_t type, int code, long status)
{
	bool keep_frames = false;
	// translate keycode
	// type change UP to DOWN when capslock key in macosx
	if (!translate_keysym(type, code, (short)status, &code, &keep_frames)) {
		// key down
#ifdef LOG_MEASURE
		logging->out_debugf(_T("SDL_KEYDOWN: scancode:%02x sym:%02x(%s) mod:%02x")
			,e->key.keysym.scancode,e->key.keysym.sym,SDL_GetKeyName(e->key.keysym.sym),e->key.keysym.mod);
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
		logging->out_debugf(_T("SDL_KEYUP: scancode:%02x sym:%02x(%s) mod:%02x")
			,e->key.keysym.scancode,e->key.keysym.sym,SDL_GetKeyName(e->key.keysym.sym),e->key.keysym.mod);
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
		wxSetCursor(wxNullCursor);
		// move mouse cursor to the center of window
		wxPoint pt;
		pt.x = display_size.w / 2;
		pt.y = display_size.h / 2;
		wxMouseState ms = wxGetMouseState();
		ms.SetPosition(pt);
	}
}

void EMU_OSD::disable_mouse(int mode)
{
	// disable mouse emulation
	if(!mouse_disabled) {
		wxSetCursor(wxCursor(wxCURSOR_ARROW));
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

