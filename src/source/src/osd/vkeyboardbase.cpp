/** @file vkeyboardbase.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2022.12.21 -

	@brief [ virtual keyboard ]
*/

#include <string.h>
//#include <malloc.h>
#include "vkeyboardbase.h"
#include "../emu.h"
#include "../fifo.h"
//#include "../logging.h"

#if defined(_BML3MK5)
#include "../gui/vkeyboard_bml3mk5.h"
#elif defined(_MBS1)
#include "../gui/vkeyboard_mbs1.h"
#endif

extern EMU *emu;

namespace Vkbd {

Base::Base()
{
	memset(&pressed_key, 0, sizeof(pressed_key));
	pressed_key.code = -1;
	pressed_key.array_idx = -1;

	for(int i=0; i<ARRAYKEYS_END; i++) {
		array_keys[i].a.code = -1;
		array_keys[i].a.nums = 0;
		array_keys[i].a.arr = NULL;
	}
	for(int i=0; i<TOGGLEKEYS_END; i++) {
		toggle_keys[i].pressed = false;
		toggle_keys[i].a.code = -1;
		toggle_keys[i].a.nums = 0;
		toggle_keys[i].a.arr = NULL;
	}

	for(int i=0; i<LEDPARTS_END; i++) {
		led_stat[i].on = false;
		led_stat[i].code = -1;
	}

	offset_x = 0;
	offset_y = 0;

	key_status = NULL;
	key_status_mask = 0;
	key_status_size = 0;
	key_history = NULL;

	prepared = false;
	closed = false;

	pSurface = NULL;
	for(int i=0; i<BITMAPIDS_END; i++) {
		pBitmaps[i] = NULL;
	}

	// set array and toggle keys
	code_rev_map_size = 0;
	int j = 0;
	while(1) {
		const Pos_t *vy = &cVkbdKeyPos[j++];
		if (vy->h == 0) break;

		int i = 0;
		while(1) {
			const Hori_t *vx = &vy->px[i++];
			if (vx->w == 0) break;

			if (code_rev_map_size < vx->code) {
				code_rev_map_size = vx->code;
			}

			if (vx->kind > KEYKIND_NORMAL) {
				// array / toggle
				ArrKeys_t *tk;
				if (vx->kind >= KEYKIND_TOGGLE) tk = &toggle_keys[vx->kidx].a;
				else  tk = &array_keys[vx->kidx].a;
				tk->nums++;
				if (tk->nums > 1) {
					tk->arr = (PressedInfo_t *)realloc(tk->arr, sizeof(PressedInfo_t) * tk->nums);
				} else {
					tk->code = vx->code;
					tk->arr = (PressedInfo_t *)malloc(sizeof(PressedInfo_t));
				}
				PressedInfo_t *itm = &tk->arr[tk->nums-1];
				set_pressed_info(itm, vx->x, vy->y, vx->w, vy->h, vx->parts_num);
			}
		}
	}

	// set reverse map
	code_rev_map_size++;
	code_rev_map = (PosHoriMap_t *)malloc(sizeof(PosHoriMap_t) * code_rev_map_size);
	for(int i=0; i<code_rev_map_size; i++) {
		code_rev_map[i].vert = -1;
		code_rev_map[i].hori = -1;
	}
	j = 0;
	while(1) {
		const Pos_t *vy = &cVkbdKeyPos[j];
		if (vy->h == 0) break;

		int i = 0;
		while(1) {
			const Hori_t *vx = &vy->px[i];
			if (vx->w == 0) break;

			if (vx->code >= 0) {
				if (code_rev_map[vx->code].vert < 0) {
					code_rev_map[vx->code].vert = j;
					code_rev_map[vx->code].hori = i;
				}
			}
			i++;
		}
		j++;
	}
}

Base::~Base()
{
	if (code_rev_map) {
		free(code_rev_map);
		code_rev_map = NULL;
		code_rev_map_size = 0;
	}

	for(int i=0; i<ARRAYKEYS_END; i++) {
		ArrayKeys_t *tk = &array_keys[i];
		if (tk->a.nums > 0) {
			free(tk->a.arr);
			tk->a.nums = 0;
			tk->a.arr = NULL;
		}
	}
	for(int i=0; i<TOGGLEKEYS_END; i++) {
		ToggleKeys_t *tk = &toggle_keys[i];
		if (tk->a.nums > 0) {
			free(tk->a.arr);
			tk->a.nums = 0;
			tk->a.arr = NULL;
		}
	}
	unload_bitmap();
}

void Base::SetStatusBufferPtr(uint8_t *buffer, int size, uint8_t mask)
{
	key_status = buffer;
	key_status_size = size;
	key_status_mask = mask;
}

void Base::SetHistoryBufferPtr(FIFOINT *buffer)
{
	key_history = buffer;
	if (key_history) {
		key_history->clear();
	}
}

void Base::Show(bool show)
{
	prepared = true;
}

void Base::CloseBase()
{
	if (key_status) {
		for(int i=0; i<key_status_size; i++) {
			key_status[i] &= ~key_status_mask;
		}
	}
//	for(int i=0; i<ARRAYKEYS_END; i++) {
//		ArrayKeys_t *tk = &array_keys[i];
//		tk->pressed = false;
//	}
	for(int i=0; i<TOGGLEKEYS_END; i++) {
		ToggleKeys_t *tk = &toggle_keys[i];
		tk->pressed = false;
	}

	closed = true;
}

bool Base::UpdateStatus(uint32_t flag)
{
	bool changed = false;

	if (!prepared) return (!closed);

	// katakana
	changed |= update_status_one(LEDPARTS_LED_KATA, (flag & 0x10) != 0);
	// hiragana
	changed |= update_status_one(LEDPARTS_LED_HIRA, (flag & 0x20) != 0);
	// caps
	changed |= update_status_one(LEDPARTS_LED_CAPS, (flag & 0x40) == 0);
	// reset switch
	changed |= update_status_one(LEDPARTS_RESET_SW, (flag & 0x80) != 0);
	// mode switch
	changed |= update_status_one(LEDPARTS_MODE_SW, (flag & 0x2) != 0);
	// power switch
	changed |= update_status_one(LEDPARTS_POWER_SW, (flag & 0x1) == 0);
#if defined(_BML3MK5)
	changed |= update_status_one(LEDPARTS_POWER_LED, (flag & 0x1) == 0);
#endif


	// key on/off history
	int cnt = key_history->count();
	for(int i=0; i<cnt; i++) {
		int code = key_history->read();
		if (code < 0 || code >= code_rev_map_size) continue;
		int vert = code_rev_map[code].vert;
		if (vert < 0) continue;
		bool pressed = (key_status[code] != 0);
		const Pos_t *vy = &cVkbdKeyPos[vert];
		const Hori_t *vx = &vy->px[code_rev_map[code].hori];
		update_parts(vy, vx, pressed);
		changed = true;
	}

	if (changed) update_window();

	return (!closed);
}

bool Base::update_status_one(short idx, bool onoff)
{
	if (onoff != led_stat[idx].on) {
		led_stat[idx].on = onoff;
		need_update_led(idx, led_stat[idx]);
		return true;
	}
	return false;
}

void Base::update_parts(const Pos_t *vy, const Hori_t *vx, bool onoff)
{
	if (vx->kind > KEYKIND_NORMAL) {
		// array or toggle key
		ArrKeys_t *tk;
		if (vx->kind >= KEYKIND_TOGGLE) {
			// toggle
			tk = &toggle_keys[vx->kidx].a;
		} else {
			// array
			tk = &array_keys[vx->kidx].a;
		}
		for(int n=0; n<tk->nums; n++) {
			need_update_window(&tk->arr[n], onoff);
		}
	} else if (vx->kind < KEYKIND_NORMAL) {
		// no animation
	} else {
		// press a key
		PressedInfo_t info;
		set_pressed_info(&info, vx->x, vy->y, vx->w, vy->h, vx->parts_num);
		need_update_window(&info, onoff);
//		logging->out_debugf(_T("Vkbd::Base::MouseDown: led:%d onoff:%d"), pressed_key.info.led_parts_num, pressed_key.info.led_bitmap_num);
	}
}

void Base::MouseDown(int px, int py)
{
	bool found = false;
	bool pressed = false;

//	noanime_key_code = -1;

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

				pressed_key.kind = vx->kind;
				pressed_key.code = vx->code;
				pressed_key.array_idx = -1;
				if (vx->kind > KEYKIND_NORMAL) {
					// array or toggle key
					if (vx->kind >= KEYKIND_TOGGLE) {
						// toggle
						ToggleKeys_t *tk;
						tk = &toggle_keys[vx->kidx];
						tk->pressed = !tk->pressed;
						pressed = tk->pressed;
					} else {
						// array
						pressed = true;
						pressed_key.array_idx = vx->kidx;
					}
				} else {
					// press a key
					pressed = true;
				}

				// set key status
				if (pressed) {
					emu->vkey_key_down(vx->code, key_status_mask);
				} else {
					emu->vkey_key_up(vx->code, key_status_mask);
				}
			}
		}
	}
}

