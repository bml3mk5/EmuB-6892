/** @file wxw_screen.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2014.03.21

	@brief [ wxw screen ]

	@note
	This code is based on win32_screen.cpp of the Common Source Code Project.
	Author : Takeda.Toshiya
*/

#include <wx/wx.h>
#ifdef USE_OPENGL
#include "../opengl.h"
#endif
#include <wx/display.h>
#include <wx/image.h>
#include <wx/rawbmp.h>
#include "wxw_emu.h"
#include "../../vm/vm.h"
#ifdef USE_LEDBOX
#include "../../gui/ledbox.h"
#endif
#ifdef USE_MESSAGE_BOARD
#include "../../msgboard.h"
#endif
#include "../../csurface.h"
#include "../../video/rec_video.h"
#include "wxw_main.h"
#include "../../vm/vm.h"
#include "../../gui/gui.h"
#include "../../config.h"
#include "../../utils.h"

//static wxVideoMode screen_mode_wx[SCREEN_MODE_MAX];

//static int window_pos_x = -1;
//static int window_pos_y = -1;

void EMU_OSD::EMU_SCREEN()
{
	scrBmp = new CSurface();
	screen_flags = 0;

#ifdef USE_OPENGL
	texGLMixed = NULL;
//	mix_texture_name = 0;
	src_tex_l = src_tex_t = 0.0;
	src_tex_r = src_tex_b = 1.0;
	src_pyl_l = src_pyl_t = -1.0;
	src_pyl_r = src_pyl_b = 1.0;
	opengl = NULL;
	next_use_opengl = 0;
#endif

#ifdef USE_LEDBOX
	ledbox = NULL;
#endif
}

///
/// initialize screen
///
void EMU_OSD::initialize_screen()
{
	EMU::initialize_screen();

#ifdef USE_OPENGL
	next_use_opengl = pConfig->use_opengl;

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
#endif
	if (gui) {
		gui->ReleaseLedBox();
	}

	delete scrBmp;
	scrBmp = NULL;

	EMU::release_screen();
}

///
/// change pixel format
///
void EMU_OSD::change_pixel_format()
{
	if (pixel_format != NULL) {
		// change pixel format
#if defined(__WXMSW__)
#  ifdef USE_OPENGL
		if (pConfig->use_opengl) {
			pixel_format->PresetRGBA();
		} else
#  endif
		{
			pixel_format->PresetBGRA();
		}
#elif defined(__WXOSX__)
#  ifdef USE_OPENGL
		if (pConfig->use_opengl) {
			pixel_format->PresetARGB();
		} else
#  endif
		{
			pixel_format->PresetARGB();
		}
#elif defined(__WXGTK__)
#  ifdef USE_OPENGL
		if (pConfig->use_opengl) {
			pixel_format->PresetRGBA();
		} else
#  endif
		{
			pixel_format->PresetBGRA();
		}
#else
		pixel_format->PresetRGBA();
#endif
	}
}

