/** @file win_screen.cpp

	Skelton for retropc emulator

	@author Takeda.Toshiya, Sasaji
	@date   2006.08.18 -

	@brief [ win32 screen ]
*/

#include "win_emu.h"
#include "win_main.h"
#include "win_apiex.h"
#include "../../vm/vm.h"
#include "../../gui/gui.h"
#include "../../config.h"
#ifdef USE_MESSAGE_BOARD
#include "../../msgboard.h"
#endif
#ifdef USE_LEDBOX
#include "../../gui/ledbox.h"
#endif
#include "win_csurface.h"
#include "../../video/rec_video.h"

void EMU_OSD::EMU_SCREEN()
{
	dwStyle = 0;
	hWindow = NULL;
	window_dest_dpi = USER_DEFAULT_SCREEN_DPI;

#ifdef USE_DIRECT3D
# ifndef USE_SCREEN_D3D_TEXTURE
#  ifdef USE_SCREEN_D3D_MIX_SURFACE
	pD3Dmixsuf = NULL;
#  endif
	pD3Dbacksuf = NULL;
# endif
	pD3Dorigin = NULL;
	pD3Dsource = NULL;
# ifdef USE_SCREEN_ROTATE
	pD3Drotate = NULL;
# endif
	lpD3DBmp = NULL;
#endif /* USE_DIRECT3D */

#ifdef USE_DIRECT2D
# ifdef USE_SCREEN_MIX_SURFACE
	pD2Dmixrender = NULL;
# endif
	pD2Dorigin = NULL;
	pD2Dsource = NULL;
# ifdef USE_SCREEN_ROTATE
	pD2Drotate = NULL;
# endif
#endif /* USE_DIRECT2D */

#ifdef USE_LEDBOX
	ledbox = NULL;
#endif
}

void EMU_OSD::initialize_screen()
{
	EMU::initialize_screen();

#ifdef USE_DIRECT3D
	initialize_d3device(hMainWindow);
	create_d3device(hMainWindow);
//	create_d3dofflinesurface();
#endif

#ifdef USE_DIRECT2D
	initialize_d2dfactory(hMainWindow);
	create_d2drender(hMainWindow);
//	create_d2dofflinesurface();
#endif

#ifdef USE_MESSAGE_BOARD
	msgboard = new MsgBoard(this);
	if (msgboard) {
		msgboard->InitScreen(hMainWindow, screen_size.w, screen_size.h);
		msgboard->SetVisible(FLG_SHOWMSGBOARD ? true : false);
	}
#endif

#ifdef USE_LEDBOX
	if (gui) {
		ledbox = gui->CreateLedBox();
	}
#endif
}

///
/// release screen
///
void EMU_OSD::release_screen()
{
#ifdef USE_DIRECT2D
	release_d2drender();
	terminate_d2dfactory();
#endif

#ifdef USE_DIRECT3D
	release_d3device();
	terminate_d3device();
#endif

	if (gui) {
		gui->ReleaseLedBox();

		gui->RestoreDrawingMethod(pConfig->drawing_method);
	}

	EMU::release_screen();
}

///
/// create / recreate window
///
/// @param[in] disp_no : (unused)
/// @param[in] x       : window position x
/// @param[in] y       : window position y
/// @param[in] width   : window width
/// @param[in] height  : window height
/// @param[in] flags   : (unused)
/// @return true / false
bool EMU_OSD::create_screen(int disp_no, int x, int y, int width, int height, uint32_t flags)
{
	if (screen_mode.WithinDisp(x, y) < 0) {
		x = CW_USEDEFAULT;
		y = CW_USEDEFAULT;
	}

	//  | WS_THICKFRAME
	dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE;
	// Don't set WS_EX_COMPOSITED and WS_EX_TRANSPARENT because menu items are hidden by main window on Windows XP.
	dwExStyle = 0; /* WS_EX_COMPOSITED | WS_EX_TRANSPARENT; */

	hWindow = ::CreateWindowEx(dwExStyle, _T(CLASS_NAME), _T(DEVICE_NAME), dwStyle,
	                         x, y, width, height, NULL, NULL, hInstance, NULL);

	hMainWindow = hWindow;

	return (hWindow != NULL);
}
///
/// create / recreate offline surface
///
bool EMU_OSD::create_offlinesurface()
{
	if (!pixel_format) return false;

	if (sufOrigin && !sufOrigin->IsEnable()) {
		if (!sufOrigin->Create(screen_size.w, screen_size.h, *pixel_format)) {
			logging->out_log(LOG_ERROR, _T("EMU_OSD::create_offlinesurface sufOrigin failed."));
			return false;
		}
	}
#ifdef USE_SCREEN_ROTATE
	if (sufRotate) {
		sufRotate->Release();
		if (!sufRotate->Create(source_size.w, source_size.h, *pixel_format)) {
			logging->out_log(LOG_ERROR, _T("EMU_OSD::create_offlinesurface sufRotate failed."));
			return false;
		}
	}
#endif
	if (!create_mixedsurface()) {
		return false;
	}
#ifdef USE_SMOOTH_STRETCH
	if (sufStretch1 && sufStretch2) {
		sufStretch1->Release();
		sufStretch2->Release();
		stretch_screen = false;
		if(stretch_power.w != 1 || stretch_power.h != 1) {
			sufStretch1->Create(source_size.w * stretch_power.w, source_size.h * stretch_power.h);
			SetStretchBltMode(sufStretch1->GetDC(), COLORONCOLOR);

			sufStretch2->Create(stretched_size.w, stretched_size.h);
			SetStretchBltMode(sufStretch2->GetDC(), COLORONCOLOR);

			stretch_screen = true;
		}
	}
#endif

#ifdef USE_DIRECT3D
	create_d3dofflinesurface();
#endif

#ifdef USE_DIRECT2D
	create_d2dofflinesurface();
#endif

	disable_screen &= ~DISABLE_SURFACE;

	return true;
}
///
/// create / recreate mixed surface
///
bool EMU_OSD::create_mixedsurface()
{
	if (sufMixed) {
		sufMixed->Release();
		if (pConfig->drawing_method & DRAWING_METHOD_DBUFFER_MASK) {
			if (!sufMixed->Create(display_size.w, display_size.h, *pixel_format)) {
				logging->out_log(LOG_ERROR, _T("EMU_OSD::create_offlinesurface sufMixed failed."));
				return false;
			}
		}
	}
	return true;
}

