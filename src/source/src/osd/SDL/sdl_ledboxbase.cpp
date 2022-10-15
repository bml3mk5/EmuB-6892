﻿/** @file sdl_ledboxbase.cpp

	HITACHI BASIC MASTER LEVEL3 Mark5 / MB-S1 Emulator
	Skelton for retropc emulator

	@author Sasaji
	@date   2012.03.08 -

	@brief [ led box ]
*/
#include "sdl_ledboxbase.h"
#include "sdl_cbitmap.h"
#include "../../utils.h"
#include "../../cchar.h"
#include "../../utility.h"
#ifdef USE_PERFORMANCE_METER
#include "../../config.h"
#endif

LedBoxBase::LedBoxBase() : CSurface()
{
	for(int i=0; i<LED_TYPE_END; i++) {
		led[i] = NULL;
	}

	flag_old = -1;
	dist.x = 0; dist.y = 0;
	mode = 0;
	prev_mode = -1;
	dist_set[0].x = 2; dist_set[0].y = 2;
	dist_set[1].x = 2; dist_set[1].y = 2;
	memset(&win_pt, 0, sizeof(win_pt));
	memset(&parent_pt, 0, sizeof(parent_pt));

	enable = false;
	visible = false;
	inside = false;
}

LedBoxBase::~LedBoxBase()
{
	for(int i=0; i<LED_TYPE_END; i++) {
		delete led[i];
	}
}

