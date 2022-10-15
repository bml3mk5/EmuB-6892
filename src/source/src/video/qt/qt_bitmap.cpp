/** @file qt_bitmap.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2017.02.26 -

	@brief [ cbitmap ]
*/

#include <QImage>
#include "qt_bitmap.h"
#include "../../utils.h"

CBitmap::CBitmap()
	: CBitmapBase()
{
}

CBitmap::CBitmap(const _TCHAR *file_name, CPixelFormat *format)
	: CBitmapBase()
{
	this->Load(file_name, format);
}

CBitmap::CBitmap(CBitmap &src, int x, int y, int w, int h)
	: CBitmapBase(src, x, y, w, h)
{
}

CBitmap::~CBitmap()
{
}

bool CBitmap::Load(const _TCHAR *file_name, CPixelFormat *format)
{
	bool enable = false;

	QString wfile_name(QTChar::fromTChar(file_name));
    QImage img(wfile_name);
    if (!img.isNull()) {
        int width = img.width();
        int height = img.height();
		if (format) {
			enable = Create(width, height, *format);
		} else {
			enable = Create(width, height, CPixelFormat::RGBA32);
		}
		if (enable) {
			DrawImage(img, NULL, *suf, NULL);
		}
	}

	return enable;
}