/// calculate the client size of window or fullscreen
///
/// @param [in] width : new width or -1 set current width
/// @param [in] height : new height or -1 set current height
/// @param [in] magnify
/// @param [in] now_window : true:window / false:fullscreen
void EMU_OSD::set_display_size(int width, int height, double magnify, bool now_window)
{
//	bool display_size_changed = false;
#ifdef USE_SCREEN_ROTATE
	bool stretch_changed = false;
#endif

	if(width != -1 && (display_size.w != width || display_size.h != height)) {
		display_size.w = width;
		display_size.h = height;
//		display_size_changed = true;
#ifdef USE_SCREEN_ROTATE
		stretch_changed = true;
#endif
	}

#ifdef USE_SCREEN_ROTATE
	VmRectWH prev_source_size = source_size;

	if(pConfig->monitor_type & 1) {
		stretch_changed |= (source_size.w != screen_size.h);
		stretch_changed |= (source_size.h != screen_size.w);
		stretch_changed |= (source_aspect_size.w != screen_aspect_size.h);
		stretch_changed |= (source_aspect_size.h != screen_aspect_size.w);

		source_size.w = screen_size.h;
		source_size.h = screen_size.w;
		source_size.x = screen_size.y;
		source_size.y = screen_size.x;
		source_aspect_size.w = screen_aspect_size.h;
		source_aspect_size.h = screen_aspect_size.w;
	} else
#endif
	{
#ifdef USE_SCREEN_ROTATE
		stretch_changed |= (source_size.w != screen_size.w);
		stretch_changed |= (source_size.h != screen_size.h);
		stretch_changed |= (source_aspect_size.w != screen_aspect_size.w);
		stretch_changed |= (source_aspect_size.h != screen_aspect_size.h);
#endif
		source_size = screen_size;
		source_aspect_size = screen_aspect_size;
	}

	// fullscreen and stretch screen
	if(pConfig->stretch_screen && !now_window && display_size.w >= source_size.w && display_size.h >= source_size.h) {
		if (pConfig->stretch_screen == 1) {
			// fit to full screen
			mixed_size = source_size;
			if (mixed_ratio.w < mixed_ratio.h) {
				mixed_size.h = mixed_size.h * mixed_ratio.w / mixed_ratio.h;
			} else {
				mixed_size.w = mixed_size.w * mixed_ratio.h / mixed_ratio.w;
			}
			mixed_size.x = (source_size.w - mixed_size.w) / 2;
			mixed_size.y = (source_size.h - mixed_size.h) / 2;
			mixed_size.y = adjust_y_position(mixed_size.h, mixed_size.y);

			stretched_size.w = (display_size.h * source_aspect_size.w) / source_aspect_size.h;
			stretched_size.h = display_size.h;
			stretched_dest_real.x = - mixed_size.x * display_size.h / source_aspect_size.h;
			stretched_dest_real.y = - mixed_size.y * display_size.h / source_aspect_size.h;
			if(stretched_size.w > display_size.w) {
				stretched_size.w = display_size.w;
				stretched_size.h = (display_size.w * source_aspect_size.h) / source_aspect_size.w;
				stretched_dest_real.x = - mixed_size.x * display_size.w / source_aspect_size.w;
				stretched_dest_real.y = - mixed_size.y * display_size.w / source_aspect_size.w;
			}
			if (mixed_ratio.w < mixed_ratio.h) {
				stretched_dest_real.y = stretched_dest_real.y * mixed_ratio.h / mixed_ratio.w;
			} else {
				stretched_dest_real.x = stretched_dest_real.x * mixed_ratio.w / mixed_ratio.h;
			}
			stretched_size.x = (display_size.w - stretched_size.w) / 2;
			stretched_size.y = (display_size.h - stretched_size.h) / 2;
			stretched_dest_real.x += stretched_size.x;
			stretched_dest_real.y += stretched_size.y;
#if defined(USE_DIRECT3D) && defined(USE_SCREEN_D3D_TEXTURE)
			int ply_w = stretched_size.w;
			int ply_h = stretched_size.h;
			if (mixed_ratio.w < mixed_ratio.h) {
				ply_h = ply_h * mixed_ratio.h / mixed_ratio.w;
			} else {
				ply_w = ply_w * mixed_ratio.w / mixed_ratio.h;
			}
			reD3Dply.left = (display_size.w - ply_w) / 2;
			reD3Dply.top  = (display_size.h - ply_h) / 2;
			reD3Dply.right = reD3Dply.left + ply_w;
			reD3Dply.bottom = reD3Dply.top + ply_h;
#endif

		} else {
			// fit text area to full screen (cut off padding area)
			VmSize min_size;
			SIZE_IN(min_size, LIMIT_MIN_WINDOW_WIDTH, LIMIT_MIN_WINDOW_HEIGHT);
#ifdef USE_SCREEN_ROTATE
			if(pConfig->monitor_type & 1) {
				SWAP(int, min_size.w, min_size.h);
			}
#endif
			VmSize mixed_rsize, mixed_rrsize;
			mixed_size = source_size;
			SIZE_IN(mixed_rsize, mixed_size.w, mixed_size.h);
			mixed_rrsize = mixed_rsize;

			VmSize min_rsize, min_rrsize;
			min_rsize = min_size;
			min_rrsize = min_size;
			if (mixed_ratio.w < mixed_ratio.h) {
				min_rsize.h = min_rsize.h * mixed_ratio.h / mixed_ratio.w;
				min_rrsize.h = min_rrsize.h * mixed_ratio.w / mixed_ratio.h;
				mixed_rsize.h = mixed_rsize.h * mixed_ratio.h / mixed_ratio.w;
				mixed_rrsize.h = mixed_rrsize.h * mixed_ratio.w / mixed_ratio.h;
			} else {
				min_rsize.w = min_rsize.w * mixed_ratio.w / mixed_ratio.h;
				min_rrsize.w = min_rrsize.w * mixed_ratio.h / mixed_ratio.w;
				mixed_rsize.w = mixed_rsize.w * mixed_ratio.w / mixed_ratio.h;
				mixed_rrsize.w = mixed_rrsize.w * mixed_ratio.h / mixed_ratio.w;
			}

			double magx = (double)display_size.w / min_rsize.w;
			double magy = (double)display_size.h / min_rsize.h;

			bool mag_based_w = (magx < magy);

			stretched_dest_real.x = 0;
			stretched_dest_real.y = 0;
			if(mag_based_w) {
				// magnify = display_size.w / min_rsize.w

				mixed_size.x = (mixed_size.w - min_size.w) / 2;
				mixed_size.w = min_size.w;
				stretched_size.x = 0;
				stretched_size.w = display_size.w;
				stretched_dest_real.x = (display_size.w - (mixed_rsize.w * display_size.w / min_rsize.w)) / 2;

				if ((mixed_size.h * min_rsize.h * display_size.w / min_rsize.w / min_size.h) >= display_size.h) {
					int mh = display_size.h * min_rrsize.h * min_rsize.w / display_size.w / min_size.h;
					mixed_size.y = (mixed_size.h - mh) / 2;
					mixed_size.h = mh;
					stretched_size.y = 0;
					stretched_size.h = display_size.h;
					stretched_dest_real.y = (display_size.h - (mixed_rsize.h * display_size.w / min_rsize.w)) / 2;
#if defined(USE_DIRECT3D) && defined(USE_SCREEN_D3D_TEXTURE)
					reD3Dply.top  = stretched_dest_real.y;
					reD3Dply.bottom = reD3Dply.top + mixed_rsize.h * display_size.w / min_rsize.w;
#endif
				} else {
					int mh = mixed_rrsize.h;
					int sh = mixed_size.h * display_size.w / min_rsize.w;
					mixed_size.y = (mixed_size.h - mh) / 2;
					mixed_size.h = mh;
					stretched_size.y = (display_size.h - sh) / 2;
					stretched_size.h = sh;
					stretched_dest_real.y = stretched_size.y;
#if defined(USE_DIRECT3D) && defined(USE_SCREEN_D3D_TEXTURE)
					reD3Dply.top  = stretched_dest_real.y;
					reD3Dply.bottom = reD3Dply.top + stretched_size.h;
#endif
				}
#if defined(USE_DIRECT3D) && defined(USE_SCREEN_D3D_TEXTURE)
					reD3Dply.left = stretched_dest_real.x;
					reD3Dply.right = - reD3Dply.left + stretched_size.w;
#endif

			} else {
				// magnify = display_size.h / min_rsize.h

				mixed_size.y = (mixed_size.h - min_size.h) / 2;
				mixed_size.h = min_size.h;
				stretched_size.y = 0;
				stretched_size.h = display_size.h;
				stretched_dest_real.y = (display_size.h - (mixed_rsize.h * display_size.h / min_rsize.h)) / 2;

				if ((mixed_size.w * min_rsize.w * display_size.h / min_rsize.h / min_size.w) >= display_size.w) {
					int mw = display_size.w * min_rrsize.w * min_rsize.h / display_size.h / min_size.w;
					mixed_size.x = (mixed_size.w - mw) / 2;
					mixed_size.w = mw;
					stretched_size.x = 0;
					stretched_size.w = display_size.w;
					stretched_dest_real.x = (display_size.w - (mixed_rsize.w * display_size.h / min_rsize.h)) / 2;
#if defined(USE_DIRECT3D) && defined(USE_SCREEN_D3D_TEXTURE)
					reD3Dply.left = stretched_dest_real.x;
					reD3Dply.right = reD3Dply.left + mixed_rsize.w * display_size.h / min_rsize.h;
#endif
				} else {
					int mw = mixed_rrsize.w;
					int sw = mixed_size.w * display_size.h / min_rsize.h;
					mixed_size.x = (mixed_size.w - mw) / 2;
					mixed_size.w = mw;
					stretched_size.x = (display_size.w - sw) / 2;
					stretched_size.w = sw;
					stretched_dest_real.x = stretched_size.x;
#if defined(USE_DIRECT3D) && defined(USE_SCREEN_D3D_TEXTURE)
					reD3Dply.left = stretched_dest_real.x;
					reD3Dply.right = reD3Dply.left + stretched_size.w;
#endif
				}
#if defined(USE_DIRECT3D) && defined(USE_SCREEN_D3D_TEXTURE)
				reD3Dply.top  = stretched_dest_real.y;
				reD3Dply.bottom = - reD3Dply.top + stretched_size.h;
#endif

			}
		}
	}
	// window or non-streach mode
	else {
		for(int n = 0; n <= 1; n++) {
			if (n == 0) {
				mixed_size.w = (int)((double)display_size.w / magnify + 0.5);
				mixed_size.h = (int)((double)display_size.h / magnify + 0.5);
			} else {
				mixed_size = source_size;
			}
			if (mixed_ratio.w < mixed_ratio.h) {
				mixed_size.h = mixed_size.h * mixed_ratio.w / mixed_ratio.h;
			} else {
				mixed_size.w = mixed_size.w * mixed_ratio.h / mixed_ratio.w;
			}
			mixed_size.x = (source_size.w - mixed_size.w) / 2;
			mixed_size.y = (source_size.h - mixed_size.h) / 2;
			if (mixed_size.x >= 0 && mixed_size.y >= 0) {
				break;
			}
		}
		mixed_size.y = adjust_y_position(mixed_size.h, mixed_size.y);

		stretched_size.w = (int)((double)source_aspect_size.w * magnify + 0.5);
		stretched_size.h = (int)((double)source_aspect_size.h * magnify + 0.5);
		stretched_size.x = (display_size.w - stretched_size.w) / 2;
		stretched_size.y = (display_size.h - stretched_size.h) / 2;
		stretched_dest_real.x = (int)((double)- mixed_size.x * magnify + 0.5);
		stretched_dest_real.y = (int)((double)- mixed_size.y * magnify + 0.5);
		if (mixed_ratio.w < mixed_ratio.h) {
			stretched_dest_real.y = stretched_dest_real.y * mixed_ratio.h / mixed_ratio.w;
		} else {
			stretched_dest_real.x = stretched_dest_real.x * mixed_ratio.w / mixed_ratio.h;
		}
#if defined(USE_DIRECT3D) && defined(USE_SCREEN_D3D_TEXTURE)
		int ply_w = stretched_size.w;
		int ply_h = stretched_size.h;
		if (mixed_ratio.w < mixed_ratio.h) {
			ply_h = ply_h * mixed_ratio.h / mixed_ratio.w;
		} else {
			ply_w = ply_w * mixed_ratio.w / mixed_ratio.h;
		}
		reD3Dply.left = (display_size.w - ply_w) / 2;
		reD3Dply.top  = (display_size.h - ply_h) / 2;
		reD3Dply.right = reD3Dply.left + ply_w;
		reD3Dply.bottom = reD3Dply.top + ply_h;
#endif
		if (stretched_size.x < 0) {
			stretched_size.x = 0;
			stretched_size.w = display_size.w;
		}
		if (stretched_size.y < 0) {
			stretched_size.y = 0;
			stretched_size.h = display_size.h;
		}
		stretched_dest_real.x += stretched_size.x;
		stretched_dest_real.y += stretched_size.y;
	}

#ifdef USE_SMOOTH_STRETCH
	int new_pow_x = 1, new_pow_y = 1;
	while(stretched_size.w > source_size.w * new_pow_x) {
		new_pow_x++;
	}
	while(stretched_size.h > source_size.h * new_pow_y) {
		new_pow_y++;
	}

	// support high quality stretch only for x1 window size in gdi mode
	if(new_pow_x > 1 && new_pow_y > 1) {
		new_pow_x = new_pow_y = 1;
	}

//	if(stretched_size.w == source_width * new_pow_x && stretched_size.h == source_height * new_pow_y) {
//		new_pow_x = new_pow_y = 1;
//	}
	if(stretch_power.w != new_pow_x || stretch_power.h != new_pow_y) {
		stretch_power.w = new_pow_x;
		stretch_power.h = new_pow_y;
		stretch_changed = true;
	}

	// re create surface later 
#if 0
	if(stretch_changed) {
		sufStretch1->Release();
		sufStretch2->Release();
		stretch_screen = false;
		if(stretch_power.w != 1 || stretch_power.h != 1) {
			sufStretch1->Create(source_size.w * stretch_power.w, source_size.h * stretch_power.h);
			SetStretchBltMode(sufStretch1->GetDC(), COLORONCOLOR);

			sufStretch2->Create(stretched_size.w, stretched_size.h);
			SetStretchBltMode(sufStretch2->GetDC(), COLORONCOLOR);

			stretch_screen = true;
		}
	}
#endif
#endif

	change_rec_video_size(pConfig->screen_video_size);

	first_invalidate = true;
	screen_size_changed = false;
#ifdef _DEBUG_LOG
	logging->out_debugf(_T("set_display_size: w:%d h:%d permille:%.1f %s"), width, height, magnify, now_window ? _T("window") : _T("fullscreen"));
	logging->out_debugf(_T("         display: w:%d h:%d"), display_size.w, display_size.h);
	logging->out_debugf(_T("          screen: w:%d h:%d"), screen_size.w, screen_size.h);
	logging->out_debugf(_T("   screen aspect: w:%d h:%d"), screen_aspect_size.w, screen_aspect_size.h);
	logging->out_debugf(_T("          source: w:%d h:%d"), source_size.w, source_size.h);
	logging->out_debugf(_T("   source aspect: w:%d h:%d"), source_aspect_size.w, source_aspect_size.h);
	logging->out_debugf(_T("           mixed: w:%d h:%d"), mixed_size.w, mixed_size.h);
	logging->out_debugf(_T("         stretch: w:%d h:%d"), stretched_size.w, stretched_size.h);
	logging->out_debugf(_T("     screen dest: x:%d y:%d"), screen_size.x, screen_size.y);
	logging->out_debugf(_T("     source dest: x:%d y:%d"), source_size.x, source_size.y);
	logging->out_debugf(_T("      mixed dest: x:%d y:%d"), mixed_size.x, mixed_size.y);
	logging->out_debugf(_T("    stretch dest: x:%d y:%d"), stretched_size.x, stretched_size.y);
	logging->out_debugf(_T(" stretch dest re: x:%d y:%d"), stretched_dest_real.x, stretched_dest_real.y);
#endif

#ifdef USE_DIRECT3D
	SetRect(&reD3Dmix, mixed_size.x, mixed_size.y, mixed_size.x + mixed_size.w, mixed_size.y + mixed_size.h);
	SetRect(&reD3Dsuf, stretched_size.x, stretched_size.y, stretched_size.x + stretched_size.w, stretched_size.y + stretched_size.h);

#ifdef _DEBUG_LOG
	logging->out_debugf(_T("D3D          mix: l:%d t:%d r:%d b:%d w:%d h:%d"),reD3Dmix.left,reD3Dmix.top,reD3Dmix.right,reD3Dmix.bottom,mixed_size.w,mixed_size.h);
	logging->out_debugf(_T("             suf: l:%d t:%d r:%d b:%d"),reD3Dsuf.left,reD3Dsuf.top,reD3Dsuf.right,reD3Dsuf.bottom);
#ifdef USE_SCREEN_D3D_TEXTURE
	logging->out_debugf(_T("             ply: l:%d t:%d r:%d b:%d"),reD3Dply.left,reD3Dply.top,reD3Dply.right,reD3Dply.bottom);
#endif
#endif
#endif /* USE_DIRECT3D */

	if (now_window) {
		stretched_size.x += display_margin.left;
		stretched_size.y += display_margin.top;
#ifdef USE_DIRECT3D
		reD3Dsuf.left += display_margin.left;
		reD3Dsuf.top += display_margin.top;
		reD3Dsuf.right += display_margin.left;
		reD3Dsuf.bottom += display_margin.top;
#ifdef _DEBUG_LOG
		logging->out_debugf(_T(" margin      suf: l:%d t:%d r:%d b:%d"),reD3Dsuf.left,reD3Dsuf.top,reD3Dsuf.right,reD3Dsuf.bottom);
#endif
#endif /* USE_DIRECT3D */
	}

	lock_screen();

#ifdef USE_SCREEN_ROTATE
	if(pConfig->monitor_type) {
		sufSource = sufRotate;
	} else
#endif
	{
		sufSource = sufOrigin;
	}

#ifdef USE_DIRECT2D
	if (mD2DRender.GetRender()) {
		reset_d2drender(hMainWindow);
	}

#endif	/* USE_DIRECT2D */

#ifdef USE_DIRECT3D
	if (mD3DDevice.GetDevice()) {
		HRESULT hre;

		if (now_window) {
			mD3DDevice.SetPresentParametersSize(0, 0);
		} else {
			mD3DDevice.SetPresentParametersSize(display_size.w, display_size.h);
		}
		// save buffer to dib temporary
		if ((pConfig->drawing_method & DRAWING_METHOD_DIRECT3D_MASK) != 0 && pD3Dorigin && sufOrigin && sufOrigin->IsEnable()) {
#ifdef USE_SCREEN_D3D_TEXTURE
			copy_d3dtex_dib(pD3Dorigin->GetD3DTexture(), sufOrigin->GetBuffer(), true);
#else
			copy_d3dsuf_dib(pD3Dorigin->GetD3DSurface(), sufOrigin->GetBuffer(), true);
#endif
		}
		// re create back buffer to fit to the main screen(window)
		hre = reset_d3device(hMainWindow);
		if (hre == D3D_OK) {
			mD3DDevice.ClearDevice();
			// restore buffer from dib
			if (pD3Dorigin && sufOrigin && sufOrigin->IsEnable()) {
#ifdef USE_SCREEN_D3D_TEXTURE
				copy_d3dtex_dib(pD3Dorigin->GetD3DTexture(), sufOrigin->GetBuffer(), false);
#else
				copy_d3dsuf_dib(pD3Dorigin->GetD3DSurface(), sufOrigin->GetBuffer(), false);
#endif
			}
		}
		if (mD3DDevice.GetDevice()) {
			int w, h;
			if (mD3DDevice.GetBackBufferSize(w, h)) {
				logging->out_debugf(_T("d3dbackbuffer: w:%d h:%d"), w, h);

				if (w < (int)(reD3Dsuf.right - reD3Dsuf.left)) {
					// adjust width to fit the surface
					uint32_t new_mw = (uint32_t)((uint64_t)w * (reD3Dmix.right - reD3Dmix.left) / (reD3Dsuf.right - reD3Dsuf.left));
					uint32_t new_sw = (uint32_t)((uint64_t)new_mw * (reD3Dsuf.right - reD3Dsuf.left) / (reD3Dmix.right - reD3Dmix.left));
					reD3Dmix.right = reD3Dmix.left + new_mw;
					reD3Dsuf.right = reD3Dsuf.left + new_sw;
				}
				if (h < (int)(reD3Dsuf.bottom - reD3Dsuf.top)) {
					// adjust height to fit the surface
					uint32_t new_mh = (uint32_t)((uint64_t)h * (reD3Dmix.bottom - reD3Dmix.top) / (reD3Dsuf.bottom - reD3Dsuf.top));
					uint32_t new_sh = (uint32_t)((uint64_t)new_mh * (reD3Dsuf.bottom - reD3Dsuf.top) / (reD3Dmix.bottom - reD3Dmix.top));
					reD3Dmix.bottom = reD3Dmix.top + new_mh;
					reD3Dsuf.bottom = reD3Dsuf.top + new_sh;
				}

				logging->out_debugf(_T("          mix: l:%d t:%d r:%d b:%d"),reD3Dmix.left,reD3Dmix.top,reD3Dmix.right,reD3Dmix.bottom);
				logging->out_debugf(_T("          suf: l:%d t:%d r:%d b:%d"),reD3Dsuf.left,reD3Dsuf.top,reD3Dsuf.right,reD3Dsuf.bottom);
#ifdef USE_SCREEN_D3D_TEXTURE
				logging->out_debugf(_T("          ply: l:%d t:%d r:%d b:%d"),reD3Dply.left,reD3Dply.top,reD3Dply.right,reD3Dply.bottom);
#endif
			}
		}
#ifdef USE_SCREEN_ROTATE
		if(pConfig->monitor_type) {
			pD3Dsource = pD3Drotate;
		} else
#endif
		{
			pD3Dsource = pD3Dorigin;
		}

#ifdef USE_SCREEN_D3D_TEXTURE
		pD3Dsource->SetD3DTexturePosition(reD3Dply);
#endif
	}

#endif	/* USE_DIRECT3D */

#ifdef USE_SCREEN_ROTATE
	if ((source_size.w != prev_source_size.w && source_size.h != prev_source_size.h) || stretch_changed) {
		create_offlinesurface();
	}
#else
	create_mixedsurface();
#endif

	// send display size to vm
	set_vm_display_size();

	set_ledbox_position(now_window);

	set_msgboard_position();

	unlock_screen();

	calc_vm_screen_size();
}

