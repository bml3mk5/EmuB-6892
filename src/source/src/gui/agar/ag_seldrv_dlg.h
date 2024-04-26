/** @file ag_seldrv_dlg.h

	Skelton for retropc emulator
	SDL + Agar edition

	@author Sasaji
	@date   2015.12.12

	@brief [ ag_seldrv_dlg ]
*/

#ifndef AG_SELDRV_DLG_H
#define AG_SELDRV_DLG_H

#include "ag_dlg.h"
#include "../../vm/vm.h"

namespace GUI_AGAR
{

/**
	@brief select drive dialog
*/
class AG_SELDRV_DLG : public AG_DLG {
	friend class AG_GUI_BASE;

private:
	int def_drv;
	int def_num;
	_TCHAR prefix[32];

	AG_Button *btn[USE_FLOPPY_DISKS];

	void SelectDrive(AG_Window *win, int sel_drv);

	static void OnOk(AG_Event *);
	static void OnClose(AG_Event *);

public:
	AG_SELDRV_DLG(EMU *, AG_GUI_BASE *);
	~AG_SELDRV_DLG();

	void Create(int sel_drv, int sel_num);
	void Close(AG_Window *);
	void SetPrefix(const _TCHAR *);
};

}; /* namespace GUI_AGAR */

#endif /* AG_SELDRV_DLG_H */
