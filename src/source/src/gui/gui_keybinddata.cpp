/** @file gui_keybinddata.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.5.12

	@brief [ keybind data for gui ]
*/

#include "gui_keybinddata.h"
#include "../emu.h"
#include "../clocale.h"
#include "../utility.h"
#include "../keycode.h"

KeybindData::KeybindData()
{
	tab_num = 0;
	devtype = 0;
	vm_type = 0;
}

KeybindData::~KeybindData()
{
}

void KeybindData::Init(EMU *emu, int new_tabnum, int new_devtype, int new_vmtype)
{
	tab_num = new_tabnum;
	devtype = new_devtype;
	vm_type = new_vmtype;

	// get parameters
	memset(table, 0, sizeof(table));
	memset(row2idx_map, 0, sizeof(row2idx_map));
	vmkey_map_size = 0;

	vkkey_defmap = NULL;
	vkkey_defmap_rows = 0;
	vkkey_defmap_cols = 0;

	vkkey_map = NULL;
	memset(vkkey_preset_map, 0, sizeof(vkkey_preset_map));

	// set initial data
	int presets = emu->get_parami(VM::ParamVkKeyPresets);
	presets = (presets < KEYBIND_PRESETS ? presets : KEYBIND_PRESETS);

	SetVmKeyMap((uint16_t *)emu->get_paramv(VM::ParamVmKeyMap0 + tab_num), emu->get_parami(VM::ParamVmKeyMapSize0 + tab_num));

	SetVkKeyDefMap((uint32_t *)emu->get_paramv(VM::ParamVkKeyDefMap0 + tab_num), emu->get_parami(VM::ParamVkKeyMapKeys0 + tab_num), emu->get_parami(VM::ParamVkKeyMapAssign));

	SetVkKeyMap((uint32_t *)emu->get_paramv(VM::ParamVkKeyMap0 + tab_num));

	for(int j=0; j<presets; j++) {
		SetVkKeyPresetMap((uint32_t *)emu->get_paramv(VM::ParamVkKeyPresetMap00 + tab_num + j * KEYBIND_MAX_NUM), j);
	}

	// set disable key
	max_rows = 0;
	for(int idx=0; idx<KBCTRL_MAX_LINES; idx++) {
		if (table[idx].vm_keycode >= vkkey_defmap_rows) {
			table[idx].enabled = false;
		}

		if (table[idx].enabled) {
			table[idx].row = max_rows;
			row2idx_map[max_rows] = idx;
			max_rows++;
		} else {
			table[idx].row = -1;
		}

		uint32_t code = 0;
		for(int col=0; col<vkkey_defmap_cols; col++) {
			table[idx].cols[col].col = col;
			if (table[idx].enabled) {
				code = *(vkkey_map + table[idx].vm_keycode * vkkey_defmap_cols + col);
			}
			if (devtype == 1) SetVkJoyCode(&table[idx].cols[col],code,NULL);
			else SetVkKeyCode(&table[idx].cols[col],code,NULL);
		}
	}
}

/// @brief Set key map for virtual machine
///
/// @param[in] vmKeyMap : array of key map
/// @param[in] size     : line of key map
void KeybindData::SetVmKeyMap(uint16_t *vmKeyMap, int size)
{
	if (tab_num < 0 || tab_num >= KEYBIND_MAX_NUM) return;

	for(int i=0; i<KBCTRL_MAX_LINES && i<size; i++) {
		SetVmKeyCode(i,vmKeyMap[i]);
	}
	vmkey_map_size = (size > KBCTRL_MAX_LINES) ? KBCTRL_MAX_LINES : size;
}

/// @brief Set code for virtual machine
///
/// @param[in] idx  : index of key map
/// @param[in] code : key code
void KeybindData::SetVmKey(int idx, uint16_t code)
{
	if (tab_num < 0 || tab_num >= KEYBIND_MAX_NUM) return;

	//	vmkey_map[idx] = code;
	SetVmKeyCode(idx,code);
}

/// @brief Set code for virtual machine
///
/// @param[in] idx  : index of key map
/// @param[in] code : key code
/// @return true
bool KeybindData::SetVmKeyCode(int idx, uint16_t code)
{
	if (idx < 0 || idx >= KBCTRL_MAX_LINES) {
		return false;
	}
	if (code >= 0xff) {
		table[idx].enabled = false;
	} else {
		table[idx].enabled = true;
	}

	table[idx].vm_keycode = code;

	return true;
}

