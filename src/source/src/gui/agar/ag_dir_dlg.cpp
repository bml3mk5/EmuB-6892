/** @file ag_dir_dlg.cpp

	Skelton for retropc emulator
	SDL + Ager edition

	@author Sasaji
	@date   2012.3.3

	@brief [ ag_dir_dlg ]
*/

#include "ag_dir_dlg.h"
#include "../gui.h"
#include "ag_gui_base.h"
#include "ag_gui.h"
#include "../../utility.h"

namespace GUI_AGAR
{

AG_DIR_DLG::AG_DIR_DLG(EMU *parent_emu, AG_GUI_BASE *parent_gui) : AG_DLG(parent_emu, parent_gui)
{
	flags = AG_DIRDLG_CLOSEWIN | AG_DIRDLG_EXPAND;
	parent_win = NULL;
	buf_dir = NULL;
	buf_len = 0;
}

AG_DIR_DLG::~AG_DIR_DLG()
{
}

/**
 * create load dialog
 */
void AG_DIR_DLG::CreateLoad(const char *title, char *dir, int len, AG_Window *parent, void *attr)
{
	AG_Window *win = AG_WindowNew(AG_WINDOW_MODAL);

	AG_WindowSetCaptionS(win, title);

	AG_DirDlg *dd = AG_DirDlgNew(win, flags | AG_DIRDLG_LOAD);

	if (dir != NULL && dir[0] != '\0') {
		AG_DirDlgSetDirectoryS(dd, dir);
	} else {
		AG_DirDlgSetDirectoryS(dd, emu->application_path());
	}

	parent_win = parent;
	buf_dir = dir;
	buf_len = len;

//	AG_SetEvent(fd->comTypes, "combo-selected", OnSelectFileType, "%Cp", fd);

	AG_DirDlgOkAction(dd, OnClickOkLoad, "%Cp %p", this, attr);
	AG_DirDlgCancelAction(dd, OnCloseLoad, "%Cp %Cp", this, dd);
	AG_SetEvent(win, "window-close", OnCloseLoad, "%Cp %Cp", this, dd);
	AG_AddEvent(win, "key-up", OnKeyUpLoad, "%Cp %Cp", this, dd);

//	gui->PostEtSystemPause(true);
	AG_WindowSetGeometry(win, 0, 0, 300, 250);
	AG_WindowSetPosition(win, AG_WINDOW_CENTER, 0);
	AG_WindowShow(win);
}

//////////

void AG_DIR_DLG::CloseLoad(AG_DirDlg *dd)
{
	flags = dd->flags;

	AG_Window *win = AG_ParentWindow((void *)dd);
	if (win != NULL) {
		// close window
		AG_ObjectDetach(win);
		if (parent_win != NULL) AG_WindowFocus(parent_win);
	}
	parent_win = NULL;
	buf_dir = NULL;
	buf_len = 0;
//	gui->PostEtSystemPause(false);
}

void AG_DIR_DLG::LoadDir(AG_DirDlg *dd, void *attr)
{
	flags = dd->flags;

	if (buf_dir != NULL && buf_len > 0) {
		memcpy(buf_dir, dd->cwd, buf_len);
		UTILITY::add_path_separator(buf_dir);
	}

	AG_Window *win = AG_ParentWindow((void *)dd);
	if (win != NULL) {
		// close window
		AG_ObjectDetach(win);
		if (parent_win != NULL) AG_WindowFocus(parent_win);
	}
	parent_win = NULL;
	buf_dir = NULL;
	buf_len = 0;
//	gui->PostEtSystemPause(false);
}

/*
 * Event Handler (static)
 */
void AG_DIR_DLG::OnCloseLoad(AG_Event *event)
{
	AG_DIR_DLG *dlg = (AG_DIR_DLG *)AG_PTR(1);
	AG_DirDlg *dd = (AG_DirDlg *)AG_PTR(2);

	dlg->CloseLoad(dd);
}

void AG_DIR_DLG::OnKeyUpLoad(AG_Event *event)
{
	AG_DIR_DLG *dlg = (AG_DIR_DLG *)AG_PTR(1);
	AG_DirDlg *dd = (AG_DirDlg *)AG_PTR(2);
	int key = AG_INT(4);

	switch(key) {
	case AG_KEY_ESCAPE:
		dlg->CloseLoad(dd);
		break;
	}
}

void AG_DIR_DLG::OnClickOkLoad(AG_Event *event)
{
	AG_DirDlg *dd = (AG_DirDlg *)AG_SELF();
	AG_DIR_DLG *dlg = (AG_DIR_DLG *)AG_PTR(1);
	void *attr = AG_PTR(2);

//	if (AG_DirDlgCheckReadAccess(dd) == -1) {
//		AG_TextMsgFromError();
//		return;
//	}
	dlg->LoadDir(dd, attr);
}

}; /* namespace GUI_AGAR */
