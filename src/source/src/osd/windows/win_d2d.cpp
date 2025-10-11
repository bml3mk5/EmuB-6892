/** @file win_d2d.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2025.05.11 -

	@brief [ Direct2D ]
*/

#include "win_d2d.h"
#include "win_csurface.h"
#include <tchar.h>

//#pragma comment(lib, "D2D1.lib")

// ===========================================================================

CD2DFactory gD2DFactory;

// ---------------------------------------------------------------------------

int CD2DFactory::mRefCount = 0;
ID2D1Factory *CD2DFactory::pFactory = NULL;

#ifdef USE_DIRECT2D
static int mDllRefCount = 0;
static HMODULE hD2D1Dll = NULL;

static HRESULT (WINAPI *f_D2D1CreateFactory)(
	D2D1_FACTORY_TYPE factoryType,
	REFIID riid,
	CONST D2D1_FACTORY_OPTIONS *pFactoryOptions,
	void **ppIFactory
) = NULL;

#endif /* USE_DIRECT2D */

// ---------------------------------------------------------------------------

CD2DFactory::CD2DFactory()
{
	Load();
}

CD2DFactory::~CD2DFactory()
{
	ReleaseFactory();
	Unload();
}

/// @brief Load Direct2D library
/// @return true : loaded successfly
bool CD2DFactory::Load()
{
#ifdef USE_DIRECT2D
	mDllRefCount++;

	if (hD2D1Dll) return true;

	try {
		// load D2D1.dll
		hD2D1Dll = ::LoadLibrary(_T("D2D1"));
		if (!hD2D1Dll) return false;

		f_D2D1CreateFactory = (HRESULT(WINAPI *)(D2D1_FACTORY_TYPE, REFIID, CONST D2D1_FACTORY_OPTIONS *, void **))
			::GetProcAddress(hD2D1Dll, "D2D1CreateFactory");
	}
	catch(...) {
		f_D2D1CreateFactory = NULL;
		hD2D1Dll = NULL;
	}
#endif /* USE_DIRECT2D */
	return true;
}

void CD2DFactory::Unload()
{
#ifdef USE_DIRECT2D
	if (!hD2D1Dll) return;

	mDllRefCount--;
	if (mDllRefCount > 0) return;

	::FreeLibrary(hD2D1Dll);

	f_D2D1CreateFactory = NULL;
	hD2D1Dll = NULL;
#endif /* USE_DIRECT2D */
}

/// @brief Create Factory
/// @return HRESULT
HRESULT CD2DFactory::CreateFactory()
{
	HRESULT hre = S_FALSE;

#ifdef USE_DIRECT2D
	if (!f_D2D1CreateFactory) return hre;

	mRefCount++;

	if (pFactory) return S_OK;

	hre = f_D2D1CreateFactory(
		D2D1_FACTORY_TYPE_MULTI_THREADED,
		__uuidof(ID2D1Factory),
		NULL,
		(void **)&pFactory);
	if (hre != S_OK) {
		pFactory = NULL;
	}
#endif /* USE_DIRECT2D */

	return hre;
}

void CD2DFactory::ReleaseFactory()
{
#ifdef USE_DIRECT2D
	if (!pFactory) return;

	mRefCount--;
	if (mRefCount > 0) return;

	pFactory->Release();
	pFactory = NULL;
#endif /* USE_DIRECT2D */
}

ID2D1Factory *CD2DFactory::GetFactory()
{
	return pFactory;
}

// ===========================================================================

CD2DRender::CD2DRender()
{
	pRender = NULL;
	mRect.left = mRect.top = mRect.right = mRect.bottom = 0L;
	mInterpolation = D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR;
}

CD2DRender::~CD2DRender()
{
}

ID2D1RenderTarget *CD2DRender::GetRender()
{
	return pRender;
}

void CD2DRender::SetInterpolation(int mode)
{
	if (mode == 0) {
		mInterpolation = D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR;
	} else {
		mInterpolation = D2D1_BITMAP_INTERPOLATION_MODE_LINEAR;
	}
}

void CD2DRender::GetDpi(FLOAT &dpiX, FLOAT &dpiY) const
{
#ifdef USE_DIRECT2D
	if (pRender) {
		pRender->GetDpi(&dpiX, &dpiY);
	}
#endif /* USE_DIRECT2D */
}

