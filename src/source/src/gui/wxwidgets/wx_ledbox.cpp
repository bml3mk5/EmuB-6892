/** @file wx_ledbox.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2017.02.21 -

	@brief [ led box ]
*/

#include "wx_ledbox.h"

#ifdef WX_LEDBOX_H

#include <wx/wx.h>
#include "../../emu.h"
#include "../../utils.h"

extern EMU *emu;

//
// for wx widgets
//
LedBox::LedBox() : LedBoxBase()
{
	dlg = NULL;
	parent = NULL;
	pStart.x = 0;
	pStart.y = 0;
}

LedBox::~LedBox()
{
//	delete dlg;
}

void LedBox::show_dialog()
{
	if (dlg) {
		dlg->Show(visible && !inside);
	}
}

void LedBox::CreateDialogBox()
{
	if (!enable) return;

	dlg = new MyLedBox(parent, this);
	if (dlg) {
		adjust_dialog_size();
	}
}

void LedBox::SetParent(wxWindow *parent)
{
	this->parent = parent;
}

void LedBox::Move()
{
	if (dlg) {
		int x, y;
		parent->GetPosition(&x, &y);
		x += dist.x;
		y += dist.y;
		dlg->Move(x, y);
	}
}

void LedBox::move_in_place(int place)
{
	if (dlg) {
//		int rw, rh;
//		wxDisplaySize(&rw, &rh);
		int w, h;
		dlg->GetSize(&w, &h);
		int pw, ph;
		parent->GetSize(&pw, &ph);
		if (win_pt.place != place) {
			if (place & 1) {
				dist.x = pw - w;
			} else {
				dist.x = 0;
			}
			if (place & 2) {
				dist.y = ph - (place & 0x10 ? h : 4);
			} else {
				dist.y = 0 - (place & 0x10 ? 0 : h - 4);
			}
		}
		int x, y;
		parent->GetPosition(&x, &y);
		x += dist.x;
		y += dist.y;
//		if (y < 0) y = 0;
//		if (y + h > rh) y = rh - h;

		dlg->Move(x, y);
	}
}

#ifdef NO_TITLEBAR
void LedBox::mouse_press(int x, int y)
{
	pStart.x = x;
	pStart.y = y;
}

void LedBox::mouse_move(int x, int y)
{
	if (dlg) {
		int px;
		int py;
		parent->GetPosition(&px, &py);
		int wx;
		int wy;
		dlg->GetPosition(&wx, &wy);

		int nx = x - pStart.x + wx;
		int ny = y - pStart.y + wy;
		dist.x = wx - px;
		dist.y = wy - py;

		dlg->Move(nx, ny);
	}
}
#endif

#ifndef NO_TITLEBAR
void LedBox::set_dist()
{
	WINDOWINFO wi;
	WINDOWINFO wid;

	GetWindowInfo(hParent, &wi);
	GetWindowInfo(hDlg, &wid);
	dist.x = wid.rcWindow.left - wi.rcWindow.left;
	dist.y = wid.rcWindow.top - wi.rcWindow.top;
}
#endif

void LedBox::adjust_dialog_size()
{
	if (dlg) {
//		int w = surface->w;
//		int h = surface->h;

		dlg->SetClientSize(Width() + 2, Height() + 2);
	}
}

void LedBox::need_update_dialog()
{
	if (dlg) {
//		dlg->Update();
		dlg->Refresh();
	}
}

void LedBox::update_dialog(wxBitmap &bmp, const wxRect &re)
{
	if (!dlg) return;

	VmRectWH sre;
	sre.x = (re.GetLeft() > 0 ? re.GetLeft() : 0);
	sre.y = (re.GetTop() > 0 ? re.GetTop() : 0);
	sre.w = (re.GetWidth() < Width() ? re.GetWidth() : Width());
	sre.h = (re.GetHeight() < Height() ? re.GetHeight() : Height());

	Blit24(sre, bmp, sre);
//	SDL_UTILS::copy_surface(suf, &sre, bmp, sre);
}

// Attach Event
BEGIN_EVENT_TABLE(MyLedBox, wxDialog)
	EVT_PAINT(MyLedBox::OnPaint)
	EVT_LEFT_DOWN(MyLedBox::OnMouseDown)
	EVT_LEFT_UP(MyLedBox::OnMouseUp)
	EVT_MOTION(MyLedBox::OnMouseMove)
END_EVENT_TABLE()

MyLedBox::MyLedBox(wxWindow *parent, LedBox *ledbox)
	: wxDialog(parent, IDD_KBLEDBOX, wxT(""), wxDefaultPosition, wxDefaultSize, wxBORDER_NONE)
{
	this->ledbox = ledbox;
	bmp = new wxBitmap(ledbox->Width(), ledbox->Height(), 24);
	pressing = false;
}

MyLedBox::~MyLedBox()
{
	delete bmp;
}

bool MyLedBox::Show(bool show)
{
	bool rc = wxDialog::Show(show);
	if (m_parent) {
		m_parent->SetFocus();
	}
	return rc;
}

/// paint screen
void MyLedBox::OnPaint(wxPaintEvent &event)
{
	wxRegionIterator upd(GetUpdateRegion());
	while (upd) {
		ledbox->update_dialog(*bmp, upd.GetRect());
		upd++;
	}
	wxPaintDC dc(this);
	dc.DrawBitmap(*bmp, 1, 1);
	dc.SetPen(*wxBLACK_PEN);
	dc.SetBrush(*wxTRANSPARENT_BRUSH);
	dc.DrawRectangle(0, 0, bmp->GetWidth() + 2, bmp->GetHeight() + 2);
}

void MyLedBox::OnMouseDown(wxMouseEvent &event)
{
#ifdef NO_TITLEBAR
	ledbox->mouse_press(event.GetX(), event.GetY());
#endif
	pressing = true;
}

void MyLedBox::OnMouseUp(wxMouseEvent & WXUNUSED(event))
{
	pressing = false;
	if (m_parent) {
		m_parent->SetFocus();
	}
}

void MyLedBox::OnMouseMove(wxMouseEvent &event)
{
	if (pressing && event.Dragging()) {
#ifdef NO_TITLEBAR
		ledbox->mouse_move(event.GetX(), event.GetY());
#else
		ledbox->set_dist();
#endif
	}
}

#endif /* WX_LEDBOX_H */
