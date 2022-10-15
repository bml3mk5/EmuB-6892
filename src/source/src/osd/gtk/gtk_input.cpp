/** @file gtk_input.cpp

	Skelton for retropc emulator
	GTK+ + SDL edition

	@author Sasaji
	@date   2017.01.21

	@brief [ gtk input ]

	@note
	This code is based on win32_input.cpp of the Common Source Code Project.
	Author : Takeda.Toshiya
*/

#include "../../emu.h"
#include "../../vm/vm.h"
//#include "../../fifo.h"
#include "../../fileio.h"
#include "../../config.h"
#include "../emu_input.h"
#include <gtk/gtk.h>
#include <gdk/gdk.h>

void EMU_OSD::EMU_INPUT()
{
#ifdef USE_JOYSTICK
	for(int i=0; i<2; i++) {
		joy[i] = NULL;
	}
#endif

	bkup_cursor = NULL;
	pressed_global_key = false;
}

void EMU_OSD::initialize_input()
{
#ifdef USE_JOYSTICK
	// initialize joysticks
	if (SDL_InitSubSystem(SDL_INIT_JOYSTICK)) {
		logging->out_logf(LOG_WARN, _T("SDL_InitSubSystem(SDL_INIT_JOYSTICK): %s."), SDL_GetError());
	}
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
	SDL_JoystickEventState(SDL_IGNORE);
	reset_joystick();
#endif
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
#ifdef USE_JOYSTICK
	// release joystick
	for(int i = 0; i < 2; i++) {
		if (joy[i] != NULL) {
			SDL_JoystickClose(joy[i]);
		}
		joy[i] = NULL;
		joy_enabled[i] = false;
	}
#endif
}

void EMU_OSD::update_input()
{
//	logging->out_debug("EMU::update_input");

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

#ifdef USE_LIGHTPEN
	if(FLG_USELIGHTPEN) {
		// get current status
		VmPoint pt;
		GdkModifierType mstat = get_pointer_state_on_window(window, &pt);

		mouse_status[2] = (mstat & GDK_BUTTON1_MASK ? 1 : 0) | (mstat & GDK_BUTTON3_MASK ? 2 : 0);

		pt.x = mixed_size.w * (pt.x - stretched_dest_real.x - stretched_size.x) / stretched_size.w;
		pt.y = mixed_size.h * (pt.y - stretched_dest_real.y - stretched_size.y) / stretched_size.h;

		mouse_status[0] = pt.x;
		mouse_status[1] = pt.y;
	}
#endif
}

GdkModifierType EMU_OSD::get_pointer_state_on_window(GtkWidget *window, VmPoint *pt, GdkDevice **device)
{
	GdkDisplay *display = gtk_widget_get_display(window);
	GdkWindow *dwindow = gtk_widget_get_window(window);
#if GTK_CHECK_VERSION(3,20,0)
	GdkSeat *seat = gdk_display_get_default_seat(display);
	GdkDevice *ddevice = gdk_seat_get_pointer(seat);
#elif GTK_CHECK_VERSION(3,0,0)
	GdkDeviceManager *manager = gdk_display_get_device_manager(display);
	GdkDevice *ddevice = gdk_device_manager_get_client_pointer(manager);
#else
#error "need GTK+3.0"
#endif
	GdkModifierType mstat;
	gdk_window_get_device_position(dwindow, ddevice, &pt->x, &pt->y, &mstat);
//	logging->out_debugf(_T("get: pt.x:%d pt.y:%d"), pt->x, pt->y);
	if (device) *device = ddevice;
	return mstat;
}

void EMU_OSD::set_pointer_on_window(GtkWidget *window, GdkDevice *ddevice, VmPoint *pt)
{
	GdkScreen *dscreen = gtk_widget_get_screen(window);
	GdkWindow *dwindow = gtk_widget_get_window(window);
	VmPoint wp;
//	gdk_device_get_position(ddevice, &dscreen, &dp.x, &dp.y);
	gdk_window_get_origin(dwindow, &wp.x, &wp.y);
	gdk_device_warp(ddevice, dscreen, wp.x + pt->x, wp.y + pt->y);
	gdk_display_flush(gdk_device_get_display(ddevice));
//	logging->out_debugf(_T("set: dp.x:%d dp.y:%d wp.x:%d wp.y:%d pt.x:%d pt.y:%d"), dp.x, dp.y, wp.x, wp.y, pt->x, pt->y);
}

