/** @file wx_joyset_dlg.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2023.01.08

	@brief [ wx_joyset_dlg ]
*/

#ifndef WX_JOYSET_DLG_H
#define WX_JOYSET_DLG_H

#include "wx_dlg.h"
#include "wx_keybind_ctrl.h"
#include "../../config.h"

/**
	@brief Joypad setting dialog box
*/
class MyJoySettingDlg : public MyKeybindBaseDlg
{
private:
	wxWindow *CreateBook(wxWindow *parent, int tab, int tab_offset);

	void InitDialog();
	void SetData();

public:
	MyJoySettingDlg(MyFrame *, wxWindowID, EMU *, GUI_BASE *);
	~MyJoySettingDlg();

	int ShowModal();

	enum {
		IDC_NOTEBOOK = 1,
		IDC_LIST_0,
	};

	DECLARE_EVENT_TABLE()
};

#endif /* WX_JOYSET_DLG_H */
