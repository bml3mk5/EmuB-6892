/** @file qt_cbitmap.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2017.02.26 -

	@brief [ cbitmap ]
*/

#ifndef QT_CBITMAP_BASE_H
#define QT_CBITMAP_BASE_H

#include "../../common.h"
#include "qt_csurface.h"

/**
	@brief manage bitmap
*/
class CBitmapBase : public CSurface
{
public:
	CBitmapBase();
	CBitmapBase(CBitmapBase &src, int x, int y, int w, int h);
	virtual ~CBitmapBase();

	virtual	bool Load(const _TCHAR *file_name, CPixelFormat *format);
	virtual bool Copy(CBitmapBase &src, int x, int y, int w, int h);
};

#if defined(USE_QT)
#include "../../video/qt/qt_bitmap.h"
#endif

#endif /* QT_CBITMAP_BASE_H */
