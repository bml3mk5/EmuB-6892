/** @file ag_gui_base.cpp

	Skelton for retropc emulator
	SDL + Ager edition

	@author Sasaji
	@date   2012.3.2

	@brief [ ag_gui_base ]
*/

#include "ag_gui_base.h"
#include "ag_volume_dlg.h"
#include "ag_file_dlg.h"
#include "ag_seldrv_dlg.h"
#ifdef USE_REC_VIDEO
#include "ag_recvid_dlg.h"
#endif
#ifdef USE_REC_AUDIO
#include "ag_recaud_dlg.h"
#endif
#include "../../main.h"
#include "../../config.h"
#include "../../utility.h"
#include "../../version.h"
#if defined(_WIN32)
#include "../windows/win_gui.h"
#elif (defined(__APPLE__) && defined(__MACH__))
#include "../cocoa/cocoa_gui.h"
#else
#include "../gtk_x11/gtk_x11_gui.h"
#endif

namespace GUI_AGAR
{

AG_GUI_BASE::AG_GUI_BASE(int argc, char **argv, EMU *new_emu) : GUI_BASE(argc, argv, new_emu)
{
	memset(drivers, 0, sizeof(drivers));
	now_driver = NULL;

	exit_event = 0;
	first = 120;

	filebox = NULL;
	volumebox = NULL;
	seldrvbox = NULL;
#ifdef USE_REC_VIDEO
	recvidbox = NULL;
#endif
#ifdef USE_REC_AUDIO
	recaudbox = NULL;
#endif

	now_showing = false;
}

AG_GUI_BASE::~AG_GUI_BASE()
{
	// save AGAR config
	ConfigSave();

#ifdef USE_REC_AUDIO
	delete recaudbox;
#endif
#ifdef USE_REC_VIDEO
	delete recvidbox;
#endif
	delete seldrvbox;
	delete volumebox;
	delete filebox;

	AG_DestroyGUI();
	AG_DestroyGUIGlobals();
	AG_Destroy();
//	AG_QuitGUI();
//	AG_DriverClose((AG_Driver *)agDriverSw);
}

/// Initialize Ager GUI core
/// @return -1:error 0:OK 1:OK(but use default font)
int AG_GUI_BASE::Init()
{
	int rc = 0;
	Uint32 flags = 0;

#ifdef _DEBUG
	flags |= AG_VERBOSE;
#endif

	// init AGAR GUI
	if ((rc = AG_InitCore(CONFIG_NAME, flags)) == -1) {
		logging->out_logf(LOG_ERROR, _T("AG_InitCore: %s"), AG_GetError());
		return rc;
	}
	// load AGAR config
	rc = ConfigLoad();

	// dialog
	filebox = new AG_FILE_DLG(emu, this);
	volumebox = new AG_VOLUME_DLG(emu, this);
	seldrvbox = new AG_SELDRV_DLG(emu, this);
#ifdef USE_REC_VIDEO
	recvidbox = new AG_RECVIDEO_DLG(emu, this);
#endif
#ifdef USE_REC_AUDIO
	recaudbox = new AG_RECAUDIO_DLG(emu, this);
#endif

	return rc;
}

int AG_GUI_BASE::CreateWidget(SDL_Surface *screen, int width, int height)
{
	int rc = 0;

	GUI_BASE::CreateWidget(screen, width, height);

	if (screen->flags & SDL_OPENGL) {
		now_driver = &drivers[1];
	} else {
		now_driver = &drivers[0];
	}

	if (now_driver->win == NULL) {
#if 0
		if ((rc = AG_InitGraphics(NULL)) == -1) {
			logging->out_logf(LOG_ERROR, "AG_InitGraphics: %s", AG_GetError());
			return rc;
		}
		// Display main window
		drv->win = AG_WindowNewNamedS(AG_WINDOW_NOMAXIMIZE, "test gui");
		if (drv->win == NULL) {
			rc = -1;
			logging->out_logf(LOG_ERROR, "AG_WindowNew: %s", AG_GetError());
			return rc;
		}
#else
		// Initialize Agar-GUI to reuse display
		if ((rc = AG_InitVideoSDL(screen, AG_VIDEO_NOFRAME | AG_VIDEO_OVERLAY)) == -1) {
			logging->out_logf(LOG_ERROR, _T("AG_InitVideoSDL: %s"), AG_GetError());
			return rc;
		}
		// attach ag window to SDL
		now_driver->win = AG_WindowNew(AG_WINDOW_PLAIN | AG_WINDOW_NOBACKGROUND | AG_WINDOW_NOBUTTONS);
		if (now_driver->win == NULL) {
			rc = -1;
			logging->out_logf(LOG_ERROR, _T("AG_WindowNew: %s"), AG_GetError());
			return rc;
		}
#endif
		now_driver->ops = agDriverOps;
		now_driver->sw  = agDriverSw;

		// use UNICODE key
		if (SDL_EnableUNICODE(1) == -1) {
			logging->out_logf(LOG_WARN, _T("SDL_EnableUNICODE: %s."), SDL_GetError());
			logging->out_log(LOG_WARN, _T("Disable key input on a text field in AGAR."));
		}

		// create menu
//		if ((rc = CreateMenu()) == -1) {
//			return rc;
//		}
		// set user event
		AG_AddEvent(now_driver->win, "user-process", OnProcessUserEvent, "%Cp", this);

	} else {
		// change driver
		agDriverOps = now_driver->ops;
		agDriverSw = now_driver->sw;

		AG_SetVideoSurfaceSDL(screen);
		AG_ResizeDisplay(width, height);
	}
	AG_WindowSetGeometry(now_driver->win, 0, 0, width, height);

#if (defined(__APPLE__) && defined(__MACH__))
	// support drag and drop
	set_delegate_to_sdl_window(this);
#endif

	return rc;
}

int AG_GUI_BASE::CreateMenu()
{
	int rc = 0;

#if (defined(__APPLE__) && defined(__MACH__))
	remove_window_menu();
	translate_apple_menu();
#endif

	if (now_driver == NULL) {
		rc = -1;
		return rc;
	}

	now_driver->menu = AG_MenuNew(now_driver->win, 0);
	if (now_driver->menu == NULL) {
		rc = -1;
		logging->out_logf(LOG_ERROR, _T("Error: AG_MenuNew: %s"), AG_GetError());
		return rc;
	}

	set_menu_item(now_driver->menu);

	ShowMenu();

	/* menubar height */
	int view_top = now_driver->menu->wid.h;
	emu->set_display_margin(0,view_top,0,0);
	if (view_top > 0) rc = 1; // need resize window

	return rc;
}

int AG_GUI_BASE::ResizeDisplay(int width, int height)
{
	return AG_ResizeDisplay(width, height);
}

int AG_GUI_BASE::CreateGlobalKeys()
{
	int rc = 0;

	set_global_keys();

	return rc;
}

/**
 * user event
 */
void AG_GUI_BASE::ProcessUserEvent(int id)
{
}

AG_Window *AG_GUI_BASE::GetWindow()
{
	return now_driver->win;
}

AG_DLG *AG_GUI_BASE::GetDlgPtr(int)
{
	return NULL;
}

void AG_GUI_BASE::ShowMenu()
{
	AG_WindowShow(now_driver->win);
	now_showing = true;
}

void AG_GUI_BASE::HideMenu()
{
	AG_WindowHide(now_driver->win);
	now_showing = false;
}

/**
 * gui process event (AGAR)
 */
int AG_GUI_BASE::ProcessEvent(SDL_Event *event)
{
	int rc = GUI_BASE::ProcessEvent(event);
	if (rc <= 0) return rc;

	AG_DriverEvent dev;

	sdl_event = *event;
	exit_event = 1;

	if (sdl_event.type == SDL_KEYDOWN || sdl_event.type == SDL_KEYUP) {
		translate_keycode(&sdl_event.key);
//	} else if (sdl_event.type == SDL_MOUSEMOTION) {
//		if (emu) {
//			int x = ((SDL_MouseMotionEvent *)event)->x;
//			int y = ((SDL_MouseMotionEvent *)event)->y;
//			if (0 < y && y < 24 && 0 < x && x < SCREEN_WIDTH) {
//				// show menu bar
//				ShowMenu();
//			}
//		}
	}
	AG_SDL_TranslateEvent((void *)agDriverSw, &sdl_event, &dev);
	event_proc(&dev);
	if (exit_event != -1) {
		AG_ProcessEvent(NULL, &dev);
	}
//	if (exit_event == 2) {
//		HideMenu();
//		exit_event = 1;
//	}
	return exit_event;
}

/**
 * process timeout (AGAR)
 */
void AG_GUI_BASE::PostProcessEvent()
{
	GUI_BASE::PostProcessEvent();

	Uint32 t1 = AG_GetTicks();
	if (AG_TIMEOUTS_QUEUED()) {
		// There are AG_Timeout(3) callbacks to run.
		AG_ProcessTimeouts(t1);
//		return (1);
	}
	if (first > 0) first--;
//	return (0);
}

/**
 * gui mix screen (AGAR)
 */
int AG_GUI_BASE::MixSurface()
{
	AG_Window *win;

	if (!now_showing) return 0;

	AG_LockVFS(&agDrivers);
	if (agDriverSw) {
		AG_BeginRendering(agDriverSw);
		AG_FOREACH_WINDOW(win, agDriverSw) {
			if (!win->visible) {
				continue;
			}
			AG_ObjectLock(win);
			AG_WindowDraw(win);
			AG_ObjectUnlock(win);
		}
		AG_EndRendering(agDriverSw);
	}
	AG_UnlockVFS(&agDrivers);

	return 1;
}

/**
 *	gui event procedure (AGAR)
 */
void AG_GUI_BASE::event_proc(AG_DriverEvent *dev)
{
	switch(dev->type) {
	case AG_DRIVER_MOUSE_MOTION:		// Cursor moved
		gui_focus(dev->data.motion.x, dev->data.motion.y);
		break;
	case AG_DRIVER_MOUSE_BUTTON_DOWN:	// Mouse button pressed
		break;
	case AG_DRIVER_MOUSE_BUTTON_UP:		// Mouse button released
		gui_focus(dev->data.motion.x, dev->data.button.y);
		break;
	case AG_DRIVER_MOUSE_ENTER:			// Mouse entering window (MW)
		break;
	case AG_DRIVER_MOUSE_LEAVE:			// Mouse leaving window (MW)
		break;
	case AG_DRIVER_FOCUS_IN:			// Focus on window (MW)
		break;
	case AG_DRIVER_FOCUS_OUT:			// Focus out of window (MW)
		break;
	case AG_DRIVER_KEY_DOWN:			// Key pressed
		break;
	case AG_DRIVER_KEY_UP:				// Key released
		break;
	case AG_DRIVER_EXPOSE:				// Video update needed
		break;
	case AG_DRIVER_VIDEORESIZE:			// Video resize request
		break;
	case AG_DRIVER_CLOSE:				// Window close request
		logging->out_debug(_T("AG_DRIVER_CLOSE"));
		exit_event = -1;
		break;
	case AG_DRIVER_UNKNOWN:
		break;
	}
}

void AG_GUI_BASE::gui_focus(int x, int y)
{
	if (now_showing && now_driver->menu->selecting == 0 && IsFullScreen() && (y <= 0 || 24 <= y || x <= 0 || SCREEN_WIDTH <= x)) {
		AG_Window *win;
		int i = 0;
		// count active widgets
		AG_FOREACH_WINDOW(win, agDriverSw) {
			if (!win->visible) {
				continue;
			}
			i++;
		}
		if (i == 1 && first <= 0) {
			// main menu only (submenus are closed)
//			logging->out_debugf(_T("AG_DRIVER_MOUSE_MOTION: y:%d -> Go SDL "),y);
//			exit_event = 2;
			// hide menu bar
//			AG_WindowHide(now_driver->win);
			HideMenu();
		}
	} else {
		if (emu && !now_showing) {
			if (0 < y && y < 24 && 0 < x && x < SCREEN_WIDTH) {
				// show menu bar
				ShowMenu();
			}
		}
	}
}

void AG_GUI_BASE::ScreenModeChanged(bool fullscreen)
{
	if (now_driver && !now_showing && !fullscreen) {
		// show menu bar
		ShowMenu();
	}
}

void AG_GUI_BASE::ChangeWindowMode(int num)
{
	// flush dirty area
	AG_LockVFS(&agDrivers);
	if (agDriverSw) {
		AG_BeginRendering(agDriverSw);
		AG_EndRendering(agDriverSw);
	}
	AG_UnlockVFS(&agDrivers);

	GUI_BASE::ChangeWindowMode(num);
}

void AG_GUI_BASE::ChangeFullScreenMode(int num)
{
	// flush dirty area
	AG_LockVFS(&agDrivers);
	if (agDriverSw) {
		AG_BeginRendering(agDriverSw);
		AG_EndRendering(agDriverSw);
	}
	AG_UnlockVFS(&agDrivers);

	GUI_BASE::ChangeFullScreenMode(num);
}

/**
 * create menu
 */
void AG_GUI_BASE::set_menu_item(AG_Menu *menu)
{
	AG_MenuItem *mt_control;

	mt_control = AG_MenuNode(menu->root, "Control", NULL);
	{
		AG_MenuAction(mt_control, "  Quit", NULL, OnSelectExit, "%Cp", this);
	}
}

const char *AG_GUI_BASE::menu_str(const char *str)
{
	static char buf[128];
	char lbl[128];
	strcpy(lbl, str);
	char *p = strchr(lbl, '\t');
	if (p != NULL) {
		*p = '\0';
		p++;
		sprintf(buf,"  %-24s %s", lbl, p);
	} else {
		sprintf(buf,"  %s", lbl);
	}
	return buf;
}

/**
 * create global keys
 */
void AG_GUI_BASE::set_global_keys()
{
// oops....Why cannot you set the args for event structure such like AG_SetEvent???
//	AG_BindGlobalKeyEv(AG_KEY_P, AG_KEY_LALT, OnSelectPause);
}

/****************************************
 *	gui translate keycode
 */
void AG_GUI_BASE::translate_keycode(SDL_KeyboardEvent *e)
{
#if defined(_WIN32)
	switch(e->keysym.scancode) {
		case 0x73:	// underscore
			e->keysym.sym = SDLK_UNDERSCORE;
			break;
	}
#elif defined(__APPLE__) && defined(__MACH__)
	switch(e->keysym.scancode) {
		case 0x5d:      // yen
			e->keysym.sym = SDLK_BACKSLASH;
			break;
		case 0x5e:      // underscore
			e->keysym.sym = SDLK_UNDERSCORE;
			break;
	}
#endif
}

/****************************************
 * Event Operation
 */
///
void AG_GUI_BASE::update_multi_volume_list(AG_MenuItem *mi, int drv)
{
	AG_MenuItem *ms;
	char name[32];
	bool flag = false;
	bool upd = false;	

	upd = emu->changed_cur_bank(drv);

	if (!upd) return;

	AG_MenuItemFreeChildren(mi);

	D88File *d88_file = GetD88File(drv);
	int bank_nums = d88_file->GetBanks().Count();

	for(int num=0; num<bank_nums; num++) {
		const D88Bank *d88_bank = d88_file->GetBank(num);
		sprintf(name, "%c ", num == d88_file->GetCurrentBank() ? '*' : ' ');
		GetMultiVolumeStr(num, d88_bank->GetName(), &name[2], 30);
		ms = AG_MenuAction(mi, name, NULL, OnSelectVolumeFloppyDisk, "%Cp %i %i", this, drv, num);
		AG_GUI_MENU_ENABLE(ms, true);
		flag = true;
	}
	if (bank_nums == 1) {
		AG_GUI_MENU_ENABLE(ms, false);
	}
	if (!flag) {
		ms = AG_MenuNode(mi, CMSG(None_), NULL);
		AG_GUI_MENU_ENABLE(ms, false);
	}
}

///
void AG_GUI_BASE::update_recent_list(AG_MenuItem *mi, CRecentPathList &list, int drv, AG_EventFn fn)
{
	AG_MenuItem *ms;
	char path[_MAX_PATH+8];
	const char *file;
	bool flag = false;
	bool upd = false;
	int  max = 0;

	upd = list.updated;
	list.updated = false;
	max = list.Count();

	if (!upd) return;

	AG_MenuItemFreeChildren(mi);

	for(int num=0; num<max; num++) {
		file = list[num]->path;
		if (!GetRecentFileStr(file, list[num]->num, path, 56)) break;
		ms = AG_MenuAction(mi, path, NULL, fn, "%Cp %i %i", this, drv, num);
		AG_GUI_MENU_ENABLE(ms, true);
		flag = true;
	}
	if (!flag) {
		ms = AG_MenuNode(mi, CMSG(None_), NULL);
		AG_GUI_MENU_ENABLE(ms, false);
	}
}

#if 0
///
void AG_GUI_BASE::update_recent_list(AG_MenuItem *mi, int type, int drv)
{
	AG_MenuItem *ms;
	char path[_MAX_PATH+8];
	const char *file;
	bool flag = false;
	bool upd = false;
	int  max = 0;

	switch(type) {
	case AG_FILE_DLG::DATAREC:
		upd = config.recent_datarec_path.updated;
		config.recent_datarec_path.updated = false;
		max = config.recent_datarec_path.Count();
		break;
	case AG_FILE_DLG::FLOPPY:
		upd = config.recent_disk_path[drv].updated;
		config.recent_disk_path[drv].updated = false;
		max = config.recent_disk_path[drv].Count();
		break;
	}
	if (!upd) return;

	AG_MenuItemFreeChildren(mi);

	for(int num=0; num<max; num++) {
		switch(type) {
		case AG_FILE_DLG::DATAREC:
			file = config.recent_datarec_path[num]->path;
			if (!GetRecentFileStr(file, config.recent_datarec_path[num]->num, path, 56)) break;
			ms = AG_MenuAction(mi, path, NULL, OnSelectRecentDataRec, "%Cp %i", this, num);
			AG_GUI_MENU_ENABLE(ms, true);
			flag = true;
			break;
		case AG_FILE_DLG::FLOPPY:
			file = config.recent_disk_path[drv][num]->path;
			if (!GetRecentFileStr(file, config.recent_disk_path[drv][num]->num, path, 56)) break;
			ms = AG_MenuAction(mi, path, NULL, OnSelectRecentFloppyDisk, "%Cp %i %i", this, drv, num);
			AG_GUI_MENU_ENABLE(ms, true);
			flag = true;
			break;
		}
	}
	if (!flag) {
		ms = AG_MenuNode(mi, CMSG(None), NULL);
		AG_GUI_MENU_ENABLE(ms, false);
	}
}
#endif

/// create Record video dialog
bool AG_GUI_BASE::ShowRecordVideoDialog(int fps_num)
{
#ifdef USE_REC_VIDEO
	ShowMenu();
	recvidbox->Create(fps_num, true);
#endif
	return true;
}
/// create Record audio dialog
bool AG_GUI_BASE::ShowRecordAudioDialog()
{
#ifdef USE_REC_AUDIO
	ShowMenu();
	recaudbox->Create();
#endif
	return true;
}
/// create Volume dialog
bool AG_GUI_BASE::ShowVolumeDialog(void)
{
	ShowMenu();
	volumebox->Create();
	return true;
}
/// show Floppy Disk Open Dialog
bool AG_GUI_BASE::ShowOpenFloppyDiskDialog(int drv)
{
	char title[128];

	UTILITY::sprintf_utf8(title, 128, CMSG(Open_Floppy_Disk_VDIGIT), drv);
	ShowMenu();
	filebox->CreateLoad(AG_FILE_DLG::FLOPPY, title
		, "*.d88", config.initial_disk_path, NULL, (void *)(intptr_t)drv);
	return true;
}

void AG_GUI_BASE::ShowAgSelectFloppyDriveDialog(int drv, int num)
{
	ShowMenu();
	seldrvbox->SetPrefix(CMSG(FDD));
	seldrvbox->Create(drv, num);
}

/// show New Blank Floppy Disk Open Dialog
bool AG_GUI_BASE::ShowOpenBlankFloppyDiskDialog(int drv, uint8_t type)
{
	char title[128];

	UTILITY::sprintf_utf8(title, 128, CMSG(New_Floppy_Disk_VDIGIT), drv);

	char file_name[_MAX_PATH];
	UTILITY::create_date_file_path(NULL, file_name, _MAX_PATH, _T("d88"));

	ShowMenu();
	filebox->CreateSave(AG_FILE_DLG::FLOPPY_NEW, title
		, "*.d88", config.initial_disk_path, file_name, (void *)(intptr_t)((int)type << 8 | drv));
	return true;
}

void AG_GUI_BASE::PostEtAgOpenRecentFloppyMessage(int drv, int num, int new_drv)
{
	emumsg.Send(EMUMSG_ID_RECENT_FD, new_drv, config.recent_disk_path[drv][num]->path, config.recent_disk_path[drv][num]->num, 0, true);
}

/// show Play Paste text Dialog
bool AG_GUI_BASE::ShowOpenAutoKeyDialog(void)
{
	ShowMenu();
	filebox->CreateLoad(AG_FILE_DLG::AUTOKEY, CMSG(Open_Text_File)
		, "*.txt,*.bas,*.lpt", config.initial_autokey_path, NULL);
	return true;
}
/// show Print Data Save Dialog
bool AG_GUI_BASE::ShowSavePrinterDialog(int drv)
{
	ShowMenu();
	filebox->CreateSave(AG_FILE_DLG::PRINTER, CMSG(Save_Printing_Data)
		, "*.lpt", config.initial_printer_path, NULL, (void *)(intptr_t)drv);
	return true;
}
/// show About Dialog
bool AG_GUI_BASE::ShowAboutDialog(void)
{
	char edi[128], libver[256];
	AG_AgarVersion agar_ver;

	// edition
	emu->get_edition_string(edi, sizeof(edi));
	// library
	GetLibVersionString(libver, sizeof(libver), _T(", "));
	AG_GetVersion(&agar_ver);

	AG_TextMsg(AG_MSG_INFO,
		"%s Emulator\n"
		"  Version: %s \"%s\"%s\n"
		"  %s\n"
		"  using %s\n"
		"  and Agar Version %d.%d.%d (%s)"
		,DEVICE_NAME
		,APP_VERSION
		,PLATFORM
#ifdef _DEBUG
	    ," (DEBUG Version)"
#else
		,""
#endif
		,edi
		,libver
		,agar_ver.major, agar_ver.minor, agar_ver.patch, agar_ver.release
	);
	return true;
}

/****************************************
 * Event Handler (static)
 */
// Exit
void AG_GUI_BASE::OnSelectExit(AG_Event *event)
{
	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);
	gui->Exit();
}
////////////////////////////////////////
// Reset
void AG_GUI_BASE::OnSelectReset(AG_Event *event)
{
	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);
	gui->PostEtReset();
}
// update Reset
void AG_GUI_BASE::OnUpdateReset(AG_Event *event)
{
	AG_MenuItem *mi = (AG_MenuItem *)AG_SENDER();
	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);

	AG_GUI_MENU_CHECK(mi, !gui->NowPowerOff());
}
// SpecialReset
void AG_GUI_BASE::OnSelectSpecialReset(AG_Event *event)
{
	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);
	gui->PostEtSpecialReset();
}
// WarmReset
void AG_GUI_BASE::OnSelectWarmReset(AG_Event *event)
{
	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);
	gui->PostEtWarmReset(-1);
}

