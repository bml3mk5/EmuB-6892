/** @file config.h

	Skelton for retropc emulator

	@author Takeda.Toshiya
	@date   2006.08.18 -

	@note
	Modified for BML3MK5 by Sasaji at 2011.06.17
	Modified for MBS1 by Sasaji at 2015.08.16

	@brief [ config ]

	@note
	Config ini file uses UTF-8 as string code.
	Under windows: convert to MBCS(shift jis) code from UTF-8.
	Under mac/linux: always uses UTF-8, so LANG environment should be set UTF-8. 
*/

#ifndef CONFIG_H
#define CONFIG_H

#include "common.h"
#include "vm/vm_defs.h"
#include "emu.h"
#include "cchar.h"
#include "cptrlist.h"
//#ifdef _UNICODE
//#define SI_CONVERT_GENERIC	// use generic simple.ini on all platform
//#endif
//#include "SimpleIni.h"
//#include "simple_ini.h"

class CSimpleIni;

#define FILE_VERSION	0x30

/// @ingroup Enums
/// @brief bit mask of Config::io_port
enum IOPORT_MASKS {
	IOPORT_MSK_5FDD		= 0x00001,	///< bit0: use fdd5
	IOPORT_MSK_3FDD		= 0x00002,	///< bit1: use fdd3
	IOPORT_MSK_PSG6		= 0x00004,	///< bit2: use 6voices psg
	IOPORT_MSK_EXPIA	= 0x00008,	///< bit3: use ex pia
	IOPORT_MSK_EXACIA	= 0x00010,	///< bit4: use ex acia
	IOPORT_MSK_PSG9		= 0x00020,	///< bit5: use 9voices psg
	IOPORT_MSK_KANJI	= 0x00040,	///< bit6: use kanji rom
	IOPORT_MSK_EXPSG	= 0x00080,	///< bit7: use s1 ex psg (CONFIG_VERSION >= 0x02)(for mbs1)
	IOPORT_MSK_FDDALL	= 0x00003,
	IOPORT_MSK_OS9BD	= 0x00100,	///< bit8: use OS/9 (for mbs1)
	IOPORT_MSK_CM01		= 0x00200,	///< bit9: use comm (for mbs1)
	IOPORT_MSK_KEYBD	= 0x00400,	///< bit10: use keyboard (for mbs1)
	IOPORT_MSK_MOUSE	= 0x00800,	///< bit11: use mouse (for mbs1)
	IOPORT_MSK_FMOPN	= 0x01000,	///< bit12: use fm opn (for mbs1)
	IOPORT_MSK_DISROMB	= 0x02000,	///< bit13: disable booting from rom basic (for mbs1)
	IOPORT_MSK_DISIG	= 0x04000,	///< bit14: disable ig ram
	IOPORT_MSK_RTC		= 0x08000,	///< bit15: use rtc
	IOPORT_MSK_Z80BCARD	= 0x10000,	///< bit16: use Z80B card (for mbs1)
	IOPORT_MSK_MPC68008	= 0x20000,	///< bit17: use 68008 card (for mbs1)
	IOPORT_MSK_KANJI2	= 0x40000,	///< bit18: use kanji rom (jis 2)
#if defined(_MBS1)
#if defined(USE_Z80B_CARD)
	IOPORT_MSK_ALL		= 0x5ffff,
#elif defined(USE_MPC_68008)
	IOPORT_MSK_ALL		= 0x6ffff,
#else
	IOPORT_MSK_ALL		= 0x4ffff,
#endif
#else
	IOPORT_MSK_ALL		= 0x0c07f,
#endif
};

