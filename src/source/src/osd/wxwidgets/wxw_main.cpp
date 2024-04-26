/** @file wxw_main.cpp

	Skelton for retropc emulator
	wxWidgets edition

	@author Sasaji
	@date   2012.6.30

	@brief [ wxw_main ]
*/

//#include <wx/wx.h>
#include <wx/cmdline.h>
#include <wx/rawbmp.h>
#include <wx/filename.h>
#include <wx/aboutdlg.h>
#include "wxw_main.h"
#include "wxw_emu.h"
#include "../../gui/gui.h"
#include "../../emumsg.h"
#include "../../clocale.h"
#ifdef USE_OPENGL
#include <wx/glcanvas.h>
#endif
#ifdef __WXMSW__
#include <imm.h>
#endif
#include "../../res/resource.h"
#if defined(__WXOSX__)
// not call SDL_main
#ifdef main
#undef main
#endif
#endif

#if defined(__WXMSW__) || defined(__WXGTK__)
wxIMPLEMENT_APP_NO_MAIN(MyApp);
#else
wxIMPLEMENT_APP(MyApp);
#endif
wxIMPLEMENT_CLASS(MyApp, wxApp)
wxIMPLEMENT_CLASS(MyPanel, wxPanel)
#ifdef USE_OPENGL
wxIMPLEMENT_CLASS(MyGLCanvas, wxGLCanvas)
#endif

#undef LOG_MEASURE

#define MIN_INTERVAL	4

#define CALC_NEXT_INTERVAL(now_ms, interval_ms, delta_ms) { \
	interval_ms = now_ms / FRAMES_PER_SEC; \
	now_ms = now_ms - (interval_ms * FRAMES_PER_SEC) + delta_ms; \
}

// variable

/// emulation core
EMU *emu = NULL;
GUI *gui = NULL;

int need_update_title = 0;

t_frame_count frames_result;

/// timing control
#define MAX_SKIP_FRAMES 10

// main

#if defined(__WXMSW__)
int main(int argc, char *argv[]) {
	return wxEntry(argc, argv);
}
#elif defined(__WXGTK__)
int main(int argc, char **argv) {
	// This application is implemented multi-threading code.
	// To use Xlib library on multi-thread, need invoke XInitThread() at first.
	// wxWidgets and GTK+ are not called.
	XInitThreads();
	return wxEntry(argc, argv);
}
#endif

/**
 * main application
 */
// attach events
BEGIN_EVENT_TABLE(MyApp, wxApp)
	//
#ifdef USE_ONIDLE
	EVT_IDLE(MyApp::OnIdle)
#endif

END_EVENT_TABLE()

//
MyApp::MyApp()
{
}

