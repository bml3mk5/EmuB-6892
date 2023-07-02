/** @file wx_vkeyboard.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2017.02.21 -

	@brief [ virtual keyboard ]
*/

#include <wx/wx.h>
#include "wx_vkeyboard.h"

#ifdef WX_VKEYBOARD_H

#include "../../emu.h"
#include "../../utils.h"

extern EMU *emu;


namespace Vkbd {

//
// for wx widgets
//
VKeyboard::VKeyboard(wxWindow *parent) : OSDBase()
{
	this->parent = parent;
	this->win = NULL;
}

VKeyboard::~VKeyboard()
{
	delete win;
}

void VKeyboard::Show(bool show)
{
	if (!win) return;
	
	Base::Show(show);

	win->Show(show);
}

void VKeyboard::Create(const _TCHAR *res_path)
{
	if (win) return;

	load_bitmap(res_path);

	if (pSurface) {
		wxSize sz(pSurface->Width(), pSurface->Height());
		win = new MyVKeyboard(parent, sz, this);
		if (win) {
			adjust_window_size();
//			set_dist();
			closed = false;
		}
	}
}

void VKeyboard::Close()
{
//	if (win) {
//		delete win;
//		win = NULL;
//	}

	unload_bitmap();

	CloseBase();
}

void VKeyboard::adjust_window_size()
{
	if (!win) return;

	// calc client size in the window
	win->SetClientSize(pSurface->Width(), pSurface->Height());
}

#if 0
void VKeyboard::set_dist()
{
	if (!hVkbd || !hParent) return;

	WINDOWINFO wip;
	WINDOWINFO wi;

	HDC hdcScr = ::GetDC(NULL);
//	int desktop_width = ::GetDeviceCaps(hdcScr, HORZRES);
	int desktop_height = ::GetDeviceCaps(hdcScr, VERTRES);
	::ReleaseDC(NULL, hdcScr);

	::GetWindowInfo(hParent, &wip);
	::GetWindowInfo(hVkbd, &wi);

	int wp = wip.rcWindow.right - wip.rcWindow.left;
//	int hp = wip.rcWindow.bottom - wip.rcWindow.top;
	int w = wi.rcWindow.right - wi.rcWindow.left;
	int h = wi.rcWindow.bottom - wi.rcWindow.top;

	int x = (wp - w) / 2 + wip.rcWindow.left;
	int y = wip.rcWindow.bottom;

	if (y + h > desktop_height) {
		y = (desktop_height - h);
	}

	::SetWindowPos(hVkbd, HWND_TOPMOST, x, y, 1, 1, SWP_NOSIZE | SWP_NOZORDER);
}
#endif

void VKeyboard::need_update_window(PressedInfo_t *info, bool onoff)
{
	if (!win) return;

	need_update_window_base(info, onoff);

	win->RefreshRect(wxRect(info->re.left, info->re.top, info->re.right - info->re.left, info->re.bottom - info->re.top));
}

void VKeyboard::update_window()
{
	if (!win) return;

	win->Update();
}

void VKeyboard::paint_window(wxBitmap *bmp, wxRect &re)
{
	if (!pSurface) return;

	VmRectWH s_re = { re.x, re.y, re.width, re.height };
	pSurface->Blit(s_re, *bmp, s_re);
}

#if 0
void VKeyboard::init_dialog(HWND hDlg)
{
	// disable ime
	ImmAssociateContext(hDlg, NULL);

	// set icon on sysmenu
	HICON hIcon = ::LoadIconA(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	::SendMessageA(hDlg, WM_SETICON, NULL, (LPARAM)hIcon);

	// create bitmap buffer
	HDC hdc = ::GetDC(hDlg);
	hSufDC = ::CreateCompatibleDC(hdc);
	hSufBmp = ::CreateCompatibleBitmap(hdc, pSurface->w, pSurface->h);
	bmiSuf.bmiHeader.biWidth = pSurface->w;
	bmiSuf.bmiHeader.biHeight = - pSurface->h;	// flipped
	bmiSuf.bmiHeader.biPlanes = 1;
	bmiSuf.bmiHeader.biBitCount = pSurface->format->BitsPerPixel;
	bmiSuf.bmiHeader.biCompression = BI_RGB;
	bmiSuf.bmiHeader.biSizeImage = pSurface->w * pSurface->h * pSurface->format->BytesPerPixel;
	::SelectObject(hSufDC, hSufBmp);
	::ReleaseDC(hDlg, hdc);
}
#endif

} /* namespace Vkbd */

// Attach Event
BEGIN_EVENT_TABLE(MyVKeyboard, wxFrame)
	EVT_CLOSE(MyVKeyboard::OnClose)
	EVT_PAINT(MyVKeyboard::OnPaint)
//	EVT_ERASE_BACKGROUND(MyVKeyboard::OnEraseBackground)
	EVT_CHAR_HOOK(MyVKeyboard::OnCharHook)
	EVT_KEY_DOWN(MyVKeyboard::OnKeyDown)
	EVT_KEY_UP(MyVKeyboard::OnKeyUp)
//	EVT_MOTION(MyPanel::OnMouseMotion)
	EVT_LEFT_DOWN(MyVKeyboard::OnMouseDown)
	EVT_LEFT_UP(MyVKeyboard::OnMouseUp)
//	EVT_MIDDLE_DOWN(MyPanel::OnMouseDown)
//	EVT_MIDDLE_UP(MyPanel::OnMouseDown)
//	EVT_RIGHT_DOWN(MyPanel::OnMouseDown)
//	EVT_RIGHT_UP(MyPanel::OnMouseDown)
//	EVT_MOUSE_AUX1_DOWN(MyPanel::OnMouseDown)
//	EVT_MOUSE_AUX1_UP(MyPanel::OnMouseDown)
//	EVT_MOUSE_AUX2_DOWN(MyPanel::OnMouseDown)
//	EVT_MOUSE_AUX2_UP(MyPanel::OnMouseDown)
END_EVENT_TABLE()

MyVKeyboard::MyVKeyboard(wxWindow *parent, wxSize &sz, Vkbd::VKeyboard *vkbd)
	: wxFrame(parent, IDD_VKEYBOARD, _T("Virtual Keyboard"), wxDefaultPosition, sz,
		wxMINIMIZE_BOX | wxCAPTION | wxCLOSE_BOX | wxCLIP_CHILDREN
	)
{
	this->vkbd = vkbd;
	this->bmp = new wxBitmap(sz, 24);	// 24bpp
}

MyVKeyboard::~MyVKeyboard()
{
	delete bmp;
	vkbd->win = NULL;
}

/// paint screen
void MyVKeyboard::OnPaint(wxPaintEvent & WXUNUSED(event))
{
	wxRegionIterator upd(GetUpdateRegion());
	while (upd) {
		wxRect re = upd.GetRect();
		vkbd->paint_window(bmp, re);
		upd++;
	}
	wxPaintDC dc(this);

	dc.DrawBitmap(*bmp, 0, 0);
}

void MyVKeyboard::OnClose(wxCloseEvent &event)
{
	vkbd->Close();
}

void MyVKeyboard::OnMouseDown(wxMouseEvent &event)
{
	vkbd->MouseDown(event.GetX(), event.GetY());
}

void MyVKeyboard::OnMouseUp(wxMouseEvent & WXUNUSED(event))
{
	vkbd->MouseUp();
}

void MyVKeyboard::OnCharHook(wxKeyEvent &event)
{
	event.Skip();
}

void MyVKeyboard::OnKeyDown(wxKeyEvent &event)
{
	short rawcode = (short)event.GetRawKeyCode();
	long  rawflag = (long)event.GetRawKeyFlags();
	emu->key_down_up(0, rawcode, (short)((rawflag & 0x1ff0000) >> 16));
}

void MyVKeyboard::OnKeyUp(wxKeyEvent &event)
{
	short rawcode = (short)event.GetRawKeyCode();
	long  rawflag = (long)event.GetRawKeyFlags();
	emu->key_down_up(1, rawcode, (short)((rawflag & 0x1ff0000) >> 16));
}

#endif /* WX_VKEYBOARD_H */
