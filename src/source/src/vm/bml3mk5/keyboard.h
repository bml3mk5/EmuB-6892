/** @file keyboard.h

	HITACHI BASIC MASTER LEVEL3 Mark5 Emulator 'EmuB-6892'
	Skelton for retropc emulator

	@author Sasaji
	@date   2011.06.08 -

	@brief [ keyboard ]
*/

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "../vm_defs.h"
#include "../../emu.h"
#include "../device.h"
//#include "../../config.h"
//#include "keyrecord.h"
#include "../../osd/keybind.h"

#ifdef _DEBUG
/* #define _DEBUG_KEYBOARD */
#endif

#define ADDR_KB_NMI		0xffc8
#define ADDR_LP_FLG		0xffcb
#define ADDR_KEYBOARD	0xffe0
#define ADDR_LPEN_BL	0xffd5

class EMU;
class KEYRECORD;
class CSimpleIni;

/**
	@brief keyboard
*/
class KEYBOARD : public DEVICE
{
public:
	/// @brief signals on KEYBOARD
	enum SIG_KEYBOARD_IDS {
		SIG_KEYBOARD_HSYNC		= 1,
		SIG_KEYBOARD_PIA_PA		= 2,
		SIG_KEYBOARD_PIA_PB		= 3
	};

private:
	DEVICE *d_cpu,*d_disp,*d_board;
#ifdef USE_KEY_RECORD
	KEYRECORD *reckey;
#endif
	DEVICE *d_pia;
	DEVICE *d_pia_ex2;
	DEVICE *d_psg;

	uint8_t* p_key_stat;
	uint8_t  m_scan_code;
	uint8_t  m_key_scan_code;
//	uint8_t  m_key_native_code;
	uint8_t  m_key_pressed;

	uint8_t kb_mode;
	uint8_t kb_nmi;	// break key

	// light pen
	int*	p_mouse_stat;
	uint8_t	lpen_flg;
	uint8_t	lpen_flg_prev;
	bool	lpen_bl;

#if defined(USE_JOYSTICK) || defined(USE_KEY2JOYSTICK)
	uint32_t *p_joy_stat[MAX_JOYSTICKS];
	uint32_t *p_joy_real_stat[MAX_JOYSTICKS];
#endif
#if defined(USE_PIAJOYSTICK) || defined(USE_KEY2PIAJOYSTICK)
	int		joy_pia_sel;
	uint8_t joy_pia[MAX_JOYSTICKS];
#endif
#if defined(USE_PSGJOYSTICK) || defined(USE_KEY2PSGJOYSTICK)
	int		joy_psg_sel;
	uint8_t joy_psg[MAX_JOYSTICKS];
#endif

	int	m_counter;	// keyboard counter
	int remain_count;
	int remain_count_max;

	int  *p_key_mod;
	bool m_pause_pressed;
	bool m_altkey_pressed;
	bool m_modesw_pressed;
	bool m_powersw_pressed;

	uint8_t vm_key_stat[KEYBIND_KEYS];

#ifdef _DEBUG_KEYBOARD
	int frame_counter;
	int event_counter;
#endif

	// keybind key -> key
	uint32_key_assign_t scan2key_map[KEYBIND_KEYS];
	uint32_key_assign_t scan2key_preset_map[KEYBIND_PRESETS][KEYBIND_KEYS];
	// keybind joy -> key
	uint32_key_assign_t joy2key_map[KEYBIND_KEYS];
	uint32_key_assign_t joy2key_preset_map[KEYBIND_PRESETS][KEYBIND_KEYS];
#ifdef USE_PIAJOYSTICK
	// keybind joy -> joy pia
	uint32_key_assign_t sjoy2joya_map[KEYBIND_JOYS];
	uint32_key_assign_t sjoy2joya_preset_map[KEYBIND_PRESETS][KEYBIND_JOYS];
# ifdef USE_KEY2PIAJOYSTICK
	// keybind key -> joy pia
	uint32_key_assign_t scan2joya_map[KEYBIND_JOYS];
	uint32_key_assign_t scan2joya_preset_map[KEYBIND_PRESETS][KEYBIND_JOYS];
# endif
#endif
#ifdef USE_PSGJOYSTICK
	// keybind joy -> joy psg
	uint32_key_assign_t sjoy2joyb_map[KEYBIND_JOYS];
	uint32_key_assign_t sjoy2joyb_preset_map[KEYBIND_PRESETS][KEYBIND_JOYS];
# ifdef USE_KEY2PIAJOYSTICK
	// keybind key -> joy psg
	uint32_key_assign_t scan2joyb_map[KEYBIND_JOYS];
	uint32_key_assign_t scan2joyb_preset_map[KEYBIND_PRESETS][KEYBIND_JOYS];
# endif
#endif

