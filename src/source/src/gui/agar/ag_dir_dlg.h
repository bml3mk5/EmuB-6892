/** @file ag_dir_dlg.h

	Skelton for retropc emulator
	SDL + Agar edition

	@author Sasaji
	@date   2012.03.01

	@brief [ ag_dir_dlg ]
*/

#ifndef AG_DIR_DLG_H
#define AG_DIR_DLG_H

#include "ag_dlg.h"

namespace GUI_AGAR
{

/**
	@brief dir dialog
*/
class AG_DIR_DLG : public AG_DLG {
	friend class AG_GUI_BASE;

private:
	Uint32 flags;

	int ft_all;

	AG_Window *parent_win;
	char *buf_dir;
	int   buf_len;

	// dir dialog event handler
	static void OnCloseLoad(AG_Event *);
	static void OnKeyUpLoad(AG_Event *);
	static void OnClickOkLoad(AG_Event *);

public:
	AG_DIR_DLG(EMU *, AG_GUI_BASE *);
	~AG_DIR_DLG();

	void CreateLoad(const char *title, char *dir, int len, AG_Window *parent = NULL, void *attr = NULL);

	void CloseLoad(AG_DirDlg *);
	void LoadDir(AG_DirDlg *, void *);
};

}; /* namespace GUI_AGAR */

#endif /* AG_DIR_DLG_H */
