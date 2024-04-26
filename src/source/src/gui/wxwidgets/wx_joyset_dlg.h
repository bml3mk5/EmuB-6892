/** @file wx_joyset_dlg.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2023.01.08

	@brief [ wx_joyset_dlg ]
*/

#ifndef WX_JOYSET_DLG_H
#define WX_JOYSET_DLG_H

#include "wx_dlg.h"
#include "wx_keybind_ctrl.h"
#include "../../config.h"

/**
	@brief Joypad setting dialog box
*/
class MyJoySettingDlg : public MyDialog
{
private:
	std::vector<MyKeybindListWindow *> ctrls;

	uint32_t joy_mask;

	void InitDialog();

	void SetData();
//	void OnChangeValue(wxCommandEvent &);

	bool get_vmkeylabel(int code, wxString &label);
	bool get_vkkeylabel(uint32_t code, wxString &label);
	bool get_vkjoylabel(uint32_t code, wxString &label);
	uint32_t translate_vkkey(uint32_t code);

	void load_data(int tab, int num);
	void save_data(int tab, int num);

	wxWindow *CreateBook(wxWindow *parent, int tab);

public:
	MyJoySettingDlg(wxWindow *, wxWindowID, EMU *, GUI_BASE *);
	~MyJoySettingDlg();

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

	void OnLoadDefault(wxCommandEvent &);
	void OnLoadPreset(wxCommandEvent &);
	void OnSavePreset(wxCommandEvent &);

	void OnClickAxis(wxCommandEvent &);

	DECLARE_EVENT_TABLE()
};

#endif /* WX_JOYSET_DLG_H */
