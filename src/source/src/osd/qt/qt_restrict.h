/** @file qt_restrict.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2024.11.01 -

	@brief [ restrict by Qt version ]
*/

#ifndef QT_RESTRICT_H
#define QT_RESTRICT_H

#ifdef USE_JOYSTICK
#if QT_VERSION < 0x060000
#define USE_QGAMEPAD
#else
#undef USE_JOYSTICK
#undef USE_PIAJOYSTICK
#endif
#endif

#endif /* QT_RESTRICT_H */