// 画面初期化
bool LedBoxBase::InitScreen(const _TCHAR *res_path, CPixelFormat &pixel_format)
{
	enable = false;

#if defined(_BML3MK5)
	// LED背景
	if (!create_bitmap(res_path, _T("ledbox_11.png"), pixel_format, &led[LED_TYPE_BASE])) return enable;
	// LEDパーツ作成
	if (!create_bitmap(res_path, _T("ledparts.png"), pixel_format, &led[LED_TYPE_PARTS])) return enable;
#elif defined(_MBS1)
	// LED背景
	if (!create_bitmap(res_path, _T("ledbox_14.png"), pixel_format, &led[LED_TYPE_BASE])) return enable;
	// LEDパーツ作成
	if (!create_bitmap(res_path, _T("ledparts4.png"), pixel_format, &led[LED_TYPE_PARTS])) return enable;
#endif

	// horiz
	rect_in(led_ps[LED_PARTS_HR], 7 * 16, 0, 13, 6);
	rect_in(led_ps[LED_PARTS_HG], 9 * 16, 0, 13, 6);
	// vert
	rect_in(led_ps[LED_PARTS_VR], 1 * 16, 0, 6, 13);
	rect_in(led_ps[LED_PARTS_VG], 3 * 16, 0, 6, 13);
	// datarec
	rect_in(led_ps[LED_PARTS_RR], 11 * 16, 0, 13, 11);	// play led
	rect_in(led_ps[LED_PARTS_CL], 13 * 16, 0, 11, 11);	// rec led
	// digit
	if (!create_bitmap(res_path, _T("digit10.png"), pixel_format, &led[LED_TYPE_DIGIT])) return enable;
	// tape icon
	rect_in(led_ps[LCD_PARTS_TAPE], 14 * 16, 0, 15, 11);
	// fdd icon
	rect_in(led_ps[LCD_PARTS_FDD0], 15 * 16, 0, 13, 13);
	rect_in(led_ps[LCD_PARTS_FDD1], 16 * 16, 0, 13, 13);
	rect_in(led_ps[LCD_PARTS_FDD2], 17 * 16, 0, 13, 13);
	rect_in(led_ps[LCD_PARTS_FDD3], 18 * 16, 0, 13, 13);
#if defined(_BML3MK5)
	rect_in(led_ps[LCD_PARTS_3_FDD358], 19 * 16, 0, 7, 17);
	rect_in(led_ps[LCD_PARTS_5_FDD358], 20 * 16, 0, 7, 17);
	rect_in(led_ps[LCD_PARTS_8_FDD358], 21 * 16, 0, 7, 17);
#elif defined(_MBS1)
	rect_in(led_ps[LCD_PARTS_3_FDD35H], 21 * 16, 0, 7, 17);
	rect_in(led_ps[LCD_PARTS_5_FDD35H], 22 * 16, 0, 7, 17);
	rect_in(led_ps[LCD_PARTS_H_FDD35H], 23 * 16, 0, 7, 17);
	// ab mode
	rect_in(led_ps[LCD_PARTS_AB_A], 24 * 16, 0, 10, 18);
	rect_in(led_ps[LCD_PARTS_AB_B], 25 * 16, 0, 10, 18);
	// 1,2mhz
	rect_in(led_ps[LCD_PARTS_12M_2], 26 * 16, 0, 10, 17);
	rect_in(led_ps[LCD_PARTS_12M_1], 27 * 16, 0, 10, 17);
	// s,u
	rect_in(led_ps[LCD_PARTS_SU_S], 28 * 16, 0, 10, 17);
	rect_in(led_ps[LCD_PARTS_SU_U], 29 * 16, 0, 10, 17);
#endif

	// 位置設定
	point_in(led_pt[LED_POS_BASE], 0, 0, led[LED_TYPE_BASE]);
	// keyboard led
#if defined(_BML3MK5)
	point_in(led_pt[LED_POS_KATAKANA], 4, 3, led_ps[LED_PARTS_HR]);
	point_in(led_pt[LED_POS_HIRAGANA], 4, 11, led_ps[LED_PARTS_HG]);
	point_in(led_pt[LED_POS_CAPS], 36, 3, led_ps[LED_PARTS_VR]);
#elif defined(_MBS1)
	point_in(led_pt[LED_POS_KATAKANA], 31, 3, led_ps[LED_PARTS_HR]);
	point_in(led_pt[LED_POS_HIRAGANA], 31, 11, led_ps[LED_PARTS_HG]);
	point_in(led_pt[LED_POS_CAPS], 3, 3, led_ps[LED_PARTS_VR]);
#endif
	// datarec led
	point_in(led_pt[LED_POS_PLAY], 65, 4, led_ps[LED_PARTS_RR]);
	point_in(led_pt[LCD_POS_PLAY], 78, 4, led_ps[LCD_PARTS_TAPE]);
	point_in(led_pt[LED_POS_REC],  96, 4, led_ps[LED_PARTS_CL]);
	point_in(led_pt[LCD_POS_REC], 108, 4, led_ps[LCD_PARTS_TAPE]);
	// fdd type
#if defined(_BML3MK5)
	point_in(led_pt[LCD_POS_FDD358], 162, 1, led_ps[LCD_PARTS_3_FDD358]);
#elif defined(_MBS1)
	point_in(led_pt[LCD_POS_FDD35H], 163, 1, led_ps[LCD_PARTS_3_FDD35H]);
#endif
	// fdd 0
	point_in(led_pt[LED_POS_FDD0], 173, 3, led_ps[LED_PARTS_VR]);
	point_in(led_pt[LCD_POS_FDD0], 180, 3, led_ps[LCD_PARTS_FDD0]);
	// fdd 1
	point_in(led_pt[LED_POS_FDD1], 198, 3, led_ps[LED_PARTS_VR]);
	point_in(led_pt[LCD_POS_FDD1], 205, 3, led_ps[LCD_PARTS_FDD1]);
	// fdd 2
	point_in(led_pt[LED_POS_FDD2], 223, 3, led_ps[LED_PARTS_VR]);
	point_in(led_pt[LCD_POS_FDD2], 230, 3, led_ps[LCD_PARTS_FDD2]);
	// fdd 3
	point_in(led_pt[LED_POS_FDD3], 248, 3, led_ps[LED_PARTS_VR]);
	point_in(led_pt[LCD_POS_FDD3], 255, 3, led_ps[LCD_PARTS_FDD3]);
	// digit
	rect_in(led_pt[LED_POS_DIGIT1], 147, 4, 8, 12);	// 1
	rect_in(led_pt[LED_POS_DIGIT2], 138, 4, 8, 12);	// 10
	rect_in(led_pt[LED_POS_DIGIT3], 129, 4, 8, 12);	// 100
#if defined(_MBS1)
	// ab mode
	point_in(led_pt[LCD_POS_AB], 276, 1, led_ps[LCD_PARTS_AB_A]);
	// 1,2mhz
	point_in(led_pt[LCD_POS_12M], 288, 2, led_ps[LCD_PARTS_12M_1]);
	// s,u
	point_in(led_pt[LCD_POS_SU], 298, 2, led_ps[LCD_PARTS_SU_S]);
#endif

	if (!create_surface(pixel_format)) {
		return false;
	}

	parent_pt.w = Width();
	parent_pt.h = Height();

	enable = true;
	return enable;
}

