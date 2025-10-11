/** @file wx_keybind_dlg.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2014.03.23

	@brief [ keybind dialog ]
*/

#ifndef WX_KEYBIND_DLG_H
#define WX_KEYBIND_DLG_H

#include "wx_dlg.h"
#include "wx_keybind_ctrl.h"
#include <wx/notebook.h>
#include <wx/scrolwin.h>
#include <wx/control.h>
#include <wx/textctrl.h>
#include "../gui_keybinddata.h"
#include <vector>

class MyFrame;
class MyKeybindDlg;
class MyKeybindPanel;
class MyKeybindListWindow;
class MyKeybindListCtrl;
class MyKeybindListEdit;

/**
	@brief keybind dialog
*/
class MyKeybindDlg : public MyKeybindBaseDlg
{
private:
	wxWindow *CreateBook(wxWindow *parent, int tab, int tab_offset);

	void InitDialog();
	void SetData();

public:
	MyKeybindDlg(MyFrame *, wxWindowID, EMU *, GUI_BASE *);
	~MyKeybindDlg();

	int ShowModal();

	enum {
		IDC_NOTEBOOK = 1,
		IDC_LIST_0,
	};

	DECLARE_EVENT_TABLE()
};

#endif /* WX_KEYBIND_DLG_H */
