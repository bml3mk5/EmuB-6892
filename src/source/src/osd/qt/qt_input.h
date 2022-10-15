/** @file qt_input.h

	Skelton for retropc emulator
	Qt edition

	@author Sasaji
	@date   2017.02.21

	@brief [ Qt input ]

*/

#ifndef QT_INPUT_H
#define QT_INPUT_H

#include "qt_emu.h"
#include "../../vm/vm_defs.h"
#include "../../keycode.h"

// 0x00 - 0x7f
const uint8_t qtkey2keycode[0x80] = {
	/* 0x00 - 0x0f */
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	/* 0x10 - 0x1f */
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	/* 0x20 - 0x2f */
	KEYCODE_SPACE,
	KEYCODE_1,
	KEYCODE_2,
	0,
	0,
	0,
	0,
	KEYCODE_QUOTE,
	KEYCODE_LPAREN,
	KEYCODE_RPAREN,
	KEYCODE_ASTERISK,
	KEYCODE_PLUS,
	KEYCODE_COMMA,
	KEYCODE_MINUS,
	KEYCODE_PERIOD,
	KEYCODE_SLASH,
	/* 0x30 - 0x3f */
	KEYCODE_0,
	KEYCODE_1,
	KEYCODE_2,
	KEYCODE_3,
	KEYCODE_4,
	KEYCODE_5,
	KEYCODE_6,
	KEYCODE_7,
	KEYCODE_8,
	KEYCODE_9,
	KEYCODE_COLON,
	KEYCODE_SEMICOLON,
	KEYCODE_LESS,
	KEYCODE_EQUALS,
	KEYCODE_GREATER,
	KEYCODE_QUESTION,
	/* 0x40 - 0x4f */
	KEYCODE_AT,
	KEYCODE_A,
	KEYCODE_B,
	KEYCODE_C,
	KEYCODE_D,
	KEYCODE_E,
	KEYCODE_F,
	KEYCODE_G,
	KEYCODE_H,
	KEYCODE_I,
	KEYCODE_J,
	KEYCODE_K,
	KEYCODE_L,
	KEYCODE_M,
	KEYCODE_N,
	KEYCODE_O,
	/* 0x50 - 0x5f */
	KEYCODE_P,
	KEYCODE_Q,
	KEYCODE_R,
	KEYCODE_S,
	KEYCODE_T,
	KEYCODE_U,
	KEYCODE_V,
	KEYCODE_W,
	KEYCODE_X,
	KEYCODE_Y,
	KEYCODE_Z,
	KEYCODE_LBRACKET,
	KEYCODE_BACKSLASH,
	KEYCODE_RBRACKET,
	KEYCODE_CARET,
	KEYCODE_UNDERSCORE,
	/* 0x60 - 0x6f */
	KEYCODE_GRAVE,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	/* 0x70 - 0x7f */
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0
};

