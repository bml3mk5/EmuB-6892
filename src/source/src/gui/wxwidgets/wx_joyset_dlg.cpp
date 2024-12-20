/** @file wx_joyset_dlg.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2023.01.08

	@brief [ wx_joyset_dlg ]
*/

#include <wx/wx.h>
#include <wx/statline.h>
#include "wx_joyset_dlg.h"
#include "../../emu.h"
#include "../../msgs.h"
#include "../../labels.h"
#include "../../keycode.h"
#include "../gui_keybinddata.h"
#include "../../utility.h"

// Attach Event
BEGIN_EVENT_TABLE(MyJoySettingDlg, wxDialog)
//	EVT_COMMAND_RANGE(IDC_SLIDER_0, IDC_SLIDER_99, wxEVT_SLIDER , MyJoySettingDlg::OnChangeValue)
//	EVT_NOTEBOOK_PAGE_CHANGED(IDC_NOTEBOOK, MyJoySettingDlg::OnPageChanged)
	EVT_COMMAND_RANGE(IDC_BUTTON_LOAD_DEFAULT0, IDC_BUTTON_LOAD_DEFAULT4, wxEVT_BUTTON, MyJoySettingDlg::OnLoadDefault)
	EVT_COMMAND_RANGE(IDC_BUTTON_LOAD_PRESET00, IDC_BUTTON_LOAD_PRESET44, wxEVT_BUTTON, MyJoySettingDlg::OnLoadPreset)
	EVT_COMMAND_RANGE(IDC_BUTTON_SAVE_PRESET00, IDC_BUTTON_SAVE_PRESET44, wxEVT_BUTTON, MyJoySettingDlg::OnSavePreset)
	EVT_COMMAND_RANGE(IDC_CHK_AXIS1, IDC_CHK_AXIS4, wxEVT_CHECKBOX, MyJoySettingDlg::OnClickAxis)
END_EVENT_TABLE()

MyJoySettingDlg::MyJoySettingDlg(wxWindow* parent, wxWindowID id, EMU *parent_emu, GUI_BASE *parent_gui)
	: MyDialog(parent, id, CMSG(Volume), parent_emu, parent_gui)
{
	joy_mask = ~0;
}

MyJoySettingDlg::~MyJoySettingDlg()
{
}

/**
 * create config dialog when ShowModal called
 */
