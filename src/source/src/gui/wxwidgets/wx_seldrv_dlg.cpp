/** @file wx_seldrv_dlg.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2016.11.20

	@brief [ wx_seldrv_dlg ]
*/

#include <wx/wx.h>
#include "wx_seldrv_dlg.h"
#include "../../common.h"
#include "../../emu.h"
#include "../gui.h"
#include "../../clocale.h"

// Attach Event
BEGIN_EVENT_TABLE(MySelDrvDlg, wxDialog)
	EVT_COMMAND_RANGE(IDC_BUTTON_DRIVE0, IDC_BUTTON_DRIVE7, wxEVT_BUTTON, MySelDrvDlg::OnSelectDrive)
END_EVENT_TABLE()

MySelDrvDlg::MySelDrvDlg(wxWindow* parent, wxWindowID id, EMU *parent_emu, GUI_BASE *parent_gui)
	: MyDialog(parent, id, _("Select Drive"), parent_emu, parent_gui)
{
}

MySelDrvDlg::~MySelDrvDlg()
{
}

/**
 * create config dialog when ShowModal called
 */
void MySelDrvDlg::InitDialog()
{
	wxSizerFlags flags = wxSizerFlags().Expand().Border(wxALL, 4);

	wxBoxSizer *szrAll = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *szrH = new wxBoxSizer(wxHORIZONTAL);

	// button
	wxButton *btn;
	for(int drv=0; drv<USE_FLOPPY_DISKS; drv++) {
		wxString str = wxString::Format(_T("%s%d"), prefix, drv);
		btn = new wxButton(this, IDC_BUTTON_DRIVE0 + drv, str);
		szrH->Add(btn, flags);
	}

	szrAll->Add(szrH, flags);

//	// button
//	wxSizer *szrButtons = CreateButtonSizer(wxOK|wxCANCEL);
//	szrAll->Add(szrButtons, flags);

	SetSizerAndFit(szrAll);
//	SetSizer(szrAll);
}

int MySelDrvDlg::ShowModal()
{
//	InitDialog();
	int rc = MyDialog::ShowModal();
	if (rc > IDC_BUTTON_DRIVE7) rc = -1;
	return rc;
}

void MySelDrvDlg::SetPrefix(const wxString &str)
{
	prefix = str;
}

/*
 * Event Handler
 */
void MySelDrvDlg::OnSelectDrive(wxCommandEvent &event)
{
	int id = event.GetId() - IDC_BUTTON_DRIVE0;

	EndModal(id);
}