/// @ingroup Macros
///@{
#define IOPORT_USE_5FDD		(pConfig->io_port & IOPORT_MSK_5FDD)
#define IOPORT_USE_3FDD		(pConfig->io_port & IOPORT_MSK_3FDD)
#define IOPORT_USE_PSG6		(pConfig->io_port & IOPORT_MSK_PSG6)
#define IOPORT_USE_EXPIA	(pConfig->io_port & IOPORT_MSK_EXPIA)
#define IOPORT_USE_EXACIA	(pConfig->io_port & IOPORT_MSK_EXACIA)
#define IOPORT_USE_PSG9		(pConfig->io_port & IOPORT_MSK_PSG9)
#define IOPORT_USE_KANJI	(pConfig->io_port & IOPORT_MSK_KANJI)
#define IOPORT_USE_EXPSG	(pConfig->io_port & IOPORT_MSK_EXPSG)		/* for mbs1 */
#define IOPORT_USE_FDD		(pConfig->io_port & IOPORT_MSK_FDDALL)
#define IOPORT_USE_OS9BD	(pConfig->io_port & IOPORT_MSK_OS9BD)		/* for mbs1 */
#define IOPORT_USE_CM01		(pConfig->io_port & IOPORT_MSK_CM01)		/* for mbs1 */
#define IOPORT_USE_KEYBD	(pConfig->io_port & IOPORT_MSK_KEYBD)		/* for mbs1 */
#define IOPORT_USE_MOUSE	(pConfig->io_port & IOPORT_MSK_MOUSE)		/* for mbs1 */
#define IOPORT_USE_FMOPN	(pConfig->io_port & IOPORT_MSK_FMOPN)		/* for mbs1 */
#define IOPORT_USE_DISROMB	(pConfig->io_port & IOPORT_MSK_DISROMB)	/* for mbs1 */
#define IOPORT_USE_DISIG	(pConfig->io_port & IOPORT_MSK_DISIG)
#define IOPORT_USE_RTC		(pConfig->io_port & IOPORT_MSK_RTC)
#define IOPORT_USE_Z80BCARD	(pConfig->io_port & IOPORT_MSK_Z80BCARD)	/* for mbs1 */
#define IOPORT_USE_MPC68008	(pConfig->io_port & IOPORT_MSK_MPC68008)	/* for mbs1 */
#define IOPORT_USE_KANJI2	(pConfig->io_port & IOPORT_MSK_KANJI2)
///@}

/// @ingroup Enums
/// @brief bit position of Config::io_port
enum IOPORT_POS {
	IOPORT_POS_5FDD = 0,
	IOPORT_POS_3FDD,
	IOPORT_POS_PSG6,
	IOPORT_POS_EXPIA,
	IOPORT_POS_EXACIA,
	IOPORT_POS_PSG9,
	IOPORT_POS_KANJI,
	IOPORT_POS_EXPSG,	/* for mbs1 */
	IOPORT_POS_OS9BD,	/* for mbs1 */
	IOPORT_POS_CM01,	/* for mbs1 */
	IOPORT_POS_KEYBD,	/* for mbs1 */
	IOPORT_POS_MOUSE,	/* for mbs1 */
	IOPORT_POS_FMOPN,	/* for mbs1 */
	IOPORT_POS_DISROMB,	/* for mbs1 */
	IOPORT_POS_DISIG,
	IOPORT_POS_RTC,
	IOPORT_POS_Z80BCARD, /* for mbs1 */
	IOPORT_POS_MPC68008, /* for mbs1 */
	IOPORT_POS_KANJI2,
	IOPORT_NUMS
};

/// @ingroup Enums
/// @brief bit mask of Config::misc_flags
enum MISC_FLAG_MASKS {
	MSK_SHOWLEDBOX		= 0x0001,	///< bit0: show led
	MSK_SHOWMSGBOARD	= 0x0002,	///< bit1: show msg
	MSK_USEJOYSTICK		= 0x0004,	///< bit2: use joystick
	MSK_INSIDELEDBOX	= 0x0008,	///< bit3: inside led
	MSK_LEDBOX_ALL		= 0x0009,
	MSK_USELIGHTPEN		= 0x0010,	///< bit4: enable lightpen
	MSK_USEMOUSE		= 0x0020,	///< bit5: enable mouse
	MSK_USEPIAJOYSTICK	= 0x0040,	///< bit6: use pia joystick
	MSK_USEJOYSTICK_ALL	= 0x0044,
	MSK_SHOWMSG_UNDEFOP	= 0x0080,	///< bit7: show msg when undef opcode
	MSK_SHOWMSG_ADDRERR	= 0x0100,	///< bit8: show msg when address error
	MSK_USEKEY2JOYSTICK	= 0x0400,	///< bit10: use key to joystick
	MSK_PIAJOY_ALL		= 0x0440,
	MSK_SHOWDLG_ALL		= 0x05ff,
	MSK_CLEAR_CPUREG	= 0x0800,	///< bit11: clear CPU registers at power on
	MSK_PIAJOY_NOIRQ	= 0x2000,	///< bit13: no irq when pressed buttons (for mbs1)
	MSK_PIAJOY_NEGATIVE = 0x4000,	///< bit14: pia joy signals are negative (for bml3mk5)
};

