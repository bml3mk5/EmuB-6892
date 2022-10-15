/** @file wxw_main.h

	Skelton for retropc emulator
	wxWidgets edition

	@author Sasaji
	@date   2012.6.30

	@brief [ wxw_main ]
*/

#ifndef WXW_MAIN_H
#define WXW_MAIN_H

#include "../../common.h"
#include <SDL.h>
//#include <wx/wx.h>
//#include <wx/dcbuffer.h>
#include <wx/panel.h>
#ifdef USE_OPENGL
#include <wx/glcanvas.h>
#endif
//#define _SDL_main main

#include "../../config.h"
#include "../../emu.h"

// signal to main loop
//extern SDL_cond *cond_allow_update_screen;

// frame rate
//extern const int fps[6];
//extern const int fskip[6];
//extern int rec_fps_no;

extern int need_update_title;

/// for calculate frame rate
typedef struct st_frame_count {
    int total;
    int draw;
    int skip;
} t_frame_count;

// Global Declarations
enum enumMyAppIds {
    IDF_FRAME = wxID_HIGHEST + 1,
    IDP_PANEL
};

class MyApp;
class MyFrame;
class MyPanel;
class MyGLCanvas;
class MySocket;
class MyConnection;
class MyThread;
class wxTimer;

/**
 * @brief Main application
 */
class MyApp : public wxApp {
    wxDECLARE_CLASS(MyApp);
    wxDECLARE_EVENT_TABLE();

private:
	MyThread *mThread;
#if defined(USE_ONIDLE) || defined(USE_ONTIMER)
	wxMutex *mux_allow_update_screen;
	wxCondition *cond_allow_update_screen;
#endif
#ifdef USE_ONTIMER
	wxTimer *mTimer;
#endif

	int    remain_time;

	t_frame_count main_frame;

	bool GetOptions();

	// event handler
#ifdef USE_ONIDLE
    void OnIdle(wxIdleEvent &);
#endif
#ifdef USE_ONTIMER
	void OnTimer(wxTimerEvent &);
	void UpdateFrame(int type);
#endif

public:
	MyApp();

	wxString app_path;
	wxString app_name;

	wxString ini_path;
	wxString ini_file;
	wxString res_path;
	wxString tape_file;
	wxString disk_file[2];
	wxString state_file;
	wxString autokey_file;
	wxString reckey_file;

	bool OnInit();
    int OnRun();
    int OnExit();

	void StopEmuThread();
#ifdef USE_ONTIMER
	void AdjustTimer();
#endif

	int  CheckSupportedFile(wxString &file_path);
	bool CheckFileExtension(const wxString &filename, const wxString &ext);
	void GetDirAndBaseName(const wxString &path, wxString &dir, wxString &name);
};

wxDECLARE_APP(MyApp);

/**
 * @brief Main window panel
 */
class MyPanel : public wxPanel {
    DECLARE_CLASS(MyPanel)
    DECLARE_EVENT_TABLE()

private:
	MyApp *app;
	MyFrame *frame;

	int window_width;
	int window_height;

//    void OnIdle(wxIdleEvent &);
	void OnKeyDown(wxKeyEvent &);
	void OnKeyUp(wxKeyEvent &);

	void OnPaint(wxPaintEvent &);
    void OnEraseBackground(wxEraseEvent &);
	void OnMouseMotion(wxMouseEvent &);
	void OnMouseDown(wxMouseEvent &);

//	void createScreen();

public:
    MyPanel(MyFrame *parent);
    ~MyPanel();

	void SetPanelSize(int width, int height);
	void SwapBuffers();


//	void SetGLCanvas(MyGLCanvas *p) { glcanvas = p; }
};

#ifdef USE_OPENGL
/**
 * @brief OpenGL canvas panel in window
 */
class MyGLCanvas: public wxGLCanvas
{
    DECLARE_CLASS(MyGLCanvas)
    DECLARE_EVENT_TABLE()

private:
	MyFrame *frame;
	wxGLContext* glcontext;
	bool initialized;
	bool needreset;

	void OnPaint(wxPaintEvent &);
	void OnSize(wxSizeEvent &);
	void OnKeyDown(wxKeyEvent &);
	void OnKeyUp(wxKeyEvent &);
	void OnMouseMotion(wxMouseEvent &);
	void OnMouseDown(wxMouseEvent &);

	void InitGL();
	void ResetProjectionMode();

public:
	MyGLCanvas(MyFrame *parent, wxWindowID id = wxID_ANY, const int* attrList = NULL,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize, long style = 0
	);
	~MyGLCanvas();
	void SetPanelSize(int width, int height);
	void RefreshPanel();
	bool Show(bool show = true);
};
#endif /* USE_OPENGL */

/**
 * @brief EMU thread class
 */
class MyThread : public wxThread
{
private:
	bool emu_thread_working;
#if defined(USE_ONIDLE) || defined(USE_ONTIMER)
	wxMutex *mux_need_update;
	wxCondition *cond_need_update;
#endif

    ExitCode Entry();
    MyApp *app;
public:
#if defined(USE_ONIDLE) || defined(USE_ONTIMER)
    MyThread(MyApp *parent, wxMutex *mutex, wxCondition *condition);
#else
    MyThread(MyApp *parent);
#endif
    ~MyThread();

	void Stop();
};


#endif /* WXW_MAIN_H */

