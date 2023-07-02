/** @file qt_vkeyboardbase.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2016.12.21 -

	@brief [ virtual keyboard ]
*/

#ifndef QT_VKEYBOARD_BASE_H
#define QT_VKEYBOARD_BASE_H

#include "../../common.h"
#include "../vkeyboardbase.h"

class FIFOINT;

namespace Vkbd {

/**
	@brief VKeyboard Base for Qt
*/
class OSDBase : public Base
{
protected:
	bool load_bitmap(const _TCHAR *res_path);
	void unload_bitmap();
	bool create_surface();
	bool create_bitmap(const _TCHAR *res_path, const _TCHAR *bmp_file, CBitmap **suf);

//	void update_parts(const Pos_t *, const Hori_t *, bool);
//	void mouse_up(const Pos_t *, const Hori_t *);

//	inline void set_pressed_info(PressedInfo_t *, short, short, short, short, short);

//	virtual bool update_status_one(short, bool);

//	virtual void need_update_led(short, LedStat_t &);
	virtual void need_update_window_base(PressedInfo_t *, bool);
	virtual void update_window() {}

public:
	OSDBase();
	virtual ~OSDBase();

//	virtual void SetStatusBufferPtr(uint8_t *, int, uint8_t);
//	virtual void SetHistoryBufferPtr(FIFOINT *);

	virtual void Show(bool = true) {}
//	virtual void Close();

//	virtual void MouseDown(int, int);
//	virtual void MouseUp();

//	virtual bool UpdateStatus(uint32_t);
};

} /* namespace Vkbd */

#if defined(USE_QT)
#include "../../gui/qt/qt_vkeyboard.h"
#else
namespace Vkbd {

class VKeyboard : public OSDBase
{
public:
	virtual bool Create(const _TCHAR *) { return false; }
};

} /* namespace Vkbd */
#endif

#endif /* QT_VKEYBOARD_BASE_H */

