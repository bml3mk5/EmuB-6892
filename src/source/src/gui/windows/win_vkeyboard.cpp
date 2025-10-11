/** @file win_vkeyboard.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2016.12.21 -

	@brief [ virtual keyboard ]
*/

#include "win_vkeyboard.h"
#include "../../osd/windows/win_apiex.h"
#include "../../logging.h"
#include "win_gui.h"

#ifdef _MSC_VER
#pragma comment(lib, "imm32.lib")
#endif

#include "../../emu.h"
#include "../../config.h"
#include "../../msgs.h"
#include "../../labels.h"
#include "../../res/resource.h"

#if defined(USE_SDL2)
#include "win_key_trans.h"
#endif

extern EMU *emu;

#define USE_VKEYBOARD_UPDATE_RECT

namespace Vkbd {

//
// for windows
//
VKeyboard::VKeyboard(HINSTANCE hInst, HWND hWnd) : OSDBase()
{
	hInstance = hInst;
	hParent = hWnd;
	hVkbd = NULL;
#if defined(USE_SDL) || defined(USE_SDL2)
	hSufDC = NULL;
	hSufBmp = NULL;
	memset(&bmiSuf, 0, sizeof(bmiSuf));
	bmiSuf.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
#endif
	hPopupMenu = NULL;
	RECT_IN(mSufRect, 0, 0, 0, 0);
	SIZE_IN(mOriginSize, 0, 0);
	RECT_IN(mClientSize, 0, 0, 0, 0);
	SIZE_IN(mWindowSize, 0, 0);
	SIZE_IN(mMarginSize, 0, 0);
	mUserMagnifyX = 1.0;
	mUserMagnifyY = 1.0;
	mSystemMagnifyX = 1.0;
	mSystemMagnifyY = 1.0;
	mSizeType = 0;
}

VKeyboard::~VKeyboard()
{
}

void VKeyboard::Show(bool show)
{
	if (!hVkbd) return;

	Base::Show(show);
	::ShowWindow(hVkbd, show ? SW_SHOWNORMAL : SW_HIDE);
}

#if defined(USE_WIN)
bool VKeyboard::Create()
{
	if (hVkbd) return true;

	hVkbd = ::CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_VKEYBOARD), hParent, Proc, (LPARAM)this);
	if (hVkbd) {
		load_bitmap();
		create_surface();
		create_d2d_surface();
		adjust_window_size();
		set_dist();
		closed = false;
	}
	return true;
}
#elif defined(USE_SDL) || defined(USE_SDL2)
bool VKeyboard::Create(const _TCHAR *res_path)
{
	if (hVkbd) return true;

	if (!load_bitmap(res_path)) {
		closed = true;
		return false;
	}

	hVkbd = ::CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_VKEYBOARD), hParent, Proc, (LPARAM)this);
	if (hVkbd) {
		create_d2d_surface();
		adjust_window_size();
		set_dist();
		closed = false;
	}
	return true;
}
#endif

void VKeyboard::Close()
{
	if (!hVkbd) return;

#if defined(USE_SDL) || defined(USE_SDL2)
	if (hSufBmp) {
		::DeleteObject(hSufBmp);
		hSufBmp = NULL;
	}
	if (hSufDC) {
		::ReleaseDC(hVkbd, hSufDC);
	}
#endif

	::EndDialog(hVkbd, 0);
	hVkbd = NULL;

	if (hPopupMenu) {
		::DestroyMenu(hPopupMenu);
		hPopupMenu = NULL;
	}

	release_d2d_surface();

	unload_bitmap();

	CloseBase();
}

