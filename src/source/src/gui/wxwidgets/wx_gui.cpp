/** @file wx_gui.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.09.01

	@brief [ wx_gui ]
*/

#include <wx/wx.h>
#include <wx/filename.h>
#include <wx/clipbrd.h>
#include "../../main.h"
#include "wx_gui.h"
#include "../../emu_osd.h"
#include "wx_file_dlg.h"
#include "wx_config_dlg.h"
#include "wx_keybind_dlg.h"
#include "wx_volume_dlg.h"
#include "wx_recvid_dlg.h"
#include "wx_recaud_dlg.h"
#include "wx_seldrv_dlg.h"
#include "wx_about_dlg.h"
#ifdef USE_LEDBOX
#include "wx_ledbox.h"
#endif
#ifdef USE_VKEYBOARD
#include "wx_vkeyboard.h"
#endif
#include "../../depend.h"
#include "../../utility.h"
#include "../../res/resource.h"
#if defined(_BML3MK5)
#include "../../res/common/bml3mk5.xpm"
#elif defined(_MBS1)
#include "../../res/common/mbs1.xpm"
#endif

GUI::GUI(int argc, char **argv, EMU *new_emu)
	: GUI_BASE(argc, argv, new_emu)
{
	frame = NULL;
#if defined(USE_ONIDLE) || defined(USE_ONTIMER)
	mux_need_update = NULL;
	cond_need_update = NULL;
#endif
}

GUI::~GUI()
{
	// Myframe is deleted by itself, so no delete on this
}

/// @attention invoke from emu thread
bool GUI::NeedUpdateScreen()
{
	if (need_update_screen > 0) {
#if defined(USE_ONIDLE) || defined(USE_ONTIMER)
		need_update_screen = -1;
#endif
//		logging->out_logf(LOG_DEBUG, _T("NeedUpdateScreen: nus:%d"), need_update_screen);
		// post refresh meesage to frame event loop
#if defined(__WXGTK__)
		PostCommandMessage(ID_UPDATE_SCREEN);
#else
		frame->UpdateScreen();
#endif
#ifdef USE_ONIDLE
		// send signal to main loop
		cond_need_update->Signal();
#endif
		return true;
	} else {
		return false;
	}
}

/// @attention invoke from main thread
void GUI::UpdatedScreen()
{
#if defined(USE_ONIDLE) || defined(USE_ONTIMER)
	need_update_screen = 6;
//	logging->out_logf(LOG_DEBUG, _T("UpdatedScreen: nus:%d"), need_update_screen);
#endif
#ifdef USE_ONTIMER
	frame->AdjustTimer();
#endif
}

void GUI::PreProcessEvent()
{
#if defined(USE_ONIDLE) || defined(USE_ONTIMER)
	if (need_update_screen == 0) {
//		logging->out_logf(LOG_DEBUG, _T("PreProcessEvent: nus:%d"), need_update_screen);
		if (!refreshing) {
			refreshing = true;
			frame->UpdateScreen();
		}
	}
	frame->DecreaseMenuCount();
#endif
}

void GUI::PostCommandMessage(int id, void *data1, void *data2)
{
	frame->PostUserEvent(id, data1);
}

void GUI::Exit(void)
{
	// disable updating screen
	need_update_screen = -1;
	// close virtual keyboard
	if (vkeyboard) {
		vkeyboard->Close();
		delete vkeyboard;
		vkeyboard = NULL;
	}
	// request to close window
	frame->Close();
}

