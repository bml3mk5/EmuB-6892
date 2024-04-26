/** @file wx_keybind_ctrl.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2024.02.04

	@brief [ keybind control ]
*/

#ifndef WX_KEYBIND_CTRL_H
#define WX_KEYBIND_CTRL_H

#include "wx_dlg.h"
#include <wx/notebook.h>
#include <wx/scrolwin.h>
#include <wx/control.h>
#include <wx/textctrl.h>
#include "../gui_keybinddata.h"
#include <vector>

class MyFrame;
//class MyKeybindDlg;
class MyKeybindPanel;
class MyKeybindListWindow;
class MyKeybindListCtrl;
class MyKeybindListEdit;

/**
	@brief Keybind table list
*/
class MyKeybindListWindow : public wxScrolledWindow
{
private:
	int m_tab_num;
//	MyKeybindDlg *dlg;
	KeybindData *kbdata;
	MyKeybindListCtrl *ctrl;
	wxCheckBox *chkCombi;
	uint32_t *joy_mask;

//	void OnKeyDown(wxKeyEvent &);
public:
	MyKeybindListWindow(int tab_no, wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size, int style);
	~MyKeybindListWindow();
//	void InsertColumns(int cols, const wxString *labels);
//	void AppendRow(int row, const wxString *labels, KeybindData *items);
//	void UpdateItems();

	void SetScrollSize(int px, int py);

//	MyKeybindDlg *GetDlg() { return dlg; }
	wxCheckBox *GetCombi() { return chkCombi; }
	void SetKeybindData();

	void LoadPreset(int num);
	void SavePreset(int num);

	void SetJoyMaskPtr(uint32_t *mask) { joy_mask = mask; }
	uint32_t GetJoyMask() const { return *joy_mask; }
};

/**
	@brief Keybind list control
*/
class MyKeybindListCtrl : public wxControl
{
private:
//	MyKeybindDlg *dlg;
	MyKeybindListWindow *owner;
	int tab;

	wxSizerFlags flags;

	enum {
		IDC_TIMER = 1000
	};
	wxTimer *timer;

	wxSize btn_size0;
	wxSize btn_size;
	wxPoint now_pos;
	wxSize max_size;
	int border;
	int max_cols;
	int max_rows;

	int max_btn_ids;
	MyKeybindListEdit *now_textctl;

	wxVector<MyKeybindListEdit *> textctl;

	void OnPaint(wxPaintEvent &);
//	void OnKeyDown(wxKeyEvent &);
	void OnTimer(wxTimerEvent &);
//	void OnChildFocus(wxChildFocusEvent &);
public:
	MyKeybindListCtrl(MyKeybindListWindow *parent, int tab_no, wxWindowID id, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize, int style = 0);
	~MyKeybindListCtrl();

	void InsertColumns(int cols, const wxString *labels);
	void AppendRow(int row, const wxString *labels, KeybindData *items);
	void UpdateItems();

	void GetMaxSize(int &w, int &h);
	void SetMaxSize(int w, int h);

	virtual void ScrollWindow(int dx, int dy, const wxRect *rect);

//	MyKeybindDlg *GetDlg() { return dlg; }
	void SetEditPtr(MyKeybindListEdit *ctl) { now_textctl = ctl; }

	MyKeybindListEdit *GetCell(int row, int col);

	uint32_t *GetJoyStat(int num);
	uint32_t GetJoyMask() const;
//	DECLARE_EVENT_TABLE()
};

/**
	@brief Keybind key control in the list
*/
class MyKeybindListEdit : public wxTextCtrl
{
private:
//	MyKeybindDlg *dlg;
	MyKeybindListCtrl *owner;
	int tab_num;
	int row;
	int col;
	KeybindData *kbdata;

//	void OnKeyDown(wxKeyEvent &);
	void OnCharHook(wxKeyEvent &);
//	void OnTextEnter(wxCommandEvent &);
	void OnMouseLeftDClick(wxMouseEvent &);
//	void OnJoyButtonDown(wxJoystickEvent &);
//	void OnTimer(wxTimerEvent &);
	void OnSetFocus(wxFocusEvent &);
	void OnKillFocus(wxFocusEvent &);
public:
	MyKeybindListEdit(MyKeybindListCtrl *parent, wxWindowID id, const wxString &label, int tab_num, int row, int col, KeybindData *kbdata, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize, int style = 0);
	~MyKeybindListEdit();
	void UpdateItem();
	void SetJoyLabel();
	void SetKeyLabel(uint32_t code);
	void SetLabel(uint32_t code);
	void ClearLabel();
	void ClearCellByCode(uint32_t code);
};

#endif /* WX_KEYBIND_CTRL_H */
