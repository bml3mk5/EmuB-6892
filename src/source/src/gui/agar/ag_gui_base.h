/** @file ag_gui_base.h

	Skelton for retropc emulator
	SDL + Agar edition

	@author Sasaji
	@date   2012.03.01

	@brief [ ag_gui_base ]
*/

#ifndef AG_GUI_BASE_H
#define AG_GUI_BASE_H

#include "../../common.h"
#include "../../rec_video_defs.h"
#include <sys/types.h>
#include <agar/config/have_sdl.h>
#include <agar/core.h>
#include <agar/gui.h>
#include <agar/gui/cursors.h>
#include <agar/gui/sdl.h>
#include <SDL.h>
#include "../../emu.h"
#include "../gui_base.h"

class EMU;
class GUI_BASE;
class CRecentPathList;

namespace GUI_AGAR
{

class AG_DLG;
class AG_FILE_DLG;
class AG_VOLUME_DLG;
class AG_SELDRV_DLG;
#ifdef USE_REC_VIDEO
class AG_RECVIDEO_DLG;
#endif
#ifdef USE_REC_AUDIO
class AG_RECAUDIO_DLG;
#endif

#define AG_GUI_MENU_CHECK(mitem, val) { \
	if ((mitem) && (mitem)->text) { \
		if (val) { \
			*(mitem)->text = '*'; \
		} else { \
			*(mitem)->text = ' '; \
		} \
	} \
}
#define AG_GUI_MENU_ENABLE(mitem, val) { \
	if (mitem) { \
		if (val) { \
			mitem->state = 1; \
		} else { \
			mitem->state = 0; \
		} \
	} \
}

/**
	@brief gui base class
*/
class AG_GUI_BASE : public GUI_BASE {
private:
	int exit_event;
	int first;

	virtual void set_menu_item(AG_Menu *);
	virtual void set_global_keys();

	void event_proc(AG_DriverEvent *);
	void gui_focus(int, int);

	void translate_keycode(SDL_KeyboardEvent *);

	void update_multi_volume_list(AG_MenuItem *mi, int drv);

protected:

	typedef struct st_ag_drvs {
		AG_DriverClass	*ops;
		AG_DriverSw		*sw;
		AG_Window		*win;
		AG_Menu			*menu;
	} ag_driver_t;

	ag_driver_t drivers[2];
	ag_driver_t *now_driver;

	bool now_showing;

	const char *menu_str(const char *str);

	// menu event handler
	static void OnSelectExit(AG_Event *);

	static void OnSelectReset(AG_Event *);
	static void OnUpdateReset(AG_Event *);
	static void OnSelectSpecialReset(AG_Event *);
	static void OnSelectWarmReset(AG_Event *);

	static void OnSelectDipswitch(AG_Event *);
	static void OnUpdateDipswitch(AG_Event *);

	static void OnSelectPause(AG_Event *);
	static void OnUpdatePause(AG_Event *);

	static void OnSelectCPUPower(AG_Event *);
	static void OnUpdateCPUPower(AG_Event *);

	static void OnSelectOpenAutoKey(AG_Event *);
	static void OnUpdateOpenAutoKey(AG_Event *);
	static void OnSelectStartAutoKey(AG_Event *);
	static void OnUpdateStartAutoKey(AG_Event *);
	static void OnSelectStopAutoKey(AG_Event *);

	static void OnSelectLoadDataRec(AG_Event *);
	static void OnUpdateLoadDataRec(AG_Event *);
	static void OnSelectSaveDataRec(AG_Event *);
	static void OnUpdateSaveDataRec(AG_Event *);
	static void OnSelectCloseDataRec(AG_Event *);
	static void OnSelectRecentDataRec(AG_Event *);
	static void OnUpdateRecentDataRec(AG_Event *);

	static void OnSelectOpenFloppyDisk(AG_Event *);
	static void OnUpdateOpenFloppyDisk(AG_Event *);
	static void OnSelectCloseFloppyDisk(AG_Event *);
	static void OnSelectRecentFloppyDisk(AG_Event *);
	static void OnUpdateRecentFloppyDisk(AG_Event *);
	static void OnSelectOpenBlankFloppyDisk(AG_Event *);

	static void OnUpdateMultiVolumeList(AG_Event *);
	static void OnSelectVolumeFloppyDisk(AG_Event *);
	static void OnUpdateVolumeFloppyDisk(AG_Event *);