	//for resume
#pragma pack(1)
	struct vm_state_st {
		uint8_t kb_mode;
		uint8_t kb_nmi;
		uint8_t lpen_flg;
		uint8_t lpen_flg_prev;
		uint8_t lpen_bl;

		uint8_t key_pressed;

		char  reserved1[2];

		int   counter;
		int   remain_count;
	};
#pragma pack()

	void clear_scan_code(int);
	void update_scan_code(int);
	void update_special_key();
#ifdef USE_LIGHTPEN
	void update_light_pen();
#endif
	void update_keyboard();
	void reset_joy_pia();
	void update_joy_pia();
	void reset_joy_psg();
	void update_joy_psg();

	inline bool pressing_key(int);

	bool load_cfg_file();
	void save_cfg_file();
	bool load_ini_file();
	void save_ini_file();
	void convert_map();
	static void load_ini_file_one(CSimpleIni *ini, const _TCHAR *section_name, int rows, uint32_t *map, uint32_t *preset);
	static void save_ini_file_one(CSimpleIni *ini, const _TCHAR *section_name, int rows, const uint32_t *map, const uint32_t *preset);

public:
	KEYBOARD(VM* parent_vm, EMU* parent_emu, const char* identifier) : DEVICE(parent_vm, parent_emu, identifier)
	{
		set_class_name("KEYBOARD");
	}
	~KEYBOARD() {}

	// common functions
	void initialize();
	void reset();
	void release();
	void write_signal(int id, uint32_t data, uint32_t mask);
	void event_frame();

	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);

	// unique functions
	void set_context_cpu(DEVICE* device) {
		d_cpu = device;
	}
	void set_context_disp(DEVICE* device) {
		d_disp = device;
	}
	void set_context_board(DEVICE* device) {
		d_board = device;
	}
#ifdef USE_KEY_RECORD
	void set_keyrecord(KEYRECORD *value) {
		reckey = value;
	}
#endif
	void set_context_pia(DEVICE* device) {
		d_pia = device;
	}
	void set_context_pia_ex2(DEVICE* device) {
		d_pia_ex2 = device;
	}
	void set_context_psg(DEVICE* device) {
		d_psg = device;
	}
	uint8_t get_kb_mode();
	void update_system_key();
	void system_key_down(int code);
	void system_key_up(int code);

	void event_callback(int event_id, int err);

	void save_keybind();
	void clear_joy2joyk_map();
	void set_joy2joyk_map(int num, int idx, uint32_t joy_code);

	void update_config();

#ifdef USE_KEY_RECORD
	bool play_reckey(const _TCHAR* filename);
	bool record_reckey(const _TCHAR* filename);
	void stop_reckey(bool stop_play = true, bool stop_record = true);
#endif

	void save_state(FILEIO *fio);
	bool load_state(FILEIO *fio);

#ifdef USE_DEBUGGER
	uint32_t debug_read_io8(uint32_t addr);
	void debug_event_frame();
	bool debug_write_reg(uint32_t reg_num, uint32_t data);
	bool debug_write_reg(const _TCHAR *reg, uint32_t data);
	void debug_regs_info(const _TCHAR *title, _TCHAR *buffer, size_t buffer_len);
#endif
};

#endif /* KEYBOARD_H */

