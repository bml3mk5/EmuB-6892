/** @file win_d3d.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2022.02.22 -

	@brief [ Direct3D ]
*/

#include "win_d3d.h"

#include "win_csurface.h"
#include "../../config.h"

#ifdef USE_DIRECT3D

#define VERTEX_IN(st, vx, vy, vz, vu, vv) { \
	(st).x = (FLOAT)(vx); \
	(st).y = (FLOAT)(vy); \
	(st).z = (FLOAT)(vz); \
	(st).rhw = (FLOAT)1.0; \
	(st).u = (FLOAT)(vu); \
	(st).v = (FLOAT)(vv); \
}

#define VERTEX_XYIN(st, vx, vy) { \
	(st).x = (FLOAT)(vx); \
	(st).y = (FLOAT)(vy); \
}

#define VERTEX_UVIN(st, vu, vv) { \
	(st).u = (FLOAT)(vu); \
	(st).v = (FLOAT)(vv); \
}

#define VERTEX_LIMIT(vl, vt, vr, vb, ll, tt, rr, bb) \
	float vl = (float)(ll) - (float)0.5; \
	float vt = (float)(tt) - (float)0.5; \
	float vr = (float)(rr) - (float)0.5; \
	float vb = (float)(bb) - (float)0.5;

#endif /* USE_DIRECT3D */

// ===========================================================================

CD3DDevice::CD3DDevice()
{
	pD3D = NULL;
	pDevice = NULL;

#ifdef USE_DIRECT3D
	::ZeroMemory(&mD3Dpp, sizeof(D3DPRESENT_PARAMETERS));
#endif /* USE_DIRECT3D */
}

CD3DDevice::~CD3DDevice()
{
	ReleaseDevice();
	Unload();
}

bool CD3DDevice::Load()
{
#ifdef USE_DIRECT3D
	// D3D
	pD3D = Direct3DCreate9(D3D_SDK_VERSION);
	if (!pD3D) {
		return false;
	}

	pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT , &mD3Ddm);

	mD3Dpp.BackBufferWidth = 0;
	mD3Dpp.BackBufferHeight = 0;
	mD3Dpp.BackBufferFormat = mD3Ddm.Format;
	mD3Dpp.BackBufferCount = 1;
	mD3Dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
	mD3Dpp.MultiSampleQuality = 0;
	mD3Dpp.EnableAutoDepthStencil = FALSE;
	mD3Dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	mD3Dpp.Windowed = TRUE;

#endif /* USE_DIRECT3D */
	return true;
}

void CD3DDevice::Unload()
{
#ifdef USE_DIRECT3D
	if (pD3D) {
		pD3D->Release();
		pD3D = NULL;
	}
#endif /* USE_DIRECT3D */
}

HRESULT CD3DDevice::CreateDevice(HWND hWnd)
{
	HRESULT hre = S_FALSE;

#ifdef USE_DIRECT3D
	hre = pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING ,&mD3Dpp, &pDevice);
	if (hre != D3D_OK) {
		hre = pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING ,&mD3Dpp, &pDevice);
	}
	if (hre != D3D_OK) {
		pDevice = NULL;
		return hre;
	}

	ClearDevice();

#endif /* USE_DIRECT3D */
	return hre;
}

void CD3DDevice::ReleaseDevice()
{
#ifdef USE_DIRECT3D
	if (pDevice) {
		pDevice->Release();
		pDevice = NULL;
	}
#endif /* USE_DIRECT3D */
}

HRESULT CD3DDevice::ResetDevice()
{
#ifdef USE_DIRECT3D
	if (!pDevice) return D3D_OK;

	return pDevice->Reset(&mD3Dpp);
#else
	return S_OK;
#endif /* USE_DIRECT3D */
}

PDIRECT3DDEVICE9 CD3DDevice::GetDevice()
{
	return pDevice;
}

void CD3DDevice::ClearDevice()
{
#ifdef USE_DIRECT3D
	if (!pDevice) return;

	pDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0,0,0), 0.0, 0);
#endif /* USE_DIRECT3D */
}

HRESULT CD3DDevice::TestDevice()
{
#ifdef USE_DIRECT3D
	if (!pDevice) return D3D_OK;

	return pDevice->TestCooperativeLevel();
#else
	return S_OK;
#endif /* USE_DIRECT3D */
}