/// @ingroup Macros
///@{
#define FLG_SHOWLEDBOX		(pConfig->misc_flags & MSK_SHOWLEDBOX)
#define FLG_SHOWMSGBOARD	(pConfig->misc_flags & MSK_SHOWMSGBOARD)
#define FLG_USEJOYSTICK		(pConfig->misc_flags & MSK_USEJOYSTICK)
#define FLG_INSIDELEDBOX	(pConfig->misc_flags & MSK_INSIDELEDBOX)
#define FLG_LEDBOX_ALL		(pConfig->misc_flags & MSK_LEDBOX_ALL)
#define FLG_USELIGHTPEN		(pConfig->misc_flags & MSK_USELIGHTPEN)
#define FLG_USEMOUSE		(pConfig->misc_flags & MSK_USEMOUSE)
#define FLG_USEPIAJOYSTICK	(pConfig->misc_flags & MSK_USEPIAJOYSTICK)
#define FLG_USEJOYSTICK_ALL	(pConfig->misc_flags & MSK_USEJOYSTICK_ALL)
#define FLG_SHOWMSG_UNDEFOP	(pConfig->misc_flags & MSK_SHOWMSG_UNDEFOP)
#define FLG_SHOWMSG_ADDRERR	(pConfig->misc_flags & MSK_SHOWMSG_ADDRERR)
#define FLG_USEKEY2JOYSTICK (pConfig->misc_flags & MSK_USEKEY2JOYSTICK)
#define FLG_PIAJOY_ALL		(pConfig->misc_flags & MSK_PIAJOY_ALL)
#define FLG_CLEAR_CPUREG	(pConfig->misc_flags & MSK_CLEAR_CPUREG)
#define FLG_PIAJOY_NOIRQ	(pConfig->misc_flags & MSK_PIAJOY_NOIRQ)
#define FLG_PIAJOY_NEGATIVE	(pConfig->misc_flags & MSK_PIAJOY_NEGATIVE)
///@}

/// @ingroup Enums
/// @brief bit mask of Config::original
enum ORIGINAL_MASKS {
	MSK_ORIG_CURIG		= 0x001,	///< bit0: show cursor on IG
	MSK_ORIG_PAL8		= 0x002,	///< bit1: set dark gray on palette 8 (for mbs1)
	MSK_ORIG_NOPAL64	= 0x004,	///< bit2: not use palette in ANALOG 64 color mode (for mbs1)
	MSK_ORIG_NOPAL		= 0x008,	///< bit3: not use palette in ANALOG mode (for mbs1)
	MSK_ORIG_LIMKEY		= 0x010,	///< bit4: auto key release after few frames
	MSK_ORIG_FDMSK		= 0x020,	///< bit5: ff00 mask (for mbs1)
	MSK_ORIG_FDINSERT	= 0x040,	///< bit6: check fd insert when get track
	MSK_ORIG_FDDRQ		= 0x080,	///< bit7: ignore drq when sector is not found
	MSK_ORIG_MOUSEEG	= 0x100,	///< bit8: occur mouse IRQ when both edge (for mbs1)
};

/// @ingroup Macros
///@{
#define FLG_ORIG_CURIG		(pConfig->original & MSK_ORIG_CURIG)
#define FLG_ORIG_PAL8		(pConfig->original & MSK_ORIG_PAL8)		/* for mbs1 */
#define FLG_ORIG_NOPAL64	(pConfig->original & MSK_ORIG_NOPAL64)	/* for mbs1 */
#define FLG_ORIG_NOPAL		(pConfig->original & MSK_ORIG_NOPAL)		/* for mbs1 */
#define FLG_ORIG_MOUSEEG	(pConfig->original & MSK_ORIG_MOUSEEG)	/* for mbs1 */
#define FLG_ORIG_LIMKEY		(pConfig->original & MSK_ORIG_LIMKEY)
#define FLG_ORIG_FDMSK		(pConfig->original & MSK_ORIG_FDMSK)		/* for mbs1 */
#define FLG_ORIG_FDINSERT	(pConfig->original & MSK_ORIG_FDINSERT)
#define FLG_ORIG_FDDRQ		(pConfig->original & MSK_ORIG_FDDRQ)
///@}

