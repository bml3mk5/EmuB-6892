/** @file win_vkeyboard.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2016.12.21 -

	@brief [ virtual keyboard ]
*/

#ifndef WIN_VKEYBOARD_H
#define WIN_VKEYBOARD_H

#include <windows.h>
#include <windowsx.h>
#include "../../res/resource.h"
#include "../vkeyboard.h"
#include "../../osd/windows/win_d2d.h"
#include "../../common.h"

namespace Vkbd {

/**
	@brief Virtual keyboard
*/
class VKeyboard : public OSDBase
{
private:
	HINSTANCE	hInstance;

	HWND		hVkbd;
	HWND		hParent;
#if defined(USE_SDL) || defined(USE_SDL2)
	HDC			hSufDC;
	HBITMAP		hSufBmp;
	BITMAPINFO	bmiSuf;
#endif
	HMENU		hPopupMenu;
	VmRectWH	mSufRect;
	VmSize		mOriginSize;
	VmRectWH	mClientSize;	// client size in window
	VmSize		mWindowSize;
	VmSize		mMarginSize;
	int			mSizeType;

	double mUserMagnifyX;
	double mUserMagnifyY;
	double mSystemMagnifyX;
	double mSystemMagnifyY;

	void calc_window_size(WINDOWINFO &wi);
	void adjust_window_size();
	void set_dist();
	void dpi_changed(int dpix, int dpiy);
	void changing_size(int position, RECT *rect);
	void size_changed(int size_type, int w, int h);
	void change_size(double mag);
	void need_update_window(PressedInfo_t *, bool);
	void update_window();
	void init_dialog(HWND);
	void paint_window(HDC, RECT *);
	void create_popup_menu();
	void show_popup_menu(int x, int y);
	int process_command(int id);

	static INT_PTR CALLBACK Proc(HWND, UINT, WPARAM, LPARAM);

	// Direct2D
	CD2DHwndRender mD2DRender;
	CD2DSurface mD2DSurface;

	void create_d2d_surface();
	void release_d2d_surface();
	void recreate_d2d_surface();
	void draw_d2d_render();

public:
	VKeyboard(HINSTANCE, HWND);
	~VKeyboard();

#if defined(USE_WIN)
	bool Create();
#elif defined(USE_SDL) || defined(USE_SDL2)
	bool Create(const _TCHAR *res_path);
#endif
	void Show(bool = true);
	void Close();
};

} /* namespace Vkbd */

#endif /* WIN_VKEYBOARD_H */