void EMU_OSD::set_ledbox_position(bool now_window)
{
#ifdef USE_LEDBOX
	if (gui) {
		gui->SetLedBoxPosition(now_window, 0, 0, display_size.w, display_size.h, pConfig->led_pos | (is_fullscreen() ? 0x10 : 0));
	}
#endif
}

void EMU_OSD::set_msgboard_position()
{
#ifdef USE_MESSAGE_BOARD
	if (msgboard) {
		msgboard->SetSize(display_size.w, display_size.h);
		msgboard->SetMessagePos(4, -4, 2);
		msgboard->SetInfoPos(-4, 4, 1);
	}
#endif
}

///
/// draw src screen from virtual machine
///
void EMU_OSD::draw_screen()
{
	// don't draw screen before new screen size is applied to buffers
	if(screen_size_changed) {
		return;
	}

	lock_screen();

#ifdef USE_DIRECT3D
	HRESULT hre;
	D3DLOCKED_RECT pLockedRect;
#ifdef USE_SCREEN_D3D_TEXTURE
	UINT level = 0;
#endif

	lpD3DBmp = NULL;
	if (pConfig->drawing_method & DRAWING_METHOD_DIRECT3D_MASK) {
#ifdef USE_SCREEN_D3D_TEXTURE
		hre = pD3Dorigin->GetD3DTexture()->LockRect(level, &pLockedRect, NULL, 0 /* D3DLOCK_DISCARD */);
#else
		hre = pD3Dorigin->GetD3DSurface()->LockRect(&pLockedRect, NULL, 0);
#endif
		if (hre == D3D_OK) {
			lpD3DBmp = (scrntype *)pLockedRect.pBits;
		}
	}
#endif /* USE_DIRECT3D */

	// draw screen
	if (!pConfig->now_power_off) {
		vm->draw_screen();
	} else {
		fill_gray();
	}

#ifdef USE_DIRECT3D
	if (pConfig->drawing_method & DRAWING_METHOD_DIRECT3D_MASK)	{
#ifdef USE_SCREEN_D3D_TEXTURE
		pD3Dorigin->GetD3DTexture()->UnlockRect(level);
#else
		pD3Dorigin->GetD3DSurface()->UnlockRect();
#endif
	}
#endif /* USE_DIRECT3D */

	// screen size was changed in vm->draw_screen()
	if(screen_size_changed) {
		unlock_screen();
		return;
	}

#ifdef USE_SCREEN_ROTATE
	// rotate screen
	// right turn
	// src and dst should be the same size
	if(pConfig->monitor_type) {
		int rtype = (pConfig->monitor_type & 3);
		VmSize ss, ds;
		SIZE_IN(ss, screen_size.w, screen_size.h);
		SIZE_IN(ds, source_size.w, source_size.h);
#ifdef USE_DIRECT3D
		if (pConfig->use_direct3d) {
			int nw0 = rotate_matrix[rtype].x[0] < 0 ? (ss.w - 1) : 0;
			int nh0 = rotate_matrix[rtype].y[0] < 0 ? (ss.h - 1) : 0;
			int nw1 = rotate_matrix[rtype].x[1] < 0 ? (ss.w - 1) : 0;
			int nh1 = rotate_matrix[rtype].y[1] < 0 ? (ss.h - 1) : 0;
			D3DLOCKED_RECT pLockedRectOrigin;
			D3DLOCKED_RECT pLockedRectRotate;
			hre = pD3Dorigin->LockRect(&pLockedRectOrigin, NULL, 0);
			hre = pD3Drotate->LockRect(&pLockedRectRotate, NULL, 0);
			for(int sy = 0; sy < ss.h; sy++) {
				scrntype* src = (scrntype *)pLockedRectOrigin.pBits + ss.w * sy;
				for(int sx = 0; sx < ss.w; sx++) {
					int dx = nw0 + rotate_matrix[rtype].x[0] * sx + nh0 + rotate_matrix[rtype].y[0] * sy; 
					int dy = nw1 + rotate_matrix[rtype].x[1] * sx + nh1 + rotate_matrix[rtype].y[1] * sy;
					scrntype* dst = (scrntype *)pLockedRectRotate.pBits + ds.w * dy + dx;
					*dst = *src;
					src++;
				}
			}
			pD3Drotate->UnlockRect();
			pD3Dorigin->UnlockRect();
		} else
#endif
		{
			int nw0 = rotate_matrix[rtype].x[0] < 0 ? (ss.w - 1) : 0;
			int nh0 = rotate_matrix[rtype].y[0] >= 0 ? (ss.h - 1) : 0;
			int nw1 = rotate_matrix[rtype].x[1] >= 0 ? (ss.w - 1) : 0;
			int nh1 = rotate_matrix[rtype].y[1] < 0 ? (ss.h - 1) : 0;
			for(int sy = 0; sy < ss.h; sy++) {
				scrntype* src = sufOrigin->GetBuffer() + ss.w * sy;
				for(int sx = 0; sx < ss.w; sx++) {
					int dx = nw0 + rotate_matrix[rtype].x[0] * sx + nh0 - rotate_matrix[rtype].y[0] * sy; 
					int dy = nw1 - rotate_matrix[rtype].x[1] * sx + nh1 + rotate_matrix[rtype].y[1] * sy;
					scrntype* dst = sufRotate->GetBuffer() + ds.w * dy + dx;
					*dst = *src;
					src++;
				}
			}
		}
	}
#endif

#if 0
	// ledbox is rendered to source surface, because this is the surface to record a video 
#ifdef USE_DIRECT3D
#ifndef USE_SCREEN_D3D_TEXTURE
	if (pConfig->use_direct3d) {
		if (FLG_SHOWLEDBOX && gui) {
			gui->DrawLedBox(pD3Dsource->GetD3DSurface());
		}
	} else
#endif
#endif
	{
		if (FLG_SHOWLEDBOX && gui) {
			gui->DrawLedBox(sufSource->GetDC());
		}
	}
#endif

	unlock_screen();
}