//
bool MyApp::OnInit() {
	SetAppName(_T(CONFIG_NAME));

	mThread = NULL;
#if defined(USE_ONIDLE) || defined(USE_ONTIMER)
	mux_allow_update_screen = NULL;
	cond_allow_update_screen = NULL;
#endif
#ifdef USE_ONTIMER
	mTimer = NULL;
#endif

	// get and parse options
	if (!wxApp::OnInit()) {
		return false;
	}
	if (!GetOptions()) {
		return false;
	}

	// logging
	logging = new Logging(ini_path);

	// set locale search path and catalog name
#if defined(__WXOSX__) || defined(__WXMAC__)
	clocale = new CLocale(res_path, wxT(CONFIG_NAME), wxT(""));
#else
	clocale = new CLocale(app_path, wxT(CONFIG_NAME), wxT(""));
#endif
#if 0
#if defined(__WXOSX__)
	mLocale.AddCatalogLookupPathPrefix(res_path + wxT("locale"));
#else
	mLocale.AddCatalogLookupPathPrefix(app_path + wxT("locale"));
#endif
	mLocale.AddCatalogLookupPathPrefix(wxT("locale"));
	mLocale.AddCatalog(wxT(CONFIG_NAME));
#endif
//	if (mLocale.IsAvailable(112)) {
//		int i = 112;
//	}
//	if (mLocale.IsLoaded(_T(CONFIG_NAME))) {
//		int i = 112;
//	}

#ifdef __WXMSW__
	// disable ime
//	ImmDisableIME(0);
#endif
//	ms = 1000;
//	CALC_NEXT_INTERVAL(ms, next_interval, 1000);

//	rec_next_time = 0;
//	rec_accum_time = 0;
//	memset(rec_delay, 0, sizeof(rec_delay));

//	total_frames = 0;
//	draw_frames = 0;
//	skip_frames = 0;
//	rec_delay_ptr = 0;

//	split_num = 0;

	memset(&main_frame, 0, sizeof(main_frame));

	// load image handler
	wxImageHandler *png = wxImage::FindHandler(wxBITMAP_TYPE_PNG);
	if (png == NULL) {
		png = new wxPNGHandler;
		wxImage::AddHandler(png);
	}

	// create a instance of emulation core
	emu = new EMU_OSD(app_path, ini_path, res_path);
	logging->set_receiver(emu);

	// load config
	pConfig = new Config;
	pConfig->load(ini_file);

	// change language if need
	clocale->ChangeLocaleIfNeed(pConfig->language);
	logging->out_logc(LOG_INFO, _T("Locale:["), clocale->GetLocaleName(), _T("] Lang:["), clocale->GetLanguageName(), _T("]"), NULL);

	// create a instance of GUI component
	gui = new GUI(argc, argv, emu);
	int rc = gui->Init();
	if (rc == -1) {
		rc = 1;
		return false;
	}
	emu->set_gui(gui);

	// init SDL
	uint32_t init_flags = SDL_INIT_AUDIO | SDL_INIT_TIMER;
#ifndef _DEBUG
	init_flags |= SDL_INIT_NOPARACHUTE;
#endif
	if(SDL_Init(init_flags)) {
		logging->out_logf(LOG_ERROR, _T("SDL_Init: %s."), SDL_GetError());
		return false;
	}

	// get display information
	emu->init_screen_mode();

	// set pixel format
	if (!emu->create_screen(0, pConfig->window_position_x, pConfig->window_position_y, MIN_WINDOW_WIDTH, MIN_WINDOW_HEIGHT, 0)) {
		logging->out_log(LOG_ERROR, _T("create_screen."));
		return false;
	}

    // initialization should always succeed
	return true;
}

int MyApp::OnRun() {
	// initialize emulation core
	emu->initialize();
	// restore screen mode
	if(pConfig->window_mode >= 0 && pConfig->window_mode < 8) {
		emu->change_screen_mode(pConfig->window_mode);
	}
	else if(pConfig->window_mode >= 8) {
		int prev_mode = pConfig->window_mode;
		pConfig->window_mode = 0;	// initialize window mode
		emu->change_screen_mode(prev_mode);
	}
	// use offscreen surface
	if (!emu->create_offlinesurface()) {
		return 1;
	}
//	// set again
//	emu->set_window(pConfig->window_mode, desktop_width, desktop_height);

    // generate an initial idle event to start things
	MyFrame *frame = gui->GetMyFrame();
	WakeUpIdle();

	// vm start
	if (emu) {
		emu->reset();
	}
	// open files
	frame->OpenRecentFile();

	// set default menu status
	frame->UpdateMenu();

#if defined(USE_ONIDLE) || defined(USE_ONTIMER)
	// init mutex
	mux_allow_update_screen = new wxMutex();
	cond_allow_update_screen = new wxCondition(*mux_allow_update_screen);

	gui->SetMutex(mux_allow_update_screen, cond_allow_update_screen);
#endif
#ifdef USE_ONTIMER
	mTimer = new wxTimer(this);
	if (mTimer) {
		int id = mTimer->GetId();
		Bind(wxEVT_TIMER, &MyApp::OnTimer, this, id);

		mTimer->Start(17);
	}
#endif

#if defined(USE_ONIDLE) || defined(USE_ONTIMER)
	mThread = new MyThread(this, mux_allow_update_screen, cond_allow_update_screen);
#else
	mThread = new MyThread(this);
#endif
	if (mThread->Run() != wxTHREAD_NO_ERROR) {
		return 1;
	}

//	wxSleep(1);

	// start the main loop
    return wxApp::OnRun();
}