HRESULT CD3DDevice::BeginScene(int filter_type)
{
#ifdef USE_DIRECT3D
	if (!pDevice) return S_FALSE;

//	pDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0x10, 0x10, 0x10), 0.0f, 0);

	pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
//	pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
//	pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	D3DTEXTUREFILTERTYPE filter = GetFilterType(filter_type);
	pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, filter);
	pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, filter);

	return pDevice->BeginScene();
#else
	return S_FALSE;
#endif /* USE_DIRECT3D */
}

void CD3DDevice::EndScene()
{
#ifdef USE_DIRECT3D
	if (!pDevice) return;

	pDevice->EndScene();
#endif /* USE_DIRECT3D */
}

HRESULT CD3DDevice::Present()
{
#ifdef USE_DIRECT3D
	if (!pDevice) return S_FALSE;

	return pDevice->Present(NULL,NULL,NULL,NULL);
#else
	return S_FALSE;
#endif /* USE_DIRECT3D */
}

void CD3DDevice::SetViewport(int w, int h)
{
#ifdef USE_DIRECT3D
	if (!pDevice) return;

	D3DVIEWPORT9 vp;

	vp.X = 0;
	vp.Y = 0;
	vp.Width = w;
	vp.Height = h;
	vp.MinZ = 0.0;
	vp.MaxZ = 1.0;

	// view port
	pDevice->SetViewport(&vp);
#endif /* USE_DIRECT3D */
}

HRESULT CD3DDevice::SetFVF()
{
#ifdef USE_DIRECT3D
	if (!pDevice) return S_FALSE;

	DWORD fvf = D3DFVF_XYZRHW | D3DFVF_TEX1;
	// notice x,y,z,rhw,u,v to screen
	return pDevice->SetFVF(fvf);
#else
	return S_FALSE;
#endif /* USE_DIRECT3D */
}

bool CD3DDevice::IsEnableScanlineOrInterval() const
{
#ifdef USE_DIRECT3D
	if (!pDevice) return false;

	D3DCAPS9 d3dcaps;
	pDevice->GetDeviceCaps(&d3dcaps);
	return ((d3dcaps.Caps & D3DCAPS_READ_SCANLINE) != 0 && (d3dcaps.PresentationIntervals & D3DPRESENT_INTERVAL_ONE) != 0);
#else
	return false;
#endif /* USE_DIRECT3D */
}

void CD3DDevice::SetPresentationInterval(int val)
{
#ifdef USE_DIRECT3D
	if (val == DRAWING_METHOD_DIRECT3D_S) {
		mD3Dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	} else {
		mD3Dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	}
#endif /* USE_DIRECT3D */
}

void CD3DDevice::SetPresentParametersSize(int w, int h)
{
#ifdef USE_DIRECT3D
	mD3Dpp.BackBufferWidth = (UINT)w;
	mD3Dpp.BackBufferHeight = (UINT)h;
	mD3Dpp.Windowed = TRUE;
#endif /* USE_DIRECT3D */
}

bool CD3DDevice::GetBackBufferSize(int &w, int &h)
{
#ifdef USE_DIRECT3D
	w = 0;
	h = 0;

	if (!pDevice) {
		return false;
	}

	LPDIRECT3DSURFACE9 suf = NULL;
	D3DSURFACE_DESC desc;
	pDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &suf);
	if (suf != NULL) {
		suf->GetDesc(&desc);
		w = (int)desc.Width;
		h = (int)desc.Height;
		suf->Release();
		return true;
	}
#endif /* USE_DIRECT3D */
	return false;
}

D3DTEXTUREFILTERTYPE CD3DDevice::GetFilterType(int filter_type)
{
#ifdef USE_DIRECT3D
	if (filter_type == 0) {
		return D3DTEXF_POINT;
	} else {
		return D3DTEXF_LINEAR;
	}
#else
	return 0;
#endif /* USE_DIRECT3D */
};

// ===========================================================================

CD3DSurface::CD3DSurface()
{
	pSurface = NULL;
}
CD3DSurface::~CD3DSurface()
{
	ReleaseD3DSurface();
}

HRESULT CD3DSurface::CreateD3DSurface(CD3DDevice &D3Device, int w, int h)
{
#ifdef USE_DIRECT3D
	PDIRECT3DDEVICE9 pDevice = D3Device.GetDevice();
	if (!pDevice) return S_FALSE;

	return pDevice->CreateOffscreenPlainSurface(w, h, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &pSurface, NULL);
#else
	return S_FALSE;
#endif /* USE_DIRECT3D */
}