#if 0
///
/// copy src screen to mix screen and overlap a message
///
/// @return false: cannot mix (no allocate mix surface)
bool EMU_OSD::mix_screen()
{
	lock_screen();

#ifdef USE_DIRECT3D
	if (pConfig->use_direct3d) {
		// mix d3d screen buffer
#ifndef USE_SCREEN_D3D_TEXTURE
#ifdef USE_SCREEN_D3D_MIX_SURFACE
#if 0
		pD3Device->UpdateSurface(pD3Dsource, NULL, pD3Dmixsuf, NULL);

		RECT src_re;
		src_re.left = vm_screen_size.x;
		src_re.top = vm_screen_size.y;
		src_re.right = vm_screen_size.w - vm_screen_size.x - vm_screen_size.x;
		src_re.bottom = vm_screen_size.h - vm_screen_size.y - vm_screen_size.y;
		RECT dst_re;
		dst_re.left = dst_re.top = 0;
		dst_re.right = sufMixed->Width();
		dst_re.bottom = sufMixed->Height();
		pD3Device->StretchRect(pD3Dsource, &src_re, pD3Dmixsuf, &dst_re, D3DTEXF_NONE);
#else
		HDC src_dc;
		HDC dst_dc;
		pD3Dsource->GetD3DSurface()->GetDC(&src_dc);
		pD3Dmixsuf->GetD3DSurface()->GetDC(&dst_dc);
		StretchBlt(dst_dc, 0, 0, sufMixed->Width(), sufMixed->Height(), src_dc, vm_screen_size.x, vm_screen_size.y, vm_screen_size.w, vm_screen_size.h, SRCCOPY);
		pD3Dsource->GetD3DSurface()->ReleaseDC(src_dc);
		pD3Dmixsuf->GetD3DSurface()->ReleaseDC(dst_dc);
#endif
		if (FLG_SHOWLEDBOX && ledbox) {
			ledbox->Draw(pD3Dmixsuf->GetD3DSurface());
		}

#ifdef USE_MESSAGE_BOARD
		if (msgboard) {
			msgboard->Draw(pD3Dmixsuf->GetD3DSurface());
		}
#endif
#endif /* USE_SCREEN_D3D_MIX_SURFACE */
#endif /* !USE_SCREEN_D3D_TEXTURE */
	} else
#endif /* !USE_DIRECT3D */
	{
#ifdef USE_DIRECT2D
		if (enable_direct2d) {
			// mix screen buffer
			mD2DRender.SetInterpolation(pConfig->filter_type);
#ifdef USE_SCREEN_MIX_SURFACE
			if (pConfig->double_buffering) {
				//copy to render buffer with stretched size
				pD2Dmixrender->SetInterpolation(pConfig->filter_type);
				pD2Dsource->Copy(*sufSource);
				pD2Dmixrender->BeginDraw();
				pD2Dmixrender->DrawBitmap(*pD2Dsource, stretched_size, vm_screen_size); 
#ifdef USE_LEDBOX
				if (FLG_SHOWLEDBOX && ledbox) {
					ledbox->Draw(*pD2Dmixrender);
				}
#endif
#ifdef USE_MESSAGE_BOARD
				if (msgboard) {
					msgboard->Draw(*pD2Dmixrender);
				}
				pD2Dmixrender->EndDraw();
#endif
			} else
#endif /* USE_SCREEN_MIX_SURFACE */
			{
				pD2Dsource->Copy(*sufSource);
			}
		} else
#endif /* USE_DIRECT2D */
		{
			// mix dib screen buffer
#ifdef USE_SCREEN_MIX_SURFACE
			if (pConfig->double_buffering) {
				//copy to render buffer with stretched size
				sufSource->StretchBlit(vm_screen_size, *sufMixed, stretched_size);
#ifdef USE_LEDBOX
				if (FLG_SHOWLEDBOX && ledbox) {
					ledbox->Draw(sufMixed->GetDC());
				}
#endif
#ifdef USE_MESSAGE_BOARD
				if (msgboard) {
					msgboard->Draw(sufMixed->GetDC());
				}
#endif
			}
#endif /* USE_SCREEN_MIX_SURFACE */
		}
	}

	// stretch screen
#ifdef USE_SMOOTH_STRETCH
	//	if(stretch_screen) {
	if(stretch_screen && (!pConfig->use_direct3d)) {
#if 0
		StretchBlt(sufStretch1->GetDC(), 0, 0, source_size.w * stretch_power.w, source_size.h * stretch_power.h, sufSource->GetDC(), 0, 0, source_size.w, source_size.h, SRCCOPY);
#else
		// about 50% faster than StretchBlt()
		scrntype* src = sufSource->GetBuffer() + source_size.w * (source_size.h - 1);
		scrntype* dst = sufStretch1->GetBuffer() + source_size.w * stretch_power.w * (source_size.h * stretch_power.h - 1);
		int data_len = source_size.w * stretch_power.w;

		for(int y = 0; y < source_size.h; y++) {
			if(stretch_power.w != 1) {
				scrntype* dst_tmp = dst;
				for(int x = 0; x < source_size.w; x++) {
					scrntype c = src[x];
					for(int px = 0; px < stretch_power.w; px++) {
						dst_tmp[px] = c;
					}
					dst_tmp += stretch_power.w;
				}
			}
			else {
				// faster than memcpy()
				for(int x = 0; x < source_size.w; x++) {
					dst[x] = src[x];
				}
			}
			if(stretch_power.h != 1) {
				scrntype* src_tmp = dst;
				for(int py = 1; py < stretch_power.h; py++) {
					dst -= data_len;
					// about 10% faster than memcpy()
					for(int x = 0; x < data_len; x++) {
						dst[x] = src_tmp[x];
					}
				}
			}
			src -= source_size.w;
			dst -= data_len;
		}
#endif

		StretchBlt(sufStretch2->GetDC(), 0, 0, stretched_size.w, stretched_size.h, sufStretch1->GetDC(), 0, 0, source_size.w * stretch_power.w, source_size.h * stretch_power.h, SRCCOPY);
	}
#endif

	unlock_screen();

	return true;
}
#endif

#ifdef USE_DIRECT2D
///
/// copy src screen to mix screen and overlap a message
///
void EMU_OSD::mix_screen_d2d()
{
	lock_screen();

	// mix screen buffer
	mD2DRender.SetInterpolation(pConfig->filter_type);
#ifdef USE_SCREEN_MIX_SURFACE
	if (pConfig->drawing_method & DRAWING_METHOD_DBUFFER_MASK) {
		//copy to render buffer with stretched size
		VmRectWH srcRect = vm_screen_size;
		srcRect.y = (screen_size.h - vm_screen_size.h - srcRect.y);
		pD2Dmixrender->SetInterpolation(pConfig->filter_type);
		pD2Dsource->Copy(*sufSource);
		pD2Dmixrender->BeginDraw();
		pD2Dmixrender->DrawBitmap(*pD2Dsource, stretched_size, srcRect); 
#ifdef USE_LEDBOX
		if (FLG_SHOWLEDBOX && ledbox) {
			ledbox->Draw(*pD2Dmixrender);
		}
#endif
#ifdef USE_MESSAGE_BOARD
		if (msgboard) {
			msgboard->Draw(*pD2Dmixrender);
		}
		pD2Dmixrender->EndDraw();
#endif
	} else
#endif /* USE_SCREEN_MIX_SURFACE */
	{
		pD2Dsource->Copy(*sufSource);
	}

	unlock_screen();
}
#endif /* USE_DIRECT2D */

#ifdef USE_DIRECT3D
///
/// copy src screen to mix screen and overlap a message
///
void EMU_OSD::mix_screen_d3d()
{
	lock_screen();

	// mix d3d screen buffer
#ifndef USE_SCREEN_D3D_TEXTURE
#ifdef USE_SCREEN_D3D_MIX_SURFACE
#if 0
	RECT src_re;
	src_re.left = vm_screen_size.x;
	src_re.top = vm_screen_size.y;
	src_re.right = vm_screen_size.w + vm_screen_size.x;
	src_re.bottom = vm_screen_size.h + vm_screen_size.y;
//	RECT dst_re;
//	dst_re.left = dst_re.top = 0;
//	dst_re.right = 640; //pD3Dmixsuf->Width();
//	dst_re.bottom = 480; //pD3Dmixsuf->Height();
////	pD3Dmixsuf->Update(mD3DDevice, *pD3Dsource, src_re, dst_re);
	pD3Dmixsuf->StretchRect(mD3DDevice, *pD3Dsource, src_re, pConfig->filter_type);
#else
	HDC src_dc;
	HDC dst_dc;
	pD3Dsource->GetD3DSurface()->GetDC(&src_dc);
	pD3Dmixsuf->GetD3DSurface()->GetDC(&dst_dc);
	StretchBlt(dst_dc, 0, 0, pD3Dmixsuf->Width(), pD3Dmixsuf->Height(), src_dc, vm_screen_size.x, vm_screen_size.y, vm_screen_size.w, vm_screen_size.h, SRCCOPY);
	pD3Dsource->GetD3DSurface()->ReleaseDC(src_dc);
	pD3Dmixsuf->GetD3DSurface()->ReleaseDC(dst_dc);
#endif
	if (FLG_SHOWLEDBOX && ledbox) {
		ledbox->Draw(*pD3Dmixsuf);
	}

#ifdef USE_MESSAGE_BOARD
	if (msgboard) {
		msgboard->Draw(*pD3Dmixsuf);
	}
#endif
#endif /* USE_SCREEN_D3D_MIX_SURFACE */
#endif /* !USE_SCREEN_D3D_TEXTURE */

	unlock_screen();
}
#endif /* !USE_DIRECT3D */