///
/// create / recreate window
///
bool EMU_OSD::create_screen(int disp_no, int x, int y, int width, int height, uint32_t flags)
{
	if (screen_mode.WithinDisp(x, y) < 0) {
		x = wxDefaultPosition.x;
		y = wxDefaultPosition.y;
	}
	gui->CreateMyFrame(x, y, width, height);

	screen_flags = flags;

	change_pixel_format();

#ifdef _DEBUG_LOG
	logging->out_debugf(_T("VideoInfo Rmask:%x, Gmask:%x, Bmask:%x, Amask:%x"),pixel_format->Rmask,pixel_format->Gmask,pixel_format->Bmask,pixel_format->Amask);
	logging->out_debugf(_T("VideoInfo Rshift:%d, Gshift:%d, Bshift:%d, Ashift:%d"),pixel_format->Rshift,pixel_format->Gshift,pixel_format->Bshift,pixel_format->Ashift);
#endif

	return true;
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
	if (sufMixed) {
		sufMixed->Release();
		if (!sufMixed->Create(mixed_max_size.w, mixed_max_size.h, *pixel_format)) {
			logging->out_log(LOG_ERROR, _T("EMU_OSD::create_offlinesurface sufMixed failed."));
			return false;
		}
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

	disable_screen &= ~DISABLE_SURFACE;

	return true;
}

///
/// setting window or fullscreen size
///
/// @param [in] width : new width or -1 set current width
/// @param [in] height : new height or -1 set current height
/// @param [in] power : magnify x 10
/// @param [in] now_window : true:window / false:fullscreen
void EMU_OSD::set_display_size(int width, int height, int power, bool now_window)
{
//	bool display_size_changed = false;
	bool stretch_changed = false;

	if(width != -1 && (display_size.w != width || display_size.h != height)) {
		display_size.w = width;
		display_size.h = height;
//		display_size_changed = true;
		stretch_changed = true;
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
		stretch_changed |= (source_size.w != screen_size.w);
		stretch_changed |= (source_size.h != screen_size.h);
		stretch_changed |= (source_aspect_size.w != screen_aspect_size.w);
		stretch_changed |= (source_aspect_size.h != screen_aspect_size.h);

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
				mixed_size.w = display_size.w * 10 / power;
				mixed_size.h = display_size.h * 10 / power;
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

		stretched_size.w = source_aspect_size.w * power / 10;
		stretched_size.h = source_aspect_size.h * power / 10;
		stretched_size.x = (display_size.w - stretched_size.w) / 2;
		stretched_size.y = (display_size.h - stretched_size.h) / 2;
		stretched_dest_real.x = - mixed_size.x * power / 10;
		stretched_dest_real.y = - mixed_size.y * power / 10;
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
	src_tex_l = (GLfloat)mixed_size.x / mixed_max_size.w;
	src_tex_t = (GLfloat)mixed_size.y / mixed_max_size.h;
	src_tex_r = (GLfloat)(mixed_size.x + mixed_size.w) / mixed_max_size.w;
	src_tex_b = (GLfloat)(mixed_size.y + mixed_size.h) / mixed_max_size.h;
	if (stretched_size.w < display_size.w) src_pyl_r = (GLfloat)stretched_size.w / display_size.w;
	else src_pyl_r = 1.0f;
	if (stretched_size.h < display_size.h) src_pyl_b = (GLfloat)stretched_size.h / display_size.h;
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
	logging->out_debugf(_T("set_display_size: w:%d h:%d power:%d %s"),width,height,power,now_window ? _T("window") : _T("fullscreen"));
	logging->out_debugf(_T("         display: w:%d h:%d"),display_size.w, display_size.h);
	logging->out_debugf(_T("          screen: w:%d h:%d"), screen_size.w, screen_size.h);
	logging->out_debugf(_T("   screen aspect: w:%d h:%d"), screen_aspect_size.w, screen_aspect_size.h);
	logging->out_debugf(_T("          source: w:%d h:%d"), source_size.w, source_size.h);
	logging->out_debugf(_T("   source aspect: w:%d h:%d"), source_aspect_size.w, source_aspect_size.h);
	logging->out_debugf(_T("           mixed: w:%d h:%d"), mixed_size.w, mixed_size.h);
	logging->out_debugf(_T("         stretch: w:%d h:%d"), stretched_size.w, stretched_size.h);
	logging->out_debugf(_T("      mixed dest: x:%d y:%d"), mixed_size.x, mixed_size.y);
	logging->out_debugf(_T("    stretch dest: x:%d y:%d"), stretched_size.x, stretched_size.y);
	logging->out_debugf(_T(" stretch dest re: x:%d y:%d"), stretched_dest_real.x, stretched_dest_real.y);
#ifdef USE_OPENGL
	logging->out_debugf(_T("     src polygon: l:%d t:%d r:%d b:%d"), rePyl.left, rePyl.top, rePyl.right, rePyl.bottom);
	logging->out_debugf(_T("     src polygon: l:%.5f t:%.5f r:%.5f b:%.5f"), src_pyl_l, src_pyl_t, src_pyl_r, src_pyl_b);
	logging->out_debugf(_T("     src texture: l:%.5f t:%.5f r:%.5f b:%.5f"), src_tex_l, src_tex_t, src_tex_r, src_tex_b);
#endif
#endif
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

	// send display size to vm
	set_vm_display_size();

#ifdef USE_LEDBOX
	if (gui) {
		gui->SetLedBoxPosition(now_window, mixed_size.x, mixed_size.y, mixed_size.w, mixed_size.h, pConfig->led_pos | (is_fullscreen() ? 0x10 : 0));
	}
#endif
#ifdef USE_MESSAGE_BOARD
	if (msgboard) {
		msgboard->SetSize(source_size.w, source_size.h);
		msgboard->SetMessagePos(4 + mixed_size.x,  - 4 - source_size.h + mixed_size.y + mixed_size.h, 2);
		msgboard->SetInfoPos(-4 - mixed_size.x, 4 + mixed_size.y, 1);
	}
#endif

	unlock_screen();
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

	{
#ifdef USE_LEDBOX
		if (FLG_SHOWLEDBOX && ledbox) {
			ledbox->Draw(*sufSource);
		}
#endif
	}

	unlock_screen();
}

///
/// copy src screen to mix screen and overlap a message
///
/// @return false: cannot mix (no allocate mix surface)
bool EMU_OSD::mix_screen()
{
	if (disable_screen) return false;

	lock_screen();

	sufSource->Blit(*sufMixed);

	unlock_screen();

	if (gui) {
		gui->UpdateIndicator(update_led());
	}
#ifdef USE_MESSAGE_BOARD
	if (msgboard && FLG_SHOWMSGBOARD) {
		msgboard->Draw(sufMixed);
	}
#endif
	return true;
}

///
/// update GLCanvas
///
void EMU_OSD::update_screen_gl(MyGLCanvas *panel)
{
	gui->UpdateScreen();

	if (mix_screen()) {

#ifdef USE_OPENGL
		if (texGLMixed) {
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
#if defined(__WXMSW__)
			scrntype *buffer = sufMixed->GetBuffer();
			sufMixed->Flip();
//			buffer -= ((sufMixed->Height() - 1) * sufMixed->Width());
#elif defined(__WXOSX__)
			char *buffer = (char *)sufMixed->GetBuffer();
//			buffer -= (sufMixed->Width() * sufMixed->BytesPerPixel());
			buffer++;
#else
			scrntype *buffer = sufMixed->GetBuffer();
#endif
			// draw texture using screen pixel buffer
			texGLMixed->Draw(sufMixed->Width(), sufMixed->Height(), buffer);

			panel->SwapBuffers();

			// invalidate window
			self_invalidate = true;
			skip_frame = false;
		}
#endif
	}
	gui->UpdatedScreen();
	return;
}

#ifdef __WXOSX__
//#define USE_BUFFERD_PAINT_DC
#endif
#ifdef USE_BUFFERD_PAINT_DC
#include <wx/dcbuffer.h>
#endif

void EMU_OSD::update_screen_pa(MyPanel *panel)
{
	gui->UpdateScreen();

	if (mix_screen()) {

#ifdef USE_BUFFERD_PAINT_DC
#ifdef __WXOSX__
		wxImage img(display_width, display_height);
		if (stretched_size.w == mixed_size.w && stretched_size.h == mixed_size.h) {
			sufMixed->Blit(reMix, img, reSuf);
		} else {
			sufMixed->StretchBlit(reMix, img, reSuf);
		}
		wxBitmap bmp(img);
#else
		wxBitmap bmp(display_width, display_height, 24);
		if (stretched_size.w == mixed_size.w && stretched_size.h == mixed_size.h) {
			sufMixed->Blit(reMix, bmp, reSuf);
		} else {
			sufMixed->StretchBlit(reMix, bmp, reSuf);
		}
#endif
#else
		if (first_invalidate) {
			scrBmp->FillBlack();
			first_invalidate = false;
		}
		if (stretched_size.w == mixed_size.w && stretched_size.h == mixed_size.h) {
			sufMixed->Blit(reMix, *scrBmp, reSuf);
		} else {
			sufMixed->StretchBlit(reMix, *scrBmp, reSuf);
		}
#endif

#ifdef USE_BUFFERD_PAINT_DC 
		// paint the screen
		wxBufferedPaintDC dc(panel, bmp);
//		if (!dc.IsOk()) {
//			logging->out_log(LOG_ERROR,_T("update_screen: bufferd paint dc failed."));
//		}
#else
		wxPaintDC dc(panel);
		dc.Blit(0, 0, display_size.w, display_size.h, scrBmp->GetDC(), 0, 0);
#endif

		// invalidate window
		self_invalidate = true;
		skip_frame = false;
	}
	gui->UpdatedScreen();
	return;
}

///
/// post request screen updating to draw it on main thread
///
//void EMU_OSD::need_update_screen()
//{
//	// nothing to do
//}

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
#if defined(__WXMSW__)
	return -screen_size.w;
#else
	return screen_size.w;
#endif
}

///
/// capture current screen and save to a file
///
void EMU_OSD::capture_screen()
{
	int size = pConfig->screen_video_size;
	rec_video->Capture(CAPTURE_SCREEN_TYPE, rec_video_stretched_size, sufSource, rec_video_size[size]);
}

///
/// start recording video
///
//bool EMU_OSD::start_rec_video(int type, int fps_no, bool show_dialog)
//{
//#ifdef USE_REC_VIDEO
//	int size = pConfig->screen_video_size;
//	return rec_video->Start(type, fps_no, rec_video_size[size], sufSource, show_dialog);
//#else
//	return false;
//#endif
//}

///
/// record video
///
void EMU_OSD::record_rec_video()
{
#ifdef USE_REC_VIDEO
	if (rec_video->IsRecordFrame()) {
		int size = pConfig->screen_video_size;
		rec_video->Record(rec_video_stretched_size, sufSource, rec_video_size[size]);
	}
#endif
}

/// store window position to ini file
void EMU_OSD::resume_window_placement()
{
	if (now_screenmode == NOW_FULLSCREEN) {
		pConfig->window_position_x = window_dest.x;
		pConfig->window_position_y = window_dest.y;
	} else {
		int x = 0;
		int y = 0;
		if (gui) {
			MyFrame *frame = gui->GetMyFrame();
			if (frame) frame->GetPosition(&x, &y);
		}
		pConfig->window_position_x = x;
		pConfig->window_position_y = y;
	}
}

/// change window size / switch over fullscreen and window
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
	if (!create_screen(pConfig->disp_device_no, 0, 0, pConfig->screen_width, pConfig->screen_height, screen_flags)) {
		exit(1);
	}

}

