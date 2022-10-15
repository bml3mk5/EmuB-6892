/** @file win_ledboxbase.h

	HITACHI BASIC MASTER LEVEL3 Mark5 / MB-S1 Emulator
	Skelton for retropc emulator

	@author Sasaji
	@date   2011.08.08 -

	@brief [ led box ]
*/

#ifndef _WIN_LEDBOX_BASE_H_
#define _WIN_LEDBOX_BASE_H_

#include <windows.h>

#include "win_d3d.h"
#include "../../common.h"
#include "../../csurface.h"
#include "../../cbitmap.h"

/**
	@brief LedBoxBase is the class that display the access indicator on the screen.
*/
class LedBoxBase : public CSurface
{
protected:
	uint64_t	flag_old;

	enum enum_led_type {
		LED_TYPE_DIGIT = 0,
		LED_TYPE_PARTS,
		LED_TYPE_BASE,
		LED_TYPE_END
	};
	// ビットマップ
	CBitmap *led[LED_TYPE_END];

	enum enum_led_parts {
		LED_PARTS_VR = 0,
		LED_PARTS_VG,
		LED_PARTS_HR,
		LED_PARTS_HG,
		LED_PARTS_RR,
		LED_PARTS_CL,
		LCD_PARTS_TAPE,
		LCD_PARTS_FDD0,
		LCD_PARTS_FDD1,
		LCD_PARTS_FDD2,
		LCD_PARTS_FDD3,
#if defined(_BML3MK5)
		LCD_PARTS_3_FDD358,
		LCD_PARTS_5_FDD358,
		LCD_PARTS_8_FDD358,
#elif defined(_MBS1)
		LCD_PARTS_3_FDD35H,
		LCD_PARTS_5_FDD35H,
		LCD_PARTS_H_FDD35H,
		LCD_PARTS_AB_A,
		LCD_PARTS_AB_B,
		LCD_PARTS_12M_1,
		LCD_PARTS_12M_2,
		LCD_PARTS_SU_S,
		LCD_PARTS_SU_U,
#endif
		LED_PARTS_END
	};
	// LEDパーツ内位置
	VmRectWH	led_ps[LED_PARTS_END];

	enum enum_led_pos {
		LED_POS_BASE = 0,
		LED_POS_KATAKANA,
		LED_POS_HIRAGANA,
		LED_POS_CAPS,
		LED_POS_PLAY,
		LED_POS_REC,
		LCD_POS_PLAY,
		LCD_POS_REC,
		LED_POS_FDD0,
		LED_POS_FDD1,
		LED_POS_FDD2,
		LED_POS_FDD3,
		LCD_POS_FDD0,
		LCD_POS_FDD1,
		LCD_POS_FDD2,
		LCD_POS_FDD3,
#if defined(_BML3MK5)
		LCD_POS_FDD358,
#elif defined(_MBS1)
		LCD_POS_FDD35H,
		LCD_POS_AB,
		LCD_POS_12M,
		LCD_POS_SU,
#endif
		LED_POS_DIGIT1,
		LED_POS_DIGIT2,
		LED_POS_DIGIT3,
		LED_POS_END
	};
	// LED位置
	VmRectWH	led_pt[LED_POS_END];

	// 表示位置
	struct win_pt_st {
		int left;
		int top;
		int right;
		int bottom;
		int place;
	} win_pt;
	VmRectWH    parent_pt;

	int			mode;
	int         prev_mode;
	VmPoint		dist_set[2];
	VmPoint		dist;

	bool        visible;
	bool        inside;

#ifdef USE_SCREEN_D3D_TEXTURE
	CD3DTexture texture;
#endif

	bool create_bitmap(CBitmap **l, DWORD id);
//	bool create_parts(HDC hdc, const led_t &srcl, int x, int y, int w, int h, led_t *l);
	void rect_in(VmRectWH &re, int x, int y, int w, int h);
	void point_in(VmRectWH &dst, int x, int y, VmRectWH &src);
	void point_in(VmRectWH &dst, int x, int y, CBitmap *src);
	void copy_led(VmRectWH &, CBitmap *);
	void copy_led_s(VmRectWH &, CBitmap *, VmRectWH &);
	void copy_led_s(VmRectWH &, CBitmap *, POINT &);

	/* for dialog box */
	virtual void show_dialog() {}
	virtual void move_in_place(int) {}
	virtual void need_update_dialog() {}
public:
	LedBoxBase();
	virtual ~LedBoxBase();
	bool InitScreen();
	void Show(int);
	void SetMode(int);
	void SetPos(int left, int top, int right, int bottom, int place);
	void SetPos(int place);
	void Draw(HDC);
	void Draw(LPDIRECT3DSURFACE9);
	bool Draw(PDIRECT3DDEVICE9);
	void Update(uint64_t flag);

	void SetDistance(int place, const VmPoint *ndist);
	void GetDistance(VmPoint *ndist);

	HRESULT CreateTexture(PDIRECT3DDEVICE9 pD3Device);
	void ReleaseTexture();

	/* for dialog box */
	virtual void CreateDialogBox() {}
	virtual void Move() {}
};

#ifdef _WIN32
#include "../../gui/windows/win_ledbox.h"
#endif

#endif /* _WIN_LEDBOX_BASE_H_ */

