/** @file wx_seldrv_dlg.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2016.11.20

	@brief [ wx_seldrv_dlg ]
*/

#ifndef WX_SELDRV_DLG_H
#define WX_SELDRV_DLG_H

#include "wx_dlg.h"

/**
	@brief Select drive dialog box
*/
class MySelDrvDlg : public MyDialog
{
private:
	wxString prefix;

	void InitDialog();

	void OnSelectDrive(wxCommandEvent &);

public:
	MySelDrvDlg(wxWindow *, wxWindowID, EMU *, GUI_BASE *);
	~MySelDrvDlg();

	int ShowModal();
	void SetPrefix(const wxString &);

	enum {
		IDC_BUTTON_DRIVE0 = 1,
		IDC_BUTTON_DRIVE1,
		IDC_BUTTON_DRIVE2,
		IDC_BUTTON_DRIVE3,
		IDC_BUTTON_DRIVE4,
		IDC_BUTTON_DRIVE5,
		IDC_BUTTON_DRIVE6,
		IDC_BUTTON_DRIVE7,
	};

	DECLARE_EVENT_TABLE()
};

#endif /* WX_SELDRV_DLG_H */
