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
	@brief joypad setting control
*/
class AG_JOYSET_CTRL : public AG_KEYBIND_CTRL
{
public:
#if defined(USE_PIAJOYSTICK) || defined(USE_KEY2PIAJOYSTICK)
#ifdef USE_JOYSTICKBIT
	int iChkPiaJoyNeg;
	int iRadPiaJoyConn;
#else
	int iChkPiaJoyNoIrq;
#endif
#endif
#if defined(USE_PSGJOYSTICK) || defined(USE_KEY2PSGJOYSTICK)
#ifdef USE_JOYSTICKBIT
	int iChkPsgJoyNeg;
#endif
#endif

protected:
	virtual void InitHeaderControl(AG_Box *vbox);
	virtual void InitFooterControl(AG_Box *vbox);
	virtual void SetDataInControls();

public:
	AG_JOYSET_CTRL(AG_DLG *parent, int tab_num, AG_GUI_BASE *parent_gui);
};

/**
	@brief joypad setting dialog
*/
class AG_JOYSET_DLG : public AG_DLG {
	friend class AG_GUI_BASE;

private:
	int selected_tab;

	CPtrList<AG_JOYSET_CTRL> ctrls;

#if defined(USE_PIAJOYSTICK) || defined(USE_KEY2JOYSTICK)
	int mash[MAX_JOYSTICKS][KEYBIND_JOY_BUTTONS];
	int axis[MAX_JOYSTICKS][6];
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
