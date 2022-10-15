/** @file wxw_csurface.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2019.03.02 -

	@brief [ csurface ]
*/

#include <wx/rawbmp.h>
#include <wx/dcmemory.h>
#include <wx/thread.h>
#include "wxw_csurface.h"

#if defined(__WXGTK3__)
#include <cairo/cairo.h>
#elif defined(__WXGTK__)
#include <gdk/gdk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#endif

#define CSURFACE_DEPTH	_RGB888

CSurface::CSurface()
{
	suf = NULL;
	sufDc = NULL;
	sufData = NULL;
	sufBuffer = NULL;
//	mux = NULL;
#if defined(__WXGTK__) && !defined(__WXGTK3__)
	sufPixbuf = NULL;
	pixbuf_modified = false;
#endif
}
CSurface::CSurface(long width, long height)
{
	suf = NULL;
	sufDc = NULL;
	sufData = NULL;
	sufBuffer = NULL;
//	mux = NULL;
#if defined(__WXGTK__) && !defined(__WXGTK3__)
	sufPixbuf = NULL;
	pixbuf_modified = false;
#endif

	this->Create(width, height);
}
CSurface::CSurface(long width, long height, const CPixelFormat &pixel_format)
{
	suf = NULL;
	sufDc = NULL;
	sufData = NULL;
	sufBuffer = NULL;
//	mux = NULL;
#if defined(__WXGTK__) && !defined(__WXGTK3__)
	sufPixbuf = NULL;
	pixbuf_modified = false;
#endif

	this->Create(width, height, pixel_format);
}
CSurface::CSurface(long width, long height, CPixelFormat::FormatId force_format)
{
	suf = NULL;
	sufDc = NULL;
	sufData = NULL;
	sufBuffer = NULL;
//	mux = NULL;
#if defined(__WXGTK__) && !defined(__WXGTK3__)
	sufPixbuf = NULL;
	pixbuf_modified = false;
#endif

	this->Create(width, height, force_format);
}
CSurface::~CSurface()
{
	Release();
}

// Create surface
bool CSurface::Create(long width, long height)
{
	return Create(width, height, CPixelFormat());
}

// Create surface
bool CSurface::Create(long width, long height, const CPixelFormat &pixel_format)
{
	sufFmt = pixel_format;
	suf = new wxBitmap((int)width, (int)height, CSURFACE_DEPTH);
	if (suf) {
		sufDc = new wxMemoryDC(*suf);
#if defined(__WXMSW__)
		sufData = new wxAlphaPixelData(*suf);
#endif
	}
#if defined(__WXGTK__) && !defined(__WXGTK3__)
	sufPixbuf = NULL;
	pixbuf_modified = false;
#endif
	return (suf != NULL);
}

bool CSurface::Create(long width, long height, CPixelFormat::FormatId force_format)
{
	return Create(width, height, CPixelFormat(force_format));
}

bool CSurface::Create(const VmRectWH &srcrect)
{
	return Create(srcrect.w, srcrect.h);
}

bool CSurface::Create(const VmRectWH &srcrect, const CPixelFormat &pixel_format)
{
	return Create(srcrect.w, srcrect.h, pixel_format);
}

bool CSurface::Create(const VmRectWH &srcrect, CPixelFormat::FormatId force_format)
{
	return Create(srcrect.w, srcrect.h, force_format);
}

bool CSurface::Create(const VmRectWH &srcrect, CSurface &srcsurface, CPixelFormat::FormatId force_format)
{
	bool valid = Create(srcrect.w, srcrect.h, force_format);
	if (valid) {
		srcsurface.Blit(srcrect, *this, srcrect);
	}
	return valid;
}

bool CSurface::Create(CSurface &srcsurface, const CPixelFormat &pixel_format)
{
	bool valid = Create(srcsurface.Width(), srcsurface.Height(), pixel_format);
	if (valid) {
		srcsurface.Blit(*this);
	}
	return valid;
}

