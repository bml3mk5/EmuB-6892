/** @file wx_file_dlg.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.09.01

	@brief [ file dialog ]
*/

//#include <wx/wx.h>
#include "wx_file_dlg.h"
#include "../../utility.h"

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

MyFileDialog::MyFileDialog(const wxString& message, const wxString& defaultDir, const wxString& defaultFile, const char *wildcard, long style)
            : wxFileDialog(NULL, wxEmptyString, defaultDir, defaultFile, wxEmptyString, style)
{
	SetWildcardByString(wildcard);
}

MyFileDialog::MyFileDialog(CMsg::Id messageid, const wxString& defaultDir, const wxString& defaultFile, const char *wildcard, long style)
            : wxFileDialog(NULL, wxEmptyString, defaultDir, defaultFile, wxEmptyString, style)
{
	SetMessageById(messageid);
	SetWildcardByString(wildcard);
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

/// @param[in] wildcard is such as "foo;bar;baz"
void MyFileDialog::SetWildcardByString(const char *wildcard)
{
	wxString all_cards;

	bool save = ((GetWindowStyleFlag() & wxFD_SAVE) != 0);
	int npos = 0;
	int nlen = (int)strlen(wildcard);
	char word[8];
	int word_len;
	int word_nums = 0;
	wxString exts;
	if (!save) {
		// load or open dialog
		do {
			word_len = 0;
			npos = UTILITY::get_token(wildcard, npos, nlen, word, (int)sizeof(word), ';', &word_len);
			if (word_len > 0) {
				if (word_nums > 0) {
					exts += wxT(";");
				}
				exts += wxT("*.");
				exts += wxString(word);
				word_nums++;
			}
		} while(npos >= 0);

		if (word_nums > 0) {
			all_cards += CMSG(Supported_Files);
			all_cards += wxT("(");
			all_cards += exts;
			all_cards += wxT(")|");
			all_cards += exts;
			all_cards += wxT("|");
		}

		all_cards += CMSG(All_Files);
		all_cards += wxT("(*.*)|*.*");

	} else {
		// save dialog
		do {
			word_len = 0;
			npos = UTILITY::get_token(wildcard, npos, nlen, word, (int)sizeof(word), ';', &word_len);
			if (word_len > 0) {
				exts = wxT("*.");
				exts += wxString(word);

				all_cards += wxString(word);
				all_cards += CMSG(File);
				all_cards += wxT("(");
				all_cards += exts;
				all_cards += wxT(")|");
				all_cards += exts;
				all_cards += wxT("|");

				word_nums++;
			}
		} while(npos >= 0);

		all_cards += CMSG(All_Files);
		all_cards += wxT("(*.*)|*.*");

	}

	SetWildcard(all_cards);
}
