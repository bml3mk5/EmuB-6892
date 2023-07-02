/** @file vkeyboard_bml3mk5.h

	HITACHI BASIC MASTER LEVEL3 Mark5 Emulator 'EmuB-6892'
	Skelton for retropc emulator

	@author Sasaji
	@date   2016.12.21 -

	@brief [ virtual keyboard layout for bml3mk5 ]
*/

#ifndef VKEYBOARD_BML3MK5_H
#define VKEYBOARD_BML3MK5_H

#include "vkeyboard.h"

namespace Vkbd {

const struct stBitmap_t cBmpParts[] = {
	{ BITMAPIDS_LED_PARTS, 7 * 16, 0, 13, 6 },	// LED Red Horizontal
	{ BITMAPIDS_LED_PARTS, 9 * 16, 0, 13, 6 },	// LED Green Horizontal
	{ BITMAPIDS_LED_PARTS, 1 * 16, 0, 6, 13 },	// LED Red Vertical
	{ BITMAPIDS_LED_PARTS, 8 * 16, 0, 13, 6 },	// LED Green Horizontal OFF
	{ BITMAPIDS_VKEY_MODE,     0, 0, 25, 25 },	// MODE Switch
	{ BITMAPIDS_VKEY_BREAK,    0, 0, 55, 32 },	// BREAK Key
	{ BITMAPIDS_VKEY_POWER,    0, 0, 25, 25 },	// POWER Switch OFF
	{ -1, 0, 0, 0, 0 }
};

const struct stLedPos_t cLedPos[] = {
	{ BITMAPPARTS_LED_RH,     15, 172 },	// KATAKANA Led
	{ BITMAPPARTS_LED_GH,     15, 184 },	// HIRAGANA Led
	{ BITMAPPARTS_LED_RV,     22, 238 },	// CAPSLOCK Led
	{ BITMAPPARTS_MODE,      505,   4 },	// MODE Switch
	{ BITMAPPARTS_MODE,      568,   4 },	// RESET Switch
	{ BITMAPPARTS_POWER,     634,   4 },	// POWER Switch OFF
	{ BITMAPPARTS_LED_GLH,   638,  60 },	// POWER Led OFF
	{ -1, 0, 0 }
};

const Hori_t cvKeyHori0[] = {
	{505, 25, 0x7d, KEYKIND_NOANIME, -1, -1},	// MODE
	{568, 25, 0x7f, KEYKIND_NOANIME, -1, -1},	// RESET
	{634, 25, 0x7e, KEYKIND_NOANIME, -1, -1},	// POWER
	{  0,  0,    0, 0, -1, -1}
};

const Hori_t cvKeyHori1[] = {
	{ 22, 55, 0x80, 0, -1, BITMAPPARTS_BREAK},	// BREAK
	{119, 65, 0x50, 0, -1, -1},	// PF1
	{185, 65, 0x51, 0, -1, -1},	// PF2
	{251, 65, 0x52, 0, -1, -1},	// PF3
	{317, 65, 0x53, 0, -1, -1},	// PF4
	{383, 65, 0x54, 0, -1, -1},	// PF5
	{555, 32, 0x2e, 0, -1, -1},	// HOME/CLS
	{588, 32, 0x01, 0, -1, -1},	// UP
	{621, 32, 0x02, 0, -1, -1},	// num ?
	{654, 32, 0x45, 0, -1, -1},	// num /
	{  0,  0,    0, 0, -1, -1}
};

const Hori_t cvKeyHori2[] = {
	{ 20, 32, 0x0c, 0, -1, -1},	// ESC
	{ 53, 32, 0x1a, 0, -1, -1},	// 1
	{ 86, 32, 0x1b, 0, -1, -1},	// 2
	{119, 32, 0x17, 0, -1, -1},	// 3
	{152, 32, 0x11, 0, -1, -1},	// 4
	{185, 32, 0x19, 0, -1, -1},	// 5
	{218, 32, 0x12, 0, -1, -1},	// 6
	{251, 32, 0x10, 0, -1, -1},	// 7
	{284, 32, 0x13, 0, -1, -1},	// 8
	{317, 32, 0x1c, 0, -1, -1},	// 9
	{350, 32, 0x14, 0, -1, -1},	// 0
	{383, 32, 0x16, 0, -1, -1},	// -
	{416, 32, 0x15, 0, -1, -1},	// ^
	{449, 32, 0x1f, 0, -1, -1},	// YEN
	{482, 32, 0x1e, 0, -1, -1},	// INS/DEL
	{555, 32, 0x03, 0, -1, -1},	// LEFT
	{588, 32, 0x04, 0, -1, -1},	// DOWN
	{621, 32, 0x05, 0, -1, -1},	// RIGHT
	{654, 32, 0x0f, 0, -1, -1},	// num *
	{  0,  0,    0, 0, -1, -1}
};

const Hori_t cvKeyHori3[] = {
	{ 36, 32, 0x0a, 0, -1, -1},	// KATA/HIRA
	{ 69, 32, 0x28, 0, -1, -1},	// Q 
	{102, 32, 0x2a, 0, -1, -1},	// W
	{135, 32, 0x2b, 0, -1, -1},	// E 
	{168, 32, 0x21, 0, -1, -1},	// R
	{201, 32, 0x29, 0, -1, -1},	// T
	{234, 32, 0x22, 0, -1, -1},	// Y 
	{267, 32, 0x20, 0, -1, -1},	// U
	{300, 32, 0x23, 0, -1, -1},	// I
	{333, 32, 0x2c, 0, -1, -1},	// O
	{366, 32, 0x24, 0, -1, -1},	// P
	{399, 32, 0x26, 0, -1, -1},	// @
	{432, 32, 0x25, 0, -1, -1},	// [
	{465, 57, 0x2f, KEYKIND_ARRAY, ARRAYKEYS_RETURN, -1},	// RETURN
	{555, 32, 0x1d, 0, -1, -1},	// num 7
	{588, 32, 0x0d, 0, -1, -1},	// num 8
	{621, 32, 0x0e, 0, -1, -1},	// num 9
	{654, 32, 0x3f, 0, -1, -1},	// num -
	{  0,  0,    0, 0, -1, -1}
};

const Hori_t cvKeyHori4[] = {
	{ 11, 32, 0x09, 0, -1, -1},	// CAPS
	{ 44, 32, 0x06, KEYKIND_TOGGLE, TOGGLEKEYS_CTRL, -1},	// CTRL
	{ 77, 32, 0x38, 0, -1, -1},	// A
	{110, 32, 0x3a, 0, -1, -1},	// S
	{143, 32, 0x3b, 0, -1, -1},	// D
	{176, 32, 0x31, 0, -1, -1},	// F
	{209, 32, 0x39, 0, -1, -1},	// G
	{242, 32, 0x32, 0, -1, -1},	// H
	{275, 32, 0x30, 0, -1, -1},	// J
	{308, 32, 0x33, 0, -1, -1},	// K
	{341, 32, 0x3c, 0, -1, -1},	// L
	{374, 32, 0x34, 0, -1, -1},	// ;
	{407, 32, 0x36, 0, -1, -1},	// :
	{440, 32, 0x35, 0, -1, -1},	// ]
	{473, 32, 0x0b, KEYKIND_TOGGLE, TOGGLEKEYS_GRAPH, -1},	// GRAPH
	{555, 32, 0x37, 0, -1, -1},	// num 4
	{588, 32, 0x3d, 0, -1, -1},	// num 5
	{621, 32, 0x3e, 0, -1, -1},	// num 6
	{654, 32, 0x4f, 0, -1, -1},	// num +
	{  0,  0,    0, 0, -1, -1}
};

const Hori_t cvKeyHori5[] = {
	{ 35, 57, 0x07, KEYKIND_TOGGLE, TOGGLEKEYS_SHIFT, -1},	// LSHIFT 
	{ 93, 32, 0x48, 0, -1, -1},	// Z
	{126, 32, 0x4a, 0, -1, -1},	// X
	{159, 32, 0x4b, 0, -1, -1},	// C
	{192, 32, 0x41, 0, -1, -1},	// V
	{225, 32, 0x49, 0, -1, -1},	// B
	{258, 32, 0x42, 0, -1, -1},	// N
	{291, 32, 0x40, 0, -1, -1},	// M
	{324, 32, 0x43, KEYKIND_ARRAY, ARRAYKEYS_COMMA, -1},	// ,
	{357, 32, 0x4c, 0, -1, -1},	// .
	{390, 32, 0x44, 0, -1, -1},	// /
	{423, 32, 0x46, 0, -1, -1},	// _
	{456, 57, 0x07, KEYKIND_TOGGLE, TOGGLEKEYS_SHIFT, -1},	// RSHIFT
	{555, 32, 0x47, 0, -1, -1},	// num 1
	{588, 32, 0x4d, 0, -1, -1},	// num 2
	{621, 32, 0x4e, 0, -1, -1},	// num 3
	{654, 32, 0x43, KEYKIND_ARRAY, ARRAYKEYS_COMMA, -1},	// num ,
	{  0,  0,    0, 0, -1, -1}
};

const Hori_t cvKeyHori6[] = {
	{126, 263, 0x00, 0, -1, -1},	// SPACE 
	{555,  32, 0x27, 0, -1, -1},	// num 0
	{588,  32, 0x2d, 0, -1, -1},	// num .
	{621,  65, 0x2f, KEYKIND_ARRAY, ARRAYKEYS_RETURN, -1},	// num RETURN
	{  0,   0,    0, 0, -1, -1}
};

const Pos_t cVkbdKeyPos[] = {
	{   4, 25, cvKeyHori0 },
	{  98, 32, cvKeyHori1 },
	{ 131, 32, cvKeyHori2 },
	{ 164, 32, cvKeyHori3 },
	{ 197, 32, cvKeyHori4 },
	{ 230, 32, cvKeyHori5 },
	{ 263, 32, cvKeyHori6 },
	{ 0, 0, NULL }
};

} /* namespace Vkbd */

#endif /* VKEYBOARD_BML3MK5_H */
