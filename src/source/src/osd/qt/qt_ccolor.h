/** @file qt_ccolor.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2019.10.01 -

	@brief [ ccolor ]
*/

#ifndef QT_CCOLOR_H
#define QT_CCOLOR_H

#include <QColor>

class CPixelFormat;

/**
	@brief manage color
*/
class CColor : public QColor
{
public:
	CColor();
	CColor(const CColor &src);
	~CColor();

	void Set(const CPixelFormat &format, unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha = 0xff);
};

#endif
