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
#include "../../keycode.h"
#include <QCursor>

#define KEY_KEEP_FRAMES 3

void EMU_OSD::EMU_INPUT()
{
#ifdef USE_QGAMEPAD
	joymgr = nullptr;
	for(int i=0; i<MAX_JOYSTICKS; i++) {
		joy[i] = nullptr;
	}
#endif

	mouse_logic_type = MOUSE_LOGIC_DEFAULT;
	mouse_position.x = mouse_position.y = 0;

	pressed_global_key = false;
}

void EMU_OSD::initialize_input()
{
	EMU::initialize_input();
}

void EMU_OSD::initialize_joystick()
{
#ifdef USE_QGAMEPAD
	joymgr = QGamepadManager::instance();
	for(int i = 0; i < MAX_JOYSTICKS; i++) {
		memset(&joy_prm[i], 0, sizeof(joy_prm[i]));
	}
#endif

	EMU::initialize_joystick();
}

void EMU_OSD::reset_joystick()
{
#ifdef USE_QGAMEPAD
	if (!use_joystick) return;

	const QList<int> pads = joymgr->connectedGamepads();
	for(int i = 0; i < MAX_JOYSTICKS && i < pads.length(); i++) {
		delete joy[i];
		joy[i] = new QGamepad(pads.at(i));
		set_joy_range(true, joy[i]->axisRightX(), joy[i]->axisLeftX(), pConfig->joy_axis_threshold[i][0], joy_prm[i].x);
		set_joy_range(true, joy[i]->axisRightY(), joy[i]->axisLeftY(), pConfig->joy_axis_threshold[i][1], joy_prm[i].y);
		set_joy_range(false, joy[i]->axisRightX(), joy[i]->axisLeftX(), pConfig->joy_axis_threshold[i][2], joy_prm[i].z);
		set_joy_range(false, joy[i]->axisRightY(), joy[i]->axisLeftY(), pConfig->joy_axis_threshold[i][3], joy_prm[i].r);
		set_joy_range(false, joy[i]->axisRightX(), joy[i]->axisLeftX(), pConfig->joy_axis_threshold[i][4], joy_prm[i].u);
		set_joy_range(false, joy[i]->axisRightY(), joy[i]->axisLeftY(), pConfig->joy_axis_threshold[i][5], joy_prm[i].v);
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

#ifdef USE_QGAMEPAD
	// release joystick
	for(int i = 0; i < 2; i++) {
		delete joy[i];
		joy[i] = nullptr;
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

#ifdef USE_MOUSE
	if(!mouse_disabled) {
		// get current status
		QPoint pt = QCursor::pos();

		if (mouse_logic_type == MOUSE_LOGIC_FLEXIBLE) {
			mouse_status[0]  = pt.x() - mouse_position.x;
			mouse_status[1]  = pt.y() - mouse_position.y;
			mouse_position.x = pt.x();
			mouse_position.y = pt.y();

		} else {
			mouse_status[0]  = pt.x() - display_size.w / 2;
			mouse_status[1]  = pt.y() - display_size.h / 2;
			// move mouse cursor to the center of window
			if(!(mouse_status[0] == 0 && mouse_status[1] == 0)) {
				pt.setX(display_size.w / 2);
				pt.setY(display_size.h / 2);
				QCursor::setPos(pt);
			}
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
#if defined(USE_JOYSTICK) || defined(USE_KEY2JOYSTICK)
	memset(joy_status, 0, sizeof(joy_status));
#endif
#ifdef USE_JOYSTICK
	memset(joy2joy_status, 0, sizeof(joy2joy_status));
	if (use_joystick) {
		// update joystick status
#ifdef USE_QGAMEPAD
		for(int i = 0; i < MAX_JOYSTICKS; i++) {
			uint32_t joy_stat = 0;
			if(joy_enabled[i]) {
				QGamepad *pad = joy[i];

				if(pad->buttonUp()) joy_stat |= JOYCODE_UP;
				if(pad->buttonDown()) joy_stat |= JOYCODE_DOWN;
				if(pad->buttonRight()) joy_stat |= JOYCODE_RIGHT;
				if(pad->buttonLeft()) joy_stat |= JOYCODE_LEFT;
				if(pad->buttonA()) joy_stat |= JOYCODE_BTN_A;
				if(pad->buttonB()) joy_stat |= JOYCODE_BTN_B;
				if(pad->buttonX()) joy_stat |= JOYCODE_BTN_C;
				if(pad->buttonY()) joy_stat |= JOYCODE_BTN_D;
				if(pad->buttonSelect()) joy_stat |= JOYCODE_BTN_E;
				if(pad->buttonStart()) joy_stat |= JOYCODE_BTN_F;
				if(pad->buttonL1()) joy_stat |= JOYCODE_BTN_G;
				if(pad->buttonL2() != 0.0) joy_stat |= JOYCODE_BTN_H;
				if(pad->buttonR1()) joy_stat |= JOYCODE_BTN_I;
				if(pad->buttonR2() != 0.0) joy_stat |= JOYCODE_BTN_J;

				// analog 10bits
#ifdef USE_ANALOG_JOYSTICK
				double x = pad->axisLeftX();
				double y = pad->axisLeftY();
				double z = pad->axisRightX();
				double r = pad->axisRightY();
				joy_status[i][1] = (int)((x - joy_prm[i].range.xoffset) * 1024.0 / joy_prm[i].range.xrange + 512.0);
				joy_status[i][2] = (int)((y - joy_prm[i].range.yoffset) * 1024.0 / joy_prm[i].range.yrange + 512.0);
				joy_status[i][3] = (int)((z - joy_prm[i].range.xoffset) * 1024.0 / joy_prm[i].range.xrange + 512.0);
				joy_status[i][4] = (int)((r - joy_prm[i].range.yoffset) * 1024.0 / joy_prm[i].range.yrange + 512.0);
#endif

				joy2joy_status[i][0] = joy_stat;

				// convert
				convert_joy_status(i);
			}
		}
#endif
	}
#endif
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
		// key down
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
		// key up
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

		mouse_logic_type = MOUSE_LOGIC_PREPARE;

		VmPoint pt;
		// move mouse cursor to the center of window
		pt.x = display_size.w / 2;
		pt.y = display_size.h / 2;

		mouse_position.x = pt.x;
		mouse_position.y = pt.y;
		QCursor::setPos(pt.x, pt.y);
	}
}

void EMU_OSD::disable_mouse(int mode)
{
	// disable mouse emulation
	if(!mouse_disabled) {
		// show mouse cursor
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

