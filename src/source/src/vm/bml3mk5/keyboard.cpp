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
#include "../../emu.h"
#include "../../simple_ini.h"
#include "display.h"
#include "../pia.h"
#include "../../emumsg.h"
#include "keyrecord.h"
#include "keyboard_bind.h"
#include "../../config.h"
#include "../../utility.h"

#define KEYBOARD_COUNTER_CYCLE	1
#define KEYBOARD_COUNTER_MAX	(0x80 << KEYBOARD_COUNTER_CYCLE)

#define KEYBIND_HEADER	"KEYBIND3"

void KEYBOARD::initialize()
{
	key_stat = emu->key_buffer();
	kb_mode = 0x44;
	scan_code = 0;
	key_scan_code = 0;
	key_pressed = 0;
	kb_nmi = 0x7f;

	mouse_stat = emu->mouse_buffer();
	lpen_flg = 0;
	lpen_flg_prev = 0;
	lpen_bl = false;

#ifdef USE_JOYSTICK
	joy_stat = emu->joy_buffer();
#ifdef USE_PIAJOYSTICK
	joy_pia_sel = 0;
	joy_pia[0] = 0;
	joy_pia[1] = 0;
#endif
#endif

	key_mod = emu->get_key_mod_ptr();
	pause_pressed = false;
	altkey_pressed = false;
	modesw_pressed = false;
	powersw_pressed = false;

	memset(vm_key_stat, 0, sizeof(vm_key_stat));
	emu->set_vm_key_buffer(vm_key_stat, KEYBIND_KEYS);

	counter = 0;
	remain_count = -1;
//	frame_counter = 0;
//	event_counter = 0;

	memcpy(scan2key_map, scan2key_defmap, sizeof(scan2key_defmap));
	memcpy(scan2joy_map, scan2joy_defmap, sizeof(scan2joy_defmap));
#ifdef USE_PIAJOYSTICKBIT
	memcpy(sjoy2joy_map, sjoy2joyb_defmap, sizeof(sjoy2joy_defmap));
#else
	memcpy(sjoy2joy_map, sjoy2joy_defmap, sizeof(sjoy2joy_defmap));
#endif
	for(int i=0; i<KEYBIND_PRESETS; i++) {
		memcpy(scan2key_preset_map[i], scan2key_defmap, sizeof(scan2key_defmap));
		memcpy(scan2joy_preset_map[i], scan2joy_defmap, sizeof(scan2joy_defmap));
#ifdef USE_PIAJOYSTICKBIT
		memcpy(sjoy2joy_preset_map[i], sjoy2joyb_defmap, sizeof(sjoy2joy_defmap));
#else
		memcpy(sjoy2joy_preset_map[i], sjoy2joy_defmap, sizeof(sjoy2joy_defmap));
#endif
	}

	// load keybind
	if (load_ini_file() != true) {
		load_cfg_file();
	}

	// for dialog box
	emu->set_parami(VM::ParamVmKeyMapSize0, 128);
	emu->set_paramv(VM::ParamVmKeyMap0, (void *)kb2scan_map);
	emu->set_parami(VM::ParamVmKeyMapSize1, 128);
	emu->set_paramv(VM::ParamVmKeyMap1, (void *)kb2scan_map);
	emu->set_parami(VM::ParamVmKeyMapSize2, 16);
#ifdef USE_PIAJOYSTICKBIT
	emu->set_paramv(VM::ParamVmKeyMap2, (void *)kb2sjoyb_map);
#else
	emu->set_paramv(VM::ParamVmKeyMap2, (void *)kb2sjoy_map);
#endif

	emu->set_parami(VM::ParamVkKeyMapKeys0, KEYBIND_KEYS);
	emu->set_parami(VM::ParamVkKeyMapKeys1, KEYBIND_KEYS);
	emu->set_parami(VM::ParamVkKeyMapKeys2, KEYBIND_JOYS);
	emu->set_parami(VM::ParamVkKeyMapAssign, KEYBIND_ASSIGN);
	emu->set_paramv(VM::ParamVkKeyDefMap0, (void *)scan2key_defmap);
	emu->set_paramv(VM::ParamVkKeyDefMap1, (void *)scan2joy_defmap);
#ifdef USE_PIAJOYSTICKBIT
	emu->set_paramv(VM::ParamVkKeyDefMap2, (void *)sjoy2joyb_defmap);
#else
	emu->set_paramv(VM::ParamVkKeyDefMap2, (void *)sjoy2joy_defmap);
#endif

	emu->set_paramv(VM::ParamVkKeyMap0, (void *)scan2key_map);
	emu->set_paramv(VM::ParamVkKeyMap1, (void *)scan2joy_map);
	emu->set_paramv(VM::ParamVkKeyMap2, (void *)sjoy2joy_map);

	emu->set_parami(VM::ParamVkKeyPresets, KEYBIND_PRESETS);
	for(int i=0; i<KEYBIND_PRESETS; i++) {
		emu->set_paramv(VM::ParamVkKeyPresetMap00 + i * KEYBIND_MAX_NUM, (void *)scan2key_preset_map[i]);
		emu->set_paramv(VM::ParamVkKeyPresetMap01 + i * KEYBIND_MAX_NUM, (void *)scan2joy_preset_map[i]);
		emu->set_paramv(VM::ParamVkKeyPresetMap02 + i * KEYBIND_MAX_NUM, (void *)sjoy2joy_preset_map[i]);
	}

#ifdef USE_KEY_RECORD
	reckey->set_key_scan_code_ptr(&key_scan_code);
	reckey->set_counter_ptr(&counter);
	reckey->set_remain_count_ptr(&remain_count);
	reckey->set_kb_mode_ptr(&kb_mode);
#endif

	// event
	register_frame_event(this);
}

