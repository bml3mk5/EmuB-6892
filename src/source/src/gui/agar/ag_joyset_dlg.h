/** @file ag_joyset_dlg.h

	Skelton for retropc emulator
	SDL + Agar edition

	@author Sasaji
	@date   2023.01.29

	@brief [ ag_joyset_dlg ]
*/

#ifndef AG_JOYSET_DLG_H
#define AG_JOYSET_DLG_H

#include "ag_dlg.h"
#include "../gui_keybinddata.h"
#include "ag_keybind_ctrl.h"
#include "../../emu.h"

namespace GUI_AGAR
{

/**
	@brief volume dialog
*/
class AG_JOYSET_DLG : public AG_DLG {
	friend class AG_GUI_BASE;

private:
	int selected_tab;

	CPtrList<AG_KEYBIND_CTRL> ctrls;

#if defined(USE_PIAJOYSTICK) || defined(USE_KEY2JOYSTICK)
	int mash[MAX_JOYSTICKS][KEYBIND_JOY_BUTTONS];
	int axis[MAX_JOYSTICKS][6];
#endif
#ifdef USE_PIAJOYSTICKBIT
	int iChkPiaJoyNeg;
	int iChkPiaJoyConn;
#else
	int iChkPiaJoyNoIrq;
#endif

	void load_data(int);
	void save_data(int);

	// event handler
	static void OnOk(AG_Event *);
	static void OnClose(AG_Event *);
	static void OnClickTab(AG_Event *);
	static void OnLoadDefault(AG_Event *);
	static void OnLoadPreset(AG_Event *);
	static void OnSavePreset(AG_Event *);

public:
	AG_JOYSET_DLG(EMU *, AG_GUI_BASE *);
	~AG_JOYSET_DLG();

	void Create();
	void Close(AG_Window *);
	void SetData();
};

}; /* namespace GUI_AGAR */

#endif /* AG_JOYSET_DLG_H */
