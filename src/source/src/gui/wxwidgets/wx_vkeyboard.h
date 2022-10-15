/** @file wx_vkeyboard.h

	HITACHI BASIC MASTER LEVEL3 Mark5 Emulator 'EmuB-6892'
	Skelton for retropc emulator

	@author Sasaji
	@date   2017.02.21 -

	@brief [ virtual keyboard ]
*/

#ifndef WX_VKEYBOARD_H
#define WX_VKEYBOARD_H

//#include <wx/wx.h>
#include <wx/dialog.h>
#include "../../res/resource.h"
#include "../vkeyboard.h"

class MyVKeyboard;
class wxWindow;

namespace Vkbd {

/**
	@brief Virtual keyboard
*/
class VKeyboard : public Base
{
public:
	MyVKeyboard	*win;

private:
	wxWindow	*parent;

	void adjust_window_size();
//	void set_dist();
	void need_update_window(PressedInfo_t *, bool);
	void update_window();
//	void init_dialog(HWND);

public:
	VKeyboard(wxWindow *parent);
	~VKeyboard();

	void Create(const _TCHAR *res_path);
	void Show(bool = true);
	void Close();

	void paint_window(wxBitmap *, wxRect &);
};

} /* namespace Vkbd */

/**
	@brief Virtual keyboard window
*/
class MyVKeyboard : public wxDialog
{
private:
	Vkbd::VKeyboard *vkbd;
	wxBitmap *bmp;

	void OnPaint(wxPaintEvent &);
	void OnClose(wxCloseEvent &);
	void OnMouseDown(wxMouseEvent &); 
	void OnMouseUp(wxMouseEvent &); 

public:
	MyVKeyboard(wxWindow *parent, wxSize &sz, Vkbd::VKeyboard *vkbd);
	~MyVKeyboard();

	DECLARE_EVENT_TABLE()
};

#endif /* WX_VKEYBOARD_H */
