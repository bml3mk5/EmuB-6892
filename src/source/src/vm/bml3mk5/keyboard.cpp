/** @file keyboard.cpp

	HITACHI BASIC MASTER LEVEL3 Mark5 Emulator 'EmuB-6892'
	Skelton for retropc emulator

	@author Sasaji
	@date   2011.06.08 -

	@brief [ keyboard ]
*/

//#ifdef _UNICODE
//#define SI_CONVERT_GENERIC	// use generic simple.ini on all platform
//#endif
#include "keyboard.h"
//#include "../../emu.h"
#include "../../simple_ini.h"
#include "display.h"
#include "../../emumsg.h"
#include "keyrecord.h"
#include "keyboard_bind.h"
#include "../../gui/gui_keybinddata.h"
#include "../../config.h"
#include "../../labels.h"
#include "../../utility.h"
#if defined(USE_PIAJOYSTICK) || defined(USE_KEY2PIAJOYSTICK)
#include "../pia.h"
#endif
#if defined(USE_PSGJOYSTICK) || defined(USE_KEY2PSGJOYSTICK)
#include "../ay38910.h"
#endif

#define KEYBOARD_COUNTER_CYCLE	1
#define KEYBOARD_COUNTER_MAX	(0x80 << KEYBOARD_COUNTER_CYCLE)

#define KEYBIND_HEADER	"KEYBIND3"