void MyApp::StopEmuThread() {
	if (mThread && mThread->IsRunning()) {
		mThread->Stop();
		mThread->Wait();
	}
}

#ifdef USE_ONTIMER
void MyApp::AdjustTimer()
{
	if (mTimer) {
		mTimer->Stop();
		UpdateFrame(0);
		mTimer->Start();
	}
}
#endif

int MyApp::OnExit() {
	StopEmuThread();

#ifdef USE_ONTIMER
	delete mTimer;
#endif

#if defined(USE_ONIDLE) || defined(USE_ONTIMER)
	// delete mutex
	delete cond_allow_update_screen;
	delete mux_allow_update_screen;
#endif

	// release emulation core
	if(emu) {
		emu->release();
	}

	// save config
	pConfig->save();
	pConfig->release();

	delete pConfig;

	delete mThread;

	gui->CloseMyFrame();
	delete gui;
	logging->set_receiver(NULL);
	delete emu;

	// cleanup SDL
    SDL_Quit();

	delete clocale;
	delete logging;

    // return the standard exit code
	return wxApp::OnExit();
}

/**
 *	get and parse options
 */
bool MyApp::GetOptions()
{
	// set application path and name
	GetDirAndBaseName(argv[0], app_path, app_name);
	res_path = app_path + wxString(RESDIR);
#ifdef __WXOSX__
	// When mac, app_path specify app folder (ex. foo/bar/bml3mk5.app/Contents/MacOS/)
	wxFileName fapp_path(app_path + "../../../");
	fapp_path.MakeAbsolute();
	app_path = fapp_path.GetPath(wxPATH_GET_SEPARATOR);
#endif

//	wxString usage = wxString::Format(wxT("Usage: %s [-h] [-i <ini_file>] [-t <tape_file>] [-d <disk_file>] [-s <state_file>] [-a <autokey_file>] [<support_file> ...]"),app_name);
	// parse options
	wxCmdLineParser opts(argc, argv);
	wxString val;
//	opts.SetLogo(usage);
	opts.AddOption(wxT("i"), wxT("initial"), wxT("ini_file"), wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_PARAM_MULTIPLE);
	opts.AddOption(wxT("d"), wxT("disk"), wxT("disk_file"), wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_PARAM_MULTIPLE);
	opts.AddOption(wxT("t"), wxT("tape"), wxT("tape_file"), wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_PARAM_MULTIPLE);
	opts.AddOption(wxT("s"), wxT("state"), wxT("state_file"), wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_PARAM_MULTIPLE);
	opts.AddOption(wxT("a"), wxT("autokey"), wxT("autokey_file"), wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_PARAM_MULTIPLE);
	opts.AddOption(wxT("k"), wxT("reckey"), wxT("reckey_file"), wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_PARAM_MULTIPLE);
	opts.AddParam(wxT("support_file"), wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_PARAM_MULTIPLE);
	if (opts.Parse() < 0) {
		opts.Usage();
		return false;
	}
	if (opts.Found(wxT("i"),&val)) {
		ini_file = val;
	}
	if (opts.Found(wxT("d"),&val)) {
		disk_file[1] = disk_file[0];
		disk_file[0] = val;
	}
	if (opts.Found(wxT("t"),&val)) {
		tape_file = val;
	}
	if (opts.Found(wxT("s"),&val)) {
		state_file = val;
	}
	if (opts.Found(wxT("a"),&val)) {
		autokey_file = val;
	}
	if (opts.Found(wxT("k"),&val)) {
		reckey_file = val;
	}
	for(int i=0; i<(int)opts.GetParamCount(); i++) {
		val = opts.GetParam(i);
		switch(CheckSupportedFile(val)) {
		case 1:
			tape_file = val;
			break;
		case 2:
			disk_file[1] = disk_file[0];
			disk_file[0] = val;
			break;
		case 3:
			state_file = val;
			break;
		case 4:
			autokey_file = val;
			break;
		case 5:
			ini_file = val;
			break;
		case 6:
			reckey_file = val;
			break;
		default:
			break;
		}
	}

	// check ini file
	if (!ini_file.IsEmpty()) {
		if (CheckFileExtension(ini_file, wxT("ini"))) {
			wxString ini_name;
			GetDirAndBaseName(ini_file, ini_path, ini_name);
		}
	} else {
		ini_path = app_path;
		ini_file = app_path;
		ini_file += wxT(CONFIG_NAME);
		ini_file += wxT(".ini");
	}
	return true;
}