/// @brief Set key map for host machine
///
/// @param[in] vkKeyMap : array of key map
void KeybindData::SetVkKeyMap(uint32_t *vkKeyMap)
{
	if (tab_num < 0 || tab_num >= KEYBIND_MAX_NUM) return;

	vkkey_map = vkKeyMap;
}

/// @brief Set default key map for host machine
///
/// @param[in] vkKeyDefMap : array of key map
/// @param[in] rows        : rows of key map
/// @param[in] cols        : cols of key map
void KeybindData::SetVkKeyDefMap(uint32_t *vkKeyDefMap, int rows, int cols)
{
	if (tab_num < 0 || tab_num >= KEYBIND_MAX_NUM) return;

	vkkey_defmap = vkKeyDefMap;
	vkkey_defmap_rows = rows;
	vkkey_defmap_cols = cols;
}

/// @brief Set preset key map for host machine
///
/// @param[in] vkKeyMap : array of key map
/// @param[in] idx      : preset number
void KeybindData::SetVkKeyPresetMap(uint32_t *vkKeyMap, int idx)
{
	if (tab_num < 0 || tab_num >= KEYBIND_MAX_NUM) return;

	if (idx < 4) {
		vkkey_preset_map[idx] = vkKeyMap;
	}
}

/// @brief Set key code for host machine
///
/// @param[in] obj    : columns object
/// @param[in] code   : key code
/// @param[out] label : label string of specified code
/// @return true
bool KeybindData::SetVkKeyCode(codecols_t *obj, uint32_t code, _TCHAR *label)
{
	if (label != NULL) GetVkKeyLabel(code,label,true);
	obj->vk_prev_keycode = obj->vk_keycode;
	obj->vk_keycode = code;

	return true;
}

/// @brief Set key code for host machine
///
/// @param[in] row    : row in key map
/// @param[in] col    : col in key map
/// @param[in] code   : key code
/// @param[out] label : label string of specified code
/// @return true
bool KeybindData::SetVkKeyCode(int row, int col, uint32_t code, _TCHAR *label)
{
	int idx = row2idx_map[row];
	codecols_t *obj = &table[idx].cols[col];
	return SetVkKeyCode(obj,code,label);
}

/// @brief Set joystick code for host machine
///
/// @param[in] obj    : columns object
/// @param[in] code   : joystick code
/// @param[out] label : label string of specified code
/// @return true
bool KeybindData::SetVkJoyCode(codecols_t *obj, uint32_t code, _TCHAR *label)
{
	bool rc = true;
	if (label != NULL) {
		rc = GetVkJoyLabel(code,label,true);
	}
	if (rc == true) {
		obj->vk_prev_keycode = obj->vk_keycode;
		obj->vk_keycode = code;
	}
	return true;
}

/// @brief Set joystick code for host machine
///
/// @param[in] row    : row in key map
/// @param[in] col    : col in key map
/// @param[in] code   : joystick code
/// @param[out] label : label string of specified code
/// @return true
bool KeybindData::SetVkJoyCode(int row, int col, uint32_t code, _TCHAR *label)
{
	int idx = row2idx_map[row];
	codecols_t *obj = &table[idx].cols[col];
	return SetVkJoyCode(obj,code,label);
}

/// @brief Set code for host machine
///
/// @param[in] row    : row in key map
/// @param[in] col    : col in key map
/// @param[in] code   : code
/// @param[out] label : label string of specified code
/// @return true
bool KeybindData::SetVkCode(int row, int col, uint32_t code, _TCHAR *label)
{
	if (devtype == 1) {
		return SetVkJoyCode(row, col, code, label);
	} else {
		return SetVkKeyCode(row, col, code, label);
	}
}

/// @brief get combination flag
///
/// @return value
uint32_t KeybindData::GetCombi()
{
	return *(vkkey_map + (vkkey_defmap_rows - 1) * vkkey_defmap_cols);
}

