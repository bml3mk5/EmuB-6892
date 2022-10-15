/** @file wx_about_dlg.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.09.01

	@brief [ about dialog ]
*/

#include <wx/wx.h>
#include "wx_about_dlg.h"
#include <SDL.h>
#include "../gui_base.h"
#include "../../vm/vm.h"
#include "../../version.h"
#if defined(_BML3MK5)
#include "../../res/common/bml3mk5.xpm"
#elif defined(_MBS1)
#include "../../res/common/mbs1.xpm"
#endif

MyAboutDialog::MyAboutDialog(wxWindow* parent, wxWindowID id, GUI_BASE *gui)
	: wxDialog(parent, id, _("About..."), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE)
{
	char edi[128];
	_TCHAR libver[_MAX_PATH];

	wxSizerFlags flags = wxSizerFlags().Expand().Border(wxALL, 4);

	wxBoxSizer *szrLeft   = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *szrRight  = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *szrMain   = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *szrAll    = new wxBoxSizer(wxVERTICAL);

	szrLeft->Add(new wxStaticBitmap(this, wxID_ANY,
#if defined(_BML3MK5)
		wxBitmap(bml3mk5_xpm)
#elif defined(_MBS1)
		wxBitmap(mbs1_xpm)
#endif
		, wxDefaultPosition, wxSize(64, 64)), flags);

	wxString str = wxT("");
	str += wxT(APP_NAME);
	str += wxT("\n\n");
	str += wxT("  Version: ");
	str += wxT(APP_VERSION);
	str += wxT(" \"");
	str += wxT(PLATFORM);
	str += wxT("\"");
#ifdef _DEBUG
	str += wxT(" (DEBUG Version)");
#endif
	str += wxT("\n");

	// edition
	emu->get_edition_string(edi, sizeof(edi));
	str += wxString(edi);

	// library
	gui->GetLibVersionString(libver);

	str += wxT("\n\n");
	str += wxString(libver);
	str += wxT("\n\n");
	str	+= wxT(APP_COPYRIGHT);

	szrRight->Add(new wxStaticText(this, wxID_ANY, str), flags);

	wxSizer *szrButtons = CreateButtonSizer(wxOK);
	szrMain->Add(szrLeft, flags);
	szrMain->Add(szrRight, flags);
	szrAll->Add(szrMain, flags);
	szrAll->Add(szrButtons, flags);

	SetSizerAndFit(szrAll);

//	wxAboutBox(info);
}