// change dipswitch
void AG_GUI_BASE::OnSelectDipswitch(AG_Event *event)
{
	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);
	int bit = AG_INT(2);
	gui->Dipswitch(bit);
}
// update dipswitch
void AG_GUI_BASE::OnUpdateDipswitch(AG_Event *event)
{
	AG_MenuItem *mi = (AG_MenuItem *)AG_SENDER();
	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);
	int bit = AG_INT(2);

	AG_GUI_MENU_CHECK(mi, gui->GetDipswitch() & (1 << bit));
}

// change pause
void AG_GUI_BASE::OnSelectPause(AG_Event *event)
{
	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);
	gui->TogglePause();
}
// update pause
void AG_GUI_BASE::OnUpdatePause(AG_Event *event)
{
	AG_MenuItem *mi = (AG_MenuItem *)AG_SENDER();
	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);

	AG_GUI_MENU_CHECK(mi, gui->NowPause());
}

// change cpu power
void AG_GUI_BASE::OnSelectCPUPower(AG_Event *event)
{
	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);
	int num = AG_INT(2);
	gui->PostEtCPUPower(num);
}
// update cpu power
void AG_GUI_BASE::OnUpdateCPUPower(AG_Event *event)
{
	AG_MenuItem *mi = (AG_MenuItem *)AG_SENDER();
//	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);
	int num = AG_INT(2);

	AG_GUI_MENU_CHECK(mi, config.cpu_power == num);
}
// show Open Text dialog (auto key)
void AG_GUI_BASE::OnSelectOpenAutoKey(AG_Event *event)
{
	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);

	gui->ShowOpenAutoKeyDialog();
}
// update open text (auto key)
void AG_GUI_BASE::OnUpdateOpenAutoKey(AG_Event *event)
{
	AG_MenuItem *mi = (AG_MenuItem *)AG_SENDER();
	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);

	if (gui->IsRunningAutoKey()) {
		mi->state = 0;	// disable
	} else {
		mi->state = 1;	// enable
	}
}
// show Paste Text dialog (auto key)
void AG_GUI_BASE::OnSelectStartAutoKey(AG_Event *event)
{
	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);

	gui->PostEtStartAutoKeyMessage();
}
// update paste text (auto key)
void AG_GUI_BASE::OnUpdateStartAutoKey(AG_Event *event)
{
	AG_MenuItem *mi = (AG_MenuItem *)AG_SENDER();
	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);

	if (gui->IsRunningAutoKey()) {
		mi->state = 0;	// disable
	} else {
		mi->state = 1;	// enable
	}
}
// stop auto key
void AG_GUI_BASE::OnSelectStopAutoKey(AG_Event *event)
{
	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);

	gui->PostEtStopAutoKeyMessage();
}