/**
 *	check supported file
 */
int MyApp::CheckSupportedFile(wxString &file_path)
{
	bool rc;

	rc = CheckFileExtension(file_path, wxT(".l3"))
	  || CheckFileExtension(file_path, wxT(".l3b"))
	  || CheckFileExtension(file_path, wxT(".l3c"))
	  || CheckFileExtension(file_path, wxT(".wav"))
	  || CheckFileExtension(file_path, wxT(".t9x"));
	if (rc) {
		// tape image
		return 1;
	}

#ifdef USE_FD1
	rc = CheckFileExtension(file_path, wxT(".d88"))
	  || CheckFileExtension(file_path, wxT(".td0"))
	  || CheckFileExtension(file_path, wxT(".imd"))
	  || CheckFileExtension(file_path, wxT(".dsk"))
	  || CheckFileExtension(file_path, wxT(".fdi"))
	  || CheckFileExtension(file_path, wxT(".hdm"))
	  || CheckFileExtension(file_path, wxT(".tfd"))
	  || CheckFileExtension(file_path, wxT(".xdf"))
	  || CheckFileExtension(file_path, wxT(".2d"))
	  || CheckFileExtension(file_path, wxT(".sf7"));
	if (rc) {
		// disk image
		return 2;
	}
#endif

#ifdef _BML3MK5
	rc = CheckFileExtension(file_path, wxT(".l3r"));
	if (rc) {
		// maybe state file
		return 3;
	}

	rc = CheckFileExtension(file_path, wxT(".txt"))
	  || CheckFileExtension(file_path, wxT(".bas"))
	  || CheckFileExtension(file_path, wxT(".lpt"));
	if (rc) {
		// maybe text file (for autokey)
		return 4;
	}

	rc = CheckFileExtension(file_path, wxT(".ini"));
	if (rc) {
		// maybe ini file
		return 5;
	}

	rc = CheckFileExtension(file_path, wxT(".l3k"));
	if (rc) {
		// maybe record key file
		return 6;
	}
#endif

	return 0;
}

/**
 *	check file extension
 */
bool MyApp::CheckFileExtension(const wxString &filename, const wxString &ext)
{
	wxString mext = wxT("*") + ext;
	if (filename.Upper().Matches(mext.Upper()) == true) {
		return true;
	}
	return false;
}

/**
 *	get dirname and basename
 */
void MyApp::GetDirAndBaseName(const wxString &path, wxString &dir, wxString &name)
{
	dir = wxFileName(path).GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR);
	name = wxFileName(path).GetFullName();
}


/*******************************************************
 * main window panel
 */
// attach event
BEGIN_EVENT_TABLE(MyPanel, wxPanel)
	EVT_PAINT(MyPanel::OnPaint)
	EVT_ERASE_BACKGROUND(MyPanel::OnEraseBackground)
//	EVT_CHAR_HOOK(MyPanel::OnKeyDown)
	EVT_KEY_DOWN(MyPanel::OnKeyDown)
	EVT_KEY_UP(MyPanel::OnKeyUp)
	EVT_MOTION(MyPanel::OnMouseMotion)
	EVT_LEFT_DOWN(MyPanel::OnMouseDown)
	EVT_LEFT_UP(MyPanel::OnMouseDown)
	EVT_MIDDLE_DOWN(MyPanel::OnMouseDown)
	EVT_MIDDLE_UP(MyPanel::OnMouseDown)
	EVT_RIGHT_DOWN(MyPanel::OnMouseDown)
	EVT_RIGHT_UP(MyPanel::OnMouseDown)
	EVT_MOUSE_AUX1_DOWN(MyPanel::OnMouseDown)
	EVT_MOUSE_AUX1_UP(MyPanel::OnMouseDown)
	EVT_MOUSE_AUX2_DOWN(MyPanel::OnMouseDown)
	EVT_MOUSE_AUX2_UP(MyPanel::OnMouseDown)