INT_PTR CALLBACK VKeyboard::Proc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	PAINTSTRUCT ps;
	HDC hdc;
	VKeyboard *myObj = (VKeyboard *)GetWindowLongPtr(hDlg, DWLP_USER);

	switch (message)
	{
	case WM_PAINT:
		hdc = BeginPaint(hDlg, &ps);
		myObj->paint_window(hdc, &ps.rcPaint);
		EndPaint(hDlg, &ps);
		return (INT_PTR)FALSE;
	case WM_INITDIALOG:
		SetWindowLongPtr(hDlg, DWLP_USER, lParam);
		myObj = (VKeyboard *)lParam;
		myObj->init_dialog(hDlg);
		return (INT_PTR)TRUE;
	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
		myObj->MouseDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return (INT_PTR)FALSE;
	case WM_LBUTTONUP:
		myObj->MouseUp();
		return (INT_PTR)FALSE;
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
#ifdef USE_ALT_F10_KEY
		return (INT_PTR)FALSE;	// not activate menu when hit ALT/F10
#endif
	case WM_KEYDOWN:
	case WM_KEYUP:
		if (emu) {
#if defined(USE_WIN)
			emu->key_down_up(message & 1, LOBYTE(wParam), (long)lParam);
#elif defined(USE_SDL)
			long scan_code = (long)((lParam & 0x1ff0000) >> 16);
			emu->key_down_up(message & 1, LOBYTE(wParam), scan_code);
#elif defined(USE_SDL2)
			UINT32 scan_code = GUI_WIN::translate_to_sdlkey(wParam, lParam);
			emu->key_down_up(message & 1, (int)wParam, (short)scan_code);
#endif
		}
		return (INT_PTR)FALSE;
	case WM_COMMAND:
		return (INT_PTR)myObj->process_command(LOWORD(wParam));
	case WM_CLOSE:
		myObj->Close();
		return (INT_PTR)TRUE;
	case WM_MENUCHAR:
		// ignore accel key and suppress beep
		::SetWindowLongPtr(hDlg, DWLP_MSGRESULT, MNC_CLOSE << 16);
		return (INT_PTR)TRUE;
	case WM_RBUTTONUP:
		myObj->show_popup_menu(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return (INT_PTR)FALSE;
	case WM_SIZING:
		myObj->changing_size((int)wParam, (RECT *)lParam);
		return (INT_PTR)TRUE;
	case WM_SIZE:
		myObj->size_changed((int)wParam, (int)LOWORD(lParam), (int)HIWORD(lParam));
		return (INT_PTR)TRUE;
	case WM_DPICHANGED:
		myObj->dpi_changed((int)LOWORD(wParam), (int)HIWORD(wParam));
		return (INT_PTR)TRUE;
//	case WM_DESTROY:
//		break;
	}
	return (INT_PTR)FALSE;
}

void VKeyboard::calc_window_size(WINDOWINFO &wi)
{
	magnify_x = mSystemMagnifyX * mUserMagnifyX;
	magnify_y = mSystemMagnifyY * mUserMagnifyY;

	// calc client size in the window
	GetWindowInfo(hVkbd, &wi);
	mSufRect.w = pSurface->Width();
	mSufRect.h = pSurface->Height();
	mOriginSize.w = (int)((double)mSufRect.w * mSystemMagnifyX + 0.5);
	mOriginSize.h = (int)((double)mSufRect.h * mSystemMagnifyY + 0.5);
	mClientSize.w = (int)((double)mSufRect.w * magnify_x + 0.5);
	mClientSize.h = (int)((double)mSufRect.h * magnify_y + 0.5);
	mMarginSize.w = wi.rcClient.left - wi.rcWindow.left + wi.rcWindow.right - wi.rcClient.right;
	mMarginSize.h = wi.rcClient.top - wi.rcWindow.top + wi.rcWindow.bottom - wi.rcClient.bottom;
	mWindowSize.w = mClientSize.w + mMarginSize.w;
	mWindowSize.h = mClientSize.h + mMarginSize.h;
}

void VKeyboard::adjust_window_size()
{
	if (!hVkbd || !pSurface) return;

	// get dpi on current window
	mSystemMagnifyX = (double)WIN_API_EX::GetDpiForWindow(hVkbd) / USER_DEFAULT_SCREEN_DPI;
	mSystemMagnifyY = (double)WIN_API_EX::GetDpiForWindow(hVkbd) / USER_DEFAULT_SCREEN_DPI;
	WINDOWINFO wi = { sizeof(WINDOWINFO) };
	calc_window_size(wi);
	::SetWindowPos(hVkbd, HWND_TOPMOST, 0, 0, mWindowSize.w, mWindowSize.h, SWP_NOMOVE | SWP_NOZORDER);

	recreate_d2d_surface();
}

