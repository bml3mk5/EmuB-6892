/** @file ag_gui.cpp

	HITACHI BASIC MASTER LEVEL3 Mark5 / MB-S1 Emulator 'EmuB-6892/EmuB-S1'
	Skelton for retropc emulator
	SDL + Ager edition

	@author Sasaji
	@date   2012.3.3

	@brief [ ag_gui ]
*/

#include "ag_gui.h"
#include "ag_file_dlg.h"
#include "ag_config_dlg.h"
#include "ag_keybind_dlg.h"
#include "../../clocale.h"
#include "../../main.h"
#include "../../depend.h"
#if defined(USE_LIB_GTK)
#include <gtk/gtk.h>
#endif

using namespace GUI_AGAR;

/****************************************

 GUI class using AGAR

 ****************************************/

GUI::GUI(int argc, char **argv, EMU *new_emu) : AG_GUI_BASE(argc, argv, new_emu)
{
#if defined(USE_LIB_GTK)
	gtk_init(&argc, &argv);
#endif
	configbox = NULL;
	keybindbox = NULL;
}

GUI::~GUI()
{
	delete keybindbox;
	delete configbox;
}

int GUI::Init()
{
	int rc = 0;

	rc = AG_GUI_BASE::Init();
	if (rc < 0) return rc;

	// dialog
	configbox = new AG_CONFIG_DLG(emu, this);
	keybindbox = new AG_KEYBIND_DLG(emu, this);

	return rc;
}

AG_DLG *GUI::GetDlgPtr(int id)
{
	AG_DLG *p = NULL;

	switch(id) {
		case ID_KEYBINDBOX:
			p = keybindbox;
			break;
		default:
			p = configbox;
			break;
	}
	return p;
}

/**
 * create menu bar
 */
