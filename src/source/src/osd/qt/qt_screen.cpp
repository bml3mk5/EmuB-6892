/** @file qt_screen.cpp

	Skelton for retropc emulator
	Qt edition

	@author Sasaji
	@date   2016.11.11

	@brief [ qt screen ]

	@note
	This code is based on win32_screen.cpp of the Common Source Code Project.
	Author : Takeda.Toshiya
*/

#include "qt_emu.h"
#include "../../vm/vm_defs.h"
#include <QVariant>
#include "../../gui/gui.h"
#include "../../config.h"
#ifdef USE_LEDBOX
#include "../../gui/ledbox.h"
#endif
#ifdef USE_MESSAGE_BOARD
#include "qt_msgboard.h"
#endif
#include "qt_csurface.h"
#include "../../video/rec_video.h"
#include "qt_utils.h"
#include <QPainter>

#ifdef USE_OPENGL
#define FLG_MSK_OPENGL 0x0100
#include <QOpenGLWidget>
#include <QOpenGLContext>
#include "../opengl.h"
#endif

//static int window_pos_x = -1;
//static int window_pos_y = -1;

void EMU_OSD::EMU_SCREEN()
{
	texSource = nullptr;
	texMixed = nullptr;
//#ifdef USE_SCREEN_QT_MIX_ON_RENDERER
//	texLedBox = nullptr;
//	texMsgBoard = nullptr;
//#endif

	screen = nullptr;
	m_screen_flags = 0;

#ifdef USE_OPENGL
	texGLMixed = nullptr;
//	mix_texture_name = 0;
	src_tex_l = src_tex_t = 0.0;
	src_tex_r = src_tex_b = 1.0;
	src_pyl_l = src_pyl_t = -1.0;
	src_pyl_r = src_pyl_b = 1.0;
	opengl = nullptr;
//	next_use_opengl = 0;
	mixed_max_size = desktop_size;

#ifdef USE_SCREEN_OPENGL_MIX_ON_RENDERER
	sufGLMain = nullptr;
	texGLLedBox = nullptr;
	texGLMsgBoard = nullptr;
#endif
#endif /* USE_OPENGL */

#ifdef USE_LEDBOX
	ledbox = nullptr;
#endif
}

///
/// initialize screen
///
void EMU_OSD::initialize_screen()
{
	EMU::initialize_screen();

#ifdef USE_OPENGL
	initialize_opengl();
#endif

#ifdef USE_MESSAGE_BOARD
	msgboard = new MsgBoard(this);
	if (msgboard) {
		msgboard->InitScreen(pixel_format, screen_size.w, screen_size.h);
		msgboard->SetMessagePos(4 + reMix.x, -4 - reMix.y, 2);
		msgboard->SetInfoPos(-4 - reMix.x, 4 + reMix.y, 1);
		msgboard->SetVisible(FLG_SHOWMSGBOARD ? true : false);
	}
#endif

#ifdef USE_LEDBOX
	if (gui) {
		ledbox = gui->CreateLedBox(res_path.Get(), pixel_format);
	}
#endif
}

///
/// release screen
///
void EMU_OSD::release_screen()
{
#ifdef USE_OPENGL
	release_opengl();
	terminate_opengl();
#endif

	if (gui) {
		gui->ReleaseLedBox();

		gui->RestoreDrawingMethod(pConfig->drawing_method);
	}

	EMU::release_screen();

	release_qt_texture();
}

///
/// create window
///
bool EMU_OSD::create_screen(int UNUSED_PARAM(disp_no), int x, int y, int width, int height, uint32_t flags)
{
	bool rc = true;
//	uint32_t set_flags = 0;
	VmRect margin = { 0, 0, 0, 0 };

	if (!mainwindow) {
		mainwindow = new MainWindow(nullptr);
		set_qt_window(mainwindow);

		if (screen_mode.WithinDisp(x, y) >= 0) {
			mainwindow->move(x, y);
		}

		mainwindow->show();
	}

	lock_screen();

	if (now_screenmode != NOW_FULLSCREEN) {
		margin = display_margin;
	}
#ifdef USE_OPENGL
	if (pConfig->drawing_method & DRAWING_METHOD_OPENGL_MASK) {
		flags |= FLG_MSK_OPENGL;
	}
	enabled_drawing_method |= DRAWING_METHOD_OPENGL_MASK;
#endif
	if (screen != nullptr) {
		delete screen;
	}
	screen = new QRect(0, 0, margin.left + width + margin.right, margin.top + height + margin.bottom);
	if (screen == nullptr) {
		logging->out_log(LOG_ERROR, _T("EMU_OSD::create_screen new QImage failed."));
		rc = false;
		goto FIN;
	}
	m_screen_flags = flags;

#ifdef _DEBUG_LOG
	logging->out_debugf(_T("screen_flags:0x%x"), m_screen_flags);
#endif
	// create gui window
	if (gui->CreateWidget(screen, screen->width(), screen->height()) == -1) {
		rc = false;
		goto FIN;
	}

FIN:
	unlock_screen();

	return rc;
}