#ifdef USE_DATAREC
bool GUI::ShowLoadDataRecDialog(void)
{
	const CMsg::Id filter[] = {
		CMsg::Supported_Files_l3_l3b_l3c_wav_t9x,
		CMsg::All_Files_,
		CMsg::End
	};

	MyFileDialog *dlg = new MyFileDialog(
		CMsg::Play_Data_Recorder_Tape,
		wxString(config.initial_datarec_path),
		wxEmptyString,
		filter,
		wxFD_OPEN | wxFD_FILE_MUST_EXIST);

	SystemPause(true);

	int rc = dlg->ShowModal();
	wxString path = dlg->GetPath();
	delete dlg;

	if (rc == wxID_OK) {
		PostEtLoadDataRecMessage(path);
	} else {
		SystemPause(false);
	}
	return (rc == wxID_OK);
}
bool GUI::ShowSaveDataRecDialog(void)
{
	const CMsg::Id filter[] = {
		CMsg::L3_File_l3,
		CMsg::L3B_File_l3b,
		CMsg::L3C_File_l3c,
		CMsg::Wave_File_wav,
		CMsg::T9X_File_t9x,
		CMsg::All_Files_,
		CMsg::End
	};

	MyFileDialog *dlg = new MyFileDialog(
		CMsg::Record_Data_Recorder_Tape,
		wxString(config.initial_datarec_path),
		wxEmptyString,
		filter,
		wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

	SystemPause(true);

	int rc = dlg->ShowModal();
	wxString path = dlg->GetPath();
	wxString dir  = wxFileName(path).GetPath();
	delete dlg;

	if (rc == wxID_OK) {
		PostEtSaveDataRecMessage(path);
	} else {
		SystemPause(false);
	}
	return (rc == wxID_OK);
}
#endif

#ifdef USE_FD1
bool GUI::ShowOpenFloppyDiskDialog(int drv)
{
	const CMsg::Id filter[] = {
		CMsg::Supported_Files_d88,
		CMsg::All_Files_,
		CMsg::End
	};

	wxString title = wxString::Format(CMSG(Open_Floppy_Disk_VDIGIT), drv);

	MyFileDialog *dlg = new MyFileDialog(
		title,
		wxString(config.initial_disk_path),
		wxEmptyString,
		filter,
		wxFD_OPEN | wxFD_FILE_MUST_EXIST);

	SystemPause(true);

	int rc = dlg->ShowModal();
	wxString path = dlg->GetPath();
	uint32_t flags = 0;
	delete dlg;

	if (rc == wxID_OK) {
		PostEtOpenFloppyMessage(drv, path, 0, flags, true);
	} else {
		SystemPause(false);
	}
	return (rc == wxID_OK);
}
int  GUI::ShowSelectFloppyDriveDialog(int drv)
{
	MySelDrvDlg dlg(frame, IDD_RECVIDEOBOX, emu, this);
	dlg.SetPrefix(CMSG(FDD));
	int new_drv = dlg.ShowModal();
	return new_drv >= 0 ? new_drv : drv;
}
bool GUI::ShowOpenBlankFloppyDiskDialog(int drv, uint8_t type)
{
	const CMsg::Id filter[] = {
		CMsg::Supported_Files_d88,
		CMsg::All_Files_,
		CMsg::End
	};

	wxString title = wxString::Format(CMSG(New_Floppy_Disk_VDIGIT), drv);

	_TCHAR file_name[128];
	UTILITY::create_date_file_path(NULL, file_name, 128, _T("d88"));

	MyFileDialog *dlg = new MyFileDialog(
		title,
		wxString(config.initial_disk_path),
		file_name,
		filter,
		wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

	SystemPause(true);

	int sts = dlg->ShowModal();
	wxString path = dlg->GetPath();
	uint32_t flags = 0;
	delete dlg;

	bool rc = (sts == wxID_OK);
	if (rc) {
		rc = emu->create_blank_floppy_disk(path, type);
	}
	if (rc) {
		PostEtOpenFloppyMessage(drv, path, 0, flags, true);
	} else {
		SystemPause(false);
	}
	return rc;
}
#endif

bool GUI::ShowLoadStateDialog(void)
{
	const CMsg::Id filter[] = {
		CMsg::Supported_Files_l3r,
		CMsg::All_Files_,
		CMsg::End
	};

	MyFileDialog *dlg = new MyFileDialog(
		CMsg::Load_Status_Data,
		wxString(config.initial_state_path),
		wxEmptyString,
		filter,
		wxFD_OPEN | wxFD_FILE_MUST_EXIST);

	SystemPause(true);

	int rc = dlg->ShowModal();
	wxString path = dlg->GetPath();
	delete dlg;

	if (rc == wxID_OK) {
		PostEtLoadStatusMessage(path);
	} else {
		SystemPause(false);
	}
	return (rc == wxID_OK);
}
bool GUI::ShowSaveStateDialog(bool cont)
{
	const CMsg::Id filter[] = {
		CMsg::Supported_Files_l3r,
		CMsg::All_Files_,
		CMsg::End
	};

	MyFileDialog *dlg = new MyFileDialog(
		CMsg::Save_Status_Data,
		wxString(config.initial_state_path),
		wxEmptyString,
		filter,
		wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

	SystemPause(true);

	int rc = dlg->ShowModal();
	wxString path = dlg->GetPath();
	wxString dir  = wxFileName(path).GetPath();
	delete dlg;

	if (rc == wxID_OK) {
		PostEtSaveStatusMessage(path, cont);
	} else {
		SystemPause(false);
	}
	return (rc == wxID_OK);
}

bool GUI::ShowOpenAutoKeyDialog(void)
{
	const CMsg::Id filter[] = {
		CMsg::Supported_Files_txt_bas_lpt,
		CMsg::All_Files_,
		CMsg::End
	};

	MyFileDialog *dlg = new MyFileDialog(
		CMsg::Open_Text_File,
		wxString(config.initial_autokey_path),
		wxEmptyString,
		filter,
		wxFD_OPEN | wxFD_FILE_MUST_EXIST);

	SystemPause(true);

	int rc = dlg->ShowModal();
	wxString path = dlg->GetPath();
	delete dlg;

	if (rc == wxID_OK) {
		PostEtLoadAutoKeyMessage(path);
	} else {
		SystemPause(false);
	}
	return (rc == wxID_OK);
}

bool GUI::ShowPlayRecKeyDialog(void)
{
	const CMsg::Id filter[] = {
		CMsg::Supported_Files_l3k,
		CMsg::All_Files_,
		CMsg::End
	};

	MyFileDialog *dlg = new MyFileDialog(
		CMsg::Play_Recorded_Keys,
		wxString(config.initial_state_path),
		wxEmptyString,
		filter,
		wxFD_OPEN | wxFD_FILE_MUST_EXIST);

	SystemPause(true);

	int rc = dlg->ShowModal();
	wxString path = dlg->GetPath();
	delete dlg;

	if (rc == wxID_OK) {
		PostEtLoadRecKeyMessage(path);
	} else {
		SystemPause(false);
	}
	return (rc == wxID_OK);
}

bool GUI::ShowRecordRecKeyDialog(void)
{
	const CMsg::Id filter[] = {
		CMsg::Supported_Files_l3k,
		CMsg::All_Files_,
		CMsg::End
	};

	MyFileDialog *dlg = new MyFileDialog(
		CMsg::Record_Input_Keys,
		wxString(config.initial_state_path),
		wxEmptyString,
		filter,
		wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

	SystemPause(true);

	int rc = dlg->ShowModal();
	wxString path = dlg->GetPath();
	wxString dir  = wxFileName(path).GetPath();
	delete dlg;

	if (rc == wxID_OK) {
		PostEtSaveRecKeyMessage(path, false);
	} else {
		SystemPause(false);
	}
	return (rc == wxID_OK);
}

bool GUI::ShowRecordStateAndRecKeyDialog(void)
{
	bool rc = ShowSaveStateDialog(true);
	if (!rc) return rc;
	return ShowRecordRecKeyDialog();
}

bool GUI::ShowSavePrinterDialog(int drv)
{
	const CMsg::Id filter[] = {
		CMsg::Supported_Files_lpt,
		CMsg::All_Files_,
		CMsg::End
	};

	MyFileDialog *dlg = new MyFileDialog(
		CMsg::Save_Printing_Data,
		wxString(config.initial_printer_path),
		wxEmptyString,
		filter,
		wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

	SystemPause(true);

	int rc = dlg->ShowModal();
	wxString path = dlg->GetPath();
	wxString dir  = wxFileName(path).GetPath();
	delete dlg;

	if (rc == wxID_OK) {
		PostEtSavePrinterMessage(drv, path);
	} else {
		SystemPause(false);
	}
	return (rc == wxID_OK);
}

bool GUI::ShowRecordVideoDialog(int fps_num)
{
	MyRecVideoDlg dlg(frame, IDD_RECVIDEOBOX, emu, this);

	SystemPause(true);

	int rc = dlg.ShowModal();
	if (rc == wxID_OK) {
		PostEtStartRecordVideo(fps_num);
	} else {
		SystemPause(false);
	}
	return (rc == wxID_OK);
}
bool GUI::ShowRecordAudioDialog(void)
{
	MyRecAudioDlg dlg(frame, IDD_RECAUDIOBOX, emu, this);

	SystemPause(true);

	int rc = dlg.ShowModal();
	if (rc == wxID_OK) {
		PostEtStartRecordSound();
	} else {
		SystemPause(false);
	}
	return (rc == wxID_OK);
}
bool GUI::ShowRecordVideoAndAudioDialog(int fps_num)
{
	MyRecVideoDlg dlg1(frame, IDD_RECVIDEOBOX, emu, this);

	SystemPause(true);

	int rc = dlg1.ShowModal();
	if (rc != wxID_OK) {
		SystemPause(false);
		return false;
	}

	MyRecAudioDlg dlg2(frame, IDD_RECAUDIOBOX, emu, this);

	rc = dlg2.ShowModal();
	if (rc == wxID_OK) {
		PostEtStartRecordVideo(fps_num);
	} else {
		SystemPause(false);
	}
	return (rc == wxID_OK);
}

bool GUI::ShowVolumeDialog(void)
{
	MyVolumeDlg dlg(frame, IDD_VOLUME, emu, this);

	dlg.ShowModal();
	return true;
}

bool GUI::ShowKeybindDialog(void)
{
	MyKeybindDlg dlg(frame, IDD_KEYBIND, emu, this);

	SystemPause(true);

	int rc = dlg.ShowModal();
	if (rc == wxID_OK) {
//		save_config();
		emu->save_keybind();
	}
	SystemPause(false);
	return (rc == wxID_OK);
}

bool GUI::ShowConfigureDialog(void)
{
	MyConfigDlg dlg(frame, IDD_CONFIGURE, emu, this);

	SystemPause(true);

	int rc = dlg.ShowModal();
	if (rc == wxID_OK) {
		config.save();
#ifdef USE_OPENGL
		emu->change_opengl_attr();
#endif
	}
	SystemPause(false);
	return (rc == wxID_OK);
}

bool GUI::ShowAboutDialog(void)
{
	MyAboutDialog dlg(frame, wxID_ANY, this);
	dlg.ShowModal();
	return true;
}

bool GUI::ShowVirtualKeyboard(void)
{
#ifdef USE_VKEYBOARD
	if (!vkeyboard) {
//		vkeyboard = new Vkbd::VKeyboard(frame->GetParent());
		vkeyboard = new Vkbd::VKeyboard(frame);

		uint8_t *buf;
		int siz;
		emu->get_vm_key_status_buffer(&buf, &siz);
		FIFOINT *his = emu->get_vm_key_history();
		vkeyboard->SetStatusBufferPtr(buf, siz, VM_KEY_STATUS_VKEYBOARD);
		vkeyboard->SetHistoryBufferPtr(his);
		vkeyboard->Create(emu->resource_path());
		vkeyboard->Show();
	} else {
		vkeyboard->Close();
	}
	return true;
#else
	return false;
#endif
}

void GUI::GetLibVersionString(_TCHAR *str, int max_len, const _TCHAR *sep_str)
{
	int len = 0;

	wxVersionInfo verinfo = wxGetLibraryVersionInfo();
	if (verinfo.HasDescription()) {
		wxString desc = verinfo.GetDescription();
		_tcscpy(str, desc.t_str());
	} else {
		UTILITY::stprintf(str, max_len, _T("wxWidgets Version %d.%d.%d")
			, verinfo.GetMajor(), verinfo.GetMinor(), verinfo.GetMicro());
	}
	_tcscat(str, sep_str);

	len = (int)_tcslen(str);
	GUI_BASE::GetLibVersionString(&str[len], max_len - len, sep_str);
}

void GUI::CreateLedBoxSub()
{
#ifdef USE_LEDBOX
	ledbox->SetParent(frame);
#endif
}

/// input text automatically from clipboard
bool GUI::StartAutoKey(void)
{
	if (!wxTheClipboard->Open()) return false;
	if (wxTheClipboard->IsSupported(wxDF_TEXT)) {
		wxTextDataObject data;
		wxTheClipboard->GetData(data);

		emu->start_auto_key(data.GetText());
	}
	wxTheClipboard->Close();
	return true;
}

//
//
//

void GUI::CreateMyFrame(int x, int y, int w, int h)
{
	if (!frame) {
		frame = new MyFrame(&wxGetApp(), emu, this, x, y, w, h);
	}
}

void GUI::CloseMyFrame()
{
}

MyFrame *GUI::GetMyFrame()
{
	return frame;
}

#if defined(USE_ONIDLE) || defined(USE_ONTIMER)
void GUI::SetMutex(wxMutex *mutex, wxCondition *condition)
{
	mux_need_update = mutex;
	cond_need_update = condition;
}
#endif

/*******************************************************
 * main window frame
 */
IMPLEMENT_CLASS(MyFrame, wxFrame)

// add custom event
wxDECLARE_EVENT(wxEVT_USER1, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_USER1, wxCommandEvent);

// attach event
BEGIN_EVENT_TABLE(MyFrame, wxFrame)
	EVT_CLOSE(MyFrame::OnClose)
	//
	EVT_CHAR_HOOK(MyFrame::OnCharHook)
	//
//	EVT_MOTION(MyFrame::OnMouseMotion)
	EVT_MOVE_END(MyFrame::OnMoveEnd)
	// menu and global key
	EVT_COMMAND(ID_ACCEL_SCREEN, wxEVT_USER1, MyFrame::OnSelect)
	EVT_COMMAND_RANGE(20000, 21999, wxEVT_USER1, MyFrame::OnSelect)
	EVT_MENU_RANGE(20000, 21999, MyFrame::OnSelect)
	// exit
	EVT_MENU(wxID_EXIT, MyFrame::OnExit)
	// help menu
	EVT_MENU(wxID_ABOUT, MyFrame::OnHelpAbout)
	// update menu
	EVT_MENU_OPEN(MyFrame::OnUpdateMenu)
	EVT_MENU_CLOSE(MyFrame::OnCloseMenu)
	// update screen
	EVT_COMMAND(ID_UPDATE_SCREEN, wxEVT_USER1, MyFrame::OnUpdateScreen)
	// update title
	EVT_COMMAND(ID_UPDATE_TITLE, wxEVT_USER1, MyFrame::OnUpdateTitle)
END_EVENT_TABLE()

MyFrame::MyFrame(MyApp *parent, EMU *new_emu, GUI_BASE *new_gui, int x, int y, int w, int h)
	: wxFrame(NULL, IDF_FRAME, wxT(DEVICE_NAME), wxPoint(x, y), wxSize(w, h),
		   wxCAPTION | wxSYSTEM_MENU | wxMINIMIZE_BOX | wxCLOSE_BOX)
{
	app = parent;
	emu = new_emu;
	gui = new_gui;

	// icon
#ifdef __WXMSW__
 #if defined(_BML3MK5)
	SetIcon(wxIcon(wxT("bml3mk5")));
 #elif defined(_MBS1)
	SetIcon(wxIcon(wxT("mbs1")));
 #endif
#elif defined(__WXGTK__) || defined(__WXMOTIF__)
 #if defined(_BML3MK5)
	SetIcon(wxIcon(bml3mk5_xpm));
 #elif defined(_MBS1)
	SetIcon(wxIcon(mbs1_xpm));
 #endif
#endif

	// initialize
	panel = NULL;
	glcanvas = NULL;

	enable_opengl = false;

	fskip_remain = 0;
	rec_fps_no = -1;

	now_updating_menu = 0;
	now_expanding_menu = false;

    // create the main menubar
    menuBar = new wxMenuBar;
	CreateMenu(menuBar);

	// add the menu bar to the MyFrame
    SetMenuBar(menuBar);
	now_showmenu = true;

    // create the MyPanel
    panel = new MyPanel(this);

//	key_mod = emu->get_key_mod_ptr();

	// set window size
	SetClientSize(config.screen_width, config.screen_height);
	Centre();
	Show();

#ifdef USE_OPENGL
	// opengl attribute set
	const int attr_list[] = {
		WX_GL_RGBA,	1,			// Use true color (the default if no attributes at all are specified); do not use a palette.
//		WX_GL_LEVEL,	0,		// Must be followed by 0 for main buffer, >0 for overlay, <0 for underlay.
		WX_GL_DOUBLEBUFFER, 1,	// Use double buffering if present (on if no attributes specified).
//		WX_GL_STEREO,	0,		// Use stereoscopic display.
//		WX_GL_AUX_BUFFERS,	2,	// Specifies number of auxiliary buffers.
#if defined(_RGB555)
		WX_GL_BUFFER_SIZE, 16,	// Specifies the number of bits for buffer if not WX_GL_RGBA.
		WX_GL_MIN_RED,	5,		// Use red buffer with most bits (> MIN_RED bits)
		WX_GL_MIN_GREEN, 5,		// Use green buffer with most bits (> MIN_GREEN bits)
		WX_GL_MIN_BLUE,	5,		// Use blue buffer with most bits (> MIN_BLUE bits)
		WX_GL_MIN_ALPHA, 0,		// Use alpha buffer with most bits (> MIN_ALPHA bits)
		WX_GL_DEPTH_SIZE,	0,	// Specifies number of bits for Z-buffer (typically 0, 16 or 32).
#elif defined(_RGB565)
		WX_GL_BUFFER_SIZE, 16,	// Specifies the number of bits for buffer if not WX_GL_RGBA.
		WX_GL_MIN_RED,	5,		// Use red buffer with most bits (> MIN_RED bits)
		WX_GL_MIN_GREEN, 6,		// Use green buffer with most bits (> MIN_GREEN bits)
		WX_GL_MIN_BLUE,	5,		// Use blue buffer with most bits (> MIN_BLUE bits)
		WX_GL_MIN_ALPHA, 0,		// Use alpha buffer with most bits (> MIN_ALPHA bits)
		WX_GL_DEPTH_SIZE,	0,	// Specifies number of bits for Z-buffer (typically 0, 16 or 32).
#elif defined(_RGB888)
		WX_GL_BUFFER_SIZE, 32,	// Specifies the number of bits for buffer if not WX_GL_RGBA.
		WX_GL_MIN_RED,	8,		// Use red buffer with most bits (> MIN_RED bits)
		WX_GL_MIN_GREEN, 8,		// Use green buffer with most bits (> MIN_GREEN bits)
		WX_GL_MIN_BLUE,	8,		// Use blue buffer with most bits (> MIN_BLUE bits)
		WX_GL_MIN_ALPHA, 8,		// Use alpha buffer with most bits (> MIN_ALPHA bits)
		WX_GL_DEPTH_SIZE,	0,	// Specifies number of bits for Z-buffer (typically 0, 16 or 32).
#endif
//		WX_GL_STENCIL_SIZE, 0,	// Specifies number of bits for stencil buffer.
//		WX_GL_MIN_ACCUM_RED, 8,	// Specifies minimal number of red accumulator bits.
//		WX_GL_MIN_ACCUM_GREEN,8,// Specifies minimal number of green accumulator bits.
//		WX_GL_MIN_ACCUM_BLUE,8, // Specifies minimal number of blue accumulator bits.
//		WX_GL_MIN_ACCUM_ALPHA,8,// Specifies minimal number of alpha accumulator bits.
//		WX_GL_SAMPLE_BUFFERS,1,	// 1 for multisampling support (antialiasing)
//		WX_GL_SAMPLES,4,		// 4 for 2x2 antialiasing supersampling on most graphics cards
		0
	};

	// attribute check
	if (!wxGLCanvas::IsDisplaySupported(attr_list)) {
		logging->out_log(LOG_ERROR,_T("wxGLCanvas::IsDisplaySupported NG"));

		enable_opengl = false;
	} else {
		// create opengl canvas
		glcanvas = new MyGLCanvas(this, wxID_ANY, attr_list, wxDefaultPosition, wxSize(config.screen_width, config.screen_height));

		enable_opengl = true;
	}
	emu->set_use_opengl(enable_opengl ? config.use_opengl : 0);
#endif /* USE_OPENGL */

	// select display panel
	ChangePanel(config.use_opengl);

	// Our MyFrame is the Top Window
    parent->SetTopWindow(this);

	// Accel key
//	wxAcceleratorTable *atbl = this->GetAcceleratorTable();
//	delete atbl;	// destroy default accel keys
//	this->SetAcceleratorTable(wxNullAcceleratorTable);
}

MyFrame::~MyFrame() {
//	wxMenuBar *mb = GetMenuBar();
//	if (mb == NULL) {
//		delete menuBar;
//	}

//	delete glcanvas;
//	delete panel;
}

void MyFrame::PostUserEvent(int id, void *data1)
{
	wxCommandEvent e(wxEVT_USER1, id);
	e.SetEventObject(this);
	e.SetClientData(data1);
	GetEventHandler()->AddPendingEvent(e);
}

void MyFrame::OnSelect(wxCommandEvent &event)
{
	// menu or shortcut key
	if (now_updating_menu > 0) {
		// disable select menu item when menu is updating.
		return;
	}
	gui->ProcessEvent(event);
}

void MyFrame::OnClose(wxCloseEvent & WXUNUSED(event))
{
	Exit();
}
void MyFrame::OnExit(wxCommandEvent & WXUNUSED(event))
{
	Exit();
}
void MyFrame::Exit()
{
	if (emu) emu->resume_window_placement();

	if (gui) gui->Exit();
}
void MyFrame::Close()
{
	// stop emu thread
	app->StopEmuThread();
	//
	emu = NULL;
	gui = NULL;
	// window
	Destroy();
}
void MyFrame::OnMoveEnd(wxMoveEvent&)
{
	gui->MoveLedBox();
}

/// Open dropped file
void MyFrame::OpenDroppedFile(wxString &path)
{
	gui->OpenFileByExtention(path);
}

/// create menu
void MyFrame::CreateMenu(wxMenuBar *mb)
{
	MyMenu *ms;
	_TCHAR cmsg[128];

    // create the control menu
    menuControl = new MyMenu;
    menuControl->AppendCheckItemById(ID_RESET, CMsg::PowerOn);
#if defined(_BML3MK5)
    menuControl->AppendCheckItemById(ID_DIPSWITCH3, CMsg::MODE_Switch);
#endif
    menuControl->AppendById(ID_WARM_RESET, CMsg::Reset_Switch);
#if defined(_MBS1)
	menuControl->AppendSeparator();
		ms = new MyMenu;
		ms->AppendRadioItemById(ID_SYSTEM_MODE_1, CMsg::A_Mode_S1);
		ms->AppendRadioItemById(ID_SYSTEM_MODE_2, CMsg::B_Mode_L3);
	menuControl->AppendSubMenuById(ms, CMsg::System_Mode);
#endif
	menuControl->AppendSeparator();
		ms = new MyMenu;
		ms->AppendRadioItemById(ID_FDD_TYPE_1, CMsg::Non_FDD);
		ms->AppendRadioItemById(ID_FDD_TYPE_2, CMsg::FD3inch_compact_FDD);
#if defined(_BML3MK5)
		ms->AppendRadioItemById(ID_FDD_TYPE_3, CMsg::FD5inch_mini_FDD);
		ms->AppendRadioItemById(ID_FDD_TYPE_4, CMsg::FD8inch_standard_FDD);
#elif defined(_MBS1)
		ms->AppendRadioItemById(ID_FDD_TYPE_3, CMsg::FD5inch_mini_FDD_2D_Type);
		ms->AppendRadioItemById(ID_FDD_TYPE_4, CMsg::FD5inch_mini_FDD_2HD_Type);
#endif
	menuControl->AppendSubMenuById(ms, CMsg::FDD_Type);
	menuControl->AppendSeparator();
    menuControl->AppendCheckItemById(ID_PAUSE, CMsg::Pause);
	menuControl->AppendSeparator();
		ms = new MyMenu;
		ms->AppendRadioItemById(ID_CPU_POWER0, CMsg::CPU_x0_5);
		ms->AppendRadioItemById(ID_CPU_POWER1, CMsg::CPU_x1);
		ms->AppendRadioItemById(ID_CPU_POWER2, CMsg::CPU_x2);
		ms->AppendRadioItemById(ID_CPU_POWER3, CMsg::CPU_x4);
		ms->AppendRadioItemById(ID_CPU_POWER4, CMsg::CPU_x8);
		ms->AppendRadioItemById(ID_CPU_POWER5, CMsg::CPU_x16);
	menuControl->AppendSubMenuById(ms, CMsg::CPU_Speed);
	menuControl->AppendSeparator();
	menuControl->AppendCheckItemById(ID_SYNC_IRQ, CMsg::Sync_Machine_Speed_With_CPU_Speed);
	menuControl->AppendSeparator();
		ms = new MyMenu;
		ms->AppendById(ID_AUTOKEY_OPEN, CMsg::Open_);
		ms->AppendById(ID_AUTOKEY_START, CMsg::Paste);
		ms->AppendById(ID_AUTOKEY_STOP, CMsg::Stop);
	menuControl->AppendSubMenuById(ms, CMsg::Auto_Key);
	menuControl->AppendSeparator();
		ms = new MyMenu;
		ms->AppendCheckItemById(ID_RECKEY_PLAY, CMsg::Play_);
		ms->AppendById(ID_RECKEY_STOP_PLAY, CMsg::Stop_Playing);
		ms->AppendSeparator();
		ms->AppendCheckItemById(ID_RECKEY_REC, CMsg::Record_);
		ms->AppendById(ID_RECKEY_STOP_REC, CMsg::Stop_Recording);
	menuControl->AppendSubMenuById(ms, CMsg::Record_Key);
	menuControl->AppendSeparator();
	menuControl->AppendById(ID_LOAD_STATE, CMsg::Load_State_);
	menuControl->AppendById(ID_SAVE_STATE, CMsg::Save_State_);
	menuControl->AppendSeparator();
	menuStateRecent = new MyMenu;
	menuControl->AppendSubMenuById(menuStateRecent, CMsg::Recent_State_Files);
	menuControl->AppendSeparator();
	menuControl->AppendById(wxID_EXIT, CMsg::Exit_);
    // add the control menu to the menu bar
    mb->Append(menuControl, CMSG(Control));

	// create the tape menu
	menuTape = new MyMenu;
    menuTape->AppendCheckItemById(ID_PLAY_DATAREC, CMsg::Play_);
    menuTape->AppendCheckItemById(ID_REC_DATAREC, CMsg::Rec_);
    menuTape->AppendById(ID_CLOSE_DATAREC, CMsg::Eject);
	menuTape->AppendSeparator();
    menuTape->AppendById(ID_REWIND_DATAREC, CMsg::Rewind);
    menuTape->AppendById(ID_FAST_FORWARD_DATAREC, CMsg::F_F_);
    menuTape->AppendById(ID_STOP_DATAREC, CMsg::Stop);
	menuTape->AppendSeparator();
    menuTape->AppendCheckItemById(ID_REAL_DATAREC, CMsg::Real_Mode);
	menuTape->AppendSeparator();
	menuTapeRecent = new MyMenu;
	menuTape->AppendSubMenuById(menuTapeRecent, CMsg::Recent_Files);
    // add the tape menu to the menu bar
    mb->Append(menuTape, CMSG(Tape));

	// create the fdd menu
	for(int drv=0; drv<FDD_NUMS; drv++) {
		menuFdd[drv] = new MyMenu;
		menuFdd[drv]->AppendCheckItemById(ID_OPEN_FD1 + (drv * 100), CMsg::Insert_);
		menuFdd[drv]->AppendById(ID_CHANGE_FD1 + (drv * 100), CMsg::Change_Side_to_B);
		menuFdd[drv]->AppendById(ID_CLOSE_FD1 + (drv * 100), CMsg::Eject);
			ms = new MyMenu;
			ms->AppendById(ID_OPEN_BLANK_2D_FD1 + (drv * 100), CMsg::Insert_Blank_2D_);
			ms->AppendById(ID_OPEN_BLANK_2HD_FD1 + (drv * 100), CMsg::Insert_Blank_2HD_);
		menuFdd[drv]->AppendSubMenuById(ms, CMsg::New);
		menuFdd[drv]->AppendSeparator();
		menuFdd[drv]->AppendCheckItemById(ID_WRITEPROTECT_FD1 + (drv * 100), CMsg::Write_Protect);
		menuFdd[drv]->AppendSeparator();
			menuFddMulti[drv] = new MyMenu;
		menuFdd[drv]->AppendSubMenuById(menuFddMulti[drv], CMsg::Multi_Volume);
		menuFdd[drv]->AppendSeparator();
			menuFddRecent[drv] = new MyMenu;
		menuFdd[drv]->AppendSubMenuById(menuFddRecent[drv], CMsg::Recent_Files);
		// add the fdd menu to the menu bar
		mb->Append(menuFdd[drv], wxString::Format(CMSG(FDDVDIGIT),drv));
	}

	// create the screen menu
	menuScreen = new MyMenu;
		ms = new MyMenu;
		ms->AppendRadioItemById(ID_SCREEN_VFRAME, CMsg::Auto);
		ms->AppendRadioItemById(ID_SCREEN_FPS60, CMsg::F60fps);
		ms->AppendRadioItemById(ID_SCREEN_FPS30, CMsg::F30fps);
		ms->AppendRadioItemById(ID_SCREEN_FPS20, CMsg::F20fps);
		ms->AppendRadioItemById(ID_SCREEN_FPS15, CMsg::F15fps);
		ms->AppendRadioItemById(ID_SCREEN_FPS12, CMsg::F12fps);
		ms->AppendRadioItemById(ID_SCREEN_FPS10, CMsg::F10fps);
	menuScreen->AppendSubMenuById(ms, CMsg::Frame_Rate);
	menuScreen->AppendSeparator();
		ms = new MyMenu;
		for(int i=0; i<2; i++) {
			if (!gui->GetRecordVideoSizeStr(i, cmsg)) break;
			wxString str(cmsg);
			ms->AppendRadioItem(ID_SCREEN_REC_SIZE1 + i, str);
		}
		ms->AppendSeparator();
		ms->AppendCheckItemById(ID_SCREEN_REC60, CMsg::Rec_60fps);
		ms->AppendCheckItemById(ID_SCREEN_REC30, CMsg::Rec_30fps);
		ms->AppendCheckItemById(ID_SCREEN_REC20, CMsg::Rec_20fps);
		ms->AppendCheckItemById(ID_SCREEN_REC15, CMsg::Rec_15fps);
		ms->AppendCheckItemById(ID_SCREEN_REC12, CMsg::Rec_12fps);
		ms->AppendCheckItemById(ID_SCREEN_REC10, CMsg::Rec_10fps);
		ms->AppendById(ID_SCREEN_STOP, CMsg::Stop);
		ms->AppendSeparator();
		ms->AppendById(ID_SCREEN_CAPTURE, CMsg::Capture);
	menuScreen->AppendSubMenuById(ms, CMsg::Record_Screen);
	menuScreen->AppendSeparator();
		ms = new MyMenu;
		for(int i=0; i<WINDOW_MODE_MAX; i++) {
			if (!gui->GetWindowModeStr(i, cmsg)) break;
			wxString str(cmsg);
			ms->AppendCheckItem(ID_SCREEN_WINDOW1 + i, str);
		}
	menuScreen->AppendSubMenuById(ms, CMsg::Window);
		ms = new MyMenu;
		ms->AppendCheckItemById(ID_SCREEN_STRETCH, CMsg::Stretch_Screen);
		ms->AppendCheckItemById(ID_SCREEN_CUTOUT, CMsg::Cutout_Screen);
		ms->AppendSeparator();
		for(int disp_no=0; disp_no<gui->GetDisplayDeviceCount(); disp_no++) {
			MyMenu *mss = new MyMenu;
			for(int i=0; i<gui->GetFullScreenModeCount(disp_no); i++) {
				if (!gui->GetFullScreenModeStr(disp_no, i, cmsg)) break;
				wxString str(cmsg);
				mss->AppendCheckItem(disp_no * VIDEO_MODE_MAX + ID_SCREEN_FULLSCREEN0_01 + i, str);
			}
			gui->GetDisplayDeviceStr(CMSG(Display), disp_no, cmsg);
			wxString str(cmsg);
			ms->AppendSubMenu(mss, str);
		}
	menuScreen->AppendSubMenuById(ms, CMsg::Fullscreen);
		ms = new MyMenu;
		for(int i=0; i<gui->GetPixelAspectModeCount(); i++) {
			gui->GetPixelAspectModeStr(i, cmsg);
			wxString str(cmsg);
			ms->AppendCheckItem(ID_SCREEN_PIXEL_ASPECT0 + i, str);
		}
	menuScreen->AppendSubMenuById(ms, CMsg::Aspect_Ratio);
	menuScreen->AppendSeparator();
		ms = new MyMenu;
		ms->AppendCheckItemById(ID_SCREEN_SCANLINE0, CMsg::Full_Draw);
		ms->AppendCheckItemById(ID_SCREEN_SCANLINE1, CMsg::Scanline);
		ms->AppendCheckItemById(ID_SCREEN_SCANLINE2, CMsg::Stripe);
		ms->AppendCheckItemById(ID_SCREEN_SCANLINE3, CMsg::Checker);
	menuScreen->AppendSubMenuById(ms, CMsg::Drawing_Mode);
	menuScreen->AppendSeparator();
	menuScreen->AppendCheckItemById(ID_SCREEN_AFTERIMAGE1, CMsg::Afterimage1);
	menuScreen->AppendCheckItemById(ID_SCREEN_AFTERIMAGE2, CMsg::Afterimage2);
#ifdef USE_KEEPIMAGE
	menuScreen->AppendSeparator();
	menuScreen->AppendCheckItemById(ID_SCREEN_KEEPIMAGE1, CMsg::Keepimage1);
	menuScreen->AppendCheckItemById(ID_SCREEN_KEEPIMAGE2, CMsg::Keepimage2);
#endif
#if defined(_MBS1)
	menuScreen->AppendSeparator();
	menuScreen->AppendRadioItemById(ID_SCREEN_DIGITAL, CMsg::Digital_RGB);
	menuScreen->AppendRadioItemById(ID_SCREEN_ANALOG, CMsg::Analog_RGB);
#endif
	menuScreen->AppendSeparator();
	menuScreen->AppendCheckItemById(ID_SCREEN_OPENGL_SYNC, CMsg::Use_OpenGL_Sync);
	menuScreen->AppendCheckItemById(ID_SCREEN_OPENGL_ASYNC, CMsg::Use_OpenGL_Async);
		ms = new MyMenu;
		ms->AppendCheckItemById(ID_SCREEN_OPENGL_FILTER0, CMsg::Nearest_Neighbour);
		ms->AppendCheckItemById(ID_SCREEN_OPENGL_FILTER1, CMsg::Linear);
	menuScreen->AppendSubMenuById(ms, CMsg::OpenGL_Filter);
    // add the screen menu to the menu bar
    mb->Append(menuScreen, CMSG(Screen));

	// create the sound menu
	menuSound = new MyMenu;
	menuSound->AppendById(ID_SOUND_VOLUME, CMsg::Volume_);
	menuSound->AppendSeparator();
		ms = new MyMenu;
		ms->AppendById(ID_SOUND_REC, CMsg::Start_);
		ms->AppendById(ID_SOUND_STOP, CMsg::Stop);
	menuSound->AppendSubMenuById(ms, CMsg::Record_Sound);
	menuSound->AppendSeparator();
//	menuSound->AppendRadioItemById(ID_SOUND_FREQ0, CMsg::F2000Hz);
//	menuSound->AppendRadioItemById(ID_SOUND_FREQ1, CMsg::F4000Hz);
	menuSound->AppendRadioItemById(ID_SOUND_FREQ2, CMsg::F8000Hz);
	menuSound->AppendRadioItemById(ID_SOUND_FREQ3, CMsg::F11025Hz);
	menuSound->AppendRadioItemById(ID_SOUND_FREQ4, CMsg::F22050Hz);
	menuSound->AppendRadioItemById(ID_SOUND_FREQ5, CMsg::F44100Hz);
	menuSound->AppendRadioItemById(ID_SOUND_FREQ6, CMsg::F48000Hz);
	menuSound->AppendRadioItemById(ID_SOUND_FREQ7, CMsg::F96000Hz);
	menuSound->AppendSeparator();
	menuSound->AppendRadioItemById(ID_SOUND_LATE0, CMsg::S50msec);
	menuSound->AppendRadioItemById(ID_SOUND_LATE1, CMsg::S75msec);
	menuSound->AppendRadioItemById(ID_SOUND_LATE2, CMsg::S100msec);
	menuSound->AppendRadioItemById(ID_SOUND_LATE3, CMsg::S200msec);
	menuSound->AppendRadioItemById(ID_SOUND_LATE4, CMsg::S300msec);
	menuSound->AppendRadioItemById(ID_SOUND_LATE5, CMsg::S400msec);
    // add the sound menu to the menu bar
    mb->Append(menuSound, CMSG(Sound));

	// create the device menu
	menuDevice = new MyMenu;
	for(int dev=0; dev<MAX_PRINTER; dev++) {
		int id = (ID_PRINTER1_SAVE - ID_PRINTER0_SAVE) * dev;
		ms = new MyMenu;
		ms->AppendById(ID_PRINTER0_SAVE + id, CMsg::Save_);
		ms->AppendById(ID_PRINTER0_PRINT + id, CMsg::Print_to_mpprinter);
		ms->AppendById(ID_PRINTER0_CLEAR + id, CMsg::Clear);
		ms->AppendSeparator();
		ms->AppendCheckItemById(ID_PRINTER0_ONLINE + id, CMsg::Online);
		ms->AppendSeparator();
		ms->AppendCheckItemById(ID_PRINTER0_DIRECT + id, CMsg::Send_to_mpprinter_concurrently);
		menuDevice->AppendSubMenu(ms, wxString::Format(CMSG(LPTVDIGIT),dev));
	}
	menuDevice->AppendSeparator();
	for(int dev=0; dev<MAX_COMM; dev++) {
		int id = (ID_COMM1_SERVER - ID_COMM0_SERVER) * dev;
		ms = new MyMenu;
		ms->AppendCheckItemById(ID_COMM0_SERVER + id, CMsg::Enable_Server);
		{
			menuCommConnect[dev] = new MyMenu;
			ms->AppendSubMenuById(menuCommConnect[dev], CMsg::Connect);
		}
		ms->AppendSeparator();
		ms->AppendCheckItemById(ID_COMM0_THROUGH + id, CMsg::Comm_With_Byte_Data);
		ms->AppendSeparator();
		{
			MyMenu *mms = new MyMenu;
			mms->AppendCheckItemById(ID_COMM0_BINARY + id, CMsg::Binary_Mode);
			mms->AppendSeparator();
			mms->AppendCheckItemById(ID_COMM0_WILLECHO + id, CMsg::Send_WILL_ECHO);
			ms->AppendSubMenuById(mms, CMsg::Options_For_Telnet);
		}
		menuDevice->AppendSubMenu(ms, wxString::Format(CMSG(COMVDIGIT),dev));
	}
    // add the device menu to the menu bar
    mb->Append(menuDevice, CMSG(Devices));

	// create the options menu
	menuOptions = new MyMenu;
	menuOptions->AppendCheckItemById(ID_OPTIONS_LEDBOX_SHOW, CMsg::Show_LED);
#ifdef USE_OUTSIDE_LEDBOX
	menuOptions->AppendCheckItemById(ID_OPTIONS_LEDBOX_INSIDE, CMsg::Inside_LED);
#endif
	menuOptions->AppendCheckItemById(ID_OPTIONS_MSGBOARD, CMsg::Show_Message);
#ifdef USE_PERFORMANCE_METER
	menuOptions->AppendCheckItemById(ID_OPTIONS_PMETER, CMsg::Show_Performance_Meter);
#endif
	menuOptions->AppendSeparator();
#ifdef USE_JOYSTICK
	menuOptions->AppendCheckItemById(ID_OPTIONS_JOYPAD0, CMsg::Use_Joypad_Key_Assigned);
#ifdef USE_PIAJOYSTICK
	menuOptions->AppendCheckItemById(ID_OPTIONS_JOYPAD1, CMsg::Use_Joypad_PIA_Type);
#endif
#endif
#ifdef USE_LIGHTPEN
	menuOptions->AppendCheckItemById(ID_OPTIONS_LIGHTPEN, CMsg::Enable_Lightpen);
#endif
#ifdef USE_MOUSE
	menuOptions->AppendCheckItemById(ID_OPTIONS_MOUSE, CMsg::Enable_Mouse);
#endif
	menuOptions->AppendCheckItemById(ID_OPTIONS_LOOSEN_KEY, CMsg::Loosen_Key_Stroke_Game);
	menuOptions->AppendSeparator();
	menuOptions->AppendById(ID_OPTIONS_KEYBIND, CMsg::Keybind_);
	menuOptions->AppendCheckItemById(ID_OPTIONS_VKEYBOARD, CMsg::Virtual_Keyboard);
	menuOptions->AppendSeparator();
#ifdef USE_DEBUGGER
	menuOptions->AppendById(ID_OPEN_DEBUGGER0, CMsg::Start_Debugger);
	menuOptions->AppendById(ID_CLOSE_DEBUGGER, CMsg::Stop_Debugger);
	menuOptions->AppendSeparator();
#endif
	menuOptions->AppendById(ID_OPTIONS_CONFIG, CMsg::Configure_);
    // add the options menu to the menu bar
    mb->Append(menuOptions, CMSG(Options));

	// create the help menu
    MyMenu *menuHelp = new MyMenu;
    menuHelp->AppendById(wxID_ABOUT, CMsg::About_);
    // add the help menu to the menu bar
    mb->Append(menuHelp, CMSG(Help));
}

/// update menu status
void MyFrame::OnUpdateMenu(wxMenuEvent &event)
{
	wxMenu *menu = event.GetMenu();
	now_expanding_menu = true;
	UpdateMenu(menu);
}

void MyFrame::OnCloseMenu(wxMenuEvent &event)
{
	now_expanding_menu = false;
}

/// update screen
void MyFrame::OnUpdateScreen(wxCommandEvent &)
{
	UpdateScreen();
}

void MyFrame::OnUpdateTitle(wxCommandEvent &event)
{
	UpdateTitle(event.GetClientData());
}

/// update menu status
void MyFrame::UpdateMenu(int flags)
{
#if defined(USE_ONIDLE) || defined(USE_ONTIMER)
#ifdef __WXGTK__
	// Gtk send event when change menu item (check or radio) in program.
	// So, disable this event and don't process it.
	now_updating_menu = 3;
#endif
#endif
	if (flags & 0x0001) UpdateMenuControl(true);
	if (flags & 0x0002) UpdateMenuTape(true);
	if (flags & 0x0004) UpdateMenuFdd(0, true);
	if (flags & 0x0008) UpdateMenuFdd(1, true);
	if (flags & 0x0010) UpdateMenuScreen();
	if (flags & 0x0020) UpdateMenuSound();
	if (flags & 0x0040) UpdateMenuPrinter(0);
	if (flags & 0x0080) UpdateMenuPrinter(1);
	if (flags & 0x0100) UpdateMenuPrinter(2);
	if (flags & 0x0200) UpdateMenuComm(0, true);
	if (flags & 0x0400) UpdateMenuComm(1, true);
	if (flags & 0x0800) UpdateMenuOptions();
}
void MyFrame::UpdateMenu(wxMenu *menu)
{
#if defined(USE_ONIDLE) || defined(USE_ONTIMER)
#ifdef __WXGTK__
	// Gtk send event when change menu item (check or radio) in program.
	// So, disable this event and don't process it.
	now_updating_menu = 3;
#endif
#endif
	if (menu == menuControl) UpdateMenuControl(false);
	if (menu == menuTape) UpdateMenuTape(false);
	if (menu == menuFdd[0]) UpdateMenuFdd(0, false);
	if (menu == menuFdd[1]) UpdateMenuFdd(1, false);
	if (menu == menuScreen) UpdateMenuScreen();
	if (menu == menuSound) UpdateMenuSound();
	if (menu == menuDevice) UpdateMenuPrinter(0);
	if (menu == menuDevice) UpdateMenuPrinter(1);
	if (menu == menuDevice) UpdateMenuPrinter(2);
	if (menu == menuDevice) UpdateMenuComm(0, false);
	if (menu == menuDevice) UpdateMenuComm(1, false);
	if (menu == menuOptions) UpdateMenuOptions();
}
/// update recent files
void MyFrame::UpdateMenuRecentFiles(MyMenu *menu, int id, CRecentPathList &recent_path, bool remake)
{
	if (remake) {
		bool flag = false;
		int count = (int)menu->GetMenuItemCount();
		for(int i=0; i<count; i++) {
			if (menu->FindItem(id + i) != NULL) {
				menu->Delete(id + i);
			}
		}
		for(int i=0; i<recent_path.Count() && i<MAX_HISTORY; i++) {
			_TCHAR path[_MAX_PATH];
			path[0] = '\0';
			gui->GetRecentFileStr(recent_path.Item(i)->path, recent_path.Item(i)->num, path, 72);
			if (path[0] != '\0') {
				menu->Append(id + i, wxString(path));
				flag = true;
			}
		}
		if(!flag) {
			menu->AppendById(id, CMsg::None_);
			menu->Enable(id, false);
		}
	}
}
/// update multi volume
void MyFrame::UpdateMenuMultiVolume(MyMenu *menu, int drv, int id, bool remake)
{
	bool flag = false;
	int count = (int)menu->GetMenuItemCount();
	D88File *d88_file = emu->get_d88_file(drv);
	remake |= emu->changed_cur_bank(drv);
	if (remake) {
		for(int i=0; i<count; i++) {
			if (menu->FindItem(id + i) != NULL) {
				menu->Delete(id + i);
			}
		}
	}
	int bank_nums = d88_file->GetBanks().Count();
	if(bank_nums >= 1) {
		for(int i=0; i < bank_nums; i++) {
			const D88Bank *d88_bank = d88_file->GetBank(i);
			if (remake) {
				wxString name = wxString::Format("%2d: ", i + 1);
				if (d88_bank->GetNameLength() > 0) {
					name += d88_bank->GetName();
				} else {
					name += CMSG(no_label);
				}
				menu->AppendCheckItem(id + i, name);
			}
			menu->Enable(id + i, bank_nums > 1);
			menu->Check(id + i, i == d88_file->GetCurrentBank());
		}
		flag = true;
	}
	if(remake && !flag) {
		menu->AppendCheckItemById(id, CMsg::None_);
		menu->Enable(id, false);
	}
}
/// select radio item
#define SELECT_MENU_ITEM(menu, id1, id2, val) { \
	for(int i=0; i<=(id2-id1); i++) { \
		if (menu->FindItem(id1+i) != NULL) { \
			menu->Check(id1+i, (val == i)); \
		} \
	} \
}

/// enable items
#define ENABLE_MENU_ITEMS(menu, id1, id2, val) { \
	for(int i=0; i<=(id2-id1); i++) { \
		if (menu->FindItem(id1+i) != NULL) { \
			menu->Enable(id1+i, val); \
		} \
	} \
}

/// update control menu status
void MyFrame::UpdateMenuControl(bool remake)
{
	menuControl->Check(ID_RESET, !config.now_power_off);
#if defined(_BML3MK5)
	menuControl->Check(ID_DIPSWITCH3, (config.dipswitch & (1 << 2)) ? true : false);
#endif
#if defined(_MBS1)
	menuControl->Check(ID_SYSTEM_MODE_1, gui->GetSystemMode() == 1);
	menuControl->Check(ID_SYSTEM_MODE_2, gui->GetSystemMode() == 0);
#endif
	SELECT_MENU_ITEM(menuControl, ID_FDD_TYPE_1, ID_FDD_TYPE_3, gui->NextFddType());
	SELECT_MENU_ITEM(menuControl, ID_CPU_POWER0, ID_CPU_POWER5, config.cpu_power);
	menuControl->Check(ID_SYNC_IRQ, config.sync_irq);
	menuControl->Check(ID_RECKEY_PLAY, config.reckey_playing);
	menuControl->Check(ID_RECKEY_REC,  config.reckey_recording);
	// update recent files
	UpdateMenuRecentFiles(menuStateRecent, ID_RECENT_STATE, config.recent_state_path, config.recent_state_path.updated || remake);
	config.recent_state_path.updated = false;
}
/// update tape menu status
void MyFrame::UpdateMenuTape(bool remake)
{
	menuTape->Check(ID_PLAY_DATAREC, (emu->datarec_opened(true)));
	menuTape->Check(ID_REC_DATAREC, (emu->datarec_opened(false)));
	menuTape->Check(ID_REAL_DATAREC, config.realmode_datarec);
	// update recent files
	UpdateMenuRecentFiles(menuTapeRecent, ID_RECENT_DATAREC, config.recent_datarec_path, config.recent_datarec_path.updated || remake);
	config.recent_datarec_path.updated = false;
}
/// update fdd menu status
void MyFrame::UpdateMenuFdd(int drv, bool remake)
{
	// open
	menuFdd[drv]->Check(ID_OPEN_FD1 + (drv * 100), emu->disk_inserted(drv));
	// side
	menuFdd[drv]->Enable(ID_CHANGE_FD1 + (drv * 100), (config.fdd_type != 0));
	CMsg::Id labelid;
	int side = emu->get_disk_side(drv);
	if (side) {
		// ..Change side to A
		labelid = CMsg::Change_Side_to_A;
	} else {
		labelid = CMsg::Change_Side_to_B;
	}
	menuFdd[drv]->SetLabel(ID_CHANGE_FD1 + (drv * 100), gMessages.Get(labelid));
	// write protect
	menuFdd[drv]->Enable(ID_WRITEPROTECT_FD1 + (drv * 100), emu->disk_inserted(drv));
	menuFdd[drv]->Check(ID_WRITEPROTECT_FD1 + (drv * 100), emu->disk_write_protected(drv));
	// update multi volume
	UpdateMenuMultiVolume(menuFddMulti[drv], drv, (ID_SELECT_D88_BANK1 + (drv * 100)), config.recent_disk_path[drv].updated || remake);
	// update recent files
	UpdateMenuRecentFiles(menuFddRecent[drv], (ID_RECENT_FD1 + (drv * 100)), config.recent_disk_path[drv],  config.recent_disk_path[drv].updated || remake);
	config.recent_disk_path[drv].updated = false;
}
/// update screen menu status
void MyFrame::UpdateMenuScreen()
{
	SELECT_MENU_ITEM(menuScreen, ID_SCREEN_VFRAME, ID_SCREEN_FPS10, config.fps_no);
	SELECT_MENU_ITEM(menuScreen, ID_SCREEN_WINDOW1, ID_SCREEN_WINDOW8, config.window_mode);
	SELECT_MENU_ITEM(menuScreen, ID_SCREEN_REC_SIZE1, ID_SCREEN_REC_SIZE2, config.screen_video_size);
	SELECT_MENU_ITEM(menuScreen, ID_SCREEN_PIXEL_ASPECT0, ID_SCREEN_PIXEL_ASPECT2, gui->GetPixelAspectMode());
	bool now_rec = gui->NowRecordingVideo() | gui->NowRecordingSound();
	if (!now_rec) {
		SELECT_MENU_ITEM(menuScreen, ID_SCREEN_REC60, ID_SCREEN_REC10, -1);
	}
	ENABLE_MENU_ITEMS(menuScreen, ID_SCREEN_REC_SIZE1, ID_SCREEN_REC_SIZE2, !now_rec);
	ENABLE_MENU_ITEMS(menuScreen, ID_SCREEN_REC60, ID_SCREEN_REC10, !now_rec);
	if (config.window_mode < WINDOW_MODE_MAX) {
		ENABLE_MENU_ITEMS(menuScreen, ID_SCREEN_WINDOW1, ID_SCREEN_WINDOW8, true);
		ENABLE_MENU_ITEMS(menuScreen, ID_SCREEN_FULLSCREEN0_01, ID_SCREEN_FULLSCREEN5_01 + VIDEO_MODE_MAX, true);
	} else {
		ENABLE_MENU_ITEMS(menuScreen, ID_SCREEN_WINDOW1, ID_SCREEN_WINDOW8, true);
		ENABLE_MENU_ITEMS(menuScreen, ID_SCREEN_FULLSCREEN0_01, ID_SCREEN_FULLSCREEN5_01 + VIDEO_MODE_MAX, false);
	}
	menuScreen->Check(ID_SCREEN_STRETCH, config.stretch_screen == 1);
	menuScreen->Check(ID_SCREEN_CUTOUT, config.stretch_screen == 2);
	menuScreen->Check(ID_SCREEN_SCANLINE0, gui->GetDrawMode() == 0);
	menuScreen->Check(ID_SCREEN_SCANLINE1, gui->GetDrawMode() == 1);
	menuScreen->Check(ID_SCREEN_SCANLINE2, gui->GetDrawMode() == 2);
	menuScreen->Check(ID_SCREEN_SCANLINE3, gui->GetDrawMode() == 3);
	menuScreen->Check(ID_SCREEN_AFTERIMAGE1, (config.afterimage == 1));
	menuScreen->Check(ID_SCREEN_AFTERIMAGE2, (config.afterimage == 2));
#ifdef USE_KEEPIMAGE
	menuScreen->Check(ID_SCREEN_KEEPIMAGE1, (config.keepimage == 1));
	menuScreen->Check(ID_SCREEN_KEEPIMAGE2, (config.keepimage == 2));
#endif
#if defined(_MBS1)
	menuScreen->Check(ID_SCREEN_DIGITAL, gui->GetRGBTypeMode() == 0);
	menuScreen->Check(ID_SCREEN_ANALOG, gui->GetRGBTypeMode() == 1);
#endif
#ifdef USE_OPENGL
	menuScreen->Check(ID_SCREEN_OPENGL_SYNC, (config.use_opengl == 1));
	menuScreen->Check(ID_SCREEN_OPENGL_ASYNC, (config.use_opengl == 2));
#endif
	menuScreen->Enable(ID_SCREEN_OPENGL_SYNC, enable_opengl);
	menuScreen->Enable(ID_SCREEN_OPENGL_ASYNC, enable_opengl);
#ifdef USE_OPENGL
	SELECT_MENU_ITEM(menuScreen, ID_SCREEN_OPENGL_FILTER0, ID_SCREEN_OPENGL_FILTER1, gui->GetOpenGLFilter());
#endif
}
/// update sound menu status
void MyFrame::UpdateMenuSound()
{
	bool now_rec = gui->NowRecordingVideo() | gui->NowRecordingSound();
	menuSound->Enable(ID_SOUND_REC, !now_rec);
//	menuSound->Enable(ID_SOUND_STOP, now_rec);
	SELECT_MENU_ITEM(menuSound, ID_SOUND_FREQ0, ID_SOUND_FREQ7, config.sound_frequency);
	SELECT_MENU_ITEM(menuSound, ID_SOUND_LATE0, ID_SOUND_LATE4, config.sound_latency);
}
/// update printer menu status
void MyFrame::UpdateMenuPrinter(int dev)
{
	if (0 <= dev && dev < 3) {
		int id = (ID_PRINTER1_SAVE - ID_PRINTER0_SAVE) * dev;
		int size = emu->get_printer_buffer_size(dev);
		menuDevice->Enable(ID_PRINTER0_SAVE + id, size > 0);
		menuDevice->Enable(ID_PRINTER0_PRINT + id, size > 0);
		menuDevice->Check(ID_PRINTER0_ONLINE + id, gui->IsOnlinePrinter(dev));
		menuDevice->Check(ID_PRINTER0_DIRECT + id, gui->IsEnablePrinterDirect(dev));
	}
}
/// update comm menu status
void MyFrame::UpdateMenuComm(int dev, bool remake)
{
	if (0 <= dev && dev <= 2) {
		int id = (ID_COMM1_SERVER - ID_COMM0_SERVER) * dev;
		menuDevice->Check(ID_COMM0_SERVER + id, gui->IsEnableCommServer(dev));
//		menuDevice->Check(ID_COMM0_CONNECT + id, gui->NowConnectingComm(dev, 0));
		menuDevice->Check(ID_COMM0_THROUGH + id, gui->NowCommThroughMode(dev));
		menuDevice->Check(ID_COMM0_BINARY + id, gui->NowCommBinaryMode(dev));

		UpdateMenuCommConnect(dev, menuCommConnect[dev]);
	}
}
/// update comm connection list
void MyFrame::UpdateMenuCommConnect(int dev, MyMenu *menu)
{
	int width = (ID_COMM1_PORT1 - ID_COMM0_PORT1) * dev;
	int count = (int)menu->GetMenuItemCount();
	int id = 0;
	for(int i=0; i<count; i++) {
		wxMenuItem *item = menu->FindItemByPosition(0);
		menu->Delete(item);
	}

	id = ID_COMM0_CONNECT + (ID_COMM1_SERVER - ID_COMM0_SERVER) * dev;
	menu->AppendCheckItemById(id, CMsg::Ethernet);
	menu->Check(id, gui->NowConnectingComm(dev, 0));

	int uarts = gui->EnumUarts();

	if (uarts > 0) {
		menu->AppendSeparator();
	}

	for(int i=0; i<uarts; i++) {
		_TCHAR str[128];
		id = ID_COMM0_PORT1 + i + width;
		gui->GetUartDescription(i, str, sizeof(str) / sizeof(_TCHAR));
		menu->AppendCheckItem(id, str);
		menu->Check(id, gui->NowConnectingComm(dev, i + 1));
	}
}
/// update options menu status
void MyFrame::UpdateMenuOptions()
{
	menuOptions->Check(ID_OPTIONS_LEDBOX_SHOW, FLG_SHOWLEDBOX != 0);
#ifdef USE_OUTSIDE_LEDBOX
	menuOptions->Check(ID_OPTIONS_LEDBOX_INSIDE, FLG_INSIDELEDBOX != 0);
#endif
	menuOptions->Check(ID_OPTIONS_MSGBOARD, FLG_SHOWMSGBOARD != 0);
#ifdef USE_PERFORMANCE_METER
	menuOptions->Check(ID_OPTIONS_PMETER, gui->IsShownPMeter());
#endif
#ifdef USE_JOYSTICK
	menuOptions->Check(ID_OPTIONS_JOYPAD0, FLG_USEJOYSTICK != 0);
#ifdef USE_PIAJOYSTICK
	menuOptions->Check(ID_OPTIONS_JOYPAD1, FLG_USEPIAJOYSTICK != 0);
#endif
#endif
#ifdef USE_LIGHTPEN
	menuOptions->Check(ID_OPTIONS_LIGHTPEN, gui->IsEnableLightpen());
#endif
#ifdef USE_MOUSE
	menuOptions->Check(ID_OPTIONS_MOUSE, gui->IsEnableMouse());
#endif
	menuOptions->Check(ID_OPTIONS_LOOSEN_KEY, gui->IsLoosenKeyStroke());
	menuOptions->Check(ID_OPTIONS_VKEYBOARD, gui->IsShownVirtualKeyboard());
#ifdef USE_DEBUGGER
	menuOptions->Enable(ID_OPEN_DEBUGGER0, !gui->IsDebuggerOpened());
#endif
}

void MyFrame::DecreaseMenuCount()
{
	if (now_updating_menu > 0) now_updating_menu--;
}

#ifdef USE_ONTIMER
void MyFrame::AdjustTimer()
{
	app->AdjustTimer();
}
#endif

void MyFrame::UpdateScreen()
{
//	if (now_expanding_menu) return;

#ifdef USE_OPENGL
	if (enable_opengl
#ifdef OPENGL_IMMCHANGE
		&& config.use_opengl != 0
#else
		&& emu && emu->now_use_opengl() != 0
#endif
	) {
		glcanvas->Refresh();
	} else
#endif /* USE_OPENGL */
	{
		panel->Refresh();
	}
//#if defined(__WXGTK__)
//	wxWindow::Refresh();
//#else
//	wxFrame::Refresh();
//#endif
}

void MyFrame::UpdateTitle(const void *result)
{
	const t_frame_count *frame_result = (const t_frame_count *)result;

	wxString buf = wxString::Format(wxT("%s - %d/%dfps"), wxT(DEVICE_NAME), frame_result->draw, frame_result->total);
	SetTitle(buf);
	need_update_title = 0;
}

/// change wxPanel and wxGLCanvas
void MyFrame::ChangePanel(int is_opengl)
{
#ifdef USE_OPENGL
	if (enable_opengl && is_opengl != 0) {
		if (panel) panel->Show(false);
		if (glcanvas) glcanvas->SetFocus();
	} else
#endif /* USE_OPENGL */
	{
#ifdef USE_OPENGL
		if (glcanvas) glcanvas->Show(false);
#endif /* USE_OPENGL */
		if (panel) panel->SetFocus();
	}
}

/// version information dialog
void MyFrame::OnHelpAbout(wxCommandEvent & WXUNUSED(event))
{
	gui->ShowAboutDialog();
}

/// set client size on MyPanel and MyFrame
void MyFrame::SetClientSize(int width, int height)
{
	wxWindow::SetClientSize(width, height);
	if (panel) panel->SetPanelSize(width, height);
#ifdef USE_OPENGL
	if (glcanvas) glcanvas->SetPanelSize(width, height);
#endif
}

/// set focus to panel
void MyFrame::SetFocus()
{
	wxFrame::SetFocus();

#ifdef USE_OPENGL
	if (enable_opengl && config.use_opengl != 0) {
		glcanvas->SetFocus();
	} else
#endif /* USE_OPENGL */
	{
		panel->SetFocus();
	}
}

/**
 * mouse events
 */
void MyFrame::OnMouseMotion(wxMouseEvent &event)
{
	if(IsFullScreen()) {
		wxPoint p = event.GetPosition();
		if(p.y == 0 && !now_showmenu) {
			SetMenuBar(menuBar);
//			long style = wxFULLSCREEN_ALL & ~wxFULLSCREEN_NOMENUBAR;
//			ShowFullScreen(true, style);
			now_showmenu = true;
		}
		else if(p.y > 32 && now_showmenu) {
			SetMenuBar(NULL);
//			long style = wxFULLSCREEN_ALL;
//			ShowFullScreen(true, style);
			now_showmenu = false;
		}
	}
	if (emu) {
		EMU_OSD *emu_osd = (EMU_OSD *)emu;
		emu_osd->update_mouse_event(event);
	}
}
void MyFrame::OnMouseDown(wxMouseEvent &event)
{
	if (emu) {
		EMU_OSD *emu_osd = (EMU_OSD *)emu;
		emu_osd->update_mouse_event(event);
	}
}

void MyFrame::GoUnfullscreen(int width, int height)
{
	if (IsFullScreen()) {
		// go window
		wxMenuBar *mb = GetMenuBar();
		if (mb == NULL) {
			SetMenuBar(menuBar);
		}
//		SetWindowStyleFlag(wxDEFAULT_FRAME_STYLE);
		wxTopLevelWindow::ShowFullScreen(false, 0);
	}
}

void MyFrame::GoFullscreen(int width, int height)
{
	if (!IsFullScreen()) {
		// go fullscreen
		SetMenuBar(NULL);
		now_showmenu = false;
//		SetWindowStyleFlag(0);
		wxTopLevelWindow::ShowFullScreen(true, wxFULLSCREEN_ALL);
	}
}

/**
 * keyboad events
 */
void MyFrame::OnCharHook(wxKeyEvent &event) {
//	int code = event.GetKeyCode();
//	int unicode = (int)event.GetUnicodeKey();
//	uint32_t rawcode = (uint32_t)event.GetRawKeyCode();
//	logging->out_debugf(_T("CharHook:code:0x%04x unicode:0x%04x rawcode:0x%04x"), code, unicode, rawcode);
	event.Skip();
}

void MyFrame::OnKeyDown(wxKeyEvent &event) {
//	int code = event.GetKeyCode();
//	int unicode = (int)event.GetUnicodeKey();
	short rawcode = (short)event.GetRawKeyCode();
	long  rawflag = (long)event.GetRawKeyFlags();
	emu->key_down_up(0, rawcode, (short)((rawflag & 0x1ff0000) >> 16));
}

void MyFrame::OnKeyUp(wxKeyEvent &event) {
//	int code = event.GetKeyCode();
//	int unicode = (int)event.GetUnicodeKey();
	short rawcode = (short)event.GetRawKeyCode();
	long  rawflag = (long)event.GetRawKeyFlags();
	emu->key_down_up(1, rawcode, (short)((rawflag & 0x1ff0000) >> 16));
}

/**
 *	open recent file
 */
void MyFrame::OpenRecentFile()
{
	wxString path;

	if (gui) {
		if (!app->tape_file.IsEmpty()) {
			path = app->tape_file;
			gui->PostEtLoadDataRecMessage(path);
		}
#ifdef USE_FD1
		// auto open recent file
		for(int drv=0; drv<2; drv++) {
			int bank_num = 0;
			path.Empty();
			if (!app->disk_file[drv].IsEmpty()) {
				path = app->disk_file[drv];
			}
			else if (config.recent_disk_path[drv].Count() > 0 && config.recent_disk_path[drv][0]->path.Length() > 0) {
				path = wxString(config.recent_disk_path[drv][0]->path);
				bank_num = config.recent_disk_path[drv][0]->num;
			}
			if (!path.IsEmpty()) {
				gui->PostEtOpenFloppyMessage(drv, path, bank_num, 0, false);
			}
		}
#endif
		if (!app->state_file.IsEmpty()) {
			path = app->state_file;
			gui->PostEtLoadStatusMessage(path);
		}
		if (!app->autokey_file.IsEmpty()) {
			path = app->autokey_file;
			gui->PostEtLoadAutoKeyMessage(path);
		}
		if (!app->reckey_file.IsEmpty()) {
			path = app->reckey_file;
			gui->PostEtLoadRecKeyMessage(path);
		}
	}

	// release buffer
	app->tape_file.Empty();
	app->disk_file[0].Empty();
	app->disk_file[1].Empty();
	app->state_file.Empty();
	app->autokey_file.Empty();
	app->reckey_file.Empty();
}

/*******************************************************
 * File Drag and Drop
 */
MyFileDropTarget::MyFileDropTarget(MyFrame *parent)
			: frame(parent)
{
}

bool MyFileDropTarget::OnDropFiles(wxCoord x, wxCoord y ,const wxArrayString &filenames)
{
	if (filenames.Count() > 0) {
		wxString name = filenames.Item(0);
		frame->OpenDroppedFile(name);
	}
    return true;
}