void KEYBOARD::initialize()
{
	p_key_stat = emu->key_buffer();
	kb_mode = 0x44;
	m_scan_code = 0;
	m_key_scan_code = 0;
	m_key_pressed = 0;
	kb_nmi = 0x7f;

	p_mouse_stat = emu->mouse_buffer();
	lpen_flg = 0;
	lpen_flg_prev = 0;
	lpen_bl = false;

#if defined(USE_JOYSTICK) || defined(USE_KEY2JOYSTICK)
	for(int i=0; i<MAX_JOYSTICKS; i++) {
		p_joy_stat[i] = emu->joy_buffer(i);
		p_joy_real_stat[i] = emu->joy_real_buffer(i);
	}
#endif
#if defined(USE_PIAJOYSTICK) || defined(USE_KEY2PIAJOYSTICK)
	for(int i=0; i<MAX_JOYSTICKS; i++) {
		joy_pia[i] = 0;
	}
	joy_pia_sel = 0;
#endif
#if defined(USE_PSGJOYSTICK) || defined(USE_KEY2PSGJOYSTICK)
	for(int i=0; i<MAX_JOYSTICKS; i++) {
		joy_psg[i] = 0;
	}
	joy_psg_sel = 0;
#endif

	p_key_mod = emu->get_key_mod_ptr();
	m_pause_pressed = false;
	m_altkey_pressed = false;
	m_modesw_pressed = false;
	m_powersw_pressed = false;

	memset(vm_key_stat, 0, sizeof(vm_key_stat));
	emu->set_vm_key_status_buffer(vm_key_stat, KEYBIND_KEYS);

	m_counter = 0;
	remain_count = -1;
//	frame_counter = 0;
//	event_counter = 0;

	// set key -> key default mapping
	memcpy(scan2key_map, scan2key_defmap, sizeof(scan2key_map));
#ifdef USE_JOYSTICK
	// set joy -> key default mapping
	memcpy(joy2key_map, joy2key_defmap, sizeof(joy2key_map));
# ifdef USE_PIAJOYSTICK
	// set joy -> joy pia default mapping
#  ifdef USE_JOYSTICKBIT
	memcpy(sjoy2joya_map, sjoy2joybit_defmap, sizeof(sjoy2joya_map));
#  else
	memcpy(sjoy2joya_map, sjoy2joy_defmap, sizeof(sjoy2joya_map));
#  endif
# endif
# ifdef USE_PSGJOYSTICK
	// set joy -> joy psg default mapping
#  ifdef USE_JOYSTICKBIT
	memcpy(sjoy2joyb_map, sjoy2psgjoybit_defmap, sizeof(sjoy2joyb_map));
#  else
	memcpy(sjoy2joyb_map, sjoy2joy_defmap, sizeof(sjoy2joyb_map));
#  endif
# endif
#endif /* USE_JOYSTICK */
#ifdef USE_KEY2PIAJOYSTICK
	// set key -> joy pia default mapping
# ifdef USE_JOYSTICKBIT
	memcpy(scan2joya_map, scan2joybit_defmap, sizeof(scan2joya_map));
# else
	memcpy(scan2joya_map, scan2joy_defmap, sizeof(scan2joya_map));
# endif
#endif /* USE_KEY2PIAJOYSTICK */
#ifdef USE_KEY2PSGJOYSTICK
	// set key -> joy psg default mapping
# ifdef USE_JOYSTICKBIT
	memcpy(scan2joyb_map, scan2psgjoybit_defmap, sizeof(scan2joyb_map));
# else
	memcpy(scan2joyb_map, scan2joy_defmap, sizeof(scan2joyb_map));
# endif
#endif /* USE_KEY2PSGJOYSTICK */

	for(int i=0; i<KEYBIND_PRESETS; i++) {
		// set key -> key preset mapping
		memcpy(scan2key_preset_map[i], scan2key_defmap, sizeof(scan2key_preset_map[0]));
#ifdef USE_JOYSTICK
		// set joy -> key preset mapping
		memcpy(joy2key_preset_map[i], joy2key_defmap, sizeof(joy2key_preset_map[0]));
# ifdef USE_PIAJOYSTICK
		// set joy -> joy pia preset mapping
#  ifdef USE_JOYSTICKBIT
		memcpy(sjoy2joya_preset_map[i], sjoy2joybit_defmap, sizeof(sjoy2joya_preset_map[0]));
#  else
		memcpy(sjoy2joya_preset_map[i], sjoy2joy_defmap, sizeof(sjoy2joya_preset_map[0]));
#  endif
# endif
# ifdef USE_PSGJOYSTICK
		// set joy -> joy psg preset mapping
#  ifdef USE_JOYSTICKBIT
		memcpy(sjoy2joyb_preset_map[i], sjoy2joybit_defmap, sizeof(sjoy2joyb_preset_map[0]));
#  else
		memcpy(sjoy2joyb_preset_map[i], sjoy2joy_defmap, sizeof(sjoy2joyb_preset_map[0]));
#  endif
# endif
#endif /* USE_JOYSTICK */
#ifdef USE_KEY2PIAJOYSTICK
		// set key -> joy pia preset mapping
# ifdef USE_JOYSTICKBIT
		memcpy(scan2joya_preset_map[i], scan2joybit_defmap, sizeof(scan2joya_preset_map[0]));
# else
		memcpy(scan2joya_preset_map[i], scan2joy_defmap, sizeof(scan2joya_preset_map[0]));
# endif
#endif /* USE_KEY2PIAJOYSTICK */
#ifdef USE_KEY2PSGJOYSTICK
		// set key -> joy psg preset mapping
# ifdef USE_JOYSTICKBIT
		memcpy(scan2joyb_preset_map[i], scan2joybit_defmap, sizeof(scan2joyb_preset_map[0]));
# else
		memcpy(scan2joyb_preset_map[i], scan2joy_defmap, sizeof(scan2joyb_preset_map[0]));
# endif
#endif /* USE_KEY2PSGJOYSTICK */
	}

	clear_joy2joyk_map();

	// load keybind
	if (load_ini_file() != true) {
		load_cfg_file();
	}

	convert_map();

	// for dialog box
	int max_tabs = 0;
	for(; max_tabs < 4 && LABELS::keybind_tab[max_tabs] != CMsg::End; max_tabs++) {}

	gKeybind.SetVmKeyMap(Keybind::TAB_KEY2KEY, kb_scan2key_map, (int)(sizeof(kb_scan2key_map) / sizeof(kb_scan2key_map[0])));
#ifdef USE_JOYSTICK
	gKeybind.SetVmKeyMap(Keybind::TAB_JOY2KEY, kb_scan2key_map, (int)(sizeof(kb_scan2key_map) / sizeof(kb_scan2key_map[0])));
# ifdef USE_PIAJOYSTICK
#  ifdef USE_JOYSTICKBIT
	gKeybind.SetVmKeyMap(Keybind::TAB_JOY2JOY, kb_sjoy2joybit_map, (int)(sizeof(kb_sjoy2joybit_map) / sizeof(kb_sjoy2joybit_map[0])));
#  else
	gKeybind.SetVmKeyMap(Keybind::TAB_JOY2JOY, kb_sjoy2joy_map, (int)(sizeof(kb_sjoy2joy_map) / sizeof(kb_sjoy2joy_map[0])));
#  endif
# endif /* USE_PIAJOYSTICK */
# ifdef USE_PSGJOYSTICK
#  ifdef USE_JOYSTICKBIT
	gKeybind.SetVmKeyMap(Keybind::TAB_JOY2JOYB, kb_sjoy2joybit_map, (int)(sizeof(kb_sjoy2joybit_map) / sizeof(kb_sjoy2joybit_map[0])));
#  else
	gKeybind.SetVmKeyMap(Keybind::TAB_JOY2JOYB, kb_sjoy2joy_map, (int)(sizeof(kb_sjoy2joy_map) / sizeof(kb_sjoy2joy_map[0])));
#  endif
# endif /* USE_PSGJOYSTICK */
#endif /* USE_JOYSTICK */
#ifdef USE_KEY2PIAJOYSTICK
# ifdef USE_JOYSTICKBIT
	gKeybind.SetVmKeyMap(Keybind::TAB_KEY2JOY, kb_sjoy2joybit_map, (int)(sizeof(kb_sjoy2joybit_map) / sizeof(kb_sjoy2joybit_map[0])));
# else
	gKeybind.SetVmKeyMap(Keybind::TAB_KEY2JOY, kb_scan2joy_map, (int)(sizeof(kb_scan2joy_map) / sizeof(kb_scan2joy_map[0])));
# endif
#endif /* USE_KEY2PIAJOYSTICK */
#ifdef USE_KEY2PSGJOYSTICK
# ifdef USE_JOYSTICKBIT
	gKeybind.SetVmKeyMap(Keybind::TAB_KEY2JOYB, kb_sjoy2joybit_map, (int)(sizeof(kb_sjoy2joybit_map) / sizeof(kb_sjoy2joybit_map[0])));
# else
	gKeybind.SetVmKeyMap(Keybind::TAB_KEY2JOYB, kb_scan2joy_map, (int)(sizeof(kb_scan2joy_map) / sizeof(kb_scan2joy_map[0])));
# endif
#endif /* USE_KEY2PSGJOYSTICK */

	gKeybind.SetVkKeySize(Keybind::TAB_KEY2KEY, KEYBIND_KEYS);
#ifdef USE_JOYSTICK
	gKeybind.SetVkKeySize(Keybind::TAB_JOY2KEY, KEYBIND_KEYS);
# ifdef USE_PIAJOYSTICK
	gKeybind.SetVkKeySize(Keybind::TAB_JOY2JOY, KEYBIND_JOYS);
# endif
# ifdef USE_PSGJOYSTICK
	gKeybind.SetVkKeySize(Keybind::TAB_JOY2JOYB, KEYBIND_JOYS);
# endif
#endif /* USE_JOYSTICK */
#ifdef USE_KEY2PIAJOYSTICK
	gKeybind.SetVkKeySize(Keybind::TAB_KEY2JOY, KEYBIND_JOYS);
#endif /* USE_KEY2PIAJOYSTICK */
#ifdef USE_KEY2PSGJOYSTICK
	gKeybind.SetVkKeySize(Keybind::TAB_KEY2JOYB, KEYBIND_JOYS);
#endif /* USE_KEY2PSGJOYSTICK */

	gKeybind.SetVkKeyDefMap(Keybind::TAB_KEY2KEY, scan2key_defmap);
#ifdef USE_JOYSTICK
	gKeybind.SetVkKeyDefMap(Keybind::TAB_JOY2KEY, joy2key_defmap);
# ifdef USE_PIAJOYSTICK
#  ifdef USE_JOYSTICKBIT
	gKeybind.SetVkKeyDefMap(Keybind::TAB_JOY2JOY, sjoy2joybit_defmap);
#  else
	gKeybind.SetVkKeyDefMap(Keybind::TAB_JOY2JOY, sjoy2joy_defmap);
#  endif
# endif /* USE_PIAJOYSTICK */
# ifdef USE_PSGJOYSTICK
#  ifdef USE_JOYSTICKBIT
	gKeybind.SetVkKeyDefMap(Keybind::TAB_JOY2JOYB, sjoy2psgjoybit_defmap);
#  else
	gKeybind.SetVkKeyDefMap(Keybind::TAB_JOY2JOYB, sjoy2joy_defmap);
#  endif
# endif /* USE_PSGJOYSTICK */
#endif /* USE_JOYSTICK */
#ifdef USE_KEY2PIAJOYSTICK
# ifdef USE_JOYSTICKBIT
	gKeybind.SetVkKeyDefMap(Keybind::TAB_KEY2JOY, scan2joybit_defmap);
# else
	gKeybind.SetVkKeyDefMap(Keybind::TAB_KEY2JOY, scan2joy_defmap);
# endif
#endif /* USE_KEY2PIAJOYSTICK */
#ifdef USE_KEY2PSGJOYSTICK
# ifdef USE_JOYSTICKBIT
	gKeybind.SetVkKeyDefMap(Keybind::TAB_KEY2JOYB, scan2psgjoybit_defmap);
# else
	gKeybind.SetVkKeyDefMap(Keybind::TAB_KEY2JOYB, scan2joy_defmap);
# endif
#endif /* USE_KEY2PSGJOYSTICK */

	gKeybind.SetVkKeyMap(Keybind::TAB_KEY2KEY, scan2key_map);
#ifdef USE_JOYSTICK
	gKeybind.SetVkKeyMap(Keybind::TAB_JOY2KEY, joy2key_map);
# ifdef USE_PIAJOYSTICK
	gKeybind.SetVkKeyMap(Keybind::TAB_JOY2JOY, sjoy2joya_map);
# endif
# ifdef USE_PSGJOYSTICK
	gKeybind.SetVkKeyMap(Keybind::TAB_JOY2JOYB, sjoy2joyb_map);
# endif
#endif /* USE_JOYSTICK */
#ifdef USE_KEY2PIAJOYSTICK
	gKeybind.SetVkKeyMap(Keybind::TAB_KEY2JOY, scan2joya_map);
#endif /* USE_KEY2PIAJOYSTICK */
#ifdef USE_KEY2PSGJOYSTICK
	gKeybind.SetVkKeyMap(Keybind::TAB_KEY2JOYB, scan2joyb_map);
#endif /* USE_KEY2PSGJOYSTICK */

	for(int i=0; i<KEYBIND_PRESETS; i++) {
		gKeybind.SetVkKeyPresetMap(Keybind::TAB_KEY2KEY, i, scan2key_preset_map[i]);
#ifdef USE_JOYSTICK
		gKeybind.SetVkKeyPresetMap(Keybind::TAB_JOY2KEY, i, joy2key_preset_map[i]);
# ifdef USE_PIAJOYSTICK
		gKeybind.SetVkKeyPresetMap(Keybind::TAB_JOY2JOY, i, sjoy2joya_preset_map[i]);
# endif
# ifdef USE_PSGJOYSTICK
		gKeybind.SetVkKeyPresetMap(Keybind::TAB_JOY2JOYB, i, sjoy2joyb_preset_map[i]);
# endif
#endif /* USE_JOYSTICK */
#ifdef USE_KEY2PIAJOYSTICK
		gKeybind.SetVkKeyPresetMap(Keybind::TAB_KEY2JOY, i, scan2joya_preset_map[i]);
#endif /* USE_KEY2PIAJOYSTICK */
#ifdef USE_KEY2PSGJOYSTICK
		gKeybind.SetVkKeyPresetMap(Keybind::TAB_KEY2JOYB, i, scan2joyb_preset_map[i]);
#endif /* USE_KEY2PSGJOYSTICK */
	}

#ifdef USE_KEY_RECORD
	reckey->set_key_scan_code_ptr(&m_key_scan_code);
	reckey->set_counter_ptr(&m_counter);
	reckey->set_remain_count_ptr(&remain_count);
	reckey->set_kb_mode_ptr(&kb_mode);
#endif

	// clear PIA port
	reset_joy_pia();
	// clear PSG port
	reset_joy_psg();

	// event
	register_frame_event(this);
}