bool LedBoxBase::create_bitmap(const _TCHAR *res_path, const _TCHAR *bmp_file, CPixelFormat &format, CBitmap **suf)
{
	_TCHAR path[_MAX_PATH];
	UTILITY::stprintf(path, _MAX_PATH, _T("%s%s"), res_path, bmp_file);
	*suf = new CBitmap(path, &format);
	if (*suf == NULL || !(*suf)->IsEnable()) return false;
	return true;
}

bool LedBoxBase::create_surface(CPixelFormat &format)
{
	if (led[LED_TYPE_BASE] == NULL || !led[LED_TYPE_BASE]->IsEnable()) return false;
	Release();

	int w, h;
	w = led[LED_TYPE_BASE]->Width();
	h = led[LED_TYPE_BASE]->Height();
#ifdef USE_PERFORMANCE_METER
	if (config.show_pmeter) {
		w += 108;
	}
#endif
	bool rc = Create(w, h, format);
	if (!rc) return false;

#if defined(USE_SDL)
	// See SDL_ConvertSurface source code
	SDL_Surface *led_suf = led[LED_TYPE_BASE]->Get();
	led_suf->flags &= ~SDL_SRCALPHA;
	SDL_Rect sre;
	sre.x = 0; sre.y =0;
	sre.w = led_suf->w;	sre.h = led_suf->h;
	SDL_BlitSurface(led_suf, &sre, suf, &sre);
	SDL_SetAlpha(suf, SDL_SRCALPHA, 0xff);
	led_suf->flags |= SDL_SRCALPHA;
#endif

	return true;
}

void LedBoxBase::rect_in(SDL_Rect &re, int x, int y, int w, int h)
{
	re.x = x;
	re.y = y;
	re.w = w;
	re.h = h;
}

void LedBoxBase::point_in(SDL_Rect &dst, int x, int y, SDL_Rect &src)
{
	dst.x = x;
	dst.y = y;
	dst.w = src.w;
	dst.h = src.h;
}

void LedBoxBase::point_in(SDL_Rect &dst, int x, int y, CBitmap *src)
{
	dst.x = x;
	dst.y = y;
	dst.w = src->Width();
	dst.h = src->Height();
}

void LedBoxBase::Show(int flag)
{
	visible = ((flag & 1) != 0);
	inside = ((flag & 8) != 0);

	show_dialog();
}

