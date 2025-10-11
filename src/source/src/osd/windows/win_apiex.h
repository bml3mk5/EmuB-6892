/** @file win_apiex.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2025.05.18 -

	@brief [ win api ex ]
*/

#ifndef _WIN_API_EX_H_
#define _WIN_API_EX_H_

#include <windows.h>

// WM_DPICHANGED is defined in WinUser.h on Windows8 or lator (WINVER >= 0x0601)
#ifndef WM_DPICHANGED
#define WM_DPICHANGED 0x02E0
#endif

///
/// This namespace allows newer functions to be called on older OS.
///
namespace WIN_API_EX
{
	void Load();
	void Unload();

	UINT GetDpiForSystem();
	UINT GetDpiForWindow(HWND hwnd);
};

#endif /* _WIN_MAIN_H_ */