void KEYBOARD::convert_map()
{
	emu->clear_vm_key_map();
	for(int k=0; k<KEYBIND_KEYS; k++) {
		for(int i=0; i<KEYBIND_ASSIGN; i++) {
			emu->set_vm_key_map(scan2key_map[k].d[i], k);
		}
	}
#ifdef USE_PIAJOYSTICK
	emu->clear_joy2joy_idx(DEV_PIAJOY);
	for(uint32_t k=0; k<KEYBIND_JOYS; k++) {
# ifndef USE_JOYSTICKBIT
		emu->set_joy2joy_idx(DEV_PIAJOY, k, sjoy2joy_defmap[k].d[0]);
# else
		emu->set_joy2joy_idx(DEV_PIAJOY, k, 1 << k);
# endif
	}
	emu->clear_joy2joy_map(DEV_PIAJOY);
	for(uint32_t k=0; k<KEYBIND_JOYS; k++) {
		for(int i=0; i<KEYBIND_ASSIGN; i++) {
			emu->set_joy2joy_map(DEV_PIAJOY, i, k, sjoy2joya_map[k].d[i]);
		}
	}
#endif /* USE_PIAJOYSTICK */
#ifdef USE_PSGJOYSTICK
	emu->clear_joy2joy_idx(DEV_PSGJOY);
	for(uint32_t k=0; k<KEYBIND_JOYS; k++) {
# ifndef USE_JOYSTICKBIT
		emu->set_joy2joyb_idx(DEV_PSGJOY, k, sjoy2joyb_defmap[k].d[0]);
# else
		emu->set_joy2joy_idx(DEV_PSGJOY, k, 1 << k);
# endif
	}
	emu->clear_joy2joy_map(DEV_PSGJOY);
	for(uint32_t k=0; k<KEYBIND_JOYS; k++) {
		for(int i=0; i<KEYBIND_ASSIGN; i++) {
			emu->set_joy2joy_map(DEV_PSGJOY, i, k, sjoy2joyb_map[k].d[i]);
		}
	}
#endif /* USE_PSGJOYSTICK */
#ifdef USE_KEY2PIAJOYSTICK
	emu->clear_key2joy_map(DEV_PIAJOY);
	for(uint32_t k=0; k<KEYBIND_JOYS; k++) {
		for(int i=0; i<KEYBIND_ASSIGN; i++) {
# ifndef USE_JOYSTICKBIT
			if (k >= 0x0c) {
				// buttons
				emu->set_key2joy_map(DEV_PIAJOY, scan2joya_map[k].d[i], i, (k - 0x0c) | 0x80000000);
			} else {
				// allows
				emu->set_key2joy_map(DEV_PIAJOY, scan2joya_map[k].d[i], i, k);
			}
# else
			// each bits
			emu->set_key2joy_map(DEV_PIAJOY, scan2joya_map[k].d[i], i, k);
# endif
		}
	}
#endif /* USE_KEY2PIAJOYSTICK */
#ifdef USE_KEY2PSGJOYSTICK
	emu->clear_key2joy_map(DEV_PSGJOY);
	for(uint32_t k=0; k<KEYBIND_JOYS; k++) {
		for(int i=0; i<KEYBIND_ASSIGN; i++) {
# ifndef USE_JOYSTICKBIT
			if (k >= 0x0c) {
				// buttons
				emu->set_key2joy_map(DEV_PSGJOY, scan2joyb_map[k].d[i], i, (k - 0x0c) | 0x80000000);
			} else {
				// allows
				emu->set_key2joy_map(DEV_PSGJOY, scan2joyb_map[k].d[i], i, k);
			}
# else
			// each bits
			emu->set_key2joy_map(DEV_PSGJOY, scan2joyb_map[k].d[i], i, k);
# endif
		}
	}
#endif /* USE_KEY2PSGJOYSTICK */
}

void KEYBOARD::reset()
{
	kb_mode = 0;
	m_scan_code = 0;
	m_key_scan_code = 0;
	m_key_pressed = 0;
	kb_nmi = 0x7f;

	lpen_flg = 0;
	lpen_flg_prev = 0;
	lpen_bl = false;

	m_counter = 0;
	remain_count = -1;
	remain_count_max = FLG_ORIG_LIMKEY ? (KEYBOARD_COUNTER_MAX << 3) - 2 : -1;

#if defined(USE_PIAJOYSTICK) || defined(USE_KEY2PIAJOYSTICK)
	joy_pia_sel = 0;
	joy_pia[0] = 0;
	joy_pia[1] = 0;
#endif
#if defined(USE_PSGJOYSTICK) || defined(USE_KEY2PSGJOYSTICK)
	joy_psg_sel = 0;
	joy_psg[0] = 0;
	joy_psg[1] = 0;
#endif
#ifdef _DEBUG_KEYBOARD
	frame_counter = 0;
	event_counter = 0;
#endif
	// clear PIA port
	reset_joy_pia();
}

