/** @file wx_file_dlg.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.09.01

	@brief [ file dialog ]
*/

//#include <wx/wx.h>
#include "wx_file_dlg.h"

MyFileDialog::MyFileDialog(const wxString& message, const wxString& defaultDir, const wxString& defaultFile, const wxString& wildcard, long style)
            : wxFileDialog(NULL, message, defaultDir, defaultFile, wildcard, style)
{
}

MyFileDialog::MyFileDialog(const wxString& message, const wxString& defaultDir, const wxString& defaultFile, const CMsg::Id wildcard[], long style)
            : wxFileDialog(NULL, message, defaultDir, defaultFile, wxEmptyString, style)
{
	SetWildcardByIdList(wildcard);
}

MyFileDialog::MyFileDialog(CMsg::Id messageid, const wxString& defaultDir, const wxString& defaultFile, const CMsg::Id wildcard[], long style)
            : wxFileDialog(NULL, wxEmptyString, defaultDir, defaultFile, wxEmptyString, style)
{
	SetMessageById(messageid);
	SetWildcardByIdList(wildcard);
}

void MyFileDialog::SetMessageById(CMsg::Id messageid)
{
	const _TCHAR *message = gMessages.Get(messageid);
	SetMessage(message);
}

void MyFileDialog::SetWildcardByIdList(const CMsg::Id wildcard[])
{
	wxString all_cards;

	for(int i=0; wildcard[i] != 0 && wildcard[i] != CMsg::End; i++) {
		wxString str = gMessages.Get(wildcard[i]);
		int spos = str.Find(wxT('('), true);
		if (spos < 0) continue;
		int epos = str.Find(wxT(')'), true);
		if (spos >= epos) continue;

		wxString sub = str.Mid(spos + 1, epos - spos - 1);
		str += wxT("|");
		str += sub;

		if (i > 0) {
			all_cards += wxT("|");
		}
		all_cards += str;
	}
	SetWildcard(all_cards);
}