void MyJoySettingDlg::InitDialog()
{
	_TCHAR str[64];

	wxSizerFlags flags = wxSizerFlags().Expand().Border(wxALL, 2);

	wxBoxSizer *szrAll = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *szrAllH = new wxBoxSizer(wxHORIZONTAL);
	szrAll->Add(szrAllH);

	wxBoxSizer *vbox = NULL;
	wxBoxSizer *hbox = NULL;

#if defined(USE_JOYSTICK) || defined(USE_KEY2JOYSTICK)
	wxSize sz_st(80, -1);
//	wxSize sz_s1(55, -1);
	wxSize sz_sl(100, -1);
	wxBoxSizer *szrMain = new wxBoxSizer(wxHORIZONTAL);
	for(int i=0; i<MAX_JOYSTICKS ; i++) {
		vbox = new wxBoxSizer(wxVERTICAL);
		szrMain->Add(vbox, flags);
		UTILITY::stprintf(str, 64, CMSGV(CMsg::JoypadVDIGIT), i + 1);
		vbox->Add(new wxStaticText(this, wxID_ANY, str, wxDefaultPosition, sz_st), flags);

		hbox = new wxBoxSizer(wxHORIZONTAL);
		vbox->Add(hbox, flags);
		hbox->Add(new MyStaticText(this, wxID_ANY, CMsg::Button_Mashing_Speed), flags);
		hbox->AddSpacer(32);
		hbox->Add(new wxStaticText(this, wxID_ANY, _T("0 <-> 3")), flags);

		for(int k=0; k<KEYBIND_JOY_BUTTONS; k++) {
			int kk = k + VM_JOY_LABEL_BUTTON_A;
			if (kk >= VM_JOY_LABELS_MAX) break;

			hbox = new wxBoxSizer(wxHORIZONTAL);
			vbox->Add(hbox, flags);
			CMsg::Id id = (CMsg::Id)cVmJoyLabels[kk].id;
			hbox->Add(new MyStaticText(this, wxID_ANY, id, wxDefaultPosition, sz_st, wxALIGN_CENTRE));
			int val = pConfig->joy_mashing[i][k];
			int n= i * KEYBIND_JOY_BUTTONS + k;
			wxSlider *sl = new wxSlider(this, IDC_SLIDER_JOY1 + n, val, 0, 3, wxDefaultPosition, sz_sl, wxSL_HORIZONTAL | wxSL_AUTOTICKS);
			sl->SetTickFreq(1);
			hbox->Add(sl);
		}

		hbox = new wxBoxSizer(wxHORIZONTAL);
		vbox->Add(hbox, flags);
		hbox->Add(new MyStaticText(this, wxID_ANY, CMsg::Analog_to_Digital_Sensitivity), flags);
		hbox->AddSpacer(32);
		hbox->Add(new wxStaticText(this, wxID_ANY, _T("0 <-> 10")), flags);

		for(int k=0; k<6; k++) {
			hbox = new wxBoxSizer(wxHORIZONTAL);
			vbox->Add(hbox, flags);
			CMsg::Id id = LABELS::joypad_axis[k];
			hbox->Add(new MyStaticText(this, wxID_ANY, id, wxDefaultPosition, sz_st, wxALIGN_CENTRE));
			int val = 10 - pConfig->joy_axis_threshold[i][k];
			int n= i * 6 + k;
			wxSlider *sl = new wxSlider(this, IDC_SLIDER_AXIS1 + n, val, 0, 10, wxDefaultPosition, sz_sl, wxSL_HORIZONTAL | wxSL_AUTOTICKS);
			sl->SetTickFreq(1);
			hbox->Add(sl);
		}
	}
	szrAllH->Add(szrMain, flags);

	//

	int tab_offset = KeybindData::JS_TABS_MIN;

	vbox = new wxBoxSizer(wxVERTICAL);

	notebook = new MyNotebook(this, IDC_NOTEBOOK, wxDefaultPosition, wxDefaultSize);
	for(int tab_num=tab_offset; tab_num<KeybindData::JS_TABS_MAX; tab_num++) {
		notebook->AddPageById(CreateBook(notebook, tab_num, tab_offset), LABELS::joysetting_tab[tab_num-tab_offset]);
	}
	vbox->Add(notebook, flags);

	MyCheckBox *chk;

#ifdef USE_PIAJOYSTICKBIT
	// check button
	chk = new MyCheckBox(this, IDC_CHK_PIAJOY_NEGATIVE, CMsg::Signals_are_negative_logic);
	chk->SetValue(FLG_PIAJOY_NEGATIVE != 0);
	vbox->Add(chk, flags);
	chk = new MyCheckBox(this, IDC_COMBO_PIAJOY_CONNTO, CMsg::Connect_to_standard_PIA_A_port);
	chk->SetValue(pConfig->piajoy_conn_to != 0);
	vbox->Add(chk, flags);
#else
	chk = new MyCheckBox(this, IDC_CHK_PIAJOY_NOIRQ, CMsg::No_interrupt_caused_by_pressing_the_button);
	chk->SetValue(FLG_PIAJOY_NOIRQ != 0);
	vbox->Add(chk, flags);
#endif

	// load and save buttons
	hbox = new wxBoxSizer(wxHORIZONTAL);
	wxButton *btn;
	btn = new MyButton(this, IDC_BUTTON_LOAD_DEFAULT0, CMsg::Load_Default);
	hbox->Add(btn, flags);
	vbox->Add(hbox, flags);

	wxString wstr;
	for(int i=0; i<KEYBIND_PRESETS; i++) {
		hbox = new wxBoxSizer(wxHORIZONTAL);
		wstr = wxString::Format(CMSG(Load_Preset_VDIGIT), i + 1);
		btn = new wxButton(this, IDC_BUTTON_LOAD_PRESET00 + i, wstr);
		hbox->Add(btn, flags);
		wstr = wxString::Format(CMSG(Save_Preset_VDIGIT), i + 1);
		btn = new wxButton(this, IDC_BUTTON_SAVE_PRESET00 + i, wstr);
		hbox->Add(btn, flags);
		vbox->Add(hbox, flags);
	}

	// axes of joypad
	hbox = new wxBoxSizer(wxHORIZONTAL);
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
	vbox->Add(hbox, flags);

	szrAllH->Add(vbox, flags);

#endif

	// button
	wxSizer *szrButtons = CreateButtonSizer(wxOK|wxCANCEL);
	szrAll->Add(szrButtons, flags);

	SetSizerAndFit(szrAll);
}

wxWindow *MyJoySettingDlg::CreateBook(wxWindow *parent, int tab, int)
{
	wxSizerFlags flags = wxSizerFlags().Expand().Border(wxALL, 2);

	wxPanel *page = new wxPanel(parent);

	wxBoxSizer *szrAll = new wxBoxSizer(wxVERTICAL);

	// table
	MyKeybindListWindow *ctrl = new MyKeybindListWindow(tab, page, IDC_LIST_0 + tab, wxDefaultPosition, wxSize(-1,260), wxBORDER_SIMPLE | wxVSCROLL);
	ctrl->SetJoyMaskPtr(&joy_mask);
	ctrls.push_back(ctrl);
	szrAll->Add(ctrl, flags);

#if 0
	// checkbox
	wxCheckBox *chk = ctrl->GetCombi();
	if (chk) {
		szrAll->Add(chk, flags);
	} else {
		szrAll->Add(new wxStaticText(page, wxID_ANY, wxEmptyString), flags);
	}
#endif

    page->SetSizer(szrAll);

	return page;
}

int MyJoySettingDlg::ShowModal()
{
	int rc = MyDialog::ShowModal();
	if (rc == wxID_OK) {
		SetData();
	}
	return rc;
}