#if 0
D2D1_SIZE_U CD2DRender::GetPixelSize() const
{
#ifdef USE_DIRECT2D
	if (pRender) {
		return pRender->GetPixelSize();
	}
#endif /* USE_DIRECT2D */
	return mRect;
}
#endif

/// @brief Set matrix for flipping vertical
void CD2DRender::FlipVertical()
{
#ifdef USE_DIRECT2D
	if (!pRender) return;

	D2D1::Matrix3x2F matrix = D2D1::Matrix3x2F::Identity();
	matrix._22 = -1.0F;
	matrix._32 = (FLOAT)(mRect.bottom - mRect.top);
	pRender->SetTransform(matrix);
#endif /* USE_DIRECT2D */
}

/// @brief Draw bitmap from bitmap surface
/// @param[in] srcSurface : Bitmap Surface
void CD2DRender::DrawBitmap(CD2DSurface &srcSurface)
{
#ifdef USE_DIRECT2D
	if (!pRender) return;

	pRender->DrawBitmap(srcSurface.GetSurface(), 0, 1.0F, mInterpolation, 0);
#endif /* USE_DIRECT2D */
}

/// @brief Draw bitmap from bitmap surface
/// @param[in] srcSurface : Bitmap Surface
/// @param[in] dstRect
/// @param[in] srcRect
void CD2DRender::DrawBitmap(CD2DSurface &srcSurface, VmRectWH &dstRect, VmRectWH &srcRect)
{
#ifdef USE_DIRECT2D
	if (!pRender) return;

	D2D1_RECT_F srcRectF = D2D1::RectF((FLOAT)srcRect.x, (FLOAT)srcRect.y, (FLOAT)srcRect.x + srcRect.w, (FLOAT)srcRect.y + srcRect.h);
	D2D1_RECT_F dstRectF = D2D1::RectF((FLOAT)dstRect.x, (FLOAT)dstRect.y, (FLOAT)dstRect.x + dstRect.w, (FLOAT)dstRect.y + dstRect.h);
	pRender->DrawBitmap(srcSurface.GetSurface(), dstRectF, 1.0F, mInterpolation, srcRectF);
#endif /* USE_DIRECT2D */
}

/// @brief Draw bitmap from bitmap render target
/// @param[in] srcRender : Render Target
void CD2DRender::DrawBitmap(CD2DBitmapRender &srcRender)
{
#ifdef USE_DIRECT2D
	if (!pRender) return;

	ID2D1Bitmap *bitmap;
	if (srcRender.GetBitmapRender()->GetBitmap(&bitmap) == S_OK) {
		pRender->DrawBitmap(bitmap, 0, 1.0F, mInterpolation, 0);
	}
#endif /* USE_DIRECT2D */
}

/// @brief Draw bitmap from bitmap render target
/// @param[in] srcRender : Render Target
/// @param[in] dstRect
/// @param[in] srcRect
void CD2DRender::DrawBitmap(CD2DBitmapRender &srcRender, VmRectWH &dstRect, VmRectWH &srcRect)
{
#ifdef USE_DIRECT2D
	if (!pRender) return;

	D2D1_RECT_F srcRectF = D2D1::RectF((FLOAT)srcRect.x, (FLOAT)srcRect.y, (FLOAT)srcRect.x + srcRect.w, (FLOAT)srcRect.y + srcRect.h);
	D2D1_RECT_F dstRectF = D2D1::RectF((FLOAT)dstRect.x, (FLOAT)dstRect.y, (FLOAT)dstRect.x + dstRect.w, (FLOAT)dstRect.y + dstRect.h);
	ID2D1Bitmap *bitmap;
	if (srcRender.GetBitmapRender()->GetBitmap(&bitmap) == S_OK) {
		pRender->DrawBitmap(bitmap, dstRectF, 1.0F, mInterpolation, srcRectF);
	}
#endif /* USE_DIRECT2D */
}

// ===========================================================================

CD2DDCRender::CD2DDCRender() : CD2DRender()
{
}

CD2DDCRender::~CD2DDCRender()
{
	ReleaseRender();
}

