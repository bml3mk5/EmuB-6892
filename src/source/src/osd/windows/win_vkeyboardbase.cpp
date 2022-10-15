/** @file win_vkeyboardbase.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2016.12.21 -

	@brief [ virtual keyboard ]
*/

#include <string.h>
//#include <malloc.h>
#include "win_vkeyboardbase.h"

#if defined(_BML3MK5)
#include "../../gui/vkeyboard_bml3mk5.h"
#elif defined(_MBS1)
#include "../../gui/vkeyboard_mbs1.h"
#endif

namespace Vkbd {

static const int csBitmapIds[BITMAPIDS_END] = {
	IDB_VKEYBOARD,
	IDB_LED_PARTS,
	IDB_VKEY_MODE,
	IDB_VKEY_BREAK,
	IDB_VKEY_POWER,
#if defined(_MBS1)
	IDB_VKEY_RESET
#endif
};

Base::Base()
{
	memset(&pressed_key, 0, sizeof(pressed_key));
	pressed_key.pressed = false;
	pressed_key.code = -1;
	pressed_key.info.parts_num = -1;

	pushed_array_key_idx = -1;
	for(int i=0; i<ARRAYKEYS_END; i++) {
		array_keys[i].pressed = false;
		array_keys[i].code = -1;
		array_keys[i].nums = 0;
		array_keys[i].arr = NULL;
	}
	for(int i=0; i<TOGGLEKEYS_END; i++) {
		toggle_keys[i].pressed = false;
		toggle_keys[i].code = -1;
		toggle_keys[i].nums = 0;
		toggle_keys[i].arr = NULL;
	}

	memset(led_onoff, 0, sizeof(led_onoff));

	noanime_key_code = -1;

	offset_x = 0;
	offset_y = 0;

	key_status = NULL;
	key_status_size = 0;

	closed = false;

//	hInstance = hInst;
//	hVkbd = NULL;
	pSurface = NULL;
	hBrush = NULL;
	for(int i=0; i<BITMAPIDS_END; i++) {
		pBitmaps[i] = NULL;
	}

	// set array and toggle keys
	int j = 0;
	while(1) {
		const Pos_t *vy = &cVkbdKeyPos[j++];
		if (vy->h == 0) break;

		int i = 0;
		while(1) {
			const Hori_t *vx = &vy->px[i++];
			if (vx->w == 0) break;

			if (vx->kind > 0) {
				// array / toggle
				ArrayKeys_t *tk;
				if (vx->kind >= 2) tk = &toggle_keys[vx->kidx];
				else  tk = &array_keys[vx->kidx];
				tk->nums++;
				if (tk->nums > 1) {
					tk->arr = (PressedInfo_t *)realloc(tk->arr, sizeof(PressedInfo_t) * tk->nums);
				} else {
					tk->arr = (PressedInfo_t *)malloc(sizeof(PressedInfo_t));
				}
				PressedInfo_t *itm = &tk->arr[tk->nums-1];
				set_pressed_info(itm, vx->x, vy->y, vx->w, vy->h, vx->parts_num);
			}
		}
	}
}

Base::~Base()
{
	for(int i=0; i<ARRAYKEYS_END; i++) {
		ArrayKeys_t *tk = &array_keys[i];
		if (tk->nums > 0) {
			free(tk->arr);
			tk->nums = 0;
			tk->arr = NULL;
		}
	}
	for(int i=0; i<TOGGLEKEYS_END; i++) {
		ArrayKeys_t *tk = &toggle_keys[i];
		if (tk->nums > 0) {
			free(tk->arr);
			tk->nums = 0;
			tk->arr = NULL;
		}
	}
	unload_bitmap();
}

void Base::SetStatusBufferPtr(uint8_t *buffer, int size)
{
	key_status = buffer;
	key_status_size = size;
}

void Base::Create()
{
	closed = false;
}

void Base::Close()
{
	if (key_status) {
		for(int i=0; i<key_status_size; i++) {
			key_status[i] &= ~0x10;
		}
	}
	for(int i=0; i<ARRAYKEYS_END; i++) {
		ArrayKeys_t *tk = &array_keys[i];
		tk->pressed = false;
	}
	for(int i=0; i<TOGGLEKEYS_END; i++) {
		ArrayKeys_t *tk = &toggle_keys[i];
		tk->pressed = false;
	}

	closed = true;
}

bool Base::UpdateStatus(uint32_t flag)
{
	bool changed = false;

	// katakana
	changed |= update_status_one(LEDPARTS_LED_KATA, (flag & 0x10) != 0);
	// hiragana
	changed |= update_status_one(LEDPARTS_LED_HIRA, (flag & 0x20) != 0);
	// caps
	changed |= update_status_one(LEDPARTS_LED_CAPS, (flag & 0x40) == 0);
	// mode switch
	changed |= update_status_one(LEDPARTS_MODE_SW, (flag & 0x2) != 0);
	// power switch
	changed |= update_status_one(LEDPARTS_POWER_SW, (flag & 0x1) == 0);
#if defined(_BML3MK5)
	changed |= update_status_one(LEDPARTS_POWER_LED, (flag & 0x1) == 0);
#endif

	if (changed) update_window();

	return (!closed);
}

bool Base::update_status_one(short idx, bool onoff)
{
	if (onoff != led_onoff[idx]) {
		led_onoff[idx] = onoff;
		need_update_led(idx, onoff);
		return true;
	}
	return false;
}

void Base::MouseDown(int px, int py)
{
	bool found = false;
	bool pressed = false;

	noanime_key_code = -1;

	int j = 0;
	while(!found) {
		const Pos_t *vy = &cVkbdKeyPos[j++];
		int y = vy->y + offset_y;
		int h = vy->h;
		if (h == 0) break;
		if (py < y || (y + h) <= py) continue;

		int i = 0;
		while(!found) {
			const Hori_t *vx = &vy->px[i++];
			int x = vx->x + offset_x;
			int w = vx->w;
			if (w == 0) break;
			if (x <= px && px < (x + w)) {
				found = true;

				if (vx->kind > 0) {
					// array or toggle key
					ArrayKeys_t *tk;
					if (vx->kind >= 2) {
						// toggle
						tk = &toggle_keys[vx->kidx];
						tk->pressed = !tk->pressed;
						pressed = tk->pressed;
						tk->code = vx->code;
					} else {
						// array
						tk = &array_keys[vx->kidx];
						tk->pressed = true;
						pressed = tk->pressed;
						tk->code = vx->code;
						pushed_array_key_idx = vx->kidx;
					}
					for(int n=0; n<tk->nums; n++) {
						need_update_window(&tk->arr[n], pressed);
					}
					update_window();
				} else if (vx->kind < 0) {
					// not animation
					noanime_key_code = vx->code;
					pressed = true;
				} else {
					// press a key
					pressed_key.pressed = true;
					pressed = pressed_key.pressed;
					pressed_key.code = vx->code;
					pushed_array_key_idx = -1;
					set_pressed_info(&pressed_key.info, vx->x, vy->y, vx->w, vy->h, vx->parts_num);
					need_update_window(&pressed_key.info, pressed);
					update_window();
				}

				// set key status
				if (key_status && 0 <= vx->code && vx->code < key_status_size) {
					key_status[vx->code] = (pressed ? key_status[vx->code] | 0x10 : key_status[vx->code] & ~0x10);
				}
			}
		}
	}
}

void Base::MouseUp()
{
	short code = -1;
	if (pushed_array_key_idx >= 0) {
		ArrayKeys_t *tk = &array_keys[pushed_array_key_idx];
		tk->pressed = false;
		code = tk->code;
		tk->code = -1;
		for(int n=0; n<tk->nums; n++) {
			need_update_window(&tk->arr[n], false);
		}
		update_window();
		// clear key status
		if (key_status && 0 <= code && code < key_status_size) {
			key_status[code] &= ~0x10;
		}
	}
	if (noanime_key_code >= 0) {
		code = noanime_key_code;
		noanime_key_code = -1;
		// clear key status
		if (key_status && 0 <= code && code < key_status_size) {
			key_status[code] &= ~0x10;
		}
	}
	if (pressed_key.code >= 0) {
		pressed_key.pressed = false;
		code = pressed_key.code;
		pressed_key.code = -1;
		need_update_window(&pressed_key.info, false);
		update_window();
		// clear key status
		if (key_status && 0 <= code && code < key_status_size) {
			key_status[code] &= ~0x10;
		}
	}
}

void Base::set_pressed_info(PressedInfo_t *info, short x, short y, short w, short h, short parts_num)
{
	info->re.left = x + offset_x;
	info->re.top = y + offset_y;
	info->re.right = x + w + offset_x;
	info->re.bottom = y + h + offset_y;
	info->parts_num = parts_num;
	if (parts_num >= 0) {
		const Bitmap_t *bp = &cBmpParts[parts_num];
		if (bp->w > 0) info->re.right = x + bp->w + offset_x;
		if (bp->h > 0) info->re.bottom = y + bp->h + offset_y;
	}
}

void Base::need_update_led(short idx, bool onoff)
{
	const LedPos_t *lp = &cLedPos[idx]; 
	const Bitmap_t *bp = &cBmpParts[lp->parts_num];

	PressedInfo_t info;
	info.parts_num = lp->parts_num;
	info.re.left = lp->x + offset_x;
	info.re.top = lp->y + offset_y;
	info.re.right = info.re.left + bp->w;
	info.re.bottom = info.re.top + bp->h;
	need_update_window(&info, onoff);
}

void Base::load_bitmap()
{
	// set bitmaps
//	if (!hVkbd) return;

//	HDC hdc = ::GetDC(hVkbd);
	for(int i=0; i<BITMAPIDS_END; i++) {
		pBitmaps[i] = new CBitmap(csBitmapIds[i], 1);
	}
//	::ReleaseDC(hVkbd, hdc);

	// brush
#ifdef _BML3MK5
	hBrush = ::CreateSolidBrush(RGB(0x80,0x80,0x80));
#else
	hBrush = ::CreateSolidBrush(RGB(0x30,0x30,0x30));
#endif
}

bool Base::create_surface()
{
	if (pSurface) return true;
//	if (!hVkbd) return false;

	CBitmap *bmp = pBitmaps[BITMAPIDS_BASE];
//	HDC hdc = ::GetDC(hVkbd);
	pSurface = new CSurface(bmp->Width(), bmp->Height());
	::BitBlt(pSurface->GetDC(), 0, 0, pSurface->Width(), pSurface->Height(), bmp->GetDC(), 0, 0, SRCCOPY);
//	::ReleaseDC(hVkbd, hdc);
	return true;
}

void Base::unload_bitmap()
{
	if (pSurface) {
		delete pSurface;
		pSurface = NULL;
	}

	// release bitmaps
	for(int i=0; i<BITMAPIDS_END; i++) {
		delete pBitmaps[i];
		pBitmaps[i] = NULL;
	}

	if (hBrush) {
		::DeleteObject(hBrush);
		hBrush = NULL;
	}
}

void Base::need_update_window(PressedInfo_t *info, bool onoff)
{
	const Bitmap_t *bp = NULL;

	Rect_t dstrect = info->re;

	if (onoff) {
		// on
		if (info->parts_num >= 0) {
			bp = &cBmpParts[info->parts_num];
			blit_surface(pBitmaps[bp->idx], bp->x, bp->y, &dstrect);
		} else {
			fill_rect(&dstrect);
			dstrect.top += 3;
			blit_surface(pBitmaps[BITMAPIDS_BASE], info->re.left, info->re.top,	&dstrect);
		}
	} else {
		// off
		blit_surface(pBitmaps[BITMAPIDS_BASE], dstrect.left, dstrect.top, &dstrect);
	}
}

void Base::fill_rect(Rect_t *re)
{
	if (!pSurface) return;

	RECT wre = { re->left, re->top, re->right, re->bottom };
	::FillRect(pSurface->GetDC(), &wre, hBrush);
}

bool Base::blit_surface(CBitmap *pSrcBmp, short src_x, short src_y, Rect_t *dst_re)
{
	if (!pSurface || !pSrcBmp) return false;

	::BitBlt(pSurface->GetDC(), dst_re->left, dst_re->top, dst_re->right - dst_re->left, dst_re->bottom - dst_re->top,
		pSrcBmp->GetDC(), src_x, src_y, SRCCOPY);

	return true;
}

} /* namespace Vkbd */