/// @brief set combination flag
///
/// @param[in] value
void KeybindData::SetCombi(uint32_t value)
{
	*(vkkey_map + (vkkey_defmap_rows - 1) * vkkey_defmap_cols) = value;
}

/// @brief get cell string
///
/// @param[in] row    : row in key map
/// @param[in] col    : col in key map
/// @return label string
const _TCHAR *KeybindData::GetCellString(int row, int col)
{
	static _TCHAR label[128];
	GetCellString(row, col, label);
	return label;
}

/// @brief get cell string
///
/// @param[in] row    : row in key map
/// @param[in] col    : col in key map
/// @param[out] label : string
/// @return true / false
bool KeybindData::GetCellString(int row, int col, _TCHAR *label)
{
	bool rc = false;
	int idx = row2idx_map[row];
	if(col < 0) {
		if (vm_type == 2) rc = GetVmJoyBitLabel(idx, label, true);
		else if (vm_type == 1) rc = GetVmJoyLabel(idx, label, true);
		else rc = GetVmKeyLabel(idx, label, true);
	} else {
		if (devtype == 1) rc = GetVkJoyLabel(table[idx].cols[col].vk_keycode,label,true);
		else rc = GetVkKeyLabel(table[idx].cols[col].vk_keycode,label,true);
	}
	return rc;
}

/// @brief table is valid ?
///
/// @param[in] index : table number
bool KeybindData::IsEnable(int index)
{
	return table[index].enabled;
}

/// @brief load default preset
void KeybindData::LoadDefaultPreset()
{
	LoadPreset(-1);
}

/// @brief load preset
///
/// @param[in] idx : preset number
void KeybindData::LoadPreset(int idx)
{
	int rows = vmkey_map_size;
	int cols = vkkey_defmap_cols;
	uint32_t *dst;
	uint32_t code;

	if (0 <= idx && idx <= KEYBIND_PRESETS) {
		dst = vkkey_preset_map[idx];
	} else {
		dst = vkkey_defmap;
	}

	for(int row=0; row<rows; row++) {
		if (table[row].enabled) {
			for(int col=0; col<cols; col++) {
				code = dst[table[row].vm_keycode * cols + col];
				if (devtype == 1) SetVkJoyCode(&table[row].cols[col],code,NULL);
				else SetVkKeyCode(&table[row].cols[col],code,NULL);
			}
		}
	}
	if (devtype == 1 && (vm_type == 0 || vm_type == 2)) {
		SetCombi(dst[(vkkey_defmap_rows - 1) * cols]);
	}
}

/// @brief save preset
///
/// @param[in] idx : preset number
void KeybindData::SavePreset(int idx)
{
	int rows = vmkey_map_size;
	int cols = vkkey_defmap_cols;
	uint32_t *dst;
	uint32_t code;

	if (0 <= idx && idx <= KEYBIND_PRESETS) {
		dst = vkkey_preset_map[idx];
	} else {
		return;
	}

	for(int row=0; row<rows; row++) {
		if (table[row].enabled) {
			for(int col=0; col<cols; col++) {
				code = table[row].cols[col].vk_keycode;
				dst[table[row].vm_keycode * cols + col] = code;
			}
		}
	}
	if (devtype == 1 && (vm_type == 0 || vm_type == 2)) {
		dst[(vkkey_defmap_rows - 1) * cols] = GetCombi();
	}
}

/// @brief set data to table
void KeybindData::SetData()
{
	int rows = vmkey_map_size;
	int cols = vkkey_defmap_cols;
	uint32_t *dst = vkkey_map;
	uint32_t code = 0;

	for(int row=0; row<rows; row++) {
		if (table[row].enabled) {
			for(int col=0; col<cols; col++) {
				code = table[row].cols[col].vk_keycode;
				dst[table[row].vm_keycode * cols + col] = code;
			}
		}
	}
}

