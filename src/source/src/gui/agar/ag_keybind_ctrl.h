/** @file ag_keybind_ctrl.h

	Skelton for retropc emulator
	SDL + Agar edition

	@author Sasaji
	@date   2024.11.20

	@brief [ ag_keybind_ctrl ]
*/

#ifndef AG_KEYBIND_CTRL_H
#define AG_KEYBIND_CTRL_H

#include "ag_dlg.h"
#include "../gui_keybinddata.h"
#include "../../cptrlist.h"

namespace GUI_AGAR
{

/**
	@brief keybind control
*/
class AG_KEYBIND_CTRL
{
public:
	AG_GUI_BASE *gui;
	int m_tab_num;
	AG_DLG *dlg;

	KeybindData *kbdata;

	AG_NotebookTab *tab;

	AG_Table *tbl;

	int			 m_combi;
	AG_Checkbox *chkCombi; 

	Uint32 tm1;

	struct selected_st {
		int row;
		int col;
		AG_TableCell *cell;
	} selected;

public:
	AG_KEYBIND_CTRL(AG_DLG *parent, int tab_num, AG_GUI_BASE *parent_gui);
	~AG_KEYBIND_CTRL();
	void Init(EMU *emu, AG_Notebook *nb, const char *title);
	void Final();
	void AdjustColumnPosition();
	void CopyColumnPositionAndSize(const AG_KEYBIND_CTRL *src_ctrl);

	void Update();
	void SetData();

	void UpdateKey(AG_Table *, int, int, Uint32);
	void UpdateJoy();
	void ClickCell(AG_Table *, int, int);
	void LoadData(int);
	void SaveData(int);

	static void OnUpdate(AG_Event *);
	static void OnClickCell(AG_Event *);
	static void OnKeyDown(AG_Event *);
	static void OnLoadDefault(AG_Event *);
	static void OnLoadPreset(AG_Event *);
	static void OnSavePreset(AG_Event *);
};

}; /* namespace GUI_AGAR */

#endif /* AG_KEYBIND_CTRL_H */