// 0x01000000 - 0x0100007f
const uint8_t qtkey2keycode2[0x80] = {
	/* 0x00 - 0x0f */
	KEYCODE_ESCAPE,
	KEYCODE_TAB,
	0,	// Key_Backtab
	KEYCODE_BACKSPACE,
	KEYCODE_RETURN,
	KEYCODE_KP_ENTER,
	KEYCODE_INSERT,
	KEYCODE_DELETE,
	KEYCODE_PAUSE,
	KEYCODE_SYSREQ,	// Key_Print
	KEYCODE_SYSREQ,	// Key_SysReq
	KEYCODE_CLEAR,
	0,
	0,
	0,
	0,
	/* 0x10 - 0x1f */
	KEYCODE_HOME,
	KEYCODE_END,
	KEYCODE_LEFT,
	KEYCODE_UP,
	KEYCODE_RIGHT,
	KEYCODE_DOWN,
	KEYCODE_PAGEUP,
	KEYCODE_PAGEDOWN,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	/* 0x20 - 0x2f */
    KEYCODE_LSHIFT,
#ifdef Q_OS_MACX
    KEYCODE_LGUI,
    KEYCODE_LCTRL,
#else
    KEYCODE_LCTRL,
    KEYCODE_LGUI,
#endif
    KEYCODE_LALT,
	KEYCODE_CAPSLOCK,
	KEYCODE_NUMLOCK,
	KEYCODE_SCROLLLOCK,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	/* 0x30 - 0x3f */
	KEYCODE_F1,
	KEYCODE_F2,
	KEYCODE_F3,
	KEYCODE_F4,
	KEYCODE_F5,
	KEYCODE_F6,
	KEYCODE_F7,
	KEYCODE_F8,
	KEYCODE_F9,
	KEYCODE_F10,
	KEYCODE_F11,
	KEYCODE_F12,
	KEYCODE_F13,
	KEYCODE_F14,
	KEYCODE_F15,
	KEYCODE_F16,
	/* 0x40 - 0x4f */
	KEYCODE_F17,
	KEYCODE_F18,
	KEYCODE_F19,
	KEYCODE_F20,
	KEYCODE_F21,
	KEYCODE_F22,
	KEYCODE_WORLD_0,
	KEYCODE_WORLD_1,
	KEYCODE_WORLD_2,
	KEYCODE_WORLD_3,
	KEYCODE_WORLD_4,
	KEYCODE_WORLD_5,
	KEYCODE_WORLD_6,
	KEYCODE_WORLD_7,
	KEYCODE_WORLD_8,
	KEYCODE_WORLD_9,
	/* 0x50 - 0x5f */
	0,	// Key_F33
	0,	// Key_F34
	0,	// Key_F35
	KEYCODE_LSUPER,
	KEYCODE_RSUPER,
	KEYCODE_MENU,
	KEYCODE_LGUI,
	KEYCODE_RGUI,
	KEYCODE_HELP,
	0,	// Key_Direction_L
	0,
	0,
	0,
	0,
	0,
	/* 0x60 - 0x6f */
	0,	// Key_Direction_R
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	/* 0x70 - 0x7f */
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0
};

#ifdef _WIN32
typedef struct st_nc2kc {
	short nc;
	short kc;
} nc2kc_t;

const uint8_t nativecode2keycode[128] = {
	/* 0x00 - 0x0f */
	0,
	KEYCODE_ESCAPE,		// ESC
	KEYCODE_1,			// 1 !
	KEYCODE_2,			// 2 "
	KEYCODE_3,			// 3 #
	KEYCODE_4,			// 4 $
	KEYCODE_5,			// 5 %
	KEYCODE_6,			// 6 &
	KEYCODE_7,			// 7 '
	KEYCODE_8,			// 8 (
	KEYCODE_9,			// 9 )
	KEYCODE_0,			// 0
	KEYCODE_MINUS,		// - = (JP) - _ (US)
	KEYCODE_CARET,		// ~ ^ (JP) + = (US)
	KEYCODE_BACKSPACE,	// BackSpace
	KEYCODE_TAB,		// Tab
	/* 0x10 - 0x1f */
	KEYCODE_Q,			// Q
	KEYCODE_W,			// W
	KEYCODE_E,			// E
	KEYCODE_R,			// R
	KEYCODE_T,			// T
	KEYCODE_Y,			// Y
	KEYCODE_U,			// U
	KEYCODE_I,			// I
	KEYCODE_O,			// O
	KEYCODE_P,			// P
	KEYCODE_AT,			// @ ` (JP) [ { (US)
	KEYCODE_LBRACKET,	// [ { (JP) ] } (US)
	KEYCODE_RETURN,		// Enter
	KEYCODE_LCTRL,		// Left Ctrl
	KEYCODE_A,			// A
	KEYCODE_S,			// S
	/* 0x20 - 0x2f */
	KEYCODE_D,			// D
	KEYCODE_F,			// F
	KEYCODE_G,			// G
	KEYCODE_H,			// H
	KEYCODE_J,			// J
	KEYCODE_K,			// K
	KEYCODE_L,			// L
	KEYCODE_SEMICOLON,	// ; + (JP) : ; (US)
	KEYCODE_COLON,		// : * (JP) " ' (US)
	KEYCODE_GRAVE,		// Kanji `
	KEYCODE_LSHIFT,		// Left Shift
	KEYCODE_RBRACKET,	// ] } (JP) | \ (US)
	KEYCODE_Z,			// Z
	KEYCODE_X,			// X
	KEYCODE_C,			// C
	KEYCODE_V,			// V
	/* 0x30 - 0x3f */
	KEYCODE_B,			// B
	KEYCODE_N,			// N
	KEYCODE_M,			// M
	KEYCODE_COMMA,		// , <
	KEYCODE_PERIOD,		// . >
	KEYCODE_SLASH,		// / ?
	KEYCODE_RSHIFT,		// Right Shift
	KEYCODE_KP_MULTIPLY,// num *
	KEYCODE_LALT,		// Left Alt
	KEYCODE_SPACE,		// Space
	0,
	KEYCODE_F1,			// F1
	KEYCODE_F2,			// F2
	KEYCODE_F3,			// F3
	KEYCODE_F4,			// F4
	KEYCODE_F5,			// F5
	/* 0x40 - 0x4f */
	KEYCODE_F6,			// F6
	KEYCODE_F7,			// F7
	KEYCODE_F8,			// F8
	KEYCODE_F9,			// F9
	KEYCODE_F10,		// F10
	KEYCODE_PAUSE,		// Pause / Break
	KEYCODE_SCROLLLOCK,	// Scroll Lock
	KEYCODE_KP_7,		// num 7
	KEYCODE_KP_8,		// num 8
	KEYCODE_KP_9,		// num 9
	KEYCODE_KP_MINUS,	// num -
	KEYCODE_KP_4,		// num 4
	KEYCODE_KP_5,		// num 5
	KEYCODE_KP_6,		// num 6
	KEYCODE_KP_PLUS,	// num +
	KEYCODE_KP_1,		// num 1
	/* 0x50 - 0x5f */
	KEYCODE_KP_2,		// num 2
	KEYCODE_KP_3,		// num 3
	KEYCODE_KP_0,		// num 0
	KEYCODE_KP_PERIOD,	// num .
	0,
	0,
	0,
	KEYCODE_F11,		// F11
	KEYCODE_F12,		// F12
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	/* 0x60 - 0x6f */
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	/* 0x70 - 0x7f */
	KEYCODE_KATAHIRA,	// Katakana / Hiragana (JP)
	0,
	0,
	KEYCODE_UNDERSCORE,	// _
	0,
	0,
	0,
	0,
	0,
	KEYCODE_HENKAN,		// Henkan (JP)
	0,
	KEYCODE_MUHENKAN,	// Muhenkan (JP)
	0,
	KEYCODE_BACKSLASH,	// Yen
	0,
	0
};

