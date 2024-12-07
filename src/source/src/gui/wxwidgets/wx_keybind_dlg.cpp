/** @file wx_keybind_dlg.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2014.03.23

	@brief [ keybind dialog ]
*/

#include <wx/wx.h>
#include "wx_keybind_dlg.h"
#include "wx_keybind_ctrl.h"
#include "../../emu.h"
#include "../../config.h"
#include "../../labels.h"
#include "../../keycode.h"
#include "wx_gui.h"

// Attach Event
BEGIN_EVENT_TABLE(MyKeybindDlg, wxDialog)
	EVT_COMMAND_RANGE(IDC_BUTTON_LOAD_DEFAULT0, IDC_BUTTON_LOAD_DEFAULT4, wxEVT_BUTTON, MyKeybindDlg::OnLoadDefault)
	EVT_COMMAND_RANGE(IDC_BUTTON_LOAD_PRESET00, IDC_BUTTON_LOAD_PRESET44, wxEVT_BUTTON, MyKeybindDlg::OnLoadPreset)
	EVT_COMMAND_RANGE(IDC_BUTTON_SAVE_PRESET00, IDC_BUTTON_SAVE_PRESET44, wxEVT_BUTTON, MyKeybindDlg::OnSavePreset)
	EVT_COMMAND_RANGE(IDC_CHK_AXIS1, IDC_CHK_AXIS4, wxEVT_CHECKBOX, MyKeybindDlg::OnClickAxis)
//	EVT_BUTTON(wxID_CANCEL, MyKeybindDlg::OnCancel)
//	EVT_KEY_DOWN(MyKeybindDlg::OnKeyDown)
END_EVENT_TABLE()

MyKeybindDlg::MyKeybindDlg(MyFrame* parent, wxWindowID id, EMU *parent_emu, GUI_BASE *parent_gui)
	: MyDialog(parent, id, CMSG(Keybind), parent_emu, parent_gui)
{
	frame = parent;

//	tm1 = 0;

	joy_mask = ~0;
}

MyKeybindDlg::~MyKeybindDlg()
{
}

/**
 * create config dialog when ShowModal called
 */
void MyKeybindDlg::InitDialog()
{
	wxSizerFlags flags = wxSizerFlags().Expand().Border(wxALL, 4);

	int tab_offset = KeybindData::KB_TABS_MIN;

	wxBoxSizer *szrAll = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *szrMain = new wxBoxSizer(wxHORIZONTAL);

	notebook = new MyNotebook(this, IDC_NOTEBOOK, wxDefaultPosition, wxDefaultSize);
	for(int tab_num=tab_offset; tab_num<KeybindData::KB_TABS_MAX; tab_num++) {
		notebook->AddPageById(CreateBook(notebook, tab_num, tab_offset), LABELS::keybind_tab[tab_num-tab_offset]);
	}
	szrMain->Add(notebook, flags);

	// load and save buttons
	wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
	wxButton *btn;
	btn = new MyButton(this, IDC_BUTTON_LOAD_DEFAULT0, CMsg::Load_Default);
	vbox->Add(btn, flags);
	vbox->Add(new wxStaticText(this, wxID_ANY, wxT(" ")), flags);
	for(int i=0; i<KEYBIND_PRESETS; i++) {
		wxString str = wxString::Format(CMSG(Load_Preset_VDIGIT), i + 1);
		btn = new wxButton(this, IDC_BUTTON_LOAD_PRESET00 + i, str);
		vbox->Add(btn, flags);
	}
	vbox->Add(new wxStaticText(this, wxID_ANY, wxT(" ")), flags);
	for(int i=0; i<KEYBIND_PRESETS; i++) {
		wxString str = wxString::Format(CMSG(Save_Preset_VDIGIT), i + 1);
		btn = new wxButton(this, IDC_BUTTON_SAVE_PRESET00 + i, str);
		vbox->Add(btn, flags);
	}
	szrMain->Add(vbox, flags);

	szrAll->Add(szrMain, flags);

	// axes of joypad
	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
	MyCheckBox *chk;
	chk = new MyCheckBox(this, IDC_CHK_AXIS1, CMsg::Enable_Z_axis);
	chk->SetValue((joy_mask & (JOYCODE_Z_LEFT | JOYCODE_Z_RIGHT)) != 0);
	hbox->Add(chk, flags);
	chk = new MyCheckBox(this, IDC_CHK_AXIS2, CMsg::Enable_R_axis);
	chk->SetValue((joy_mask & (JOYCODE_R_UP | JOYCODE_R_DOWN)) != 0);
	hbox->Add(chk, flags);
	chk = new MyCheckBox(this, IDC_CHK_AXIS3, CMsg::Enable_U_axis);
	chk->SetValue((joy_mask & (JOYCODE_U_LEFT | JOYCODE_U_RIGHT)) != 0);
	hbox->Add(chk, flags);
	chk = new MyCheckBox(this, IDC_CHK_AXIS4, CMsg::Enable_V_axis);
	chk->SetValue((joy_mask & (JOYCODE_V_UP | JOYCODE_V_DOWN)) != 0);
	hbox->Add(chk, flags);
	szrAll->Add(hbox, flags);

	// ok cancel button
	wxSizer *szrButtons = CreateButtonSizer(wxOK|wxCANCEL);
	szrAll->Add(szrButtons, flags);

	SetSizerAndFit(szrAll);
}

