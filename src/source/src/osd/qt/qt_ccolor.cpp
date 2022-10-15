/** @file qt_ccolor.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2019.10.01 -

	@brief [ ccolor ]
*/

#include "qt_ccolor.h"
#include "../../cpixfmt.h"

CColor::CColor()
	: QColor()
{
}

CColor::CColor(const CColor &src)
	: QColor(src)
{
}


CColor::~CColor()
{
}

void CColor::Set(const CPixelFormat &UNUSED_PARAM(format), unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha)
{
	this->setRed(red);
	this->setGreen(green);
	this->setBlue(blue);
	this->setAlpha(alpha);
}
