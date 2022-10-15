/** @file gui_keybinddata.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.5.12

	@brief [ keybind data for gui ]
*/

#ifndef GUI_KEYBINDDATA_H
#define GUI_KEYBINDDATA_H

#include "../common.h"
#include "../vm/vm.h"

#define KBLABEL_MAXLEN	100

#define KBCTRL_MAX_LINES 128
#define KBCTRL_MAX_COLS  2

/// keycode on a column in keybind data
typedef struct codecols_st {
	int     col;
	uint32_t  vk_keycode;
	uint32_t  vk_prev_keycode;
} codecols_t;

/// keycodes per row in keybind data
typedef struct codetable_st {
	int        row;
	codecols_t cols[KBCTRL_MAX_COLS];
	uint16_t     vm_keycode;
	bool       enabled;
} codetable_t;

class EMU;

/// keybind data for GUI
class KeybindData
{
protected:
	int max_rows;

	codetable_t table[KBCTRL_MAX_LINES];
	int     vmkey_map_size;
	int		row2idx_map[KBCTRL_MAX_LINES];

	uint32_t *vkkey_defmap;
	int     vkkey_defmap_rows;
	int     vkkey_defmap_cols;

	uint32_t *vkkey_map;
	uint32_t *vkkey_preset_map[KEYBIND_PRESETS];

public:
	int tab_num;
	int devtype;	// 1:joypad 0:keyboard
	int vm_type;	// 2:PIA(SKIPPER) 1:PIA(S1) 0:key

	KeybindData();
	virtual ~KeybindData();

	virtual void Init(EMU *emu, int new_tabnum, int new_devtype, int new_vmtype);

	virtual void SetVmKeyMap(uint16_t *vmKeyMap, int size);
	virtual void SetVmKey(int idx, uint16_t code);
	virtual bool SetVmKeyCode(int idx, uint16_t code);
	virtual void SetVkKeyMap(uint32_t *vkKeyMap);
	virtual void SetVkKeyDefMap(uint32_t *vkKeyDefMap, int rows, int cols);
	virtual void SetVkKeyPresetMap(uint32_t *vkKeyMap, int idx);
	virtual bool SetVkKeyCode(codecols_t *obj, uint32_t code, _TCHAR *label);
	virtual bool SetVkKeyCode(int row, int col, uint32_t code, _TCHAR *label);
	virtual bool SetVkJoyCode(codecols_t *obj, uint32_t code, _TCHAR *label);
	virtual bool SetVkJoyCode(int row, int col, uint32_t code, _TCHAR *label);
	virtual bool SetVkCode(int row, int col, uint32_t code, _TCHAR *label);

	virtual uint32_t GetCombi();
	virtual void SetCombi(uint32_t value);

	virtual void LoadDefaultPreset();
	virtual void LoadPreset(int idx);
	virtual void SavePreset(int idx);

	virtual void SetData();

	virtual int GetNumberOfRows() { return max_rows; }
	virtual int GetNumberOfColumns() { return vkkey_defmap_cols; }
	virtual const _TCHAR *GetCellString(int row, int col);
	virtual bool GetCellString(int row, int col, _TCHAR *label);
	virtual bool IsEnable(int index);

	static bool GetVmKeyLabel(int code, _TCHAR *label, bool translate = false);
	static bool GetVmJoyLabel(int code, _TCHAR *label, bool translate = false);
	static bool GetVmJoyBitLabel(int code, _TCHAR *label, bool translate = false);
	static bool GetVkKeyLabel(uint32_t code, _TCHAR *label, bool translate = false);
	static bool GetVkJoyLabel(uint32_t code, _TCHAR *label, bool translate = false);
};

#endif /* GUI_KEYBINDDATA_H */