bool CSurface::Create(const wxImage &image, const CPixelFormat &pixel_format)
{
	sufFmt = pixel_format;
	suf = new wxBitmap(image.GetWidth(), image.GetHeight(), CSURFACE_DEPTH);
	bool has_alpha = image.HasAlpha();
	if (suf) {
		wxAlphaPixelData dstData(*suf);
		wxAlphaPixelData::Iterator dstIt(dstData);

		int sright = image.GetWidth();
		int sbottom = image.GetHeight();
		int dright = suf->GetWidth();
		int dbottom = suf->GetHeight();

		for(int sy=0, dy=0; sy < sbottom && dy < dbottom ; sy++, dy++) {
			dstIt.MoveTo(dstData, 0, dy);
			for(int sx=0, dx = 0; sx < sright && dx < dright; sx++, dx++) {
				uint32_t data = (uint32_t)image.GetRed(sx, sy) << (pixel_format.Rshift)
					| (uint32_t)image.GetGreen(sx, sy) << (pixel_format.Gshift)
					| (uint32_t)image.GetBlue(sx, sy) << (pixel_format.Bshift);
				if (has_alpha) {
					data |= (uint32_t)image.GetAlpha(sx, sy) << (pixel_format.Ashift);
				} else {
					data |= (uint32_t)0xff << (pixel_format.Ashift);
				}
				dstIt.Data() = data;
				dstIt++;
			}
		}
	}
	if (suf) {
		sufDc = new wxMemoryDC(*suf);
#if defined(__WXMSW__)
		sufData = new wxAlphaPixelData(*suf);
#endif
	}
#if defined(__WXGTK__) && !defined(__WXGTK3__)
	sufPixbuf = NULL;
	pixbuf_modified = false;
#endif
	return (suf != NULL);
}

// release surface
void CSurface::Release()
{
//	if (mux) {
//		delete mux;
//		mux = NULL;
//	}
#if defined(__WXGTK__) && !defined(__WXGTK3__)
	if (sufPixbuf) {
		g_object_unref(sufPixbuf);
		pixbuf_modified = false;
		sufPixbuf = NULL;
	}
#endif
	if (sufData) {
		delete sufData;
		sufData = NULL;
	}
	if (sufDc) {
		delete sufDc;
		sufDc = NULL;
	}
	if (suf) {
		delete suf;
		suf = NULL;
	}
}

wxBitmap *CSurface::Get()
{
	return suf;
}

wxMemoryDC *CSurface::GetDC()
{
//	UngetBuffer();
//	if (!sufDc) {
//		sufDc = new wxMemoryDC(*suf);
//	}
	return sufDc;
}

void CSurface::UngetDC()
{
//	if (sufDc) {
//		delete sufDc;
//		sufDc = NULL;
//	}
}

/// @attention Under Windows: returned pointer allows the bottom line in the surface
scrntype *CSurface::GetBuffer()
{
#if defined(__WXOSX__)
	if (!sufBuffer) {
		sufBuffer = (void *)suf->GetRawAccess();
	}
#elif defined(__WXGTK__)
# ifdef __WXGTK3__
	if (!sufBuffer) {
		wxDCImpl *impl = sufDc->GetImpl();
		cairo_t *cr = (cairo_t *)impl->GetCairoContext();
		cairo_surface_t *csuf = cairo_get_target(cr);
		sufBuffer = (void *)cairo_image_surface_get_data(csuf);
	}
# else
	if (!sufBuffer) {
		// TODO
		GdkPixbuf *pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, true, 8, suf->GetWidth(), suf->GetHeight());
		sufPixbuf = (void *)pixbuf;
		sufBuffer = (void *)gdk_pixbuf_get_pixels(pixbuf);
	}
	pixbuf_modified = true;
# endif
#elif defined(__WXMSW__)
	if (!sufBuffer) {
		scrntype *p = (scrntype *)suf->GetRawData(*sufData, suf->GetDepth());
		p -= (Width() * (Height() - 1));
		sufBuffer = (void *)p;
	}
#else
	sufBuffer = NULL;
#endif
	return (scrntype *)sufBuffer;
}

scrntype *CSurface::GetBuffer(int y)
{
	scrntype *p = GetBuffer();
#if defined(__WXMSW__)
	p += Width() * (Height() - y - 1);
#else
	p += Width() * y;
#endif
	return p; 
}

void CSurface::UngetBuffer()
{
//	UngetDC();
//	if (sufBuffer) {
//		suf->UngetRawData(*sufData);
//		delete sufData;
//		sufBuffer = NULL;
//		sufData = NULL;
//	}
}

int CSurface::GetBufferSize()
{
	return suf->GetWidth() * suf->GetHeight() * suf->GetDepth() / 8;
}

bool CSurface::IsEnable()
{
	return (suf != NULL);
}

int CSurface::Width()
{
	return suf->GetWidth();
}

int CSurface::Height()
{
	return suf->GetHeight();
}

int CSurface::BitsPerPixel()
{
	return suf->GetDepth();
}

int CSurface::BytesPerPixel()
{
	return suf->GetDepth() / 8;
}

int CSurface::BytesPerLine()
{
	return suf->GetWidth() * suf->GetDepth() / 8;
}

