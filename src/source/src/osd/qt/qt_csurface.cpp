/** @file qt_csurface.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2017.01.03 -

	@brief [ csurface ]
*/
#include "qt_csurface.h"
#include "qt_utils.h"

#include <QPainter>

CSurface::CSurface()
{
	suf = nullptr;
	sufbuf = nullptr;
}
CSurface::CSurface(long width, long height)
{
	suf = nullptr;
	sufbuf = nullptr;

	this->Create(width, height);
}
CSurface::CSurface(long width, long height, const CPixelFormat &pixel_format)
{
	suf = nullptr;
	sufbuf = nullptr;

	this->Create(width, height, pixel_format);
}
CSurface::CSurface(long width, long height, QImage::Format pixel_format)
{
	suf = nullptr;
	sufbuf = nullptr;

	this->Create(width, height, pixel_format);
}
CSurface::CSurface(long width, long height, CPixelFormat::FormatId force_format)
{
	suf = nullptr;
	sufbuf = nullptr;

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
	QImage::Format format;
	pixel_format.ConvTo(format);

	return Create(width, height, format);
}

// Create surface
bool CSurface::Create(long width, long height, QImage::Format pixel_format)
{
	size_t size = static_cast<size_t>(width * height) * sizeof(uint32_t);
	sufbuf = new uint32_t[size];
	suf = new QImage(reinterpret_cast<uchar *>(sufbuf), width, height, pixel_format);
	return (suf != nullptr && !suf->isNull());
}

// Create surface
bool CSurface::Create(long width, long height, CPixelFormat::FormatId force_format)
{
	QImage::Format format;
	CPixelFormat tmp(force_format);
	tmp.ConvTo(format);

	return Create(width, height, format);
}

bool CSurface::Create(const VmRectWH &srcrect)
{
	return Create(srcrect.w, srcrect.h);
}