/// @ingroup Enums
/// @brief bit mask of Config::option_fdd
enum OPTIONFDD_MASKS {
	MSK_DELAY_FDSEARCH	= 0x0001,	///< bit0: ignore delay by searching sector
	MSK_DELAY_FDSEEK	= 0x0002,	///< bit1: ignore delay by seeking track
	MSK_DELAY_FD_MASK	= 0x0003,	///< delay fdd flags mask
	MSK_DELAY_FD_SFT	= 0,		///< delay fdd flags shift
	MSK_CHECK_FDDENSITY	= 0x0010,	///< bit4: check a difference of density in a floppy disk
	MSK_CHECK_FDMEDIA	= 0x0020,	///< bit5: check a difference of media in a floppy disk
	MSK_CHECK_FD_MASK	= 0x0030,	///< check fd media flags mask
	MSK_CHECK_FD_SFT	= 4,		///< check fd media flags shift
	MSK_SAVE_FDPLAIN	= 0x0100,	///< bit8: save plain image as it is (default: save it as converted d88 image)
	MSK_SAVE_FD_MASK	= 0x0100,	///< save fd flags mask
	MSK_SAVE_FD_SFT		= 8,		///< save fd flags shift
};

/// @ingroup Macros
///@{
#define FLG_DELAY_FDSEARCH	(pConfig->option_fdd & MSK_DELAY_FDSEARCH)
#define FLG_DELAY_FDSEEK	(pConfig->option_fdd & MSK_DELAY_FDSEEK)
#define FLG_CHECK_FDDENSITY	(pConfig->option_fdd & MSK_CHECK_FDDENSITY)
#define FLG_CHECK_FDMEDIA	(pConfig->option_fdd & MSK_CHECK_FDMEDIA)
#define FLG_SAVE_FDPLAIN	(pConfig->option_fdd & MSK_SAVE_FDPLAIN)
///@}

/// @ingroup Enums
/// @brief bit mask of Config::option_hdd
enum OPTIONHDD_MASKS {
	MSK_DELAY_HDSEEK	= 0x0002,	///< bit1: ignore delay by seeking track
	MSK_DELAY_HD_MASK	= 0x0002,	///< delay hdd flags mask
	MSK_DELAY_HD_SFT	= 0,		///< delay hdd flags shift
};

/// @ingroup Macros
///@{
#define FLG_DELAY_HDSEEK	(pConfig->option_hdd & MSK_DELAY_HDSEEK)
///@}

/// @ingroup Enums
/// @brief value of Config::fdd_type
enum FDDTYPE_POS {
	FDD_TYPE_NOFDD		= 0,
	FDD_TYPE_3FDD		= 1,
	FDD_TYPE_5FDD		= 2,
	FDD_TYPE_58FDD		= 3,
};

/// @ingroup Enums
/// @brief mask of Config::fdd_type
enum FDDTYPE_MASKS {
	MSK_FDD_TYPE		= 0x3,
};

/// @ingroup Enums
/// @brief sound volumes
enum VOLUME_POS {
	VOLUME_MASTER = 0,
	VOLUME_BEEP,
#if defined(_MBS1)
	VOLUME_PSG,
	VOLUME_EXPSG_FM,
	VOLUME_EXPSG_SSG,
	VOLUME_EXPSG_ADPCM,
	VOLUME_EXPSG_RHYTHM,
	VOLUME_FMOPN_FM,
	VOLUME_FMOPN_SSG,
	VOLUME_FMOPN_ADPCM,
	VOLUME_FMOPN_RHYTHM,
#endif
	VOLUME_PSG6,
	VOLUME_PSG9,
	VOLUME_RELAY,
	VOLUME_CMT,
	VOLUME_FDD,
	VOLUME_NUMS
};

/// directory path
class CDirPath : public CTchar
{
public:
	void SetFromPath(const _TCHAR *file_path);
	void Set(const _TCHAR *dir_path);
private:
	void Set(const CDirPath &) {}
	void Set(const _TCHAR *, int) {}
//	CDirPath &operator=(const CTchar &src);
//	CDirPath &operator=(const _TCHAR *src_str);
};

/// file path
class CFilePath : public CTchar
{
public:
	void Set(const _TCHAR *file_path);
};

#define MAX_HISTORY	20

/// recent path
class CRecentPath
{
public:
	CTchar path;
	int    num;
public:
	CRecentPath();
	CRecentPath(const CRecentPath &src);
	CRecentPath(const _TCHAR *srcpath, int srcnum);
	~CRecentPath();
	void Set(const _TCHAR *srcpath, int srcnum);
	void Clear();
	bool Match(const _TCHAR *tagpath, int tagnum);
};

/// recent path list
class CRecentPathList : public CPtrList<CRecentPath>
{
public:
	bool updated;
public:
	CRecentPathList();
	~CRecentPathList();
	bool Match(const _TCHAR *tagpath, int tagnum);
	void Update(const _TCHAR *tagpath, int tagnum);
};