/// @brief Create DC Render Target
/// @param[in] D2DFactory : Render Target
/// @param[in] w : Width
/// @param[in] h : Height
/// @return HRESULT
HRESULT CD2DDCRender::CreateRender(CD2DFactory &D2DFactory, int w, int h)
{
	HRESULT hre = S_FALSE;

#ifdef USE_DIRECT2D
	ID2D1Factory *pFactory = D2DFactory.GetFactory();
	if (!pFactory) {
		return hre;
	}
	mRect.left = mRect.top = mRect.right = mRect.bottom = 0L;

	D2D1_SIZE_U size = D2D1::SizeU(w, h);

	D2D1_RENDER_TARGET_PROPERTIES rprop;

	// use Device Context
	FLOAT dpiX, dpiY;
//	pFactory->GetDesktopDpi(&dpiX, &dpiY);
//  Always Use 96dpi
	dpiX = (FLOAT)USER_DEFAULT_SCREEN_DPI;
	dpiY = (FLOAT)USER_DEFAULT_SCREEN_DPI;
	rprop = D2D1::RenderTargetProperties(
		D2D1_RENDER_TARGET_TYPE_DEFAULT,
		D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE),
		dpiX, dpiY
	);
	hre = pFactory->CreateDCRenderTarget(&rprop, (ID2D1DCRenderTarget **)&pRender);
	if (hre != S_OK) {
		pRender = NULL;
	}

	mRect.right = (LONG)w;
	mRect.bottom = (LONG)h;

#endif /* USE_DIRECT2D */
	return hre;
}

void CD2DDCRender::ReleaseRender()
{
#ifdef USE_DIRECT2D
	if (pRender) {
		pRender->Release();
		pRender = NULL;
		mRect.left = mRect.top = mRect.right = mRect.bottom = 0L;
	}
#endif /* USE_DIRECT2D */
}

ID2D1DCRenderTarget *CD2DDCRender::GetDCRender()
{
	return (ID2D1DCRenderTarget *)pRender;
}

void CD2DDCRender::BindDC(HDC hdc)
{
#ifdef USE_DIRECT2D
	if (pRender) {
		((ID2D1DCRenderTarget *)pRender)->BindDC(hdc, &mRect);
	}
#endif /* USE_DIRECT2D */
}

/// @brief Begin drawing
/// @param[in] hdc : Handle of DC
void CD2DDCRender::BeginDraw(HDC hdc)
{
#ifdef USE_DIRECT2D
	if (pRender) {
		((ID2D1DCRenderTarget *)pRender)->BindDC(hdc, &mRect);
		pRender->BeginDraw();
	}
#endif /* USE_DIRECT2D */
}

/// @brief End drawing
/// @return HRESULT
HRESULT CD2DDCRender::EndDraw()
{
#ifdef USE_DIRECT2D
	if (pRender) {
		return pRender->EndDraw();
	}
#endif /* USE_DIRECT2D */
	return S_OK;
}

// ===========================================================================

CD2DHwndRender::CD2DHwndRender() : CD2DRender()
{
}

CD2DHwndRender::~CD2DHwndRender()
{
	ReleaseRender();
}

/// @brief Create DC Render Target
/// @param[in] D2DFactory : Render Target
/// @param[in] hWnd : Window Handle
/// @param[in] w : Width
/// @param[in] h : Height
/// @param[in] interval : 0: Async VSync  1: Wait VSync
/// @return HRESULT
HRESULT CD2DHwndRender::CreateRender(CD2DFactory &D2DFactory, HWND hWnd, int w, int h, int interval)
{
	HRESULT hre = S_FALSE;

#ifdef USE_DIRECT2D
	ID2D1Factory *pFactory = D2DFactory.GetFactory();
	if (!pFactory) {
		return hre;
	}
	mRect.left = mRect.top = mRect.right = mRect.bottom = 0L;

	D2D1_SIZE_U size = D2D1::SizeU(w, h);

	D2D1_RENDER_TARGET_PROPERTIES rprop;
	D2D1_HWND_RENDER_TARGET_PROPERTIES hprop;

	// use Device Context
	FLOAT dpiX, dpiY;
//	pFactory->GetDesktopDpi(&dpiX, &dpiY);
	//  Always Use 96dpi
	dpiX = (FLOAT)USER_DEFAULT_SCREEN_DPI;
	dpiY = (FLOAT)USER_DEFAULT_SCREEN_DPI;
	rprop = D2D1::RenderTargetProperties(
		D2D1_RENDER_TARGET_TYPE_DEFAULT,
		D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE),
		dpiX, dpiY
	);
	D2D1_PRESENT_OPTIONS options;
	if (interval) {
		options = D2D1_PRESENT_OPTIONS_NONE; // Sync VSync
	} else {
		options = D2D1_PRESENT_OPTIONS_IMMEDIATELY; // Async VSync
	}
	hprop = D2D1::HwndRenderTargetProperties(
		hWnd,
		size,
		options
	);
	hre = pFactory->CreateHwndRenderTarget(rprop, hprop, (ID2D1HwndRenderTarget **)&pRender);
	if (hre != S_OK) {
		pRender = NULL;
	}

	mRect.right = (LONG)w;
	mRect.bottom = (LONG)h;

