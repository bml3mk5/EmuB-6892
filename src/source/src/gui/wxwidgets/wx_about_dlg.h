/** @file wx_about_dlg.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.09.01

	@brief [ about dialog ]
*/

#ifndef WX_ABOUT_DLG_H
#define WX_ABOUT_DLG_H

//#include <wx/wx.h>
#include <wx/dialog.h>

class GUI_BASE;

/**
	@brief About dialog box
*/
class MyAboutDialog : public wxDialog
{
public:
	MyAboutDialog(wxWindow* parent, wxWindowID id, GUI_BASE *gui);
};

#endif /* WX_ABOUT_DLG_H */