void GUI::set_menu_item(AG_Menu *menu)
{
	AG_MenuItem *mt_control;
	AG_MenuItem *mt_tape;
	AG_MenuItem *mt_fdd;
	AG_MenuItem *mt_screen;
	AG_MenuItem *mt_sound;
	AG_MenuItem *mt_devices;
	AG_MenuItem *mt_options;
	AG_MenuItem *mt_help;

	AG_MenuItem *mi;
	AG_MenuItem *ms;
	AG_MenuItem *mss;

	char name[128];

	// create menu
	mt_control = AG_MenuNode(menu->root, CMSG(Control), NULL);
	{
		//                                **--------------- ------
		mi = AG_MenuAction(mt_control, menu_str(CMSG(PowerOn_Alt)), NULL, OnSelectReset, "%Cp", this);
			 AG_MenuSetPollFn(mi, OnUpdateReset, "%Cp", this);
#ifdef _BML3MK5
		mi = AG_MenuAction(mt_control, menu_str(CMSG(MODE_Switch_Alt)), NULL, OnSelectDipswitch, "%Cp %i", this, 2);
			 AG_MenuSetPollFn(mi, OnUpdateDipswitch, "%Cp %i", this, 2);
#endif
		mi = AG_MenuAction(mt_control, menu_str(CMSG(Reset_Switch_Alt)), NULL, OnSelectWarmReset, "%Cp", this);
#ifdef _MBS1
		AG_MenuSeparator(mt_control);
		mi = AG_MenuNode(mt_control,   menu_str(CMSG(System_Mode)), NULL);
			ms = AG_MenuAction(mi,     menu_str(CMSG(A_Mode_S1_Alt)), NULL, OnSelectSystemMode, "%Cp %i", this, 1);
				 AG_MenuSetPollFn(ms, OnUpdateSystemMode, "%Cp %i", this, 1);
			ms = AG_MenuAction(mi,     menu_str(CMSG(B_Mode_L3_Alt)), NULL, OnSelectSystemMode, "%Cp %i", this, 0);
				 AG_MenuSetPollFn(ms, OnUpdateSystemMode, "%Cp %i", this, 0);
#endif
		AG_MenuSeparator(mt_control);
		mi = AG_MenuNode(mt_control,   menu_str(CMSG(FDD_Type)), NULL);
			ms = AG_MenuAction(mi,     menu_str(CMSG(No_FDD_Alt)), NULL, OnSelectFDDType, "%Cp %i", this, 0);
				 AG_MenuSetPollFn(ms, OnUpdateFDDType, "%Cp %i", this, 0);
			ms = AG_MenuAction(mi,     menu_str(CMSG(FD3inch_compact_FDD_Alt)), NULL, OnSelectFDDType, "%Cp %i", this, 1);
				 AG_MenuSetPollFn(ms, OnUpdateFDDType, "%Cp %i", this, 1);
#ifdef _MBS1
			ms = AG_MenuAction(mi,     menu_str(CMSG(FD5inch_mini_FDD_2D_Type_Alt)), NULL, OnSelectFDDType, "%Cp %i", this, 2);
				 AG_MenuSetPollFn(ms, OnUpdateFDDType, "%Cp %i", this, 2);
			ms = AG_MenuAction(mi,     menu_str(CMSG(FD5inch_mini_FDD_2HD_Type_Alt)), NULL, OnSelectFDDType, "%Cp %i", this, 3);
				 AG_MenuSetPollFn(ms, OnUpdateFDDType, "%Cp %i", this, 3);
#else
			ms = AG_MenuAction(mi,     menu_str(CMSG(FD5inch_mini_FDD_Alt)), NULL, OnSelectFDDType, "%Cp %i", this, 2);
				 AG_MenuSetPollFn(ms, OnUpdateFDDType, "%Cp %i", this, 2);
			ms = AG_MenuAction(mi,     menu_str(CMSG(FD8inch_standard_FDD_Alt)), NULL, OnSelectFDDType, "%Cp %i", this, 3);
				 AG_MenuSetPollFn(ms, OnUpdateFDDType, "%Cp %i", this, 3);
#endif
		AG_MenuSeparator(mt_control);
		mi = AG_MenuAction(mt_control, menu_str(CMSG(Pause_Alt)), NULL, OnSelectPause, "%Cp", this);
			 AG_MenuSetPollFn(mi, OnUpdatePause, "%Cp", this);
		AG_MenuSeparator(mt_control);
		mi = AG_MenuNode(mt_control,   menu_str(CMSG(CPU_Speed)), NULL);
			ms = AG_MenuAction(mi,     menu_str(CMSG(CPU_x05_Alt)), NULL, OnSelectCPUPower, "%Cp %i", this, 0);
				 AG_MenuSetPollFn(ms, OnUpdateCPUPower, "%Cp %i", this, 0);
			ms = AG_MenuAction(mi,     menu_str(CMSG(CPU_x1_Alt)), NULL, OnSelectCPUPower, "%Cp %i", this, 1);
				 AG_MenuSetPollFn(ms, OnUpdateCPUPower, "%Cp %i", this, 1);
			ms = AG_MenuAction(mi,     menu_str(CMSG(CPU_x2_Alt)), NULL, OnSelectCPUPower, "%Cp %i", this, 2);
				 AG_MenuSetPollFn(ms, OnUpdateCPUPower, "%Cp %i", this, 2);
			ms = AG_MenuAction(mi,     menu_str(CMSG(CPU_x4_Alt)), NULL, OnSelectCPUPower, "%Cp %i", this, 3);
				 AG_MenuSetPollFn(ms, OnUpdateCPUPower, "%Cp %i", this, 3);
			ms = AG_MenuAction(mi,     menu_str(CMSG(CPU_x8_Alt)), NULL, OnSelectCPUPower, "%Cp %i", this, 4);
				 AG_MenuSetPollFn(ms, OnUpdateCPUPower, "%Cp %i", this, 4);
			ms = AG_MenuAction(mi,     menu_str(CMSG(CPU_x16_Alt)), NULL, OnSelectCPUPower, "%Cp %i", this, 5);
				 AG_MenuSetPollFn(ms, OnUpdateCPUPower, "%Cp %i", this, 5);
			AG_MenuSeparator(mi);
			ms = AG_MenuAction(mi, menu_str(CMSG(Sync_With_CPU_Speed_Alt)), NULL, OnSelectSyncIRQ, "%Cp", this);
				AG_MenuSetPollFn(ms, OnUpdateSyncIRQ, "%Cp", this);
#ifdef _MBS1
//		AG_MenuSeparator(mt_control);
//		mi = AG_MenuAction(mt_control, menu_str(CMSG(Memory_No_Wait)), NULL, OnSelectMemNoWait, "%Cp", this);
//			 AG_MenuSetPollFn(mi, OnUpdateMemNoWait, "%Cp", this);
#endif
		AG_MenuSeparator(mt_control);
		mi = AG_MenuNode(mt_control,   menu_str(CMSG(Auto_Key)), NULL);
			ms = AG_MenuAction(mi,     menu_str(CMSG(Open_)), NULL, OnSelectOpenAutoKey, "%Cp", this);
				 AG_MenuSetPollFn(ms, OnUpdateOpenAutoKey, "%Cp", this);
			ms = AG_MenuAction(mi,     menu_str(CMSG(Paste)), NULL, OnSelectStartAutoKey, "%Cp", this);
				 AG_MenuSetPollFn(ms, OnUpdateStartAutoKey, "%Cp", this);
			 ms = AG_MenuAction(mi    , menu_str(CMSG(Stop)), NULL, OnSelectStopAutoKey, "%Cp", this);
		AG_MenuSeparator(mt_control);
		mi = AG_MenuNode(mt_control,   menu_str(CMSG(Record_Key)), NULL);
			ms = AG_MenuAction(mi,     menu_str(CMSG(Play_Alt_E)), NULL, OnSelectPlayRecKey, "%Cp", this);
				 AG_MenuSetPollFn(ms, OnUpdatePlayRecKey, "%Cp", this);
			ms = AG_MenuAction(mi    , menu_str(CMSG(Stop_Playing)), NULL, OnSelectStopPlayRecKey, "%Cp", this);
			AG_MenuSeparator(mi);
			ms = AG_MenuAction(mi,     menu_str(CMSG(Record_)), NULL, OnSelectRecordRecKey, "%Cp", this);
				 AG_MenuSetPollFn(ms, OnUpdateRecordRecKey, "%Cp", this);
			ms = AG_MenuAction(mi    , menu_str(CMSG(Stop_Recording)), NULL, OnSelectStopRecordRecKey, "%Cp", this);
		AG_MenuSeparator(mt_control);
		mi = AG_MenuAction(mt_control, menu_str(CMSG(Load_State_Alt)), NULL, OnSelectLoadState, "%Cp", this);
		mi = AG_MenuAction(mt_control, menu_str(CMSG(Save_State_)), NULL, OnSelectSaveState, "%Cp", this);
		AG_MenuSeparator(mt_control);
		mi = AG_MenuDynamicItem(mt_control, menu_str(CMSG(Recent_State_Files)), NULL, OnUpdateRecentState, "%Cp", this);
		AG_MenuSeparator(mt_control);
		mi = AG_MenuAction(mt_control, menu_str(CMSG(Exit_Alt)), NULL, OnSelectExit, "%Cp", this);
	}
	mt_tape = AG_MenuNode(menu->root, CMSG(Tape), NULL);
	{
		//                             **--------------- ------
		mi = AG_MenuAction(mt_tape, menu_str(CMSG(Play_Alt_F7)), NULL, OnSelectLoadDataRec, "%Cp", this);
			 AG_MenuSetPollFn(mi, OnUpdateLoadDataRec, "%Cp", this);
		mi = AG_MenuAction(mt_tape, menu_str(CMSG(Rec_)), NULL, OnSelectSaveDataRec, "%Cp", this);
			 AG_MenuSetPollFn(mi, OnUpdateSaveDataRec, "%Cp", this);
		mi = AG_MenuAction(mt_tape, menu_str(CMSG(Eject)), NULL, OnSelectCloseDataRec, "%Cp", this);
		AG_MenuSeparator(mt_tape);
		mi = AG_MenuAction(mt_tape, menu_str(CMSG(Rewind_Alt)), NULL, OnSelectRewindDataRec, "%Cp", this);
		mi = AG_MenuAction(mt_tape, menu_str(CMSG(F_F_Alt)), NULL, OnSelectFastForwardDataRec, "%Cp", this);
		mi = AG_MenuAction(mt_tape, menu_str(CMSG(Stop_Alt)), NULL, OnSelectStopDataRec, "%Cp", this);
		AG_MenuSeparator(mt_tape);
		mi = AG_MenuAction(mt_tape, menu_str(CMSG(Real_Mode)), NULL, OnSelectRealModeDataRec, "%Cp", this);
			 AG_MenuSetPollFn(mi, OnUpdateRealModeDataRec, "%Cp", this);
		AG_MenuSeparator(mt_tape);
		mi = AG_MenuDynamicItem(mt_tape, menu_str(CMSG(Recent_Files)), NULL, OnUpdateRecentDataRec, "%Cp", this);
	}
	for(int drv=0; drv<USE_DRIVE; drv++) {
		sprintf(name,CMSG(FDDVDIGIT),drv);
		mt_fdd = AG_MenuNode(menu->root, name, NULL);
		{
		//                                **--------------- ------
			sprintf(name,              menu_str(CMSG(Insert_Alt)), drv + 9);
			mi = AG_MenuAction(mt_fdd, name, NULL, OnSelectOpenFloppyDisk, "%Cp %i", this, drv);
				 AG_MenuSetPollFn(mi, OnUpdateOpenFloppyDisk, "%Cp %i", this, drv);
			mi = AG_MenuAction(mt_fdd, menu_str(CMSG(Change_Side_to_B)), NULL, OnSelectChangeSideFloppyDisk, "%Cp %i", this, drv);
				 AG_MenuSetPollFn(mi, OnUpdateChangeSideFloppyDisk, "%Cp %i", this, drv);
			mi = AG_MenuAction(mt_fdd, menu_str(CMSG(Eject)), NULL, OnSelectCloseFloppyDisk, "%Cp %i", this, drv);
			mi = AG_MenuNode(mt_fdd,   menu_str(CMSG(New)), NULL);
				ms = AG_MenuAction(mi, menu_str(CMSG(Insert_Blank_2D_)),  NULL, OnSelectOpenBlankFloppyDisk, "%Cp %i %i", this, drv, 0x00);
				ms = AG_MenuAction(mi, menu_str(CMSG(Insert_Blank_2HD_)), NULL, OnSelectOpenBlankFloppyDisk, "%Cp %i %i", this, drv, 0x20);
			AG_MenuSeparator(mt_fdd);
			mi = AG_MenuAction(mt_fdd, menu_str(CMSG(Write_Protect)), NULL, OnSelectWriteProtectFloppyDisk, "%Cp %i", this, drv);
				 AG_MenuSetPollFn(mi, OnUpdateWriteProtectFloppyDisk, "%Cp %i", this, drv);
			AG_MenuSeparator(mt_fdd);
			mi = AG_MenuDynamicItem(mt_fdd, menu_str(CMSG(Multi_Volume)), NULL, OnUpdateMultiVolumeList, "%Cp %i", this, drv);
			AG_MenuSeparator(mt_fdd);
			mi = AG_MenuDynamicItem(mt_fdd, menu_str(CMSG(Recent_Files)), NULL, OnUpdateRecentFloppyDisk, "%Cp %i", this, drv);
		}
	}
	mt_screen = AG_MenuNode(menu->root, CMSG(Screen), NULL);
	{
		//                               **--------------- ------
		mi = AG_MenuNode(mt_screen,   menu_str(CMSG(Frame_Rate)), NULL);
			ms = AG_MenuAction(mi,    menu_str(CMSG(Auto)), NULL, OnSelectFrameRate, "%Cp %i", this, -1);
				 AG_MenuSetPollFn(ms, OnUpdateFrameRate, "%Cp %i", this, -1);
			ms = AG_MenuAction(mi,    menu_str(CMSG(F60fps)), NULL, OnSelectFrameRate, "%Cp %i", this, 0);
				 AG_MenuSetPollFn(ms, OnUpdateFrameRate, "%Cp %i", this, 0);
			ms = AG_MenuAction(mi,    menu_str(CMSG(F30fps)), NULL, OnSelectFrameRate, "%Cp %i", this, 1);
				 AG_MenuSetPollFn(ms, OnUpdateFrameRate, "%Cp %i", this, 1);
			ms = AG_MenuAction(mi,    menu_str(CMSG(F20fps)), NULL, OnSelectFrameRate, "%Cp %i", this, 2);
				 AG_MenuSetPollFn(ms, OnUpdateFrameRate, "%Cp %i", this, 2);
			ms = AG_MenuAction(mi,    menu_str(CMSG(F15fps)), NULL, OnSelectFrameRate, "%Cp %i", this, 3);
				 AG_MenuSetPollFn(ms, OnUpdateFrameRate, "%Cp %i", this, 3);
			ms = AG_MenuAction(mi,    menu_str(CMSG(F12fps)), NULL, OnSelectFrameRate, "%Cp %i", this, 4);
				 AG_MenuSetPollFn(ms, OnUpdateFrameRate, "%Cp %i", this, 4);
			ms = AG_MenuAction(mi,    menu_str(CMSG(F10fps)), NULL, OnSelectFrameRate, "%Cp %i", this, 5);
				 AG_MenuSetPollFn(ms, OnUpdateFrameRate, "%Cp %i", this, 5);
		AG_MenuSeparator(mt_screen);
		mi = AG_MenuNode(mt_screen,   menu_str(CMSG(Record_Screen)), NULL);
		{
			for(int i = 0; i < 2; i++) {
				strcpy(name, "  ");
				GetRecordVideoSizeStr(i, &name[2]);
				ms = AG_MenuAction(mi, name, NULL, OnSelectScreenRecordSize, "%Cp %i", this, i);
					 AG_MenuSetPollFn(ms, OnUpdateScreenRecordSize, "%Cp %i", this, i);
			}
			AG_MenuSeparator(mi);
#ifdef USE_REC_VIDEO
			ms = AG_MenuAction(mi,    menu_str(CMSG(Rec_60fps)), NULL, OnSelectScreenRecordFrameRate, "%Cp %i", this, 0);
			AG_MenuSetPollFn(ms, OnUpdateScreenRecordFrameRate, "%Cp %i", this, 0);
			ms = AG_MenuAction(mi,    menu_str(CMSG(Rec_30fps)), NULL, OnSelectScreenRecordFrameRate, "%Cp %i", this, 1);
			AG_MenuSetPollFn(ms, OnUpdateScreenRecordFrameRate, "%Cp %i", this, 1);
			ms = AG_MenuAction(mi,    menu_str(CMSG(Rec_20fps)), NULL, OnSelectScreenRecordFrameRate, "%Cp %i", this, 2);
			AG_MenuSetPollFn(ms, OnUpdateScreenRecordFrameRate, "%Cp %i", this, 2);
			ms = AG_MenuAction(mi,    menu_str(CMSG(Rec_15fps)), NULL, OnSelectScreenRecordFrameRate, "%Cp %i", this, 3);
			AG_MenuSetPollFn(ms, OnUpdateScreenRecordFrameRate, "%Cp %i", this, 3);
			ms = AG_MenuAction(mi,    menu_str(CMSG(Rec_12fps)), NULL, OnSelectScreenRecordFrameRate, "%Cp %i", this, 4);
			AG_MenuSetPollFn(ms, OnUpdateScreenRecordFrameRate, "%Cp %i", this, 4);
			ms = AG_MenuAction(mi,    menu_str(CMSG(Rec_10fps)), NULL, OnSelectScreenRecordFrameRate, "%Cp %i", this, 5);
			AG_MenuSetPollFn(ms, OnUpdateScreenRecordFrameRate, "%Cp %i", this, 5);
			ms = AG_MenuAction(mi,    menu_str(CMSG(Stop)), NULL, OnSelectStopScreenRecord, "%Cp", this);
//			AG_MenuSetPollFn(ms, OnUpdateStopScreenRecord, "%Cp", this);
			AG_MenuSeparator(mi);
#endif
			ms = AG_MenuAction(mi,    menu_str(CMSG(Capture)), NULL, OnSelectScreenCapture, "%Cp", this);
		}
		AG_MenuSeparator(mt_screen);
		mi = AG_MenuNode(mt_screen,   menu_str(CMSG(Window)), NULL);
		for(int i = 0; i < GetWindowModeCount(); i++) {
			strcpy(name, "  ");
			GetWindowModeStr(i, &name[2]);
			ms = AG_MenuAction(mi, name, NULL, OnSelectWindowMode, "%Cp %i", this, i);
				 AG_MenuSetPollFn(ms, OnUpdateWindowMode, "%Cp %i", this, i);
		}
		mi = AG_MenuNode(mt_screen,   menu_str(CMSG(Fullscreen)), NULL);
			ms = AG_MenuAction(mi, menu_str(CMSG(Stretch_Screen_Alt)), NULL, OnSelectStretchScreen, "%Cp", this);
				 AG_MenuSetPollFn(ms, OnUpdateStretchScreen, "%Cp", this);
			ms = AG_MenuAction(mi, menu_str(CMSG(Cutout_Screen_Alt)), NULL, OnSelectCutoutScreen, "%Cp", this);
				 AG_MenuSetPollFn(ms, OnUpdateCutoutScreen, "%Cp", this);
			AG_MenuSeparator(mi);
		for(int disp_no = 0; disp_no < GetDisplayDeviceCount(); disp_no++) {
			strcpy(name, "  ");
			GetDisplayDeviceStr(CMSG(Display), disp_no, &name[2]);
			ms = AG_MenuNode(mi,      name, NULL);
			for(int i = 0; i < GetFullScreenModeCount(disp_no); i++) {
				strcpy(name, "  ");
				GetFullScreenModeStr(disp_no, i, &name[2]);
				mss = AG_MenuAction(ms, name, NULL, OnSelectScreenMode, "%Cp %i", this, disp_no * VIDEO_MODE_MAX + i);
					  AG_MenuSetPollFn(mss, OnUpdateScreenMode, "%Cp %i", this, disp_no * VIDEO_MODE_MAX + i);
			}
		}
		mi = AG_MenuNode(mt_screen,   menu_str(CMSG(Aspect_Ratio)), NULL);
		for(int i = 0; i < GetPixelAspectModeCount(); i++) {
			strcpy(name, "  ");
			GetPixelAspectModeStr(i, &name[2]);
			ms = AG_MenuAction(mi, name, NULL, OnSelectPixelAspect, "%Cp %i", this, i);
				 AG_MenuSetPollFn(ms, OnUpdatePixelAspect, "%Cp %i", this, i);
		}
		AG_MenuSeparator(mt_screen);
		mi = AG_MenuNode(mt_screen,   menu_str(CMSG(Drawing_Mode)), NULL);
			ms = AG_MenuAction(mi, menu_str(CMSG(Full_Draw_Alt)), NULL, OnSelectScanLine, "%Cp %i", this, 0);
				 AG_MenuSetPollFn(ms, OnUpdateScanLine, "%Cp %i", this, 0);
			ms = AG_MenuAction(mi, menu_str(CMSG(Scanline_Alt)), NULL, OnSelectScanLine, "%Cp %i", this, 1);
				 AG_MenuSetPollFn(ms, OnUpdateScanLine, "%Cp %i", this, 1);
			ms = AG_MenuAction(mi, menu_str(CMSG(Stripe_Alt)), NULL, OnSelectScanLine, "%Cp %i", this, 2);
				 AG_MenuSetPollFn(ms, OnUpdateScanLine, "%Cp %i", this, 2);
			ms = AG_MenuAction(mi, menu_str(CMSG(Checker_Alt)), NULL, OnSelectScanLine, "%Cp %i", this, 3);
				 AG_MenuSetPollFn(ms, OnUpdateScanLine, "%Cp %i", this, 3);
		AG_MenuSeparator(mt_screen);
		mi = AG_MenuAction(mt_screen, menu_str(CMSG(Afterimage1_Alt)), NULL, OnSelectAfterImage, "%Cp %i", this, 1);
			 AG_MenuSetPollFn(mi, OnUpdateAfterImage, "%Cp %i", this, 1);
		mi = AG_MenuAction(mt_screen, menu_str(CMSG(Afterimage2_Alt)), NULL, OnSelectAfterImage, "%Cp %i", this, 2);
			 AG_MenuSetPollFn(mi, OnUpdateAfterImage, "%Cp %i", this, 2);
#ifdef USE_KEEPIMAGE
		AG_MenuSeparator(mt_screen);
		mi = AG_MenuAction(mt_screen, menu_str(CMSG(Keepimage1)), NULL, OnSelectKeepImage, "%Cp %i", this, 1);
			 AG_MenuSetPollFn(mi, OnUpdateKeepImage, "%Cp %i", this, 1);
		mi = AG_MenuAction(mt_screen, menu_str(CMSG(Keepimage2)), NULL, OnSelectKeepImage, "%Cp %i", this, 2);
			 AG_MenuSetPollFn(mi, OnUpdateKeepImage, "%Cp %i", this, 2);
#endif
#ifdef _MBS1
		AG_MenuSeparator(mt_screen);
		mi = AG_MenuAction(mt_screen, menu_str(CMSG(Digital_RGB)), NULL, OnSelectRGBType, "%Cp %i", this, 0);
			 AG_MenuSetPollFn(mi, OnUpdateRGBType, "%Cp %i", this, 0);
		mi = AG_MenuAction(mt_screen, menu_str(CMSG(Analog_RGB)), NULL, OnSelectRGBType, "%Cp %i", this, 1);
			 AG_MenuSetPollFn(mi, OnUpdateRGBType, "%Cp %i", this, 1);
#endif
#ifdef USE_OPENGL
		AG_MenuSeparator(mt_screen);
		mi = AG_MenuAction(mt_screen, menu_str(CMSG(Use_OpenGL_Sync_Alt)), NULL, OnSelectUseOpenGL, "%Cp %i", this, 1);
			 AG_MenuSetPollFn(mi, OnUpdateUseOpenGL, "%Cp %i", this, 1);
		mi = AG_MenuAction(mt_screen, menu_str(CMSG(Use_OpenGL_Async_Alt)), NULL, OnSelectUseOpenGL, "%Cp %i", this, 2);
			 AG_MenuSetPollFn(mi, OnUpdateUseOpenGL, "%Cp %i", this, 2);
		mi = AG_MenuNode(mt_screen,   menu_str(CMSG(OpenGL_Filter)), NULL);
			ms = AG_MenuAction(mi, menu_str(CMSG(Nearest_Neighbour_Alt)), NULL, OnSelectOpenGLFilter, "%Cp %i", this, 0);
				 AG_MenuSetPollFn(ms, OnUpdateOpenGLFilter, "%Cp %i", this, 0);
			ms = AG_MenuAction(mi, menu_str(CMSG(Linear_Alt)), NULL, OnSelectOpenGLFilter, "%Cp %i", this, 1);
				 AG_MenuSetPollFn(ms, OnUpdateOpenGLFilter, "%Cp %i", this, 1);
#endif
	}
	mt_sound = AG_MenuNode(menu->root, CMSG(Sound), NULL);
	{
		//                              **--------------- ------
		mi = AG_MenuAction(mt_sound, menu_str(CMSG(Volume_Alt)), NULL, OnSelectSoundVolume, "%Cp", this);
		AG_MenuSeparator(mt_sound);
		mi = AG_MenuNode(mt_sound,   menu_str(CMSG(Record_Sound)), NULL);
		{
			ms = AG_MenuAction(mi, menu_str(CMSG(Start_)), NULL, OnSelectSoundStartRecord, "%Cp", this);
				 AG_MenuSetPollFn(ms, OnUpdateSoundStartRecord, "%Cp", this);
			ms = AG_MenuAction(mi, menu_str(CMSG(Stop)), NULL, OnSelectSoundStopRecord, "%Cp", this);
//				 AG_MenuSetPollFn(ms, OnUpdateSoundStopRecord, "%Cp", this);
		}
		AG_MenuSeparator(mt_sound);
//		mi = AG_MenuAction(mt_sound, menu_str(CMSG(F2000Hz)), NULL, OnSelectSoundRate, "%Cp %i", this, 0);
//			 AG_MenuSetPollFn(mi, OnUpdateSoundRate, "%Cp %i", this, 0);
//		mi = AG_MenuAction(mt_sound, menu_str(CMSG(F4000Hz)), NULL, OnSelectSoundRate, "%Cp %i", this, 1);
//			 AG_MenuSetPollFn(mi, OnUpdateSoundRate, "%Cp %i", this, 1);
		mi = AG_MenuAction(mt_sound, menu_str(CMSG(F8000Hz)), NULL, OnSelectSoundRate, "%Cp %i", this, 2);
			 AG_MenuSetPollFn(mi, OnUpdateSoundRate, "%Cp %i", this, 2);
		mi = AG_MenuAction(mt_sound, menu_str(CMSG(F11025Hz)), NULL, OnSelectSoundRate, "%Cp %i", this, 3);
			 AG_MenuSetPollFn(mi, OnUpdateSoundRate, "%Cp %i", this, 3);
		mi = AG_MenuAction(mt_sound, menu_str(CMSG(F22050Hz)), NULL, OnSelectSoundRate, "%Cp %i", this, 4);
			 AG_MenuSetPollFn(mi, OnUpdateSoundRate, "%Cp %i", this, 4);
		mi = AG_MenuAction(mt_sound, menu_str(CMSG(F44100Hz)), NULL, OnSelectSoundRate, "%Cp %i", this, 5);
			 AG_MenuSetPollFn(mi, OnUpdateSoundRate, "%Cp %i", this, 5);
		mi = AG_MenuAction(mt_sound, menu_str(CMSG(F48000Hz)), NULL, OnSelectSoundRate, "%Cp %i", this, 6);
			 AG_MenuSetPollFn(mi, OnUpdateSoundRate, "%Cp %i", this, 6);
		mi = AG_MenuAction(mt_sound, menu_str(CMSG(F96000Hz)), NULL, OnSelectSoundRate, "%Cp %i", this, 7);
			 AG_MenuSetPollFn(mi, OnUpdateSoundRate, "%Cp %i", this, 7);
		AG_MenuSeparator(mt_sound);
		mi = AG_MenuAction(mt_sound, menu_str(CMSG(S50msec)), NULL, OnSelectSoundLatency, "%Cp %i", this, 0);
			 AG_MenuSetPollFn(mi, OnUpdateSoundLatency, "%Cp %i", this, 0);
		mi = AG_MenuAction(mt_sound, menu_str(CMSG(S75msec)), NULL, OnSelectSoundLatency, "%Cp %i", this, 1);
			 AG_MenuSetPollFn(mi, OnUpdateSoundLatency, "%Cp %i", this, 1);
		mi = AG_MenuAction(mt_sound, menu_str(CMSG(S100msec)), NULL, OnSelectSoundLatency, "%Cp %i", this, 2);
			 AG_MenuSetPollFn(mi, OnUpdateSoundLatency, "%Cp %i", this, 2);
		mi = AG_MenuAction(mt_sound, menu_str(CMSG(S200msec)), NULL, OnSelectSoundLatency, "%Cp %i", this, 3);
			 AG_MenuSetPollFn(mi, OnUpdateSoundLatency, "%Cp %i", this, 3);
		mi = AG_MenuAction(mt_sound, menu_str(CMSG(S300msec)), NULL, OnSelectSoundLatency, "%Cp %i", this, 4);
			 AG_MenuSetPollFn(mi, OnUpdateSoundLatency, "%Cp %i", this, 4);
		mi = AG_MenuAction(mt_sound, menu_str(CMSG(S400msec)), NULL, OnSelectSoundLatency, "%Cp %i", this, 5);
			 AG_MenuSetPollFn(mi, OnUpdateSoundLatency, "%Cp %i", this, 5);
	}
	mt_devices = AG_MenuNode(menu->root, CMSG(Devices), NULL);
	{
		for(int drv = 0; drv < MAX_PRINTER; drv++) {
			sprintf(name,menu_str(CMSG(LPTVDIGIT)), drv);
			mi = AG_MenuNode(mt_devices, name, NULL);
			{
				ms = AG_MenuAction(mi, menu_str(CMSG(Save_)), NULL, OnSelectSavePrinter, "%Cp %i", this, drv);
					 AG_MenuSetPollFn(ms, OnUpdateSavePrinter, "%Cp %i", this, drv);
				ms = AG_MenuAction(mi, menu_str(CMSG(Print_to_mpprinter)), NULL, OnSelectPrintPrinter, "%Cp %i", this, drv);
					 AG_MenuSetPollFn(ms, OnUpdatePrintPrinter, "%Cp %i", this, drv);
				ms = AG_MenuAction(mi, menu_str(CMSG(Clear)), NULL, OnSelectClearPrinter, "%Cp %i", this, drv);
//					 AG_MenuSetPollFn(ms, OnUpdateClearPrinter, "%Cp %i", this, drv);
				AG_MenuSeparator(mi);
				ms = AG_MenuAction(mi, menu_str(CMSG(Online)), NULL, OnSelectPrinterOnline, "%Cp %i", this, drv);
					 AG_MenuSetPollFn(ms, OnUpdatePrinterOnline, "%Cp %i", this, drv);
				AG_MenuSeparator(mi);
				ms = AG_MenuAction(mi, menu_str(CMSG(Send_to_mpprinter_concurrently)), NULL, OnSelectDirectPrinter, "%Cp %i", this, drv);
					 AG_MenuSetPollFn(ms, OnUpdateDirectPrinter, "%Cp %i", this, drv);
			}
		}
		AG_MenuSeparator(mt_devices);
		for(int drv = 0; drv < MAX_COMM; drv++) {
			sprintf(name,menu_str(CMSG(COMVDIGIT)), drv);
			mi = AG_MenuNode(mt_devices, name, NULL);
			{
				ms = AG_MenuAction(mi, menu_str(CMSG(Enable_Server)), NULL, OnSelectCommServer, "%Cp %i", this, drv);
					 AG_MenuSetPollFn(ms, OnUpdateCommServer, "%Cp %i", this, drv);
				ms = AG_MenuDynamicItem(mi, menu_str(CMSG(Connect)), NULL, OnUpdateCommConnectList, "%Cp %i", this, drv);
				AG_MenuSeparator(mi);
				ms = AG_MenuAction(mi, menu_str(CMSG(Comm_With_Byte_Data)), NULL, OnSelectCommThroughMode, "%Cp %i", this, drv);
					 AG_MenuSetPollFn(ms, OnUpdateCommThroughMode, "%Cp %i", this, drv);
				AG_MenuSeparator(mi);
				ms = AG_MenuNode(mi, menu_str(CMSG(Options_For_Telnet)), NULL);
				{
					mss = AG_MenuAction(ms, menu_str(CMSG(Binary_Mode)), NULL, OnSelectCommBinaryMode, "%Cp %i", this, drv);
						  AG_MenuSetPollFn(mss, OnUpdateCommBinaryMode, "%Cp %i", this, drv);
					AG_MenuSeparator(ms);
					mss = AG_MenuAction(ms, menu_str(CMSG(Send_WILL_ECHO)), NULL, OnSelectSendCommTelnetCommand, "%Cp %i %i", this, drv, 1);
				}
			}
		}
	}
	mt_options = AG_MenuNode(menu->root, CMSG(Options), NULL);
	{
		//                                **--------------- ------
		mi = AG_MenuAction(mt_options, menu_str(CMSG(Show_LED_Alt)), NULL, OnSelectLedBox, "%Cp", this);
			 AG_MenuSetPollFn(mi, OnUpdateLedBox, "%Cp", this);
#ifdef USE_OUTSIDE_LEDBOX
		mi = AG_MenuAction(mt_options, menu_str(CMSG(Inside_LED_Alt)), NULL, OnSelectInsideLed, "%Cp", this);
			 AG_MenuSetPollFn(mi, OnUpdateInsideLed, "%Cp", this);
#endif
		mi = AG_MenuAction(mt_options, menu_str(CMSG(Show_Message_Alt)), NULL, OnSelectMsgBoard, "%Cp", this);
			 AG_MenuSetPollFn(mi, OnUpdateMsgBoard, "%Cp", this);
#ifdef USE_PERFORMANCE_METER
		mi = AG_MenuAction(mt_options, menu_str(CMSG(Show_Performance_Meter)), NULL, OnSelectPMeter, "%Cp", this);
			 AG_MenuSetPollFn(mi, OnUpdatePMeter, "%Cp", this);
#endif
		AG_MenuSeparator(mt_options);
		mi = AG_MenuAction(mt_options, menu_str(CMSG(Use_Joypad_Key_Assigned_Alt)), NULL, OnSelectUseJoypad, "%Cp %i", this, 1);
			 AG_MenuSetPollFn(mi, OnUpdateUseJoypad, "%Cp %i", this, 1);
#ifdef USE_PIAJOYSTICK
		mi = AG_MenuAction(mt_options, menu_str(CMSG(Use_Joypad_PIA_Type_Alt)), NULL, OnSelectUseJoypad, "%Cp %i", this, 2);
			 AG_MenuSetPollFn(mi, OnUpdateUseJoypad, "%Cp %i", this, 2);
#endif
#ifdef USE_LIGHTPEN
		mi = AG_MenuAction(mt_options, menu_str(CMSG(Enable_Lightpen_Alt)), NULL, OnSelectEnableLightpen, "%Cp", this);
			 AG_MenuSetPollFn(mi, OnUpdateEnableLightpen, "%Cp", this);
#endif
#ifdef USE_MOUSE
		mi = AG_MenuAction(mt_options, menu_str(CMSG(Use_Mouse_Alt)), NULL, OnSelectEnableMouse, "%Cp", this);
			 AG_MenuSetPollFn(mi, OnUpdateEnableMouse, "%Cp", this);
#endif
		mi = AG_MenuAction(mt_options, menu_str(CMSG(Loosen_Key_Stroke_Game)), NULL, OnSelectLoosenKeyStroke, "%Cp", this);
			 AG_MenuSetPollFn(mi, OnUpdateLoosenKeyStroke, "%Cp", this);
		AG_MenuSeparator(mt_options);
		mi = AG_MenuAction(mt_options, menu_str(CMSG(Keybind_Alt)), NULL, OnSelectKeybindBox, "%Cp", this);
		mi = AG_MenuAction(mt_options, menu_str(CMSG(Virtual_Keyboard)), NULL, OnSelectVirtualKeyboard, "%Cp", this);
			 AG_MenuSetPollFn(mi, OnUpdateVirtualKeyboard, "%Cp", this);
#ifdef USE_DEBUGGER
		AG_MenuSeparator(mt_options);
		mi = AG_MenuAction(mt_options, menu_str(CMSG(Start_Debugger_Alt)), NULL, OnSelectOpenDebugger, "%Cp %i", this, 0);
			 AG_MenuSetPollFn(mi, OnUpdateOpenDebugger, "%Cp %i", this, 0);
		mi = AG_MenuAction(mt_options, menu_str(CMSG(Stop_Debugger)), NULL, OnSelectCloseDebugger, "%Cp", this);
#endif
		AG_MenuSeparator(mt_options);
		mi = AG_MenuAction(mt_options, menu_str(CMSG(Configure_Alt)), NULL, OnSelectConfigureBox, "%Cp", this);
	}
	mt_help = AG_MenuNode(menu->root, CMSG(Help), NULL);
	{
		mi = AG_MenuAction(mt_help, menu_str(CMSG(About_)), NULL, OnSelectAbout, "%Cp", this);
	}
}

