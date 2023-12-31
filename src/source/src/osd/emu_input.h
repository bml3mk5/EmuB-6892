/** @file emu_input.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2019.10.01 -

	@brief [ emu input ]
*/

#ifndef EMU_INPUT_H
#define EMU_INPUT_H

#define KEY_KEEP_FRAMES 3

#if defined(USE_WIN)
#include "windows/win_input.h"
#elif defined(USE_SDL) || defined(USE_SDL2)
#include "SDL/sdl_input.h"
#elif defined(USE_WX) || defined(USE_WX2)
#include "wxwidgets/wxw_input.h"
#elif defined(USE_QT)
#include "qt/qt_input.h"
#endif

#ifdef USE_AUTO_KEY

#define AUTO_KEY_SHIFT	0x07
#define AUTO_KEY_RETURN	0x2f
#define AUTO_KEY_KANA	0x0a
#define AUTO_KEY_GRAPH	0x0b
#define AUTO_KEY_NONE	0xff
#define AUTO_KEY_MASK		0x007f
#define AUTO_KEY_PRESS_MASK 0x0080
#define AUTO_KEY_SHIFT_MASK	0x0100
#define AUTO_KEY_GRAPH_MASK	0x0200
#define AUTO_KEY_UPPER_MASK	0x0400
#define AUTO_KEY_LOWER_MASK	0x0800
#define AUTO_KEY_KATA_MASK	0x1000
#define AUTO_KEY_HIRA_MASK	0x2000

static const int autokey_table[256] = {
	// 0x100: shift
	// 0x200: graph
	// 0x400: alphabet
	// 0x800: ALPHABET
	// 0x1000: katakana
	// 0x2000: hiragana
	// use vm key scan code 0x80 - 0xfe
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x0af,0x000,0x000,0x0af,0x000,0x000,
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x08c,0x083,0x085,0x081,0x084,
	0x080,0x19a,0x19b,0x197,0x191,0x199,0x192,0x190,0x193,0x19c,0x1b6,0x1b4,0x0c3,0x096,0x0cc,0x0c4,
	0x094,0x09a,0x09b,0x097,0x091,0x099,0x092,0x090,0x093,0x09c,0x0b6,0x0b4,0x1c3,0x196,0x1cc,0x1c4,
	0x0a6,0x4b8,0x4c9,0x4cb,0x4bb,0x4ab,0x4b1,0x4b9,0x4b2,0x4a3,0x4b0,0x4b3,0x4bc,0x4c0,0x4c2,0x4ac,
	0x4a4,0x4a8,0x4a1,0x4ba,0x4a9,0x4a0,0x4c1,0x4aa,0x4ca,0x4a2,0x4c8,0x0a5,0x09f,0x0b5,0x095,0x1c6,
	0x000,0x8b8,0x8c9,0x8cb,0x8bb,0x8ab,0x8b1,0x8b9,0x8b2,0x8a3,0x8b0,0x8b3,0x8bc,0x8c0,0x8c2,0x8ac,
	0x8a4,0x8a8,0x8a1,0x8ba,0x8a9,0x8a0,0x8c1,0x8aa,0x8ca,0x8a2,0x8c8,0x000,0x000,0x000,0x000,0x000,
	// hiragana1
	0x2c8,0x2ca,0x2cb,0x2c1,0x2c9,0x2c2,0x2c0,0x2c3,0x2cc,0x2bc,0x2ac,0x29c,0x293,0x2a3,0x2b3,0x2bd,
	0x2cd,0x28d,0x2be,0x2b7,0x2b4,0x2bf,0x2cf,0x2c4,0x29d,0x28e,0x2c7,0x2ce,0x2a1,0x2a9,0x2b1,0x2b9,
	// katakana
	0x029a,0x11cc,0x11a5,0x11b5,0x11c3,0x11c4,0x1194,0x1197,0x11ab,0x1191,0x1199,0x1192,0x1190,0x1193,0x119c,0x11c8,
	0x109f,0x1097,0x10ab,0x1091,0x1099,0x1092,0x10a9,0x10b9,0x10b2,0x10b6,0x10c9,0x10ca,0x10bb,0x10a1,0x10a4,0x10cb,
	0x10a8,0x10b8,0x10c8,0x10aa,0x10ba,0x10a0,0x10a3,0x109a,0x10c3,0x10b3,0x10b1,0x10c1,0x109b,0x1095,0x1096,0x10b0,
	0x10c2,0x10b5,0x10c4,0x10c0,0x1090,0x1093,0x109c,0x10ac,0x10bc,0x10cc,0x10b4,0x10c6,0x1094,0x10a2,0x10a6,0x10a5,
	// hiragana2
	0x296,0x2a6,0x2b6,0x2c6,0x2a2,0x2a0,0x2b2,0x2b0,0x2a8,0x2b8,0x2aa,0x2ba,0x2ab,0x2bb,0x2c5,0x28f,
	0x2a7,0x29f,0x29b,0x297,0x291,0x299,0x292,0x290,0x2ad,0x294,0x2a4,0x295,0x2a5,0x20a2,0x2b5,0x000,
};
#endif

#endif	/* EMU_INPUT_H */