HRESULT CD3DSurface::CreateD3DMemorySurface(CD3DDevice &D3Device, int w, int h)
{
#ifdef USE_DIRECT3D
	PDIRECT3DDEVICE9 pDevice = D3Device.GetDevice();
	if (!pDevice) return S_FALSE;

	return pDevice->CreateOffscreenPlainSurface(w, h, D3DFMT_X8R8G8B8, D3DPOOL_SYSTEMMEM, &pSurface, NULL);
#else
	return S_FALSE;
#endif /* USE_DIRECT3D */
}

HRESULT CD3DSurface::CreateD3DBackBufferSurface(CD3DDevice &D3Device)
{
#ifdef USE_DIRECT3D
	PDIRECT3DDEVICE9 pDevice = D3Device.GetDevice();
	if (!pDevice) return S_FALSE;

	return pDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pSurface);
#else
	return S_FALSE;
#endif /* USE_DIRECT3D */
}

void CD3DSurface::ReleaseD3DSurface()
{
#ifdef USE_DIRECT3D
	if (pSurface) {
		pSurface->Release();
		pSurface = NULL;
	}
#endif /* USE_DIRECT3D */
}

PDIRECT3DSURFACE9 CD3DSurface::GetD3DSurface()
{
	return pSurface;
}

HRESULT CD3DSurface::GetDC(HDC *phdc)
{
#ifdef USE_DIRECT3D
	if (!pSurface) return S_FALSE;

	return pSurface->GetDC(phdc);
#else
	return S_FALSE;
#endif /* USE_DIRECT3D */
}

void CD3DSurface::ReleaseDC(HDC hdc)
{
#ifdef USE_DIRECT3D
	if (!pSurface) return;

	pSurface->ReleaseDC(hdc);
#endif /* USE_DIRECT3D */
}

HRESULT CD3DSurface::Update(CD3DDevice &D3DDevice, CD3DSurface &srcSurface, RECT &srcRect, RECT &dstRect)
{
#ifdef USE_DIRECT3D
	PDIRECT3DDEVICE9 pDevice = D3DDevice.GetDevice();
	if (!pDevice) return S_FALSE;

	POINT dstPoint = { dstRect.left, dstRect.top };
	return pDevice->UpdateSurface(srcSurface.GetD3DSurface(), &srcRect, pSurface, &dstPoint);
#else
	HRESULT S_FALSE;
#endif /* USE_DIRECT3D */
}

HRESULT CD3DSurface::Update(CD3DDevice &D3DDevice, CD3DSurface &srcSurface, RECT &srcRect)
{
#ifdef USE_DIRECT3D
	PDIRECT3DDEVICE9 pDevice = D3DDevice.GetDevice();
	if (!pDevice) return S_FALSE;

	return pDevice->UpdateSurface(srcSurface.GetD3DSurface(), &srcRect, pSurface, NULL);
#else
	HRESULT S_FALSE;
#endif /* USE_DIRECT3D */
}

HRESULT CD3DSurface::StretchRect(CD3DDevice &D3DDevice, CD3DSurface &srcSurface, RECT &srcRect, RECT &dstRect, int filter_type)
{
#ifdef USE_DIRECT3D
	PDIRECT3DDEVICE9 pDevice = D3DDevice.GetDevice();
	if (!pDevice) return S_FALSE;

	D3DTEXTUREFILTERTYPE filter = CD3DDevice::GetFilterType(filter_type);
	return pDevice->StretchRect(srcSurface.GetD3DSurface(), &srcRect, pSurface, &dstRect, filter);
#else
	HRESULT S_FALSE;
#endif /* USE_DIRECT3D */
}

HRESULT CD3DSurface::StretchRect(CD3DDevice &D3DDevice, CD3DSurface &srcSurface, RECT &srcRect, int filter_type)
{
#ifdef USE_DIRECT3D
	PDIRECT3DDEVICE9 pDevice = D3DDevice.GetDevice();
	if (!pDevice) return S_FALSE;

	D3DTEXTUREFILTERTYPE filter = CD3DDevice::GetFilterType(filter_type);
	return pDevice->StretchRect(srcSurface.GetD3DSurface(), &srcRect, pSurface, NULL, filter);
#else
	HRESULT S_FALSE;
#endif /* USE_DIRECT3D */
}