/**
 * user event
 */
void GUI::ProcessUserEvent(int id)
{
	switch(id) {
//	case EVT_OPEN_LOAD_RECKEY_DIALOG:
//		PlayRecKeyBox(true);
//		break;
	case EVT_OPEN_SAVE_RECKEY_DIALOG:
		RecordRecKeyBox(true);
		break;
	}
}

// create keybind dialog
bool GUI::ShowKeybindDialog(void)
{
	ShowMenu();
	keybindbox->Create();
	return true;
}
// create config dialog
bool GUI::ShowConfigureDialog(void)
{
	ShowMenu();
	configbox->Create();
	return true;
}
// load State
bool GUI::ShowLoadStateDialog(void)
{
	ShowMenu();
	filebox->CreateLoad(AG_FILE_DLG::STATUS, CMSG(Load_Status_Data)
		, "*.l3r", config.initial_state_path, NULL);
	return true;
}
// save State
bool GUI::ShowSaveStateDialog(bool cont)
{
	ShowMenu();
	filebox->CreateSave(AG_FILE_DLG::STATUS, CMSG(Save_Status_Data)
		, "*.l3r", config.initial_state_path, NULL);
	return true;
}
// save State and RecordKey
bool GUI::ShowRecordRecKeyDialog(void)
{
	return true;
}
bool GUI::ShowRecordStateAndRecKeyDialog(void)
{
	ShowMenu();
	filebox->CreateSave(AG_FILE_DLG::STATUS_AND_RECKEY, CMSG(Save_Status_Data)
		, "*.l3r", config.initial_state_path, NULL);
	return true;
}
// Play RecordKey
bool GUI::ShowPlayRecKeyDialog(void)
{
	ShowMenu();
	filebox->CreateLoad(AG_FILE_DLG::RECKEY, CMSG(Play_Recorded_Keys)
		, "*.l3k", config.initial_state_path, NULL);
	return true;
}
// Record RecordKey
void GUI::RecordRecKeyBox(bool with_status)
{
	ShowMenu();
	filebox->CreateSave(with_status ? AG_FILE_DLG::RECKEY_AFTER_STATUS : AG_FILE_DLG::RECKEY, CMSG(Record_Input_Keys)
		, "*.l3k", config.initial_state_path, NULL);
}
// show Data Recorder Load Dialog
bool GUI::ShowLoadDataRecDialog(void)
{
	ShowMenu();
	filebox->CreateLoad(AG_FILE_DLG::DATAREC, CMSG(Play_Data_Recorder_Tape)
		, "*.l3,*.l3b,*.l3c,*.wav,*.t9x", config.initial_datarec_path, NULL);
	return true;
}
// show Data Recorder Save Dialog
bool GUI::ShowSaveDataRecDialog(void)
{
	ShowMenu();
	filebox->CreateSave(AG_FILE_DLG::DATAREC, CMSG(Record_Data_Recorder_Tape)
		, "*.l3,*.l3b,*.l3c,*.wav,*.t9x", config.initial_datarec_path, NULL);
	return true;
}

