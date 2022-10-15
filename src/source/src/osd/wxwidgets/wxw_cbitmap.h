/** @file wxw_cbitmap.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2017.02.26 -

	@brief [ cbitmap ]
*/

#ifndef WXW_CBITMAP_H
#define WXW_CBITMAP_H

#include "wxw_csurface.h"

/**
	@brief manage bitmap
*/
class CBitmap : public CSurface
{
protected:

public:
	CBitmap();
	CBitmap(const _TCHAR *file_name, CPixelFormat *format);
	CBitmap(CBitmap &src, int x, int y, int w, int h);
	~CBitmap();

	bool Copy(CBitmap &src, int x, int y, int w, int h);
	bool Load(const _TCHAR *file_name, CPixelFormat *format);
};

#endif /* WXW_CBITMAP_H */
