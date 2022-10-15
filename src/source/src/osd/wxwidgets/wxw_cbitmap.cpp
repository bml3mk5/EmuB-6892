/** @file wxw_cbitmap.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2017.02.26 -

	@brief [ cbitmap ]
*/

//#include <wx/wx.h>
#include <wx/image.h>
#include "wxw_cbitmap.h"
#include "../../cchar.h"

CBitmap::CBitmap()
	: CSurface()
{
}

CBitmap::CBitmap(const _TCHAR *file_name, CPixelFormat *format)
	: CSurface()
{
	this->Load(file_name, format);
}

CBitmap::CBitmap(CBitmap &src, int x, int y, int w, int h)
	: CSurface()
{
	this->Copy(src, x, y, w, h);
}

CBitmap::~CBitmap()
{
}

bool CBitmap::Copy(CBitmap &src, int x, int y, int w, int h)
{
	VmRectWH re;

	re.x = x;
	re.y = y;
	re.w = w;
	re.h = h;

	return Create(re, src);
}

bool CBitmap::Load(const _TCHAR *file_name, CPixelFormat *format)
{
	bool enable = false;

	wxString wfile_name(file_name);
	wxImage img(wfile_name, wxBITMAP_TYPE_PNG);
	if (img.IsOk()) {
		enable = Create(img, *format);
	}

	return enable;
}