int CD3DSurface::Width() const
{
#ifdef USE_DIRECT3D
	if (pSurface) {
		D3DSURFACE_DESC desc;
		pSurface->GetDesc(&desc);
		return (int)desc.Width;
	}
#endif /* USE_DIRECT3D */
	return 0;
}

int CD3DSurface::Height() const
{
#ifdef USE_DIRECT3D
	if (pSurface) {
		D3DSURFACE_DESC desc;
		pSurface->GetDesc(&desc);
		return (int)desc.Height;
	}
#endif /* USE_DIRECT3D */
	return 0;
}

// ===========================================================================

CD3DTexture::CD3DTexture()
{
	pTexture = NULL;
#ifdef USE_DIRECT3D
	m_first = true;
#endif /* USE_DIRECT3D */
}
CD3DTexture::~CD3DTexture()
{
	ReleaseD3DTexture();
}

HRESULT CD3DTexture::CreateD3DTexture(CD3DDevice &D3Device, int w, int h)
{
#ifdef USE_DIRECT3D
	PDIRECT3DDEVICE9 pDevice = D3Device.GetDevice();
	if (!pDevice) return S_FALSE;

	int tw = w / 16;
	if (w < 320) {
		tw = 320;
	} else {
		tw = ((tw + 1) & ~1) * 16;
	}
	int th = h;
	if (th < 32) th = 32;

	HRESULT hre = pDevice->CreateTexture(tw, th, 1, D3DUSAGE_DYNAMIC, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &pTexture, NULL);

	// set vertex
	if (m_first) {
		VERTEX_LIMIT(vl, vt, vr, vb, 0, 0, w, h)
		VERTEX_IN(m_vertex[0], vl, vt, 0, 0, 0);
		VERTEX_IN(m_vertex[1], vr, vt, 0, (float)w/tw, 0);
		VERTEX_IN(m_vertex[2], vr, vb, 0, (float)w/tw, (float)h/th);
		VERTEX_IN(m_vertex[3], vl, vb, 0, 0, (float)h/th);
	}
	m_first = false;
	return hre;
#else
	return S_FALSE;
#endif /* USE_DIRECT3D */
}

void CD3DTexture::ReleaseD3DTexture()
{
#ifdef USE_DIRECT3D
	if (pTexture) {
		pTexture->Release();
		pTexture = NULL;
	}
#endif /* USE_DIRECT3D */
}

HRESULT CD3DTexture::DrawD3DTexture(CD3DDevice &D3Device)
{
#ifdef USE_DIRECT3D
	PDIRECT3DDEVICE9 pDevice = D3Device.GetDevice();
	if (!pDevice) return S_FALSE;

	HRESULT hre = pDevice->SetTexture(0, pTexture);

	if (hre == D3D_OK) {
//		pD3Device->SetTextureStageState(0,	D3DTSS_ALPHAARG1	, D3DTA_TEXTURE	);
//		pD3Device->SetTextureStageState(0,	D3DTSS_ALPHAOP	, D3DTOP_SELECTARG1	);

		hre = pDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, m_vertex, sizeof(d3d_vertex_t));
	}
	return hre;
#else
	return S_FALSE;
#endif /* USE_DIRECT3D */
}

bool CD3DTexture::CopyD3DTextureFrom(CSurface *suf)
{
#ifdef USE_DIRECT3D
	int dst_w = GetD3DTextureWidth();

	D3DLOCKED_RECT pLockedRect;
	HRESULT hre = pTexture->LockRect(0, &pLockedRect, NULL, 0);
	if (hre != D3D_OK) {
		return false;
	}
	// copy to main context
	int src_w = suf->Width();
	int src_h = suf->Height();
	scrntype *src = suf->GetBuffer(src_h - 1);
	scrntype *dst = (scrntype *)pLockedRect.pBits;
	for(int y=0; y<src_h; y++) {
		for(int x=0; x<src_w; x++) {
			dst[x] = src[x];
		}
		src -= src_w;
		dst += dst_w;
	}
	pTexture->UnlockRect(0);

	return true;
#else
	return false;
#endif /* USE_DIRECT3D */
}