#endif /* USE_DIRECT2D */
	return hre;
}

void CD2DHwndRender::ReleaseRender()
{
#ifdef USE_DIRECT2D
	if (pRender) {
		pRender->Release();
		pRender = NULL;
		mRect.left = mRect.top = mRect.right = mRect.bottom = 0L;
	}
#endif /* USE_DIRECT2D */
}

ID2D1HwndRenderTarget *CD2DHwndRender::GetHwndRender()
{
	return (ID2D1HwndRenderTarget *)pRender;
}

/// @brief Begin drawing
void CD2DHwndRender::BeginDraw()
{
#ifdef USE_DIRECT2D
	if (pRender) {
		pRender->BeginDraw();
	}
#endif /* USE_DIRECT2D */
}

/// @brief End drawing
/// @return HRESULT
HRESULT CD2DHwndRender::EndDraw()
{
#ifdef USE_DIRECT2D
	if (pRender) {
		return pRender->EndDraw();
	}
#endif /* USE_DIRECT2D */
	return S_OK;
}

// ===========================================================================

CD2DBitmapRender::CD2DBitmapRender() : CD2DRender()
{
}

CD2DBitmapRender::~CD2DBitmapRender()
{
	ReleaseRender();
}

/// @brief Create Bitmap Render Target
/// @param[in] D2DRender : Render Target
/// @param[in] w : Width
/// @param[in] h : Height
/// @return HRESULT
HRESULT CD2DBitmapRender::CreateRender(CD2DRender &D2DRender, int w, int h)
{
	HRESULT hre = S_FALSE;

#ifdef USE_DIRECT2D
	ID2D1RenderTarget *pParentRender = D2DRender.GetRender();
	if (!pParentRender) {
		return hre;
	}
	mRect.left = mRect.top = mRect.right = mRect.bottom = 0L;

	D2D1_SIZE_U size = D2D1::SizeU(w, h);

	D2D1_COMPATIBLE_RENDER_TARGET_OPTIONS options = D2D1_COMPATIBLE_RENDER_TARGET_OPTIONS_NONE;

	hre = pParentRender->CreateCompatibleRenderTarget(NULL, &size, NULL, options, (ID2D1BitmapRenderTarget **)&pRender);

	if (hre != S_OK) {
		pRender = NULL;
	}

	mRect.right = (LONG)w;
	mRect.bottom = (LONG)h;

#endif /* USE_DIRECT2D */
	return hre;
}

void CD2DBitmapRender::ReleaseRender()
{
#ifdef USE_DIRECT2D
	if (pRender) {
		pRender->Release();
		pRender = NULL;
		mRect.left = mRect.top = mRect.right = mRect.bottom = 0L;
	}
#endif /* USE_DIRECT2D */
}

ID2D1BitmapRenderTarget *CD2DBitmapRender::GetBitmapRender()
{
	return (ID2D1BitmapRenderTarget *)pRender;
}

void CD2DBitmapRender::BeginDraw()
{
#ifdef USE_DIRECT2D
	if (pRender) {
		pRender->BeginDraw();
	}
#endif /* USE_DIRECT2D */
}

HRESULT CD2DBitmapRender::EndDraw()
{
#ifdef USE_DIRECT2D
	if (pRender) {
		return pRender->EndDraw();
	}
#endif /* USE_DIRECT2D */
	return S_OK;
}

