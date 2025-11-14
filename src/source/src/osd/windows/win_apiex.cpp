/** @file win_apiex.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2025.05.18 -

	@brief [ win api ex ]
*/

#include "win_apiex.h"
#include <tchar.h>

#if _MSC_VER >= 1920 && WINVER >= 0x0b00
#define USE_INTERNAL_USER32_DLL
#endif

#ifdef USE_INTERNAL_USER32_DLL
#pragma comment(lib, "User32.lib")
#endif

namespace WIN_API_EX
{

// in User32.dll
static int mUser32DllRefCount = 0;
static bool bUser32DllLoaded = false;
static HMODULE hUser32Dll = NULL;
// GetDpiForSystem is defined in User32.dll on Windows 10 version 1607 (WINVER >= 0x0605)
static UINT (WINAPI *f_GetDpiForSystem)() = NULL;
// GetDpiForWindow is defined in User32.dll on Windows 10 version 1607 (WINVER >= 0x0605)
static UINT (WINAPI *f_GetDpiForWindow)(HWND hwnd) = NULL;

static void LoadUser32Dll()
{
	mUser32DllRefCount++;

	if (hUser32Dll) return;

	try {
		hUser32Dll = ::GetModuleHandle(_T("User32"));
		if (!hUser32Dll) {
			hUser32Dll = ::LoadLibrary(_T("User32"));
			if (!hUser32Dll) return;
			bUser32DllLoaded = true;
		}

		f_GetDpiForSystem = (UINT(WINAPI *)())::GetProcAddress(hUser32Dll, "GetDpiForSystem");
		f_GetDpiForWindow = (UINT(WINAPI *)(HWND))::GetProcAddress(hUser32Dll, "GetDpiForWindow");
	}
	catch (...) {
		hUser32Dll = NULL;
		f_GetDpiForSystem = NULL;
		f_GetDpiForWindow = NULL;
	}
}

void Load()
{
	LoadUser32Dll();
}

static void UnloadUser32Dll()
{
	mUser32DllRefCount--;

	if (!hUser32Dll) return;
	if (mUser32DllRefCount > 0) return;

	f_GetDpiForSystem = NULL;
	f_GetDpiForWindow = NULL;

	if (bUser32DllLoaded) ::FreeLibrary(hUser32Dll);
	bUser32DllLoaded = false;
	hUser32Dll = NULL;
}

void Unload()
{
	UnloadUser32Dll();
}

UINT GetDpiForSystem()
{
	UINT dpi = 0;
#ifndef USE_INTERNAL_USER32_DLL
	if (f_GetDpiForSystem) {
		dpi = f_GetDpiForSystem();
	}
#else
	dpi = ::GetDpiForSystem();
#endif
	if (dpi == 0) {
		HDC hdc = GetDC(NULL);
		dpi = (UINT)GetDeviceCaps(hdc, LOGPIXELSY);
		ReleaseDC(NULL, hdc);
	}
	if (dpi == 0) {
		dpi = USER_DEFAULT_SCREEN_DPI;
	}
	return dpi;
}

UINT GetDpiForWindow(HWND hwnd)
{
	UINT dpi = 0;
#ifndef USE_INTERNAL_USER32_DLL
	if (f_GetDpiForWindow && hwnd) {
		dpi = f_GetDpiForWindow(hwnd);
	}
#else
	if (hwnd) {
		dpi = ::GetDpiForWindow(hwnd);
	}
#endif
	else {
		HDC hdc = GetDC(hwnd);
		dpi = (UINT)GetDeviceCaps(hdc, LOGPIXELSY);
		ReleaseDC(hwnd, hdc);
	}
	if (dpi == 0) {
		dpi = USER_DEFAULT_SCREEN_DPI;
	}
	return dpi;
}

}; /* WIN_API_EX */