/// @brief Read/Write config file
///
///	@note
///	Config ini file uses UTF-8 as string code.
///	Under windows: convert to MBCS(shift jis) code from UTF-8.
///	Under mac/linux: always uses UTF-8, so LANG environment should be set UTF-8. 
class Config
{
public:
	int version1;	// config file version
	int version2;

protected:
	// recent files
#ifdef USE_FD1
	CDirPath initial_disk_path;
	CRecentPathList recent_disk_path[USE_FLOPPY_DISKS];
	CRecentPath opened_disk_path[USE_FLOPPY_DISKS];
#endif
#ifdef USE_HD1
	CDirPath initial_hard_disk_path;
	CRecentPathList recent_hard_disk_path[USE_HARD_DISKS];
	CRecentPath opened_hard_disk_path[USE_HARD_DISKS];
#endif
#ifdef USE_DATAREC
	CDirPath initial_datarec_path;
	CRecentPathList recent_datarec_path;
	CRecentPath opened_datarec_path;
	bool realmode_datarec;
#endif
#ifdef USE_CART1
	CDirPath initial_cart_path;
	CRecentPath recent_cart_path[1];
#endif
#ifdef USE_QD1
	CDirPath initial_quickdisk_path;
	CRecentPath recent_quickdisk_path[1];
#endif
#ifdef USE_MEDIA
	CDirPath initial_media_path;
	CRecentPath recent_media_path;
#endif
#ifdef USE_BINARY_FILE1
	CDirPath initial_binary_path;
	CRecentPath recent_binary_path[2];
#endif
#ifdef USE_STATE
	CDirPath initial_state_path;
	CRecentPath saved_state_path;
	CRecentPathList recent_state_path;
#endif
#ifdef USE_AUTO_KEY
	CDirPath initial_autokey_path;
	CRecentPath opened_autokey_path;
#endif
#ifdef USE_PRINTER
	CDirPath initial_printer_path;
#endif

public:
	// screen
	int window_mode;
	int window_position_x;
	int window_position_y;
	uint8_t stretch_screen;
//	bool cutout_screen;
	int pixel_aspect;
	uint8_t capture_type;

	// sound
	int sound_frequency;
	int sound_latency;

	// virtual machine
	int cpu_power;
	bool now_power_off;
	bool use_power_off;
#ifdef USE_FD1
	bool ignore_crc;
	int  mount_fdd;
#endif
#ifdef USE_HD1
	int  mount_hdd;
#endif
#ifdef USE_DIPSWITCH
	uint8_t dipswitch;
#endif
#ifdef USE_BOOT_MODE
	int boot_mode;		// MZ-800, PASOPIA, PC-8801MA, PC-98DO
#endif
#ifdef USE_CPU_CLOCK_LOW
	bool cpu_clock_low;	// PC-8801MA, PC-9801E, PC-9801VM, PC-98DO
#endif
#if defined(_HC80) || defined(_PASOPIA) || defined(_PC8801MA)
	int device_type;
#endif
#if defined(USE_MONITOR_TYPE) || defined(USE_SCREEN_ROTATE)
	int monitor_type;
#endif
#ifdef USE_SCANLINE
	uint8_t scan_line;
#endif
#if defined(_MBS1)
	uint8_t sys_mode;
	bool mem_nowait;
	uint8_t tvsuper;
#endif
	bool sync_irq;
#ifdef USE_AFTERIMAGE
	int  afterimage;
#endif
#ifdef USE_KEEPIMAGE
	int  keepimage;
#endif
#ifdef USE_DIRECT3D
	uint8_t use_direct3d;
	uint8_t d3d_filter_type;
#endif
#ifdef USE_OPENGL
	uint8_t use_opengl;
	uint8_t gl_filter_type;
#endif

#if defined(_BML3MK5) || defined(_MBS1)
	uint8_t exram_size_num;
	int8_t  disptmg_skew;
	int8_t  curdisp_skew;

	int fdd_type;

	uint8_t rloss[2];
	uint8_t gloss[2];
	uint8_t bloss[2];
#endif

