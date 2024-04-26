/** @file wxw_common.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2017.11.01 -

	@brief [ common header ]
*/

#ifndef WXW_COMMON_H
#define WXW_COMMON_H

#if defined(_MSC_VER) && (_MSC_VER <= 1500)
#include "inttypes.h"
#else
#include <stdint.h>
#endif

#include <string.h>

#if defined(_WIN32)
#include <tchar.h>
#else
#ifdef _T
#undef _T
#endif
#include "../../extra/tchar.h"
#endif

#if defined(_MSC_VER)
#define WIN32_LEAN_AND_MEAN
#include <stdlib.h>

// disable warnings C4189, C4995 and C4996 for microsoft visual c++ 2005
#if (_MSC_VER >= 1400)
#pragma warning( disable : 4819 )
#pragma warning( disable : 4995 )
#pragma warning( disable : 4996 )
#endif

// check memory leaks
#if defined(_DEBUG)
////#define _CRTDBG_MAP_ALLOC
//#undef _malloca
#include <wx/any.h>
#include <wx/vector.h>
#include <wx/thread.h>
#include <crtdbg.h>
#include <new>
//#define malloc(s) _malloc_dbg(s, _NORMAL_BLOCK, __FILE__, __LINE__)
//#if _MSC_VER > 1500
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#undef wxUSE_ANY
//#endif
#endif

#endif /* _MSC_VER */

/********** platform dependency **********/
#if defined(_WIN32)
/********** windows **********/

#elif (defined(__APPLE__) && defined(__MACH__))
/********** macosx **********/
#include <unistd.h>
#define __stdcall

#else
/********** linux (Unix) **********/
#include <unistd.h>
#define __stdcall

#define USE_LIB_GTK
#ifdef GUI_TYPE_AGAR
#define USE_X11_LEDBOX
#define USE_X11_VKEYBOARD
#else
#define USE_GTK_LEDBOX
#define USE_GTK_VKEYBOARD
#endif

#endif

#define CDelay(ms) wxMilliSleep(ms)

#endif /* WXW_COMMON_H */
