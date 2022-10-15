/** @file wxw_csurface.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2019.03.02 -

	@brief [ csurface ]
*/

#ifndef WXW_CSURFACE_H
#define WXW_CSURFACE_H

#ifdef wxUSE_ANY
#undef wxUSE_ANY
#endif
#include "../../common.h"
#include "../../depend.h"
#include "../../cpixfmt.h"
#include <wx/rawbmp.h>

class wxMemoryDC;
class wxMutex;

/**
	@brief manage CSurface
*/
class CSurface
{
protected:
	wxBitmap *suf;
	wxMemoryDC *sufDc;
	wxAlphaPixelData *sufData;
	void *sufBuffer;
	CPixelFormat sufFmt;
#if defined(__WXGTK__) && !defined(__WXGTK3__)
	void *sufPixbuf;
	bool pixbuf_modified;
#endif
//	wxMutex *mux;

public:
	CSurface();
	CSurface(long width, long height);
	CSurface(long width, long height, const CPixelFormat &pixel_format);
	CSurface(long width, long height, CPixelFormat::FormatId force_format);
	virtual ~CSurface();

	bool Create(long width, long height);
	bool Create(long width, long height, const CPixelFormat &pixel_format);
	bool Create(long width, long height, CPixelFormat::FormatId force_format);
	bool Create(const VmRectWH &srcrect);
	bool Create(const VmRectWH &srcrect, const CPixelFormat &pixel_format);
	bool Create(const VmRectWH &srcrect, CPixelFormat::FormatId force_format);
	bool Create(const VmRectWH &srcrect, CSurface &srcsurface, CPixelFormat::FormatId force_format = CPixelFormat::NONE);
	bool Create(CSurface &srcsurface, const CPixelFormat &pixel_format);
	bool Create(const wxImage &image, const CPixelFormat &pixel_format);
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

	wxBitmap *Get();
	wxMemoryDC *GetDC();
	void UngetDC();
	const CPixelFormat &GetPixelFormat() const;

	bool Lock();
	void Unlock();

	bool Blit(CSurface &dst);
	bool Blit(CSurface &dst, const VmRectWH &dst_re);
	bool Blit(const VmRectWH &src_re, CSurface &dst, const VmRectWH &dst_re);
	bool Blit(const VmRectWH &src_re, wxBitmap &dst, const VmRectWH &dst_re);
	bool Blit(const VmRectWH &src_re, wxImage &dst, const VmRectWH &dst_re);

	bool BlitWithoutAlpha(const VmRectWH &src_re, CSurface &dst, const VmRectWH &dst_re);
	bool Blit24(const VmRectWH &src_re, wxBitmap &dst, const VmRectWH &dst_re);

	bool StretchBlit(const VmRectWH &src_re, CSurface &dst, const VmRectWH &dst_re);
	bool StretchBlit(const VmRectWH &src_re, wxBitmap &dst, const VmRectWH &dst_re);
	bool StretchBlit(const VmRectWH &src_re, wxImage &dst, const VmRectWH &dst_re);

	bool BlitFlippedWithoutAlpha(const VmRectWH &src_re, CSurface &dst, const VmRectWH &dst_re);

#if 0
	static bool Blit(CSurface &src, CSurface &dst);
	static bool Blit(CSurface &src, VmRectWH &src_re, CSurface &dst, VmRectWH &dst_re);
	static bool Blit(CSurface &src, VmRectWH &src_re, wxBitmap &dst, VmRectWH &dst_re);
	static bool Blit(CSurface &src, VmRectWH &src_re, wxImage &dst, VmRectWH &dst_re);

	static bool StretchBlit(CSurface &src, VmRectWH &src_re, CSurface &dst, VmRectWH &dst_re);
	static bool StretchBlit(CSurface &src, VmRectWH &src_re, wxBitmap &dst, VmRectWH &dst_re);
	static bool StretchBlit(CSurface &src, VmRectWH &src_re, wxImage &dst, VmRectWH &dst_re);

#endif
	bool Flip();

	void FillBlack();

#if defined(__WXGTK__) && !defined(__WXGTK3__)
	void CopyPixbufToPixmap();
#endif
};

#endif /* WXW_CSURFACE_H */
