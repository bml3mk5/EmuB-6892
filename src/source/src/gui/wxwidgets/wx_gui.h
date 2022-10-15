/** @file wx_gui.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.09.01

	@brief [ wx_gui ]
*/

#ifndef WX_GUI_H
#define WX_GUI_H

#include "../gui_base.h"
#include "wx_dlg.h"
//#include <wx/wx.h>
//#include <wx/dialog.h>
#include <wx/dnd.h>

#define FDD_NUMS	MAX_DRIVE

class CRecentPathList;
class MyApp;
class MyFrame;
class MyPanel;
class MyGLCanvas;
class MyConnection;

/**
 * @brief GUI class
 */
class GUI : public GUI_BASE {
private:
	MyFrame *frame;
#if defined(USE_ONIDLE) || defined(USE_ONTIMER)
	wxMutex *mux_need_update;
	wxCondition *cond_need_update;
#endif

public:
	GUI(int argc, char **argv, EMU *new_emu);
	~GUI();

	bool NeedUpdateScreen();
	void UpdatedScreen();
	void PreProcessEvent();
	void PostCommandMessage(int id, void *data1 = NULL, void *data2 = NULL);

	void Exit(void);

#ifdef USE_DATAREC
	bool ShowLoadDataRecDialog(void);
	bool ShowSaveDataRecDialog(void);
#endif

#ifdef USE_FD1
	bool ShowOpenFloppyDiskDialog(int drv);
	int  ShowSelectFloppyDriveDialog(int drv);
	bool ShowOpenBlankFloppyDiskDialog(int drv, uint8_t type);
#endif

	bool ShowLoadStateDialog(void);
	bool ShowSaveStateDialog(bool cont);

	bool ShowOpenAutoKeyDialog(void);

	bool ShowPlayRecKeyDialog(void);
	bool ShowRecordRecKeyDialog(void);

	bool ShowRecordStateAndRecKeyDialog(void);

	bool ShowSavePrinterDialog(int drv);

	bool ShowRecordVideoDialog(int fps_num);
	bool ShowRecordAudioDialog(void);
	bool ShowRecordVideoAndAudioDialog(int fps_num);

	bool ShowVolumeDialog(void);

	bool ShowKeybindDialog(void);
	bool ShowConfigureDialog(void);

	bool ShowAboutDialog(void);

	bool ShowVirtualKeyboard(void);

	void GetLibVersionString(_TCHAR *str, int max_len = _MAX_PATH, const _TCHAR *sep_str = _T("\n"));

	void CreateLedBoxSub();

	bool StartAutoKey(void);

	/// @name access wxFrame
	//@{
	void CreateMyFrame(int x, int y, int w, int h);
	void CloseMyFrame();
	MyFrame *GetMyFrame();
#if defined(USE_ONIDLE) || defined(USE_ONTIMER)
	void SetMutex(wxMutex *mutex, wxCondition *condition);
#endif
	//@}
};

/**
 * @brief Main window frame
 */
class MyFrame : public wxFrame {
    DECLARE_CLASS(MyFrame)
    DECLARE_EVENT_TABLE()

private:
	EMU *emu;
	GUI_BASE *gui;

	MyApp *app;
	MyPanel *panel;
	MyGLCanvas *glcanvas;

	wxMenuBar *menuBar;	
	MyMenu *menuControl;
	MyMenu *menuStateRecent;
	MyMenu *menuTape;
	MyMenu *menuTapeRecent;
	MyMenu *menuFdd[FDD_NUMS];
	MyMenu *menuFddMulti[FDD_NUMS];
	MyMenu *menuFddRecent[FDD_NUMS];
	MyMenu *menuScreen;
	MyMenu *menuSound;
    MyMenu *menuDevice;
    MyMenu *menuCommConnect[MAX_COMM];
	MyMenu *menuOptions;

//	int opened_datarec_type;
	bool enable_opengl;

	int fskip_remain;
	int rec_fps_no;

//	int *key_mod;

	bool now_showmenu;
	int  now_updating_menu;
	bool now_expanding_menu;

	// 
	void CreateMenu(wxMenuBar *mb);


	// event handler
	void OnSelect(wxCommandEvent &);
	void OnClose(wxCloseEvent &);
	void OnExit(wxCommandEvent &);
    void OnHelpAbout(wxCommandEvent &);
	void OnMoveEnd(wxMoveEvent&);

	void OnUpdateMenu(wxMenuEvent &);
	void OnCloseMenu(wxMenuEvent &);

	void OnUpdateScreen(wxCommandEvent &);
	void OnUpdateTitle(wxCommandEvent &);

	void UpdateMenuRecentFiles(MyMenu *menu, int id, CRecentPathList &recent_path, bool remake);
	void UpdateMenuMultiVolume(MyMenu *menu, int drv, int id, bool remake);

public:
    MyFrame(MyApp *parent, EMU *new_emu, GUI_BASE *new_gui, int x, int y, int w, int h);
	~MyFrame();

	void UpdateMenu(int flags = -1);
	void UpdateMenu(wxMenu *menu);
	void UpdateMenuControl(bool remake);
	void UpdateMenuTape(bool remake);
	void UpdateMenuFdd(int drv, bool remake);
	void UpdateMenuScreen();
	void UpdateMenuSound();
	void UpdateMenuPrinter(int dev);
	void UpdateMenuComm(int dev, bool remake);
	void UpdateMenuCommConnect(int dev, MyMenu *menu);
	void UpdateMenuOptions();

	void DecreaseMenuCount();

#ifdef USE_ONTIMER
	void AdjustTimer();
#endif
	void UpdateScreen();
	void UpdateTitle(const void *result);
	void ChangePanel(int is_opengl);

	void Exit();
	void Close();

	void OpenDroppedFile(wxString &path);
	void OpenRecentFile();

	//
	void PostUserEvent(int id, void *data1);

	// override
	void SetClientSize(int width, int height);
	void SetFocus();

	void GoUnfullscreen(int width, int height);
	void GoFullscreen(int width, int height);

//	void TranslateKeyCode(int &code, wxChar &unicode, wxUint32_t &rawcode);
//	bool ExecuteGlobalKeys(int code);

	void OnCharHook(wxKeyEvent &);
	void OnKeyDown(wxKeyEvent &);
	void OnKeyUp(wxKeyEvent &);

	void OnMouseMotion(wxMouseEvent &);
	void OnMouseDown(wxMouseEvent &);

    MyApp   *GetApp()   { return app;	}
    MyPanel *GetPanel() { return panel;	}
};

/**
 * @brief Process dropped files
 */
class MyFileDropTarget : public wxFileDropTarget
{
private:
    MyFrame *frame;
public:
    MyFileDropTarget(MyFrame *parent);
    bool OnDropFiles(wxCoord x, wxCoord y ,const wxArrayString &filenames);
};


#endif /* WX_GUI_H */
