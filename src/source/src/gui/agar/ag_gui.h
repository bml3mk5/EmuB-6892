/** @file ag_gui.h

	Skelton for retropc emulator
	SDL + Agar edition

	@author Sasaji
	@date   2012.03.01

	@brief [ ag_gui ]
*/

#ifndef AG_GUI_H
#define AG_GUI_H

#include "ag_gui_base.h"

namespace GUI_AGAR
{

#define ID_CONFIGBOX	0
#define ID_KEYBINDBOX	1
#define ID_JOYSETBOX	2

#define EVT_OPEN_LOAD_RECKEY_DIALOG	1
#define EVT_OPEN_SAVE_RECKEY_DIALOG	2

class AG_CONFIG_DLG;
class AG_KEYBIND_DLG;
class AG_JOYSET_DLG;

}; /* namespace GUI_AGAR */

/**
	@brief gui vm dependency
*/
class GUI: public GUI_AGAR::AG_GUI_BASE {
public:
	GUI(int, char **, EMU *);
	virtual ~GUI();

	// dialog
	GUI_AGAR::AG_CONFIG_DLG *configbox;
	GUI_AGAR::AG_KEYBIND_DLG *keybindbox;
	GUI_AGAR::AG_JOYSET_DLG *joysetbox;

	virtual int Init();

	GUI_AGAR::AG_DLG *GetDlgPtr(int);

	void ProcessUserEvent(int);

	virtual bool ShowLoadDataRecDialog(void);
	virtual bool ShowSaveDataRecDialog(void);

	virtual bool ShowLoadStateDialog(void);
	virtual bool ShowSaveStateDialog(bool);
	virtual bool ShowPlayRecKeyDialog(void);
	virtual bool ShowRecordRecKeyDialog(void);
	virtual bool ShowRecordStateAndRecKeyDialog(void);
	virtual bool ShowJoySettingDialog(void);
	virtual bool ShowKeybindDialog(void);
	virtual bool ShowConfigureDialog(void);

	/// implement on win_gui.cpp, cocoa_gui.mm
	virtual bool StartAutoKey(void);
	virtual bool ShowVirtualKeyboard(void);

	void RecordRecKeyBox(bool with_status);

private:
	void set_menu_item(AG_Menu *);

#ifdef _MBS1
	static void OnSelectSystemMode(AG_Event *);
	static void OnUpdateSystemMode(AG_Event *);
#endif
	static void OnSelectFDDType(AG_Event *);
	static void OnUpdateFDDType(AG_Event *);
	static void OnSelectSyncIRQ(AG_Event *);
	static void OnUpdateSyncIRQ(AG_Event *);
#ifdef _MBS1
	static void OnSelectMemNoWait(AG_Event *);
	static void OnUpdateMemNoWait(AG_Event *);
#endif
	static void OnSelectLoadState(AG_Event *);
	static void OnSelectSaveState(AG_Event *);
	static void OnSelectRecentState(AG_Event *);
	static void OnUpdateRecentState(AG_Event *);
	static void OnSelectPlayRecKey(AG_Event *);
	static void OnUpdatePlayRecKey(AG_Event *);
	static void OnSelectStopPlayRecKey(AG_Event *);
	static void OnSelectRecordRecKey(AG_Event *);
	static void OnUpdateRecordRecKey(AG_Event *);
	static void OnSelectStopRecordRecKey(AG_Event *);
	static void OnSelectRewindDataRec(AG_Event *);
	static void OnSelectFastForwardDataRec(AG_Event *);
	static void OnSelectStopDataRec(AG_Event *);
	static void OnSelectRealModeDataRec(AG_Event *);
	static void OnUpdateRealModeDataRec(AG_Event *);
	static void OnSelectChangeSideFloppyDisk(AG_Event *);
	static void OnUpdateChangeSideFloppyDisk(AG_Event *);
	static void OnSelectWriteProtectFloppyDisk(AG_Event *);
	static void OnUpdateWriteProtectFloppyDisk(AG_Event *);
	static void OnSelectStretchScreen(AG_Event *);
	static void OnUpdateStretchScreen(AG_Event *);
	static void OnSelectCutoutScreen(AG_Event *);
	static void OnUpdateCutoutScreen(AG_Event *);
	static void OnSelectAfterImage(AG_Event *);
	static void OnUpdateAfterImage(AG_Event *);
#ifdef USE_KEEPIMAGE
	static void OnSelectKeepImage(AG_Event *);
	static void OnUpdateKeepImage(AG_Event *);
#endif
#ifdef _MBS1
	static void OnSelectRGBType(AG_Event *);
	static void OnUpdateRGBType(AG_Event *);
#endif
#ifdef USE_OPENGL
	static void OnSelectUseOpenGL(AG_Event *);
	static void OnUpdateUseOpenGL(AG_Event *);
	static void OnSelectOpenGLFilter(AG_Event *);
	static void OnUpdateOpenGLFilter(AG_Event *);
#endif
	static void OnSelectLedBox(AG_Event *);
	static void OnUpdateLedBox(AG_Event *);
	static void OnSelectInsideLed(AG_Event *);
	static void OnUpdateInsideLed(AG_Event *);
	static void OnSelectMsgBoard(AG_Event *);
	static void OnUpdateMsgBoard(AG_Event *);
#ifdef USE_PERFORMANCE_METER
	static void OnSelectPMeter(AG_Event *);
	static void OnUpdatePMeter(AG_Event *);
#endif
	static void OnSelectUseJoypad(AG_Event *);
	static void OnUpdateUseJoypad(AG_Event *);
	static void OnSelectEnableKey2Joy(AG_Event *);
	static void OnUpdateEnableKey2Joy(AG_Event *);
#ifdef USE_LIGHTPEN
	static void OnSelectEnableLightpen(AG_Event *);
	static void OnUpdateEnableLightpen(AG_Event *);
#endif
#ifdef USE_MOUSE
	static void OnSelectEnableMouse(AG_Event *);
	static void OnUpdateEnableMouse(AG_Event *);
#endif
	static void OnSelectLoosenKeyStroke(AG_Event *);
	static void OnUpdateLoosenKeyStroke(AG_Event *);
	static void OnSelectJoypadSetting(AG_Event *);
	static void OnSelectKeybindBox(AG_Event *);
	static void OnSelectConfigureBox(AG_Event *);
	static void OnSelectPrintPrinter(AG_Event *);
	static void OnUpdatePrintPrinter(AG_Event *);
	static void OnSelectDirectPrinter(AG_Event *);
	static void OnUpdateDirectPrinter(AG_Event *);
	static void OnSelectPrinterOnline(AG_Event *);
	static void OnUpdatePrinterOnline(AG_Event *);
	static void OnSelectCommServer(AG_Event *);
	static void OnUpdateCommServer(AG_Event *);
	static void OnUpdateCommConnectList(AG_Event *);
	void        update_comm_connect_list(AG_MenuItem *mi, int drv);
	static void OnSelectCommConnect(AG_Event *);
	static void OnUpdateCommConnect(AG_Event *);
	static void OnSelectCommThroughMode(AG_Event *);
	static void OnUpdateCommThroughMode(AG_Event *);
	static void OnSelectCommBinaryMode(AG_Event *);
	static void OnUpdateCommBinaryMode(AG_Event *);
	static void OnSelectSendCommTelnetCommand(AG_Event *);
};

#endif /* AG_GUI_H */
