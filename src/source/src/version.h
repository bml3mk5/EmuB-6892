/** @file version.h

	HITACHI BASIC MASTER LEVEL3 Mark5 Emulator 'EmuB-6892'
	Skelton for retropc emulator

	@author Sasaji
	@date   2012.03.08

	@brief [ version info ]
*/

#ifndef VERSION_H
#define VERSION_H

#define APP_NAME		"HITACHI BASIC MASTER LEVEL3 MARK5 Emulator 'EmuB-6892'"
#define APP_FILENAME	"bml3mk5.exe"
#define APP_INTERNAME	"EmuB-6892"
#define APP_COPYRIGHT	"Copyright (C) 2011,2012-2024 Common Source Code Project, Sasaji"
#define APP_VERSION		"1.9.5.2023"
#define APP_VER_MAJOR	1
#define APP_VER_MINOR	9
#define APP_VER_REV		5
#define APP_VER_BUILD	2023

#if defined(__MINGW32__)
#if defined(x86_64) || defined(__x86_64)
#define PLATFORM "Windows(MinGW) 64bit"
#elif defined(i386) || defined(__i386)
#define PLATFORM "Windows(MinGW) 32bit"
#else
#define PLATFORM "Windows(MinGW)"
#endif
#elif defined(_WIN32)
#if defined(_WIN64) || defined(_M_X64)
#define PLATFORM "Windows 64bit"
#else
#define PLATFORM "Windows 32bit"
#endif
#elif defined(linux)
#ifdef __x86_64
#define PLATFORM "Linux 64bit"
#elif __i386
#define PLATFORM "Linux 32bit"
#else
#define PLATFORM "Linux"
#endif
#elif defined(__APPLE__) && defined(__MACH__)
#ifdef __x86_64
#define PLATFORM "MacOSX 64bit"
#elif __i386
#define PLATFORM "MacOSX 32bit"
#else
#define PLATFORM "MacOSX"
#endif
#elif defined(__FreeBSD__)
#ifdef __x86_64
#define PLATFORM "FreeBSD 64bit"
#elif __i386
#define PLATFORM "FreeBSD 32bit"
#else
#define PLATFORM "FreeBSD"
#endif
#else
#define PLATFORM "Unknown"
#endif

#endif /* VERSION_H */