/// array of vm key label
static const struct {
	int id;
	const _TCHAR *label;
} sVmKeyLabels[] = {
	// 0x00
	{ -1, _T("") },
	{ -1, _T("BREAK") },
	{ -1, _T("RESET") },
	{ -1, _T("MODE") },
	{ CMsg::Bracket_Pause, _T("(Pause)") },
	// 0x05
#if defined(__APPLE__) && defined(__MACH__)
	{ -1, _T("(option)") },
#else
	{ -1, _T("(Alt)") },
#endif
	// 0x06
#ifdef _BML3MK5
	{ -1, _T("INS/DEL") },
#else
	{ -1, _T("INS") },
#endif
	// 0x07
#ifdef _BML3MK5
	{ CMsg::num_INS_DEL, _T("num INS/DEL") },
#else
	{ -1, _T("DEL") },
#endif
	{ -1, _T("BS") },
	{ -1, _T("TAB") },
	{ -1, _T("") },
	{ -1, _T("") },
	{ -1, _T("HOME/CLS") },
	{ -1, _T("RETURN") },
	{ -1, _T("ENTER") },
	{ -1, _T("HELP") },
	// 0x10
	{ -1, _T("SHIFT") },
	{ -1, _T("CTRL") },
	{ -1, _T("") },
	{ -1, _T("") },
	{ -1, _T("CAPS LOCK") },
	{ CMsg::KATA_HIRA, _T("KATA/HIRA") },
	{ -1, _T("GRAPH") },
	{ CMsg::HENKAN, _T("HENKAN") },
	{ CMsg::MUHENKAN, _T("MUHENKAN") },
	{ -1, _T("COMMAND") },
	{ -1, _T("OPTION") },
	{ -1, _T("ESC") },
	{ CMsg::Allow_RIGHT, _T("RIGHT") },
	{ CMsg::Allow_LEFT, _T("LEFT") },
	{ CMsg::Allow_UP, _T("UP") },
	{ CMsg::Allow_DOWN, _T("DOWN") },
	// 0x20
	{ CMsg::SPACE, _T("SPACE") },
	// 0x21 - 0x28  button1 - 8
	{ -1, _T("") },
	{ -1, _T("") },
	{ -1, _T("") },
	{ -1, _T("") },
	{ -1, _T("") },
	{ -1, _T("") },
	{ -1, _T("") },
	{ -1, _T("") },
	// 0x29 - 0x2d  reserved
	{ -1, _T("") },
	{ -1, _T("") },
	{ -1, _T("") },
	{ -1, _T("") },
	{ -1, _T("") },
	// 0x2e
#ifdef _MBS1
	{ -1, _T("COPY") },
	{ CMsg::num_Comma, _T("num ,") },
#else
	{ -1, _T("") },
	{ -1, _T("") },
#endif
	// 0x30
	{ -1, _T("0") },
	{ -1, _T("1") },
	{ -1, _T("2") },
	{ -1, _T("3") },
	{ -1, _T("4") },
	{ -1, _T("5") },
	{ -1, _T("6") },
	{ -1, _T("7") },
	{ -1, _T("8") },
	{ -1, _T("9") },
	{ -1, _T(": *") },
	{ -1, _T("; +") },
	{ -1, _T(", <") },
	{ -1, _T("- =") },
	{ -1, _T(". >") },
	{ -1, _T("/ ?") },
	// 0x40
	{ -1,_T(" @ ") },
	{ -1, _T("A") },
	{ -1, _T("B") },
	{ -1, _T("C") },
	{ -1, _T("D") },
	{ -1, _T("E") },
	{ -1, _T("F") },
	{ -1, _T("G") },
	{ -1, _T("H") },
	{ -1, _T("I") },
	{ -1, _T("J") },
	{ -1, _T("K") },
	{ -1, _T("L") },
	{ -1, _T("M") },
	{ -1, _T("N") },
	{ -1, _T("O") },
	// 0x50
	{ -1, _T("P") },
	{ -1, _T("Q") },
	{ -1, _T("R") },
	{ -1, _T("S") },
	{ -1, _T("T") },
	{ -1, _T("U") },
	{ -1, _T("V") },
	{ -1, _T("W") },
	{ -1, _T("X") },
	{ -1, _T("Y") },
	{ -1, _T("Z") },
	{ -1,  _T(" [ ") },
	{ -1,  _T(" \\ ") },
	{ -1,  _T(" ] ") },
	{ -1,  _T(" ^ ") },
	{ -1,  _T(" _ ") },
	// 0x60 - 0x69  num 0 - num 9
	{ -1, _T("") },
	{ -1, _T("") },
	{ -1, _T("") },
	{ -1, _T("") },
	{ -1, _T("") },
	{ -1, _T("") },
	{ -1, _T("") },
	{ -1, _T("") },
	{ -1, _T("") },
	{ -1, _T("") },
	// 0x6a
	{ CMsg::num_Multiply, _T("num *") },
	{ CMsg::num_Plus, _T("num +") },
	{ CMsg::num_Question, _T("num ?") },
	{ CMsg::num_Minus, _T("num -") },
	{ CMsg::num_Point, _T("num .") },
	{ CMsg::num_Devide, _T("num /") },
	// 0x70 - 0x7f  PF1 - PF16
	{ -1, NULL }
};