const CPixelFormat &CSurface::GetPixelFormat() const
{
	return sufFmt;
}

bool CSurface::Lock()
{
//	if (!mux) {
//		mux = new wxMutex();
//	}
//	return (mux->Lock() == wxMUTEX_NO_ERROR);
	return true;
}

void CSurface::Unlock()
{
//	if (mux) {
//		mux->Unlock();
//	}
}

bool CSurface::Blit(CSurface &dst)
{
	int w = MIN(Width(), dst.Width());
	int h = MIN(Height(), dst.Height());
#if defined(__WXGTK__) && !defined(__WXGTK3__)
	dst.CopyPixbufToPixmap();
	CopyPixbufToPixmap();
#endif
	return dst.GetDC()->Blit(0, 0, w, h, sufDc, 0, 0);
}

bool CSurface::Blit(CSurface &dst, const VmRectWH &dst_re)
{
	int w = MIN(Width(), dst_re.w);
	int h = MIN(Height(), dst_re.h);
#if defined(__WXGTK__) && !defined(__WXGTK3__)
	dst.CopyPixbufToPixmap();
	CopyPixbufToPixmap();
#endif
	return dst.GetDC()->Blit(dst_re.x, dst_re.y, w, h, sufDc, 0, 0);
}

bool CSurface::Blit(const VmRectWH &src_re, CSurface &dst, const VmRectWH &dst_re)
{
	int w = MIN(src_re.w, dst_re.w);
	int h = MIN(src_re.h, dst_re.h);
#if defined(__WXGTK__) && !defined(__WXGTK3__)
	dst.CopyPixbufToPixmap();
	CopyPixbufToPixmap();
#endif
	return dst.GetDC()->Blit(dst_re.x, dst_re.y, w, h, sufDc, src_re.x, src_re.y);
}

bool CSurface::Blit(const VmRectWH &src_re, wxBitmap &dst, const VmRectWH &dst_re)
{
	int w = MIN(src_re.w, dst_re.w);
	int h = MIN(src_re.h, dst_re.h);
#if defined(__WXGTK__) && !defined(__WXGTK3__)
	CopyPixbufToPixmap();
#endif
	wxMemoryDC dstDc(dst);
	return dstDc.Blit(dst_re.x, dst_re.y, w, h, sufDc, src_re.x, src_re.y);
}

bool CSurface::Blit(const VmRectWH &src_re, wxImage &dst, const VmRectWH &dst_re)
{
	int w = MIN(src_re.w, dst_re.w);
	int h = MIN(src_re.h, dst_re.h);
#if defined(__WXGTK__) && !defined(__WXGTK3__)
	CopyPixbufToPixmap();
#endif
	wxBitmap dstBmp(dst);
	wxMemoryDC dstDc(dstBmp);
	return dstDc.Blit(dst_re.x, dst_re.y, w, h, sufDc, src_re.x, src_re.y);
}

/// blit pixels except alpha channel
bool CSurface::BlitWithoutAlpha(const VmRectWH &src_re, CSurface &dst, const VmRectWH &dst_re)
{
	uint32_t r, g, b;
	wxAlphaPixelData dstDat(*dst.Get());
	wxAlphaPixelData::Iterator dstIt(dstDat);
	for(int sy = src_re.y, dy = dst_re.y; sy < (src_re.y + src_re.h) && dy < (dst_re.y + dst_re.h); sy++, dy++) {
		scrntype *pSrc = GetBuffer(sy) + src_re.x;
		dstIt.MoveTo(dstDat, dst_re.x, dy);
		for(int sx = src_re.x, dx = dst_re.x; sx < (src_re.x + src_re.w) && dx < (dst_re.x + dst_re.w); sx++, dx++) {
			r = ((*pSrc & sufFmt.Rmask) >> sufFmt.Rshift);
			g = ((*pSrc & sufFmt.Gmask) >> sufFmt.Gshift);
			b = ((*pSrc & sufFmt.Bmask) >> sufFmt.Bshift);
			dstIt.Data() = (r << dst.sufFmt.Rshift)
				| (g << dst.sufFmt.Gshift)
				| (b << dst.sufFmt.Bshift)
				| (0xff << dst.sufFmt.Ashift);
			pSrc++;
			dstIt++;
		}
	}
	return true;
}