///
/// create / recreate offline surface
///
bool EMU_OSD::create_offlinesurface()
{
	if (!pixel_format) return false;
	if (!screen) return true;

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

	if (!create_qt_texture()) {
		return false;
	}

	disable_screen &= ~DISABLE_SURFACE;

	return true;
}

bool EMU_OSD::create_mixedsurface()
{
	if (sufMixed) {
		sufMixed->Release();
#ifdef USE_SCREEN_MIX_SURFACE
		if (!sufMixed->Create(display_size.w, display_size.h, *pixel_format)) {
			logging->out_log(LOG_ERROR, _T("EMU_OSD::create_mixedsurface failed."));
			return false;
		}
#endif
	}
	return true;
}

void EMU_OSD::set_screen_filter_type()
{
	set_opengl_filter_type();
}

bool EMU_OSD::create_qt_texture()
{
	{
		if (!texSource) {
			texSource = new CSurface(screen_size.w, screen_size.h);
			if (!texSource) {
				logging->out_log(LOG_ERROR, _T("create_wx_texture: texSource failed."));
				return false;
			}
		}

		if (!create_qt_mixedtexture()) {
			return false;
		}

	}
	return true;
}

bool EMU_OSD::create_qt_mixedtexture()
{
	{
		delete texMixed;
		texMixed = new CSurface(display_size.w, display_size.h);
		if (!texMixed) {
			logging->out_log(LOG_ERROR, _T("create_wx_mixedtexture: texMixed failed."));
			return false;
		}
	}
	return true;
}

void EMU_OSD::reset_qt_texture()
{
	release_qt_texture();
	create_qt_texture();
}

void EMU_OSD::release_qt_texture()
{
	delete texMixed;
	texMixed = NULL;

	delete texSource;
	texSource = NULL;
}