////////////////////////////////////////
// Show Data Recorder Load Dialog
void AG_GUI_BASE::OnSelectLoadDataRec(AG_Event *event)
{
	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);
//	char *ext = AG_STRING(2);

	gui->ShowLoadDataRecDialog();
}
// update data rec load
void AG_GUI_BASE::OnUpdateLoadDataRec(AG_Event *event)
{
	AG_MenuItem *mi = (AG_MenuItem *)AG_SENDER();
	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);

	AG_GUI_MENU_CHECK(mi, gui->IsOpenedLoadDataRecFile());
}
// Show Data Recorder Save Dialog
void AG_GUI_BASE::OnSelectSaveDataRec(AG_Event *event)
{
	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);
//	char *ext = AG_STRING(2);

	gui->ShowSaveDataRecDialog();
}
// update data rec save
void AG_GUI_BASE::OnUpdateSaveDataRec(AG_Event *event)
{
	AG_MenuItem *mi = (AG_MenuItem *)AG_SENDER();
	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);

	AG_GUI_MENU_CHECK(mi, gui->IsOpenedSaveDataRecFile());
}
// Close Data Recorder file
void AG_GUI_BASE::OnSelectCloseDataRec(AG_Event *event)
{
	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);

	gui->PostEtCloseDataRecMessage();
}
// select Recent file (Datarec)
void AG_GUI_BASE::OnSelectRecentDataRec(AG_Event *event)
{
	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);
	int num = AG_INT(3);

	gui->PostEtLoadRecentDataRecMessage(num);
}
// update Recent Data Rec
void AG_GUI_BASE::OnUpdateRecentDataRec(AG_Event *event)
{
	AG_MenuItem *mi = (AG_MenuItem *)AG_SENDER();
	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);

	gui->update_recent_list(mi, config.recent_datarec_path, 0, OnSelectRecentDataRec);
}