void KEYBOARD::release()
{
	emu->set_vm_key_status_buffer(NULL, 0);
	// save keybind
	save_keybind();
}

// ----------------------------------------------------------------------------

void KEYBOARD::save_keybind()
{
	// save keybind
	save_cfg_file();
	save_ini_file();
}

void KEYBOARD::clear_joy2joyk_map()
{
}

void KEYBOARD::set_joy2joyk_map(int num, int idx, uint32_t joy_code)
{
}

/// cfg file is no longer supported.
bool KEYBOARD::load_cfg_file()
{
	return true;
}

/// cfg file is no longer supported.
void KEYBOARD::save_cfg_file()
{
}

// ----------------------------------------------------------------------------

void KEYBOARD::load_ini_file_one(CSimpleIni *ini, const _TCHAR *section_name, int rows, uint32_t *map, uint32_t *preset)
{
	_TCHAR section[100];
	int cols = KEYBIND_ASSIGN;

	const CSimpleIniSection *csection = ini->Find(section_name);
	int count = 0;
	if (csection) {
		memset(map, 0, sizeof(uint32_t) * rows * cols);
		count = csection->Count();
	}
	for (int idx = 0; idx < count; idx++) {
		const CSimpleIniItem *item = csection->Item(idx);
		int k = 0;
		int i = 0;
		int rc = _stscanf(item->GetKey().Get(), _T("%02x_%d"), &k, &i);
		if (rc != 2 || k < 0 || k >= rows || i < 0 || i >= cols) {
			continue;
		}
		long v = item->GetLong(0);
		map[k * cols + i] = (uint32_t)(v & 0xffffffff);
	}
	for (int no = 0; no < KEYBIND_PRESETS; no++) {
		uint32_t *preset1 = &preset[no * rows * cols];
		UTILITY::stprintf(section, 100, _T("%sPreset%d"), section_name, no + 1);
		csection = ini->Find(section);
		count = 0;
		if (csection) {
			memset(preset1, 0, sizeof(uint32_t) * rows * cols);
			count = csection->Count();
		}
		for (int idx = 0; idx < count; idx++) {
			const CSimpleIniItem *item = csection->Item(idx);
			int k = 0;
			int i = 0;
			int rc = _stscanf(item->GetKey().Get(), _T("%02x_%d"), &k, &i);
			if (rc != 2 || k < 0 || k >= rows || i < 0 || i >= cols) {
				continue;
			}
			long v = item->GetLong(0);
			preset1[k * cols + i] = (uint32_t)(v & 0xffffffff);
		}
	}
}

bool KEYBOARD::load_ini_file()
{
	const _TCHAR *app_path;

	CSimpleIni *ini = new CSimpleIni();
//#ifdef _UNICODE
//	ini->SetUnicode(true);
//#endif

	// load ini file
	app_path = emu->initialize_path();

	_TCHAR file_path[_MAX_PATH];
	UTILITY::concat(file_path, _MAX_PATH, app_path, _T("keybind.ini"), NULL);

	if (ini->LoadFile(file_path) == true) {
		logging->out_logf_x(LOG_INFO, CMsg::VSTR_was_loaded, _T("keybind.ini"));
	} else {
		logging->out_logf_x(LOG_WARN, CMsg::VSTR_couldn_t_be_loaded, _T("keybind.ini"));
		delete ini;
		return false;
	}
	// check file version
	if (_tcscmp(ini->GetValue(_T(""), _T("Version"), _T("")), _T(VK_KEY_TYPE)) != 0) {
		logging->out_logf_x(LOG_WARN, CMsg::VSTR_is_not_compatible_use_default_setting, _T("keybind.ini"));
		delete ini;
		return false;
	}

	load_ini_file_one(ini, _T("Keyboard2Key"), KEYBIND_KEYS, (uint32_t *)scan2key_map, (uint32_t *)scan2key_preset_map);
#ifdef USE_JOYSTICK
	load_ini_file_one(ini, _T("Joypad2Key"), KEYBIND_KEYS, (uint32_t *)joy2key_map, (uint32_t *)joy2key_preset_map);
# ifdef USE_PIAJOYSTICK
	load_ini_file_one(ini, _T("Joypad2Joy"), KEYBIND_JOYS, (uint32_t *)sjoy2joya_map, (uint32_t *)sjoy2joya_preset_map);
# endif
# ifdef USE_PSGJOYSTICK
	load_ini_file_one(ini, _T("Joypad2PSGJoy"), KEYBIND_JOYS, (uint32_t *)sjoy2joyb_map, (uint32_t *)sjoy2joyb_preset_map);
# endif
#endif /* USE_JOYSTICK */
#ifdef USE_KEY2PIAJOYSTICK
	load_ini_file_one(ini, _T("Keyboard2Joy"), KEYBIND_JOYS, (uint32_t *)scan2joya_map, (uint32_t *)scan2joya_preset_map);
#endif /* USE_KEY2PIAJOYSTICK */
#ifdef USE_KEY2PSGJOYSTICK
	load_ini_file_one(ini, _T("Keyboard2PSGJoy"), KEYBIND_JOYS, (uint32_t *)scan2joyb_map, (uint32_t *)scan2joyb_preset_map);
#endif /* USE_KEY2PSGJOYSTICK */

	delete ini;

	return true;
}

void KEYBOARD::save_ini_file_one(CSimpleIni *ini, const _TCHAR *section_name, int rows, const uint32_t *map, const uint32_t *preset)
{
	_TCHAR section[100];
	_TCHAR key[100];
	int cols = KEYBIND_ASSIGN;

    for (int k = 0; k < rows; k++) {
		for (int i = 0; i < cols; i++) {
			UTILITY::stprintf(key, 100, _T("%02x_%d"), k, i);
			const uint32_t v = map[k * cols + i];
			if (v != 0) {
				ini->SetLongValue(section_name, key, v, NULL, true);
			}
		}
	}
    for (int no = 0; no < KEYBIND_PRESETS; no++) {
		UTILITY::stprintf(section, 100, _T("%sPreset%d"), section_name, no + 1);
		for (int k = 0; k < rows; k++) {
			for (int i = 0; i < cols; i++) {
				const uint32_t *preset1 = &preset[no * rows * cols];
				UTILITY::stprintf(key, 100, _T("%02x_%d"), k, i);
				const uint32_t v = preset1[k * cols + i];
				if (v != 0) {
					ini->SetLongValue(section, key, v, NULL, true);
				}
			}
		}
	}
}

