/** @file qt_csurface.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2017.01.03 -

	@brief [ csurface ]
*/

#ifndef QT_CSURFACE_H
#define QT_CSURFACE_H

#include "../../common.h"
#ifdef Bool
#undef Bool
#endif
#ifdef Status
#undef Status
#endif
#ifdef CursorShape
#undef CursorShape
#endif
#include <QImage>
#include "../../cpixfmt.h"

/**
	@brief manage CSurface
*/
class CSurface
{
protected:
    QImage *suf;
    uint32_t *sufbuf;

public:
	CSurface();
	CSurface(long width, long height);
	CSurface(long width, long height, const CPixelFormat &pixel_format);
	CSurface(long width, long height, QImage::Format pixel_format);
	CSurface(long width, long height, CPixelFormat::FormatId force_format);
	virtual ~CSurface();

	bool Create(long width, long height);
	bool Create(long width, long height, const CPixelFormat &pixel_format);
	bool Create(long width, long height, QImage::Format pixel_format);
	bool Create(long width, long height, CPixelFormat::FormatId force_format);
	bool Create(const VmRectWH &srcrect);
	bool Create(const VmRectWH &srcrect, const CPixelFormat &pixel_format);
	bool Create(const VmRectWH &srcrect, CPixelFormat::FormatId force_format);
	bool Create(const VmRectWH &srcrect, CSurface &srcsurface, CPixelFormat::FormatId force_format = CPixelFormat::NONE);
	bool Create(CSurface &srcsurface, const CPixelFormat &pixel_format);

	void Release();

	scrntype *GetBuffer();
	scrntype *GetBuffer(int y);
	void UngetBuffer();
	int GetBufferSize();

	bool IsEnable();

	int Width();
	int Height();

	int BitsPerPixel();
	int BytesPerPixel();
	int BytesPerLine();

	QImage *Get();
	QImage::Format GetNativePixelFormat();
	CPixelFormat GetPixelFormat() const;

	bool Lock();
	void Unlock();

	bool Blit(CSurface &dst);
	bool Blit(CSurface &dst, const VmRectWH &dst_re);
	bool Blit(const VmRectWH &src_re, CSurface &dst, const VmRectWH &dst_re);
	bool Blit(const VmRectWH &src_re, QImage &dst, const VmRectWH &dst_re);

	bool StretchBlit(const VmRectWH &src_re, CSurface &dst, const VmRectWH &dst_re);

	bool BlitFlipped(const VmRectWH &src_re, CSurface &dst, const VmRectWH &dst_re);

	bool StretchBlitFlipped(const VmRectWH &src_re, CSurface &dst, const VmRectWH &dst_re);

	static void DrawImage(QImage &src, const VmRectWH *reSrc, QImage &dst, const VmRectWH *reDst);
	static void DrawImageFlipped(QImage &src, const CPixelFormat &fmtSrc, const VmRectWH *reSrc, QImage &dst, const CPixelFormat &fmtDst, const VmRectWH *reDst);

	static void DrawScaledImage(QImage &src, const VmRectWH *reSrc, QImage &dst, const VmRectWH *reDst);
};

#endif /* QT_CSURFACE_H */