void LedBoxBase::Update(uint64_t flag)
{
	if (!enable) return;

	if (flag != flag_old) {
		led[LED_TYPE_BASE]->Blit(*this);

		// katakana
		if (flag & 0x10) {
			copy_led_s(led_pt[LED_POS_KATAKANA], led[LED_TYPE_PARTS], led_ps[LED_PARTS_HR]);
		}
		// hiragana
		if (flag & 0x20) {
			copy_led_s(led_pt[LED_POS_HIRAGANA], led[LED_TYPE_PARTS], led_ps[LED_PARTS_HG]);
		}
		// caps
		if (!(flag & 0x40)) {
			copy_led_s(led_pt[LED_POS_CAPS], led[LED_TYPE_PARTS], led_ps[LED_PARTS_VR]);
		}
		// play
		if (flag & 0x100) {
			copy_led_s(led_pt[LED_POS_PLAY], led[LED_TYPE_PARTS], led_ps[LED_PARTS_RR]);
		}
		// rec
		if (flag & 0x200) {
			copy_led_s(led_pt[LED_POS_REC], led[LED_TYPE_PARTS], led_ps[LED_PARTS_CL]);
		}
		// play lcd icon
		if (flag & 0x400) {
			copy_led_s(led_pt[LCD_POS_PLAY], led[LED_TYPE_PARTS], led_ps[LCD_PARTS_TAPE]);
		}
		// rec lcd icon
		if (flag & 0x800) {
			copy_led_s(led_pt[LCD_POS_REC], led[LED_TYPE_PARTS], led_ps[LCD_PARTS_TAPE]);
		}

		// fdd type
#if defined(_BML3MK5)
		if ((flag & 0xc) == 0x4) {
			copy_led_s(led_pt[LCD_POS_FDD358], led[LED_TYPE_PARTS], led_ps[LCD_PARTS_3_FDD358]);
		} else if ((flag & 0xc) == 0x8) {
			copy_led_s(led_pt[LCD_POS_FDD358], led[LED_TYPE_PARTS], led_ps[LCD_PARTS_5_FDD358]);
		} else if (flag & 0xc) {
			copy_led_s(led_pt[LCD_POS_FDD358], led[LED_TYPE_PARTS], led_ps[LCD_PARTS_8_FDD358]);
		}
#elif defined(_MBS1)
		if ((flag & 0xc) == 0x4) {
			copy_led_s(led_pt[LCD_POS_FDD35H], led[LED_TYPE_PARTS], led_ps[LCD_PARTS_3_FDD35H]);
		} else if ((flag & 0xc) == 0x8) {
			copy_led_s(led_pt[LCD_POS_FDD35H], led[LED_TYPE_PARTS], led_ps[LCD_PARTS_5_FDD35H]);
		} else if (flag & 0xc) {
			copy_led_s(led_pt[LCD_POS_FDD35H], led[LED_TYPE_PARTS], led_ps[LCD_PARTS_H_FDD35H]);
		}
#endif

		// fdd
		uint32_t flag_part = (uint32_t)(flag >> 28);
		for(int drv=0; drv<4; drv++) {
			if ((flag_part & 0x11) == 0x01) {
				copy_led_s(led_pt[LED_POS_FDD0 + drv], led[LED_TYPE_PARTS], led_ps[LED_PARTS_VG]);
			} else if ((flag_part & 0x11) == 0x11) {
				copy_led_s(led_pt[LED_POS_FDD0 + drv], led[LED_TYPE_PARTS], led_ps[LED_PARTS_VR]);
			}
			if (flag_part & 0x0100) {
				copy_led_s(led_pt[LCD_POS_FDD0 + drv], led[LED_TYPE_PARTS], led_ps[LCD_PARTS_FDD0 + drv]);
			}
			flag_part >>= 1;
		}
#if defined(_MBS1)
		// abmode
		if (flag & 0x10000000000) {
			copy_led_s(led_pt[LCD_POS_AB], led[LED_TYPE_PARTS], led_ps[LCD_PARTS_AB_A]);
		} else {
			copy_led_s(led_pt[LCD_POS_AB], led[LED_TYPE_PARTS], led_ps[LCD_PARTS_AB_B]);
		}
		// 1,2mhz
		if (flag & 0x20000000000) {
			copy_led_s(led_pt[LCD_POS_12M], led[LED_TYPE_PARTS], led_ps[LCD_PARTS_12M_2]);
		} else {
			copy_led_s(led_pt[LCD_POS_12M], led[LED_TYPE_PARTS], led_ps[LCD_PARTS_12M_1]);
		}
		// s,u
		if (flag & 0x40000000000) {
			copy_led_s(led_pt[LCD_POS_SU], led[LED_TYPE_PARTS], led_ps[LCD_PARTS_SU_S]);
		} else {
			copy_led_s(led_pt[LCD_POS_SU], led[LED_TYPE_PARTS], led_ps[LCD_PARTS_SU_U]);
		}
#endif

		// cmt counter
		uint32_t count = ((flag >> 12) & 0xffff);
		int i = (count / 12);
		int f = (count % 12);
		SDL_Rect re;
		rect_in(re, 0, 0, led_pt[LED_POS_DIGIT1].w, led_pt[LED_POS_DIGIT1].h);
		re.y = (i % 10) * led_pt[LED_POS_DIGIT1].h + f;
		copy_led_s(led_pt[LED_POS_DIGIT1], led[LED_TYPE_DIGIT], re);
		re.y = ((i / 10) % 10) * led_pt[LED_POS_DIGIT2].h + ((i % 10) == 9 ? f : 0);
		copy_led_s(led_pt[LED_POS_DIGIT2], led[LED_TYPE_DIGIT], re);
		re.y = ((i / 100) % 10) * led_pt[LED_POS_DIGIT3].h + ((i % 100) == 99 ? f : 0);
		copy_led_s(led_pt[LED_POS_DIGIT3], led[LED_TYPE_DIGIT], re);

#ifdef USE_PERFORMANCE_METER
		// for debug
		if (config.show_pmeter) {
			re.x = suf->w - 108; re.y = 0;
			re.w = 108; re.h = suf->h;
			SDL_FillRect(suf, &re, SDL_MapRGBA(suf->format, 0x40, 0x40, 0x40, 0xff));
			for(int pos=0; pos < 11; pos++) {
				re.x = suf->w - 104 + (pos * 10); re.y = suf->h * ((pos % 5 == 0) ? 1 : 2) / 8;
				re.w = 1; re.h = suf->h * ((pos % 5 == 0) ? 3 : 2) / 8;
				SDL_FillRect(suf, &re, SDL_MapRGBA(suf->format, 0xff, 0xff, 0xff, 0xff));
			}
			int value = (flag >> 48);
			if (value > 100) value = 100;
			re.x = suf->w - 104; re.y = suf->h / 2 + 2;
			re.w = value; re.h = 2;
			SDL_FillRect(suf, &re, SDL_MapRGBA(suf->format, 0xff, 0x0, 0x0, 0xff));
		}
#endif

		if (visible && !inside) {
			need_update_dialog();
		}
	}

	flag_old = flag;
}

