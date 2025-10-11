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
#include "../../labels.h"
#include "../../res/resource.h"

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
	wxSize sz(pSurface->Width(), pSurface->Height());
	win->SetClientSize(sz);
	sz.x = (int)(0.25 * sz.x + 0.5);
	sz.y = (int)(0.25 * sz.y + 0.5);
	win->SetMinClientSize(sz);
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

	wxRect re;
	re.x = (int)(magnify_x * info->re.left + 0.5);
	re.y = (int)(magnify_y * info->re.top + 0.5);
	re.width = (int)(magnify_x * (info->re.right - info->re.left) + 0.5);
	re.height = (int)(magnify_y * (info->re.bottom - info->re.top) + 0.5);

	win->RefreshRect(re);
}

void VKeyboard::update_window()
{
	if (!win) return;

	win->Update();
}

void VKeyboard::paint_window(wxBitmap *bmp, wxRect &re)
{
	if (!pSurface) return;

	VmRectWH s_re;
	s_re.x = (int)((double)re.x / magnify_x + 0.5);
	s_re.y = (int)((double)re.y / magnify_y + 0.5);
	s_re.w = (int)((double)re.width / magnify_x + 0.5);
	s_re.h = (int)((double)re.height / magnify_y + 0.5);
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

void VKeyboard::changing_size()
{
	int w = 1;
	int h = 1;
	win->GetClientSize(&w, &h);

	magnify_x = (double)w / pSurface->Width();
	magnify_y = (double)h / pSurface->Height();
}

void VKeyboard::change_size(double mag)
{
	magnify_x = mag;
	magnify_y = mag;
	int w = (int)(magnify_x * pSurface->Width() + 0.5);
	int h = (int)(magnify_y * pSurface->Height() + 0.5);
	win->SetClientSize(w, h);
}

} /* namespace Vkbd */

// Attach Event
BEGIN_EVENT_TABLE(MyVKeyboard, wxFrame)
	EVT_CLOSE(MyVKeyboard::OnClose)
	EVT_PAINT(MyVKeyboard::OnPaint)
	EVT_SIZE(MyVKeyboard::OnSize)
//	EVT_ERASE_BACKGROUND(MyVKeyboard::OnEraseBackground)
	EVT_CHAR_HOOK(MyVKeyboard::OnCharHook)
	EVT_KEY_DOWN(MyVKeyboard::OnKeyDown)
	EVT_KEY_UP(MyVKeyboard::OnKeyUp)
//	EVT_MOTION(MyPanel::OnMouseMotion)
	EVT_LEFT_DOWN(MyVKeyboard::OnMouseLeftDown)
	EVT_LEFT_UP(MyVKeyboard::OnMouseLeftUp)
//	EVT_MIDDLE_DOWN(MyPanel::OnMouseDown)
//	EVT_MIDDLE_UP(MyPanel::OnMouseDown)
	EVT_RIGHT_DOWN(MyVKeyboard::OnMouseRightDown)
//	EVT_RIGHT_UP(MyPanel::OnMouseDown)
//	EVT_MOUSE_AUX1_DOWN(MyPanel::OnMouseDown)
//	EVT_MOUSE_AUX1_UP(MyPanel::OnMouseDown)
//	EVT_MOUSE_AUX2_DOWN(MyPanel::OnMouseDown)
//	EVT_MOUSE_AUX2_UP(MyPanel::OnMouseDown)
	EVT_MENU_RANGE(ID_WINDOW_SIZE1, ID_WINDOW_SIZE4, MyVKeyboard::OnSelectMenu)
END_EVENT_TABLE()

MyVKeyboard::MyVKeyboard(wxWindow *parent, wxSize &sz, Vkbd::VKeyboard *vkbd)
	: wxFrame(parent, IDD_VKEYBOARD, _T("Virtual Keyboard"), wxDefaultPosition, sz,
		wxMINIMIZE_BOX | wxMAXIMIZE_BOX | wxCAPTION | wxCLOSE_BOX | wxCLIP_CHILDREN | wxRESIZE_BORDER
	)
{
	this->vkbd = vkbd;
	this->bmp = new wxBitmap(sz, 24);	// 24bpp
	this->popupMenu = NULL;
}

MyVKeyboard::~MyVKeyboard()
{
	delete popupMenu;
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
	int dst_w = 1;
	int dst_h = 1;
	this->GetClientSize(&dst_w, &dst_h);
	wxMemoryDC mdc(*bmp);
	wxPaintDC dc(this);
//	dc.DrawBitmap(*bmp, 0, 0);
	dc.StretchBlit(0, 0, dst_w, dst_h,
		&mdc, 0, 0, bmp->GetWidth(), bmp->GetHeight());
}

void MyVKeyboard::OnClose(wxCloseEvent &event)
{
	vkbd->Close();
}

void MyVKeyboard::OnMouseLeftDown(wxMouseEvent &event)
{
	vkbd->MouseDown(event.GetX(), event.GetY());
}

void MyVKeyboard::OnMouseLeftUp(wxMouseEvent & WXUNUSED(event))
{
	vkbd->MouseUp();
}

void MyVKeyboard::OnMouseRightDown(wxMouseEvent &event)
{
	show_popup_menu(event.GetX(), event.GetY());
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

void MyVKeyboard::OnSize(wxSizeEvent &event)
{
	vkbd->changing_size();
	Refresh();
}

void MyVKeyboard::OnSelectMenu(wxCommandEvent &event)
{
	int num = event.GetId() - ID_WINDOW_SIZE1;
	vkbd->change_size((double)LABELS::window_size[num].percent / 100.0);
}

void MyVKeyboard::create_popup_menu()
{
	if (popupMenu) return;

	popupMenu = new wxMenu();
	for(int i=0; LABELS::window_size[i].msg_id != CMsg::End; i++) {
		if (LABELS::window_size[i].msg_id != CMsg::Null) {
			popupMenu->Append(ID_WINDOW_SIZE1 + i, CMSGV(LABELS::window_size[i].msg_id)); 
		} else {
			popupMenu->AppendSeparator();
		}
	}
}

void MyVKeyboard::show_popup_menu(int x, int y)
{
	if (!popupMenu) {
		create_popup_menu();
	}

	PopupMenu(popupMenu, x, y);
}

#endif /* WX_VKEYBOARD_H */