END_EVENT_TABLE()

MyPanel::MyPanel(MyFrame *parent) : wxPanel(parent, IDP_PANEL, wxDefaultPosition, wxDefaultSize, 0) {
	frame = parent;
	app = frame->GetApp();

#ifdef __WXMSW__
	// disable ime
	HWND hWnd = GetHWND();
	HIMC hImc = ImmAssociateContext(hWnd, NULL);
#endif

	// ensure the size of the wxPanel
	window_width = WINDOW_WIDTH;
	window_height = WINDOW_HEIGHT;

	// drag and drop
	SetDropTarget(new MyFileDropTarget(parent));
}

MyPanel::~MyPanel() {
}

void MyPanel::OnEraseBackground(wxEraseEvent & WXUNUSED(event))
{
	/* do nothing */
}

/// paint screen
void MyPanel::OnPaint(wxPaintEvent & WXUNUSED(event))
{
	if (emu) {
		EMU_OSD *emu_osd = (EMU_OSD *)emu;
		emu_osd->update_screen_pa(this);
	}
}

void MyPanel::OnKeyDown(wxKeyEvent &event)
{
	frame->OnKeyDown(event);
}

void MyPanel::OnKeyUp(wxKeyEvent &event)
{
	frame->OnKeyUp(event);
}

/// 
void MyPanel::OnMouseMotion(wxMouseEvent &event)
{
	frame->OnMouseMotion(event);
}

void MyPanel::OnMouseDown(wxMouseEvent &event)
{
	frame->OnMouseDown(event);
}

/// set size
void MyPanel::SetPanelSize(int width, int height)
{
    wxSize size(width, height);
	SetSize(size);
    SetMinSize(size);
    SetMaxSize(size);
	window_width = width;
	window_height = height;
}

#ifdef USE_OPENGL
/*******************************************************
 * panel for OpenGL
 */
// attach event
BEGIN_EVENT_TABLE(MyGLCanvas, wxGLCanvas)
	EVT_PAINT(MyGLCanvas::OnPaint)
//	EVT_CHAR_HOOK(MyGLCanvas::OnKeyDown)
	EVT_KEY_DOWN(MyGLCanvas::OnKeyDown)
	EVT_KEY_UP(MyGLCanvas::OnKeyUp)
	EVT_MOTION(MyGLCanvas::OnMouseMotion)
	EVT_LEFT_DOWN(MyGLCanvas::OnMouseDown)
	EVT_LEFT_UP(MyGLCanvas::OnMouseDown)
	EVT_MIDDLE_DOWN(MyGLCanvas::OnMouseDown)
	EVT_MIDDLE_UP(MyGLCanvas::OnMouseDown)
	EVT_RIGHT_DOWN(MyGLCanvas::OnMouseDown)
	EVT_RIGHT_UP(MyGLCanvas::OnMouseDown)
	EVT_MOUSE_AUX1_DOWN(MyGLCanvas::OnMouseDown)
	EVT_MOUSE_AUX1_UP(MyGLCanvas::OnMouseDown)
	EVT_MOUSE_AUX2_DOWN(MyGLCanvas::OnMouseDown)
	EVT_MOUSE_AUX2_UP(MyGLCanvas::OnMouseDown)
END_EVENT_TABLE()

MyGLCanvas::MyGLCanvas(MyFrame *parent, wxWindowID id, const int *attrList, const wxPoint &pos, const wxSize &size, long style)
	: wxGLCanvas(parent, id, attrList, pos, size, style)
{
	frame = parent;
	glcontext = new wxGLContext(this);

#ifdef __WXMSW__
	// disable ime
	HWND hWnd = GetHWND();
	HIMC hImc = ImmAssociateContext(hWnd, NULL);
#endif

	initialized = false;
	needreset = true;

	// drag and drop
	SetDropTarget(new MyFileDropTarget(parent));
}

