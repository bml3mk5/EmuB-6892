/** @file wx_keybind_dlg.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2014.03.23

	@brief [ keybind dialog ]
*/

#ifndef WX_KEYBIND_DLG_H
#define WX_KEYBIND_DLG_H

#include "wx_dlg.h"
//#include "../../wxw_main.h"
#include <wx/notebook.h>
#include <wx/scrolwin.h>
#include <wx/control.h>
#include <wx/textctrl.h>
//#include <wx/dataview.h>
#include "../gui_keybinddata.h"

//#define KBCTRL_MAX_LINES 128
//#define KBCTRL_MAX_COLS  2

//#define MAX_TAB_NUM 2
//#define MAX_PRESET	4

#if 0
typedef struct codecols_st {
	int      tab;
	int      col;
	uint32_t  vk_keycode;
	uint32_t  vk_prev_keycode;
} codecols_t;

typedef struct codetable_st {
	int        row;
	codecols_t cols[KBCTRL_MAX_COLS];
	uint16_t    vm_keycode;
	bool       enabled;
} codetable_t;
#endif

class MyFrame;
class MyKeybindDlg;
class MyKeybindListWindow;
class MyKeybindListCtrl;
class MyKeybindListEdit;

/**
	@brief keybind dialog
*/
class MyKeybindDlg : public MyDialog
{
private:
	MyFrame *frame;
	KeybindData *kbdata[KEYBIND_MAX_NUM];

#if 0
	codetable_t table[KEYBIND_MAX_NUM][KBCTRL_MAX_LINES];
	int     vmkey_map_size[KEYBIND_MAX_NUM];
//	int		row2idx_map[KEYBIND_MAX_NUM][KBCTRL_MAX_LINES];

	uint32_t *vkkey_defmap[KEYBIND_MAX_NUM];
	int     vkkey_defmap_rows[KEYBIND_MAX_NUM];
	int     vkkey_defmap_cols[KEYBIND_MAX_NUM];

	uint32_t *vkkey_map[KEYBIND_MAX_NUM];
	uint32_t *vkkey_preset_map[KEYBIND_MAX_NUM][MAX_PRESET];
#endif

	MyKeybindListWindow *list[KEYBIND_MAX_NUM];
	wxCheckBox *chkCombi;

	uint32_t tm1;

	uint32_t *joy_stat;	// joystick #1, #2 (b0 = up, b1 = down, b2 = left, b3 = right, b4-b31 = trigger #1-#28

	bool get_vmkeylabel(int code, wxString &label);
	bool get_vkkeylabel(uint32_t code, wxString &label);
	bool get_vkjoylabel(uint32_t code, wxString &label);
	uint32_t translate_vkkey(uint32_t code);

//	void click_ok(AG_Window *);
//	void click_cancel(AG_Window *);
//	void click_cell(AG_Table *, int, int);

	void load_data(int tab, int num);
	void save_data(int tab, int num);

	wxWindow *CreateBook(wxWindow *parent, int tab);
//	wxWindow *CreateList(wxWindow *parent);
#if 0
	void update_key(AG_Table *, int, int, uint32_t);
	void update_joy();

	static void OnOk(AG_Event *);
	static void OnClose(AG_Event *);
	static void OnUpdate(AG_Event *);
	static void OnClickTab(AG_Event *);
	static void OnClickCell(AG_Event *);
	static void OnKeyDown(AG_Event *);
#endif
	void OnLoadDefault(wxCommandEvent &);
	void OnLoadPreset(wxCommandEvent &);
	void OnSavePreset(wxCommandEvent &);

//	void OnCancel(wxCommandEvent &);
//	void OnKeyDown(wxKeyEvent &);

	void InitDialog();
	void UpdateDialog();
	void ModifyParam();

public:
	MyKeybindDlg(MyFrame *, wxWindowID, EMU *, GUI_BASE *);
	~MyKeybindDlg();

	int ShowModal();

	enum {
		IDC_NOTEBOOK = 1,
		IDC_LIST_0,
		IDC_LIST_1,