void KEYBOARD::release()
{
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
bool KEYBOARD::load_ini_file()
{
	const _TCHAR *app_path;
	_TCHAR section[100];

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


	UTILITY::stprintf(section, 100, _T("Keyboard"));
	const CSimpleIniSection *csection = ini->Find(section);
	int count = 0;
	if (csection) {
		memset(scan2key_map, 0, sizeof(scan2key_map));
		count = csection->Count();
	}
	for (int idx = 0; idx < count; idx++) {
		const CSimpleIniItem *item = csection->Item(idx);
		int k = 0;
		int i = 0;
		int rc = _stscanf(item->GetKey(), _T("%02x_%d"), &k, &i);
		if (rc != 2 || k < 0 || k >= KEYBIND_KEYS || i < 0 || i >= KEYBIND_ASSIGN) {
			continue;
		}
		long v = item->GetLong(0);
		scan2key_map[k][i] = (uint32_t)(v & 0xffffffff);
	}
	for (int no = 0; no < KEYBIND_PRESETS; no++) {
		UTILITY::stprintf(section, 100, _T("KeyboardPreset%d"), no + 1);
		csection = ini->Find(section);
		count = 0;
		if (csection) {
			memset(scan2key_preset_map[no], 0, sizeof(scan2key_preset_map[no]));
			count = csection->Count();
		}
		for (int idx = 0; idx < count; idx++) {
			const CSimpleIniItem *item = csection->Item(idx);
			int k = 0;
			int i = 0;
			int rc = _stscanf(item->GetKey(), _T("%02x_%d"), &k, &i);
			if (rc != 2 || k < 0 || k >= KEYBIND_KEYS || i < 0 || i >= KEYBIND_ASSIGN) {
				continue;
			}
			long v = item->GetLong(0);
			scan2key_preset_map[no][k][i] = (uint32_t)(v & 0xffffffff);
		}
	}
	UTILITY::stprintf(section, 100, _T("Joypad"));
	csection = ini->Find(section);
	count = 0;
	if (csection) {
		memset(scan2joy_map, 0, sizeof(scan2joy_map));
		count = csection->Count();
	}
	for (int idx = 0; idx < count; idx++) {
		const CSimpleIniItem *item = csection->Item(idx);
		int k = 0;
		int i = 0;
		int rc = _stscanf(item->GetKey(), _T("%02x_%d"), &k, &i);
		if (rc != 2 || k < 0 || k >= KEYBIND_KEYS || i < 0 || i >= KEYBIND_ASSIGN) {
			continue;
		}
		long v = item->GetLong(0);
		scan2joy_map[k][i] = (uint32_t)(v & 0xffffffff);
	}
	for (int no = 0; no < KEYBIND_PRESETS; no++) {
		UTILITY::stprintf(section, 100, _T("JoypadPreset%d"), no + 1);
		csection = ini->Find(section);
		count = 0;
		if (csection) {
			memset(scan2joy_preset_map[no], 0, sizeof(scan2joy_preset_map[no]));
			count = csection->Count();
		}
		for (int idx = 0; idx < count; idx++) {
			const CSimpleIniItem *item = csection->Item(idx);
			int k = 0;
			int i = 0;
			int rc = _stscanf(item->GetKey(), _T("%02x_%d"), &k, &i);
			if (rc != 2 || k < 0 || k >= KEYBIND_KEYS || i < 0 || i >= KEYBIND_ASSIGN) {
				continue;
			}
			long v = item->GetLong(0);
			scan2joy_preset_map[no][k][i] = (uint32_t)(v & 0xffffffff);
		}
	}
#ifdef USE_PIAJOYSTICK
	UTILITY::stprintf(section, 100, _T("JoypadPIA"));
	csection = ini->Find(section);
	count = 0;
	if (csection) {
		memset(sjoy2joy_map, 0, sizeof(sjoy2joy_map));
		count = csection->Count();
	}
	for (int idx = 0; idx < count; idx++) {
		const CSimpleIniItem *item = csection->Item(idx);
		int k = 0;
		int i = 0;
		int rc = _stscanf(item->GetKey(), _T("%02x_%d"), &k, &i);
		if (rc != 2 || k < 0 || k >= KEYBIND_JOYS || i < 0 || i >= KEYBIND_ASSIGN) {
			continue;
		}
		long v = item->GetLong(0);
		sjoy2joy_map[k][i] = (uint32_t)(v & 0xffffffff);
	}
	for (int no = 0; no < KEYBIND_PRESETS; no++) {
		UTILITY::stprintf(section, 100, _T("JoypadPIAPreset%d"), no + 1);
		csection = ini->Find(section);
		count = 0;
		if (csection) {
			memset(sjoy2joy_preset_map[no], 0, sizeof(sjoy2joy_preset_map[no]));
			count = csection->Count();
		}
		for (int idx = 0; idx < count; idx++) {
			const CSimpleIniItem *item = csection->Item(idx);
			int k = 0;
			int i = 0;
			int rc = _stscanf(item->GetKey(), _T("%02x_%d"), &k, &i);
			if (rc != 2 || k < 0 || k >= KEYBIND_JOYS || i < 0 || i >= KEYBIND_ASSIGN) {
				continue;
			}
			long v = item->GetLong(0);
			sjoy2joy_preset_map[no][k][i] = (uint32_t)(v & 0xffffffff);
		}
	}
#endif
#if 0
	// convert system key (alt) 0x60 -> 0x78
	for (int k = 0x60; k <= 0x61; k++) {
		for (int i = 0; i < KEYBIND_ASSIGN; i++) {
			if (scan2key_map[k+0x18][i] == 0) {
				scan2key_map[k+0x18][i] = scan2key_map[k][i];
			}
			scan2key_map[k][i] = 0;
			if (scan2joy_map[k+0x18][i] == 0) {
				scan2joy_map[k+0x18][i] = scan2joy_map[k][i];
			}
			scan2joy_map[k][i] = 0;
			for (int no = 0; no < KEYBIND_PRESETS; no++) {
				if (scan2key_preset_map[no][k+0x18][i] == 0) {
					scan2key_preset_map[no][k+0x18][i] = scan2key_preset_map[no][k][i];
				}
				scan2key_preset_map[no][k][i] = 0;
				if (scan2joy_preset_map[no][k+0x18][i] == 0) {
					scan2joy_preset_map[no][k+0x18][i] = scan2joy_preset_map[no][k][i];
				}
				scan2joy_preset_map[no][k][i] = 0;
			}
		}
	}
#endif
//	keys.clear();

	delete ini;

	return true;
}

void KEYBOARD::save_ini_file()
{
	const _TCHAR *app_path;
	_TCHAR comment[100];
	_TCHAR section[100];
	_TCHAR key[100];

	CSimpleIni *ini = new CSimpleIni();
//#ifdef _UNICODE
//	ini->SetUnicode(true);
//#endif

	// section
	UTILITY::stprintf(comment, 100, _T("; %s keybind file"), _T(DEVICE_NAME));
	ini->SetValue(_T(""), _T("Version"), _T(VK_KEY_TYPE), comment);

	UTILITY::stprintf(section, 100, _T("Keyboard"));
    for (int k = 0; k < KEYBIND_KEYS; k++) {
		for (int i = 0; i < KEYBIND_ASSIGN; i++) {
			UTILITY::stprintf(key, 100, _T("%02x_%d"), k, i);
			if (scan2key_map[k][i] != 0) {
				ini->SetLongValue(section, key, scan2key_map[k][i], NULL, true);
			}
		}
	}
    for (int no = 0; no < KEYBIND_PRESETS; no++) {
		UTILITY::stprintf(section, 100, _T("KeyboardPreset%d"), no + 1);
		for (int k = 0; k < KEYBIND_KEYS; k++) {
			for (int i = 0; i < KEYBIND_ASSIGN; i++) {
				UTILITY::stprintf(key, 100, _T("%02x_%d"), k, i);
				if (scan2key_preset_map[no][k][i] != 0) {
					ini->SetLongValue(section, key, scan2key_preset_map[no][k][i], NULL, true);
				}
			}
		}
	}
	UTILITY::stprintf(section, 100, _T("Joypad"));
    for (int k = 0; k < KEYBIND_KEYS; k++) {
		for (int i = 0; i < KEYBIND_ASSIGN; i++) {
			UTILITY::stprintf(key, 100, _T("%02x_%d"), k, i);
			if (scan2joy_map[k][i] != 0) {
				ini->SetLongValue(section, key, scan2joy_map[k][i], NULL, true);
			}
		}
	}
    for (int no = 0; no < KEYBIND_PRESETS; no++) {
		UTILITY::stprintf(section, 100, _T("JoypadPreset%d"), no + 1);
		for (int k = 0; k < KEYBIND_KEYS; k++) {
			for (int i = 0; i < KEYBIND_ASSIGN; i++) {
				UTILITY::stprintf(key, 100, _T("%02x_%d"), k, i);
				if (scan2joy_preset_map[no][k][i] != 0) {
					ini->SetLongValue(section, key, scan2joy_preset_map[no][k][i], NULL, true);
				}
			}
		}
	}
#ifdef USE_PIAJOYSTICK
	UTILITY::stprintf(section, 100, _T("JoypadPIA"));
    for (int k = 0; k < KEYBIND_JOYS; k++) {
		for (int i = 0; i < KEYBIND_ASSIGN; i++) {
			UTILITY::stprintf(key, 100, _T("%02x_%d"), k, i);
			if (sjoy2joy_map[k][i] != 0) {
				ini->SetLongValue(section, key, sjoy2joy_map[k][i], NULL, true);
			}
		}
	}
    for (int no = 0; no < KEYBIND_PRESETS; no++) {
		UTILITY::stprintf(section, 100, _T("JoypadPIAPreset%d"), no + 1);
		for (int k = 0; k < KEYBIND_JOYS; k++) {
			for (int i = 0; i < KEYBIND_ASSIGN; i++) {
				UTILITY::stprintf(key, 100, _T("%02x_%d"), k, i);
				if (sjoy2joy_preset_map[no][k][i] != 0) {
					ini->SetLongValue(section, key, sjoy2joy_preset_map[no][k][i], NULL, true);
				}
			}
		}
	}
#endif

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

void KEYBOARD::reset()
{
	kb_mode = 0;
	scan_code = 0;
	key_scan_code = 0;
	key_pressed = 0;
	kb_nmi = 0x7f;

	lpen_flg = 0;
	lpen_flg_prev = 0;
	lpen_bl = false;

	counter = 0;
	remain_count = -1;
	remain_count_max = FLG_ORIG_LIMKEY ? (KEYBOARD_COUNTER_MAX << 3) - 2 : -1;

#ifdef USE_PIAJOYSTICK
	joy_pia_sel = 0;
	joy_pia[0] = 0;
	joy_pia[1] = 0;
#endif
#ifdef _DEBUG_KEYBOARD
	frame_counter = 0;
	event_counter = 0;
#endif
}

void KEYBOARD::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch (id) {
#ifdef _BML3MK5
		case SIG_KEYBOARD_HSYNC:
			if (data & mask) update_keyboard();
			break;
#endif
#ifdef USE_PIAJOYSTICK
#ifdef USE_PIAJOYSTICKBIT
		case SIG_KEYBOARD_PIA_PB:
//			if (FLG_USEPIAJOYSTICK) {
//				d_pia->write_signal(PIA::SIG_PIA_PB, joy_pia[0], 0xff);
//			}
			break;
#else
		case SIG_KEYBOARD_PIA_PA:
			// always set
			joy_pia_sel = (data & 0x40 ? 1 : 0);
			d_pia->write_signal(PIA::SIG_PIA_PA, ~joy_pia[joy_pia_sel], 0x3f);	// negative
			break;
#endif
#endif
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
			if ((kb_mode & 0x40) && (key_scan_code & 0x80) != 0) {
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
			data = key_scan_code;
			key_scan_code &= 0x7f;

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
	scan_code = (key_counter) & ((kb_mode & 0x08) ? 0x07 : 0x7f);
	if (key_scan_code < 0x80) {
		key_scan_code = scan_code;
	}

//	logging->out_debugf("kbd csc fr:%04d ct:%03x rc:%02d s:%02x k:%02x"
//	, frame_counter, counter, remain_count, scan_code, key_scan_code);

}

void KEYBOARD::update_scan_code(int key_counter)
{
	bool pressed = false;

	// if kb bit3 is set, enable scan_code is between 0x00 and 0x07
	scan_code = (key_counter) & ((kb_mode & 0x08) ? 0x07 : 0x7f);
	if (scan_code == 0x7f) {
		if (key_pressed == 1) {
			pressed = true;
			scan_code |= 0x80;
			remain_count = remain_count_max;
		}
		if (key_pressed > 0) key_pressed--;
	} else if (scan_code >= 0x70) {
		// no assaign
	} else {
#ifdef _BML3MK5
		pressed = pressing_key(scan_code);
#endif
		if (pressed == true) {
			scan_code |= 0x80;
			key_pressed = 2;
			remain_count = remain_count_max;
		}
	}

	if (key_scan_code < 0x80) {
		key_scan_code = scan_code;
	} else if (scan_code >= 0x80) {
		key_scan_code = scan_code;
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
				if (pressed && modesw_pressed == false) {
#ifdef _BML3MK5
					emu->change_dipswitch(2);
#endif
				}
				modesw_pressed = pressed;
				break;
			case 0x7e:
				// power switch
				if (pressed	&& powersw_pressed == false) {
					emumsg.Send(EMUMSG_ID_RESET);
				}
				powersw_pressed = pressed;
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
	reckey->processing_lightpen_status(mouse_stat);
#endif

	if ((lpen_bl && FLG_USELIGHTPEN) || config.reckey_playing) {
		lpen_flg = (mouse_stat[2] & 1) ? 0x80 : 0;
		if (lpen_flg != 0) {
			// push light pen
			// LPSTB signal
			d_disp->write_signal(DISPLAY::SIG_DISPLAY_LPSTB
				,((mouse_stat[0] & 0xffff) << 16) | (mouse_stat[1] & 0xffff)
				, 0xffffffff);

			// IRQ interrupt
			d_board->write_signal(SIG_CPU_IRQ, SIG_IRQ_LIGHTPEN_MASK, SIG_IRQ_LIGHTPEN_MASK);

		}
#if 0
		if (config.reckey_playing) {
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
	return (config.now_power_off ? 4 : (kb_mode & 0x07));
}

void KEYBOARD::update_system_key()
{
	bool pressed = false;

	for(int code = 0x78; code <= 0x79; code++) {
		pressed = pressing_key(code);
		switch(code) {
			case 0x78:
				// pause
				if (pressed && pause_pressed == false) {
//					emu->set_pause(2, emu->get_pause(2) ? false : true);
					emumsg.Send(EMUMSG_ID_PAUSE);
				}
				pause_pressed = pressed;
				break;
			case 0x79:
				// alt
				altkey_pressed = pressed;
				*key_mod = (altkey_pressed) ? (*key_mod) | KEY_MOD_ALT_KEY : (*key_mod) & ~KEY_MOD_ALT_KEY;
				break;
		}
	}
}

void KEYBOARD::update_joy_pia()
{
#ifdef USE_PIAJOYSTICK
	uint32_t code = 0;
	joy_pia[0] = 0;
	joy_pia[1] = 0;

	if (FLG_USEPIAJOYSTICK) {
#ifdef USE_PIAJOYSTICKBIT
		for(int n = 0; n < 8; n++) {
			for(int i=0; i<2; i++) {
				code = sjoy2joy_map[n][i];
				if (code == 0) continue;
				if ((joy_stat[i] & code) == code) {
					joy_pia[i] |= (1 << n);
				}
			}
		}
#else
		for(int n = 0; sjoy2joy_allow_map[n] >= 0; n++) {
			for(int i=0; i<2; i++) {
				code = sjoy2joy_map[sjoy2joy_allow_map[n]][i];
				if ((joy_stat[i] & code) == code) {
					joy_pia[i] = sjoy2joy_allow_map[n];
				}
			}
		}
		for(int n = 0; sjoy2joy_button_map[n] >= 0; n++) {
			for(int i=0; i<2; i++) {
				code = sjoy2joy_map[sjoy2joy_button_map[n]][i];
				if (joy_stat[i] & code) {
					joy_pia[i] |= (0x10 << (sjoy2joy_button_map[n] - 0x10));
					// irq
					d_pia->write_signal(i == 0 ? PIA::SIG_PIA_CA1 : PIA::SIG_PIA_CA2,0,1);
				} else {
					d_pia->write_signal(i == 0 ? PIA::SIG_PIA_CA1 : PIA::SIG_PIA_CA2,1,1);
				}
			}
		}
		d_pia->write_signal(PIA::SIG_PIA_PA, ~joy_pia[joy_pia_sel], 0x3f);	// negative
#endif
	}
#ifdef USE_KEY_RECORD
	reckey->processing_joypia_status(joy_pia);
#endif

#ifdef USE_PIAJOYSTICKBIT
	if (FLG_USEPIAJOYSTICK) {
		d_pia->write_signal(PIA::SIG_PIA_PB, sjoy2joy_map[0x17][0] ? ~joy_pia[0] : joy_pia[0], 0xff);
	}
#endif
#endif
}

// ----------------------------------------------------------------------------

bool KEYBOARD::pressing_key(int key_code)
{
	uint32_t code = 0;
	bool pressed = false;
	int i = 0;

	for(i=0; i<2; i++) {
		code = scan2key_map[key_code][i];
		if (code == 0) break;

		// key pressed ?
		if (key_stat[code]) {
			pressed = true;
			break;
		}
	}
#ifdef USE_JOYSTICK
	if (pressed != true && FLG_USEJOYSTICK) {
		for(i=0; i<2; i++) {
			code = scan2joy_map[key_code][i];
			if (code == 0) continue;

			// joypad pressed ?
			// allow key or button
			if (
				((scan2joy_map[0x81][0] & 1) == 0 && ((joy_stat[i] & 0xf & code) == code || (joy_stat[i] & 0xfffffff0 & code) == code))
			||  ((scan2joy_map[0x81][0] & 1) != 0 && ((joy_stat[i] & 0xf) == code || (joy_stat[i] & 0xfffffff0) == code))
			) {
				pressed = true;
				break;
			}
		}
	}
#endif
	if (pressed != true) {
		// auto key pressed ?
		if (vm_key_stat[key_code]) {
			pressed = true;
		}
	}
#ifdef USE_KEY_RECORD
	pressed = reckey->processing_keys(key_code, pressed);
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
		key_scan_code &= 0x7f;
		counter = (counter - 2) % KEYBOARD_COUNTER_MAX;
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
	if (key_scan_code < 0x80) {
		if ((counter & 1) == 0) {
			clear_scan_code(counter >> 1);
		} else {
			update_scan_code(counter >> 1);
		}
		if (!now_reset) counter++;
		if (counter >= KEYBOARD_COUNTER_MAX) counter = 0;
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
	vm_state.key_pressed = key_pressed;
	vm_state.counter = Int32_LE(counter);
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
	key_pressed = vm_state.key_pressed;
	counter = Int32_LE(vm_state.counter);
	remain_count = Int32_LE(vm_state.remain_count);
	if (vm_state_i.version >= 2) {
		config.misc_flags = (vm_state.lpen_bl & 2) ? (config.misc_flags | MSK_USELIGHTPEN) : (config.misc_flags & ~MSK_USELIGHTPEN);
	}
	key_scan_code = 0;
#ifdef USE_PIAJOYSTICK
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
			data = key_scan_code;
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

bool KEYBOARD::debug_write_reg(const _TCHAR *reg, uint32_t data)
{
	return false;
}

void KEYBOARD::debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	buffer[0] = _T('\0');
	UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%s):%02X"), 0, _T("KBNMI"), kb_nmi);
	UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%s):%02X"), 1, _T("KBMODE"), kb_mode);
}

#endif

