/** @file ag_seldrv_dlg.cpp

	Skelton for retropc emulator
	SDL + Ager edition

	@author Sasaji
	@date   2015.12.12

	@brief [ ag_seldrv_dlg ]
*/

#include "ag_seldrv_dlg.h"
#include "../../video/rec_video.h"
#include <agar/core/types.h>
#include "../gui.h"
#include "ag_gui_base.h"
#include "../../clocale.h"
#include "../../utility.h"

namespace GUI_AGAR
{

AG_SELDRV_DLG::AG_SELDRV_DLG(EMU *parent_emu, AG_GUI_BASE *parent_gui) : AG_DLG(parent_emu, parent_gui)
{
	def_drv = 0;
	def_num = 0;
	UTILITY::tcscpy(prefix, sizeof(prefix) / sizeof(prefix[0]), _T("Drive"));
}

AG_SELDRV_DLG::~AG_SELDRV_DLG()
{
}

/**
 * create select drive dialog
 */
void AG_SELDRV_DLG::Create(int sel_drv, int sel_num)
{
	_TCHAR label[64];

	AG_Window *win = AG_WindowNew(AG_WINDOW_MODAL | AG_WINDOW_DIALOG | AG_WINDOW_NORESIZE);
	AG_WindowSetCaptionS(win, CMSG(Select_Drive));

	AG_Box *hbox;

	def_drv = sel_drv;
	def_num = sel_num;

	//
	hbox = AG_BoxNewHoriz(win, AG_BOX_HFILL);
	for(int drv=0; drv<USE_FLOPPY_DISKS; drv++) {
		UTILITY::sprintf_utf8(label, 64, _T("%s%d"), prefix, drv);
		btn[drv] = AG_ButtonNewFn(hbox, 0, label, OnOk, "%Cp %Cp %i", this, win, drv);
	}

	AG_SetEvent(win, "window-close", OnClose, "%Cp %Cp", this, win);

//	gui->PostEtSystemPause(true);
	AG_WindowShow(win);
}

void AG_SELDRV_DLG::SelectDrive(AG_Window *win, int sel_drv)
{
	gui->PostEtAgOpenRecentFloppyMessage(def_drv, def_num, sel_drv);
	AG_ObjectDetach(win);
	gui->PostEtSystemPause(false);
}

void AG_SELDRV_DLG::Close(AG_Window *win)
{
	gui->PostEtAgOpenRecentFloppyMessage(def_drv, def_num, def_drv);
	AG_ObjectDetach(win);
	gui->PostEtSystemPause(false);
}

void AG_SELDRV_DLG::SetPrefix(const _TCHAR *val)
{
	UTILITY::tcscpy(prefix, sizeof(prefix) / sizeof(prefix[0]), val);
}

/*
 * Event Handler (static)
 */
void AG_SELDRV_DLG::OnOk(AG_Event *event)
{
	AG_SELDRV_DLG *dlg = (AG_SELDRV_DLG *)AG_PTR(1);
	AG_Window *win = (AG_Window *)AG_PTR(2);
	int drv = AG_INT(3);

	dlg->SelectDrive(win, drv);
}

void AG_SELDRV_DLG::OnClose(AG_Event *event)
{
	AG_SELDRV_DLG *dlg = (AG_SELDRV_DLG *)AG_PTR(1);
	AG_Window *win = (AG_Window *)AG_PTR(2);

	dlg->Close(win);
}

}; /* namespace GUI_AGAR */