const nc2kc_t nativecode2keycode2[] = {
	{ 0x11c, KEYCODE_KP_ENTER },// num Enter
	{ 0x11d, KEYCODE_RCTRL },	// Right ctrl
	{ 0x12a, KEYCODE_SYSREQ },	// Print Screen / SysRq
	{ 0x135, KEYCODE_KP_DIVIDE },// num /
	{ 0x138, KEYCODE_RALT },	// Right Alt
	{ 0x145, KEYCODE_NUMLOCK },	// Num Lock
	{ 0x147, KEYCODE_HOME },	// Home
	{ 0x148, KEYCODE_UP },		// Up allow
	{ 0x149, KEYCODE_PAGEUP },	// Page Up
	{ 0x14b, KEYCODE_LEFT },	// Left allow
	{ 0x14d, KEYCODE_RIGHT },	// Right allow
	{ 0x14f, KEYCODE_END },		// End
	{ 0x150, KEYCODE_DOWN },	// Down allow
	{ 0x151, KEYCODE_PAGEDOWN },// Page Down
	{ 0x152, KEYCODE_INSERT },	// Insert
	{ 0x153, KEYCODE_DELETE },	// Delete
	{ 0x15b, KEYCODE_LSUPER },	// Left Win
	{ 0x15c, KEYCODE_RSUPER },	// Right Win
	{ 0x15d, KEYCODE_MENU },	// Application
	{ -1, -1 }
};
#elif defined(__APPLE__) && defined(__MACH__)
typedef struct st_nc2kc {
    short nc;
    short kc;
} nc2kc_t;