/// @brief get key label for virtual machine
///
/// @param[in]  code      : key code
/// @param[out] label
/// @param[in]  translate : translate by locale
/// @return true if known key code
bool KeybindData::GetVmKeyLabel(int code, _TCHAR *label, bool translate) {
	bool known_key = true;
	const _TCHAR *p = NULL;
	const _TCHAR *fp = NULL;

	if (label) {
		label[0] = '\0';
	}

	if (0x30 <= code && code <= 0x39) {
		// 0 - 9
		UTILITY::stprintf(label, KBLABEL_MAXLEN, _T("%c") ,code);
	} else if (0x41 <= code && code <= 0x5a) {
		// A - Z
		UTILITY::stprintf(label, KBLABEL_MAXLEN, _T("%c"), code);
	} else if (0x60 <= code && code <= 0x69) {
		// num 0 - num 9
		fp = gMessages.Get(CMsg::num_VCHAR, translate);
		UTILITY::stprintf(label, KBLABEL_MAXLEN, fp, code - 0x30);
	} else if (0x70 <= code && code <= 0x7f) {
		// PF1 - PF16
		fp = gMessages.Get(CMsg::PF_VDIGIT, translate);
		UTILITY::stprintf(label, KBLABEL_MAXLEN, fp, code - 0x6f);
	} else if (0 <= code && code < 0x70) {
		CMsg::Id id = (CMsg::Id)sVmKeyLabels[code].id;
		if (id > 0 && translate) {
			p = gMessages.Get(id);
		} else {
			p = sVmKeyLabels[code].label;
		}
		UTILITY::tcscpy(label, KBLABEL_MAXLEN, p);
	} else {
		UTILITY::stprintf(label, KBLABEL_MAXLEN, _T("0x%02x") ,code);
		known_key = false;
	}

	return known_key;
}

/// array of vm joystick label
static const struct {
	int id;
	const _TCHAR *label;
} sVmJoyLabels[] = {
	{ -1, _T("") },
	{ CMsg::up,         _T("up") },
	{ CMsg::up_right,   _T("up+right") },
	{ CMsg::right,      _T("right") },
	{ CMsg::down_right, _T("down+right") },
	{ CMsg::down,       _T("down") },
	{ CMsg::down_left,  _T("down+left") },
	{ CMsg::left,       _T("left") },
	{ CMsg::up_left,    _T("up+left") },
	{ CMsg::button,     _T("button") },
	{ -1, NULL }
};

/// @brief get joystick label for virtual machine
///
/// @param[in]  code      : joystick code
/// @param[out] label
/// @param[in]  translate : translate by locale
/// @return true if known key code
bool KeybindData::GetVmJoyLabel(int code, _TCHAR *label, bool translate) {
	bool known_key = true;
	const _TCHAR *p = NULL;

	*label = _T('\0');

	if (0 <= code && code < 10) {
		CMsg::Id id = (CMsg::Id)sVmJoyLabels[code].id;
		if (id > 0) {
			p = gMessages.Get(id, translate);
		} else {
			p = sVmJoyLabels[code].label;
		}
		UTILITY::tcscpy(label, KBLABEL_MAXLEN, p);
	} else if (code >= 0x0a && code <= 0x0f) {
		p = gMessages.Get(CMsg::button_VCHAR);
		UTILITY::stprintf(label, KBLABEL_MAXLEN, p, code + 0x41 - 0x0a);
	} else {
		*label = _T('\0');
		known_key = false;
	}

	return known_key;
}

