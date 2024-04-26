/** @file labels.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2021.04.18 -

	@brief [ labels ]
*/

#include "labels.h"
#include "config.h"
#include "rec_video_defs.h"
#include "vm/vm_defs.h"

namespace LABELS {

/// Tab titles on configure dialog
const CMsg::Id tabs[] = {
	CMsg::Mode,
	CMsg::Screen,
	CMsg::Tape_FDD,
	CMsg::Network,
	CMsg::CPU_Memory,
#if defined(_MBS1)
	CMsg::Sound,
#endif
	CMsg::End
};

/// I/O port list
const CMsg::Id io_port[] = { 
#if defined(_MBS1)
//	CMsg::FDC5__FF00,
//	CMsg::FDC3L_FF18,
	CMsg::PSG6L_FF30,
	CMsg::ExLPT_FF3C,
	CMsg::ExCOM_FF40,
	CMsg::PSG9L_FF70,
	CMsg::KANJI_FF75,
	CMsg::ExPSG_FFE6,
	CMsg::OS9Ex_FE11,
	CMsg::Comm__FF77,
	CMsg::Keybd_FFE0,
	CMsg::Mouse_FFDC,
	CMsg::FMSyn_FF1E,
	CMsg::DisROM_FFCE,
	CMsg::DisIGL_FFE9,
	CMsg::RTC___FF38,
	CMsg::Z80BCD_FF7F,
	CMsg::MPC68008_FE1A,
#else
//	CMsg::FDC5__FF00,
//	CMsg::FDC3__FF18,
	CMsg::PSG6__FF30,
	CMsg::ExLPT_FF3C,
	CMsg::ExCOM_FF40,
	CMsg::PSG9__FF70,
	CMsg::KANJI_FF75,
	CMsg::DisIG_FFE9,
	CMsg::RTC___FF38,
#endif
	CMsg::End
};

/// I/O port bit position
const uint8_t io_port_pos[] = {
#if defined(_MBS1)
	IOPORT_POS_PSG6,
	IOPORT_POS_EXPIA,
	IOPORT_POS_EXACIA,
	IOPORT_POS_PSG9,
	IOPORT_POS_KANJI,
	IOPORT_POS_EXPSG,
	IOPORT_POS_OS9BD,
	IOPORT_POS_CM01,
	IOPORT_POS_KEYBD,
	IOPORT_POS_MOUSE,
	IOPORT_POS_FMOPN,
	IOPORT_POS_DISROMB,
	IOPORT_POS_DISIG,
	IOPORT_POS_RTC,
	IOPORT_POS_Z80BCARD,
	IOPORT_POS_MPC68008,
#else
	IOPORT_POS_PSG6,
	IOPORT_POS_EXPIA,
	IOPORT_POS_EXACIA,
	IOPORT_POS_PSG9,
	IOPORT_POS_KANJI,
	IOPORT_POS_DISIG,
	IOPORT_POS_RTC,
#endif
	0
};

#if defined(_MBS1)
/// system mode
const CMsg::Id sys_mode[] = {
	CMsg::A_Mode_S1,CMsg::B_Mode_L3,CMsg::End
};
#endif

/// floppy type
const CMsg::Id fdd_type[] = {
	CMsg::Non_FDD,
#if defined(_MBS1)
	CMsg::FD3inch_compact_FDD_L3,
	CMsg::FD5inch_mini_FDD_2D_Type,
	CMsg::FD5inch_mini_FDD_2HD_Type,
#else
	CMsg::FD3inch_compact_FDD,
	CMsg::FD5inch_mini_FDD,
	CMsg::FD8inch_standard_FDD,
#endif
	CMsg::End
};

/// correct wave on loading
const CMsg::Id correct[] = {
	CMsg::None_,
	CMsg::COS_Wave,
	CMsg::SIN_Wave,
	CMsg::End
};

/// correct amplify
const _TCHAR *correct_amp[] = {
	_T("1200Hz"), _T("2400Hz"), NULL
};

/// sampling rate
const _TCHAR *wav_sampling_rate[] = {
	_T("11025"), _T("22050"), _T("44100"), _T("48000"), NULL
};
/// sample bits
const _TCHAR *wav_sampling_bits[] = {
	_T("8"), _T("16"), NULL
};

/// display skew
const _TCHAR *disp_skew[] = {
#if defined(_MBS1)
	_T("-2"), _T("-1"), _T("0"), _T("1"), _T("2"), NULL
#else
	_T("0"), _T("1"), _T("2"), NULL
#endif
};

#ifdef USE_DIRECT3D
/// drawing
const CMsg::Id d3d_use[] = {
	CMsg::Default, CMsg::Use_Direct3D_Sync, CMsg::Use_Direct3D_Async, CMsg::End
};
/// filter
const CMsg::Id d3d_filter[] = {
	CMsg::None_, CMsg::Point, CMsg::Linear, CMsg::End
};
#endif
#ifdef USE_OPENGL
/// drawing
const CMsg::Id opengl_use[] = {
	CMsg::Default, CMsg::Use_OpenGL_Sync, CMsg::Use_OpenGL_Async, CMsg::End
};
/// filter
const CMsg::Id opengl_filter[] = {
	CMsg::Nearest_Neighbour, CMsg::Linear, CMsg::End
};
#endif

/// show led box
const CMsg::Id led_show[] = {
	CMsg::Hide, CMsg::Show_Inside, CMsg::Show_Outside, CMsg::End
};
/// led box position
const CMsg::Id led_pos[] = {
	CMsg::LeftTop, CMsg::RightTop, CMsg::LeftBottom, CMsg::RightBottom, CMsg::End
};

/// format on capture
const _TCHAR *capture_fmt[] = {
	_T("BMP"),
#ifdef USE_CAPTURE_SCREEN_PNG
	_T("PNG"),
#endif
	NULL
};

/// sound samples
const CMsg::Id sound_samples[] = {
	CMsg::F8000Hz,
	CMsg::F11025Hz,
	CMsg::F22050Hz,
	CMsg::F44100Hz,
	CMsg::F48000Hz,
	CMsg::F96000Hz,
	CMsg::End
};

/// sound late
const CMsg::Id sound_late[] = {
	CMsg::S50msec,
	CMsg::S75msec,
	CMsg::S100msec,
	CMsg::S200msec,
	CMsg::S300msec,
	CMsg::S400msec,
	CMsg::End
};

/// baud rate
const CMsg::Id comm_baud[] = {
	CMsg::S_300baud_F_1200baud, CMsg::S_600baud_F_2400baud, CMsg::S_1200baud_F_4800baud, CMsg::S_2400baud_F_9600baud, CMsg::End
};

/// UART baud rate
const  _TCHAR *comm_uart_baudrate[] = {
	_T("110"), _T("300"), _T("600"), _T("1200"), _T("2400"), _T("4800"), _T("9600"),
	_T("19200"), _T("38400"), _T("57600"),	_T("115200"),
	_T("230400"), _T("460800"), _T("921600"),
	NULL
};

/// UART data bit
const _TCHAR *comm_uart_databit[] = {
	_T("7"), _T("8"), NULL
};

/// UART parity
const CMsg::Id comm_uart_parity[] = {
	CMsg::None_, CMsg::Odd, CMsg::Even, CMsg::End
};

/// UART stop bit
const _TCHAR *comm_uart_stopbit[] = {
	_T("1"), _T("2"), NULL
};

/// UART flow control
const CMsg::Id comm_uart_flowctrl[] = {
	CMsg::None_, CMsg::Xon_Xoff, CMsg::Hardware, CMsg::End
};

#if defined(_MBS1)
/// extended memory size
const CMsg::Id exram_size[] = {
	CMsg::None_, CMsg::S64KB, CMsg::S128KB, CMsg::S256KB, CMsg::S512KB, CMsg::End
};
/// FM synth clock
const _TCHAR *fmopn_clock[] = {
	_T("3,579,545"), _T("1,008,000"), _T("2,016,000"), _T("4,032,000"), NULL
};
/// FM synth interrupt
const CMsg::Id fmopn_irq[] = {
	CMsg::None_, CMsg::IRQ, CMsg::FIRQ, CMsg::End
};
/// Chip type on Extended PSG port
//const CMsg::Id expsg_type[] = {
//	CMsg::PSG_like_AY_3_8910, CMsg::OPN_like_YM2203, CMsg::OPNA_like_YM2608, CMsg::End
//};
/// Chip type of Sound Card and Extended PSG port
const CMsg::Id type_of_soundcard[] = {
	CMsg::PSG_like_AY_3_8910_1MHz, CMsg::PSG_like_AY_3_8910_2MHz,
	CMsg::OPN_like_YM2203_2MHz, CMsg::OPN_like_YM2203_3MHz,	CMsg::OPN_like_YM2203_4MHz,
	CMsg::OPNA_like_YM2608_4MHz, CMsg::OPNA_like_YM2608_8MHz,
	CMsg::End
};
#if defined(USE_Z80B_CARD)
/// Z80B interrupt
const CMsg::Id z80bcard_irq[] = {
	CMsg::IRQ, CMsg::NMI, CMsg::End
};
#endif
#endif

/// extension of a data recorder image
const char *datarec_exts = "l3;l3b;l3c;wav;t9x";

/// extension of a floppy disk image
const char *floppy_disk_exts = "d88;dsk;img;2d;2hd";
const char *blank_floppy_disk_exts = "d88";

/// extension of a hard disk image
const char *hard_disk_exts = "hdf";
const char *blank_hard_disk_exts = "hdf";

/// extension of a state file
const char *state_file_exts = "l3r";

/// extension of a key recording file
const char *key_rec_file_exts = "l3k";

/// extension of a auto keying file
const char *autokey_file_exts = "txt;bas;lpt";

/// extension of a printing file
const char *printing_file_exts = "lpt;txt;bas";

	
/// Volume labels
const CMsg::Id volume[] = {
	CMsg::Master,
    CMsg::Beep,
#if defined(_MBS1)
    CMsg::PSG,
	CMsg::ExPSG_CR_FM,
    CMsg::ExPSG_CR_SSG,
    CMsg::ExPSG_CR_ADPCM,
    CMsg::ExPSG_CR_Rhythm,
    CMsg::OPN_CR_FM,
    CMsg::OPN_CR_SSG,
    CMsg::OPNA_CR_ADPCM,
    CMsg::OPNA_CR_Rhythm,
	CMsg::Null,		// wrap position
#endif
    CMsg::PSG6_CR,
    CMsg::PSG9_CR,
    CMsg::Relay,
    CMsg::CMT,
    CMsg::FDD,
	CMsg::End
};

/// Keybind labels
const CMsg::Id keybind_col[][2] = {
#ifdef _MBS1
	// _MBS1
	{ CMsg::S1_Key,		CMsg::BindVDIGIT },
#ifdef USE_JOYSTICK
	{ CMsg::S1_Key,		CMsg::JoypadVDIGIT },
#ifdef USE_PIAJOYSTICK
	{ CMsg::PIA_on_S1,	CMsg::JoypadVDIGIT },
#endif
#endif
#ifdef USE_KEY2JOYSTICK
	{ CMsg::PIA_on_S1,  CMsg::BindVDIGIT },
#endif
#else
	// _BML3MK5
	{ CMsg::Level3_Key,	CMsg::BindVDIGIT },
#ifdef USE_JOYSTICK
	{ CMsg::Level3_Key,	CMsg::JoypadVDIGIT },
#ifdef USE_PIAJOYSTICK
	{ CMsg::PIA_on_L3,	CMsg::JoypadVDIGIT },
#endif
#endif
#ifdef USE_KEY2JOYSTICK
	{ CMsg::PIA_on_L3,  CMsg::BindVDIGIT },
#endif
#endif
	{ CMsg::End, CMsg::End }
};

/// Keybind tabs
const CMsg::Id keybind_tab[] = {
	CMsg::Keyboard,
#ifdef USE_JOYSTICK
	CMsg::Joypad_Key_Assigned,
#ifdef USE_PIAJOYSTICK
	CMsg::Joypad_PIA_Type,
#endif
#endif
#ifdef USE_KEY2JOYSTICK
	CMsg::Key_to_Joypad,
#endif
	CMsg::End
};

/// Keybind buttons
const CMsg::Id keybind_btn[] = {
	CMsg::Load_Default,
	CMsg::Null,		// separate space
	CMsg::Load_Preset_1,
	CMsg::Load_Preset_2,
	CMsg::Load_Preset_3,
	CMsg::Load_Preset_4,
	CMsg::Null,		// separate space
	CMsg::Save_Preset_1,
	CMsg::Save_Preset_2,
	CMsg::Save_Preset_3,
	CMsg::Save_Preset_4,
	CMsg::End
};

/// Keybind options
const CMsg::Id keybind_combi[] = {
	CMsg::Null,
#ifdef USE_JOYSTICK
	CMsg::Recognize_as_another_key_when_pressed_two_buttons,
#ifdef USE_PIAJOYSTICK
# ifndef USE_PIAJOYSTICKBIT
	CMsg::Null,
# else
	CMsg::Signals_are_negative_logic,
# endif
#endif
#endif
#ifdef USE_KEY2JOYSTICK
	CMsg::Null,
#endif
	CMsg::End
};

/// Joypad axis
const CMsg::Id joypad_axis[] = {
	CMsg::X_axis,
	CMsg::Y_axis,
	CMsg::Z_axis,
	CMsg::R_axis,
	CMsg::U_axis,
	CMsg::V_axis,
	CMsg::End
};

}; /* namespace LABELS */