/*
 * Event Handler (static)
 */
#ifdef _MBS1
// select System Mode
void GUI::OnSelectSystemMode(AG_Event *event)
{
	GUI *gui = (GUI *)AG_PTR(1);
	int num = AG_INT(2);

	gui->ChangeSystemMode(num);
}
// update System Mode
void GUI::OnUpdateSystemMode(AG_Event *event)
{
	AG_MenuItem *mi = (AG_MenuItem *)AG_SENDER();
	GUI *gui = (GUI *)AG_PTR(1);
	int num = AG_INT(2);

	AG_GUI_MENU_CHECK(mi, gui->GetSystemMode() == num);
}
#endif
// select FDD Type
void GUI::OnSelectFDDType(AG_Event *event)
{
	GUI *gui = (GUI *)AG_PTR(1);
	int num = AG_INT(2);

	gui->ChangeFddType(num);
}
// update FDD Type
void GUI::OnUpdateFDDType(AG_Event *event)
{
	AG_MenuItem *mi = (AG_MenuItem *)AG_SENDER();
	GUI *gui = (GUI *)AG_PTR(1);
	int num = AG_INT(2);

	AG_GUI_MENU_CHECK(mi, gui->NextFddType() == num);
}
// select sync irq
void GUI::OnSelectSyncIRQ(AG_Event *event)
{
	GUI *gui = (GUI *)AG_PTR(1);
	gui->PostEtToggleSyncIRQ();
}
// update sync irq
void GUI::OnUpdateSyncIRQ(AG_Event *event)
{
	AG_MenuItem *mi = (AG_MenuItem *)AG_SENDER();
	GUI *gui = (GUI *)AG_PTR(1);

	AG_GUI_MENU_CHECK(mi, gui->NowSyncIRQ());
}
#ifdef _MBS1
// select memory no wait
void GUI::OnSelectMemNoWait(AG_Event *event)
{
	GUI *gui = (GUI *)AG_PTR(1);
	gui->ToggleMemNoWait();
}
// update memory no wait
void GUI::OnUpdateMemNoWait(AG_Event *event)
{
	AG_MenuItem *mi = (AG_MenuItem *)AG_SENDER();
	GUI *gui = (GUI *)AG_PTR(1);

	AG_GUI_MENU_CHECK(mi, gui->NowMemNoWait());
}
#endif