wxWindow *MyKeybindDlg::CreateBook(wxWindow *parent, int tab, int)
{
	wxSizerFlags flags = wxSizerFlags().Expand().Border(wxALL, 2);

	wxPanel *page = new wxPanel(parent);

	wxBoxSizer *szrAll = new wxBoxSizer(wxVERTICAL);

	// table
	MyKeybindListWindow *ctrl = new MyKeybindListWindow(tab, page, IDC_LIST_0 + tab, wxDefaultPosition, wxSize(-1,400), wxBORDER_SIMPLE | wxVSCROLL);
	ctrl->SetJoyMaskPtr(&joy_mask);
	ctrls.push_back(ctrl);
	szrAll->Add(ctrl, flags);

	wxCheckBox *chk = ctrl->GetCombi();
	if (chk) szrAll->Add(chk, flags);

    page->SetSizer(szrAll);

	return page;
}

void MyKeybindDlg::UpdateDialog()
{
}

int MyKeybindDlg::ShowModal()
{
//	Init_Dialog();
	int rc = MyDialog::ShowModal();
	if (rc == wxID_OK) {
		ModifyParam();
	}
	return rc;
}

void MyKeybindDlg::ModifyParam()
{
	for(int tab=0; tab<(int)ctrls.size(); tab++) {
		ctrls[tab]->SetKeybindData();
	}
}

void MyKeybindDlg::SetTitleLabel(const _TCHAR *vm, const _TCHAR *vk, const _TCHAR *vj)
{
}

/// load keybind data to dialog from buffer
void MyKeybindDlg::load_data(int tab, int num)
{
	ctrls[tab]->LoadPreset(num);
}

/// save keybind data to buffer from dialog
void MyKeybindDlg::save_data(int tab, int num)
{
	ctrls[tab]->SavePreset(num);
}

bool MyKeybindDlg::get_vmkeylabel(int code, wxString &label)
{
	_TCHAR clabel[128];
	bool rc = KeybindData::GetVmKeyLabel(code, clabel);
	label = wxString(clabel);
	return rc;
}

uint32_t MyKeybindDlg::translate_vkkey(uint32_t code)
{
	return code;
}

bool MyKeybindDlg::get_vkkeylabel(uint32_t code, wxString &label)
{
	_TCHAR clabel[128];
	bool rc = KeybindData::GetVkKeyLabel(code, clabel);
	label = wxString(clabel);
	return rc;
}

bool MyKeybindDlg::get_vkjoylabel(uint32_t code, wxString &label)
{
	_TCHAR clabel[128];
	bool rc = KeybindData::GetVkJoyLabel(code, clabel);
	label = wxString(clabel);
	return rc;
}

/*
 * Event Handler
 */
void MyKeybindDlg::OnLoadDefault(wxCommandEvent &event) {
//	int id = event.GetId() - IDC_BUTTON_LOAD_DEFAULT0;
//	int num = id % KeybindData::TABS_MAX;
	int tab = notebook->GetSelection();
	if (tab < 0 || tab >= (int)ctrls.size()) return;
	load_data(tab, -1);
}
void MyKeybindDlg::OnLoadPreset(wxCommandEvent &event) {
	int id = event.GetId() - IDC_BUTTON_LOAD_PRESET00;
	int num = id % KeybindData::TABS_MAX;
	int tab = notebook->GetSelection();
	if (tab < 0 || tab >= (int)ctrls.size()) return;
	load_data(tab, num);
}
void MyKeybindDlg::OnSavePreset(wxCommandEvent &event) {
	int id = event.GetId() - IDC_BUTTON_SAVE_PRESET00;
	int num = id % KeybindData::TABS_MAX;
	int tab = notebook->GetSelection();
	if (tab < 0 || tab >= (int)ctrls.size()) return;
	save_data(tab, num);
}
void MyKeybindDlg::OnClickAxis(wxCommandEvent &event) {
	int id = event.GetId();
	wxCheckBox *chk = (wxCheckBox *)FindWindowById(id);
	if (!chk) return;
	bool checked = chk->GetValue();
	uint32_t mask = 0;
	switch(id) {
	case IDC_CHK_AXIS1:
		mask = (JOYCODE_Z_LEFT | JOYCODE_Z_RIGHT);
		break;
	case IDC_CHK_AXIS2:
		mask = (JOYCODE_R_UP | JOYCODE_R_DOWN);
		break;
	case IDC_CHK_AXIS3:
		mask = (JOYCODE_U_LEFT | JOYCODE_U_RIGHT);
		break;
	case IDC_CHK_AXIS4:
		mask = (JOYCODE_V_UP | JOYCODE_V_DOWN);
		break;
	}
	BIT_ONOFF(joy_mask, mask, checked);
}

#if 0
void MyKeybindDlg::OnCancel(wxCommandEvent &event) {
	int id = event.GetId();
	int i= 0;
}

void MyKeybindDlg::OnKeyDown(wxKeyEvent &event)
{
	int code = event.GetKeyCode();
	wxChar unicode = event.GetUnicodeKey();
	wxUint32_t rawcode = event.GetRawKeyCode();
	frame->TranslateKeyCode(code, unicode, rawcode);


}
#endif