void KEYBOARD::save_ini_file()
{
	const _TCHAR *app_path;
	_TCHAR comment[100];

	CSimpleIni *ini = new CSimpleIni();
//#ifdef _UNICODE
//	ini->SetUnicode(true);
//#endif

	// section
	UTILITY::stprintf(comment, 100, _T("; %s keybind file"), _T(DEVICE_NAME));
	ini->SetValue(_T(""), _T("Version"), _T(VK_KEY_TYPE), comment);

	save_ini_file_one(ini, _T("Keyboard2Key"), KEYBIND_KEYS, (const uint32_t *)scan2key_map, (const uint32_t *)scan2key_preset_map);
#ifdef USE_JOYSTICK
	save_ini_file_one(ini, _T("Joypad2Key"), KEYBIND_KEYS, (const uint32_t *)joy2key_map, (const uint32_t *)joy2key_preset_map);
# ifdef USE_PIAJOYSTICK
	save_ini_file_one(ini, _T("Joypad2Joy"), KEYBIND_JOYS, (const uint32_t *)sjoy2joya_map, (const uint32_t *)sjoy2joya_preset_map);
# endif
# ifdef USE_PSGJOYSTICK
	save_ini_file_one(ini, _T("Joypad2PSGJoy"), KEYBIND_JOYS, (const uint32_t *)sjoy2joyb_map, (const uint32_t *)sjoy2joyb_preset_map);
# endif
#endif /* USE_JOYSTICK */
#ifdef USE_KEY2PIAJOYSTICK
	save_ini_file_one(ini, _T("Keyboard2Joy"), KEYBIND_JOYS, (const uint32_t *)scan2joya_map, (const uint32_t *)scan2joya_preset_map);
#endif /* USE_KEY2PIAJOYSTICK */
#ifdef USE_KEY2PSGJOYSTICK
	save_ini_file_one(ini, _T("Keyboard2PSGJoy"), KEYBIND_JOYS, (const uint32_t *)scan2joyb_map, (const uint32_t *)scan2joyb_preset_map);
#endif /* USE_KEY2PSGJOYSTICK */

	// save ini file
	app_path = emu->initialize_path();

	_TCHAR file_path[_MAX_PATH];
	UTILITY::concat(file_path, _MAX_PATH, app_path, _T("keybind.ini"), NULL);

	if (ini->SaveFile(file_path) == true) {
		logging->out_logf_x(LOG_INFO, CMsg::VSTR_was_saved, _T("keybind.ini"));
	} else {
		logging->out_logf_x(LOG_WARN, CMsg::VSTR_couldn_t_be_saved, _T("keybind.ini"));
	}

	delete ini;
}

// ----------------------------------------------------------------------------

void KEYBOARD::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch (id) {
#ifdef _BML3MK5
		case SIG_KEYBOARD_HSYNC:
			if (data & mask) update_keyboard();
			break;
#endif
#ifdef USE_PIAJOYSTICK
# ifdef USE_JOYSTICKBIT
		case SIG_KEYBOARD_PIA_PB:
//			if (FLG_USEPIAJOYSTICK) {
//				d_pia->write_signal(PIA::SIG_PIA_PB, joy_pia[0], 0xff);
//			}
			break;
# else
		case SIG_KEYBOARD_PIA_PA:
			// always set
			joy_pia_sel = (data & 0x40 ? 1 : 0);
			d_pia->write_signal(PIA::SIG_PIA_PA, ~joy_pia[joy_pia_sel], 0x3f);	// negative
			break;
# endif
#endif /* USE_PIAJOYSTICK */
		case SIG_CPU_RESET:
			now_reset = (data & mask) ? true : false;
			reset();
			break;
	}
}

void KEYBOARD::write_io8(uint32_t addr, uint32_t data)
{
	if (now_reset) return;

	switch (addr & 0xffff) {
#ifdef USE_LIGHTPEN
		case ADDR_LPEN_BL:
			lpen_bl = (data & 0x80) ? true : false;
//			logging->out_debugf("w_lpenbl: %02x",lpen_bl);
			break;
#endif
		case ADDR_KEYBOARD:
			kb_mode = data;

			// keyborad irq interrupt
			if ((kb_mode & 0x40) && (m_key_scan_code & 0x80) != 0) {
				d_board->write_signal(SIG_CPU_IRQ, SIG_IRQ_KEYBOARD_MASK, SIG_IRQ_KEYBOARD_MASK);
			} else {
				d_board->write_signal(SIG_CPU_IRQ, 0, SIG_IRQ_KEYBOARD_MASK);
			}
			// break nmi reset
			if ((kb_nmi & 0x80) != 0 && (kb_mode & 0x80) == 0) {
				d_board->write_signal(SIG_CPU_NMI, 0, SIG_NMI_KEYBREAK_MASK);
			}
#ifdef _DEBUG_KEYBOARD
			logging->out_debugf("kbd wio 1 fr:%04d ct:%03x kb:%02x"
			, frame_counter, counter, kb_mode);
#endif
			break;
	}
}

uint32_t KEYBOARD::read_io8(uint32_t addr)
{
	uint32_t data = 0;

	if (now_reset) return data;

	switch (addr & 0xffff) {
		case ADDR_KB_NMI:
			// break key (0xffc8)
			data = kb_nmi; // & 0x80;
			if (!pressing_key(0x80)) {
				kb_nmi = 0x7f;

				d_board->write_signal(SIG_CPU_NMI, 0, SIG_NMI_KEYBREAK_MASK);
			}
			break;
		case ADDR_LP_FLG:
			// light pen flag (0xffcb)
			data = (lpen_flg & 0x80) | 0x7f;
			lpen_flg = 0;
#ifdef USE_LIGHTPEN
			// release IRQ signal
			d_board->write_signal(SIG_CPU_IRQ, 0, SIG_IRQ_LIGHTPEN_MASK);
#endif
//			logging->out_debugf("r_lpenflg:%04d ct:%03x d:%02x"
//			, frame_counter, counter, data);
			break;
		case ADDR_KEYBOARD:
			// keyboard (0xffe0)
			data = m_key_scan_code;
			m_key_scan_code &= 0x7f;

			emu->recognized_key(data);

			// release IRQ signal
			d_board->write_signal(SIG_CPU_IRQ, 0, SIG_IRQ_KEYBOARD_MASK);
//			key_native_code= 0;

#ifdef _DEBUG_KEYBOARD
			logging->out_debugf("kbd rio 1 fr:%04d ct:%03x rc:%4d d:%02x k:%02x"
			, frame_counter, counter, remain_count, data, key_scan_code);
#endif

			remain_count = -1;
			break;
	}
	return data;
}

void KEYBOARD::clear_scan_code(int key_counter)
{
//	if (kb_mode & 0x40) {
//		key_counter = (int)(key_counter / KEYBOARD_COUNTER_CYCLE);
//	}

	// if kb bit3 is set, enable scan_code is between 0x00 and 0x07
	m_scan_code = (key_counter) & ((kb_mode & 0x08) ? 0x07 : 0x7f);
	if (m_key_scan_code < 0x80) {
		m_key_scan_code = m_scan_code;
	}

//	logging->out_debugf("kbd csc fr:%04d ct:%03x rc:%02d s:%02x k:%02x"
//	, frame_counter, counter, remain_count, scan_code, key_scan_code);

}

