/** @file qt_vkeyboardbase.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2016.12.21 -

	@brief [ virtual keyboard ]
*/

#include <string.h>
//#include <malloc.h>
#include "qt_vkeyboardbase.h"
#include <QPainter>
#include "../../emu.h"
#include "../../fifo.h"
//#include "../../logging.h"

//#if defined(_BML3MK5)
//#include "../../gui/vkeyboard_bml3mk5.h"
//#elif defined(_MBS1)
//#include "../../gui/vkeyboard_mbs1.h"
//#endif

#include "../../utils.h"
#include "../../utility.h"

extern EMU *emu;

namespace Vkbd {

const _TCHAR *csBitmapFileNames[BITMAPIDS_END] = {
#if defined(_BML3MK5)
	_T("bml3mk5_keyboard1.png"),
	_T("ledparts.png"),
	_T("bml3mk5_key_mode.png"),
	_T("bml3mk5_key_break.png"),
	_T("bml3mk5_key_power.png")
#elif defined(_MBS1)
	_T("mbs1_keyboard2.png"),
	_T("ledparts4.png"),
	_T("mbs1_key_mode.png"),
	_T("mbs1_key_break.png"),
	_T("mbs1_key_power.png"),
	_T("mbs1_key_reset.png")
#endif
};

OSDBase::OSDBase() : Base()
{
}

OSDBase::~OSDBase()
{
}

bool OSDBase::load_bitmap(const _TCHAR *res_path)
{
	bool rc = true;
	for(int i=0; i<BITMAPIDS_END; i++) {
		if (!pBitmaps[i]) {
			rc = (rc && create_bitmap(res_path, csBitmapFileNames[i], &pBitmaps[i]));
		}
	}
	if (rc) {
		create_surface();
	}
	return rc;
}

bool OSDBase::create_surface()
{
	if (pSurface) return true;
	if (!pBitmaps[BITMAPIDS_BASE]) return false;
	if (!pBitmaps[BITMAPIDS_BASE]->IsEnable()) return false;

#if defined(USE_QT)
	pSurface = new CSurface(
		pBitmaps[BITMAPIDS_BASE]->Width(),
		pBitmaps[BITMAPIDS_BASE]->Height(),
		QImage::Format_RGB32);
#endif
	if (pSurface == NULL) return false;

	pBitmaps[BITMAPIDS_BASE]->Blit(*pSurface);
	return true;
}

bool OSDBase::create_bitmap(const _TCHAR *res_path, const _TCHAR *bmp_file, CBitmap **suf)
{
	_TCHAR path[_MAX_PATH];
	UTILITY::stprintf(path, _MAX_PATH, _T("%s%s") ,res_path, bmp_file);
	*suf = new CBitmap(path, NULL);
	if (*suf == NULL || !(*suf)->IsEnable()) return false;
	return true;
}

void OSDBase::unload_bitmap()
{
	Base::unload_bitmap();
}

/// @param[in] info : information of mouse
/// @param[in] onoff : pressed a key?
void OSDBase::need_update_window_base(PressedInfo_t *info, bool onoff)
{
	const Bitmap_t *bp = NULL;

	VmRectWH dstrect;
	dstrect.x = info->re.left;
	dstrect.y = info->re.top;
	dstrect.w = info->re.right - info->re.left + 1;
	dstrect.h = info->re.bottom - info->re.top + 1;
	int parts_num = info->parts_num;

	if (onoff) {
		// key pressed
		VmRectWH srcrect;
		if (parts_num >= 0) {
			// another parts
			bp = &cBmpParts[parts_num];
			srcrect.x = bp->x;
			srcrect.y = bp->y;
			srcrect.w = bp->w;
			srcrect.h = bp->h;
			if (pBitmaps[bp->idx]) pBitmaps[bp->idx]->Blit(srcrect, *pSurface, dstrect);
		} else {
			// set base parts shifted down 3px
			QPainter qp(pSurface->Get());
#if defined(_BML3MK5)
            qp.fillRect(dstrect.x, dstrect.y, dstrect.w, dstrect.h, QColor(0x80, 0x80, 0x80));
#elif defined(_MBS1)
            qp.fillRect(dstrect.x, dstrect.y, dstrect.w, dstrect.h, QColor(0x30, 0x30, 0x30));
#endif
            qp.end();
			dstrect.h -= 3;
			srcrect = dstrect;
			dstrect.y += 3;
			pBitmaps[BITMAPIDS_BASE]->Blit(srcrect, *pSurface, dstrect);
		}
	} else {
		// key released
		pBitmaps[BITMAPIDS_BASE]->Blit(dstrect, *pSurface, dstrect);
	}
}

} /* namespace Vkbd */
