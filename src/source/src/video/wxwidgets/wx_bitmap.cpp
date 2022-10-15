/** @file wx_bitmap.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2017.02.26 -

	@brief [ cbitmap ]
*/

#include <wx/wx.h>
#include <wx/image.h>
#include "wx_bitmap.h"
#include "../../cchar.h"

CBitmap::CBitmap()
	: CBitmapBase()
{
}

CBitmap::CBitmap(const _TCHAR *file_name, SDL_PixelFormat *format)
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

bool CBitmap::Load(const _TCHAR *file_name, SDL_PixelFormat *format)
{
	bool enable = false;

	wxPNGHandler *png = new wxPNGHandler;
	wxImage::AddHandler(png);

	wxString wfile_name(file_name);
	wxImage img(wfile_name, wxBITMAP_TYPE_PNG);
	if (img.IsOk()) {
		int width = img.GetWidth();
		int height = img.GetHeight();

		enable = Create(width, height, format, format ? 0 : CSurface::BGRA32);
		if (enable) {
			Lock();
			scrntype *dot = (scrntype *)suf->pixels;
			for(int y=0; y<height; y++) {
				for(int x=0; x<width; x++) {
					dot[y * width + x] = SDL_MapRGB(suf->format,
						(Uint8)img.GetRed(x, y),
						(Uint8)img.GetGreen(x, y),
						(Uint8)img.GetBlue(x, y)
					);
				}
			}
			Unlock();
		}
	}

	wxImage::RemoveHandler(png->GetName());

	return enable;
}