////////////////////////////////////////
// Show Floppy Disk Open Dialog
void AG_GUI_BASE::OnSelectOpenFloppyDisk(AG_Event *event)
{
	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);
	int   drv = AG_INT(2);
//	char *ext = AG_STRING(3);

	gui->ShowOpenFloppyDiskDialog(drv);
}
// update floppy open
void AG_GUI_BASE::OnUpdateOpenFloppyDisk(AG_Event *event)
{
	AG_MenuItem *mi = (AG_MenuItem *)AG_SENDER();
	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);
	int   drv = AG_INT(2);

	AG_GUI_MENU_CHECK(mi, gui->InsertedFloppyDisk(drv));
}
// Close Floppy Disk file
void AG_GUI_BASE::OnSelectCloseFloppyDisk(AG_Event *event)
{
	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);
	int   drv = AG_INT(2);

	gui->PostEtCloseFloppyMessage(drv);
}
// select Recent file (floppy disk)
void AG_GUI_BASE::OnSelectRecentFloppyDisk(AG_Event *event)
{
	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);
	int   drv = AG_INT(2);
	int   num = AG_INT(3);

	gui->ShowAgSelectFloppyDriveDialog(drv, num);
//	gui->PostEtOpenFloppyMessage(drv, file_path, 0, 0, true);
}
// update Recent Floppy Disk
void AG_GUI_BASE::OnUpdateRecentFloppyDisk(AG_Event *event)
{
	AG_MenuItem *mi = (AG_MenuItem *)AG_SENDER();
	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);
	int drv = AG_INT(2);

	gui->update_recent_list(mi, config.recent_disk_path[drv], drv, OnSelectRecentFloppyDisk);
}
// Show Blank Floppy Disk Dialog
void AG_GUI_BASE::OnSelectOpenBlankFloppyDisk(AG_Event *event)
{
	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);
	int   drv = AG_INT(2);
	int   type = AG_INT(3);

	gui->ShowOpenBlankFloppyDiskDialog(drv, (uint8_t)type);
}
void AG_GUI_BASE::OnUpdateMultiVolumeList(AG_Event *event)
{
	AG_MenuItem *mi = (AG_MenuItem *)AG_SENDER();
	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);
	int   drv = AG_INT(2);

	gui->update_multi_volume_list(mi, drv);
}
/// select bank number of multi volume (floppy disk)
void AG_GUI_BASE::OnSelectVolumeFloppyDisk(AG_Event *event)
{
	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);
	int   drv = AG_INT(2);
	int   bank_num = AG_INT(3);

	gui->PostEtOpenFloppySelectedVolume(drv, bank_num);
}
/// update bank number of multi volume
void AG_GUI_BASE::OnUpdateVolumeFloppyDisk(AG_Event *event)
{
//	AG_MenuItem *mi = (AG_MenuItem *)AG_SENDER();
//	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);
//	int   drv = AG_INT(2);
//	int   bank_num = AG_INT(3);
//
//	AG_GUI_MENU_CHECK(mi, gui->InsertedFloppyDisk(drv));
}