	// CONFIG_VERSION >= 0x05 on bml3mk5
	// bit0: use fdd5  bit1: use fdd3  bit2: use psg  bit3: use ex pia
	// bit4: use ex acia  bit5: use 9psg  bit6: use kanji
	// bit7: use s1 ex psg (CONFIG_VERSION >= 0x02 on mbs1)
	// bit8: use OS/9  bit9: use comm  bit10: use keyboard  bit11: use mouse (on mbs1)
	// bit12: use fm opn (on mbs1)
	// bit13: disable booting from rom basic
	// bit14: disable ig ram
	// bit15: use rtc
	// bit16: use Z80B card
	// bit17: use 68008 card
	// bit18: use kanji jis2
	int io_port;
	// bit0: show led  bit1: show msg  bit2: use joystick  bit3: inside led
	// bit4: enable lightpen bit5: enable mouse bit6: use pia joystick
	// bit7: show msg when undef opcode  bit8: show msg when address error
	// bit10: use key 2 joystick
	// bit11: clear CPU registers at power on
	int misc_flags;
	// bit0: show cursor on IG  bit1: set dark gray on palette 8 (on mbs1)
	// bit2: not use palette in ANALOG 64 color mode  bit3: not use palette in ANALOG mode (on mbs1)
	// bit4: auto key release after few frames
	// bit5: ff00 mask (on mbs1)
	// bit6: check fd insert when get track  bit7: ignore drq when sector is not found
	// bit8: occur mouse IRQ when both edge (on mbs1)
	int original;

#ifdef USE_FD1
	// bit0: ignore delay by searching sector
	// bit1: ignore delay by seeking track
	// bit4: check a difference of density in a floppy disk
	// bit5: check a difference of media in a floppy disk
	// bit8: save plain image as it is (default: save it as converted d88 image)
	int option_fdd;
#endif
#ifdef USE_HD1
	// bit1: ignore delay by seeking track
	int option_hdd;
#endif

	int fps_no;
	uint8_t screen_video_size;

	int disp_device_no;
	int screen_width;
	int screen_height;

#ifdef USE_MESSAGE_BOARD
	CTchar msgboard_info_fontname;
	CTchar msgboard_msg_fontname;
	uint8_t msgboard_info_fontsize;
	uint8_t msgboard_msg_fontsize;
#endif

	CDirPath rom_path;

	int volume;
	bool mute;
#ifdef USE_FD1
	int fdd_volume;
	bool fdd_mute;
#endif
#ifdef USE_HD1
	int hdd_volume;
	bool hdd_mute;
#endif

	//
#if defined(GUI_TYPE_AGAR)
	CTchar menu_fontname;
	uint8_t menu_fontsize;
#endif

#if defined(_BML3MK5) || defined(_MBS1)
	int beep_volume;
	int psg6_volume;
	int psg9_volume;
	int relay_volume;
	int cmt_volume;
	bool beep_mute;
	bool psg6_mute;
	bool psg9_mute;
	bool relay_mute;
	bool cmt_mute;
#ifdef _MBS1
	int psg_volume;
	int psgexfm_volume;
	int psgexssg_volume;
	int psgexpcm_volume;
	int psgexrhy_volume;
	int opnfm_volume;
	int opnssg_volume;
	int opnpcm_volume;
	int opnrhy_volume;
	bool psg_mute;
	bool psgexfm_mute;
	bool psgexssg_mute;
	bool psgexpcm_mute;
	bool psgexrhy_mute;
	bool opnfm_mute;
	bool opnssg_mute;
	bool opnpcm_mute;
	bool opnrhy_mute;

//	int opn_clock;
	int opn_irq;

	// type of sound chip
	enum en_chip_type_of_sound {
		CHIP_AY38910 = 0,
		CHIP_YM2203,
		CHIP_YM2608
	};
	struct st_type_of_sound {
		int chip_type;
		int clock;
	};
	enum en_type_of_sound {
		CHIP_AY38910_1MHZ,	// 1MHz
		CHIP_AY38910_2MHZ,	// 2MHz
		CHIP_YM2203_2MHZ,	// 2MHz
		CHIP_YM2203_3MHZ,	// 3.58MHz
		CHIP_YM2203_4MHZ,	// 4MHz
		CHIP_YM2608_4MHZ,	// 4MHz
		CHIP_YM2608_8MHZ,	// 8MHz
		END_TYPE_OF_SOUND
	};
	static const struct st_type_of_sound c_type_of_sound[];

	// type of chip on fm synth card
	int type_of_fmopn;
	// clock of fm synth chip on fm synth card
	int clock_of_fmopn;
	// type of chip on external psg port
	int type_of_expsg;
	// clock of fm synth chip on external psg port
	int clock_of_expsg;
//	int use_opn_expsg;
#endif
#endif

#ifdef MAX_PRINTER
	CTchar printer_server_host[MAX_PRINTER];
	int    printer_server_port[MAX_PRINTER];
	bool   printer_direct[MAX_PRINTER];
	bool   printer_online[MAX_PRINTER];
	double printer_delay[MAX_PRINTER];
#endif

#ifdef MAX_COMM
	int    comm_dipswitch[MAX_COMM];
	CTchar comm_server_host[MAX_COMM];
	int    comm_server_port[MAX_COMM];
	bool   comm_server[MAX_COMM];
//	bool   comm_connect[MAX_COMM];
	bool   comm_through[MAX_COMM];
	bool   comm_binary[MAX_COMM];
#endif

#ifdef USE_UART
	int    comm_uart_baudrate;
	int    comm_uart_databit;
	int    comm_uart_parity;
	int    comm_uart_stopbit;
	int    comm_uart_flowctrl;
#endif