void KEYBOARD::update_scan_code(int key_counter)
{
	bool pressed = false;

	// if kb bit3 is set, enable scan_code is between 0x00 and 0x07
	m_scan_code = (key_counter) & ((kb_mode & 0x08) ? 0x07 : 0x7f);
	if (m_scan_code == 0x7f) {
		if (m_key_pressed == 1) {
			pressed = true;
			m_scan_code |= 0x80;
			remain_count = remain_count_max;
		}
		if (m_key_pressed > 0) m_key_pressed--;
	} else if (m_scan_code >= 0x70) {
		// no assaign
	} else {
#ifdef _BML3MK5
		pressed = pressing_key(m_scan_code);
#endif
		if (pressed == true) {
			m_scan_code |= 0x80;
			m_key_pressed = 2;
			remain_count = remain_count_max;
		}
	}

	if (m_key_scan_code < 0x80) {
		m_key_scan_code = m_scan_code;
	} else if (m_scan_code >= 0x80) {
		m_key_scan_code = m_scan_code;
	}

	// keyborad irq interrupt
	if ((kb_mode & 0x40) && (pressed == true)) {
		d_board->write_signal(SIG_CPU_IRQ, SIG_IRQ_KEYBOARD_MASK, SIG_IRQ_KEYBOARD_MASK);
	}

//	pressed = false;

#ifdef _DEBUG_KEYBOARD
	if (pressed) {
		logging->out_debugf("kbd usc 2 fr:%04d ct:%03x rc:%4d s:%02x k:%02x"
		, frame_counter, counter, remain_count, scan_code, key_scan_code);
	}
#endif
}

void KEYBOARD::update_special_key()
{
	bool pressed = false;

	for(int code = 0x7d; code <= 0x80; code++) {
		pressed = pressing_key(code);
		switch(code) {
			case 0x7d:
				// mode switch
				if (pressed && m_modesw_pressed == false) {
#ifdef _BML3MK5
					emu->change_dipswitch(2);
#endif
				}
				m_modesw_pressed = pressed;
				break;
			case 0x7e:
				// power switch
				if (pressed	&& m_powersw_pressed == false) {
					emumsg.Send(EMUMSG_ID_RESET);
				}
				m_powersw_pressed = pressed;
				break;
			case 0x7f:
				// reset switch
				if (pressed != now_reset) {
					d_board->write_signal(SIG_CPU_RESET, pressed ? 2 : 0, 2);
				}
				break;
			case 0x80:
				// break key
				if (pressed == true) {
					if (!(kb_nmi & 0x80)) {
						kb_nmi = 0xff;
						// keyborad nmi interrupt
						if (kb_mode & 0x80) {
							d_board->write_signal(SIG_CPU_NMI, SIG_NMI_KEYBREAK_MASK, SIG_NMI_KEYBREAK_MASK);
						}
					}
//				} else {
//					kb_nmi = 0x7f;
				}
				break;
		}
	}
}

#ifdef USE_LIGHTPEN
void KEYBOARD::update_light_pen()
{
#ifdef USE_KEY_RECORD
	reckey->processing_lightpen_status(p_mouse_stat);
#endif

	if ((lpen_bl && FLG_USELIGHTPEN) || pConfig->reckey_playing) {
		lpen_flg = (p_mouse_stat[2] & 1) ? 0x80 : 0;
		if (lpen_flg != 0) {
			// push light pen
			// LPSTB signal
			d_disp->write_signal(DISPLAY::SIG_DISPLAY_LPSTB
				,((p_mouse_stat[0] & 0xffff) << 16) | (p_mouse_stat[1] & 0xffff)
				, 0xffffffff);

			// IRQ interrupt
			d_board->write_signal(SIG_CPU_IRQ, SIG_IRQ_LIGHTPEN_MASK, SIG_IRQ_LIGHTPEN_MASK);

		}
#if 0
		if (pConfig->reckey_playing) {
			lpen_flg = (reckey->get_lpen_recp_stat(2) & 1) ? 0x80 : 0;
			if (lpen_flg != 0) {
				// push light pen
				// LPSTB signal
				d_disp->write_signal(DISPLAY::SIG_DISPLAY_LPSTB
					,((reckey->get_lpen_recp_stat(0) & 0xffff) << 16) | (reckey->get_lpen_recp_stat(1) & 0xffff)
					, 0xffffffff);

				// IRQ interrupt
				d_board->write_signal(SIG_CPU_IRQ, SIG_IRQ_LIGHTPEN_MASK, SIG_IRQ_LIGHTPEN_MASK);

			}
		}
#endif
		lpen_flg_prev = lpen_flg;

	}
//	logging->out_debugf("lpen bl:%02x flg:%02x x:%03d y:%03d reg:%02x%02x",lpen_bl,lpen_flg, mouse_stat[0], mouse_stat[1], regs[16], regs[17]);
}
#endif

uint8_t KEYBOARD::get_kb_mode()
{
	return (pConfig->now_power_off ? 4 : (kb_mode & 0x07));
}

void KEYBOARD::update_system_key()
{
	bool pressed = false;

	for(int code = 0x78; code <= 0x79; code++) {
		pressed = pressing_key(code);
		switch(code) {
			case 0x78:
				// pause
				if (pressed && m_pause_pressed == false) {
//					emu->set_pause(2, emu->get_pause(2) ? false : true);
					emumsg.Send(EMUMSG_ID_PAUSE);
				}
				m_pause_pressed = pressed;
				break;
			case 0x79:
				// alt
				m_altkey_pressed = pressed;
				*p_key_mod = (m_altkey_pressed) ? (*p_key_mod) | KEY_MOD_ALT_KEY : (*p_key_mod) & ~KEY_MOD_ALT_KEY;
				break;
		}
	}
}

void KEYBOARD::reset_joy_pia()
{
	uint8_t pia_a, pia_b;

#if defined(USE_PIAJOYSTICK) || defined(USE_KEY2PIAJOYSTICK)
	pia_a = FLG_PIAJOY_NEGATIVE != 0 ? ~joy_pia[0] : joy_pia[0];
#else
	pia_a = 0xff;
#endif  /* USE_PIAJOYSTICK || USE_KEY2PIAJOYSTICK */
	pia_b = pia_a;

#ifdef USE_JOYSTICKBIT
	switch(pConfig->piajoy_conn_to) {
	case 1:
		pia_b = 0xff;
		break;
	default:
		pia_a = 0xff;
		break;
	}
#endif

	d_pia->write_signal(PIA::SIG_PIA_PA, pia_a, 0xff);
	d_pia_ex2->write_signal(PIA::SIG_PIA_PB, pia_b, 0xff);
}