////////////////////////////////////////
/// Change Frame Rate
void AG_GUI_BASE::OnSelectFrameRate(AG_Event *event)
{
	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);
	int num = AG_INT(2);

	gui->ChangeFrameRate(num);
}
// update Frame Rate
void AG_GUI_BASE::OnUpdateFrameRate(AG_Event *event)
{
	AG_MenuItem *mi = (AG_MenuItem *)AG_SENDER();
	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);
	int num = AG_INT(2);

	AG_GUI_MENU_CHECK(mi, gui->GetFrameRateNum() == num);
}
/// Change Screen Record Size
void AG_GUI_BASE::OnSelectScreenRecordSize(AG_Event *event)
{
	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);
	int num = AG_INT(2);

	gui->PostEtResizeRecordVideoSurface(num);
}
/// update Screen Record Size
void AG_GUI_BASE::OnUpdateScreenRecordSize(AG_Event *event)
{
	AG_MenuItem *mi = (AG_MenuItem *)AG_SENDER();
	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);
	int num = AG_INT(2);

	AG_GUI_MENU_CHECK(mi, gui->GetRecordVideoSurfaceNum() == num);
	AG_GUI_MENU_ENABLE(mi, !(gui->NowRecordingVideo() | gui->NowRecordingSound()));
}
/// Change Frame Rate for record video
void AG_GUI_BASE::OnSelectScreenRecordFrameRate(AG_Event *event)
{
	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);
	int num = AG_INT(2);

