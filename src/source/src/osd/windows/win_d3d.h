/** @file win_d3d.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2022.02.22 -

	@brief [ Direct3D ]
*/

#ifndef WIN_D3D_H
#define WIN_D3D_H

#include "../../vm/vm_defs.h"

#include <windows.h>
#include "../../common.h"

#ifdef USE_DIRECT3D

//#define DIRECT3D_VERSION 0x900
#include <d3d9.h>
#pragma comment(lib, "d3d9.lib")
#ifdef USE_DIRECT3DX
#include <d3dx9.h>
#pragma comment(lib, "d3dx9.lib")
#endif

typedef struct st_d3d_vertex {
	float x,y,z,rhw;
	float u,v;
} d3d_vertex_t;

#else

#define PDIRECT3D9 void *
#define PDIRECT3DDEVICE9 void *
#define PDIRECT3DSURFACE9 void *
#define PDIRECT3DTEXTURE9 void *
#define D3DTEXTUREFILTERTYPE int

#endif /* USE_DIRECT3D */

class CSurface;

/**
	@brief the device
*/
class CD3DDevice
{
protected:
	PDIRECT3D9 pD3D;
#ifdef USE_DIRECT3D
	D3DPRESENT_PARAMETERS	mD3Dpp;
	D3DDISPLAYMODE			mD3Ddm;
#endif /* USE_DIRECT3D */
	PDIRECT3DDEVICE9	pDevice;

public:
	CD3DDevice();
	virtual ~CD3DDevice();

	bool Load();
	void Unload();

	HRESULT CreateDevice(HWND hWnd);
	void ReleaseDevice();
	HRESULT ResetDevice();

	PDIRECT3DDEVICE9 GetDevice();

	void ClearDevice();
	HRESULT TestDevice();

	HRESULT BeginScene(int filter_type);
	void EndScene();
	HRESULT Present();

	void SetViewport(int w, int h);
	HRESULT SetFVF();

	bool IsEnableScanlineOrInterval() const;
	void SetPresentationInterval(int val);

	void SetPresentParametersSize(int w, int h);

	bool GetBackBufferSize(int &w, int &h);

	static D3DTEXTUREFILTERTYPE GetFilterType(int filter_type);
};

/**
	@brief one surface
*/
class CD3DSurface
{
protected:
	PDIRECT3DSURFACE9 pSurface;

public:
	CD3DSurface();
	virtual ~CD3DSurface();

	HRESULT CreateD3DSurface(CD3DDevice &D3Device, int w, int h);
	HRESULT CreateD3DMemorySurface(CD3DDevice &D3Device, int w, int h);
	HRESULT CreateD3DBackBufferSurface(CD3DDevice &D3Device);
	void ReleaseD3DSurface();

	PDIRECT3DSURFACE9 GetD3DSurface();

	HRESULT GetDC(HDC *phdc);
	void ReleaseDC(HDC hdc);

	HRESULT Update(CD3DDevice &D3DDevice, CD3DSurface &srcSurface, RECT &srcRect, RECT &dstRect);
	HRESULT Update(CD3DDevice &D3DDevice, CD3DSurface &srcSurface, RECT &srcRect);
	HRESULT StretchRect(CD3DDevice &D3DDevice, CD3DSurface &srcSurface, RECT &srcRect, RECT &dstRect, int filter_type);
	HRESULT StretchRect(CD3DDevice &D3DDevice, CD3DSurface &srcSurface, RECT &srcRect, int filter_type);

	int Width() const;
	int Height() const;
};

/**
	@brief one texture with one square polygon
*/
class CD3DTexture
{
protected:
	PDIRECT3DTEXTURE9 pTexture;

#ifdef USE_DIRECT3D
	d3d_vertex_t m_vertex[4];

	bool m_first;
#endif /* USE_DIRECT3D */

public:
	CD3DTexture();
	virtual ~CD3DTexture();

	HRESULT CreateD3DTexture(CD3DDevice &D3Device, int w, int h);
	void ReleaseD3DTexture();

	HRESULT DrawD3DTexture(CD3DDevice &D3Device);

	bool CopyD3DTextureFrom(CSurface *suf);
	bool CopyD3DTextureFrom(CSurface *suf, int suf_top, int suf_h);

	void SetD3DTexturePosition(RECT &re);
	void SetD3DTexturePositionUv(RECT &re);
	void SetD3DTexturePosition(VmRectWH &re);
	void SetD3DTexturePosition(int x, int y, int w, int h);
	void SetD3DTexturePositionUV(float u, float v, float uw, float vh);

	PDIRECT3DTEXTURE9 GetD3DTexture();
	int GetD3DTextureWidth() const;
	int GetD3DTextureHeight() const;
};

#endif /* WIN_D3D_H */