MyGLCanvas::~MyGLCanvas()
{
	delete glcontext;
}

void MyGLCanvas::OnPaint(wxPaintEvent & WXUNUSED(event))
{
	// must always be here
//    wxPaintDC dc(this);

	if (emu && IsShownOnScreen()) {
		SetCurrent(*glcontext);

		if (!initialized) {
	        InitGL();
			initialized = true;
		}
		if (needreset) {
			ResetProjectionMode();
			needreset = false;
		}
		EMU_OSD *emu_osd = (EMU_OSD *)emu;
		emu_osd->update_screen_gl(this);
	}
}

void MyGLCanvas::OnSize(wxSizeEvent& WXUNUSED(event))
{
    ResetProjectionMode();
}

//extern void wglGetProcAddress(const char *);

void MyGLCanvas::InitGL()
{
	EMU_OSD *emu_osd = (EMU_OSD *)emu;
	emu_osd->realize_opengl(this);
}

void MyGLCanvas::ResetProjectionMode()
{
    if ( !IsShownOnScreen() )
        return;

    // This is normally only necessary if there is more than one wxGLCanvas
    // or more than one wxGLContext in the application.
    SetCurrent(*glcontext);

    int w, h;
    GetClientSize(&w, &h);

	EMU_OSD *emu_osd = (EMU_OSD *)emu;
	emu_osd->set_mode_opengl(this, w, h);

	// set swap interval 
	emu_osd->set_interval_opengl();
}

void MyGLCanvas::OnKeyDown(wxKeyEvent &event)
{
	frame->OnKeyDown(event);
}

void MyGLCanvas::OnKeyUp(wxKeyEvent &event)
{
	frame->OnKeyUp(event);
}
void MyGLCanvas::OnMouseMotion(wxMouseEvent &event)
{
	frame->OnMouseMotion(event);
}
void MyGLCanvas::OnMouseDown(wxMouseEvent &event)
{
	frame->OnMouseDown(event);
}

void MyGLCanvas::SetPanelSize(int width, int height)
{
	wxGLCanvas::SetClientSize(width, height);
	needreset = true;
}

void MyGLCanvas::RefreshPanel() {
	needreset = true;
}

bool MyGLCanvas::Show(bool show) {
	needreset = show;
	return wxGLCanvas::Show(show);
}
#endif /* USE_OPENGL */

#ifdef USE_ONIDLE
void MyApp::OnIdle(wxIdleEvent &)
{
//	Uint32 current_time = 0;
//	Uint32 next_time = 0;

	if (gui) {
//		current_time = wxGetLocalTimeMillis();

		gui->PreProcessEvent();

		gui->PostProcessEvent();

//		next_time = current_time + 16;

		mux_allow_update_screen->Lock();
		if (cond_allow_update_screen->WaitTimeout(17) != wxCOND_NO_ERROR) {
			gui->DecreaseUpdateScreenCount();
		}
		mux_allow_update_screen->Unlock();
		if (need_update_title > 0) {
			need_update_title = 0;
//			int ratio = 0;
//			if (total_frames) ratio = (int)(100.0 * (double)draw_frames / (double)total_frames + 0.5);
			wxString buf = wxString::Format(wxT("%s - %d/%dfps"), wxT(DEVICE_NAME), frames_result.draw, frames_result.total);
			gui->GetMyFrame()->SetTitle(buf);

//			update_fps_time += 1000;
//			if(update_fps_time <= current_time) {
//				update_fps_time = current_time + 1000;
//			}
//			total_frames = draw_frames = 0;
		}
		main_frame.total++;

	} else {
		wxMilliSleep(16);
	}
	wxWakeUpIdle();
}
#endif
#ifdef USE_ONTIMER
void MyApp::OnTimer(wxTimerEvent &)
{
	UpdateFrame(1);
}
void MyApp::UpdateFrame(int type)
{
	if (gui) {
		gui->PreProcessEvent();

		gui->PostProcessEvent();

		mux_allow_update_screen->Lock();
		gui->DecreaseUpdateScreenCount();
		mux_allow_update_screen->Unlock();

		if (need_update_title > 0) {
			need_update_title = 0;
			wxString buf = wxString::Format(wxT("%s - %d/%dfps"), wxT(DEVICE_NAME), frames_result.draw, frames_result.total);
			gui->GetMyFrame()->SetTitle(buf);
		}
		main_frame.total++;
	}
}
#endif