const uint8_t vkcode2keycode[128] = {
	/* 0x00 - 0x0f */
	KEYCODE_A,			// A
	KEYCODE_S,			// S
	KEYCODE_D,			// D
	KEYCODE_F,			// F
	KEYCODE_H,			// H
	KEYCODE_G,			// G
	KEYCODE_Z,			// Z
	KEYCODE_X,			// X
	KEYCODE_C,			// C
	KEYCODE_V,			// V
	0,
	KEYCODE_B,			// B
	KEYCODE_Q,			// Q
	KEYCODE_W,			// W
	KEYCODE_E,			// E
	KEYCODE_R,			// R
	/* 0x10 - 0x1f */
	KEYCODE_Y,			// Y
	KEYCODE_T,			// T
	KEYCODE_1,			// 1 !
	KEYCODE_2,			// 2 "
	KEYCODE_3,			// 3 #
	KEYCODE_4,			// 4 $
	KEYCODE_6,			// 6 &
	KEYCODE_5,			// 5 %
	KEYCODE_CARET,		// ~ ^ (JP) + = (US)
	KEYCODE_9,			// 9 )
	KEYCODE_7,			// 7 '
	KEYCODE_MINUS,		// - = (JP) - _ (US)
	KEYCODE_8,			// 8 (
	KEYCODE_0,			// 0
	KEYCODE_LBRACKET,	// [ { (JP) ] } (US)
	KEYCODE_O,			// O
	/* 0x20 - 0x2f */
	KEYCODE_U,			// U
	KEYCODE_AT,			// @ ` (JP) [ { (US)
	KEYCODE_I,			// I
	KEYCODE_P,			// P
	KEYCODE_RETURN,		// Enter
	KEYCODE_L,			// L
	KEYCODE_J,			// J
	KEYCODE_COLON,		// : * (JP) " ' (US)
	KEYCODE_K,			// K
	KEYCODE_SEMICOLON,	// ; + (JP) : ; (US)
	KEYCODE_RBRACKET,	// ] } (JP) | \ (US)
	KEYCODE_COMMA,		// , <
	KEYCODE_SLASH,		// / ?
	KEYCODE_N,			// N
	KEYCODE_M,			// M
	KEYCODE_PERIOD,		// . >
	/* 0x30 - 0x3f */
	KEYCODE_TAB,		// Tab
	KEYCODE_SPACE,		// Space
	KEYCODE_GRAVE,		// Kanji `
	KEYCODE_BACKSPACE,	// BackSpace
	0, //KEYCODE_IB_ENTER,		// iBook Enter
	KEYCODE_ESCAPE,		// ESC
	KEYCODE_RGUI,		// Right command
	KEYCODE_LGUI,		// Left command
	KEYCODE_LSHIFT,		// Left Shift
	KEYCODE_CAPSLOCK,	// CapsLock
	KEYCODE_LALT,		// Left Alt
	KEYCODE_LCTRL,		// Left Ctrl
	KEYCODE_RSHIFT,		// Right Shift
	KEYCODE_RALT,		// Right Alt
	KEYCODE_RCTRL,		// Right Ctrl
	KEYCODE_FUNCTION,	// fn
	/* 0x40 - 0x4f */
	KEYCODE_F17,		// F17
	KEYCODE_KP_PERIOD,	// num .
	0,
	KEYCODE_KP_MULTIPLY,// num *
	0,
	KEYCODE_KP_PLUS,	// num +
	0,
	KEYCODE_NUMLOCK,	// clear
	0,
	0,
	0,
	KEYCODE_KP_DIVIDE,	// num /
	KEYCODE_KP_ENTER,	// num enter
	KEYCODE_KP_MINUS,	// num -
	0,
	KEYCODE_F18,		// F18
	/* 0x50 - 0x5f */
	KEYCODE_F19,		// F19
	KEYCODE_KP_EQUALS,	// num equal
	KEYCODE_KP_0,		// num 0
	KEYCODE_KP_1,		// num 1
	KEYCODE_KP_2,		// num 2
	KEYCODE_KP_3,		// num 3
	KEYCODE_KP_4,		// num 4
	KEYCODE_KP_5,		// num 5
	KEYCODE_KP_6,		// num 6
	KEYCODE_KP_7,		// num 7
	0,
	KEYCODE_KP_8,		// num 8
	KEYCODE_KP_9,		// num 9
	KEYCODE_BACKSLASH,	// yen(backslash)
	KEYCODE_UNDERSCORE,	// _
	0,
	/* 0x60 - 0x6f */
	KEYCODE_F5,			// F5
	KEYCODE_F6,			// F6
	KEYCODE_F7,			// F7
	KEYCODE_F3,			// F3
	KEYCODE_F8,			// F8
	KEYCODE_F9,			// F9
	0,
	KEYCODE_F11,		// F11
	0,
	KEYCODE_F13,		// F13
	KEYCODE_F16,		// F16
	KEYCODE_F14,		// F14
	0,
	KEYCODE_F10,		// F10
	KEYCODE_MENU,		// menu
	KEYCODE_F12,		// F12
	/* 0x70 - 0x7f */
	0,
	KEYCODE_F15,		// F15
	KEYCODE_INSERT,		// insert
	KEYCODE_HOME,		// home
	KEYCODE_PAGEUP,		// page up
	KEYCODE_DELETE,		// delete x
	KEYCODE_F4,			// F4
	KEYCODE_END,		// end
	KEYCODE_F2,			// F2
	KEYCODE_PAGEDOWN,	// page down
	KEYCODE_F1,			// F1
	KEYCODE_LEFT,		// left
	KEYCODE_RIGHT,		// right
	KEYCODE_DOWN,		// down
	KEYCODE_UP,			// up
	KEYCODE_POWER		// power

//	KEYCODE_PAUSE,		// Pause / Break
//	KEYCODE_SCROLLLOCK,	// Scroll Lock
//	KEYCODE_KATAHIRA,	// Katakana / Hiragana (JP)
//	KEYCODE_UNDERSCORE,	// _
//	KEYCODE_HENKAN,		// Henkan (JP)
//	KEYCODE_MUHENKAN,	// Muhenkan (JP)
};

