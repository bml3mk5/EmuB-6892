/** @file wxw_ccolor.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2019.10.01 -

	@brief [ ccolor ]
*/

#ifndef WXW_CCOLOR_H
#define WXW_CCOLOR_H

#include <wx/colour.h>

class CPixelFormat;

/**
	@brief manage color
*/
class CColor : public wxColour
{
public:
	CColor();
	CColor(const CColor &src);
	~CColor();

	void Set(const CPixelFormat &format, unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha = 0xff);
};

#endif /* WXW_CCOLOR_H */