#ifdef USE_REC_VIDEO
	gui->ShowRecordVideoDialog(num);
#endif
}
/// update Frame Rate for record video
void AG_GUI_BASE::OnUpdateScreenRecordFrameRate(AG_Event *event)
{
	AG_MenuItem *mi = (AG_MenuItem *)AG_SENDER();
	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);
	int num = AG_INT(2);

	AG_GUI_MENU_CHECK(mi, gui->GetRecordVideoFrameNum() == num);
	AG_GUI_MENU_ENABLE(mi, !(gui->NowRecordingVideo() | gui->NowRecordingSound()));
}
/// Change Stop Recording Screen
void AG_GUI_BASE::OnSelectStopScreenRecord(AG_Event *event)
{
	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);

	gui->PostEtStopRecordVideo();
}
/// update Stop Recording Screen
void AG_GUI_BASE::OnUpdateStopScreenRecord(AG_Event *event)
{
	AG_MenuItem *mi = (AG_MenuItem *)AG_SENDER();
	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);

	AG_GUI_MENU_ENABLE(mi, gui->NowRecordingVideo());
}
/// Change Screen Capture
void AG_GUI_BASE::OnSelectScreenCapture(AG_Event *event)
{
	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);

	gui->PostEtCaptureScreen();
}
/// Change Window Mode
void AG_GUI_BASE::OnSelectWindowMode(AG_Event *event)
{
	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);
	int num = AG_INT(2);

	gui->ChangeWindowMode(num);
}