		IDC_BUTTON_LOAD_DEFAULT0,
		IDC_BUTTON_LOAD_DEFAULT1,
		IDC_BUTTON_LOAD_DEFAULT2,

		IDC_BUTTON_LOAD_PRESET00,
		IDC_BUTTON_LOAD_PRESET01,
		IDC_BUTTON_LOAD_PRESET02,
		IDC_BUTTON_LOAD_PRESET10,
		IDC_BUTTON_LOAD_PRESET11,
		IDC_BUTTON_LOAD_PRESET12,
		IDC_BUTTON_LOAD_PRESET20,
		IDC_BUTTON_LOAD_PRESET21,
		IDC_BUTTON_LOAD_PRESET22,
		IDC_BUTTON_LOAD_PRESET30,
		IDC_BUTTON_LOAD_PRESET31,
		IDC_BUTTON_LOAD_PRESET32,

		IDC_BUTTON_SAVE_PRESET00,
		IDC_BUTTON_SAVE_PRESET01,
		IDC_BUTTON_SAVE_PRESET02,
		IDC_BUTTON_SAVE_PRESET10,
		IDC_BUTTON_SAVE_PRESET11,
		IDC_BUTTON_SAVE_PRESET12,
		IDC_BUTTON_SAVE_PRESET20,
		IDC_BUTTON_SAVE_PRESET21,
		IDC_BUTTON_SAVE_PRESET22,
		IDC_BUTTON_SAVE_PRESET30,
		IDC_BUTTON_SAVE_PRESET31,
		IDC_BUTTON_SAVE_PRESET32,

		IDC_CHECK_COMBI,
	};
//	void SetVmKeyMap(int tab_num, uint16_t *vmKeyMap, int size);
//	void SetVmKey(int tab_num, int idx, uint16_t code);
//	bool SetVmKeyCode(int tab_num, int idx, uint16_t code);

//	void SetVkKeyMap(int tab_num, uint32_t *vkKeyMap);
//	void SetVkKeyDefMap(int tab_num, uint32_t *vkKeyDefMap, int rows, int cols);
//	void SetVkKeyPresetMap(int tab_num, uint32_t *vkKeyMap, int idx);
//	bool SetVkKeyCode(codecols_t *obj, uint32_t code, wxString &label);
//	bool SetVkJoyCode(codecols_t *obj, uint32_t code, wxString &label);
	void SetTitleLabel(const _TCHAR *vm, const _TCHAR *vk, const _TCHAR *vj);

	MyFrame *GetFrame() { return frame; }
	uint32_t GetJoyStat(int num) { return joy_stat ? joy_stat[num] : 0; }

	DECLARE_EVENT_TABLE()
};

/**
	@brief Keybind table list
*/
class MyKeybindListWindow : public wxScrolledWindow
{
private:
	MyKeybindDlg *dlg;
	MyKeybindListCtrl *list;
	int tab;

//	void OnKeyDown(wxKeyEvent &);
public:
	MyKeybindListWindow(MyKeybindDlg *parent_dlg, int tab_no, wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size, int style);
	~MyKeybindListWindow();
	void InsertColumns(int cols, const wxString *labels);
	void AppendRow(int row, const wxString *labels, KeybindData *items);
	void UpdateItems();

	void SetScrollSize(int px, int py);

	MyKeybindDlg *GetDlg() { return dlg; }
};

/**
	@brief Keybind list control
*/
class MyKeybindListCtrl : public wxControl
{
private:
	MyKeybindDlg *dlg;
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

	MyKeybindDlg *GetDlg() { return dlg; }
	void SetEditPtr(MyKeybindListEdit *ctl) { now_textctl = ctl; }

//	DECLARE_EVENT_TABLE()
};

/**
	@brief Keybind key control in the list
*/
class MyKeybindListEdit : public wxTextCtrl
{
private:
	MyKeybindDlg *dlg;
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
};

#endif /* WX_KEYBIND_DLG_H */