///
/// copy src screen to mix screen and overlap a message
///
void EMU_OSD::mix_screen_dc()
{
	lock_screen();

	// mix dib screen buffer
#ifdef USE_SCREEN_MIX_SURFACE
	if (pConfig->drawing_method & DRAWING_METHOD_DBUFFER_MASK) {
		//copy to render buffer with stretched size
		sufSource->StretchBlit(vm_screen_size, *sufMixed, stretched_size);
#ifdef USE_LEDBOX
		if (FLG_SHOWLEDBOX && ledbox) {
			ledbox->Draw(sufMixed->GetDC());
		}
#endif
#ifdef USE_MESSAGE_BOARD
		if (msgboard) {
			msgboard->Draw(sufMixed->GetDC());
		}
#endif
	}
#endif /* USE_SCREEN_MIX_SURFACE */

	// stretch screen
#ifdef USE_SMOOTH_STRETCH
	//	if(stretch_screen) {
	if(stretch_screen && (!pConfig->use_direct3d)) {
#if 0
		StretchBlt(sufStretch1->GetDC(), 0, 0, source_size.w * stretch_power.w, source_size.h * stretch_power.h, sufSource->GetDC(), 0, 0, source_size.w, source_size.h, SRCCOPY);
#else
		// about 50% faster than StretchBlt()
		scrntype* src = sufSource->GetBuffer() + source_size.w * (source_size.h - 1);
		scrntype* dst = sufStretch1->GetBuffer() + source_size.w * stretch_power.w * (source_size.h * stretch_power.h - 1);
		int data_len = source_size.w * stretch_power.w;

		for(int y = 0; y < source_size.h; y++) {
			if(stretch_power.w != 1) {
				scrntype* dst_tmp = dst;
				for(int x = 0; x < source_size.w; x++) {
					scrntype c = src[x];
					for(int px = 0; px < stretch_power.w; px++) {
						dst_tmp[px] = c;
					}
					dst_tmp += stretch_power.w;
				}
			}
			else {
				// faster than memcpy()
				for(int x = 0; x < source_size.w; x++) {
					dst[x] = src[x];
				}
			}
			if(stretch_power.h != 1) {
				scrntype* src_tmp = dst;
				for(int py = 1; py < stretch_power.h; py++) {
					dst -= data_len;
					// about 10% faster than memcpy()
					for(int x = 0; x < data_len; x++) {
						dst[x] = src_tmp[x];
					}
				}
			}
			src -= source_size.w;
			dst -= data_len;
		}
#endif

		StretchBlt(sufStretch2->GetDC(), 0, 0, stretched_size.w, stretched_size.h, sufStretch1->GetDC(), 0, 0, source_size.w * stretch_power.w, source_size.h * stretch_power.h, SRCCOPY);
	}
#endif

	unlock_screen();
}

///
/// post request screen updating to draw it on main thread
///
void EMU_OSD::need_update_screen()
{
	// invalidate window
//	if (!first_invalidate && (pConfig->use_direct2d && enable_direct2d)) {
//		::PostMessage(hMainWindow, WM_USERPAINT, 0L, 0L);
//	} else {
		UINT flags = RDW_INVALIDATE | RDW_INTERNALPAINT | (first_invalidate ? RDW_ERASE : RDW_NOERASE);
		::RedrawWindow(hMainWindow, NULL, NULL, flags);
//	}
	self_invalidate = true;
//	skip_frame = false;
}

///
/// pointer on source screen
///
/// @return pointer on source screen
scrntype* EMU_OSD::screen_buffer(int y)
{
#ifdef USE_DIRECT3D
	if (lpD3DBmp && (pConfig->drawing_method & DRAWING_METHOD_DIRECT3D_MASK) != 0) {
		return lpD3DBmp + screen_size.w * y;
	} else
#endif
	return sufOrigin->GetBuffer() + screen_size.w * (screen_size.h - y - 1);
}

///
/// offset on source screen
///
/// @return offset on source screen
int EMU_OSD::screen_buffer_offset()
{
#ifdef USE_DIRECT3D
	if (lpD3DBmp && (pConfig->drawing_method & DRAWING_METHOD_DIRECT3D_MASK) != 0) {
		return screen_size.w;
	} else
#endif
	return -screen_size.w;
}

///
/// change screen size on vm
///
void EMU_OSD::set_vm_screen_size(int screen_width, int screen_height, int window_width, int window_height, int window_width_aspect, int window_height_aspect)
{
	EMU::set_vm_screen_size(screen_width, screen_height, window_width, window_height, window_width_aspect, window_height_aspect);

	calc_vm_screen_size();

	first_invalidate = true;
}

void EMU_OSD::calc_vm_screen_size()
{
	calc_vm_screen_size_sub(mixed_size, vm_screen_size);

#ifdef _DEBUG
	logging->out_debugf(_T("vm_display_size: w:%d h:%d"), vm_display_size.w, vm_display_size.h);
	logging->out_debugf(_T("vm_screen_size: x:%d y:%d w:%d h:%d"), vm_screen_size.x, vm_screen_size.y, vm_screen_size.w, vm_screen_size.h);
#endif

#if defined(USE_DIRECT3D) && defined(USE_SCREEN_D3D_TEXTURE)
	if (pD3Dorigin) {
		float uw = (float)vm_display_size.w / MIN_WINDOW_WIDTH;
		float vh = (float)vm_display_size.h / MIN_WINDOW_HEIGHT;

		float ux = 0.0;
		if (vm_display_size.w < SCREEN_WIDTH) {
			ux = ((float)screen_size.x - (float)screen_size.x * uw) / (float)screen_size.w;
		} else {
			ux = - (float)screen_size.x * uw / (float)screen_size.w;
		}
		float vy = ((float)screen_size.y - (float)screen_size.y * vh) / (float)screen_size.h;

		pD3Dorigin->SetD3DTexturePositionUV(ux, vy, uw, vh);

#ifdef _DEBUG
		logging->out_debugf(_T("D3DTexturePositionUV: ux:%.3f vy:%.3f uw:%.3f vh:%.3f"), ux, vy, uw, vh);
#endif
	}
#endif
}

void EMU_OSD::calc_vm_screen_size_sub(const VmRectWH &src_size, VmRectWH &vm_size)
{
	double sw = (double)vm_display_size.w * src_size.w / MIN_WINDOW_WIDTH;
	vm_size.w = (int)(sw + 0.5);
	if (vm_display_size.w < SCREEN_WIDTH) {
		vm_size.x = (int)((double)screen_size.x - ((sw - vm_display_size.w) / 2.0) + 0.5);
	} else {
		vm_size.x = (int)(- ((sw - vm_display_size.w) / 2.0) + 0.5);
	}
	double sh = (double)vm_display_size.h * src_size.h / MIN_WINDOW_HEIGHT;
	vm_size.h = (int)(sh + 0.5);
	vm_size.y = (int)((double)screen_size.y - ((sh - vm_display_size.h) / 2.0) + 0.5);
}

#ifdef USE_DIRECT2D
///
/// update screen using Direct2D
///
void EMU_OSD::update_screen_d2d()
{
	HRESULT hre;

	if(disable_screen) return;

#ifdef USE_SCREEN_MIX_SURFACE
	if (pConfig->drawing_method & DRAWING_METHOD_DBUFFER_MASK) {
		// copy from mixed buffer
		lock_screen();
		mD2DRender.BeginDraw();
		mD2DRender.FlipVertical();
		mD2DRender.DrawBitmap(*pD2Dmixrender); 
		hre = mD2DRender.EndDraw();
		unlock_screen();
	} else
#endif
	{
		// draw to device context directly
		VmRectWH srcRect = vm_screen_size;
		srcRect.y = (screen_size.h - vm_screen_size.h - srcRect.y);
		lock_screen();
		mD2DRender.BeginDraw();
		mD2DRender.FlipVertical();
		mD2DRender.DrawBitmap(*pD2Dsource, stretched_size, srcRect);
#ifdef USE_LEDBOX
		ledbox->Draw(mD2DRender);
#endif
#ifdef USE_MESSAGE_BOARD
		msgboard->Draw(mD2DRender);
#endif
		hre = mD2DRender.EndDraw();
		unlock_screen();
	}
	if (hre == S_OK) {
		first_invalidate = self_invalidate = false;
	} else if (hre == D2DERR_RECREATE_TARGET) {
		lock_screen();
		reset_d2drender(hMainWindow);
		unlock_screen();
	}
}
#endif

#ifdef USE_DIRECT3D
///
/// update screen using Direct3D
///
void EMU_OSD::update_screen_d3d()
{
	HRESULT hre = D3D_OK;

	hre = mD3DDevice.TestDevice();
	if (hre == D3DERR_DEVICELOST) {
		// now lost device
		return;
	} else if (hre == D3DERR_DEVICENOTRESET) {
		// reset device to use
		lock_screen();
		reset_d3device(hMainWindow);
		unlock_screen();
		return;
	}

	if(disable_screen) return;

	if (hre == D3D_OK) {

#ifdef USE_SCREEN_D3D_TEXTURE
		// set texture and draw screen
		lock_screen();
		hre = mD3DDevice.BeginScene(pConfig->filter_type);
		if (hre == D3D_OK) {
			hre = mD3DDevice.SetFVF();
			pD3Dsource->DrawD3DTexture(mD3DDevice);
#ifdef USE_LEDBOX
			ledbox->Draw(mD3DDevice);
#endif
#ifdef USE_MESSAGE_BOARD
			msgboard->Draw(mD3DDevice);
#endif
			mD3DDevice.EndScene();

		}
		if (hre == D3D_OK) {
			hre = mD3DDevice.Present();
		}
		unlock_screen();

#else /* !USE_SCREEN_D3D_TEXTURE */

		lock_screen();
		// get back buffer
		hre = pD3Dbacksuf->CreateD3DBackBufferSurface(mD3DDevice);
		if (hre == D3D_OK) {
#ifdef USE_SCREEN_D3D_MIX_SURFACE
			// copy to back buffer from offline surface
			hre = pD3Dbacksuf->StretchRect(mD3DDevice, *pD3Dmixsuf, reD3Dsuf, reD3Dsuf, pConfig->filter_type);
#else
			// copy to back buffer from offline surface
			RECT src_re;
			src_re.left = vm_screen_size.x;
			src_re.top = vm_screen_size.y;
			src_re.right = vm_screen_size.w + vm_screen_size.x;
			src_re.bottom = vm_screen_size.h + vm_screen_size.y;
			hre = pD3Dbacksuf->StretchRect(mD3DDevice, *pD3Dsource, src_re, reD3Dsuf, pConfig->filter_type);
#ifdef USE_LEDBOX
			ledbox->Draw(*pD3Dsource);
#endif
#ifdef USE_MESSAGE_BOARD
			msgboard->Draw(*pD3Dsource);
#endif

#endif
			pD3Dbacksuf->ReleaseD3DSurface();
		}
		// draw
		if (hre == D3D_OK) {
			hre = mD3DDevice.Present();
		}
		unlock_screen();

#endif /* USE_SCREEN_D3D_TEXTURE */

		first_invalidate = self_invalidate = false;
	}
}
#endif