void EMU_OSD::update_mouse()
{
	if (!initialized) return;

	// update mouse status
	memset(mouse_status, 0, sizeof(mouse_status));
	if(!mouse_disabled) {
		// get current status
		VmPoint pt;
		GdkDevice *ddevice = NULL;
		GdkModifierType mstat = get_pointer_state_on_window(window, &pt, &ddevice);

		mouse_status[0]  = pt.x - display_size.w / 2;
		mouse_status[1]  = pt.y - display_size.h / 2;

		mouse_status[2] =  (mstat & GDK_BUTTON1_MASK ? 1 : 0);
		mouse_status[2] |= (mstat & GDK_BUTTON3_MASK ? 2 : 0);
		mouse_status[2] |= (mstat & GDK_BUTTON2_MASK ? 4 : 0);
		// move mouse cursor to the center of window
		if(!(mouse_status[0] == 0 && mouse_status[1] == 0)) {
			pt.x = display_size.w / 2;
			pt.y = display_size.h / 2;
			set_pointer_on_window(window, ddevice, &pt);
		}
	}
}

#if 0
void EMU_OSD::update_mouse_event(GdkEventMotion *event)
{
	if (!initialized) return;

	// update mouse status
	memset(mouse_status, 0, sizeof(mouse_status));
	if(!mouse_disabled) {
		mouse_status[0]  = (int)(event->x - display_size.w / 2.0);
		mouse_status[1]  = (int)(event->y - display_size.h / 2.0);

		mouse_status[2] =  (event->state & GDK_BUTTON1_MASK ? 1 : 0);
		mouse_status[2] |= (event->state & GDK_BUTTON3_MASK ? 2 : 0);
		mouse_status[2] |= (event->state & GDK_BUTTON2_MASK ? 4 : 0);

		// move mouse cursor to the center of window
		if(!(mouse_status[0] == 0 && mouse_status[1] == 0)) {
			VmPoint wp, pt;
			pt.x = display_size.w / 2;
			pt.y = display_size.h / 2;
			GdkScreen *dscreen = gdk_window_get_screen(event->window);
			gdk_window_get_origin(event->window, &wp.x, &wp.y);
			gdk_device_warp(event->device, dscreen, wp.x + pt.x, wp.y + pt.y);
			gdk_display_flush(gdk_device_get_display(event->device));
		}
	}
}
#endif

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
			if (!SDL_JoystickGetAttached(joy[i])) {
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
/// @param[in] code   : key code
/// @param[in] status : scan code
int EMU_OSD::key_down_up(uint8_t type, int code, long status)
{
	bool keep_frames = false;

	// translate keycode
	// type change UP to DOWN when capslock key in macosx
	if (!translate_keysym(type, code, (short)status, &code, &keep_frames)) {
#ifdef LOG_MEASURE
		logging->out_debugf(_T("KEYDOWN: code:%02x scancode:%02x")
			, code, scan_code);
#endif
		if (key_mod & KEY_MOD_ALT_KEY) {
			// notify key down
			system_key_down(code);
			// execute for pressed global key
			execute_global_keys(code, 0);
		} else {
			key_down(code, keep_frames);
		}
	} else {
#ifdef LOG_MEASURE
		logging->out_debugf(_T("KEYUP: code:%02x scancode:%02x")
			, code, scan_code);
#endif
		if (key_mod & KEY_MOD_ALT_KEY) {
			// notify key down
			system_key_down(code);
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

void EMU_OSD::key_up(int code, bool keep_frames)
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
		GdkWindow *dwindow = gtk_widget_get_window(window);
		GdkDisplay *ddisplay = gdk_window_get_display(dwindow);
		GdkCursor *cursor = gdk_window_get_cursor(dwindow);
		if (!bkup_cursor) {
			bkup_cursor = gdk_cursor_new_for_display(ddisplay, GDK_BLANK_CURSOR);
		}
		gdk_window_set_cursor(dwindow, bkup_cursor);
		bkup_cursor = cursor;
		// move mouse cursor to the center of window
		VmPoint pt;
		GdkDevice *ddevice;
		get_pointer_state_on_window(window, &pt, &ddevice);
		pt.x = display_size.w / 2;
		pt.y = display_size.h / 2;
		set_pointer_on_window(window, ddevice, &pt);
	}
}

void EMU_OSD::disable_mouse(int mode)
{
	// disable mouse emulation
	if(!mouse_disabled) {
		// show cursor
		GdkWindow *dwindow = gtk_widget_get_window(window);
		GdkCursor *cursor = gdk_window_get_cursor(dwindow);
		gdk_window_set_cursor(dwindow, bkup_cursor);
		bkup_cursor = cursor;
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