/// update Window Mode
void AG_GUI_BASE::OnUpdateWindowMode(AG_Event *event)
{
	AG_MenuItem *mi = (AG_MenuItem *)AG_SENDER();
	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);
	int num = AG_INT(2);

	AG_GUI_MENU_CHECK(mi, gui->GetWindowMode() == num);
}
/// Change Screen Mode
void AG_GUI_BASE::OnSelectScreenMode(AG_Event *event)
{
	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);
	int num = AG_INT(2);

	gui->ChangeFullScreenMode(num);
}
/// update Screen Mode
void AG_GUI_BASE::OnUpdateScreenMode(AG_Event *event)
{
	AG_MenuItem *mi = (AG_MenuItem *)AG_SENDER();
	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);
	int num = AG_INT(2);

	AG_GUI_MENU_CHECK(mi, gui->GetFullScreenMode() == num);
	AG_GUI_MENU_ENABLE(mi, !gui->IsFullScreen());
}
/// Change Scan Line
void AG_GUI_BASE::OnSelectScanLine(AG_Event *event)
{
	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);
	int num = AG_INT(2);

	gui->PostEtChangeDrawMode(num);
}
/// update Scan Line
void AG_GUI_BASE::OnUpdateScanLine(AG_Event *event)
{
	AG_MenuItem *mi = (AG_MenuItem *)AG_SENDER();
	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);
	int num = AG_INT(2);

	AG_GUI_MENU_CHECK(mi, gui->GetDrawMode() == num);
}
/// Change Pixel Aspect Ratio
void AG_GUI_BASE::OnSelectPixelAspect(AG_Event *event)
{
	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);
	int num = AG_INT(2);

	gui->ChangePixelAspect(num);
}
/// Update Pixel Aspect Ratio
void AG_GUI_BASE::OnUpdatePixelAspect(AG_Event *event)
{
	AG_MenuItem *mi = (AG_MenuItem *)AG_SENDER();
	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);
	int num = AG_INT(2);

	AG_GUI_MENU_CHECK(mi, gui->GetPixelAspectMode() == num);
}

