/** @file qt_vkeyboardbase.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2016.12.21 -

	@brief [ virtual keyboard ]
*/

#ifndef QT_VKEYBOARD_BASE_H
#define QT_VKEYBOARD_BASE_H

#include "../../common.h"
#include "../../cbitmap.h"

namespace Vkbd {

enum enKeyKindIndex {
	KEYKIND_NOANIME = -1,
	KEYKIND_NORMAL = 0,
	KEYKIND_ARRAY = 1,
	KEYKIND_TOGGLE = 2
};

typedef struct {
	short x;
	short w;
	short code;
	short kind;	// 0:normal 1:arraykey 2:togglekey
	short kidx;
	short parts_num; // bitmap parts number
} Hori_t;

typedef struct {
	short y;
	short h;
	const Hori_t *px;
} Pos_t;

enum enArrayKeysIndex {
	ARRAYKEYS_RETURN = 0,
	ARRAYKEYS_END
};

enum enToggleKeysIndex {
	TOGGLEKEYS_MODE = 0,
	TOGGLEKEYS_SHIFT,
	TOGGLEKEYS_CTRL,
	TOGGLEKEYS_GRAPH,
	TOGGLEKEYS_END
};

enum enBitmapIdsIndex {
	BITMAPIDS_BASE = 0,
	BITMAPIDS_LED_PARTS,
	BITMAPIDS_VKEY_MODE,
	BITMAPIDS_VKEY_BREAK,
	BITMAPIDS_VKEY_POWER,
#if defined(_MBS1)
	BITMAPIDS_VKEY_RESET,
#endif
	BITMAPIDS_END
};

enum enBitmapPartsIndex {
	BITMAPPARTS_LED_RH = 0,
	BITMAPPARTS_LED_GH,
	BITMAPPARTS_LED_RV,
#if defined(_BML3MK5)
	BITMAPPARTS_LED_GLH,
#endif
	BITMAPPARTS_MODE,
	BITMAPPARTS_BREAK,
	BITMAPPARTS_POWER,
#if defined(_MBS1)
	BITMAPPARTS_RESET,
#endif
	BITMAPPARTS_END
};

typedef struct stBitmap_t {
	short idx;	
	short x;
	short y;
	short w;
	short h;
} Bitmap_t;

enum enLedPartsIndex {
	LEDPARTS_LED_KATA = 0,
	LEDPARTS_LED_HIRA,
	LEDPARTS_LED_CAPS,
	LEDPARTS_MODE_SW,
	LEDPARTS_POWER_SW,
#if defined(_BML3MK5)
	LEDPARTS_POWER_LED,
#endif
	LEDPARTS_END
};

typedef struct stLedPos_t {
	short parts_num;
	short x;
	short y;
} LedPos_t;

// constant tables
extern const struct stLedPos_t cLedPos[];
extern const struct stBitmap_t cBmpParts[];

/**
	@brief VKeyboard Base
*/
class Base
{
protected:
	typedef struct {
		short left;
		short top;
		short right;
		short bottom;
	} Rect_t;

	typedef struct {
		Rect_t re;
		short parts_num; // bitmap parts number
	} PressedInfo_t;

	typedef struct {
		bool pressed;
		short code;
		PressedInfo_t info;
	} PressedKeys_t;

	typedef struct {
		bool pressed;
		short code;
		short nums;	// item nums of arr 
		PressedInfo_t *arr;
	} ArrayKeys_t;

	int offset_x;
	int offset_y;

	bool closed;

	PressedKeys_t pressed_key;

	short pushed_array_key_idx;
	ArrayKeys_t array_keys[ARRAYKEYS_END];

	ArrayKeys_t toggle_keys[TOGGLEKEYS_END];

	short noanime_key_code;

	bool led_onoff[LEDPARTS_END];

	uint8_t *key_status;
	int key_status_size;

	CSurface *pSurface;
	CBitmap  *pBitmaps[BITMAPIDS_END];

	bool load_bitmap(const _TCHAR *res_path);
	void unload_bitmap();
	bool create_surface();
	bool create_bitmap(const _TCHAR *res_path, const _TCHAR *bmp_file, CBitmap **suf);

	virtual void set_pressed_info(PressedInfo_t *, short, short, short, short, short);

	virtual bool update_status_one(short, bool);

	virtual void need_update_led(short, bool);
	virtual void need_update_window(PressedInfo_t *, bool);
	virtual void update_window() {}

public:
	Base();
	virtual ~Base();

	virtual void SetStatusBufferPtr(uint8_t *, int);

	virtual bool Create();
	virtual void Show(bool = true) {}
	virtual void Close();

	virtual void MouseDown(int, int);
	virtual void MouseUp();

	virtual bool UpdateStatus(uint32_t);
};

} /* namespace Vkbd */

#if defined(USE_QT)
#include "../../gui/qt/qt_vkeyboard.h"
#else
namespace Vkbd {

class VKeyboard : public Base
{
public:
	virtual bool Create(const _TCHAR *) { return Base::Create() }
};

} /* namespace Vkbd */
#endif

#endif /* QT_VKEYBOARD_BASE_H */

