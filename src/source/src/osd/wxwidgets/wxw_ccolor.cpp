/** @file wxw_ccolor.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2019.10.01 -

	@brief [ ccolor ]
*/

#include "wxw_ccolor.h"
#include "../../cpixfmt.h"

CColor::CColor()
	: wxColour()
{
}

CColor::CColor(const CColor &src)
	: wxColour(src)
{
}

CColor::~CColor()
{
}

void CColor::Set(const CPixelFormat &format, unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha)
{
	int s_r, s_g, s_b, s_a;
	wxUint32 r, g, b, a;
	CPixelFormat dstFmt;
	CPixelFormat colFmt(CPixelFormat::RGBA32);
#if defined(__WXMSW__)
	dstFmt.PresetBGRA();
#elif defined(__WXOSX__) || defined(__WXMAC__)
	dstFmt.PresetARGB();
#else
	dstFmt.PresetRGBA();
#endif
	r = (wxUint32)red;
	g = (wxUint32)green;
	b = (wxUint32)blue;
	a = (wxUint32)alpha;

	r <<= format.Rshift;
	g <<= format.Gshift;
	b <<= format.Bshift;
	a <<= format.Ashift;

	s_r = (int)colFmt.Rshift - dstFmt.Rshift;
	s_g = (int)colFmt.Gshift - dstFmt.Gshift;
	s_b = (int)colFmt.Bshift - dstFmt.Bshift;
	s_a = (int)colFmt.Ashift - dstFmt.Ashift;

	r = (s_r >= 0 ? r << s_r : r >> -s_r);
	g = (s_g >= 0 ? g << s_g : g >> -s_g);
	b = (s_b >= 0 ? b << s_b : b >> -s_b);
	a = (s_a >= 0 ? a << s_a : a >> -s_a);

	SetRGBA(r | g | b | a);
}
