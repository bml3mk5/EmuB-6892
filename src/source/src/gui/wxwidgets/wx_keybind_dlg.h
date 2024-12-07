/** @file wx_keybind_dlg.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2014.03.23

	@brief [ keybind dialog ]
*/

#ifndef WX_KEYBIND_DLG_H
#define WX_KEYBIND_DLG_H

#include "wx_dlg.h"
#include <wx/notebook.h>
#include <wx/scrolwin.h>
#include <wx/control.h>
#include <wx/textctrl.h>
#include "../gui_keybinddata.h"
#include <vector>

class MyFrame;
class MyKeybindDlg;
class MyKeybindPanel;
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

	std::vector<MyKeybindListWindow *> ctrls;

//	uint32_t tm1;
	uint32_t joy_mask;

	MyNotebook *notebook;

	bool get_vmkeylabel(int code, wxString &label);
	bool get_vkkeylabel(uint32_t code, wxString &label);
	bool get_vkjoylabel(uint32_t code, wxString &label);
	uint32_t translate_vkkey(uint32_t code);

//	void click_ok(AG_Window *);
//	void click_cancel(AG_Window *);
//	void click_cell(AG_Table *, int, int);

	void load_data(int tab, int num);
	void save_data(int tab, int num);

	wxWindow *CreateBook(wxWindow *parent, int tab, int tab_offset);
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

	void OnClickAxis(wxCommandEvent &);

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

		IDC_BUTTON_LOAD_DEFAULT0 = 20,
		IDC_BUTTON_LOAD_DEFAULT4 = 24,

		IDC_BUTTON_LOAD_PRESET00 = 100,
		IDC_BUTTON_LOAD_PRESET44 = 144,

		IDC_BUTTON_SAVE_PRESET00 = 200,
		IDC_BUTTON_SAVE_PRESET44 = 244,

	};
	void SetTitleLabel(const _TCHAR *vm, const _TCHAR *vk, const _TCHAR *vj);

	MyFrame *GetFrame() { return frame; }
//	uint32_t GetJoyMask() { return joy_mask; }

	DECLARE_EVENT_TABLE()
};

#endif /* WX_KEYBIND_DLG_H */