bool CSurface::Blit24(const VmRectWH &src_re, wxBitmap &dst, const VmRectWH &dst_re)
{
	wxNativePixelData dstDat(dst);
	wxNativePixelData::Iterator dstIt(dstDat);
	for(int sy = src_re.y, dy = dst_re.y; sy < (src_re.y + src_re.h) && dy < (dst_re.y + dst_re.h); sy++, dy++) {
		scrntype *pSrc = GetBuffer(sy) + src_re.x;
		dstIt.MoveTo(dstDat, dst_re.x, dy);
		for(int sx = src_re.x, dx = dst_re.x; sx < (src_re.x + src_re.w) && dx < (dst_re.x + dst_re.w); sx++, dx++) {
			dstIt.Red() = ((*pSrc & sufFmt.Rmask) >> sufFmt.Rshift);
			dstIt.Green() = ((*pSrc & sufFmt.Gmask) >> sufFmt.Gshift);
			dstIt.Blue() = ((*pSrc & sufFmt.Bmask) >> sufFmt.Bshift);
			pSrc++;
			dstIt++;
		}
	}
	return true;
}

bool CSurface::StretchBlit(const VmRectWH &src_re, CSurface &dst, const VmRectWH &dst_re)
{
#if defined(__WXGTK__) && !defined(__WXGTK3__)
	dst.CopyPixbufToPixmap();
	CopyPixbufToPixmap();
#endif
	return dst.GetDC()->StretchBlit(dst_re.x, dst_re.y, dst_re.w, dst_re.h, sufDc, src_re.x, src_re.y, src_re.w, src_re.h);
}

bool CSurface::StretchBlit(const VmRectWH &src_re, wxBitmap &dst, const VmRectWH &dst_re)
{
#if defined(__WXGTK__) && !defined(__WXGTK3__)
	CopyPixbufToPixmap();
#endif
	wxMemoryDC dstDc(dst);
	return dstDc.StretchBlit(dst_re.x, dst_re.y, dst_re.w, dst_re.h, sufDc, src_re.x, src_re.y, src_re.w, src_re.h);
}

bool CSurface::StretchBlit(const VmRectWH &src_re, wxImage &dst, const VmRectWH &dst_re)
{
#if defined(__WXGTK__) && !defined(__WXGTK3__)
	CopyPixbufToPixmap();
#endif
	wxBitmap dstBmp(dst);
	wxMemoryDC dstDc(dstBmp);
	return dstDc.StretchBlit(dst_re.x, dst_re.y, dst_re.w, dst_re.h, sufDc, src_re.x, src_re.y, src_re.w, src_re.h);
}

bool CSurface::BlitFlippedWithoutAlpha(const VmRectWH &src_re, CSurface &dst, const VmRectWH &dst_re)
{
	wxAlphaPixelData dstDat(*dst.Get());
	wxAlphaPixelData::Iterator dstIt(dstDat);
	uint32_t r, g, b;
	for(int sy = (src_re.y + src_re.h - 1), dy = dst_re.y; sy >= src_re.y && dy < (dst_re.y + dst_re.h); sy--, dy++) {
		scrntype *pSrc = GetBuffer(sy) + src_re.x;
		dstIt.MoveTo(dstDat, dst_re.x, dy);
		for(int sx = src_re.x, dx = dst_re.x; sx < (src_re.x + src_re.w) && dx < (dst_re.x + dst_re.w); sx++, dx++) {
			r = ((*pSrc & sufFmt.Rmask) >> sufFmt.Rshift);
			g = ((*pSrc & sufFmt.Gmask) >> sufFmt.Gshift);
			b = ((*pSrc & sufFmt.Bmask) >> sufFmt.Bshift);
			dstIt.Data() = (r << dst.sufFmt.Rshift)
				| (g << dst.sufFmt.Gshift)
				| (b << dst.sufFmt.Bshift)
				| (0xff << dst.sufFmt.Ashift);
			pSrc++;
			dstIt++;
		}
	}
	return true;
}

#if 0
bool CSurface::Blit(CSurface &src, CSurface &dst)
{
	return dst.GetDC()->Blit(0, 0, dst.Width(), dst.Height(), src.GetDC(), 0, 0);
}

bool CSurface::Blit(CSurface &src, VmRectWH &src_re, CSurface &dst, VmRectWH &dst_re)
{
	return dst.GetDC()->Blit(dst_re.x, dst_re.y, dst_re.w, dst_re.h, src.GetDC(), src_re.x, src_re.y);
}

bool CSurface::Blit(CSurface &src, VmRectWH &src_re, wxBitmap &dst, VmRectWH &dst_re)
{
	wxMemoryDC dstDc(dst);
	return dstDc.Blit(dst_re.x, dst_re.y, dst_re.w, dst_re.h, src.GetDC(), src_re.x, src_re.y);
}

