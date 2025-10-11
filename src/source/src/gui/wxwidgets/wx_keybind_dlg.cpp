/** @file wx_keybind_dlg.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2014.03.23

	@brief [ keybind dialog ]
*/

#include <wx/wx.h>
#include "wx_keybind_dlg.h"
#include "../../emu.h"
#include "../../config.h"
#include "../../labels.h"
#include "../../keycode.h"
#include "wx_gui.h"

// Attach Event
BEGIN_EVENT_TABLE(MyKeybindDlg, MyKeybindBaseDlg)
END_EVENT_TABLE()

MyKeybindDlg::MyKeybindDlg(MyFrame* parent, wxWindowID id, EMU *parent_emu, GUI_BASE *parent_gui)
	: MyKeybindBaseDlg(parent, id, CMSG(Keybind), parent_emu, parent_gui)
{
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
	create_footer(szrAll, flags);

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

int MyKeybindDlg::ShowModal()
{
	int rc = MyDialog::ShowModal();
	if (rc == wxID_OK) {
		SetData();
	}
	return rc;
}

void MyKeybindDlg::SetData()
{
	for(int tab=0; tab<(int)ctrls.size(); tab++) {
		ctrls[tab]->SetKeybindData();
	}
}