	CDirPath snapshot_path;

#ifdef USE_LEDBOX
	uint8_t led_pos;
	VmPoint led_dist[2];
#endif

#if defined(USE_WIN)
	CFilePath font_path;
#endif
#if defined(USE_SDL) || defined(USE_SDL2) || defined(USE_WX) || defined(USE_WX2)
	CDirPath font_path;
#endif

	bool reckey_recording;
	bool reckey_playing;

#ifdef USE_DATAREC
	bool wav_reverse;
	bool wav_half;
	bool wav_correct;
	uint8_t wav_correct_type;
	int wav_correct_amp[2];

	uint8_t wav_sample_rate;
	uint8_t wav_sample_bits;
#endif

#ifdef USE_SOUND_DEVICE_TYPE
	int sound_device_type;
#endif

#ifdef USE_DIRECTINPUT
	enum en_direct_input_flags {
		DIRECTINPUT_ENABLE = 0x01,
		DIRECTINPUT_AVAIL = 0x04,
	};
	// bit0:enable/disable bit2:available
	uint8_t use_direct_input;
#endif

#ifdef USE_DEBUGGER
	int    debugger_imm_start;
	CTchar debugger_server_host;
	int    debugger_server_port;
#endif

#if defined(USE_JOYSTICK) || defined(USE_KEY2JOYSTICK)
	int    joy_mashing[MAX_JOYSTICKS][KEYBIND_JOY_BUTTONS];
	int    joy_axis_threshold[MAX_JOYSTICKS][6];
#endif
#ifdef USE_PIAJOYSTICKBIT
	int    piajoy_conn_to;
#endif

#ifdef USE_PERFORMANCE_METER
	bool   show_pmeter;
#endif

	CTchar language;

#if defined(_MBS1) && defined(USE_Z80B_CARD)
	int    z80b_card_out_irq;
#endif

public:
	Config();
	~Config();

	void initialize();
	void load(const _TCHAR *path);
	void save();
	void release();

