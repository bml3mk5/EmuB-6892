/** @file ag_keybind_dlg.h

	Skelton for retropc emulator
	SDL + Agar edition

	@author Sasaji
	@date   2012.4.8

	@brief [ ag_keybind_dlg ]
*/

#ifndef AG_KEYBIND_DLG_H
#define AG_KEYBIND_DLG_H

#include "ag_dlg.h"
#include "../gui_keybinddata.h"

namespace GUI_AGAR
{

/**
	@brief keybind dialog
*/
class AG_KEYBIND_DLG : public AG_DLG {
	friend class AG_GUI_BASE;

private:
	int     combi[2];
	AG_Checkbox *chkCombi[2]; 

	KeybindData *kbdata[KEYBIND_MAX_NUM];

	AG_NotebookTab *tabs[KEYBIND_MAX_NUM];
	int selected_tab;
	struct selected_st {
		int row;
		int col;
		AG_TableCell *cell;
	} selected[KEYBIND_MAX_NUM];

	AG_Table *tbls[KEYBIND_MAX_NUM];

	Uint32 tm1;

	Uint32 *joy_stat;	// joystick #1, #2 (b0 = up, b1 = down, b2 = left, b3 = right, b4-b31 = trigger #1-#28

	Uint32 translate_vkkey(Uint32);

	void click_cancel(AG_Window *);
	void click_cell(AG_Table *, int, int);

	void load_data(int);
	void save_data(int);

	void update_key(AG_Table *, int, int, Uint32);
	void update_joy();

	static void OnOk(AG_Event *);
	static void OnClose(AG_Event *);
	static void OnUpdate(AG_Event *);
	static void OnClickTab(AG_Event *);
	static void OnClickCell(AG_Event *);
	static void OnKeyDown(AG_Event *);
	static void OnLoadDefault(AG_Event *);
	static void OnLoadPreset(AG_Event *);
	static void OnSavePreset(AG_Event *);

public:
	AG_KEYBIND_DLG(EMU *, AG_GUI_BASE *);
	~AG_KEYBIND_DLG();

	void Create();
	void Update();
	void Close(AG_Window *);
	int  SetData(AG_Window *);

};

}; /* namespace GUI_AGAR */

#endif /* AG_KEYBIND_DLG_H */