/**
 * EMU Thread Class
 *
 *  @attention called by another thread
 */
#if defined(USE_ONIDLE) || defined(USE_ONTIMER)
MyThread::MyThread(MyApp *parent, wxMutex *mutex, wxCondition *condition)
#else
MyThread::MyThread(MyApp *parent)
#endif
	: wxThread(wxTHREAD_JOINABLE)
{
	app = parent;
#if defined(USE_ONIDLE) || defined(USE_ONTIMER)
	mux_need_update = mutex;
	cond_need_update = condition;
#endif
	emu_thread_working = false;
}
MyThread::~MyThread()
{
	emu_thread_working = false;
}
/**
 *  emulator thread event loop
 */
wxThread::ExitCode MyThread::Entry()
{
	emu_thread_working = true;

	const int fskip[6] = {1, 2, 3, 4, 5, 6};
	int fskip_remain = 0;

	t_frame_count frames = { 0, 0, 0 };

	int ms = 1000;
	int next_interval;
	CALC_NEXT_INTERVAL(ms, next_interval, 1000);

	int split_num = 0;

	wxLongLong current_time = 0;
	int    remain_time = 0;
	wxLongLong ideal_next_time = 0;
	wxLongLong real_next_time = 0;
#ifdef LOG_MEASURE
	uint32_t skip_reason = 0;
#endif
#ifdef USE_PERFORMANCE_METER
	uint32_t lpCount1,lpCount2,lpCount3;
	lpCount1 = lpCount2 = lpCount3 = 0;
#endif

	// wait a sec.
	CDelay(500);

	wxLongLong update_fps_time = wxGetLocalTimeMillis() + 1000;

	// play sound
	emu->mute_sound(false);

	// process coming messages at first.
	while (emumsg.Process()) {}

	ideal_next_time = wxGetLocalTimeMillis();

#ifdef USE_PERFORMANCE_METER
	lpCount1 = ideal_next_time;
#endif

	// loop
	while (emu_thread_working) {
		split_num = 0;
		// get next period
		CALC_NEXT_INTERVAL(ms, next_interval, 1000);

//		start_time = next_time;
//		next_time += emu->now_skip() ? 0 : next_interval;
		ideal_next_time += next_interval;
		real_next_time += next_interval;
		current_time = wxGetLocalTimeMillis();
#ifdef LOG_MEASURE
		skip_reason = 0;
#endif

		// reset ideal time when proccesing is too late...
		if (current_time > ideal_next_time + 200) {
			ideal_next_time = current_time + next_interval;
#ifdef LOG_MEASURE
			skip_reason = 0x0200;
#endif
		}
		// sync next time when the real time have one frame difference from ideal time.
		if ((real_next_time > (next_interval + ideal_next_time)) || (ideal_next_time > (next_interval + real_next_time))) {
			real_next_time = ideal_next_time;
#ifdef LOG_MEASURE
			skip_reason = 0x0100;
#endif
		}
#ifdef LOG_MEASURE
		logging->out_logf(LOG_DEBUG, "A1: cur:%ld nt:%ld %ld ms:%d int:%d st:%04x", current_time, ideal_next_time, real_next_time, ms, next_interval, skip_reason);
#endif
#if (FRAME_SPLIT_NUM > 1)
		for(int i=0; i < (FRAME_SPLIT_NUM - 1); i++) {
			// drive machine
			if(emu)	emu->run(split_num);
			split_num++;
			while (emumsg.Process()) {}
		}
#ifdef LOG_MEASURE
		logging->out_logf(LOG_DEBUG, "A2: rti:%ld nt:%ld",wxGetLocalTimeMillis(),ideal_next_time);
#endif
#endif
		if(emu) {
			// drive machine
			emu->run(split_num);

			// record video
			emu->record_rec_video();

			frames.total++;

			if(pConfig->fps_no >= 0) {
				if (fskip_remain <= 0) {
					// constant frames per 1 second
					if (gui->NeedUpdateScreen()) {
						frames.draw++;
						frames.skip = 0;
					} else {
						frames.skip++;
					}
					fskip_remain = fskip[pConfig->fps_no];
#ifdef LOG_MEASURE
					skip_reason |= 0x11;
#endif
				} else {
					frames.skip++;
#ifdef LOG_MEASURE
					skip_reason |= 0x12;
#endif
				}
				if (fskip_remain > 0) fskip_remain--;
			}
			else {
				current_time = wxGetLocalTimeMillis();
				if(real_next_time > current_time) {
					// update window if enough time
					if (gui->NeedUpdateScreen()) {
						frames.draw++;
						frames.skip = 0;
					} else {
						frames.skip++;
					}
#ifdef LOG_MEASURE
					skip_reason |= 0x01;
#endif
				}
				else if(frames.skip > MAX_SKIP_FRAMES) {
					// update window at least once per 10 frames
					if (gui->NeedUpdateScreen()) {
						frames.draw++;
						frames.skip = 0;
					} else {
						frames.skip++;
					}
#ifdef LOG_MEASURE
					skip_reason |= 0x02;
#endif
//					real_next_time = wxGetLocalTimeMillis();
				}
				else {
					frames.skip++;
#ifdef LOG_MEASURE
					skip_reason |= 0x03;
#endif
				}
			}
			emu->skip_screen(frames.skip > 0);
		}
#ifdef LOG_MEASURE
		logging->out_logf(LOG_DEBUG, "A3: cur:%ld nt:%ld (total:%d fps:%d skip:%d) st:%04x",current_time,ideal_next_time,frames.total,frames.draw,frames.skip,skip_reason);
#endif
		gui->PostDrive();

		while (emumsg.Process()) {}
		if (frames.skip > 0) {
			real_next_time -= 2;
		}
		current_time = wxGetLocalTimeMillis();
#ifdef USE_PERFORMANCE_METER
		if (pConfig->show_pmeter) {
			lpCount2 = current_time;
		}
#endif
		remain_time = (int)(real_next_time - current_time).ToLong();
#if defined(__APPLE__) && defined(__MACH__)
		if (remain_time > 3) {
			CDelay(remain_time - 2);
		}
#else
		if (remain_time > 3) {
			CDelay(remain_time - 1);
		}
#endif
#ifdef LOG_MEASURE
		logging->out_logf(LOG_DEBUG, "A4: rti:%ld nt:%ld %ld fu:%ld",wxGetLocalTimeMillis(),ideal_next_time,real_next_time,update_fps_time);
#endif
		// calc frame rate
		current_time = wxGetLocalTimeMillis();
		if(update_fps_time <= current_time + 8) {
			frames_result = frames;
#if defined(USE_ONIDLE) || defined(USE_ONTIMER)
			need_update_title = 1;
#else
			if (need_update_title <= 0) {
				need_update_title = 5;
				gui->PostCommandMessage(ID_UPDATE_TITLE, &frames_result);
			} else {
				need_update_title--;
			}
#endif

			update_fps_time += 1000;
			if(update_fps_time <= current_time) {
				update_fps_time = current_time + 1000;
			}
			frames.total = frames.draw = 0;
		}
#ifdef USE_PERFORMANCE_METER
		if (pConfig->show_pmeter) {
			lpCount3 =  current_time;
			if (lpCount3 > lpCount1) {
				gdPMvalue = ((lpCount2 - lpCount1) * 100 / (lpCount3 - lpCount1)) & 0xfff;
			}
			lpCount1 = lpCount3;
		}
#endif
	}

	emu->release_on_emu_thread();

	return 0;
}

void MyThread::Stop()
{
	emu_thread_working = false;
}
