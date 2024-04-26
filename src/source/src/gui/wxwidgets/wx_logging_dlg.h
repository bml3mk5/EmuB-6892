/** @file wx_logging_dlg.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2024.04.13 -

	@brief [ wx_logging_dlg ]
*/

#ifndef WX_LOGGING_DLG_H
#define WX_LOGGING_DLG_H

#include "wx_dlg.h"
#include "../../config.h"

/**
	@brief Log dialog box
*/
class MyLoggingDlg : public MyDialog
{
private:
	wxTextCtrl *txtPath;
	wxTextCtrl *txtLog;
	wxButton *btnUpdate;
	wxButton *btnClose;

	wxSize m_client_size;
	bool m_initialized;

	TCHAR *p_buffer;
	int m_buffer_size;

	void InitDialog();

	void OnClose(wxCloseEvent &);
	void OnSize(wxSizeEvent &);
	void OnUpdateButton(wxCommandEvent &);

	void AdjustButtonPosition();

public:
	MyLoggingDlg(wxWindow *, wxWindowID, EMU *, GUI_BASE *);
	~MyLoggingDlg();

	DECLARE_EVENT_TABLE()
};

#endif /* WX_LOGGING_DLG_H */
