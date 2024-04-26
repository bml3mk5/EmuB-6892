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
	for(int i=0; i<MAX_JOYSTICKS; i++) {
		joy[i] = NULL;
	}
#endif

	mouse_logic_type = MOUSE_LOGIC_DEFAULT;
	mouse_position.x = mouse_position.y = 0;

	pressed_global_key = false;
}

void EMU_OSD::initialize_input()
{
#ifdef USE_JOYSTICK
	// initialize joysticks
# ifdef USE_SDL_JOYSTICK
	if (SDL_InitSubSystem(SDL_INIT_JOYSTICK)) {
		logging->out_logf(LOG_WARN, _T("SDL_InitSubSystem(SDL_INIT_JOYSTICK): %s."), SDL_GetError());
	}

	SDL_JoystickEventState(SDL_IGNORE);
#ifdef USE_WX2
	// enable joystick status on all window
	SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
#endif
# else
	int joy_num = wxJoystick::GetNumberJoysticks();
	logging->out_logf(LOG_DEBUG, _T("wxJoystick::GetNumberJoysticks: %d"), joy_num);
# endif
#endif

	EMU::initialize_input();
}

void EMU_OSD::initialize_joystick()
{
#ifdef USE_JOYSTICK
	for(int i=0; i<MAX_JOYSTICKS; i++) {
		memset(&joy_prm[i], 0, sizeof(joy_prm[i]));
	}
#endif

	EMU::initialize_joystick();
}

