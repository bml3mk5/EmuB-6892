/** @file parseopt.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2017.11.01 -

	@brief [ parseopt ]
*/

#ifndef PARSEOPT_H
#define PARSEOPT_H

#if defined(USE_WIN)
#include "osd/windows/win_parseopt.h"
#elif defined(USE_SDL) || defined(USE_SDL2) || defined(USE_WX)
#include "osd/SDL/sdl_parseopt.h"
#elif defined(USE_QT)
#include "osd/qt/qt_parseopt.h"
#endif

#endif /* PARSEOPT_H */
