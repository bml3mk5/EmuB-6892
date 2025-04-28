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
#define APP_COPYRIGHT	"Copyright (C) 2011,2012-2025 Common Source Code Project, Sasaji"
#define APP_VERSION		"2.0.5.2118"
#define APP_VER_MAJOR	2
#define APP_VER_MINOR	0
#define APP_VER_REV		5
#define APP_VER_BUILD	2118

#if defined(__MINGW32__)
#if defined(x86_64) || defined(__x86_64)
#define PLATFORM "Windows(MinGW) Intel 64bit"
#elif defined(i386) || defined(__i386)
#define PLATFORM "Windows(MinGW) Intel 32bit"
#else
#define PLATFORM "Windows(MinGW)"
#endif
#elif defined(_WIN32)
#if defined(_M_ARM64)
#define PLATFORM "Windows Arm 64bit"
#elif defined(_M_ARM)
#define PLATFORM "Windows Arm 32bit"
#elif defined(_M_X64)
#define PLATFORM "Windows Intel 64bit"
#elif defined(_M_IX86)
#define PLATFORM "Windows Intel 32bit"
#elif defined(x64)
#define PLATFORM "Windows 64bit"
#elif defined(Win32)
#define PLATFORM "Windows 32bit"
#else
#define PLATFORM "Windows"
#endif
#elif defined(linux)
#ifdef __x86_64
#define PLATFORM "Linux Intel 64bit"
#elif __i386
#define PLATFORM "Linux Intel 32bit"
#else
#define PLATFORM "Linux"
#endif
#elif defined(__APPLE__) && defined(__MACH__)
#ifdef __arm64
#define PLATFORM "MacOS Arm 64bit"
#elif __x86_64
#define PLATFORM "MacOS Intel 64bit"
#elif __i386
#define PLATFORM "MacOS Intel 32bit"
#else
#define PLATFORM "MacOS"
#endif
#elif defined(__FreeBSD__)
#ifdef __x86_64
#define PLATFORM "FreeBSD Intel 64bit"
#elif __i386
#define PLATFORM "FreeBSD Intel 32bit"
#else
#define PLATFORM "FreeBSD"
#endif
#else
#define PLATFORM "Unknown"
#endif

#endif /* VERSION_H */