// Show Status Data Load Dialog
void GUI::OnSelectLoadState(AG_Event *event)
{
	GUI *gui = (GUI *)AG_PTR(1);
//	char *ext = AG_STRING(2);

	gui->ShowLoadStateDialog();
}
// Show Status Data Save Dialog
void GUI::OnSelectSaveState(AG_Event *event)
{
	GUI *gui = (GUI *)AG_PTR(1);
//	char *ext = AG_STRING(2);

	gui->ShowSaveStateDialog(true);
}
// select Recent file
void GUI::OnSelectRecentState(AG_Event *event)
{
	GUI *gui = (GUI *)AG_PTR(1);
	int num = AG_INT(3);

	gui->PostEtLoadRecentStatusMessage(num);
}
// update Recent Data Rec
void GUI::OnUpdateRecentState(AG_Event *event)
{
	AG_MenuItem *mi = (AG_MenuItem *)AG_SENDER();
	GUI *gui = (GUI *)AG_PTR(1);

	gui->update_recent_list(mi, config.recent_state_path, 0, OnSelectRecentState);
}

// Show Play RecordKey Dialog
void GUI::OnSelectPlayRecKey(AG_Event *event)
{
	GUI *gui = (GUI *)AG_PTR(1);
//	char *ext = AG_STRING(2);

	gui->ShowPlayRecKeyDialog();
}
// update Play RecordKey
void GUI::OnUpdatePlayRecKey(AG_Event *event)
{
	AG_MenuItem *mi = (AG_MenuItem *)AG_SENDER();
	GUI *gui = (GUI *)AG_PTR(1);

	AG_GUI_MENU_CHECK(mi, gui->NowPlayingRecKey());
}
// stop Play RecordKey
void GUI::OnSelectStopPlayRecKey(AG_Event *event)
{
	GUI *gui = (GUI *)AG_PTR(1);

	gui->StopPlayRecKey();
}

