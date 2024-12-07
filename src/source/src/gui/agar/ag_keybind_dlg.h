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
#include "ag_keybind_ctrl.h"
#include "../../cptrlist.h"

namespace GUI_AGAR
{
/**
	@brief keybind dialog
*/
class AG_KEYBIND_DLG : public AG_DLG {
	friend class AG_GUI_BASE;

private:
	int selected_tab;

	CPtrList<AG_KEYBIND_CTRL> ctrls;

//	Uint32 tm1;

	Uint32 translate_vkkey(Uint32);

	void click_cancel(AG_Window *);
	void load_data(int);
	void save_data(int);

	static void OnOk(AG_Event *);
	static void OnClose(AG_Event *);
//	static void OnUpdate(AG_Event *);
	static void OnClickTab(AG_Event *);
//	static void OnClickCell(AG_Event *);
//	static void OnKeyDown(AG_Event *);
	static void OnLoadDefault(AG_Event *);
	static void OnLoadPreset(AG_Event *);
	static void OnSavePreset(AG_Event *);

public:
	AG_KEYBIND_DLG(EMU *, AG_GUI_BASE *);
	~AG_KEYBIND_DLG();

	void Create();
//	void Update(int tab_num);
	void Close(AG_Window *);
	int  SetData(AG_Window *);

//	void UpdateKey(AG_Table *, int, int, Uint32);
//	void UpdateJoy();
//	void ClickCell(AG_Table *, int, int);
};

}; /* namespace GUI_AGAR */

#endif /* AG_KEYBIND_DLG_H */