void LedBoxBase::copy_led(SDL_Rect &p, CBitmap *l)
{
	l->Blit(*this, p);
}

void LedBoxBase::copy_led_s(SDL_Rect &p, CBitmap *l, SDL_Rect &re)
{
	l->Blit(re, *this, p);
}

void LedBoxBase::SetPos(int left, int top, int right, int bottom, int place)
{
	win_pt.left = left;
	win_pt.top = top;
	win_pt.right = right;
	win_pt.bottom = bottom;

	if (place & 1) {
		// base right
		parent_pt.x = right - parent_pt.w;
	} else {
		// base left
		parent_pt.x = left;
	}
	if (place & 2) {
		// base bottom
		parent_pt.y = bottom - parent_pt.h;
	} else {
		// base top
		parent_pt.y = top;
	}

	move_in_place(place);

	win_pt.place = place;
}

void LedBoxBase::SetPos(int place)
{
	SetPos(win_pt.left, win_pt.top, win_pt.right, win_pt.bottom, place);
}

void LedBoxBase::Draw(CSurface &screen)
{
	if (!visible || !inside || !enable) return;

	Blit(screen, parent_pt);
}

void LedBoxBase::SetMode(int val)
{
	val = (val < 0) ? 0 : (val > 1 ? 1 : val);
	dist_set[mode].x = dist.x;
	dist_set[mode].y = dist.y;
	prev_mode = mode;
	mode = val;
	dist.x = dist_set[mode].x;
	dist.y = dist_set[mode].y;
}

void LedBoxBase::SetDistance(int place, const VmPoint *ndist)
{
	win_pt.place = place;
	dist_set[0].x = ndist[0].x;
	dist_set[0].y = ndist[0].y;
	dist_set[1].x = ndist[1].x;
	dist_set[1].y = ndist[1].y;

	dist.x = dist_set[mode].x;
	dist.y = dist_set[mode].y;
}

void LedBoxBase::GetDistance(VmPoint *ndist)
{
	dist_set[mode].x = dist.x;
	dist_set[mode].y = dist.y;

	ndist[0].x = dist_set[0].x;
	ndist[0].y = dist_set[0].y;
	ndist[1].x = dist_set[1].x;
	ndist[1].y = dist_set[1].y;
}