void KEYBOARD::reset_joy_psg()
{
#if defined(USE_PSGJOYSTICK) || defined(USE_KEY2PSGJOYSTICK)
# if defined(SIG_AY_3_8910_PORT_A)
	d_psg->write_signal(SIG_AY_3_8910_PORT_A, FLG_PSGJOY_NEGATIVE != 0 ? ~joy_psg[0] : joy_psg[0], 0xff);
# endif
# if defined(SIG_AY_3_8910_PORT_B)
	d_psg->write_signal(SIG_AY_3_8910_PORT_B, FLG_PSGJOY_NEGATIVE != 0 ? ~joy_psg[1] : joy_psg[1], 0xff);
# endif
#endif
}

void KEYBOARD::update_joy_pia()
{
#if defined(USE_PIAJOYSTICK) || defined(USE_KEY2PIAJOYSTICK)
	joy_pia[0] = 0;
	joy_pia[1] = 0;

	if (FLG_PIAJOY_ALL) {
# ifndef USE_JOYSTICKBIT
		// MB-S1 port
		// (b0:up, b1:down, b2:left, b3:right, b4:trig2, b5:trig1
		for(int i=0; i<MAX_JOYSTICKS; i++) {
			joy_pia[i] = (p_joy_stat[i][0] & 0xf);
			joy_pia[i] |= ((p_joy_stat[i][0] & 0x20000) >> 13);
			joy_pia[i] |= ((p_joy_stat[i][0] & 0x10000) >> 11);
		}
# else
		uint32_t stat = (p_joy_stat[0][0] | p_joy_stat[1][0]);
		joy_pia[0] =(stat & 0xff);
# endif
	}

# ifdef USE_KEY_RECORD
	reckey->processing_joypia_status(joy_pia);
# endif

	if(FLG_PIAJOY_ALL != 0 || pConfig->reckey_playing) {
# ifndef USE_JOYSTICKBIT
		for(int i=0; i<2; i++) {
			// irq
			if ((joy_pia[i] & ~0xf) != 0 && FLG_PIAJOY_NOIRQ == 0) {
				d_pia->write_signal(i == 0 ? PIA::SIG_PIA_CA1 : PIA::SIG_PIA_CA2, 0, 1);
			} else {
				d_pia->write_signal(i == 0 ? PIA::SIG_PIA_CA1 : PIA::SIG_PIA_CA2, 1, 1);
			}
		}
		d_pia->write_signal(PIA::SIG_PIA_PA, ~joy_pia[joy_pia_sel], 0x3f);	// negative
# else
		switch(pConfig->piajoy_conn_to) {
		case 1:
			d_pia->write_signal(PIA::SIG_PIA_PA, FLG_PIAJOY_NEGATIVE != 0 ? ~joy_pia[0] : joy_pia[0], 0xff);
			break;
		default:
			d_pia_ex2->write_signal(PIA::SIG_PIA_PB, FLG_PIAJOY_NEGATIVE != 0 ? ~joy_pia[0] : joy_pia[0], 0xff);
			break;
		}
# endif
	}
#endif /* USE_PIAJOYSTICK || USE_KEY2PIAJOYSTICK */
}

void KEYBOARD::update_joy_psg()
{
#if defined(USE_PSGJOYSTICK) || defined(USE_KEY2PSGJOYSTICK)
	joy_psg[0] = 0;
	joy_psg[1] = 0;

	if (FLG_PSGJOY_ALL) {
		joy_psg[0] =(p_joy_stat[0][0] & 0xff);
		joy_psg[1] =(p_joy_stat[1][0] & 0xff);
	}

# ifdef USE_KEY_RECORD
	reckey->processing_joypsg_status(joy_psg);
# endif

	if(FLG_PSGJOY_ALL != 0 || pConfig->reckey_playing) {
# if defined(SIG_AY_3_8910_PORT_A)
		d_psg->write_signal(SIG_AY_3_8910_PORT_A, FLG_PSGJOY_NEGATIVE != 0 ? ~joy_psg[0] : joy_psg[0], 0xff);
# endif
# if defined(SIG_AY_3_8910_PORT_B)
		d_psg->write_signal(SIG_AY_3_8910_PORT_B, FLG_PSGJOY_NEGATIVE != 0 ? ~joy_psg[1] : joy_psg[1], 0xff);
# endif
	}
#endif /* USE_PSGJOYSTICK || USE_KEY2PSGJOYSTICK */
}

// ----------------------------------------------------------------------------

bool KEYBOARD::pressing_key(int key_code)
{
	uint32_t code = 0;
	bool pressed = false;
	int i = 0;

#ifdef USE_KEY_RECORD
	reckey->playing_key();
#endif

#if 0
	for(i=0; i<2; i++) {
		code = scan2key_map[key_code][i];
		if (code == 0) break;

		// key pressed ?
		if (key_stat[code]) {
			pressed = true;
			break;
		}
	}
#else
	// key pressed ?
	if (vm_key_stat[key_code] & VM_KEY_STATUS_MASK) {
		pressed = true;
	}

#endif
#if defined(USE_JOYSTICK)
	if (!pressed && FLG_USEJOYSTICK) {
		for(i=0; i<MAX_JOYSTICKS; i++) {
			code = joy2key_map[key_code].d[i];
			if (code == 0) continue;

			// joypad pressed ?
			// allow key or button
			if (!(joy2key_map[KEYBIND_KEYS - 1].d[0] & 1)) {
				if ((p_joy_real_stat[i][0] & 0xf & code) == code || (p_joy_real_stat[i][0] & 0xfffffff0 & code) == code) {
					pressed = true;
					break;
				}
			} else {
				if ((p_joy_real_stat[i][0] & 0xf) == code || (p_joy_real_stat[i][0] & 0xfffffff0) == code) {
					pressed = true;
					break;
				}
			}
		}
	}
#endif
#if 0
	if (pressed != true) {
		// auto key pressed ?
		if (vm_key_stat[key_code]) {
			pressed = true;
		}
	}
#endif
#ifdef USE_KEY_RECORD
//	pressed = reckey->processing_keys(key_code, pressed);
	reckey->recording_key(key_code, pressed);
#endif

	return pressed;
}

// ----------------------------------------------------------------------------
// event handler
// ----------------------------------------------------------------------------

void KEYBOARD::event_frame()
{
#ifdef USE_KEY_RECORD
	// read record key if need
//	reckey->reading_keys(1);
	reckey->read_to_cache();
	reckey->playing_system_keys();
#endif

	// reset switch & break key
	update_special_key();
	// other system keys
	update_system_key();
#ifdef USE_LIGHTPEN
	// light pen
	update_light_pen();
#endif
	// joystick on PIA
	update_joy_pia();
	// joystick on PSG
	update_joy_psg();
}

void KEYBOARD::event_callback(int event_id, int err)
{
}

