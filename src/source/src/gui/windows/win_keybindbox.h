/** @file win_keybindbox.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2012.03.31 -

	@brief [ keybind box ]
*/

#ifndef WIN_KEYBINDBOX_H
#define WIN_KEYBINDBOX_H

#include <windows.h>
#include "win_keybindctrl.h"
//#include "win_dialogbox.h"
#include <vector>

class EMU;

namespace GUI_WIN
{

/**
	@brief Keybind dialog box
*/
class KeybindBox : public KeybindBaseBox
{
private:
	INT_PTR onInitDialog(UINT message, WPARAM wParam, LPARAM lParam);

	void select_tabctrl(int tab_num);

public:
	KeybindBox(HINSTANCE, CFont *, EMU *, GUI *);
	~KeybindBox();
};

}; /* namespace GUI_WIN */

#endif /* WIN_KEYBINDBOX_H */