typedef struct st_ks2kc {
	short ks;
	short kc;
} ks2kc_t;

const ks2kc_t keysym2keycode[] = {
	{ 97,  KEYCODE_A },
	{ 301, KEYCODE_CAPSLOCK },
	{ 303, KEYCODE_RSHIFT },
	{ 304, KEYCODE_LSHIFT },
	{ 305, KEYCODE_RCTRL },
	{ 306, KEYCODE_LCTRL },
	{ 307, KEYCODE_RALT },
	{ 308, KEYCODE_LALT },
	{ 309, KEYCODE_RGUI },
	{ 310, KEYCODE_LGUI },
	{ -1, -1 }
};

const nc2kc_t nativecode2keycode2[] = {
    { 0x11c, KEYCODE_KP_ENTER },// num Enter
    { 0x11d, KEYCODE_RCTRL },	// Right ctrl
    { 0x12a, KEYCODE_SYSREQ },	// Print Screen / SysRq
    { 0x135, KEYCODE_KP_DIVIDE },// num /
    { 0x138, KEYCODE_RALT },	// Right Alt
    { 0x145, KEYCODE_NUMLOCK },	// Num Lock
    { 0x147, KEYCODE_HOME },	// Home
    { 0x148, KEYCODE_UP },		// Up allow
    { 0x149, KEYCODE_PAGEUP },	// Page Up
    { 0x14b, KEYCODE_LEFT },	// Left allow
    { 0x14d, KEYCODE_RIGHT },	// Right allow
    { 0x14f, KEYCODE_END },		// End
    { 0x150, KEYCODE_DOWN },	// Down allow
    { 0x151, KEYCODE_PAGEDOWN },// Page Down
    { 0x152, KEYCODE_INSERT },	// Insert
    { 0x153, KEYCODE_DELETE },	// Delete
    { 0x15b, KEYCODE_LSUPER },	// Left Win
    { 0x15c, KEYCODE_RSUPER },	// Right Win
    { 0x15d, KEYCODE_MENU },	// Application
    { -1, -1 }
};