// Show Record RecordKey Dialog
void GUI::OnSelectRecordRecKey(AG_Event *event)
{
	GUI *gui = (GUI *)AG_PTR(1);
//	char *ext = AG_STRING(2);

	gui->ShowRecordStateAndRecKeyDialog();
}
// update Record RecordKey
void GUI::OnUpdateRecordRecKey(AG_Event *event)
{
	AG_MenuItem *mi = (AG_MenuItem *)AG_SENDER();
	GUI *gui = (GUI *)AG_PTR(1);

	AG_GUI_MENU_CHECK(mi, gui->NowRecordingRecKey());
}
// stop Play RecordKey
void GUI::OnSelectStopRecordRecKey(AG_Event *event)
{
	GUI *gui = (GUI *)AG_PTR(1);

	gui->StopRecordRecKey();
}

// update Rewind Data Recorder file
void GUI::OnSelectRewindDataRec(AG_Event *event)
{
	GUI *gui = (GUI *)AG_PTR(1);

	gui->PostEtRewindDataRecMessage();
}
// update Fast-Forward Data Recorder file
void GUI::OnSelectFastForwardDataRec(AG_Event *event)
{
	GUI *gui = (GUI *)AG_PTR(1);

	gui->PostEtFastForwardDataRecMessage();
}
// update Stop Data Recorder file
void GUI::OnSelectStopDataRec(AG_Event *event)
{
	GUI *gui = (GUI *)AG_PTR(1);

	gui->PostEtStopDataRecMessage();
}
// update Realmode Data Recorder
void GUI::OnSelectRealModeDataRec(AG_Event *event)
{
	GUI *gui = (GUI *)AG_PTR(1);

	gui->PostEtToggleRealModeDataRecMessage();
}
// update Realmode Data Recorder
void GUI::OnUpdateRealModeDataRec(AG_Event *event)
{
	GUI *gui = (GUI *)AG_PTR(1);
	AG_MenuItem *mi = (AG_MenuItem *)AG_SENDER();

	AG_GUI_MENU_CHECK(mi, gui->NowRealModeDataRec());
}

