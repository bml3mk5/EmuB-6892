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
BEGIN_EVENT_TABLE(MyJoySettingDlg, MyKeybindBaseDlg)
END_EVENT_TABLE()

MyJoySettingDlg::MyJoySettingDlg(MyFrame *parent, wxWindowID id, EMU *parent_emu, GUI_BASE *parent_gui)
	: MyKeybindBaseDlg(parent, id, CMSG(Volume), parent_emu, parent_gui)
{
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
//		notebook->AddPageById(CreateBook(notebook, tab_num, tab_offset), LABELS::joysetting_tab[tab_num-tab_offset]);
		UTILITY::stprintf(str, sizeof(str) / sizeof(str[0]), _T("%d"), tab_num + 1 - tab_offset);
		notebook->AddPage(CreateBook(notebook, tab_num, tab_offset), str);
	}
	vbox->Add(notebook, flags);

//	MyCheckBox *chk;

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
	create_footer(vbox, flags);

	szrAllH->Add(vbox, flags);

#endif

	// button
	wxSizer *szrButtons = CreateButtonSizer(wxOK|wxCANCEL);
	szrAll->Add(szrButtons, flags);

	SetSizerAndFit(szrAll);
}

wxWindow *MyJoySettingDlg::CreateBook(wxWindow *parent, int tab, int tab_offset)
{
	wxSizerFlags flags = wxSizerFlags().Expand().Border(wxALL, 2);

	wxPanel *page = new wxPanel(parent);

	wxBoxSizer *szrAll = new wxBoxSizer(wxVERTICAL);

	// title
	MyStaticText *txt = new MyStaticText(page, wxID_ANY, LABELS::joysetting_tab[tab-tab_offset]);
	szrAll->Add(txt, flags);

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

	MyCheckBox *chk;

#if defined(USE_PIAJOYSTICK) || defined(USE_KEY2PIAJOYSTICK)
	if (tab == Keybind::TAB_JOY2JOY) {
# ifdef USE_JOYSTICKBIT
		// check button
		chk = new MyCheckBox(page, IDC_CHK_PIAJOY_NEGATIVE, CMsg::Signals_are_negative_logic);
		chk->SetValue(FLG_PIAJOY_NEGATIVE != 0);
		szrAll->Add(chk, flags);
		wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
		hbox->Add(new MyStaticText(page, wxID_ANY, CMsg::Connect_to_), flags);
		for(int i=0; LABELS::joysetting_opts[i] != CMsg::End; i++) {
			MyRadioButton *rad = new MyRadioButton(page, IDC_COMBO_PIAJOY_CONNTO + i, LABELS::joysetting_opts[i]);
			rad->SetValue(pConfig->piajoy_conn_to == i);
			hbox->Add(rad, flags);
		}
		szrAll->Add(hbox, flags);
# else
		chk = new MyCheckBox(page, IDC_CHK_PIAJOY_NOIRQ, CMsg::No_interrupt_caused_by_pressing_the_button);
		chk->SetValue(FLG_PIAJOY_NOIRQ != 0);
		szrAll->Add(chk, flags);
# endif
	}
#endif
#if defined(USE_PSGJOYSTICK) || defined(USE_KEY2PSGJOYSTICK)
	if (tab == Keybind::TAB_JOY2JOYB) {
# ifdef USE_JOYSTICKBIT
		// check button
		chk = new MyCheckBox(page, IDC_CHK_PSGJOY_NEGATIVE, CMsg::Signals_are_negative_logic);
		chk->SetValue(FLG_PSGJOY_NEGATIVE != 0);
		szrAll->Add(chk, flags);
# endif
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

	// check button
	MyCheckBox *chk = NULL;
#if defined(USE_PIAJOYSTICK) || defined(USE_KEY2PIAJOYSTICK)
# ifdef USE_JOYSTICKBIT
	chk = (MyCheckBox *)FindWindowById(IDC_CHK_PIAJOY_NEGATIVE);
	if (chk) {
		BIT_ONOFF(pConfig->misc_flags, MSK_PIAJOY_NEGATIVE, chk->GetValue());
	}
	for(int i=0; LABELS::joysetting_opts[i] != CMsg::End; i++) {
		MyRadioButton *rad = (MyRadioButton *)FindWindowById(IDC_COMBO_PIAJOY_CONNTO + i);
		if (rad->GetValue()) {
			pConfig->piajoy_conn_to = i;
			break;
		}
	}
# else
	chk = (MyCheckBox *)FindWindowById(IDC_CHK_PIAJOY_NOIRQ);
	if (chk) {
		BIT_ONOFF(pConfig->misc_flags, MSK_PIAJOY_NOIRQ, chk->GetValue());
	}
# endif
#endif
#if defined(USE_PSGJOYSTICK) || defined(USE_KEY2PSGJOYSTICK)
# ifdef USE_JOYSTICKBIT
	chk = (MyCheckBox *)FindWindowById(IDC_CHK_PSGJOY_NEGATIVE);
	if (chk) {
		BIT_ONOFF(pConfig->misc_flags, MSK_PSGJOY_NEGATIVE, chk->GetValue());
	}
# endif
#endif
}
