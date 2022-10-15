/** @file wx_ledbox.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2017.02.21 -

	@brief [ led box ]
*/

#ifndef WX_LEDBOX_H
#define WX_LEDBOX_H

#include <wx/dialog.h>
#include "../../res/resource.h"
#include "../ledbox.h"

#define NO_TITLEBAR

class MyLedBox;
class wxWindow;
class wxBitmap;

/**
	@brief LedBox
*/
class LedBox : public LedBoxBase
{
private:
	VmPoint pStart;
	MyLedBox *dlg;
	wxWindow *parent;

	void show_dialog();
	void move_in_place(int place);
	void need_update_dialog();

#ifdef NO_TITLEBAR
	void mouse_press(int x, int y);
	void mouse_move(int x, int y);
#else
	void set_dist();
#endif
	void adjust_dialog_size();
	void update_dialog(wxBitmap &bmp, const wxRect &re);

public:
	friend class MyLedBox;

	LedBox();
	~LedBox();

	void CreateDialogBox();
	void Move();
	void SetParent(wxWindow *parent);
};

/**
	@brief LedBox window
*/
class MyLedBox : public wxDialog
{
private:
	LedBox *ledbox;
	wxBitmap *bmp;
	bool pressing;

	void OnPaint(wxPaintEvent &);
	void OnClose(wxCloseEvent &);
	void OnMouseDown(wxMouseEvent &); 
	void OnMouseUp(wxMouseEvent &); 
	void OnMouseMove(wxMouseEvent &); 

public:
	MyLedBox(wxWindow *parent, LedBox *ledbox);
	~MyLedBox();

	// override
	bool Show(bool show);

	DECLARE_EVENT_TABLE()
};

#endif /* WX_LEDBOX_H */
