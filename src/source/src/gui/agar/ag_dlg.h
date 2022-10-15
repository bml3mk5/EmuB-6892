/** @file ag_dlg.h

	Skelton for retropc emulator
	SDL + Agar edition

	@author Sasaji
	@date   2012.04.08

	@brief [ ag_dlg ]
*/

#ifndef AG_DLG_H
#define AG_DLG_H

#include "../../common.h"
#include <sys/types.h>
#include <agar/config/have_sdl.h>
#include <agar/core.h>
#include <agar/gui.h>
#include <agar/gui/cursors.h>
#include <agar/gui/sdl.h>
#include <SDL.h>
#include "../../cchar.h"
#include "../../cptrlist.h"
#include "../../msgs.h"

class EMU;

namespace GUI_AGAR
{

class AG_GUI_BASE;

/**
	@brief dialog base class
*/
class AG_DLG {
protected:
	EMU         *emu;
	AG_GUI_BASE *gui;

public:
	AG_DLG(EMU *parent_emu, AG_GUI_BASE *parent_gui) {
		emu = parent_emu;
		gui = parent_gui;
	}
	virtual ~AG_DLG() {}

	virtual void Create() {}
	virtual void Create(int) {}
	virtual void Create(int, int) {}
	virtual void Create(int, bool) {}
	virtual void Update() {}
	virtual void Close(AG_Window *win) {
		AG_ObjectDetach(win);
	}
	virtual int SetData(AG_Window *win) {
		return 1;
	}

	virtual AG_Label *LabelNew(void *parent, const char *label);
	virtual AG_Textbox *TextboxNew(void *parent, const char *label, int box_len, char *buf, int buf_len);
	virtual AG_UCombo *UComboNew(void *parent, const char **list, int selnum, AG_EventFn cb, int index = -1);
	virtual AG_UCombo *UComboNew(void *parent, const CMsg::Id *list, int selnum, AG_EventFn cb, int index = -1, int appendnum = -1, CMsg::Id appendstr = CMsg::End);
	virtual AG_UCombo *UComboNew(void *parent, const CPtrList<CTchar> &list, int selnum, AG_EventFn cb, int index = -1);
	virtual AG_Radio *RadioNewInt(void *parent, unsigned int flags, const CMsg::Id *list, int *ret);
};

}; /* namespace GUI_AGAR */

#endif /* AG_DLG_H */