///
/// update screen using DC
///
void EMU_OSD::update_screen_dc(HDC hdc)
{
	if(disable_screen) return;

	// standard screen
#ifdef USE_SMOOTH_STRETCH
	if(stretch_screen) {
		BitBlt(hdc, screen_size.x, screen_size.y, stretched_size.w, stretched_size.h, sufStretch2->GetDC(), 0, 0, SRCCOPY);
	} else
#endif
	{
#ifdef USE_SCREEN_MIX_SURFACE
		if (pConfig->drawing_method & DRAWING_METHOD_DBUFFER_MASK) {
			// copy from mixed buffer
			lock_screen();
			BitBlt(hdc, 0, 0, display_size.w, display_size.h, sufMixed->GetDC(), 0, 0, SRCCOPY);
			unlock_screen();
		} else
#endif
		{
			// draw to device context directly
			lock_screen();
			BeginPath(hdc);
			if(stretched_size.w == vm_screen_size.w && stretched_size.h == vm_screen_size.h) {
				BitBlt(hdc, stretched_size.x, stretched_size.y, stretched_size.w, stretched_size.h, sufSource->GetDC(), vm_screen_size.x, vm_screen_size.y, SRCCOPY);
			}
			else {
				StretchBlt(hdc, stretched_size.x, stretched_size.y, stretched_size.w, stretched_size.h, sufSource->GetDC(), vm_screen_size.x, vm_screen_size.y, vm_screen_size.w, vm_screen_size.h, SRCCOPY);
			}
#ifdef USE_LEDBOX
			ledbox->Draw(hdc);
#endif
#ifdef USE_MESSAGE_BOARD
			msgboard->Draw(hdc);
#endif
			EndPath(hdc);
			unlock_screen();
		}
	}

	first_invalidate = self_invalidate = false;
}

///
/// render screen and display window
///
/// @note must be called by main thread
void EMU_OSD::update_screen(HWND hWnd, bool user_event)
{
	if (!initialized) return;

	HDC hdc;
	PAINTSTRUCT ps;

	if (gui) {
		gui->UpdateIndicator(update_led());
	}

	switch(pConfig->drawing_method & DRAWING_METHOD_ALL_MASK) {
#ifdef USE_DIRECT3D
	case DRAWING_METHOD_DIRECT3D_MASK:
		mix_screen_d3d();

		if (!user_event) {
			hdc = ::BeginPaint(hWnd, &ps);
			::EndPaint(hWnd, &ps);
		}
		update_screen_d3d();

		break;
#endif
#ifdef USE_DIRECT2D
	case DRAWING_METHOD_DIRECT2D_MASK:
		mix_screen_d2d();

		if (!user_event) {
			hdc = ::BeginPaint(hWnd, &ps);
			::EndPaint(hWnd, &ps);
		}
		update_screen_d2d();

		break;
#endif
	default:
		mix_screen_dc();

		hdc = ::BeginPaint(hWnd, &ps);
		update_screen_dc(hdc);
		::EndPaint(hWnd, &ps);

		break;
	}
}

#ifdef USE_DIRECT3D
#ifdef USE_SCREEN_D3D_TEXTURE
void EMU_OSD::copy_d3dtex_dib(PDIRECT3DTEXTURE9 tex, scrntype *buf, bool to_dib)
{
	HRESULT hre;
	D3DLOCKED_RECT pLockedRect;
	UINT level = 0;

	hre = tex->LockRect(level, &pLockedRect, NULL, 0);
	if (hre == D3D_OK) {
		scrntype *src;
		scrntype *out;
		if (to_dib) {
			src = (scrntype *)pLockedRect.pBits;
			out = buf;
		} else {
			src = buf;
			out = (scrntype *)pLockedRect.pBits;
		}
		out += screen_size.w * (screen_size.h - 1);
		int data_len = screen_size.w;

		for(int y = 0; y < screen_size.h; y++) {
			for(int i = 0; i < data_len; i++) {
				out[i] = src[i];
			}
			src += data_len;
			out -= data_len;
		}
		tex->UnlockRect(level);
	}
}
#else
void EMU_OSD::copy_d3dsuf_dib(PDIRECT3DSURFACE9 suf, scrntype *buf, bool to_dib)
{
	HRESULT hre;
	D3DLOCKED_RECT pLockedRect;

	hre = suf->LockRect(&pLockedRect, NULL, 0);
	if (hre == D3D_OK) {
		scrntype *src;
		scrntype *out;
		if (to_dib) {
			src = (scrntype *)pLockedRect.pBits;
			out = buf;
		} else {
			src = buf;
			out = (scrntype *)pLockedRect.pBits;
		}
		out += screen_size.w * (screen_size.h - 1);
		int data_len = screen_size.w;

		for(int y = 0; y < screen_size.h; y++) {
			for(int i = 0; i < data_len; i++) {
				out[i] = src[i];
			}
			src += data_len;
			out -= data_len;
		}
		suf->UnlockRect();
	}
}
#endif
#endif /* USE_DIRECT3D */

///
/// create the surface for recording
///
/// @return true if create successfully
bool EMU_OSD::create_recordingsurface()
{
#ifdef USE_RECORDING_SURFACE
	if (!sufRecording->IsEnable()) {
		if (!sufRecording->Create(sufOrigin->Width(), sufOrigin->Height(), *pixel_format)) {
			logging->out_log(LOG_ERROR, _T("Cannot create surface for recording."));
			return false;
		}
	}
#endif /* USE_RECORDING_SURFACE */
	return true;
}

///
/// copy current screen to the surface for recording
///
void EMU_OSD::copy_surface_for_rec()
{
	lock_screen();

#ifdef USE_RECORDING_SURFACE

# ifdef USE_DIRECT3D
	if (pD3Dorigin != NULL && (pConfig->drawing_method & DRAWING_METHOD_DIRECT3D_MASK) != 0) {
#  ifdef USE_SCREEN_D3D_TEXTURE
		copy_d3dtex_dib(pD3Dorigin->GetD3DTexture(), sufRecording->GetBuffer(), true);
#  else
		copy_d3dsuf_dib(pD3Dorigin->GetD3DSurface(), sufRecording->GetBuffer(), true);
#  endif
	} else
# endif /* USE_DIRECT3D */
	{
		sufOrigin->Blit(*sufRecording);
	}
# ifdef USE_LEDBOX
	if (ledbox) {
		ledbox->DrawForRec(sufRecording->GetDC());
	}
# endif

#else /* !USE_RECORDING_SURFACE */

# ifdef USE_DIRECT3D
	if (pD3Dorigin != NULL && (pConfig->drawing_method & DRAWING_METHOD_DIRECT3D_MASK) != 0) {
#  ifdef USE_SCREEN_D3D_TEXTURE
		copy_d3dtex_dib(pD3Dorigin->GetD3DTexture(), sufOrigin->GetBuffer(), true);
#  else
		copy_d3dsuf_dib(pD3Dorigin->GetD3DSurface(), sufOrigin->GetBuffer(), true);
#  endif
	}
# endif /* USE_DIRECT3D */

#endif /* USE_RECORDING_SURFACE */

	unlock_screen();
}

///
/// capture current screen and save to a file
///
void EMU_OSD::capture_screen()
{
#ifdef USE_CAPTURE_SCREEN
# ifdef USE_RECORDING_SURFACE
	if (!create_recordingsurface()) {
		return;
	}

	int size = pConfig->screen_video_size;

	copy_surface_for_rec();

//	calc_vm_screen_size_sub(rec_video_size[size], vm_screen_size_for_rec);

	rec_video->Capture(CAPTURE_SCREEN_TYPE, rec_video_stretched_size, sufRecording, rec_video_size[size]);

# else /* !USE_RECORDING_SURFACE */
	int size = pConfig->screen_video_size;

	copy_surface_for_rec();

	calc_vm_screen_size_sub(rec_video_size[size], vm_screen_size_for_rec);

	rec_video->Capture(CAPTURE_SCREEN_TYPE, vm_screen_size_for_rec, sufOrigin, rec_video_size[size]);

# endif /* USE_RECORDING_SURFACE */
#endif /* USE_CAPTURE_SCREEN */
}

///
/// start recording video
///
/// @return false if cannot start recording
bool EMU_OSD::start_rec_video(int type, int fps_no, bool show_dialog)
{
#ifdef USE_REC_VIDEO
# ifdef USE_RECORDING_SURFACE
	if (!create_recordingsurface()) {
		return false;
	}

	int size = pConfig->screen_video_size;
	return rec_video->Start(type, fps_no, rec_video_size[size], sufRecording, show_dialog);

# else /* !USE_RECORDING_SURFACE */
	int size = pConfig->screen_video_size;
	return rec_video->Start(type, fps_no, rec_video_size[size], sufOrigin, show_dialog);

# endif /* USE_RECORDING_SURFACE */
#else
	return false;
#endif /* USE_REC_VIDEO */
}

///
/// record one frame to stream
///
void EMU_OSD::record_rec_video()
{
#ifdef USE_REC_VIDEO
# ifdef USE_RECORDING_SURFACE
	if (rec_video->IsRecordFrame()) {
		int size = pConfig->screen_video_size;

		copy_surface_for_rec();

		rec_video->Record(rec_video_stretched_size, sufRecording, rec_video_size[size]);
	}

# else /* !USE_RECORDING_SURFACE */
	if (rec_video->IsRecordFrame()) {
		int size = pConfig->screen_video_size;

		copy_surface_for_rec();

		rec_video->Record(rec_video_stretched_size, sufOrigin, rec_video_size[size]);
	}

# endif /* USE_RECORDING_SURFACE */
#endif /* USE_REC_VIDEO */
}

///
/// change video frame size for recording
///
void EMU_OSD::change_rec_video_size(int num)
{
	EMU::change_rec_video_size(num);

#ifdef USE_REC_VIDEO
# ifdef USE_RECORDING_SURFACE
#  ifdef USE_LEDBOX
	if (ledbox) {
		ledbox->SetPosForRec(rec_video_stretched_size.x
			, rec_video_stretched_size.y
			, rec_video_stretched_size.x + rec_video_stretched_size.w
			, rec_video_stretched_size.y + rec_video_stretched_size.h);
	}
#  endif
# endif /* USE_RECORDING_SURFACE */
#endif /* USE_REC_VIDEO */
}

///
/// store window position to ini file
///
void EMU_OSD::resume_window_placement()
{
	if (now_screenmode == NOW_FULLSCREEN) {
		pConfig->window_position_x = window_dest.x;
		pConfig->window_position_y = window_dest.y;
	} else {
		WINDOWINFO wid = { sizeof(WINDOWINFO) };
		GetWindowInfo(hMainWindow, &wid);
		pConfig->window_position_x = wid.rcWindow.left;
		pConfig->window_position_y = wid.rcWindow.top;
	}
}