/// setting window or fullscreen size to display
/// @param [in] mode 0 .. 7 window mode / 8 .. 23 fullscreen mode / -1 want to go fullscreen, but unknown mode
/// @param [in] cur_width  current desktop width if mode is -1
/// @param [in] cur_height current desktop height if mode is -1
void EMU_OSD::set_window(int mode, int cur_width, int cur_height)
{
	logging->out_debugf(_T("set_window: mode:%d"), mode);

	now_resizing = true;

	MyFrame *frame = gui->GetMyFrame();

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
		window_mode_power = wm->power;

		now_screenmode = NOW_WINDOW;

		if (gui) {
//			int num = mode;
//			if (window_pos_x < 0 || window_pos_y < 0) {
//				// center
//				frame->GetPosition(&window_pos_x, &window_pos_y);
//				window_pos_x = (desktop_width - width) / 2;
//				window_pos_y = (desktop_height - height) / 2;
//			}
			if (frame->IsFullScreen()) {
				// go back to default mode
				wxDisplay disp(pConfig->disp_device_no);
				disp.ChangeMode();
//				wxRect re = disp.GetGeometry();
				frame->GoUnfullscreen(wm->width, wm->height);
				frame->SetSize(window_dest.x, window_dest.y, wxDefaultCoord, wxDefaultCoord, wxSIZE_USE_EXISTING);
			}
			frame->SetClientSize(wm->width, wm->height);

			if (scrBmp != NULL) {
//				delete scrDC;
				delete scrBmp;
			}
//			scrBmp = new wxBitmap(wm->width, wm->height, 24);
//			scrDC = new wxMemoryDC(*scrBmp);
			scrBmp = new CSurface(wm->width, wm->height, CPixelFormat::RGBA32);

			gui->ScreenModeChanged(now_screenmode == NOW_FULLSCREEN);
		}
		// set screen size to emu class
		set_display_size(width, height, window_mode_power, true);
	}
	else if(now_screenmode != NOW_FULLSCREEN) {
		// go fullscreen

		// get current position of window
		frame->GetPosition(&window_dest.x, &window_dest.y);

		const CVideoMode *sm = NULL;
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

		now_screenmode = NOW_FULLSCREEN;

		if (gui) {
//			int num = mode - WINDOW_MODE_MAX;
			wxDisplay disp(disp_no);
			wxRect re = disp.GetGeometry();
			wxVideoMode mode = disp.GetCurrentMode();
			mode.w = width;
			mode.h = height;
			disp.ChangeMode(mode);
			frame->SetSize(re.x, re.y, wxDefaultCoord, wxDefaultCoord, wxSIZE_USE_EXISTING);
			frame->SetClientSize(width, height);
			frame->GoFullscreen(width, height);

			if (scrBmp != NULL) {
//				delete scrDC;
				delete scrBmp;
			}
//			scrBmp = new wxBitmap(width, height, 24);
//			scrDC = new wxMemoryDC(*scrBmp);
			scrBmp = new CSurface(width, height, CPixelFormat::RGBA32);

			gui->ScreenModeChanged(now_screenmode == NOW_FULLSCREEN);
		}

		// set screen size to emu class
		set_display_size(width, height, 10, false);
	}
	first_change_screen = false;
	now_resizing = false;
}

