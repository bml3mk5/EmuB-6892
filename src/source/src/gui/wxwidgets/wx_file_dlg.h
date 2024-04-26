/** @file wx_file_dlg.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.09.01

	@brief [ file dialog ]
*/

#ifndef WX_FILE_DLG_H
#define WX_FILE_DLG_H

#include <wx/dialog.h>
#include <wx/filedlg.h>
#include "../../msgs.h"

/**
	@brief File dialog box
*/
class MyFileDialog: public wxFileDialog
{
public:
	MyFileDialog(const wxString& message, const wxString& defaultDir, const wxString& defaultFile, const wxString& wildcard, long style = wxFD_DEFAULT_STYLE);
	MyFileDialog(const wxString& message, const wxString& defaultDir, const wxString& defaultFile, const CMsg::Id wildcard[], long style = wxFD_DEFAULT_STYLE);
	MyFileDialog(CMsg::Id messageid, const wxString& defaultDir, const wxString& defaultFile, const CMsg::Id wildcard[], long style = wxFD_DEFAULT_STYLE);
	MyFileDialog(const wxString& message, const wxString& defaultDir, const wxString& defaultFile, const char *wildcard, long style = wxFD_DEFAULT_STYLE);
	MyFileDialog(CMsg::Id messageid, const wxString& defaultDir, const wxString& defaultFile, const char *wildcard, long style = wxFD_DEFAULT_STYLE);

	void SetMessageById(CMsg::Id messageid);
	void SetWildcardByIdList(const CMsg::Id wildcard[]);
	void SetWildcardByString(const char *wildcard);
};

#endif /* WX_FILE_DLG_H */