	static bool get_number_in_path(_TCHAR *path, int *number);
	static bool set_number_in_path(_TCHAR *path, size_t size, int number);

#ifdef USE_DATAREC
	const _TCHAR *GetInitialDataRecPath() const;
	const _TCHAR *GetInitialDataRecPathForWinGUI();
	void SetInitialDataRecPathFrom(const _TCHAR *path);
	CRecentPathList &GetRecentDataRecPathList();
	int GetRecentDataRecPathCount() const;
	int GetRecentDataRecPathLength(int pos) const;
	const _TCHAR *GetRecentDataRecPathString(int pos) const;
	int GetRecentDataRecPathNumber(int pos) const;
	void UpdateRecentDataRecPath(const _TCHAR *path, int num);
	bool UpdatedRecentDataRecPath() const;
	void RecentDataRecPathUpdated(bool val);
	CRecentPath &GetOpenedDataRecPath();
	const _TCHAR *GetOpenedDataRecPathString() const;
	int GetOpenedDataRecPathNumber() const;
	void SetOpenedDataRecPath(const _TCHAR *path, int num);
	void SetNewOpenedDataRecPath(const _TCHAR *path, int num);
	void ClearOpenedDataRecPath();
	bool NowRealModeDataRec() const;
	void SetRealModeDataRec(bool val);
#endif
#ifdef USE_FD1
	const _TCHAR *GetInitialFloppyDiskPath() const;
	const _TCHAR *GetInitialFloppyDiskPathForWinGUI();
	void SetInitialFloppyDiskPathFrom(const _TCHAR *path);
	CRecentPathList &GetRecentFloppyDiskPathList(int drv);
	int GetRecentFloppyDiskPathCount(int drv) const;
	int GetRecentFloppyDiskPathLength(int drv, int pos) const;
	const _TCHAR *GetRecentFloppyDiskPathString(int drv, int pos) const;
	int GetRecentFloppyDiskPathNumber(int drv, int pos) const;
	void UpdateRecentFloppyDiskPath(int drv, const _TCHAR *path, int num);
	bool UpdatedRecentFloppyDiskPath(int drv) const;
	void RecentFloppyDiskPathUpdated(int drv, bool val);
	CRecentPath &GetOpenedFloppyDiskPath(int drv);
	const _TCHAR *GetOpenedFloppyDiskPathString(int drv) const;
	int GetOpenedFloppyDiskPathNumber(int drv) const;
	void SetOpenedFloppyDiskPath(int drv, const _TCHAR *path, int num);
	void SetNewOpenedFloppyDiskPath(int drv, const _TCHAR *path, int num);
	void ClearOpenedFloppyDiskPath(int drv);
#endif
#ifdef USE_HD1
	const _TCHAR *GetInitialHardDiskPath() const;
	const _TCHAR *GetInitialHardDiskPathForWinGUI();
	void SetInitialHardDiskPathFrom(const _TCHAR *path);
	CRecentPathList &GetRecentHardDiskPathList(int drv);
	int GetRecentHardDiskPathCount(int drv) const;
	int GetRecentHardDiskPathLength(int drv, int pos) const;
	const _TCHAR *GetRecentHardDiskPathString(int drv, int pos) const;
	int GetRecentHardDiskPathNumber(int drv, int pos) const;
	void UpdateRecentHardDiskPath(int drv, const _TCHAR *path, int num);
	CRecentPath &GetOpenedHardDiskPath(int drv);
	const _TCHAR *GetOpenedHardDiskPathString(int drv) const;
	int GetOpenedHardDiskPathNumber(int drv) const;
	void SetOpenedHardDiskPath(int drv, const _TCHAR *path, int num);
	void SetNewOpenedHardDiskPath(int drv, const _TCHAR *path, int num);
	void ClearOpenedHardDiskPath(int drv);
	int GetHardDiskDeviceType(int drv) const;
	void SetHardDiskDeviceType(int drv, int num);
	int GetHardDiskIndex(int drv) const;
#endif
#ifdef USE_STATE
	const _TCHAR *GetInitialStatePath() const;
	const _TCHAR *GetInitialStatePathForWinGUI();
	void SetInitialStatePathFrom(const _TCHAR *path);
	CRecentPathList &GetRecentStatePathList();
	int GetRecentStatePathCount() const;
	int GetRecentStatePathLength(int pos) const;
	const _TCHAR *GetRecentStatePathString(int pos) const;
	int GetRecentStatePathNumber(int pos) const;
	void UpdateRecentStatePath(const _TCHAR *path, int num);
	bool UpdatedRecentStatePath() const;
	void RecentStatePathUpdated(bool val);
	CRecentPath &GetSavedStatePath();
	const _TCHAR *GetSavedStatePathString() const;
	int GetSavedStatePathNumber() const;
	void SetSavedStatePath(const _TCHAR *path, int num);
	void SetNewSavedStatePath(const _TCHAR *path, int num);
	void ClearSavedStatePath();
#endif
#ifdef USE_AUTO_KEY
	const _TCHAR *GetInitialAutoKeyPath() const;
	const _TCHAR *GetInitialAutoKeyPathForWinGUI();
	void SetInitialAutoKeyPathFrom(const _TCHAR *path);
	CRecentPath &GetOpenedAutoKeyPath();
	const _TCHAR *GetOpenedAutoKeyPathString() const;
	int GetOpenedAutoKeyPathNumber() const;
	void SetOpenedAutoKeyPath(const _TCHAR *path, int num);
	void SetNewOpenedAutoKeyPath(const _TCHAR *path, int num);
	void ClearOpenedAutoKeyPath();
#endif
#ifdef USE_PRINTER
	const _TCHAR *GetInitialPrinterPath() const;
	const _TCHAR *GetInitialPrinterPathForWinGUI();
	void SetInitialPrinterPathFrom(const _TCHAR *path);
#endif

private:
	CSimpleIni *ini;

	CTchar ini_file;

	bool load_ini_file(const _TCHAR *file);
	void save_ini_file(const _TCHAR *file);

	const _TCHAR *conv_to_npath(const _TCHAR *path);
	const _TCHAR *conv_from_npath(const _TCHAR *npath);
	void get_str_value(const _TCHAR *section, const _TCHAR *key, CTchar &str);
	void get_dirpath_value(const _TCHAR *section, const _TCHAR *key, CDirPath &path);
	void get_filepath_value(const _TCHAR *section, const _TCHAR *key, CFilePath &path);
	void get_recentpath_value(const _TCHAR *section, const _TCHAR *key, CRecentPathList &pathlist);

	static int conv_volume(int);
};

extern Config *pConfig;

#endif /* CONFIG_H */