void VKeyboard::dpi_changed(int dpix, int dpiy)
{
	if (!hVkbd || !pSurface) return;

	// get dpi on current window
	mSystemMagnifyX = (double)dpix / USER_DEFAULT_SCREEN_DPI;
	mSystemMagnifyY = (double)dpiy / USER_DEFAULT_SCREEN_DPI;
	WINDOWINFO wi = { sizeof(WINDOWINFO) };
	calc_window_size(wi);
	int x = ((wi.rcWindow.right - wi.rcWindow.left) - mWindowSize.w) / 2 + wi.rcWindow.left;
	int y = wi.rcWindow.top;
	::SetWindowPos(hVkbd, HWND_TOPMOST, x, y, mWindowSize.w, mWindowSize.h, SWP_NOZORDER);

	recreate_d2d_surface();
}

void VKeyboard::changing_size(int position, RECT *rect)
{
	if (!hVkbd || !pSurface) return;

	if (position != 0) {
		mUserMagnifyX = (double)(rect->right - rect->left - mMarginSize.w) / mOriginSize.w;
		mUserMagnifyY = (double)(rect->bottom - rect->top - mMarginSize.h) / mOriginSize.h;
		if (mUserMagnifyX < 0.25) {
			mUserMagnifyX = 0.25;
			rect->right = (LONG)(mUserMagnifyX * mOriginSize.w + mMarginSize.w + rect->left + 0.5);
		}
		if (mUserMagnifyY < 0.25) {
			mUserMagnifyY = 0.25;
			rect->bottom = (LONG)(mUserMagnifyY * mOriginSize.h + mMarginSize.h + rect->top + 0.5);
		}
		magnify_x = mSystemMagnifyX * mUserMagnifyX;
		magnify_y = mSystemMagnifyY * mUserMagnifyY;
		::InvalidateRect(hVkbd, NULL, FALSE);
	}
}

void VKeyboard::size_changed(int size_type, int w, int h)
{
	if (!hVkbd || !pSurface) return;

	if (size_type == SIZE_MAXIMIZED
	|| (size_type == SIZE_RESTORED && mSizeType == SIZE_MAXIMIZED)) {
		mUserMagnifyX = (double)w / mOriginSize.w;
		mUserMagnifyY = (double)h / mOriginSize.h;
		magnify_x = mSystemMagnifyX * mUserMagnifyX;
		magnify_y = mSystemMagnifyY * mUserMagnifyY;
		mSizeType = size_type;
		::InvalidateRect(hVkbd, NULL, FALSE);
	}
}

void VKeyboard::change_size(double mag)
{
	mUserMagnifyX = mag;
	mUserMagnifyY = mag;
	adjust_window_size();
	::InvalidateRect(hVkbd, NULL, FALSE);
}