// ----------------------------------------------------------------------------
void KEYBOARD::update_keyboard()
{
#ifdef _DEBUG_KEYBOARD
	if (event_counter == 0) {
		frame_counter = (frame_counter + 1) % 10000;
	}
	event_counter++;
	if (event_counter >= KEYBOARD_COUNTER_MAX) event_counter = 0;
#endif

	if (remain_count == 0) {
#ifdef _DEBUG_KEYBOARD
		logging->out_debugf("kbd ukd 1 fr:%04d ct:%03x rc:%4d s:-- k:%02x"
		, frame_counter, counter, remain_count, key_scan_code);
#endif
		m_key_scan_code &= 0x7f;
		m_counter = (m_counter - 2) % KEYBOARD_COUNTER_MAX;
		// release IRQ interrupt
		d_board->write_signal(SIG_CPU_IRQ, 0, SIG_IRQ_KEYBOARD_MASK);
	}
	if (remain_count >= 0) remain_count--;

//#ifdef USE_KEY_RECORD
//	// read record key if need
//	if ((counter & 1) != 0) {
//		reckey->reading_keys(0);
//	}
//#endif

	// when not press a key, count up and scan the key
	if (m_key_scan_code < 0x80) {
		if ((m_counter & 1) == 0) {
			clear_scan_code(m_counter >> 1);
		} else {
			update_scan_code(m_counter >> 1);
		}
		if (!now_reset) m_counter++;
		if (m_counter >= KEYBOARD_COUNTER_MAX) m_counter = 0;
	}
}

// ----------------------------------------------------------------------------
void KEYBOARD::system_key_down(int code)
{
#ifdef USE_KEY_RECORD
	reckey->recording_system_keys(code, true);
#endif
}

void KEYBOARD::system_key_up(int code)
{
#ifdef USE_KEY_RECORD
	reckey->recording_system_keys(code, false);
#endif
}

// ----------------------------------------------------------------------------
#ifdef USE_KEY_RECORD
bool KEYBOARD::play_reckey(const _TCHAR* filename)
{
	return reckey->play_reckey(filename);
}

bool KEYBOARD::record_reckey(const _TCHAR* filename)
{
	return reckey->record_reckey(filename);
}

void KEYBOARD::stop_reckey(bool stop_play, bool stop_record)
{
	reckey->stop_reckey(stop_play, stop_record);
}
#endif

// ----------------------------------------------------------------------------
void KEYBOARD::update_config()
{
//	now_autokey = emu->now_auto_key();

	remain_count_max = FLG_ORIG_LIMKEY ? (KEYBOARD_COUNTER_MAX << 3) - 2 : -1;
}

// ----------------------------------------------------------------------------
void KEYBOARD::save_state(FILEIO *fio)
{
	struct vm_state_st vm_state;

	//
	vm_state_ident.version = Uint16_LE(2);
	vm_state_ident.size = Uint32_LE(sizeof(vm_state_ident) + sizeof(vm_state));

	// copy values
	memset(&vm_state, 0, sizeof(vm_state));
	vm_state.kb_mode = kb_mode;
	vm_state.kb_nmi = kb_nmi;
	vm_state.lpen_flg = lpen_flg;
	vm_state.lpen_flg_prev = lpen_flg_prev;
	vm_state.lpen_bl = lpen_bl ? 1 : 0;
	vm_state.lpen_bl |= FLG_USELIGHTPEN ? 2 : 0;	// add version 2
	vm_state.key_pressed = m_key_pressed;
	vm_state.counter = Int32_LE(m_counter);
	vm_state.remain_count = Int32_LE(remain_count);

	fio->Fwrite(&vm_state_ident, sizeof(vm_state_ident), 1);
	fio->Fwrite(&vm_state, sizeof(vm_state), 1);
}

bool KEYBOARD::load_state(FILEIO *fio)
{
	vm_state_ident_t vm_state_i;
	struct vm_state_st vm_state;

	READ_STATE_CHUNK(fio, vm_state_i, vm_state);

	kb_mode = vm_state.kb_mode;
	kb_nmi = vm_state.kb_nmi;
	lpen_flg = vm_state.lpen_flg;
	lpen_flg_prev = vm_state.lpen_flg_prev;
	lpen_bl = (vm_state.lpen_bl & 1) ? true : false;
	m_key_pressed = vm_state.key_pressed;
	m_counter = Int32_LE(vm_state.counter);
	remain_count = Int32_LE(vm_state.remain_count);
	if (vm_state_i.version >= 2) {
		pConfig->misc_flags = (vm_state.lpen_bl & 2) ? (pConfig->misc_flags | MSK_USELIGHTPEN) : (pConfig->misc_flags & ~MSK_USELIGHTPEN);
	}
	m_key_scan_code = 0;
#ifndef USE_JOYSTICKBIT
	if (FLG_USEPIAJOYSTICK) {
		// set pia a port for joystick
		d_pia->write_io8(1,0);
		d_pia->write_io8(0,0x40);
		d_pia->write_io8(1,4);
		d_pia->write_io8(0,0xcd);
	}
#endif

	return true;
}

// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
uint32_t KEYBOARD::debug_read_io8(uint32_t addr)
{
	uint32_t data = 0;

	if (now_reset) return data;

	switch (addr & 0xffff) {
		case ADDR_KB_NMI:
			// break key (0xffc8)
			data = kb_nmi;
			break;
		case ADDR_LP_FLG:
			// light pen flag (0xffcb)
			data = (lpen_flg & 0x80) | 0x7f;
			break;
		case ADDR_KEYBOARD:
			// keyboard (0xffe0)
			data = m_key_scan_code;
			break;
	}
	return data;
}

void KEYBOARD::debug_event_frame()
{
	// reset switch & break key
	update_special_key();
	// other system keys
	update_system_key();
#ifdef USE_LIGHTPEN
	// light pen
	update_light_pen();
#endif
	// joystick on PIA
	update_joy_pia();
	// joystick on PSG
	update_joy_psg();
}

bool KEYBOARD::debug_write_reg(uint32_t reg_num, uint32_t data)
{
	switch(reg_num) {
	case 0:
		kb_nmi = data ? 0xff : 0;
		if (kb_mode & 0x80) {
			d_board->write_signal(SIG_CPU_NMI, kb_nmi ? SIG_NMI_KEYBREAK_MASK : 0, SIG_NMI_KEYBREAK_MASK);
		}
		return true;
	case 1:
		write_io8(ADDR_KEYBOARD, data);
		return true;
	}
	return false;
}

static const _TCHAR *c_reg_names[] = {
	_T("KBNMI"),
	_T("KBMODE"),
	NULL
};

bool KEYBOARD::debug_write_reg(const _TCHAR *reg, uint32_t data)
{
	uint32_t num = find_debug_reg_name(c_reg_names, reg);
	return debug_write_reg(num, data);
}

void KEYBOARD::debug_regs_info(const _TCHAR *title, _TCHAR *buffer, size_t buffer_len)
{
	UTILITY::tcscpy(buffer, buffer_len, title);
	UTILITY::tcscat(buffer, buffer_len, _T(" Registers:\n"));
	UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%s):%02X"), 0, c_reg_names[0], kb_nmi);
	UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%s):%02X"), 1, c_reg_names[1], kb_mode);
}

#endif