/// show window and move position
/// @param [in] dest_x        : move to position x (unless set SWP_NOMOVE on flags)
/// @param [in] dest_y        : move to position y (unless set SWP_NOMOVE on flags)
/// @param [in] client_width  : current desktop width
/// @param [in] client_height : current desktop height
/// @param [in] flags         : SetWindowPos flags
void EMU_OSD::set_client_pos(int dest_x, int dest_y, int client_width, int client_height, UINT flags)
{
	WINDOWINFO wid = { sizeof(WINDOWINFO) };
	::GetWindowInfo(hMainWindow, &wid);

	int width = (wid.rcClient.left - wid.rcWindow.left) + client_width + (wid.rcWindow.right - wid.rcClient.right);
	int height = (wid.rcClient.top - wid.rcWindow.top) + client_height + (wid.rcWindow.bottom - wid.rcClient.bottom);

	flags |= SWP_NOZORDER;

#ifdef _DEBUG
	logging->out_debugf(_T("set_client_pos: x:%d y:%d w:%d h:%d flags:%x"), dest_x, dest_y, width, height, flags);
#endif
	::SetWindowPos(hMainWindow, HWND_TOP, dest_x, dest_y, width, height, flags);

	int prev_height = height;

	// If the window width is changed, the menu bar height may change and the window height also.
	// So, calculate the window height again to adjust the client area.
	::GetWindowInfo(hMainWindow, &wid);

	height = (wid.rcClient.top - wid.rcWindow.top) + client_height + (wid.rcWindow.bottom - wid.rcClient.bottom);
	if (prev_height != height) {
#ifdef _DEBUG
		logging->out_debugf(_T("set_client_pos again: x:%d y:%d w:%d h:%d flags:%x"), dest_x, dest_y, width, height, flags);
#endif
		::SetWindowPos(hMainWindow, HWND_TOP, dest_x, dest_y, width, height, flags);
	}
}

/// setting window or fullscreen size to display
/// @param [in] mode 0 .. 7 window mode / 8 .. 23 fullscreen mode / -1 want to go fullscreen, but unknown mode
/// @param [in] cur_width  current desktop width if mode is -1
/// @param [in] cur_height current desktop height if mode is -1
/// @param [in] dpi : dot per inch or 0
void EMU_OSD::set_window(int mode, int cur_width, int cur_height, int dpi)
{
	static LONG style = WS_VISIBLE;
	WINDOWPLACEMENT place;
	place.length = sizeof(WINDOWPLACEMENT);

	now_resizing = true;

	if (dpi <= 0) {
		dpi = WIN_API_EX::GetDpiForWindow(hMainWindow);
	}

	if(mode >= 0 && mode < 8) {
		// go window
		if (mode >= window_mode.Count()) mode = 0;
		const CWindowMode *wm = window_mode.Get(mode);
		int width = wm->width;
		int height = wm->height;

		if(now_screenmode == NOW_FULLSCREEN) {
			// rollback dpi of window mode
			dpi = window_dest_dpi;
		}
		width = width * dpi / USER_DEFAULT_SCREEN_DPI;
		height = height * dpi / USER_DEFAULT_SCREEN_DPI;

#ifdef USE_SCREEN_ROTATE
		if (pConfig->monitor_type & 1) {
			int v = width;
			width = height;
			height = v;
		}
#endif
//		RECT rect = {0, 0, width, height};
		RECT rect = {-display_margin.left, -display_margin.top, width + display_margin.right, height + display_margin.bottom};
//		AdjustWindowRectEx(&rect, dwStyle, TRUE, 0);
		int dest_x = 0;
		int dest_y = 0;

		if(now_screenmode == NOW_FULLSCREEN) {
			// change fullscreen to window
			ChangeDisplaySettings(NULL, 0);
			SetWindowLong(hMainWindow, GWL_STYLE, style);
			if (!first_change_screen) {
				// rollback position of window mode
				dest_x = (int)window_dest.x;
				dest_y = (int)window_dest.y;
			}
#ifdef _DEBUG
			logging->out_debugf(_T("set_window: f->w x:%d y:%d w:%d h:%d"), dest_x, dest_y, rect.right - rect.left, rect.bottom - rect.top);
#endif

			now_screenmode = NOW_WINDOW;
			// show menu
			gui->ScreenModeChanged(now_screenmode == NOW_FULLSCREEN);
			// resize client
			set_client_pos(dest_x, dest_y, rect.right - rect.left, rect.bottom - rect.top, SWP_SHOWWINDOW);
		}
		else {
			if (now_screenmode == NOW_MAXIMIZE) {
				// restore from maximize
				ShowWindow(hMainWindow, SW_RESTORE);
				now_screenmode = NOW_WINDOW;
			}
			// get current position of window
			WINDOWINFO wid = { sizeof(WINDOWINFO) };
			GetWindowInfo(hMainWindow, &wid);
			dest_x = wid.rcWindow.left;
			dest_y = wid.rcWindow.top;

#ifdef _DEBUG
			logging->out_debugf(_T("set_window: w->w x:%d y:%d w:%d h:%d"), dest_x, dest_y, rect.right - rect.left, rect.bottom - rect.top);
#endif
			// resize client
			set_client_pos(dest_x, dest_y, rect.right - rect.left, rect.bottom - rect.top, SWP_NOMOVE);
		}
		pConfig->window_mode = prev_window_mode = mode;
		pConfig->window_position_x = dest_x;
		pConfig->window_position_y = dest_y;
		pConfig->disp_device_no = 0;
		pConfig->screen_width = width;
		pConfig->screen_height = height;
		window_mode_magnify = wm->magnify;
		window_mode_magnify = window_mode_magnify * dpi / USER_DEFAULT_SCREEN_DPI;

		// set screen size to emu class
		set_display_size(width, height, window_mode_magnify, true);
//#ifdef USE_MOUSE_ABSOLUTE
//		set_mouse_position();
//#endif
	}
	else if(now_screenmode != NOW_FULLSCREEN) {
		// go fullscreen

		// get current position of window
		WINDOWINFO wid = { sizeof(WINDOWINFO) };
		GetWindowInfo(hMainWindow, &wid);
		window_dest.x = wid.rcWindow.left;
		window_dest.y = wid.rcWindow.top;
		window_dest_dpi = dpi;

		const CVideoMode *sm = NULL;
		const TCHAR *dev_name = NULL;
		int disp_no = 0;
		int width = 0;
		int height = 0;
		int left = 0;
		int top = 0;

		if (mode >= 8) {
			// check mode number is valid
			if (screen_mode.GetMode((mode - 8) / VIDEO_MODE_MAX, (mode - 8) % VIDEO_MODE_MAX) < 0) {
				mode = -1;
			}
		}

		if(mode == -1) {
			// search current monitor
			disp_no = screen_mode.WithinDisp(window_dest.x, window_dest.y);
			if (disp_no < 0) {
				disp_no = 0;
			}
			// get width and height
			const CDisplayDevice *dd = screen_mode.GetDisp(disp_no);
			dev_name = dd->name.Get();
			left = dd->re.x;
			top = dd->re.y;
			width = dd->re.w;
			height = dd->re.h;

			// matching width and height
			int find = screen_mode.FindMode(disp_no, width, height);
			if (find >= 0) {
				mode = find + (disp_no * VIDEO_MODE_MAX) + 8;
			} else {
				mode = 8;
			}
		} else {
			disp_no = (mode - 8) / VIDEO_MODE_MAX;
			sm = screen_mode.GetMode(disp_no, (mode - 8) % VIDEO_MODE_MAX);

			const CDisplayDevice *dd = screen_mode.GetDisp(disp_no);
			dev_name = dd->name.Get();
			left =  dd->re.x;
			top = dd->re.y;
			width = sm ? sm->width : dd->re.w;
			height = sm ? sm->height : dd->re.h;
		}

		DEVMODE dev;
		ZeroMemory(&dev, sizeof(dev));
		dev.dmSize = sizeof(dev);
		dev.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
		dev.dmBitsPerPel = desktop_bpp;
		dev.dmPelsWidth = width;
		dev.dmPelsHeight = height;

		// try fullscreen
		if(ChangeDisplaySettingsEx(dev_name, &dev, NULL, CDS_TEST, NULL) == DISP_CHANGE_SUCCESSFUL) {
			if (now_screenmode == NOW_MAXIMIZE) {
				// restore from maximize
				ShowWindow(hMainWindow, SW_RESTORE);
			}
			GetWindowPlacement(hMainWindow, &place);
			ChangeDisplaySettingsEx(dev_name, &dev, NULL, CDS_FULLSCREEN, NULL);
			style = GetWindowLong(hMainWindow, GWL_STYLE);
			SetWindowLong(hMainWindow, GWL_STYLE, WS_VISIBLE);
			SetWindowPos(hMainWindow, HWND_TOP, left, top, width, height, SWP_SHOWWINDOW);
			SetCursorPos(width / 2, height / 2);
			prev_screenmode = now_screenmode;
			now_screenmode = NOW_FULLSCREEN;

			pConfig->window_mode = mode;
			pConfig->disp_device_no = disp_no;
			pConfig->screen_width = width;
			pConfig->screen_height = height;

			// remove menu
			gui->ScreenModeChanged(true);

			// set screen size to emu class
			set_display_size(width, height, 1.0, false);

//#ifdef USE_MOUSE_ABSOLUTE
//			set_mouse_position();
//#endif
		}
	} else {
		// now fullscreen
		if (mode >= screen_mode.CountMode(0) + 8) {
			mode = -1;
		}
		const CVideoMode *sm = screen_mode.GetMode(0, mode - 8);
		int width = sm ? sm->width : cur_width;
		int height = sm ? sm->height : cur_height;

		logging->out_debugf(_T("set_window: f->f mode:%d w:%d h:%d nf:%d"), mode, width, height, (int)now_screenmode);

		// set screen size to emu class
		set_display_size(width, height, 1.0, false);
	}

	if (msgboard) {
		msgboard->SetFont(hMainWindow, false);
	}

	first_change_screen = false;
	now_resizing = false;
//	mute_sound(false);
	set_pause(1, false);
}

void EMU_OSD::change_screen_resolution(int x, int y, int width, int height, int dpi)
{
	static bool now_dpi_changing = false;

	if (now_dpi_changing) return;
	if (now_resizing) return;

	now_dpi_changing = true;

	WINDOWINFO wid = { sizeof(WINDOWINFO) };
	::GetWindowInfo(hMainWindow, &wid);

	logging->out_logf(LOG_DEBUG, _T("DPI_CHANGED: Curr(x:%d y:%d w:%d h:%d bx:%d by:%d) -> Event(x:%d y:%d w:%d h:%d dpi:%d)")
		, wid.rcWindow.left, wid.rcWindow.top
		, wid.rcWindow.right - wid.rcWindow.left, wid.rcWindow.bottom - wid.rcWindow.top
		, wid.cxWindowBorders, wid.cyWindowBorders
		, x, y, width, height, dpi
	);
#if 0
	// calc client size in the window
	int cw = wid.rcWindow.right - wid.rcWindow.left;
	int cx = (cw - width) / 2 + wid.rcWindow.left;
	if (cw < width) {
		if (cx > x) x = cx;
	} else {
		if (cx < x) x = cx;
	}

	logging->out_logf(LOG_DEBUG, _T("DPI_CHANGED: New Pos cx:%d x:%d"), cx, x);
#endif

	// move window at first
	::SetWindowPos(hMainWindow, HWND_TOP, x, y, width, height, SWP_NOZORDER);

	set_window(pConfig->window_mode, desktop_size.w, desktop_size.h, dpi);

	now_dpi_changing = false;
}

