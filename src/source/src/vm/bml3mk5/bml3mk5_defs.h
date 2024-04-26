/** @file bml3mk5_defs.h

	HITACHI BASIC MASTER LEVEL3 MARK5 Emulator
	Skelton for retropc emulator

	@author Sasaji
	@date   2011.06.08 -

	@brief [ virtual machine ]
*/

#ifndef BML3MK5_DEFS_H
#define BML3MK5_DEFS_H

#define FRAME_SPLIT_NUM	1

#define DEVICE_NAME		"HITACHI BASIC MASTER LEVEL3 MARK5"
#define CONFIG_NAME		"bml3mk5"
#define CLASS_NAME      "BML3MK5"
#define CONFIG_VERSION		16

// device informations for virtual machine
#define USE_EMU_INHERENT_SPEC

#define FRAMES_PER_10SECS	600
#define FRAMES_PER_SEC		60
#define LINES_PER_FRAME 	262
#define CHARS_PER_LINE		64

#define CPU_CLOCKS		1008000
#define NUMBER_OF_CPUS		1
//#define USE_CPU_REAL_MACHINE_CYCLE	1
#define CLOCKS_CYCLE		120960000	// need divisible by 30

#define MAX_SOUND	8

//#define SCREEN_WIDTH		640
#define SCREEN_WIDTH		768
//#define SCREEN_HEIGHT		480
#define SCREEN_HEIGHT		512
#define LIMIT_MIN_WINDOW_WIDTH		640
#define LIMIT_MIN_WINDOW_HEIGHT		400
#define MIN_WINDOW_WIDTH		640
#define MIN_WINDOW_HEIGHT		480
#define MAX_WINDOW_WIDTH		768
#define MAX_WINDOW_HEIGHT		512

// max devices connected to the output port
#define MAX_OUTPUT	18

// device informations for win32
#define USE_SPECIAL_RESET
#define USE_DATAREC
// #define USE_ALT_F10_KEY
#define USE_AUTO_KEY		3
#define USE_AUTO_KEY_CAPS
#define USE_SCANLINE
#define USE_AFTERIMAGE
#define USE_DIPSWITCH
#define DIPSWITCH_DEFAULT 0x03
#define HAS_AY_3_8913
//#define USE_AUDIO_U8

#define USE_PRINTER
#define MAX_PRINTER		3
#define USE_LIGHTPEN
#define USE_JOYSTICK
#define USE_PIAJOYSTICK
#define USE_PIAJOYSTICKBIT
#define USE_KEY2JOYSTICK


#define USE_FD1
#define USE_FD2
#define USE_FD3
#define USE_FD4
#define HAS_MB8876
#define MAX_FLOPPY_DISKS	4
#define USE_FLOPPY_DISKS	4

#define USE_SOCKET
#define USE_UART
#define MAX_COMM		2

#define USE_RTC

#define USE_STATE
#define USE_KEY_RECORD

#define USE_LEDBOX
#define USE_MESSAGE_BOARD
#define USE_VKEYBOARD

#ifdef USE_WIN
//#define USE_SCREEN_D3D_TEXTURE
#define USE_SCREEN_D3D_MIX_SURFACE

#define USE_DIRECTINPUT
#endif

//#define USE_PERFORMANCE_METER

#define RESUME_FILE_HEADER "RESUME_BML3MK5"
#define RESUME_FILE_VERSION 1
#define RESUME_FILE_REVISION 1

#define KEYBIND_KEYS	130
#define KEYBIND_JOYS	24
#define KEYBIND_ASSIGN	2
#define KEYBIND_PRESETS	4

#define KEYBIND_JOY_BUTTONS	4

/// @ingroup Enums
/// @brief device masks of NMI signal
enum SIG_NMI_MASKS {
	SIG_NMI_TRACE_MASK		= 0x01,
	SIG_NMI_FD_MASK			= 0x04,
	SIG_NMI_KEYBREAK_MASK	= 0x08,
};

/// @ingroup Enums
/// @brief device masks of IRQ signal
enum SIG_IRQ_MASKS {
	SIG_IRQ_KEYBOARD_MASK	= 0x001,
	SIG_IRQ_LIGHTPEN_MASK	= 0x002,
	SIG_IRQ_PIAA_MASK		= 0x004,
	SIG_IRQ_PIAB_MASK		= 0x008,
	SIG_IRQ_EXPIAA_MASK		= 0x010,
	SIG_IRQ_EXPIAB_MASK		= 0x020,
	SIG_IRQ_ACIA_MASK		= 0x040,
	SIG_IRQ_EXACIA_MASK		= 0x080,
	SIG_IRQ_9PSG_MASK		= 0x100,
};

/// @ingroup Enums
/// @brief device masks of FIRQ signal
enum SIG_FIRQ_MASKS {
	SIG_FIRQ_TIMER1_MASK	= 0x01,
};

/// @ingroup Enums
/// @brief device masks of HALT signal
enum SIG_HALT_MASKS {
	SIG_HALT_FD_MASK		= 0x02,
};

//#define _FDC_DEBUG_LOG

#endif /* BML3MK5_DEFS_H */