void Base::MouseUp()
{
	short kind = pressed_key.kind;
	short code = pressed_key.code;
	if (code >= 0 && kind < KEYKIND_TOGGLE) {
		// clear key status
		emu->vkey_key_up(code, key_status_mask);
	}
	pressed_key.code = -1;
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

/// @param[in] idx : index of cLedPos
/// @param[in] st  : LED status
void Base::need_update_led(short idx, LedStat_t &st)
{
	const LedPos_t *lp = &cLedPos[idx]; 
	const Bitmap_t *bp = &cBmpParts[lp->parts_num];

	PressedInfo_t info;
	info.parts_num = lp->parts_num;
	info.re.left = lp->x + offset_x;
	info.re.top = lp->y + offset_y;
	info.re.right = info.re.left + bp->w;
	info.re.bottom = info.re.top + bp->h;

	bool onoff = st.on;
	need_update_window(&info, onoff);
}

void Base::unload_bitmap()
{
	delete pSurface;
	pSurface = NULL;

	// release bitmaps
	for(int i=0; i<BITMAPIDS_END; i++) {
		delete pBitmaps[i];
		pBitmaps[i] = NULL;
	}
}

/// @param[in] info : information of mouse
/// @param[in] onoff : pressed a key?
void Base::need_update_window(PressedInfo_t *info, bool onoff)
{
}

} /* namespace Vkbd */