/// @brief get joystick label for virtual machine
///
/// @param[in]  code      : joystick code
/// @param[out] label
/// @param[in]  translate : translate by locale
/// @return true if known key code
bool KeybindData::GetVmJoyBitLabel(int code, _TCHAR *label, bool translate) {
	bool known_key = true;
	const _TCHAR *fp = NULL;

	*label = _T('\0');

	if (code >= 0x00 && code <= 0x07) {
		fp = gMessages.Get(CMsg::bit_VDIGIT, translate);
		UTILITY::stprintf(label, KBLABEL_MAXLEN, fp, code);
	} else {
		*label = _T('\0');
		known_key = false;
	}
	return known_key;
}

/// @brief get key label for host machine
///
/// @param[in]  code      : key code
/// @param[out] label
/// @param[in]  translate : translate by locale
/// @return true if known key code
bool KeybindData::GetVkKeyLabel(uint32_t code, _TCHAR *label, bool translate) {
	bool known_key = true;
	const _TCHAR *p = NULL;
	const _TCHAR *fp = NULL;
	int id = -1;

	*label = _T('\0');

	switch(code) {
		case 0:
			p = _T("");
			break;
		case KEYCODE_BACKSPACE:
#if defined(__APPLE__) && defined(__MACH__)
			p = _T("delete");
#else
			p = _T("back space");
#endif
			break;
		case KEYCODE_TAB:
			p = _T("tab");
			break;
		case KEYCODE_CLEAR:
			p = _T("clear");
			break;
		case KEYCODE_RETURN:
#if defined(__APPLE__) && defined(__MACH__)
			p = _T("return");
#else
			p = _T("enter");
#endif
			break;
		case KEYCODE_PAUSE:
			p = _T("pause");
			break;

		case KEYCODE_ESCAPE:
			p = _T("esc");
			break;
		case KEYCODE_SPACE:
			p = _T("space");
			id = CMsg::space;
			break;

		case KEYCODE_COMMA:
			p = _T(", <");		// JP
//			id = CMsg::comma_smaller;
			break;
		case KEYCODE_MINUS:
			p = _T("- =");		// JP
//			id = CMsg::minus_equal;
			break;
		case KEYCODE_PERIOD:
			p = _T(". >");		// JP
//			id = CMsg::period_greater;
			break;
		case KEYCODE_SLASH:
			p = _T("/ ?");		// JP
//			id = CMsg::slash_question;
			break;

		case KEYCODE_COLON:
			p = _T(": *");		// JP
//			id = CMsg::colon_asterisk;
			break;
		case KEYCODE_SEMICOLON:
			p = _T("; +");		// JP
//			id = CMsg::semicolon_plus;
			break;
		case KEYCODE_AT:
			p = _T("@ `");		// JP
//			id = CMsg::at_quote;
			break;
		case KEYCODE_LBRACKET:
			p = _T("[ {");		// JP
//			id = CMsg::lbracket;
			break;
		case KEYCODE_BACKSLASH:
			p = _T("\\ |");	// JP
//			id = CMsg::backslash_bar;
			break;
		case KEYCODE_RBRACKET:
			p = _T("] }");		// JP
//			id = CMsg::rbracket;
			break;
		case KEYCODE_CARET:
			p = _T("^ ~");		// JP
//			id = CMsg::caret_tilde;
			break;
		case KEYCODE_UNDERSCORE:
			p = _T(" _ ");		// JP
//			id = CMsg::underscore;
			break;
		case KEYCODE_GRAVE:
			p = _T("kanji");	// JP
			id = CMsg::Kanji;
			break;

		case KEYCODE_DELETE:
#if defined(__APPLE__) && defined(__MACH__)
			p = _T("delete x");
#else
			p = _T("delete");
#endif
			break;

		case KEYCODE_HENKAN:	// JP
			p = _T("henkan");
			id = CMsg::henkan;
			break;
		case KEYCODE_MUHENKAN:	// JP
			p = _T("muhenkan");
			id = CMsg::muhenkan;
			break;
		case KEYCODE_KATAHIRA:	// JP
			p = _T("katakana");
			id = CMsg::katakana;
			break;
		case KEYCODE_EISU:		// JP
			p = _T("eisu");
			id = CMsg::eisu;
			break;
		case KEYCODE_KANA:		// JP
			p = _T("kana");
			id = CMsg::kana;
			break;

		case KEYCODE_KP_PERIOD:
			p = _T("num .");
			id = CMsg::num_Point;
			break;
		case KEYCODE_KP_DIVIDE:
			p = _T("num /");
			id = CMsg::num_Devide;
			break;
		case KEYCODE_KP_MULTIPLY:
			p = _T("num *");
			id = CMsg::num_Multiply;
			break;
		case KEYCODE_KP_MINUS:
			p = _T("num -");
			id = CMsg::num_Minus;
			break;
		case KEYCODE_KP_PLUS:
			p = _T("num +");
			id = CMsg::num_Plus;
			break;
		case KEYCODE_KP_ENTER:
			p = _T("num enter");
			id = CMsg::num_Enter;
			break;
		case KEYCODE_KP_EQUALS:
			p = _T("num =");
			id = CMsg::num_Equal;
			break;
		case KEYCODE_KP_COMMA:
			p = _T("num ,");
			id = CMsg::num_Comma;
			break;

		case KEYCODE_UP:
			p = _T("up");
			id = CMsg::up;
			break;
		case KEYCODE_DOWN:
			p = _T("down");
			id = CMsg::down;
			break;
		case KEYCODE_RIGHT:
			p = _T("right");
			id = CMsg::right;
			break;
		case KEYCODE_LEFT:
			p = _T("left");
			id = CMsg::left;
			break;
		case KEYCODE_INSERT:
			p = _T("insert");
			break;
		case KEYCODE_HOME:
			p = _T("home");
			break;
		case KEYCODE_END:
			p = _T("end");
			break;
		case KEYCODE_PAGEUP:
			p = _T("page up");
			break;
		case KEYCODE_PAGEDOWN:
			p = _T("page down");
			break;

		case KEYCODE_NUMLOCK:
#if defined(__APPLE__) && defined(__MACH__)
			p = _T("clear");
#else
			p = _T("numlock");
#endif
			break;
		case KEYCODE_CAPSLOCK:
			p = _T("caps lock");
			break;
		case KEYCODE_SCROLLLOCK:
			p = _T("scroll lock");
			break;
		case KEYCODE_RSHIFT:
			p = _T("right shift");
			id = CMsg::right_shift;
			break;
		case KEYCODE_LSHIFT:
			p = _T("left shift");
			id = CMsg::left_shift;
			break;
		case KEYCODE_RCTRL:
			p = _T("right ctrl");
			id = CMsg::right_ctrl;
			break;
		case KEYCODE_LCTRL:
			p = _T("left ctrl");
			id = CMsg::left_ctrl;
			break;
		case KEYCODE_RALT:
#if defined(__APPLE__) && defined(__MACH__)
			p = _T("right option");
			id = CMsg::right_option;
#else
			p = _T("right alt");
			id = CMsg::right_alt;
#endif
			break;
		case KEYCODE_LALT:
#if defined(__APPLE__) && defined(__MACH__)
			p = _T("left option");
			id = CMsg::left_option;
#else
			p = _T("left alt");
			id = CMsg::left_alt;
#endif
			break;
		case KEYCODE_RGUI:
#if defined(__APPLE__) && defined(__MACH__)
			p = _T("right command");
			id = CMsg::right_command;
#else
			p = _T("right meta");
			id = CMsg::right_meta;
#endif
			break;
		case KEYCODE_LGUI:
#if defined(__APPLE__) && defined(__MACH__)
			p = _T("left command");
			id = CMsg::left_command;
#else
			p = _T("left meta");
			id = CMsg::left_meta;
#endif
			break;
		case KEYCODE_MODE:
			p = _T("mode");
			break;
		case KEYCODE_HELP:
			p = _T("help");
			break;
		case KEYCODE_SYSREQ:
			p = _T("sys req");
			break;
		case KEYCODE_MENU:
			p = _T("menu");
			break;
		case KEYCODE_RSUPER:
			p = _T("right win");
			id = CMsg::right_win;
			break;
		case KEYCODE_LSUPER:
			p = _T("left win");
			id = CMsg::left_win;
			break;
		case KEYCODE_COMPOSE:
			p = _T("compose");
			break;
		case KEYCODE_PRINT:
			p = _T("print");
			break;
		case KEYCODE_0:
			p = _T("0");
			break;
		case KEYCODE_KP_0:
			p = _T("num 0");
			id = CMsg::num_0;
			break;
		case KEYCODE_FUNCTION:
			p = _T("fn");
			break;
		default:
			if (KEYCODE_1 <= code && code <= KEYCODE_9) {
				// 1 - 9
				UTILITY::stprintf(label, KBLABEL_MAXLEN, _T("%d"), code + 1 - KEYCODE_1);
			} else if (KEYCODE_A <= code && code <= KEYCODE_Z) {
				// A - Z
				UTILITY::stprintf(label, KBLABEL_MAXLEN, _T("%c"), code + 0x41 - KEYCODE_A);
			} else if (KEYCODE_KP_1 <= code && code <= KEYCODE_KP_9) {
				// num 1 - num 9
				fp = gMessages.Get(CMsg::num_VDIGIT, translate);
				UTILITY::stprintf(label, KBLABEL_MAXLEN, fp, code + 1 - KEYCODE_KP_1);
			} else if (KEYCODE_F1 <= code && code <= KEYCODE_F12) {
				// F1 - F12
				fp = _T("F%d");
				UTILITY::stprintf(label, KBLABEL_MAXLEN, fp, code + 1 - KEYCODE_F1);
			} else if (KEYCODE_F13 <= code && code <= KEYCODE_F15) {
				// F13 - F15
				fp = _T("F%d");
				UTILITY::stprintf(label, KBLABEL_MAXLEN, fp, code + 13 - KEYCODE_F13);
			} else if (KEYCODE_F16 <= code && code <= KEYCODE_F19) {
				// F16 - F19
				fp = _T("F%d");
				UTILITY::stprintf(label, KBLABEL_MAXLEN, fp, code + 16 - KEYCODE_F16);
			} else {
				UTILITY::stprintf(label, KBLABEL_MAXLEN, _T("0x%02x"), code);
				known_key = false;
			}
			break;
	}
	if (p != NULL) {
		if (id > 0) {
			p = gMessages.Get((CMsg::Id)id, translate);
		}
		UTILITY::tcscpy(label, KBLABEL_MAXLEN, p);
	}

	return known_key;
}