	static void OnSelectFrameRate(AG_Event *);
	static void OnUpdateFrameRate(AG_Event *);
	static void OnSelectScreenRecordSize(AG_Event *);
	static void OnUpdateScreenRecordSize(AG_Event *);
	static void OnSelectScreenRecordFrameRate(AG_Event *);
	static void OnUpdateScreenRecordFrameRate(AG_Event *);
	static void OnSelectStopScreenRecord(AG_Event *);
	static void OnUpdateStopScreenRecord(AG_Event *);
	static void OnSelectScreenCapture(AG_Event *);
	static void OnSelectWindowMode(AG_Event *);
	static void OnUpdateWindowMode(AG_Event *);
	static void OnSelectScreenMode(AG_Event *);
	static void OnUpdateScreenMode(AG_Event *);
	static void OnSelectScanLine(AG_Event *);
	static void OnUpdateScanLine(AG_Event *);
	static void OnSelectPixelAspect(AG_Event *);
	static void OnUpdatePixelAspect(AG_Event *);

	static void OnSelectSoundVolume(AG_Event *);
	static void OnSelectSoundStartRecord(AG_Event *);
	static void OnUpdateSoundStartRecord(AG_Event *);
	static void OnSelectSoundStopRecord(AG_Event *);
	static void OnUpdateSoundStopRecord(AG_Event *);
	static void OnSelectSoundRate(AG_Event *);
	static void OnUpdateSoundRate(AG_Event *);
	static void OnSelectSoundLatency(AG_Event *);
	static void OnUpdateSoundLatency(AG_Event *);

	static void OnSelectSavePrinter(AG_Event *);
	static void OnUpdateSavePrinter(AG_Event *);
	static void OnSelectClearPrinter(AG_Event *);
	static void OnUpdateClearPrinter(AG_Event *);

//	static void OnUpdateRecent(AG_Event *);

	static void OnSelectAbout(AG_Event *);

	static void OnSelectVirtualKeyboard(AG_Event *);
	static void OnUpdateVirtualKeyboard(AG_Event *);

	static void OnCloseDialog(AG_Event *);

	static void OnProcessUserEvent(AG_Event *);

#ifdef USE_DEBUGGER
	static void OnSelectOpenDebugger(AG_Event *);
	static void OnUpdateOpenDebugger(AG_Event *);
	static void OnSelectCloseDebugger(AG_Event *);
#endif

public:
	AG_GUI_BASE(int, char **, EMU *);
	virtual ~AG_GUI_BASE();

	// event data from SDL
	SDL_Event sdl_event;

	// file dialog
	AG_FILE_DLG *filebox;
	// volume dialog
	AG_VOLUME_DLG *volumebox;
	// select drive dialog
	AG_SELDRV_DLG *seldrvbox;
#ifdef USE_REC_VIDEO
	// record video dialog
	AG_RECVIDEO_DLG *recvidbox;
#endif
#ifdef USE_REC_AUDIO
	// record audio dialog
	AG_RECAUDIO_DLG *recaudbox;
#endif

	virtual int Init();

	virtual int CreateWidget(SDL_Surface *, int, int);
	virtual int CreateMenu();

	int ResizeDisplay(int width, int height);

	virtual int CreateGlobalKeys();
	virtual int ProcessEvent(SDL_Event *);
	virtual int MixSurface();
	virtual void PostProcessEvent();
	virtual void ShowMenu();
	virtual void HideMenu();

	int ConfigLoad();
	int ConfigSave();

	AG_Window *GetWindow();
	virtual AG_DLG *GetDlgPtr(int);

//	virtual bool ExecuteGlobalKeys(int key, Uint32 mod);
	virtual void ProcessUserEvent(int id);

	virtual void ScreenModeChanged(bool fullscreen);

	virtual void ChangeWindowMode(int num);
	virtual void ChangeFullScreenMode(int num);

	// event operation
	virtual bool ShowRecordVideoDialog(int fps_num);
	virtual bool ShowRecordAudioDialog();

	virtual bool ShowVolumeDialog(void);

	virtual bool ShowOpenFloppyDiskDialog(int drv);
	void         ShowAgSelectFloppyDriveDialog(int drv, int num);
	virtual bool ShowOpenBlankFloppyDiskDialog(int drv, uint8_t type);
	void         PostEtAgOpenRecentFloppyMessage(int drv, int num, int new_drv);

	virtual bool ShowOpenAutoKeyDialog(void);

	virtual bool ShowSavePrinterDialog(int drv);

	virtual bool ShowAboutDialog(void);

	void update_recent_list(AG_MenuItem *mi, CRecentPathList &list, int drv, AG_EventFn fn);
//	void update_recent_list(AG_MenuItem *mi, int type, int drv);
};

}; /* namespace GUI_AGAR */

#endif /* AG_GUI_BASE_H */