// ---------------------------------------------------------------------------

#ifdef USE_DIRECT2D
void EMU_OSD::initialize_d2dfactory(HWND hWnd)
{
	HRESULT hre = S_FALSE;

	// D2D
	hre = gD2DFactory.CreateFactory();
	if (hre == S_OK) {
		logging->out_log(LOG_DEBUG, _T("Enable Direct2D"));
		enabled_drawing_method |= DRAWING_METHOD_DIRECT2D_MASK;
	} else {
		logging->out_log(LOG_DEBUG, _T("Disable Direct2D"));
		enabled_drawing_method &= ~DRAWING_METHOD_DIRECT2D_MASK;
		return;
	}

	pD2Dorigin = new CD2DSurface();
#ifdef USE_SCREEN_MIX_SURFACE
	pD2Dmixrender = new CD2DBitmapRender();
#endif

}

void EMU_OSD::create_d2drender(HWND hWnd)
{
	HRESULT hre = S_FALSE;

	hre = mD2DRender.CreateRender(gD2DFactory, hWnd, display_size.w, display_size.h,
		(pConfig->drawing_method & DRAWING_METHOD_SYNC_MASK) != 0 ? 1 : 0
	);
	if (hre != S_OK) {
		logging->out_log(LOG_DEBUG, _T("D2DRender::CreateRender failed."));
		enabled_drawing_method &= ~DRAWING_METHOD_DIRECT2D_MASK;
	}
	if (!(enabled_drawing_method & DRAWING_METHOD_DIRECT2D_MASK)) {
		// change default drawing
		pConfig->drawing_method = DRAWING_METHOD_DEFAULT_AS;
		return;
	}
}

void EMU_OSD::create_d2dofflinesurface()
{
	HRESULT hre;

	if (!pD2Dorigin) return;

	do {
		pD2Dorigin->ReleaseSurface();
		hre = pD2Dorigin->CreateSurface(mD2DRender, screen_size.w, screen_size.h);
		if (hre != S_OK) {
			break;
		}
		hre = create_d2dmixedsurface();
		if (hre != S_OK) {
			break;
		}

#ifdef USE_SCREEN_ROTATE
		if (pD2Drotate) {
			pD2Drotate->ReleaseSurface();
			hre = pD2Drotate->CreateSurface(mD2DRender, source_size.w, source_size.h);
			if (hre != S_OK) {
				break;
			}
		}
		if(pConfig->monitor_type) {
			pD2Dsource = pD2Drotate;
		} else
#endif
		{
			pD2Dsource = pD2Dorigin;
		}

#ifdef USE_LEDBOX
		ledbox->CreateD2DSurface(mD2DRender);
#endif
#ifdef USE_MESSAGE_BOARD
		msgboard->CreateD2DSurface(gD2DFactory, mD2DRender);
#endif
	} while(0);

	if (hre != S_OK) {
		release_d2drender();
	}
}

HRESULT EMU_OSD::create_d2dmixedsurface()
{
	HRESULT hre = S_OK;
#ifdef USE_SCREEN_MIX_SURFACE
	if (pD2Dmixrender) {
		pD2Dmixrender->ReleaseRender();
		hre = pD2Dmixrender->CreateRender(mD2DRender, display_size.w, display_size.h);
	}
#endif
	return hre;
}

void EMU_OSD::reset_d2drender(HWND hWnd)
{
	// re create
	release_d2drender();
	create_d2drender(hWnd);
	create_d2dofflinesurface();
}

void EMU_OSD::release_d2drender()
{
#ifdef USE_SCREEN_ROTATE
	if (pD2Drotate) pD2Drotate->ReleaseSurface();
#endif
	pD2Dsource = NULL;

	if (pD2Dorigin) pD2Dorigin->ReleaseSurface();
//	lpD2DBmp = NULL;

#ifdef USE_LEDBOX
	ledbox->ReleaseD2DSurface();
#endif
#ifdef USE_MESSAGE_BOARD
	msgboard->ReleaseD2DSurface();
#endif

#ifdef USE_SCREEN_MIX_SURFACE
	if (pD2Dmixrender) pD2Dmixrender->ReleaseRender();
#endif

	mD2DRender.ReleaseRender();
}

void EMU_OSD::terminate_d2dfactory()
{
	if (pD2Dorigin) {
		delete pD2Dorigin;
		pD2Dorigin = NULL;
	}
#ifdef USE_SCREEN_MIX_SURFACE
	if (pD2Dmixrender) {
		delete pD2Dmixrender;
		pD2Dmixrender = NULL;
	}
#endif
	gD2DFactory.ReleaseFactory();
}
#endif /* USE_DIRECT2D */

// ---------------------------------------------------------------------------

#ifdef USE_DIRECT3D
void EMU_OSD::initialize_d3device(HWND hWnd)
{
	if (!mD3DDevice.Load()) {
		logging->out_log(LOG_DEBUG, _T("Disable Direct3D"));
		enabled_drawing_method &= ~DRAWING_METHOD_DIRECT3D_MASK;
		return;
	}

	logging->out_log(LOG_DEBUG, _T("Enable Direct3D"));
	enabled_drawing_method |= DRAWING_METHOD_DIRECT3D_MASK;

	set_d3dpresent_interval();

#ifdef USE_SCREEN_D3D_TEXTURE
	pD3Dorigin = new CD3DTexture();
#else
	pD3Dorigin = new CD3DSurface();
#ifdef USE_SCREEN_D3D_MIX_SURFACE
	pD3Dmixsuf = new CD3DSurface();
#endif
	pD3Dbacksuf = new CD3DSurface();
#endif

}

void EMU_OSD::create_d3device(HWND hWnd)
{
	HRESULT hre;

	hre = mD3DDevice.CreateDevice(hWnd);
	if (hre != D3D_OK) {
		// disable direct 3d
		logging->out_log(LOG_DEBUG, _T("D3DDevice::CreateDeivce failed."));
		enabled_drawing_method &= ~DRAWING_METHOD_DIRECT3D_MASK;
	}

	if (!(enabled_drawing_method & DRAWING_METHOD_DIRECT3D_MASK)) {
		// change default drawing
		pConfig->drawing_method = DRAWING_METHOD_DEFAULT_AS;
		return;
	}

	if (!mD3DDevice.IsEnableScanlineOrInterval()) {
		// scanline and interval one is unsupport
		if (pConfig->drawing_method == DRAWING_METHOD_DIRECT3D_S) {
			pConfig->drawing_method = DRAWING_METHOD_DIRECT3D_AS;
		}
	}
}

void EMU_OSD::create_d3dofflinesurface()
{
	HRESULT hre;
	if (!pD3Dorigin) return;

	do {
#ifdef USE_SCREEN_D3D_TEXTURE
		pD3Dorigin->ReleaseD3DTexture();
		lpD3DBmp = NULL;

		hre = pD3Dorigin->CreateD3DTexture(mD3DDevice, screen_size.w, screen_size.h);
		if (hre != D3D_OK) {
			break;
		}
		// set position
		pD3Dorigin->SetD3DTexturePosition(reD3Dply);
		// create polygon structure
		set_d3d_viewport();

#ifdef USE_LEDBOX
		ledbox->CreateTexture(mD3DDevice);
		gui->ChangeLedBoxPosition(pConfig->led_pos);
#endif
#ifdef USE_MESSAGE_BOARD
		msgboard->CreateTexture(mD3DDevice);
#endif

#else /* !USE_SCREEN_D3D_TEXTURE */
		pD3Dorigin->ReleaseD3DSurface();
		lpD3DBmp = NULL;

//		hre = pD3Dorigin->CreateD3DMemorySurface(mD3DDevice, screen_size.w, screen_size.h);
		hre = pD3Dorigin->CreateD3DSurface(mD3DDevice, screen_size.w, screen_size.h);
		if (hre != D3D_OK) {
			break;
		}
		hre = create_d3dmixedsurface();
		if (hre != D3D_OK) {
			break;
		}
#endif /* USE_SCREEN_D3D_TEXTURE */

#ifdef USE_SCREEN_ROTATE
		pD3Drotate->ReleaseD3DSurface();

		hre = pD3Drotate->CreateD3DMemorySurface(mD3DDevice, source_size.w, source_size.h);
		if (hre != D3D_OK) {
			break;
		}

		if(pConfig->monitor_type) {
			pD3Dsource = pD3Drotate;
		} else
#endif
		{
			pD3Dsource = pD3Dorigin;
		}
	} while(0);

	if (hre != D3D_OK) {
		release_d3device();
	}
}

HRESULT EMU_OSD::create_d3dmixedsurface()
{
	HRESULT hre = D3D_OK;
#ifdef USE_SCREEN_D3D_MIX_SURFACE
	if (!pD3Dmixsuf) return hre;

	pD3Dmixsuf->ReleaseD3DSurface();

	hre = pD3Dmixsuf->CreateD3DSurface(mD3DDevice, display_size.w, display_size.h);
#endif
	return hre;
}

HRESULT EMU_OSD::reset_d3device(HWND hWnd)
{
	HRESULT hre = D3D_OK;

	hre = mD3DDevice.ResetDevice();
	if (hre == D3DERR_DEVICELOST) {
		return hre;
	}
	if (hre != D3D_OK) {
		// re create
		release_d3device();
		create_d3device(hWnd);
		create_d3dofflinesurface();
	}
	return hre;
}

void EMU_OSD::release_d3device()
{
#ifdef USE_SCREEN_ROTATE
	if (pD3Drotate) pD3Drotate->ReleaseD3DSurface();
#endif
	pD3Dsource = NULL;

#ifdef USE_SCREEN_D3D_TEXTURE
	if (pD3Dorigin) pD3Dorigin->ReleaseD3DTexture();
	lpD3DBmp = NULL;
#ifdef USE_LEDBOX
	ledbox->ReleaseTexture();
#endif
#ifdef USE_MESSAGE_BOARD
	msgboard->ReleaseTexture();
#endif
#else
	if (pD3Dorigin) pD3Dorigin->ReleaseD3DSurface();
	lpD3DBmp = NULL;

#ifdef USE_SCREEN_D3D_MIX_SURFACE
	if (pD3Dmixsuf) pD3Dmixsuf->ReleaseD3DSurface();
#endif
	if (pD3Dbacksuf) pD3Dbacksuf->ReleaseD3DSurface();
#endif
	mD3DDevice.ReleaseDevice();
}

void EMU_OSD::terminate_d3device()
{
	delete pD3Dorigin;
	pD3Dorigin = NULL;
#ifndef USE_SCREEN_D3D_TEXTURE
#ifdef USE_SCREEN_D3D_MIX_SURFACE
	delete pD3Dmixsuf;
	pD3Dmixsuf = NULL;
#endif
	delete pD3Dbacksuf;
	pD3Dbacksuf = NULL;
#endif
	mD3DDevice.Unload();
}

void EMU_OSD::set_d3dpresent_interval()
{
	mD3DDevice.SetPresentationInterval(pConfig->drawing_method);
}

#ifdef USE_SCREEN_D3D_TEXTURE
void EMU_OSD::set_d3d_viewport()
{
	// view port
	mD3DDevice.SetViewport(display_size.w, display_size.h);
}
#endif

#endif // USE_DIRECT3D