bool CD3DTexture::CopyD3DTextureFrom(CSurface *suf, int suf_top, int suf_h)
{
#ifdef USE_DIRECT3D
	int dst_w = GetD3DTextureWidth();

	D3DLOCKED_RECT pLockedRect;
	HRESULT hre = pTexture->LockRect(0, &pLockedRect, NULL, 0);
	if (hre != D3D_OK) {
		return false;
	}
	// copy to main context
	int src_w = suf->Width();
	int src_h = suf->Height();
	scrntype *src = suf->GetBuffer(src_h - suf_top - 1);
	scrntype *dst = (scrntype *)pLockedRect.pBits;
	for(int y=0; y<suf_h; y++) {
		for(int x=0; x<src_w; x++) {
			dst[x] = src[x];
		}
		src -= src_w;
		dst += dst_w;
	}
	pTexture->UnlockRect(0);

	return true;
#else
	return false;
#endif /* USE_DIRECT3D */
}

void CD3DTexture::SetD3DTexturePosition(RECT &re)
{
#ifdef USE_DIRECT3D
	// set vertex
	VERTEX_LIMIT(vl, vt, vr, vb, re.left, re.top, re.right, re.bottom)
	VERTEX_XYIN(m_vertex[0], vl, vt);
	VERTEX_XYIN(m_vertex[1], vr, vt);
	VERTEX_XYIN(m_vertex[2], vr, vb);
	VERTEX_XYIN(m_vertex[3], vl, vb);
#endif /* USE_DIRECT3D */
}

void CD3DTexture::SetD3DTexturePositionUv(RECT &re)
{
#ifdef USE_DIRECT3D
	// set vertex
	int tex_w = GetD3DTextureWidth();
	int tex_h = GetD3DTextureHeight();
	float u = (float)(re.right - re.left) / tex_w;
	float v = (float)(re.bottom - re.top) / tex_h;
	VERTEX_LIMIT(vl, vt, vr, vb, re.left, re.top, re.right, re.bottom)
	VERTEX_IN(m_vertex[0], vl, vt, 0, 0, 0);
	VERTEX_IN(m_vertex[1], vr, vt, 0, u, 0);
	VERTEX_IN(m_vertex[2], vr, vb, 0, u, v);
	VERTEX_IN(m_vertex[3], vl, vb, 0, 0, v);
#endif /* USE_DIRECT3D */
}

void CD3DTexture::SetD3DTexturePosition(VmRectWH &re)
{
#ifdef USE_DIRECT3D
	// set vertex
	VERTEX_LIMIT(vl, vt, vr, vb, re.x, re.y, re.x + re.w, re.y + re.h)
	VERTEX_XYIN(m_vertex[0], vl, vt);
	VERTEX_XYIN(m_vertex[1], vr, vt);
	VERTEX_XYIN(m_vertex[2], vr, vb);
	VERTEX_XYIN(m_vertex[3], vl, vb);
#endif /* USE_DIRECT3D */
}

void CD3DTexture::SetD3DTexturePosition(int x, int y, int w, int h)
{
#ifdef USE_DIRECT3D
	// set vertex
	VERTEX_LIMIT(vl, vt, vr, vb, x, y, x + w, y + h)
	VERTEX_XYIN(m_vertex[0], vl, vt);
	VERTEX_XYIN(m_vertex[1], vr, vt);
	VERTEX_XYIN(m_vertex[2], vr, vb);
	VERTEX_XYIN(m_vertex[3], vl, vb);
#endif /* USE_DIRECT3D */
}

void CD3DTexture::SetD3DTexturePositionUV(float u, float v, float uw, float vh)
{
#ifdef USE_DIRECT3D
	VERTEX_UVIN(m_vertex[0], u, v);
	VERTEX_UVIN(m_vertex[1], u + uw, v);
	VERTEX_UVIN(m_vertex[2], u + uw, v + vh);
	VERTEX_UVIN(m_vertex[3], u, v + vh);
#endif /* USE_DIRECT3D */
}

PDIRECT3DTEXTURE9 CD3DTexture::GetD3DTexture()
{
	return pTexture;
}

int CD3DTexture::GetD3DTextureWidth() const
{
#ifdef USE_DIRECT3D
	if (pTexture) {
		D3DSURFACE_DESC desc;
		pTexture->GetLevelDesc(0, &desc);
		return (int)desc.Width;
	}
#endif /* USE_DIRECT3D */
	return 0;
}

int CD3DTexture::GetD3DTextureHeight() const
{
#ifdef USE_DIRECT3D
	if (pTexture) {
		D3DSURFACE_DESC desc;
		pTexture->GetLevelDesc(0, &desc);
		return (int)desc.Height;
	}
#endif /* USE_DIRECT3D */
	return 0;
}