#elif defined(__linux)
// X11 (linux etc.)
const uint8_t scancode2keycode[144] = {
    /* 0x00 - 0x0f */
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    KEYCODE_ESCAPE,		// ESC
    KEYCODE_1,			// 1 !
    KEYCODE_2,			// 2 "
    KEYCODE_3,			// 3 #
    KEYCODE_4,			// 4 $
    KEYCODE_5,			// 5 %
    KEYCODE_6,			// 6 &
    /* 0x10 - 0x1f */
    KEYCODE_7,			// 7 '
    KEYCODE_8,			// 8 (
    KEYCODE_9,			// 9 )
    KEYCODE_0,			// 0
    KEYCODE_MINUS,		// - = (JP) - _ (US)
    KEYCODE_CARET,		// ~ ^ (JP) + = (US)
    KEYCODE_BACKSPACE,	// BackSpace
    KEYCODE_TAB,		// Tab
    KEYCODE_Q,			// Q
    KEYCODE_W,			// W
    KEYCODE_E,			// E
    KEYCODE_R,			// R
    KEYCODE_T,			// T
    KEYCODE_Y,			// Y
    KEYCODE_U,			// U
    KEYCODE_I,			// I
    /* 0x20 - 0x2f */
    KEYCODE_O,			// O
    KEYCODE_P,			// P
    KEYCODE_AT,			// @ ` (JP) [ { (US)
    KEYCODE_LBRACKET,	// [ { (JP) ] } (US)
    KEYCODE_RETURN,		// Enter
    KEYCODE_LCTRL,		// Left Ctrl
    KEYCODE_A,			// A
    KEYCODE_S,			// S
    KEYCODE_D,			// D
    KEYCODE_F,			// F
    KEYCODE_G,			// G
    KEYCODE_H,			// H
    KEYCODE_J,			// J
    KEYCODE_K,			// K
    KEYCODE_L,			// L
    KEYCODE_SEMICOLON,	// ; + (JP) : ; (US)
    /* 0x30 - 0x3f */
    KEYCODE_COLON,		// : * (JP) " ' (US)
    KEYCODE_GRAVE,		// Kanji `
    KEYCODE_LSHIFT,		// Left Shift
    KEYCODE_RBRACKET,	// ] } (JP) | \ (US)
    KEYCODE_Z,			// Z
    KEYCODE_X,			// X
    KEYCODE_C,			// C
    KEYCODE_V,			// V
    KEYCODE_B,			// B
    KEYCODE_N,			// N
    KEYCODE_M,			// M
    KEYCODE_COMMA,		// , <
    KEYCODE_PERIOD,		// . >
    KEYCODE_SLASH,		// / ?
    KEYCODE_RSHIFT,		// Right Shift
    KEYCODE_KP_MULTIPLY,// num *
    /* 0x40 - 0x4f */
    KEYCODE_LALT,		// Left Alt
    KEYCODE_SPACE,		// Space
    KEYCODE_CAPSLOCK,	// CapsLock
    KEYCODE_F1,			// F1
    KEYCODE_F2,			// F2
    KEYCODE_F3,			// F3
    KEYCODE_F4,			// F4
    KEYCODE_F5,			// F5
    KEYCODE_F6,			// F6
    KEYCODE_F7,			// F7
    KEYCODE_F8,			// F8
    KEYCODE_F9,			// F9
    KEYCODE_F10,		// F10
    KEYCODE_NUMLOCK,	// Num Lock
    KEYCODE_SCROLLLOCK,	// Scroll Lock
    KEYCODE_KP_7,		// num 7
    /* 0x50 - 0x5f */
    KEYCODE_KP_8,		// num 8
    KEYCODE_KP_9,		// num 9
    KEYCODE_KP_MINUS,	// num -
    KEYCODE_KP_4,		// num 4
    KEYCODE_KP_5,		// num 5
    KEYCODE_KP_6,		// num 6
    KEYCODE_KP_PLUS,	// num +
    KEYCODE_KP_1,		// num 1
    KEYCODE_KP_2,		// num 2
    KEYCODE_KP_3,		// num 3
    KEYCODE_KP_0,		// num 0
    KEYCODE_KP_PERIOD,	// num .
    0,
    0,
    0,
    0,
    /* 0x60 - 0x6f */
    0,
    0,
    0,
    0,
    KEYCODE_HENKAN,     // Henkan (JP)
    KEYCODE_KATAHIRA,	// Katakana / Hiragana (JP)
    KEYCODE_MUHENKAN,	// Muhenkan (JP)
    0,
    KEYCODE_KP_ENTER,	// num enter
    KEYCODE_RCTRL,		// Right Ctrl
    KEYCODE_KP_DIVIDE,	// num /
    KEYCODE_SYSREQ,     // Print Screen / SysRq
    KEYCODE_RALT,		// Right Alt
    0,
    KEYCODE_HOME,		// home
    KEYCODE_UP,			// up
    /* 0x70 - 0x7f */
    KEYCODE_PAGEUP,		// page up
    KEYCODE_LEFT,		// left
    KEYCODE_RIGHT,		// right
    KEYCODE_END,		// end
    KEYCODE_DOWN,		// down
    KEYCODE_PAGEDOWN,	// page down
    KEYCODE_INSERT,		// insert
    KEYCODE_DELETE,		// delete
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    KEYCODE_PAUSE,		// Pause / Break
    /* 0x80 - 0x8f */
    0,
    0,
    0,
    0,
    KEYCODE_BACKSLASH,	// yen(backslash)
    KEYCODE_LSUPER,		// Left win
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0
};
#endif

#endif	// QT_INPUT_H
