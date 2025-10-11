/** @file win_d2d.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2025.05.11 -

	@brief [ Direct2D ]
*/

#ifndef WIN_D2D_H
#define WIN_D2D_H

#include "../../vm/vm_defs.h"

#include <windows.h>
#include "../../common.h"

#ifdef USE_DIRECT2D

#include <d2d1.h>
// load dll dinamically
//#pragma comment(lib, "d2d1.lib")

#else  /* !USE_DIRECT2D */

#define ID2D1Factory void
#define ID2D1RenderTarget void
#define ID2D1DCRenderTarget void
#define ID2D1HwndRenderTarget void
#define ID2D1BitmapRenderTarget void
#define D2D1_SIZE_U RECT
#define ID2D1Bitmap void
enum D2D1_BITMAP_INTERPOLATION_MODE {
	D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR = 0,
	D2D1_BITMAP_INTERPOLATION_MODE_LINEAR = 1
};

#endif /* USE_DIRECT2D */

class CSurface;
class CD2DRender;
class CD2DDCRender;
class CD2DHwndRender;
class CD2DBitmapRender;
class CD2DSurface;

/**
	@brief The Factory 
*/
class CD2DFactory
{
protected:
	static int mRefCount;
	static ID2D1Factory *pFactory;

	static bool Load();
	static void Unload();

public:
	CD2DFactory();
	virtual ~CD2DFactory();

	HRESULT CreateFactory();
	void ReleaseFactory();
	ID2D1Factory *GetFactory();
};

extern CD2DFactory gD2DFactory;

/**
	@brief Render Target
*/
class CD2DRender
{
protected:
	ID2D1RenderTarget *pRender;
	RECT mRect;	// render area = window size
	D2D1_BITMAP_INTERPOLATION_MODE mInterpolation;

public:
	CD2DRender();
	virtual ~CD2DRender();
	ID2D1RenderTarget *GetRender();

	void GetDpi(FLOAT &dpiX, FLOAT &dpiY) const;
//	D2D1_SIZE_U GetPixelSize() const;

	void SetInterpolation(int mode);

	void FlipVertical();

	void DrawBitmap(CD2DSurface &srcSurface);
	void DrawBitmap(CD2DSurface &srcSurface, VmRectWH &dstRect, VmRectWH &srcRect);
	void DrawBitmap(CD2DBitmapRender &srcRender);
	void DrawBitmap(CD2DBitmapRender &srcRender, VmRectWH &dstRect, VmRectWH &srcRect);
};

/**
	@brief DC Render Target
*/
class CD2DDCRender : public CD2DRender
{
protected:

public:
	CD2DDCRender();
	virtual ~CD2DDCRender();

	HRESULT CreateRender(CD2DFactory &D2DFactory, int w, int h);
	void ReleaseRender();
	ID2D1DCRenderTarget *GetDCRender();

	void BindDC(HDC hdc);
	void BeginDraw(HDC hdc);
	HRESULT EndDraw();
};

/**
	@brief Hwnd Render Target
*/
class CD2DHwndRender : public CD2DRender
{
protected:

public:
	CD2DHwndRender();
	virtual ~CD2DHwndRender();

	HRESULT CreateRender(CD2DFactory &D2DFactory, HWND hWnd, int w, int h, int interval);
	void ReleaseRender();
	ID2D1HwndRenderTarget *GetHwndRender();

	void BeginDraw();
	HRESULT EndDraw();
};

/**
	@brief Bitmap Render Target
*/
class CD2DBitmapRender : public CD2DRender
{
protected:

public:
	CD2DBitmapRender();
	virtual ~CD2DBitmapRender();

	HRESULT CreateRender(CD2DRender &D2DRender, int w, int h);
	void ReleaseRender();
	ID2D1BitmapRenderTarget *GetBitmapRender();

	void BeginDraw();
	HRESULT EndDraw();
};

/**
	@brief one surface
*/
class CD2DSurface
{
protected:
	ID2D1Bitmap *pSurface;

public:
	CD2DSurface();
	virtual ~CD2DSurface();

	HRESULT CreateSurface(CD2DRender &D2DRender, int w, int h);
	void ReleaseSurface();

	void Copy(CSurface &suf);
	void Copy(CSurface &suf, int y);
	void Copy(CD2DRender *render, VmRectWH &srcRect);

	ID2D1Bitmap *GetSurface();
	int GetSurfaceWidth() const;
	int GetSurfaceHeight() const;
};

#endif /* WIN_D2D_H */