// Show Floppy Disk Change Side
void GUI::OnSelectChangeSideFloppyDisk(AG_Event *event)
{
	GUI *gui = (GUI *)AG_PTR(1);
	int   drv = AG_INT(2);

	gui->PostEtChangeSideFloppyDisk(drv);
}
// update Floppy Disk Change Side
void GUI::OnUpdateChangeSideFloppyDisk(AG_Event *event)
{
	AG_MenuItem *mi = (AG_MenuItem *)AG_SENDER();
	GUI *gui = (GUI *)AG_PTR(1);
	int   drv = AG_INT(2);

	int type = gui->GetFddType();
	int side = gui->GetSideFloppyDisk(drv);

	if ((mi) && (mi)->text) {
		if (side == 1 && type == 1) {
			char *p = strchr((mi)->text, 'B');
			// ..Change side to A
			if (p != NULL) *p = 'A';
		} else {
			char *p = strchr((mi)->text, 'A');
			// ..Change side to B
			if (p != NULL) *p = 'B';
		}
	}
	AG_GUI_MENU_ENABLE(mi, side >= 0 && type == 1);
}

// Show Floppy Disk Write Protect
void GUI::OnSelectWriteProtectFloppyDisk(AG_Event *event)
{
	GUI *gui = (GUI *)AG_PTR(1);
	int   drv = AG_INT(2);

	gui->PostEtToggleWriteProtectFloppyDisk(drv);
}
// update Floppy Disk Write Protect
void GUI::OnUpdateWriteProtectFloppyDisk(AG_Event *event)
{
	AG_MenuItem *mi = (AG_MenuItem *)AG_SENDER();
	GUI *gui = (GUI *)AG_PTR(1);
	int   drv = AG_INT(2);

	AG_GUI_MENU_ENABLE(mi, gui->InsertedFloppyDisk(drv));
	AG_GUI_MENU_CHECK(mi, gui->WriteProtectedFloppyDisk(drv));
}

// Change Stretch screen
void GUI::OnSelectStretchScreen(AG_Event *event)
{
	GUI *gui = (GUI *)AG_PTR(1);

	gui->ChangeStretchScreen(1);
}
// update StretchScreen
void GUI::OnUpdateStretchScreen(AG_Event *event)
{
	GUI *gui = (GUI *)AG_PTR(1);
	AG_MenuItem *mi = (AG_MenuItem *)AG_SENDER();

	AG_GUI_MENU_CHECK(mi, gui->GetStretchScreen() == 1);
}

// Change Cutout screen
void GUI::OnSelectCutoutScreen(AG_Event *event)
{
	GUI *gui = (GUI *)AG_PTR(1);

	gui->ChangeStretchScreen(2);
}
// update CutoutScreen
void GUI::OnUpdateCutoutScreen(AG_Event *event)
{
	GUI *gui = (GUI *)AG_PTR(1);
	AG_MenuItem *mi = (AG_MenuItem *)AG_SENDER();

	AG_GUI_MENU_CHECK(mi, gui->GetStretchScreen() == 2);
}

// Change After Image
void GUI::OnSelectAfterImage(AG_Event *event)
{
	GUI *gui = (GUI *)AG_PTR(1);
	int num = AG_INT(2);
	gui->PostEtChangeAfterImage(num);
}
// update After Image
void GUI::OnUpdateAfterImage(AG_Event *event)
{
	AG_MenuItem *mi = (AG_MenuItem *)AG_SENDER();
	GUI *gui = (GUI *)AG_PTR(1);
	int num = AG_INT(2);

	AG_GUI_MENU_CHECK(mi, gui->GetAfterImageMode() == num);
}

#ifdef USE_KEEPIMAGE
// Change Keep Image
void GUI::OnSelectKeepImage(AG_Event *event)
{
	GUI *gui = (GUI *)AG_PTR(1);
	int num = AG_INT(2);
	gui->PostEtChangeKeepImage(num);
}
// update Keep Image
void GUI::OnUpdateKeepImage(AG_Event *event)
{
	AG_MenuItem *mi = (AG_MenuItem *)AG_SENDER();
	GUI *gui = (GUI *)AG_PTR(1);
	int num = AG_INT(2);

	AG_GUI_MENU_CHECK(mi, gui->GetKeepImageMode() == num);
}
#endif
#ifdef _MBS1
// Change RGB Type
void GUI::OnSelectRGBType(AG_Event *event)
{
	GUI *gui = (GUI *)AG_PTR(1);
	int num = AG_INT(2);
	gui->ChangeRGBType(num);
}
// update RGB Type
void GUI::OnUpdateRGBType(AG_Event *event)
{
	AG_MenuItem *mi = (AG_MenuItem *)AG_SENDER();
	GUI *gui = (GUI *)AG_PTR(1);
	int num = AG_INT(2);

	AG_GUI_MENU_CHECK(mi, gui->GetRGBTypeMode() == num);
}
#endif
#ifdef USE_OPENGL
// Change Use OpenGL
void GUI::OnSelectUseOpenGL(AG_Event *event)
{
	GUI *gui = (GUI *)AG_PTR(1);
	int num = AG_INT(2);
	gui->ChangeUseOpenGL(num);
}
// update Use OpenGL
void GUI::OnUpdateUseOpenGL(AG_Event *event)
{
	AG_MenuItem *mi = (AG_MenuItem *)AG_SENDER();
	GUI *gui = (GUI *)AG_PTR(1);
	int num = AG_INT(2);

	AG_GUI_MENU_CHECK(mi, gui->GetOpenGLMode() == num);
}
// Change OpenGL Filter
void GUI::OnSelectOpenGLFilter(AG_Event *event)
{
	GUI *gui = (GUI *)AG_PTR(1);
	int num = AG_INT(2);

	gui->ChangeOpenGLFilter(num);
}
// update OpenGL Filter
void GUI::OnUpdateOpenGLFilter(AG_Event *event)
{
	AG_MenuItem *mi = (AG_MenuItem *)AG_SENDER();
	GUI *gui = (GUI *)AG_PTR(1);
	int num = AG_INT(2);

	AG_GUI_MENU_CHECK(mi, gui->GetOpenGLFilter() == num);
}
#endif

// Change Ledbox
void GUI::OnSelectLedBox(AG_Event *event)
{
	GUI *gui = (GUI *)AG_PTR(1);
	gui->ToggleShowLedBox();
}
// update Ledbox
void GUI::OnUpdateLedBox(AG_Event *event)
{
	AG_MenuItem *mi = (AG_MenuItem *)AG_SENDER();
	GUI *gui = (GUI *)AG_PTR(1);

	AG_GUI_MENU_CHECK(mi, gui->IsShownLedBox());
}
// Change Inside Ledbox
void GUI::OnSelectInsideLed(AG_Event *event)
{
	GUI *gui = (GUI *)AG_PTR(1);
	gui->ToggleInsideLedBox();
}
// update Inside Ledbox
void GUI::OnUpdateInsideLed(AG_Event *event)
{
	AG_MenuItem *mi = (AG_MenuItem *)AG_SENDER();
	GUI *gui = (GUI *)AG_PTR(1);

	AG_GUI_MENU_CHECK(mi, gui->IsInsidedLedBox());
}
// Change MsgBoard
void GUI::OnSelectMsgBoard(AG_Event *event)
{
	GUI *gui = (GUI *)AG_PTR(1);
	gui->ToggleMessageBoard();
}
// update MsgBoard
void GUI::OnUpdateMsgBoard(AG_Event *event)
{
	AG_MenuItem *mi = (AG_MenuItem *)AG_SENDER();
	GUI *gui = (GUI *)AG_PTR(1);

	AG_GUI_MENU_CHECK(mi, gui->IsShownMessageBoard());
}

#ifdef USE_PERFORMANCE_METER
void GUI::OnSelectPMeter(AG_Event *event)
{
	GUI *gui = (GUI *)AG_PTR(1);
	gui->TogglePMeter();
}
void GUI::OnUpdatePMeter(AG_Event *event)
{
	AG_MenuItem *mi = (AG_MenuItem *)AG_SENDER();
	GUI *gui = (GUI *)AG_PTR(1);

	AG_GUI_MENU_CHECK(mi, gui->IsShownPMeter());
}
#endif

// Change UseJoypad
void GUI::OnSelectUseJoypad(AG_Event *event)
{
	GUI *gui = (GUI *)AG_PTR(1);
	int num = AG_INT(2);

	gui->ChangeUseJoypad(num);
}
// update UseJoypad
void GUI::OnUpdateUseJoypad(AG_Event *event)
{
	AG_MenuItem *mi = (AG_MenuItem *)AG_SENDER();
	GUI *gui = (GUI *)AG_PTR(1);
	int num = AG_INT(2);

	AG_GUI_MENU_CHECK(mi, gui->IsEnableJoypad(num));
}