// ----------
#ifdef USE_OPENGL

void EMU_OSD::initialize_opengl()
{
}

void EMU_OSD::realize_opengl(MyGLCanvas *panel)
{
	if (!opengl) {
		opengl = COpenGL::New();
		if (!texGLMixed) {
			texGLMixed = COpenGLTexture::New(opengl, 0);
		}
	}

	opengl->Initialize();

	create_opengl_texture();
}

void EMU_OSD::set_mode_opengl(MyGLCanvas *panel, int w, int h)
{
	opengl->InitViewport(w, h);
}

void EMU_OSD::create_opengl_texture()
{
//	if (mix_texture_name != 0) return;
	if (!texGLMixed) return;

	// create
//	mix_texture_name = opengl->CreateTexture(pConfig->gl_filter_type);

	texGLMixed->Create(pConfig->gl_filter_type);
	texGLMixed->CreateBuffer(src_pyl_l, src_pyl_t, src_pyl_r, src_pyl_b, src_tex_l, src_tex_t, src_tex_r, src_tex_b);

	opengl->ClearScreen();
}

void EMU_OSD::change_opengl_attr()
{
	if (!use_opengl) {
		return;
	}

//	opengl->SetTextureFilter(pConfig->gl_filter_type);
	texGLMixed->SetFilter(pConfig->gl_filter_type);
}

void EMU_OSD::release_opengl()
{
	if (use_opengl) {
		release_opengl_texture();
	}

	delete texGLMixed;
	texGLMixed = NULL;

	delete opengl;
	opengl = NULL;
}

void EMU_OSD::release_opengl_texture()
{
	if (texGLMixed) {
//		mix_texture_name = opengl->ReleaseTexture();
		texGLMixed->Release();
	}
}

void EMU_OSD::set_interval_opengl()
{
	pConfig->use_opengl = opengl->SetInterval(pConfig->use_opengl);
}

void EMU_OSD::set_opengl_attr()
{
}

void EMU_OSD::set_opengl_poly(int width, int height)
{
}

#endif // USE_OPENGL