void EMU_OSD::reset_joystick()
{
#ifdef USE_JOYSTICK
	if (!use_joystick) return;

# ifdef USE_SDL_JOYSTICK
	int joy_num = SDL_NumJoysticks();
	for(int i = 0; i < joy_num && i < MAX_JOYSTICKS; i++) {
#if !defined(USE_WX2)
		if (!SDL_JoystickOpened(i)) {
#else
		if (!SDL_JoystickGetAttached(joy[i])) {
#endif
			if ((joy[i] = SDL_JoystickOpen(i)) != NULL) {
				int axes = SDL_JoystickNumAxes(joy[i]);
				set_joy_range(axes >= 1, -32768, 32767, joy_prm[i].x);
				set_joy_range(axes >= 2, -32768, 32767, joy_prm[i].y);
				set_joy_range(axes >= 3, -32768, 32767, joy_prm[i].z);
				set_joy_range(axes >= 4, -32768, 32767, joy_prm[i].u);
				set_joy_range(axes >= 5, -32768, 32767, joy_prm[i].v);
				set_joy_range(axes >= 6, -32768, 32767, joy_prm[i].r);

				joy_prm[i].has_pov = SDL_JoystickNumHats(joy[i]) >= 1 ? 1 : 0;

#ifndef USE_WX2
				CTchar cstr(SDL_JoystickName(i));
#else
				CTchar cstr(SDL_JoystickName(joy[i]));
#endif
				logging->out_logf(LOG_DEBUG, _T("Joypad #%d: %s"), i, cstr.Get());
				joy_enabled[i] = true;
			}
		}
	}
# else
	int joy_num = wxJoystick::GetNumberJoysticks();
	for(int i = 0; i < joy_num && i < MAX_JOYSTICKS; i++) {
		delete joy[i];
		joy[i] = new wxJoystick(i);
		if (joy[i]->IsOk()) {
			int axes = joy[i]->GetNumberAxes();
			set_joy_range(axes >= 1, joy[i]->GetXMin(),joy[i]->GetXMax(), pConfig->joy_axis_threshold[i][0], joy_prm[i].x);
			set_joy_range(axes >= 2, joy[i]->GetYMin(),joy[i]->GetYMax(), pConfig->joy_axis_threshold[i][1], joy_prm[i].y);
			set_joy_range(joy[i]->HasZ(), joy[i]->GetZMin(),joy[i]->GetZMax(), pConfig->joy_axis_threshold[i][2], joy_prm[i].z);
			set_joy_range(joy[i]->HasRudder(), joy[i]->GetRudderMin(),joy[i]->GetRudderMax(), pConfig->joy_axis_threshold[i][3], joy_prm[i].r);
			set_joy_range(joy[i]->HasU(), joy[i]->GetUMin(),joy[i]->GetUMax(), pConfig->joy_axis_threshold[i][4], joy_prm[i].u);
			set_joy_range(joy[i]->HasV(), joy[i]->GetVMin(),joy[i]->GetVMax(), pConfig->joy_axis_threshold[i][5], joy_prm[i].v);

			joy_prm[i].has_pov = joy[i]->HasPOV() ? 1 : 0;

			logging->out_logf(LOG_DEBUG, _T("Joypad #%d: %s"), i, joy[i]->GetProductName().t_str());

			joy_enabled[i] = true;
		}
	}
# endif
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
	for(int i = 0; i < MAX_JOYSTICKS; i++) {
# ifdef USE_SDL_JOYSTICK
		if (joy[i] != NULL) {
			SDL_JoystickClose(joy[i]);
		}
# else
		delete joy[i];
# endif
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

	// update joystick status
	update_joystick();

	// update mouse status
	update_mouse();
}

void EMU_OSD::update_mouse()
{
	if (!initialized) return;

	// update mouse status
	memset(mouse_status, 0, sizeof(mouse_status));
	wxMouseState mstat = wxGetMouseState();
#ifdef USE_MOUSE
	if(!mouse_disabled) {
		// get current status
		wxPoint pt;
		pt.x = mstat.GetX();
		pt.y = mstat.GetY();

		mouse_status[2] =  (mstat.LeftIsDown() ? 1 : 0);
		mouse_status[2] |= (mstat.RightIsDown() ? 2 : 0);
		mouse_status[2] |= (mstat.MiddleIsDown() ? 4 : 0);

		switch(mouse_logic_type) {
		case MOUSE_LOGIC_FLEXIBLE:
			mouse_status[0]  = pt.x - mouse_position.x;
			mouse_status[1]  = pt.y - mouse_position.y;
			mouse_position.x = pt.x;
			mouse_position.y = pt.y;
			break;

		case MOUSE_LOGIC_DEFAULT:
			mouse_status[0]  = pt.x - display_size.w / 2;
			mouse_status[1]  = pt.y - display_size.h / 2;
			// move mouse cursor to the center of window
			if(!(mouse_status[0] == 0 && mouse_status[1] == 0)) {
				pt.x = display_size.w / 2;
				pt.y = display_size.h / 2;
				mstat.SetPosition(pt);
			}
			break;

		default:
			break;
		}
	}
#endif
#ifdef USE_LIGHTPEN
	if(FLG_USELIGHTPEN) {
		// get current status
		wxPoint pt;
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

//void EMU_OSD::update_mouse_event(wxMouseState &mstat)
//{
//}

void EMU_OSD::update_joystick()
{
#if defined(USE_JOYSTICK) || defined(USE_KEY2JOYSTICK)
	memset(joy_status, 0, sizeof(joy_status));
#endif
#ifdef USE_JOYSTICK
	memset(joy2joy_status, 0, sizeof(joy2joy_status));
	if (use_joystick) {
		// update joystick status
# ifdef USE_SDL_JOYSTICK
		SDL_JoystickUpdate();
		for(int i = 0; i < MAX_JOYSTICKS; i++) {
			uint32_t joy_stat = 0;
			if(joy_enabled[i]) {
#if !defined(USE_WX2)
				if (SDL_JoystickOpened(i)) {
#else
				if (SDL_JoystickGetAttached(joy[i])) {
#endif
					int axes = SDL_JoystickNumAxes(joy[i]);
					int x = SDL_JoystickGetAxis(joy[i], 0);
					int y = SDL_JoystickGetAxis(joy[i], 1);
					if(y < joy_prm[i].y.mintd) joy_stat |= JOYCODE_UP;
					if(y > joy_prm[i].y.maxtd) joy_stat |= JOYCODE_DOWN;
					if(x < joy_prm[i].x.mintd) joy_stat |= JOYCODE_LEFT;
					if(x > joy_prm[i].x.maxtd) joy_stat |= JOYCODE_RIGHT;

					int z = axes >= 3 ? SDL_JoystickGetAxis(joy[i], 2) : joy_prm[i].z.offset;
					int r = axes >= 6 ? SDL_JoystickGetAxis(joy[i], 5) : joy_prm[i].r.offset;
					if(r < joy_prm[i].r.mintd) joy_stat |= JOYCODE_R_UP;
					if(r > joy_prm[i].r.maxtd) joy_stat |= JOYCODE_R_DOWN;
					if(z < joy_prm[i].z.mintd) joy_stat |= JOYCODE_Z_LEFT;
					if(z > joy_prm[i].z.maxtd) joy_stat |= JOYCODE_Z_RIGHT;

					int u = axes >= 4 ? SDL_JoystickGetAxis(joy[i], 3) : joy_prm[i].u.offset;
					int v = axes >= 5 ? SDL_JoystickGetAxis(joy[i], 4) : joy_prm[i].v.offset;
					if(v < joy_prm[i].v.mintd) joy_stat |= JOYCODE_V_UP;
					if(v > joy_prm[i].v.maxtd) joy_stat |= JOYCODE_V_DOWN;
					if(u < joy_prm[i].u.mintd) joy_stat |= JOYCODE_U_LEFT;
					if(u > joy_prm[i].u.maxtd) joy_stat |= JOYCODE_U_RIGHT;

					int nbuttons = SDL_JoystickNumButtons(joy[i]);
					for(int n=0; n<nbuttons && n<16; n++) {
						if (SDL_JoystickGetButton(joy[i], n)) {
							joy_stat |= (0x10000 << n);
						}
					}

					if (joy_prm[i].has_pov) {
						int hatpos = SDL_JoystickGetHat(joy[i], 0);
						switch(hatpos) {
						case SDL_HAT_UP:
							joy_stat |= JOYCODE_POV_UP;
							break;
						case SDL_HAT_RIGHTUP:
							joy_stat |= JOYCODE_POV_UPRIGHT;
							break;
						case SDL_HAT_RIGHT:
							joy_stat |= JOYCODE_POV_RIGHT;
							break;
						case SDL_HAT_RIGHTDOWN:
							joy_stat |= JOYCODE_POV_DOWNRIGHT;
							break;
						case SDL_HAT_DOWN:
							joy_stat |= JOYCODE_POV_DOWN;
							break;
						case SDL_HAT_LEFTDOWN:
							joy_stat |= JOYCODE_POV_DOWNLEFT;
							break;
						case SDL_HAT_LEFT:
							joy_stat |= JOYCODE_POV_LEFT;
							break;
						case SDL_HAT_LEFTUP:
							joy_stat |= JOYCODE_POV_UPLEFT;
							break;
						}
					}

#ifdef USE_ANALOG_JOYSTICK
					// analog 10bits
					joy2joy_status[i][2] = (x - joy_prm[i].x.offset) * 1024 / joy_prm[i].x.range + 512;
					joy2joy_status[i][3] = (y - joy_prm[i].y.offset) * 1024 / joy_prm[i].y.range + 512;
					joy2joy_status[i][4] = (z - joy_prm[i].z.offset) * 1024 / joy_prm[i].z.range + 512;
					joy2joy_status[i][5] = (r - joy_prm[i].r.offset) * 1024 / joy_prm[i].r.range + 512;
					joy2joy_status[i][6] = (u - joy_prm[i].u.offset) * 1024 / joy_prm[i].u.range + 512;
					joy2joy_status[i][7] = (v - joy_prm[i].v.offset) * 1024 / joy_prm[i].v.range + 512;
#endif
				} else {
					joy_enabled[i] = false;
				}
			}
			joy2joy_status[i][0] = joy_stat;

			// convert
			convert_joy_status(i);
		}
# else
		for(int i = 0; i < MAX_JOYSTICKS; i++) {
			uint32_t joy_stat = 0;
			if(joy_enabled[i]) {
				if (joy[i]->IsOk()) {
					int y = joy[i]->GetPosition(1);
					int x = joy[i]->GetPosition(0);
					if(y < joy_prm[i].y.mintd) joy_stat |= JOYCODE_UP;
					if(y > joy_prm[i].y.maxtd) joy_stat |= JOYCODE_DOWN;
					if(x < joy_prm[i].x.mintd) joy_stat |= JOYCODE_LEFT;
					if(x > joy_prm[i].x.maxtd) joy_stat |= JOYCODE_RIGHT;

					int r = joy[i]->GetRudderPosition();
					if(r < joy_prm[i].r.mintd) joy_stat |= JOYCODE_R_UP;
					if(r > joy_prm[i].r.maxtd) joy_stat |= JOYCODE_R_DOWN;

					int z = joy[i]->GetZPosition();
					if(z < joy_prm[i].z.mintd) joy_stat |= JOYCODE_Z_LEFT;
					if(z > joy_prm[i].z.maxtd) joy_stat |= JOYCODE_Z_RIGHT;

					int v = joy[i]->GetVPosition();
					if(v < joy_prm[i].v.mintd) joy_stat |= JOYCODE_V_UP;
					if(v > joy_prm[i].v.maxtd) joy_stat |= JOYCODE_V_DOWN;

					int u = joy[i]->GetUPosition();
					if(u < joy_prm[i].u.mintd) joy_stat |= JOYCODE_U_LEFT;
					if(u > joy_prm[i].u.maxtd) joy_stat |= JOYCODE_U_RIGHT;

					joy_stat |= (joy[i]->GetButtonState() << 16);

					if (joy_prm[i].has_pov) {
						int hatpos = joy[i]->GetPOVPosition();
						switch(hatpos) {
						case SDL_HAT_UP:
							joy_stat |= JOYCODE_POV_UP;
							break;
						case SDL_HAT_RIGHTUP:
							joy_stat |= JOYCODE_POV_UPRIGHT;
							break;
						case SDL_HAT_RIGHT:
							joy_stat |= JOYCODE_POV_RIGHT;
							break;
						case SDL_HAT_RIGHTDOWN:
							joy_stat |= JOYCODE_POV_DOWNRIGHT;
							break;
						case SDL_HAT_DOWN:
							joy_stat |= JOYCODE_POV_DOWN;
							break;
						case SDL_HAT_LEFTDOWN:
							joy_stat |= JOYCODE_POV_DOWNLEFT;
							break;
						case SDL_HAT_LEFT:
							joy_stat |= JOYCODE_POV_LEFT;
							break;
						case SDL_HAT_LEFTUP:
							joy_stat |= JOYCODE_POV_UPLEFT;
							break;
						}
					}

#ifdef USE_ANALOG_JOYSTICK
					// analog 10bits
					joy2joy_status[i][2] = (x - joy_prm[i].x.offset) * 1024 / joy_prm[i].x.range + 512;
					joy2joy_status[i][3] = (y - joy_prm[i].y.offset) * 1024 / joy_prm[i].y.range + 512;
					joy2joy_status[i][4] = (z - joy_prm[i].z.offset) * 1024 / joy_prm[i].z.range + 512;
					joy2joy_status[i][5] = (r - joy_prm[i].r.offset) * 1024 / joy_prm[i].r.range + 512;
					joy2joy_status[i][6] = (u - joy_prm[i].u.offset) * 1024 / joy_prm[i].u.range + 512;
					joy2joy_status[i][7] = (v - joy_prm[i].v.offset) * 1024 / joy_prm[i].v.range + 512;
#endif
				} else {
					joy_enabled[i] = false;
				}
				joy2joy_status[i][0] = joy_stat;

				// convert
				convert_joy_status(i);
			}
		}
# endif
	}
#endif // USE_JOYSTICK
#ifdef USE_KEY2JOYSTICK
	if (key2joy_enabled) {
#ifndef USE_PIAJOYSTICKBIT
		// update key 2 joystick status
		for(int i = 0; i < MAX_JOYSTICKS; i++) {
			for(int k = 0; k < 9; k++) {
				joy_status[i][0] |= key2joy_status[i][k];
			}
		}
#else
		for(int i = 0; i < MAX_JOYSTICKS; i++) {
			joy_status[i][0] |= key2joy_status[i];
		}
#endif
	}
#endif
#if defined(USE_JOYSTICK) || defined(USE_KEY2JOYSTICK)
	for(int i = 0; i < MAX_JOYSTICKS; i++) {
		joy_status[i][0] &= joy_mashing_mask[i][joy_mashing_count];
	}
	joy_mashing_count++;
	joy_mashing_count &= 0xf;
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

		mouse_logic_type = MOUSE_LOGIC_PREPARE;

		wxPoint pt;
		// move mouse cursor to the center of window
		pt.x = display_size.w / 2;
		pt.y = display_size.h / 2;

		mouse_position.x = pt.x;
		mouse_position.y = pt.y;
		wxMouseState ms = wxGetMouseState();
		ms.SetPosition(pt);
	}
}

void EMU_OSD::disable_mouse(int mode)
{
	// disable mouse emulation
	if(!mouse_disabled) {
		// show mouse cursor
		wxSetCursor(wxCursor(wxCURSOR_ARROW));
	}
	mouse_disabled |= (mode ? 2 : 1);
}

void EMU_OSD::mouse_enter()
{
}

void EMU_OSD::mouse_move(int x, int y)
{
	if (mouse_logic_type == MOUSE_LOGIC_PREPARE) {
		if (mouse_position.x == x
		 && mouse_position.y == y) {
			// mouse warped
			mouse_logic_type = MOUSE_LOGIC_DEFAULT;
		} else {
			mouse_position.x = x;
			mouse_position.y = y;
			mouse_logic_type = MOUSE_LOGIC_FLEXIBLE;
		}
	}
}

void EMU_OSD::mouse_leave()
{
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