// ===========================================================================

CD2DSurface::CD2DSurface()
{
	pSurface = NULL;
}
CD2DSurface::~CD2DSurface()
{
	ReleaseSurface();
}

/// @brief Create surface
/// @param[in] D2DRender : Render Target
/// @param[in] w : Width
/// @param[in] h : Height
/// @return HRESULT
HRESULT CD2DSurface::CreateSurface(CD2DRender &D2DRender, int w, int h)
{
	HRESULT hre = S_FALSE;

#ifdef USE_DIRECT2D
	ID2D1RenderTarget *pRender = D2DRender.GetRender();
	if (!pRender) {
		return hre;
	}

	D2D1_SIZE_U size = D2D1::SizeU(w, h);
	FLOAT dpiX, dpiY;
	D2D1_BITMAP_PROPERTIES bprop;

	pRender->GetDpi(&dpiX, &dpiY);
	bprop = D2D1::BitmapProperties(
		D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE),
		dpiX, dpiY
	);
	hre = pRender->CreateBitmap(size, bprop, &pSurface);
#endif /* USE_DIRECT2D */
	return hre;
}

void CD2DSurface::ReleaseSurface()
{
#ifdef USE_DIRECT2D
	if (pSurface) {
		pSurface->Release();
		pSurface = NULL;
	}
#endif /* USE_DIRECT2D */
}

/// @brief Copy surface buffer data to D2D1Bitmap instance
/// @param[in] suf : Surface (DIB based)
/// 
/// @attention
///  The DIB based Surface is usually flipped vertically (the start position of the buffer places the left of the bottom line).
///  So, you set a vertical flip matrix to draw this to a render normally.
void CD2DSurface::Copy(CSurface &suf)
{
#ifdef USE_DIRECT2D
	if (!pSurface) return;

	pSurface->CopyFromMemory(0, suf.GetBuffer(), suf.GetPitch());
#endif /* USE_DIRECT2D */
}

/// @brief Copy surface buffer data to D2D1Bitmap instance
/// @param[in] suf : Surface (DIB based)
/// @param[in] y
/// 
/// @attention
///  The DIB based Surface is usually flipped vertically (the start position of the buffer places the left of the bottom line).
///  So, you set a vertical flip matrix to draw this to a render normally.
void CD2DSurface::Copy(CSurface &suf, int y)
{
#ifdef USE_DIRECT2D
	if (!pSurface) return;

	pSurface->CopyFromMemory(0, suf.GetBuffer(y), suf.GetPitch());
#endif /* USE_DIRECT2D */
}

/// @brief Copy RenderTarget to D2D1Bitmap instance
/// @param[in] render : Render Target
/// @param[in] srcRect : Area on Render Target
/// 
/// @attention
///  The DIB based Surface is usually flipped vertically (the start position of the buffer places the left of the bottom line).
///  So, you set a vertical flip matrix to draw this to a render normally.
void CD2DSurface::Copy(CD2DRender *render, VmRectWH &srcRect)
{
#ifdef USE_DIRECT2D
	if (!pSurface || !render) return;

	D2D1_RECT_U srcRectU = D2D1::RectU(srcRect.x, srcRect.y, srcRect.x + srcRect.w, srcRect.y + srcRect.h);
	D2D1_POINT_2U dstPointU = D2D1::Point2U();
	pSurface->CopyFromRenderTarget(&dstPointU, render->GetRender(), &srcRectU);
#endif /* USE_DIRECT2D */
}

ID2D1Bitmap *CD2DSurface::GetSurface()
{
	return pSurface;
}

int CD2DSurface::GetSurfaceWidth() const
{
#ifdef USE_DIRECT2D
	if (pSurface) {
		D2D1_SIZE_U size = pSurface->GetPixelSize();
		return (int)size.width;
	}
#endif /* USE_DIRECT2D */
	return 0;
}

int CD2DSurface::GetSurfaceHeight() const
{
#ifdef USE_DIRECT2D
	if (pSurface) {
		D2D1_SIZE_U size = pSurface->GetPixelSize();
		return (int)size.height;
	}
#endif /* USE_DIRECT2D */
	return 0;
}

#ifdef USE_DIRECT2D
#endif /* USE_DIRECT2D */