/// setting window or fullscreen size
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

		} else {
			// fit text area to full screen (cut off padding area)
			VmSize min_size;
			SIZE_IN(min_size, LIMIT_MIN_WINDOW_WIDTH, LIMIT_MIN_WINDOW_HEIGHT)
#ifdef USE_SCREEN_ROTATE
			if(pConfig->monitor_type & 1) {
				SWAP(int, min_size.w, min_size.h);
			}
#endif
			VmSize mixed_rsize, mixed_rrsize;
			mixed_size = source_size;
			SIZE_IN(mixed_rsize, mixed_size.w, mixed_size.h)
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

			double magx = static_cast<double>(display_size.w) / min_rsize.w;
			double magy = static_cast<double>(display_size.h) / min_rsize.h;

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
					stretched_dest_real.y = (display_size.h - (mixed_size.h * display_size.w / min_rsize.w)) / 2;
				} else {
					int mh = mixed_rrsize.h;
					int sh = mixed_size.h * display_size.w / min_rsize.w;
					mixed_size.y = (mixed_size.h - mh) / 2;
					mixed_size.h = mh;
					stretched_size.y = (display_size.h - sh) / 2;
					stretched_size.h = sh;
					stretched_dest_real.y = stretched_size.y;
				}

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
				} else {
					int mw = mixed_rrsize.w;
					int sw = mixed_size.w * display_size.h / min_rsize.h;
					mixed_size.x = (mixed_size.w - mw) / 2;
					mixed_size.w = mw;
					stretched_size.x = (display_size.w - sw) / 2;
					stretched_size.w = sw;
					stretched_dest_real.x = stretched_size.x;
				}

			}
		}
	}
	// window or non-streach mode
	else {
		for(int n = 0; n <= 1; n++) {
			if (n == 0) {
				mixed_size.w = static_cast<int>(static_cast<double>(display_size.w) / magnify + 0.5);
				mixed_size.h = static_cast<int>(static_cast<double>(display_size.h) / magnify + 0.5);
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

		stretched_size.w = static_cast<int>(static_cast<double>(source_aspect_size.w) * magnify + 0.5);
		stretched_size.h = static_cast<int>(static_cast<double>(source_aspect_size.h) * magnify + 0.5);
		stretched_size.x = (display_size.w - stretched_size.w) / 2;
		stretched_size.y = (display_size.h - stretched_size.h) / 2;
		stretched_dest_real.x = static_cast<int>(static_cast<double>(- mixed_size.x) * magnify + 0.5);
		stretched_dest_real.y = static_cast<int>(static_cast<double>(- mixed_size.y) * magnify + 0.5);
		if (mixed_ratio.w < mixed_ratio.h) {
			stretched_dest_real.y = stretched_dest_real.y * mixed_ratio.h / mixed_ratio.w;
		} else {
			stretched_dest_real.x = stretched_dest_real.x * mixed_ratio.w / mixed_ratio.h;
		}
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

	// sufMixed size normalize 2^n for opengl
	mixed_max_size.w = 0;
	mixed_max_size.h = 0;
	// 65536,32768,16384,8192,4096,2048,1024,512,256,128,64,32,16
	for(int i=65536; i>=16; i = (i >> 1)) {
		if (source_size.w <= i) mixed_max_size.w = i;
		if (source_size.h <= i) mixed_max_size.h = i;
	}

	reMix = mixed_size;
	reSuf = stretched_size;

#ifdef USE_OPENGL
	src_tex_l = static_cast<GLfloat>(mixed_size.x) / mixed_max_size.w;
	src_tex_t = static_cast<GLfloat>(mixed_size.y) / mixed_max_size.h;
	src_tex_r = static_cast<GLfloat>(mixed_size.x + mixed_size.w) / mixed_max_size.w;
	src_tex_b = static_cast<GLfloat>(mixed_size.y + mixed_size.h) / mixed_max_size.h;
	if (stretched_size.w < display_size.w) src_pyl_r = static_cast<GLfloat>(stretched_size.w) / display_size.w;
	else src_pyl_r = 1.0f;
	if (stretched_size.h < display_size.h) src_pyl_b = static_cast<GLfloat>(stretched_size.h) / display_size.h;
	else src_pyl_b = 1.0f;
	src_pyl_l = -src_pyl_r;
	src_pyl_t = src_pyl_b;
	src_pyl_b = -src_pyl_b;

	rePyl.left = stretched_size.x;
	rePyl.top = stretched_size.y;
	rePyl.right = stretched_size.x + stretched_size.w;
	rePyl.bottom = stretched_size.y + stretched_size.h;
#endif

	change_rec_video_size(pConfig->screen_video_size);

	first_invalidate = true;
	screen_size_changed = false;
#ifdef _DEBUG_LOG
	logging->out_debugf(_T("set_display_size: w:%d h:%d magnify:%.1f %s"), width, height, magnify, now_window ? _T("window") : _T("fullscreen"));
	logging->out_debugf(_T("         display: w:%d h:%d"), display_size.w, display_size.h);
	logging->out_debugf(_T("          screen: w:%d h:%d"), screen_size.w, screen_size.h);
	logging->out_debugf(_T("   screen aspect: w:%d h:%d"), screen_aspect_size.w, screen_aspect_size.h);
	logging->out_debugf(_T("          source: w:%d h:%d"), source_size.w, source_size.h);
	logging->out_debugf(_T("   source aspect: w:%d h:%d"), source_aspect_size.w, source_aspect_size.h);
	logging->out_debugf(_T("           mixed: w:%d h:%d"), mixed_size.w, mixed_size.h);
	logging->out_debugf(_T("       mixed max: w:%d h:%d"), mixed_max_size.w, mixed_max_size.h);
	logging->out_debugf(_T("         stretch: w:%d h:%d"), stretched_size.w, stretched_size.h);
	logging->out_debugf(_T("     screen dest: x:%d y:%d"), screen_size.x, screen_size.y);
	logging->out_debugf(_T("     source dest: x:%d y:%d"), source_size.x, source_size.y);
	logging->out_debugf(_T("      mixed dest: x:%d y:%d"), mixed_size.x, mixed_size.y);
	logging->out_debugf(_T("    stretch dest: x:%d y:%d"), stretched_size.x, stretched_size.y);
	logging->out_debugf(_T(" stretch dest re: x:%d y:%d"), stretched_dest_real.x, stretched_dest_real.y);
#ifdef USE_OPENGL
	logging->out_debugf(_T("     src polygon: l:%d t:%d r:%d b:%d"), rePyl.left, rePyl.top, rePyl.right, rePyl.bottom);
	logging->out_debugf(_T("     src polygon: l:%.5f t:%.5f r:%.5f b:%.5f"), src_pyl_l, src_pyl_t, src_pyl_r, src_pyl_b);
	logging->out_debugf(_T("     src texture: l:%.5f t:%.5f r:%.5f b:%.5f"), src_tex_l, src_tex_t, src_tex_r, src_tex_b);
#endif
#endif

	calc_vm_screen_size();

	if (now_window) {
		stretched_size.x += display_margin.left;
		stretched_size.y += display_margin.top;
		reSuf.x += display_margin.left;
		reSuf.y += display_margin.top;
#ifdef _DEBUG_LOG
		logging->out_debugf(_T(" margin      suf: l:%d t:%d w:%d h:%d"),reSuf.x,reSuf.y,reSuf.w,reSuf.h);
#endif
#ifdef USE_OPENGL
//		reTex.x += display_margin.left;
//		reTex.y += display_margin.top;
		rePyl.left += display_margin.left;
		rePyl.top += display_margin.top;
		rePyl.right += display_margin.left;
		rePyl.bottom += display_margin.top;
#ifdef _DEBUG_LOG
		logging->out_debugf(_T("     src polygon: l:%d t:%d r:%d b:%d"), rePyl.left, rePyl.top, rePyl.right, rePyl.bottom);
		logging->out_debugf(_T("     src polygon: l:%.5f t:%.5f r:%.5f b:%.5f"), src_pyl_l, src_pyl_t, src_pyl_r, src_pyl_b);
		logging->out_debugf(_T("     src texture: l:%.5f t:%.5f r:%.5f b:%.5f"), src_tex_l, src_tex_t, src_tex_r, src_tex_b);
#endif
#endif
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

#ifdef USE_SCREEN_ROTATE
	if ((source_size.w != prev_source_size.w && source_size.h != prev_source_size.h) || stretch_changed) {
		create_offlinesurface();
	}
#endif
	create_mixedsurface();

	create_qt_mixedtexture();

	// send display size to vm
	set_vm_display_size();

	set_ledbox_position(now_window);

	set_msgboard_position();

	unlock_screen();
}

void EMU_OSD::set_ledbox_position(bool now_window)
{
#ifdef USE_LEDBOX
	if (gui) {
#ifdef USE_OPENGL
		if (pConfig->drawing_method & DRAWING_METHOD_OPENGL_MASK) {
#ifdef USE_SCREEN_OPENGL_MIX_ON_RENDERER
			gui->SetLedBoxPosition(now_window, 0, 0, display_size.w, display_size.h, pConfig->led_pos | (is_fullscreen() ? 0x10 : 0));
#else
			gui->SetLedBoxPosition(now_window, mixed_size.x, mixed_size.y, mixed_size.w, mixed_size.h, pConfig->led_pos | (is_fullscreen() ? 0x10 : 0));
#endif
		} else
#endif
		{
			gui->SetLedBoxPosition(now_window, 0, 0, display_size.w, display_size.h, pConfig->led_pos | (is_fullscreen() ? 0x10 : 0));
		}
	}
#endif
}

void EMU_OSD::set_msgboard_position()
{
#ifdef USE_MESSAGE_BOARD
	if (msgboard) {
#ifdef USE_OPENGL
		if (pConfig->drawing_method & DRAWING_METHOD_OPENGL_MASK) {
#ifdef USE_SCREEN_OPENGL_MIX_ON_RENDERER
			msgboard->SetSize(display_size.w, display_size.h);
			msgboard->SetMessagePos(4, -4, 2);
			msgboard->SetInfoPos(-4, 4, 1);
#else
			msgboard->SetSize(source_size.w, source_size.h);
			msgboard->SetMessagePos(4 + mixed_size.x,  - 4 - source_size.h + mixed_size.y + mixed_size.h, 2);
			msgboard->SetInfoPos(-4 - mixed_size.x, 4 + mixed_size.y, 1);
#endif
		} else
#endif
		{
			msgboard->SetSize(display_size.w, display_size.h);
			msgboard->SetMessagePos(4, -4, 2);
			msgboard->SetInfoPos(-4, 4, 1);
		}
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

	if (sufOrigin->Lock()) {
		if (!pConfig->now_power_off) {
			vm->draw_screen();
		} else {
			fill_gray();
		}
		sufOrigin->Unlock();
	}

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
		{
			int nw0 = rotate_matrix[rtype].x[0] < 0 ? (ss.w - 1) : 0;
			int nh0 = rotate_matrix[rtype].y[0] < 0 ? (ss.h - 1) : 0;
			int nw1 = rotate_matrix[rtype].x[1] < 0 ? (ss.w - 1) : 0;
			int nh1 = rotate_matrix[rtype].y[1] < 0 ? (ss.h - 1) : 0;
			for(int sy = 0; sy < ss.h; sy++) {
				scrntype* src = sufOrigin->GetBuffer() + ss.w * sy;
				for(int sx = 0; sx < ss.w; sx++) {
					int dx = nw0 + rotate_matrix[rtype].x[0] * sx + nh0 + rotate_matrix[rtype].y[0] * sy; 
					int dy = nw1 + rotate_matrix[rtype].x[1] * sx + nh1 + rotate_matrix[rtype].y[1] * sy;
					scrntype* dst = sufRotate->GetBuffer() + ds.w * dy + dx;
					*dst = *src;
					src++;
				}
			}
		}
	}
#endif

#if 0
	{
		if (FLG_SHOWLEDBOX && ledbox) {
			ledbox->Draw(sufSource->Get());
		}
	}
#endif

	unlock_screen();
}

///
/// copy src screen to mix screen
///
void EMU_OSD::mix_screen_sub()
{
#ifdef USE_SCREEN_MIX_SURFACE
	lock_screen();

	sufSource->StretchBlit(vm_screen_size, *sufMixed, stretched_size);

	unlock_screen();

#ifdef USE_LEDBOX
	if (FLG_SHOWLEDBOX && ledbox) {
		ledbox->Draw(*sufMixed);
	}
#endif

#ifdef USE_MESSAGE_BOARD
	if (msgboard && FLG_SHOWMSGBOARD) {
		msgboard->Draw(*sufMixed);
	}
#endif
#endif /* USE_SCREEN_MIX_SURFACE */
}

///
/// render screen and display window
///
/// @note must be called by main thread
void EMU_OSD::update_screen_pa(QPainter *painter)
{
	if (disable_screen) return;

	if (gui) {
		gui->UpdateIndicator(update_led());
	}

#ifdef USE_SCREEN_MIX_SURFACE
	if (pConfig->drawing_method & DRAWING_METHOD_DBUFFER_MASK) {
		// double buffering
		mix_screen_sub();

		// fill black on screen
		if (first_invalidate) {
			first_invalidate = false;
		}
		// render screen
		painter->fillRect(*screen, Qt::black);
		painter->drawImage(0, 0, *(sufMixed->Get()), 0, 0, display_size.w, display_size.h, Qt::ThresholdDither);

#if 0
		if(stretched_size.w == mixed_size.w && stretched_size.h == mixed_size.h) {
		} else {
			// fit to window frame size
			QRect rm(reMix.x, reMix.y, reMix.w, reMix.h);
			QRect rs(reSuf.x, reSuf.y, reSuf.w, reSuf.h);
			painter->fillRect(*screen, Qt::black);
			painter->drawImage(rs, *(sufMixed->Get()), rm);
		}
#endif
	} else
#endif /* USE_SCREEN_MIX_SURFACE */
	{
		if (first_invalidate) {
			// fill black on screen
			first_invalidate = false;
		}

		lock_screen();

		QRect rsrc(vm_screen_size.x, vm_screen_size.y, vm_screen_size.w, vm_screen_size.h);
		QRect rdst(stretched_size.x, stretched_size.y, stretched_size.w, stretched_size.h);
		painter->drawImage(rdst, *(sufSource->Get()), rsrc);

		unlock_screen();

#ifdef USE_LEDBOX
		if (FLG_SHOWLEDBOX && ledbox) {
			ledbox->Draw(painter);
		}
#endif

#ifdef USE_MESSAGE_BOARD
		if (FLG_SHOWMSGBOARD && msgboard) {
			msgboard->Draw(painter);
		}
#endif
	}
	// invalidate window
	self_invalidate = true;
	skip_frame = false;
}

///
/// update GLContext
///
void EMU_OSD::update_screen_gl(QOpenGLContext *UNUSED_PARAM(context))
{
	/* OpenGL ********************/
	if (disable_screen) return;

	if (gui) {
		gui->UpdateIndicator(update_led());
	}

#ifndef USE_SCREEN_OPENGL_MIX_ON_RENDERER
	if (texGLMixed) {
		mix_screen_sub();

		if (first_invalidate) {
#ifdef USE_OPENGL_WH_ORTHO
			if (opengl->Version() <= 1) {
				src_pyl_l = (GLfloat)rePyl.left;
				src_pyl_t = (GLfloat)rePyl.top;
				src_pyl_r = (GLfloat)rePyl.right;
				src_pyl_b = (GLfloat)rePyl.bottom;
			}
#endif
			texGLMixed->SetPos(src_pyl_l, src_pyl_t, src_pyl_r, src_pyl_b, src_tex_l, src_tex_t, src_tex_r, src_tex_b);
			first_invalidate = false;
		}

		// draw texture using screen pixel buffer
		texGLMixed->Draw(sufMixed->Width(), sufMixed->Height(), sufMixed->GetBuffer());

		if (!self_invalidate) {
			// call this only once at first.
			change_opengl_attr();
		}
	}

#else /*  USE_SCREEN_OPENGL_MIX_ON_RENDERER */
	if (texGLMixed) {
		lock_screen();

		VmRectWH re;
		RECT_IN(re, 0, 0, vm_screen_size.x + vm_screen_size.w, vm_screen_size.y + vm_screen_size.h);
		sufSource->Blit(*sufGLMain, re);

		unlock_screen();

		if (first_invalidate) {
#ifdef USE_OPENGL_WH_ORTHO
			if (opengl->Version() <= 1) {
				src_pyl_l = static_cast<GLfloat>(rePyl.left);
				src_pyl_t = static_cast<GLfloat>(rePyl.top);
				src_pyl_r = static_cast<GLfloat>(rePyl.right);
				src_pyl_b = static_cast<GLfloat>(rePyl.bottom);
			}
#endif
			texGLMixed->SetPos(src_pyl_l, src_pyl_t, src_pyl_r, src_pyl_b, src_tex_l, src_tex_t, src_tex_r, src_tex_b);
			first_invalidate = false;
		}

		// draw texture using screen pixel buffer
		texGLMixed->Draw(sufGLMain->Width(), sufGLMain->Height(), sufGLMain->GetBuffer());

#ifdef USE_LEDBOX
		if (FLG_SHOWLEDBOX && ledbox && texGLLedBox) {
			ledbox->Draw(*texGLLedBox);
		}
#endif
#ifdef USE_MESSAGE_BOARD
		if (FLG_SHOWMSGBOARD && msgboard && texGLMsgBoard) {
			msgboard->Draw(*texGLMsgBoard);
		}
#endif
	}

#endif /* USE_SCREEN_OPENGL_MIX_ON_RENDERER */

	// invalidate window
	self_invalidate = true;
	skip_frame = false;

//	gui->UpdatedScreen();
}
///
/// post request screen updating to draw it on main thread
///
void EMU_OSD::need_update_screen()
{
	// nothing to do
}

///
/// pointer on source screen
///
/// @return pointer on source screen
scrntype* EMU_OSD::screen_buffer(int y)
{
	return sufOrigin->GetBuffer(y);
}

///
/// offset on source screen
///
/// @return offset on source screen
int EMU_OSD::screen_buffer_offset()
{
	return screen_size.w;
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

#ifdef USE_SCREEN_OPENGL_MIX_ON_RENDERER
	src_tex_l = (float)vm_screen_size.x / (float)mixed_max_size.w;
	src_tex_r = (float)(vm_screen_size.x + vm_screen_size.w) / (float)mixed_max_size.w;
	src_tex_t = (float)vm_screen_size.y / (float)mixed_max_size.h;
	src_tex_b = (float)(vm_screen_size.y + vm_screen_size.h) / (float)mixed_max_size.h;

#ifdef _DEBUG
	logging->out_debugf(_T("src tex: l:%.3f t:%.3f r:%.3f b:%.3f"), src_tex_l, src_tex_t, src_tex_r, src_tex_b);
#endif
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
#ifdef USE_RECORDING_SURFACE
	lock_screen();

	sufOrigin->Blit(*sufRecording);

# ifdef USE_LEDBOX
	if (ledbox) {
		// overlap the indicator to the surface
		ledbox->DrawForRec(*sufRecording);
	}
# endif

	unlock_screen();
#endif /* USE_RECORDING_SURFACE */
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


	rec_video->Capture(CAPTURE_SCREEN_TYPE, rec_video_stretched_size, sufRecording, rec_video_size[size]);

# else /* !USE_RECORDING_SURFACE */
	int size = pConfig->screen_video_size;

	calc_vm_screen_size_sub(rec_video_size[size], vm_screen_size_for_rec);

	rec_video->Capture(CAPTURE_SCREEN_TYPE, vm_screen_size_for_rec, sufSource, rec_video_size[size]);

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
	return rec_video->Start(type, fps_no, rec_video_size[size], sufSource, show_dialog);

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

		rec_video->Record(rec_video_stretched_size, sufSource, rec_video_size[size]);
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
		int x = 0;
		int y = 0;
		if (mainwindow) {
			x = mainwindow->x();
			y = mainwindow->y();
		}
		pConfig->window_position_x = x;
		pConfig->window_position_y = y;
	}
}

///
/// change window size / switch over fullscreen and window
///
/// @param[in] mode 0 - 7: window size  8 -:  fullscreen size  -1: switch over  -2: shift window mode
void EMU_OSD::change_screen_mode(int mode)
{
//	logging->out_debugf(_T("change_screen_mode: mode:%d cwmode:%d pwmode:%d w:%d h:%d"),mode,pConfig->window_mode,prev_window_mode,desktop_size.w,desktop_size.h);
//	if (mode == pConfig->window_mode) return;
	if (now_resizing) {
		// ignore events
		return;
	}

	if (mode == -1) {
		// switch over fullscreen and window
		if (now_screenmode != NOW_WINDOW) {
			// go window mode
			mode = prev_window_mode;
		}
	} else if (mode == -2) {
		// shift window mode 
		if (now_screenmode != NOW_WINDOW) {
			// no change
			return;
		} else {
			mode = ((pConfig->window_mode + 1) % window_mode.Count());
		}
	}
	if (now_screenmode != NOW_FULLSCREEN) {
		prev_window_mode = pConfig->window_mode;
	}
//	logging->out_debugf(_T("change_screen_mode: mode:%d cwmode:%d pwmode:%d w:%d h:%d"),mode,pConfig->window_mode,prev_window_mode,desktop_size.w,desktop_size.h);
	set_window(mode, desktop_size.w, desktop_size.h);
	create_screen(pConfig->disp_device_no, 0, 0, pConfig->screen_width, pConfig->screen_height, m_screen_flags);

}

/// setting window or fullscreen size to display
/// @param [in] mode 0 .. 7 window mode / 8 .. 23 fullscreen mode / -1 want to go fullscreen, but unknown mode
/// @param [in] cur_width  current desktop width if mode is -1
/// @param [in] cur_height current desktop height if mode is -1
/// @param [in] dpi reserved
void EMU_OSD::set_window(int mode, int cur_width, int cur_height, int dpi)
{
	logging->out_debugf(_T("set_window: mode:%d"), mode);

	now_resizing = true;

	if(mode >= 0 && mode < 8) {
		// go window
		if (mode >= window_mode.Count()) mode = 0;
		const CWindowMode *wm = window_mode.Get(mode);
		int width = wm->width;
		int height = wm->height;
#ifdef USE_SCREEN_ROTATE
		if (pConfig->monitor_type & 1) {
			int v = width;
			width = height;
			height = v;
		}
#endif

		pConfig->window_mode = mode;
		pConfig->disp_device_no = 0;
		pConfig->screen_width = width;
		pConfig->screen_height = height;
		window_mode_magnify = wm->magnify;

		if (gui) {
			mainwindow->goNormalScreen(width, height);

			if (now_screenmode == NOW_FULLSCREEN) {
				mainwindow->move(window_dest.x, window_dest.y);
			}

			gui->ScreenModeChanged(false);

			logging->out_debugf(_T("set_window: f->w mode:%d w:%d h:%d nf:%d")
				, mode
				, mainwindow->width(), mainwindow->height()
				, static_cast<int>(now_screenmode)
			);
		}

		now_screenmode = NOW_WINDOW;

		// set screen size to emu class
		set_display_size(width, height, window_mode_magnify, true);
	}
	else if(now_screenmode != NOW_FULLSCREEN) {
		// go fullscreen

		// get current position of window
		window_dest.x = mainwindow->x();
		window_dest.y = mainwindow->y();

		const CVideoMode *sm = nullptr;
		int disp_no = 0;
		int width = 0;
		int height = 0;
		int left = 0;
		int top = 0;

		if (mode >= 8) {
			// check mode number is valid
			if (!screen_mode.GetMode((mode - 8) / VIDEO_MODE_MAX, (mode - 8) % VIDEO_MODE_MAX)) {
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
			left =  dd->re.x;
			top = dd->re.y;
			width = sm ? sm->width : dd->re.w;
			height = sm ? sm->height : dd->re.h;
		}

		pConfig->window_mode = mode;
		pConfig->disp_device_no = disp_no;
		pConfig->screen_width = width;
		pConfig->screen_height = height;

		if (gui) {
			mainwindow->goFullScreen(left, top, width, height);

			gui->ScreenModeChanged(true);
		}
		logging->out_debugf(_T("set_window: w->f mode:%d w:%d h:%d nf:%d")
			, mode
			, mainwindow->width(), mainwindow->height()
			, static_cast<int>(now_screenmode)
		);

		now_screenmode = NOW_FULLSCREEN;

		// set screen size to emu class
		set_display_size(width, height, 1.0, false);
	} else {
		// now fullscreen
		if (mode >= screen_mode.CountMode(0) + 8) {
			mode = -1;
		}
		const CVideoMode *sm = screen_mode.GetMode(0, mode - 8);
		int width = sm ? sm->width : cur_width;
		int height = sm ? sm->height : cur_height;

		logging->out_debugf(_T("set_window: f->f mode:%d w:%d h:%d nf:%d")
			, mode
			, width, height
			, static_cast<int>(now_screenmode)
		);

		// set screen size to emu class
		set_display_size(width, height, 1.0, false);
	}
	first_change_screen = false;
	now_resizing = false;
}

// ---------------------------------------------------------------------------

#ifdef USE_OPENGL

void EMU_OSD::initialize_opengl()
{
}

void EMU_OSD::create_opengl()
{
	if (!opengl) {
		opengl = COpenGL::New();
	}
	if (!opengl) return;

	if (!texGLMixed) {
		texGLMixed = COpenGLTexture::New(opengl, 0);
	}

#ifdef USE_SCREEN_OPENGL_MIX_ON_RENDERER
	if (!sufGLMain) {
		sufGLMain = new CSurface();
	}
	if (!texGLLedBox) {
		texGLLedBox = COpenGLTexture::New(opengl, 1);
	}
	if (!texGLMsgBoard) {
		texGLMsgBoard = COpenGLTexture::New(opengl, 2);
	}
#endif
}

void EMU_OSD::realize_opengl(QOpenGLWidget *UNUSED_PARAM(widget))
{
	create_opengl();
	create_opengl_texture();
}

void EMU_OSD::set_mode_opengl(QOpenGLWidget *UNUSED_PARAM(widget), int w, int h)
{
	opengl->InitViewport(w, h);
}

void EMU_OSD::create_opengl_texture()
{
	if (!opengl) return;

	opengl->Initialize();

	create_opengl_mixedtexture();

#ifdef USE_SCREEN_OPENGL_MIX_ON_RENDERER
	if (sufGLMain) {
		if (!sufGLMain->Create(mixed_max_size.w, mixed_max_size.h, *pixel_format)) {
			logging->out_log(LOG_ERROR, _T("EMU_OSD::create_opengl_texture sufGLMain failed."));
			sufGLMain = NULL;
		}
	}
	if (texGLLedBox) {
		texGLLedBox->Create(pConfig->filter_type);
# ifdef _WIN32
		texGLLedBox->PresetBGRA();
# endif
	}
	if (texGLMsgBoard) {
		texGLMsgBoard->Create(pConfig->filter_type);
	}
#endif

	opengl->ClearScreen();

	set_opengl_interval();
}

void EMU_OSD::create_opengl_mixedtexture()
{
	if (!texGLMixed) return;

	texGLMixed->Release();
	texGLMixed->Create(pConfig->filter_type);
	texGLMixed->CreateBuffer(src_pyl_l, src_pyl_t, src_pyl_r, src_pyl_b, src_tex_l, src_tex_t, src_tex_r, src_tex_b);
//#ifdef _WIN32
//	texGLMixed->PresetBGRA();
//#endif
}

void EMU_OSD::reset_opengl_texture()
{
	release_opengl_texture();
	create_opengl_texture();
}

void EMU_OSD::release_opengl_texture()
{
#ifdef USE_SCREEN_OPENGL_MIX_ON_RENDERER
	if (texGLMsgBoard) {
		texGLMsgBoard->Release();
	}
	if (texGLLedBox) {
		texGLLedBox->Release();
	}
	if (sufGLMain) {
		sufGLMain->Release();
	}
#endif

	if (texGLMixed) {
		texGLMixed->Release();
	}

	if (opengl) {
		opengl->Terminate();
	}
}

void EMU_OSD::release_opengl()
{
#ifdef USE_SCREEN_OPENGL_MIX_ON_RENDERER
	delete texGLMsgBoard;
	texGLMsgBoard = NULL;

	delete texGLLedBox;
	texGLLedBox = NULL;

	delete sufGLMain;
	sufGLMain = NULL;
#endif

	delete texGLMixed;
	texGLMixed = NULL;
}

void EMU_OSD::terminate_opengl()
{
	delete opengl;
	opengl = NULL;
}

void EMU_OSD::set_opengl_interval()
{
	if (!opengl || (pConfig->drawing_method & DRAWING_METHOD_OPENGL_MASK) != 0) return;

	int interval = (pConfig->drawing_method == DRAWING_METHOD_OPENGL_S ? 1 : 2);
	opengl->SetInterval(interval);
}

void EMU_OSD::set_opengl_filter_type()
{
	if (!(pConfig->drawing_method & DRAWING_METHOD_OPENGL_MASK)) return;

//	opengl->SetTextureFilter(pConfig->filter_type);
	if (texGLMixed) texGLMixed->SetFilter(pConfig->filter_type);

#ifdef USE_SCREEN_OPENGL_MIX_ON_RENDERER
	if (texGLLedBox) texGLLedBox->SetFilter(pConfig->filter_type);
	if (texGLMsgBoard) texGLMsgBoard->SetFilter(pConfig->filter_type);
#endif
}

void EMU_OSD::set_opengl_attr()
{
}

void EMU_OSD::set_opengl_poly(int, int)
{
}

#endif /* USE_OPENGL */