/// @brief get joystick label for host machine
///
/// @param[in]  code      : joystick code
/// @param[out] label
/// @param[in]  translate : translate by locale
/// @return true if known key code
bool KeybindData::GetVkJoyLabel(uint32_t code, _TCHAR *label, bool translate) {
	bool known_key = true;
	_TCHAR numstr[8];
	const _TCHAR *p = NULL;
	const _TCHAR *fp = NULL;
	int id = -1;

	*label = _T('\0');

	switch(code) {
		case 0x00:
			p = _T("");
			break;
		case 0x01:
			p = _T("up");
			id = CMsg::up;
			break;
		case 0x02:
			p = _T("down");
			id = CMsg::down;
			break;
		case 0x04:
			p = _T("left");
			id = CMsg::left;
			break;
		case 0x05:
			p = _T("up+left");
			id = CMsg::up_left;
			break;
		case 0x06:
			p = _T("down+left");
			id = CMsg::down_left;
			break;
		case 0x08:
			p = _T("right");
			id = CMsg::right;
			break;
		case 0x09:
			p = _T("up+right");
			id = CMsg::up_right;
			break;
		case 0x0a:
			p = _T("down+right");
			id = CMsg::down_right;
			break;
		default:
			// button
			if (code & 0xfffffff0) {
				fp = gMessages.Get(CMsg::button, translate);
				UTILITY::tcscpy(label, KBLABEL_MAXLEN, fp);
				for(int i=0; i<28; i++) {
					if (code & (0x10 << i)) {
						UTILITY::stprintf(numstr, 8, _T(" %d"), i+1);
						UTILITY::tcscat(label, KBLABEL_MAXLEN, numstr);
					}
				}
			} else {
				*label = _T('\0');
				known_key = false;
			}
			break;
	}
	if (p != NULL) {
		if (id > 0) {
			p = gMessages.Get((CMsg::Id)id, translate);
		}
		UTILITY::tcscpy(label, KBLABEL_MAXLEN, p);
	}

	return known_key;
}