bool CSurface::Create(const VmRectWH &srcrect, const CPixelFormat &srcformat)
{
	return Create(srcrect.w, srcrect.h, srcformat);
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

// release surface
void CSurface::Release()
{
	delete [] sufbuf;
	sufbuf = nullptr;
	delete suf;
	suf = nullptr;
}

QImage *CSurface::Get()
{
	return suf;
}

scrntype *CSurface::GetBuffer()
{
	return reinterpret_cast<scrntype *>(suf->bits());
}

scrntype *CSurface::GetBuffer(int y)
{
	return reinterpret_cast<scrntype *>(suf->scanLine(y));
}

int CSurface::GetBufferSize()
{
#if QT_VERSION >= 0x050a00
	return static_cast<int>(suf->sizeInBytes());
#else
	return suf->byteCount();
#endif
}

bool CSurface::IsEnable()
{
	return (suf != nullptr && !suf->isNull());
}

int CSurface::Width()
{
	return suf->width();
}

int CSurface::Height()
{
	return suf->height();
}

int CSurface::BitsPerPixel()
{
	return suf->pixelFormat().bitsPerPixel();
}

int CSurface::BytesPerPixel()
{
	return suf->pixelFormat().bitsPerPixel() / 8;
}

int CSurface::BytesPerLine()
{
	return suf->bytesPerLine();
}

QImage::Format CSurface::GetNativePixelFormat()
{
	return suf->format();
}

CPixelFormat CSurface::GetPixelFormat() const
{
	CPixelFormat tmp;
	tmp.ConvFrom(suf->format());
	return tmp;
}

bool CSurface::Lock()
{
	return (suf != nullptr && !suf->isNull());
}

void CSurface::Unlock()
{
}

bool CSurface::Blit(CSurface &dst)
{
	DrawImage(*suf, nullptr, *dst.suf, nullptr);
	return true;
}

bool CSurface::Blit(CSurface &dst, const VmRectWH &dst_re)
{
	DrawImage(*suf, nullptr, *dst.suf, &dst_re);
	return true;
}

bool CSurface::Blit(const VmRectWH &src_re, CSurface &dst, const VmRectWH &dst_re)
{
	DrawImage(*suf, &src_re, *dst.suf, &dst_re);
	return true;
}

bool CSurface::Blit(const VmRectWH &src_re, QImage &dst, const VmRectWH &dst_re)
{
	DrawImage(*suf, &src_re, dst, &dst_re);
	return true;
}

bool CSurface::StretchBlit(const VmRectWH &src_re, CSurface &dst, const VmRectWH &dst_re)
{
	DrawScaledImage(*suf, &src_re, *dst.suf, &dst_re);
	return true;
}

bool CSurface::BlitFlipped(const VmRectWH &src_re, CSurface &dst, const VmRectWH &dst_re)
{
	DrawImageFlipped(*suf, GetPixelFormat(), &src_re, *dst.suf, dst.GetPixelFormat(), &dst_re);
	return true;
}

bool CSurface::StretchBlitFlipped(const VmRectWH &src_re, CSurface &dst, const VmRectWH &dst_re)
{
	QImage tmp(dst.suf->width(), dst.suf->height(), dst.suf->format());
	DrawScaledImage(*suf, &src_re, tmp, &dst_re);
	DrawImageFlipped(tmp, GetPixelFormat(), &dst_re, *dst.suf, dst.GetPixelFormat(), &dst_re);
	return true;
}

/// copy surface
/// @note each images should be the same format.
void CSurface::DrawImage(QImage &src, const VmRectWH *reSrc, QImage &dst, const VmRectWH *reDst)
{
#ifndef USE_QPAINTER
	const scrntype *pSrc;
	scrntype *pDst;

	VmRect rs;
	VmRect ds;
	if (reSrc) {
		rs.left = reSrc->x;
		rs.top = reSrc->y;
		rs.right = reSrc->x + reSrc->w;
		rs.bottom = reSrc->y + reSrc->h;
	} else {
		rs.left = 0;
		rs.top = 0;
		rs.right = src.width();
		rs.bottom = src.height();
	}
	if (reDst) {
		ds.left = reDst->x;
		ds.top = reDst->y;
		ds.right = reDst->x + reDst->w;
		ds.bottom = reDst->y + reDst->h;
	} else {
		ds.left = 0;
		ds.top = 0;
		ds.right = dst.width();
		ds.bottom = dst.height();
	}

	for(int sy=rs.top, dy=ds.top; sy < rs.bottom && dy < ds.bottom; sy++, dy++) {
		pSrc = reinterpret_cast<const scrntype *>(src.scanLine(sy));
		pSrc += rs.left;
		pDst = reinterpret_cast<scrntype *>(dst.scanLine(dy));
		pDst += ds.left;
//		memcpy(pDst, pSrc, (ds.right - ds.left) * sizeof(scrntype));
		for(int sx=rs.left, dx=ds.left; sx < rs.right && dx < ds.right; sx++, dx++) {
			*pDst++ = *pSrc++;
		}
	}
#else
	QRect rs;
	QRect ds;
	if (reSrc) {
		rs.setRect(reSrc->x, reSrc->y, reSrc->w, reSrc->h);
	} else {
		rs.setRect(0, 0, src.width(), src.height());
	}
	if (reDst) {
		ds.setRect(reDst->x, reDst->y, reDst->w, reDst->h);
	} else {
		ds.setRect(0, 0, dst.width(), dst.height());
	}
	QPainter qp(&dst);
	qp.drawImage(ds, src, rs);
#endif
}

void CSurface::DrawImageFlipped(QImage &src, const CPixelFormat &fmtSrc, const VmRectWH *reSrc, QImage &dst, const CPixelFormat &fmtDst, const VmRectWH *reDst)
{
	const scrntype *pSrc;
	scrntype *pDst;
	uint32_t r, g, b, a;

	VmRect rs;
	VmRect ds;
	if (reSrc) {
		rs.left = reSrc->x;
		rs.top = reSrc->y;
		rs.right = reSrc->x + reSrc->w;
		rs.bottom = reSrc->y + reSrc->h;
	} else {
		rs.left = 0;
		rs.top = 0;
		rs.right = src.width();
		rs.bottom = src.height();
	}
	if (reDst) {
		ds.left = reDst->x;
		ds.top = reDst->y;
		ds.right = reDst->x + reDst->w;
		ds.bottom = reDst->y + reDst->h;
	} else {
		ds.left = 0;
		ds.top = 0;
		ds.right = dst.width();
		ds.bottom = dst.height();
	}

	for(int sy=rs.bottom - 1, dy=ds.top; sy >= rs.top && dy < ds.bottom; sy--, dy++) {
		pSrc = reinterpret_cast<const scrntype *>(src.scanLine(sy));
		pSrc += rs.left;
		pDst = reinterpret_cast<scrntype *>(dst.scanLine(dy));
		pDst += ds.left;
//		memcpy(pDst, pSrc, (ds.right - ds.left) * sizeof(scrntype));
		for(int sx=rs.left, dx=ds.left; sx < rs.right && dx < ds.right; sx++, dx++) {
			r = ((*pSrc & fmtSrc.Rmask) >> fmtSrc.Rshift);
			g = ((*pSrc & fmtSrc.Gmask) >> fmtSrc.Gshift);
			b = ((*pSrc & fmtSrc.Bmask) >> fmtSrc.Bshift);
			a = ((*pSrc & fmtSrc.Amask) >> fmtSrc.Ashift);
			*pDst = (r << fmtDst.Rshift) | (g << fmtDst.Gshift) | (b << fmtDst.Bshift) | (a << fmtDst.Ashift);
			pSrc++;
			pDst++;
		}
	}
}

void CSurface::DrawScaledImage(QImage &src, const VmRectWH *reSrc, QImage &dst, const VmRectWH *reDst)
{
	QRect rs;
	QRect ds;
	if (reSrc) {
		rs.setRect(reSrc->x, reSrc->y, reSrc->w, reSrc->h);
	} else {
		rs.setRect(0, 0, src.width(), src.height());
	}
	if (reDst) {
		ds.setRect(reDst->x, reDst->y, reDst->w, reDst->h);
	} else {
		ds.setRect(0, 0, dst.width(), dst.height());
	}
	QPainter qp(&dst);
	qp.drawImage(ds, src, rs);
}
