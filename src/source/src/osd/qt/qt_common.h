/** @file qt_common.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2017.11.01 -

	@brief [ common header ]
*/

#ifndef QT_COMMON_H
#define QT_COMMON_H

#if defined(_MSC_VER) && (_MSC_VER <= 1500)
#include "inttypes.h"
#else
#include <stdint.h>
#endif

#include <string.h>

#if defined(_WIN32)
#include <tchar.h>
#else
#include "../../extra/tchar.h"
#endif

#if defined(_MSC_VER)
#include <stdlib.h>

// disable warnings C4189, C4995 and C4996 for microsoft visual c++ 2005
#if (_MSC_VER >= 1400)
#pragma warning( disable : 4100 )
#pragma warning( disable : 4819 )
#pragma warning( disable : 4995 )
#pragma warning( disable : 4996 )
#endif

#endif /* _MSC_VER */


/********** platform dependency **********/
#if defined(_WIN32)
/********** windows **********/

#define USE_NATIVE_KEYINPUT 1

// Treat char string as UTF-8 on Qt
// So, needs converting to MBCS string when call Windows API directly.
#define USE_UTF8_ON_MBCS	1

#elif (defined(__APPLE__) && defined(__MACH__))
/********** macosx **********/
#include <unistd.h>
#define __stdcall

#define USE_NATIVE_KEYINPUT 1

#else
/********** linux (Unix) **********/
#include <unistd.h>
#define __stdcall

#define USE_NATIVE_KEYINPUT 1

#endif

#define CDelay(ms) QThread::msleep(ms);


#endif /* QT_COMMON_H */
