/** @file qt_screenmode.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2019.10.01

	@brief [ screen mode ]
*/

#ifndef QT_SCREEN_MODE_H
#define QT_SCREEN_MODE_H

#include "../../common.h"
#include "../screenmode.h"


/// remain size on each displays
class ScreenMode : public ScreenModeBase
{
public:
	ScreenMode();
	~ScreenMode();

	void Enum();
	void Enum(int desktop_width, int desktop_height, int bits_per_pixel);

	static void GetDesktopSize(int *width, int *height, int *bpp);
};

#endif /* QT_SCREEN_MODE_H */