bool CSurface::Blit(CSurface &src, VmRectWH &src_re, wxImage &dst, VmRectWH &dst_re)
{
	wxBitmap dstBmp(dst);
	wxMemoryDC dstDc(dstBmp);
	return dstDc.Blit(dst_re.x, dst_re.y, dst_re.w, dst_re.h, src.GetDC(), src_re.x, src_re.y);
}

bool CSurface::StretchBlit(CSurface &src, VmRectWH &src_re, CSurface &dst, VmRectWH &dst_re)
{
	return dst.GetDC()->StretchBlit(dst_re.x, dst_re.y, dst_re.w, dst_re.h, src.GetDC(), src_re.x, src_re.y, dst_re.w, dst_re.h);
}

bool CSurface::StretchBlit(CSurface &src, VmRectWH &src_re, wxBitmap &dst, VmRectWH &dst_re)
{
	wxMemoryDC dstDc(dst);
	return dstDc.StretchBlit(dst_re.x, dst_re.y, dst_re.w, dst_re.h, src.GetDC(), src_re.x, src_re.y, dst_re.w, dst_re.h);
}

bool CSurface::StretchBlit(CSurface &src, VmRectWH &src_re, wxImage &dst, VmRectWH &dst_re)
{
	wxBitmap dstBmp(dst);
	wxMemoryDC dstDc(dstBmp);
	return dstDc.StretchBlit(dst_re.x, dst_re.y, dst_re.w, dst_re.h, src.GetDC(), src_re.x, src_re.y, dst_re.w, dst_re.h);
}

bool CSurface::BlitFlipped(CSurface &src, VmRectWH &reSrc, CSurface &dst, VmRectWH &reDst)
{
	wxAlphaPixelData srcDat(*src.Get());
	wxAlphaPixelData dstDat(*dst.Get());
	wxAlphaPixelData::Iterator srcIt(srcDat);
	wxAlphaPixelData::Iterator dstIt(dstDat);

	int sright = (reSrc.x + reSrc.w);
	int sbottom = (reSrc.y + reSrc.h);
	int dright = (reDst.x + reDst.w);
	int dbottom = (reDst.y + reDst.h);

	dst.Lock();
	src.Lock();
	for(int sy=reSrc.y, dy = dbottom - 1; sy < sbottom && dy >= reDst.y ; sy++, dy--) {
		srcIt.MoveTo(srcDat, reSrc.x, sy);
		dstIt.MoveTo(dstDat, reDst.x, dy);
		for(int sx=reSrc.x, dx = reDst.x; sx < sright && dx < dright; sx++, dx++) {
			dstIt.Data() = srcIt.Data();
//			dstIt.Red() = srcIt.Red();
//			dstIt.Green() = srcIt.Green();
//			dstIt.Blue() = srcIt.Blue();
//			dstIt.Alpha() = srcIt.Alpha();
			srcIt++;
			dstIt++;
		}
	}
	src.Unlock();
	dst.Unlock();

	return true;
}
#endif

bool CSurface::Flip()
{
	int w = suf->GetWidth();
	int h = suf->GetHeight();
	int sy = h - 1;
	int hh = (h >> 1);
	scrntype *sp = GetBuffer();
	scrntype *dp = sp;
	sp += (sy * w);
	for(int dy = 0; dy < hh; dy++) {
		for(int x = 0; x < w; x++) {
			scrntype tmp = sp[x];
			sp[x] = dp[x];
			dp[x] = tmp;
		}
		sp -= w;
		dp += w;
	}
	UngetBuffer();
	return true;
}

/// fill black on surface
void CSurface::FillBlack()
{
	scrntype *p = GetBuffer();
	if (!p) return;

	scrntype data = GetPixelFormat().Map(0, 0, 0);

	for(int i=0; i<(Width() * Height()); i++) {
		*p = data;
		p++;
	}
}

#if defined(__WXGTK__) && !defined(__WXGTK3__)
void CSurface::CopyPixbufToPixmap()
{
	if (sufPixbuf && pixbuf_modified) {
		gdk_draw_pixbuf(suf->GetPixmap(), NULL, (const GdkPixbuf *)sufPixbuf, 0, 0, 0, 0, -1, -1, GDK_RGB_DITHER_NONE, 0, 0);
//		GdkGC *gc = gdk_gc_new(suf->GetPixmap());
//		gdk_draw_rgb_32_image(suf->GetPixmap(), gc, 0, 0, suf->GetWidth(), suf->GetHeight(), GDK_RGB_DITHER_NONE, (const guchar *)sufBuffer, suf->GetWidth() * 4);
//		g_object_unref(gc);
		pixbuf_modified = false;
	}
}
#endif
