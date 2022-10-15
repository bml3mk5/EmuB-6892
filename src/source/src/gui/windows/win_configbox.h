/** @file win_configbox.h

	HITACHI BASIC MASTER LEVEL3 Mark5 Emulator 'EmuB-6892'
	HITACHI MB-S1 Emulator 'EmuB-S1'
	Skelton for retropc emulator

	@author Sasaji
	@date   2012.03.24 -

	@brief [ config box ]
*/

#ifndef WIN_CONFIGBOX_H
#define WIN_CONFIGBOX_H

#include <windows.h>
#include "win_dialogbox.h"
#include "../../config.h"
#include "../../cchar.h"
#include "../../cptrlist.h"

namespace GUI_WIN
{

/**
	@brief Config dialog box
*/
class ConfigBox : public CDialogBox
{
private:
	HINSTANCE  hInstance;

	int        fdd_type;

	int	       io_port;
#if defined(_MBS1)
	int        sys_mode;
	int        exram_size_num;
#endif

	CPtrList<CTchar> lang_list;

	CTabItems tab_items;

	int selected_tabctrl;

	INT_PTR onInitDialog(UINT, WPARAM, LPARAM);
	INT_PTR onCommand(UINT, WPARAM, LPARAM);
	INT_PTR onNotify(UINT, WPARAM, LPARAM);
	INT_PTR onOK(UINT, WPARAM, LPARAM);

	void select_tabctrl(int tab_num);

public:
	ConfigBox(HINSTANCE, CFont *, EMU *, GUI *);
	~ConfigBox();
};

}; /* namespace GUI_WIN */

#endif /* WIN_CONFIGBOX_H */