#ifdef USE_LIGHTPEN
// Change EnableLightpen
void GUI::OnSelectEnableLightpen(AG_Event *event)
{
	GUI *gui = (GUI *)AG_PTR(1);
	gui->ToggleEnableLightpen();
}
// update EnableLightpen
void GUI::OnUpdateEnableLightpen(AG_Event *event)
{
	AG_MenuItem *mi = (AG_MenuItem *)AG_SENDER();
	GUI *gui = (GUI *)AG_PTR(1);

	AG_GUI_MENU_CHECK(mi, gui->IsEnableLightpen());
}
#endif
#ifdef USE_MOUSE
// Change EnableMouse
void GUI::OnSelectEnableMouse(AG_Event *event)
{
	GUI *gui = (GUI *)AG_PTR(1);
	gui->ToggleUseMouse();
}
// update EnableMouse
void GUI::OnUpdateEnableMouse(AG_Event *event)
{
	AG_MenuItem *mi = (AG_MenuItem *)AG_SENDER();
	GUI *gui = (GUI *)AG_PTR(1);

	AG_GUI_MENU_CHECK(mi, gui->IsEnableMouse());
}
#endif
// Change Loosen Key Stroke
void GUI::OnSelectLoosenKeyStroke(AG_Event *event)
{
	GUI *gui = (GUI *)AG_PTR(1);
	gui->ToggleLoosenKeyStroke();
}
// update Loosen Key Stroke
void GUI::OnUpdateLoosenKeyStroke(AG_Event *event)
{
	AG_MenuItem *mi = (AG_MenuItem *)AG_SENDER();
	GUI *gui = (GUI *)AG_PTR(1);

	AG_GUI_MENU_CHECK(mi, gui->IsLoosenKeyStroke());
}

// create keybind dialog
void GUI::OnSelectKeybindBox(AG_Event *event)
{
	GUI *gui = (GUI *)AG_PTR(1);
	gui->ShowKeybindDialog();
}

// create config dialog
void GUI::OnSelectConfigureBox(AG_Event *event)
{
	GUI *gui = (GUI *)AG_PTR(1);
	gui->ShowConfigureDialog();
}

// Change Print Printer
void GUI::OnSelectPrintPrinter(AG_Event *event)
{
	GUI *gui = (GUI *)AG_PTR(1);
	int drv = AG_INT(2);

	gui->PostEtPrintPrinterMessage(drv);
}
// update Print Printer
void GUI::OnUpdatePrintPrinter(AG_Event *event)
{
	AG_MenuItem *mi = (AG_MenuItem *)AG_SENDER();
	GUI *gui = (GUI *)AG_PTR(1);
	int drv = AG_INT(2);

	int size = gui->GetPrinterBufferSize(drv);

	AG_GUI_MENU_ENABLE(mi, size > 0);
}

// Change Direct Printer
void GUI::OnSelectDirectPrinter(AG_Event *event)
{
	GUI *gui = (GUI *)AG_PTR(1);
	int drv = AG_INT(2);

	gui->PostEtEnablePrinterDirectMessage(drv);
}
// update Direct Printer
void GUI::OnUpdateDirectPrinter(AG_Event *event)
{
	AG_MenuItem *mi = (AG_MenuItem *)AG_SENDER();
	GUI *gui = (GUI *)AG_PTR(1);
	int drv = AG_INT(2);

	AG_GUI_MENU_CHECK(mi, gui->IsEnablePrinterDirect(drv));
}

// Change Printer Online
void GUI::OnSelectPrinterOnline(AG_Event *event)
{
	GUI *gui = (GUI *)AG_PTR(1);
	int drv = AG_INT(2);

	gui->PostEtTogglePrinterOnlineMessage(drv);
}
// update Printer Online
void GUI::OnUpdatePrinterOnline(AG_Event *event)
{
	AG_MenuItem *mi = (AG_MenuItem *)AG_SENDER();
	GUI *gui = (GUI *)AG_PTR(1);
	int drv = AG_INT(2);

	AG_GUI_MENU_CHECK(mi, gui->IsOnlinePrinter(drv));
}

// Change Comm Server
void GUI::OnSelectCommServer(AG_Event *event)
{
	GUI *gui = (GUI *)AG_PTR(1);
	int drv = AG_INT(2);

//	gui->PostEtEnableCommServerMessage(drv);
	gui->ToggleEnableCommServer(drv);
}
// update Comm Server
void GUI::OnUpdateCommServer(AG_Event *event)
{
	AG_MenuItem *mi = (AG_MenuItem *)AG_SENDER();
	GUI *gui = (GUI *)AG_PTR(1);
	int drv = AG_INT(2);

	AG_GUI_MENU_CHECK(mi, gui->IsEnableCommServer(drv));
}

// update connection list
void GUI::OnUpdateCommConnectList(AG_Event *event)
{
	AG_MenuItem *mi = (AG_MenuItem *)AG_SENDER();
	GUI *gui = (GUI *)AG_PTR(1);
	int  drv = AG_INT(2);

	gui->update_comm_connect_list(mi, drv);
}

void GUI::update_comm_connect_list(AG_MenuItem *mi, int drv)
{
	char name[128];
	int uarts = EnumUarts();

	AG_MenuItemFreeChildren(mi);

	AG_MenuItem *ms = AG_MenuAction(mi, menu_str(CMSG(Ethernet)), NULL, OnSelectCommConnect, "%Cp %i %i", this, drv, 0);
					  AG_MenuSetPollFn(ms, OnUpdateCommConnect, "%Cp %i %i", this, drv, 0);
	if (uarts > 0) {
		AG_MenuSeparator(mi);
	}
	for(int i = 0; i < uarts; i++) {
		GetUartDescription(i, name, sizeof(name)); 
		ms = AG_MenuAction(mi, menu_str(name), NULL, OnSelectCommConnect, "%Cp %i %i", this, drv, i + 1);
			 AG_MenuSetPollFn(ms, OnUpdateCommConnect, "%Cp %i %i", this, drv, i + 1);
	}
}

// Change Comm connect
void GUI::OnSelectCommConnect(AG_Event *event)
{
	GUI *gui = (GUI *)AG_PTR(1);
	int drv = AG_INT(2);
	int num = AG_INT(3);

//	gui->PostEtConnectCommMessage(drv);
	gui->ToggleConnectComm(drv, num);
}
// update Comm connect
void GUI::OnUpdateCommConnect(AG_Event *event)
{
	AG_MenuItem *mi = (AG_MenuItem *)AG_SENDER();
	GUI *gui = (GUI *)AG_PTR(1);
	int drv = AG_INT(2);
	int num = AG_INT(3);

	AG_GUI_MENU_CHECK(mi, gui->NowConnectingComm(drv, num));
}

// Change Comm through
void GUI::OnSelectCommThroughMode(AG_Event *event)
{
	GUI *gui = (GUI *)AG_PTR(1);
	int drv = AG_INT(2);

	gui->ToggleCommThroughMode(drv);
}
// update Comm through
void GUI::OnUpdateCommThroughMode(AG_Event *event)
{
	AG_MenuItem *mi = (AG_MenuItem *)AG_SENDER();
	GUI *gui = (GUI *)AG_PTR(1);
	int drv = AG_INT(2);

	AG_GUI_MENU_CHECK(mi, gui->NowCommThroughMode(drv));
}

// Change Comm binary mode
void GUI::OnSelectCommBinaryMode(AG_Event *event)
{
	GUI *gui = (GUI *)AG_PTR(1);
	int drv = AG_INT(2);

	gui->ToggleCommBinaryMode(drv);
}
// update Comm binary mode
void GUI::OnUpdateCommBinaryMode(AG_Event *event)
{
	AG_MenuItem *mi = (AG_MenuItem *)AG_SENDER();
	GUI *gui = (GUI *)AG_PTR(1);
	int drv = AG_INT(2);

	AG_GUI_MENU_CHECK(mi, gui->NowCommBinaryMode(drv));
}

// Change Comm Send Telnet Command
void GUI::OnSelectSendCommTelnetCommand(AG_Event *event)
{
	GUI *gui = (GUI *)AG_PTR(1);
	int drv = AG_INT(2);
	int num = AG_INT(3);

	gui->SendCommTelnetCommand(drv, num);
}