////////////////////////////////////////
// create Volume dialog
void AG_GUI_BASE::OnSelectSoundVolume(AG_Event *event)
{
	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);

	gui->ShowVolumeDialog();
}
// Change Sound Start Record
void AG_GUI_BASE::OnSelectSoundStartRecord(AG_Event *event)
{
	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);

#ifdef USE_REC_AUDIO
	gui->ShowRecordAudioDialog();
#endif
}
// update Sound Start Record
void AG_GUI_BASE::OnUpdateSoundStartRecord(AG_Event *event)
{
	AG_MenuItem *mi = (AG_MenuItem *)AG_SENDER();
	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);

	AG_GUI_MENU_ENABLE(mi, !(gui->NowRecordingVideo() | gui->NowRecordingSound()));
}
// Change Sound Start Record
void AG_GUI_BASE::OnSelectSoundStopRecord(AG_Event *event)
{
	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);

	gui->PostEtStopRecordSound();
}
// update Sound Start Record
void AG_GUI_BASE::OnUpdateSoundStopRecord(AG_Event *event)
{
//	AG_MenuItem *mi = (AG_MenuItem *)AG_SENDER();
//	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);

//	AG_GUI_MENU_ENABLE(mi, gui->NowRecordingVideo() | gui->NowRecordingSound());
}
// Change Sound Rate
void AG_GUI_BASE::OnSelectSoundRate(AG_Event *event)
{
	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);
	int num = AG_INT(2);

	gui->ChangeSoundFrequency(num);
}
// update Sound Rate
void AG_GUI_BASE::OnUpdateSoundRate(AG_Event *event)
{
	AG_MenuItem *mi = (AG_MenuItem *)AG_SENDER();
	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);
	int num = AG_INT(2);

	AG_GUI_MENU_CHECK(mi, gui->GetSoundFrequencyNum() == num);
}
// Change Sound Latency
void AG_GUI_BASE::OnSelectSoundLatency(AG_Event *event)
{
	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);
	int num = AG_INT(2);

	gui->ChangeSoundLatency(num);
}
// update Sound Latency
void AG_GUI_BASE::OnUpdateSoundLatency(AG_Event *event)
{
	AG_MenuItem *mi = (AG_MenuItem *)AG_SENDER();
	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);
	int num = AG_INT(2);

	AG_GUI_MENU_CHECK(mi, gui->GetSoundLatencyNum() == num);
}

////////////////////////////////////////
// Show Print Data Save Dialog
void AG_GUI_BASE::OnSelectSavePrinter(AG_Event *event)
{
	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);
	int drv = AG_INT(2);
//	char *ext = AG_STRING(3);

	gui->ShowSavePrinterDialog(drv);
}
// update Save Printer
void AG_GUI_BASE::OnUpdateSavePrinter(AG_Event *event)
{
	AG_MenuItem *mi = (AG_MenuItem *)AG_SENDER();
	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);
	int drv = AG_INT(2);

	int size = gui->GetPrinterBufferSize(drv);
	AG_GUI_MENU_ENABLE(mi, size > 0);
}
// Clear Print Data
void AG_GUI_BASE::OnSelectClearPrinter(AG_Event *event)
{
	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);
	int drv = AG_INT(2);

	gui->PostEtClearPrinterBufferMessage(drv);
}
// update Clear Printer
void AG_GUI_BASE::OnUpdateClearPrinter(AG_Event *event)
{
	AG_MenuItem *mi = (AG_MenuItem *)AG_SENDER();
	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);
	int drv = AG_INT(2);

	int size = gui->GetPrinterBufferSize(drv);
	AG_GUI_MENU_ENABLE(mi, size > 0);
}

///
/// process user event
void AG_GUI_BASE::OnProcessUserEvent(AG_Event *event)
{
	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);
	int id = AG_INT(2);

	gui->ProcessUserEvent(id);
}

////////////////////////////////////////
// select About dialog
void AG_GUI_BASE::OnSelectAbout(AG_Event *event)
{
	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);

	gui->ShowAboutDialog();
}

////////////////////////////////////////
// create virtual keyboard dialog
void AG_GUI_BASE::OnSelectVirtualKeyboard(AG_Event *event)
{
	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);

	gui->ShowVirtualKeyboard();
}

void AG_GUI_BASE::OnUpdateVirtualKeyboard(AG_Event *event)
{
	AG_MenuItem *mi = (AG_MenuItem *)AG_SENDER();
	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);

	AG_GUI_MENU_CHECK(mi, gui->IsShownVirtualKeyboard());
}

////////////////////////////////////////

#ifdef USE_DEBUGGER

void AG_GUI_BASE::OnSelectOpenDebugger(AG_Event *event)
{
	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);
//	int num = AG_INT(2);

	gui->OpenDebugger();
}
void AG_GUI_BASE::OnUpdateOpenDebugger(AG_Event *event)
{
	AG_MenuItem *mi = (AG_MenuItem *)AG_SENDER();
	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);

	AG_GUI_MENU_ENABLE(mi, !gui->IsDebuggerOpened());
}
void AG_GUI_BASE::OnSelectCloseDebugger(AG_Event *event)
{
	AG_GUI_BASE *gui = (AG_GUI_BASE *)AG_PTR(1);

	gui->CloseDebugger();
}

#endif

}; /* namespace GUI_AGAR */