void VKeyboard::set_dist()
{
	if (!hVkbd || !hParent) return;

	WINDOWINFO wip = { sizeof(WINDOWINFO) };
	WINDOWINFO wi = { sizeof(WINDOWINFO) };

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

void VKeyboard::create_popup_menu()
{
	if (hPopupMenu) return;

	hPopupMenu = ::CreatePopupMenu();

	MENUITEMINFO mii;
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fState = MFS_UNHILITE;

	for(int i=0; LABELS::window_size[i].msg_id != CMsg::End; i++) {
		if (LABELS::window_size[i].msg_id != CMsg::Null) {
			mii.fMask = MIIM_STRING | MIIM_ID;
			mii.fType = MFT_STRING;
			mii.wID = ID_WINDOW_SIZE1 + i;
			mii.dwTypeData = (LPSTR)CMSGVM(LABELS::window_size[i].msg_id);
		} else {
			mii.fMask = 0;
			mii.fType = MFT_SEPARATOR;
		}
		::InsertMenuItem(hPopupMenu, i, TRUE, &mii);
	}
}

void VKeyboard::show_popup_menu(int x, int y)
{
	if (!hVkbd || !hParent) return;

	if (!hPopupMenu) {
		create_popup_menu();
	}

	POINT pt = { x, y };
	::ClientToScreen(hVkbd, &pt);

	::TrackPopupMenuEx(hPopupMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON,
		pt.x, pt.y,
		hVkbd,
		NULL
	);
}

int VKeyboard::process_command(int id)
{
	switch(id) {
	case IDOK:
	case IDCANCEL:
		Close();
		return TRUE;
	case ID_WINDOW_SIZE1:
	case ID_WINDOW_SIZE2:
	case ID_WINDOW_SIZE3:
	case ID_WINDOW_SIZE4:
	case ID_WINDOW_SIZE5:
		change_size((double)LABELS::window_size[id - ID_WINDOW_SIZE1].percent / 100.0);
		return TRUE;
	default:
		break;
	}
	return FALSE;
}

void VKeyboard::need_update_window(PressedInfo_t *info, bool onoff)
{
	if (!hVkbd) return;

	need_update_window_base(info, onoff);

#ifdef USE_VKEYBOARD_UPDATE_RECT
	if (!mD2DRender.GetRender()) {
		if (magnify_x == 1.0 && magnify_y == 1.0) {
			RECT re = { info->re.left, info->re.top, info->re.right, info->re.bottom };
			::InvalidateRect(hVkbd, &re, TRUE);
		} else {
			RECT re = { (LONG)(info->re.left * magnify_x), (LONG)(info->re.top * magnify_y), (LONG)(info->re.right * magnify_x), (LONG)(info->re.bottom * magnify_y) };
			::InvalidateRect(hVkbd, &re, TRUE);
		}
	}
#endif
}

void VKeyboard::update_window()
{
	if (!hVkbd) return;

	if (mD2DRender.GetRender()) {
		draw_d2d_render();
	} else {
#ifndef USE_VKEYBOARD_UPDATE_RECT
		::InvalidateRect(hVkbd, NULL, TRUE);
#endif
		::UpdateWindow(hVkbd);
	}
}

void VKeyboard::paint_window(HDC hdc, RECT *re)
{
	if (!pSurface) return;

#if defined(USE_WIN)
	if (mD2DRender.GetRender()) {
		draw_d2d_render();
	} else {
#ifdef USE_VKEYBOARD_UPDATE_RECT
		if (magnify_x == 1.0 && magnify_y == 1.0) {
			::BitBlt(hdc, re->left, re->top, re->right - re->left, re->bottom - re->top,
				pSurface->GetDC(), re->left, re->top, SRCCOPY);
		} else {
			::StretchBlt(hdc, re->left, re->top, re->right - re->left, re->bottom - re->top,
				pSurface->GetDC(), (int)(re->left / magnify_x), (int)(re->top / magnify_y), (int)((re->right - re->left) / magnify_x), (int)((re->bottom - re->top) / magnify_y), SRCCOPY);
		}
#else
		if (magnify == 1.0) {
			::BitBlt(hdc, mSize.x, mSize.y, mSize.w, mSize.h, pSurface->GetDC(), 0, 0, SRCCOPY);
		} else {
			::StretchBlt(hdc, mSize.x, mSize.y, mSize.w, mSize.h,
				pSurface->GetDC(), 0, 0, mSufRect.w, mSufRect.h, SRCCOPY);
		}
#endif
	}
#elif defined(USE_SDL) || defined(USE_SDL2)
#ifdef USE_VKEYBOARD_UPDATE_RECT
	scrntype *p = (scrntype *)pSurface->GetBuffer();
	if (magnify_x == 1.0 && magnify_y == 1.0) {
		p += (re->top * mSufRect.w);
		::SetDIBits(hdc, hSufBmp, mSufRect.h - re->bottom, re->bottom - re->top, p, &bmiSuf, DIB_RGB_COLORS);
		::BitBlt(hdc, re->left, re->top, re->right - re->left, re->bottom - re->top,
			hSufDC, re->left, re->top, SRCCOPY);
	} else {
		int dstTop = (int)(re->top / magnify_y);
		int dstBottom = (int)(re->bottom / magnify_y);
		p += (dstTop * mSufRect.w);
		::SetDIBits(hdc, hSufBmp, mSufRect.h - dstBottom, dstBottom - dstTop, p, &bmiSuf, DIB_RGB_COLORS);
		::StretchBlt(hdc, re->left, re->top, re->right - re->left, re->bottom - re->top,
			hSufDC, (int)(re->left / magnify_x), dstTop, (int)((re->right - re->left) / magnify_x), dstBottom - dstTop, SRCCOPY);
	}
#else
	scrntype *p = (scrntype *)pSurface->GetBuffer();
	::SetDIBits(hdc, hSufBmp, 0, pSurface->Height(), p, &bmiSuf, DIB_RGB_COLORS);
	if (magnify == 1.0) {
		::BitBlt(hdc, mSize.x, mSize.y, mSize.w, mSize.h, hSufDC, 0, 0, SRCCOPY);
	} else {
		::StretchBlt(hdc, mSize.x, mSize.y, mSize.w, mSize.h, hSufDC, 0, 0, mSufRect.w, mSufRect.h, SRCCOPY);
	}
#endif
#endif
}

void VKeyboard::init_dialog(HWND hDlg)
{
	// disable ime
	ImmAssociateContext(hDlg, NULL);

	// set icon on sysmenu
	HICON hIcon = ::LoadIconA(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	::SendMessage(hDlg, WM_SETICON, (WPARAM)NULL, (LPARAM)hIcon);

#if defined(USE_SDL) || defined(USE_SDL2)
	// create bitmap buffer
	if (pSurface) {
		HDC hdc = ::GetDC(hDlg);
		hSufDC = ::CreateCompatibleDC(hdc);
		hSufBmp = ::CreateCompatibleBitmap(hdc, pSurface->Width(), pSurface->Height());
		bmiSuf.bmiHeader.biWidth = pSurface->Width();
		bmiSuf.bmiHeader.biHeight = - pSurface->Height();	// flipped
		bmiSuf.bmiHeader.biPlanes = 1;
		bmiSuf.bmiHeader.biBitCount = pSurface->BitsPerPixel();
		bmiSuf.bmiHeader.biCompression = BI_RGB;
		bmiSuf.bmiHeader.biSizeImage = pSurface->Width() * pSurface->Height() * pSurface->BytesPerPixel();
		::SelectObject(hSufDC, hSufBmp);
		::ReleaseDC(hDlg, hdc);
	}
#endif
}

void VKeyboard::create_d2d_surface()
{
	if (!pSurface) return;

#ifdef USE_DIRECT2D
	gD2DFactory.CreateFactory();
#endif
}

void VKeyboard::release_d2d_surface()
{
#ifdef USE_DIRECT2D
	mD2DSurface.ReleaseSurface();
	mD2DRender.ReleaseRender();
	gD2DFactory.ReleaseFactory();
#endif
}

void VKeyboard::recreate_d2d_surface()
{
	if (!pSurface) return;

#ifdef USE_DIRECT2D
	mD2DSurface.ReleaseSurface();
	mD2DRender.ReleaseRender();
	mD2DRender.CreateRender(gD2DFactory, hVkbd, mClientSize.w, mClientSize.h, 0);
	mD2DSurface.CreateSurface(mD2DRender, pSurface->Width(), pSurface->Height());
#endif
}

void VKeyboard::draw_d2d_render()
{
#ifdef USE_DIRECT2D
	mD2DSurface.Copy(*pSurface);
	mD2DRender.SetInterpolation(pConfig->filter_type);
	mD2DRender.BeginDraw();
	mD2DRender.FlipVertical();
	mD2DRender.DrawBitmap(mD2DSurface, mClientSize, mSufRect);
	mD2DRender.EndDraw();
#endif
}

} /* namespace Vkbd */
