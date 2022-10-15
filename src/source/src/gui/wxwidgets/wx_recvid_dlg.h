/** @file wx_recvid_dlg.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.09.02

	@brief [ wx_recvid_dlg ]
*/

#ifndef WX_RECVID_DLG_H
#define WX_RECVID_DLG_H

#include "wx_dlg.h"
#include <wx/notebook.h>

/**
	@brief Record video dialog box
*/
class MyRecVideoDlg : public MyDialog
{
private:
	wxNotebook *book;
	wxArrayInt codnums;
	wxArrayInt quanums;
	wxArrayInt enables;

	void InitDialog();
	void UpdateDialog();
	void ModifyParam();

	void OnChangeBook(wxBookCtrlEvent &);
	void OnChangeCodec(wxCommandEvent &);
	void OnChangeQuality(wxCommandEvent &);

public:
	MyRecVideoDlg(wxWindow *, wxWindowID, EMU *, GUI_BASE *);
	~MyRecVideoDlg();

	int ShowModal();

	enum {
		IDC_NOTEBOOK = 1,
		IDC_CODEC = 11,
		IDC_QUALITY = 21,
	};

	DECLARE_EVENT_TABLE()
};

#endif /* WX_RECVID_DLG_H */