void MyJoySettingDlg::SetData()
{
#if defined(USE_PIAJOYSTICK) || defined(USE_KEY2JOYSTICK)
	for(int i=0; i<MAX_JOYSTICKS ; i++) {
		for(int k=0; k<KEYBIND_JOY_BUTTONS; k++) {
			int id = IDC_SLIDER_JOY1 + i * KEYBIND_JOY_BUTTONS + k;
			wxSlider *sl = (wxSlider *)FindWindowById(id);
			pConfig->joy_mashing[i][k] = sl->GetValue();
		}
		for(int k=0; k<6; k++) {
			int id = IDC_SLIDER_AXIS1 + i * 6 + k;
			wxSlider *sl = (wxSlider *)FindWindowById(id);
			pConfig->joy_axis_threshold[i][k] = 10 - sl->GetValue();
		}
	}
	emu->modify_joy_mashing();
	emu->modify_joy_threshold();

	for(int tab=0; tab<(int)ctrls.size(); tab++) {
		ctrls[tab]->SetKeybindData();
	}
#endif
#ifdef USE_PIAJOYSTICKBIT
	// check button
	MyCheckBox *chk;
	chk = (MyCheckBox *)FindWindowById(IDC_CHK_PIAJOY_NEGATIVE);
	if (chk) {
		BIT_ONOFF(pConfig->misc_flags, MSK_PIAJOY_NEGATIVE, chk->GetValue());
	}
	chk = (MyCheckBox *)FindWindowById(IDC_COMBO_PIAJOY_CONNTO);
	if (chk) {
		pConfig->piajoy_conn_to = chk->GetValue() ? 1 : 0;
	}
#else
	MyCheckBox *chk;
	chk = (MyCheckBox *)FindWindowById(IDC_CHK_PIAJOY_NOIRQ);
	if (chk) {
		BIT_ONOFF(pConfig->misc_flags, MSK_PIAJOY_NOIRQ, chk->GetValue());
	}
#endif
}

/*
 * Event Handler
 */

#if 0
void MyJoySettingDlg::OnChangeValue(wxCommandEvent &event)
{
#if defined(USE_JOYSTICK) || defined(USE_KEY2JOYSTICK)
	int i = event.GetId() - IDC_SLIDER_0;
	int k = i % KEYBIND_JOY_BUTTONS;
	i /= KEYBIND_JOY_BUTTONS;
	wxSlider *sl = (wxSlider *)FindItem(event.GetId());
	pConfig->joy_mashing[i][k] = sl->GetValue();;
#endif
}
#endif

/// load keybind data to dialog from buffer
void MyJoySettingDlg::load_data(int tab, int num)
{
	ctrls[tab]->LoadPreset(num);
}

/// save keybind data to buffer from dialog
void MyJoySettingDlg::save_data(int tab, int num)
{
	ctrls[tab]->SavePreset(num);
}

bool MyJoySettingDlg::get_vmkeylabel(int code, wxString &label)
{
	_TCHAR clabel[128];
	bool rc = KeybindData::GetVmKeyLabel(code, clabel);
	label = wxString(clabel);
	return rc;
}

uint32_t MyJoySettingDlg::translate_vkkey(uint32_t code)
{
	return code;
}

bool MyJoySettingDlg::get_vkkeylabel(uint32_t code, wxString &label)
{
	_TCHAR clabel[128];
	bool rc = KeybindData::GetVkKeyLabel(code, clabel);
	label = wxString(clabel);
	return rc;
}

bool MyJoySettingDlg::get_vkjoylabel(uint32_t code, wxString &label)
{
	_TCHAR clabel[128];
	bool rc = KeybindData::GetVkJoyLabel(code, clabel);
	label = wxString(clabel);
	return rc;
}

/*
 * Event Handler
 */
//void MyJoySettingDlg::OnPageChanged(wxBookCtrlEvent &event) {
//	current_tab_num = event.GetSelection();
//}
void MyJoySettingDlg::OnLoadDefault(wxCommandEvent &event) {
	int id = event.GetId() - IDC_BUTTON_LOAD_DEFAULT0;
	int tab = notebook->GetSelection();
	if (tab < 0 || tab >= (int)ctrls.size()) return;
	load_data(tab, -1);
}
void MyJoySettingDlg::OnLoadPreset(wxCommandEvent &event) {
	int id = event.GetId() - IDC_BUTTON_LOAD_PRESET00;
	int num = id % KeybindData::TABS_MAX;
	int tab = notebook->GetSelection();
	if (tab < 0 || tab >= (int)ctrls.size()) return;
	load_data(tab, num);
}
void MyJoySettingDlg::OnSavePreset(wxCommandEvent &event) {
	int id = event.GetId() - IDC_BUTTON_SAVE_PRESET00;
	int num = id % KeybindData::TABS_MAX;
	int tab = notebook->GetSelection();
	if (tab < 0 || tab >= (int)ctrls.size()) return;
	save_data(tab, num);
}
void MyJoySettingDlg::OnClickAxis(wxCommandEvent &event) {
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

