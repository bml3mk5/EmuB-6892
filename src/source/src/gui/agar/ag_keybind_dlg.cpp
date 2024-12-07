/** @file ag_keybind_dlg.cpp

	Skelton for retropc emulator
	SDL + Ager edition

	@author Sasaji
	@date   2012.4.8

	@brief [ ag_keybind_dlg ]
*/

#include "ag_keybind_dlg.h"
#include "ag_gui_base.h"
#include "../gui.h"
#include "../../vm/vm.h"
#include "../../utility.h"
#include "../../labels.h"

namespace GUI_AGAR
{

AG_KEYBIND_DLG::AG_KEYBIND_DLG(EMU *parent_emu, AG_GUI_BASE *parent_gui) : AG_DLG(parent_emu, parent_gui)
{
	int tab_offset = KeybindData::KB_TABS_MIN;
	for(int tab_num=tab_offset; tab_num<KeybindData::KB_TABS_MAX; tab_num++) {
		AG_KEYBIND_CTRL *kc = new AG_KEYBIND_CTRL(this, tab_num, parent_gui);
		ctrls.Add(kc);
	}

//	tm1 = 0;
}

AG_KEYBIND_DLG::~AG_KEYBIND_DLG()
{
}

/*
 * create keybind dialog
 */
void AG_KEYBIND_DLG::Create()
{
	char str[128];

	// create window
	AG_Window *win = AG_WindowNew(AG_WINDOW_MODAL | AG_WINDOW_DIALOG | AG_WINDOW_NORESIZE | AG_WINDOW_MODKEYEVENTS);
	AG_WindowSetCaptionS(win, CMSG(Keybind));

	AG_Box *hboxall;
	AG_Notebook *nb;
	AG_Box *vbox;
	AG_Box *hbox;

	hboxall = AG_BoxNewHoriz(win, AG_BOX_EXPAND);

	selected_tab = 0;
	nb = AG_NotebookNew(hboxall, AG_NOTEBOOK_VFILL);

	for(int tab_num=0; tab_num<ctrls.Count(); tab_num++) {
		AG_KEYBIND_CTRL *ctrl = ctrls[tab_num];
		ctrl->Init(emu, nb, CMSGV(LABELS::keybind_tab[tab_num]));
	}

	// right button
	vbox = AG_BoxNewVert(hboxall, AG_BOX_EXPAND);
	AG_ButtonNewFn(vbox, AG_BUTTON_HFILL, CMSG(Load_Default), OnLoadDefault, "%Cp", this);
	AG_LabelNew(vbox, 0, " ");
	for(int i=0; i<4; i++) {
		UTILITY::sprintf_utf8(str, 128, CMSG(Load_Preset_VDIGIT), i+1);
		AG_ButtonNewFn(vbox, AG_BUTTON_HFILL, str, OnLoadPreset, "%Cp %i", this, i);
	}
	AG_LabelNew(vbox, 0, " ");
	for(int i=0; i<4; i++) {
		UTILITY::sprintf_utf8(str, 128, CMSG(Save_Preset_VDIGIT), i+1);
		AG_ButtonNewFn(vbox, AG_BUTTON_HFILL, str, OnSavePreset, "%Cp %i", this, i);
	}

	AG_LabelNew(vbox, 0, " ");

	// add event
	AG_AddEvent(nb, "mouse-button-down", OnClickTab, "%Cp", this);

	// bottom button
	hbox = AG_BoxNewHoriz(win, AG_BOX_EXPAND);
	AG_ButtonNewFn(hbox, 0, CMSG(OK), OnOk, "%Cp %Cp", this, win);
	AG_ButtonNewFn(hbox, 0, CMSG(Cancel), OnClose, "%Cp %Cp", this, win);

	AG_SetEvent(win, "window-close", OnClose, "%Cp %Cp", this, win);

	gui->PostEtSystemPause(true);
	AG_WindowShow(win);

	// bug fix for agar
	ctrls[0]->AdjustColumnPosition();

	for(int tab_num=1; tab_num<ctrls.Count(); tab_num++) {
		ctrls[tab_num]->CopyColumnPositionAndSize(ctrls[0]);
	}
}

#if 0
void AG_KEYBIND_DLG::Update(int tab_num)
{
	AG_KEYBIND_CTRL *kc = ctrls[tab_num];
	kc->Update();
}
#endif

Uint32 AG_KEYBIND_DLG::translate_vkkey(Uint32 code)
{
//	UINT32 new_code = code;

//	emu->translate_sdl_keysym();
	return code;
}

#if 0
void AG_KEYBIND_DLG::ClickCell(AG_Table *tbl, int row, int col)
{
	int tab = selected_tab;
	AG_KEYBIND_CTRL *kc = ctrls[tab];

	if (kc->selected.cell == AG_TableGetCell(tbl, row, col)) {
		// double click ?
		Uint32 tm2 = AG_GetTicks();
		if (tm2 - tm1 < 500) {
			if (kc->kbdata->m_devtype == KeybindData::DEVTYPE_JOYPAD) {
				kc->kbdata->SetVkJoyCode(row, col-1, 0, 0, NULL);
			} else {
				kc->kbdata->SetVkKeyCode(row, col-1, 0, NULL);
			}
			Update(tab);
		}
	}

	AG_TableDeselectRow(tbl, row);
	if (row + 1 < tbl->m) {
		if (col < 1) col = 1;
		AG_TableSelectCell(tbl, row, col);

		kc->selected.cell = AG_TableGetCell(tbl, row, col);
		kc->selected.row = row;
		kc->selected.col = col;
	}
	tm1 = AG_GetTicks();
}
#endif

void AG_KEYBIND_DLG::load_data(int num)
{
	int tab = selected_tab;
	AG_KEYBIND_CTRL *kc = ctrls[tab];
	kc->LoadData(num);
}

void AG_KEYBIND_DLG::save_data(int num)
{
	int tab = selected_tab;
	AG_KEYBIND_CTRL *kc = ctrls[tab];
	kc->SaveData(num);
}

void AG_KEYBIND_DLG::Close(AG_Window *win)
{
	AG_ObjectDetach(win);
	gui->PostEtSystemPause(false);
}

int AG_KEYBIND_DLG::SetData(AG_Window *win)
{
	for(int tab_num=0; tab_num<ctrls.Count(); tab_num++) {
		AG_KEYBIND_CTRL *kc = ctrls[tab_num];
		kc->SetData();

		kc->Final();
	}

	emu->save_keybind();

	return 1;
}

void AG_KEYBIND_DLG::click_cancel(AG_Window *win)
{
	for(int tab_num=0; tab_num<ctrls.Count(); tab_num++) {
		AG_KEYBIND_CTRL *kc = ctrls[tab_num];
		kc->Final();
	}
	Close(win);
}

#if 0
void AG_KEYBIND_DLG::UpdateKey(AG_Table *tbl, int sym, int mod, Uint32 unicode)
{
	int tab = selected_tab;
	AG_KEYBIND_CTRL *kc = ctrls[tab];

	int row = kc->selected.row;
	int col = kc->selected.col;
	short scancode = gui->sdl_event.key.keysym.scancode;
	int new_key = 0;

	if (kc->kbdata->m_devtype == KeybindData::DEVTYPE_KEYBOARD && col > 0) {
		// based SDL event
		emu->translate_keysym(0,sym,scancode,&new_key);
#ifndef USE_SDL2
		kc->kbdata->SetVkKeyCode(row, col-1, new_key, NULL);
#else
		kc->kbdata->SetVkKeyCode(row, col-1, scancode, NULL);
#endif
		Update(tab);
	}
}

void AG_KEYBIND_DLG::UpdateJoy()
{
	int tab = selected_tab;
	AG_KEYBIND_CTRL *kc = ctrls[tab];

	int row = kc->selected.row;
	int col = kc->selected.col;

	if (tab != 0 && col > 0) {
		Uint32 *joy_stat = emu->joy_real_buffer(col-1);
		if (joy_stat && (joy_stat[0] | joy_stat[1])) {
			kc->kbdata->SetVkJoyCode(row, col-1, joy_stat[0], joy_stat[1], NULL);
			Update(tab);
		}
	}
}
#endif

/*
 * Event Handler (static)
 */
void AG_KEYBIND_DLG::OnOk(AG_Event *event)
{
	AG_KEYBIND_DLG *dlg = (AG_KEYBIND_DLG *)AG_PTR(1);
	AG_Window *win = (AG_Window *)AG_PTR(2);

	if (dlg->SetData(win)) {
		dlg->Close(win);
	}
}

void AG_KEYBIND_DLG::OnClose(AG_Event *event)
{
	AG_KEYBIND_DLG *dlg = (AG_KEYBIND_DLG *)AG_PTR(1);
	AG_Window *win = (AG_Window *)AG_PTR(2);

	dlg->click_cancel(win);
}

#if 0
void AG_KEYBIND_DLG::OnUpdate(AG_Event *event)
{
//	AG_Table *tbl = (AG_Table *)AG_SELF();
	AG_KEYBIND_DLG *dlg = (AG_KEYBIND_DLG *)AG_PTR(1);

	dlg->update_joy();
}
#endif

void AG_KEYBIND_DLG::OnClickTab(AG_Event *event)
{
	AG_Notebook *tab = (AG_Notebook *)AG_SELF();
	AG_KEYBIND_DLG *dlg = (AG_KEYBIND_DLG *)AG_PTR(1);

	for(int tab_num=0; tab_num<dlg->ctrls.Count(); tab_num++) {
		if (dlg->ctrls[tab_num]->tab == tab->sel_tab) {
			dlg->selected_tab = tab_num;
			break;
		}
	}
}

#if 0
void AG_KEYBIND_DLG::OnClickCell(AG_Event *event)
{
	AG_Table *tbl = (AG_Table *)AG_SELF();
	AG_KEYBIND_DLG *dlg = (AG_KEYBIND_DLG *)AG_PTR(1);
	int row = AG_INT(2);
//	int col = AG_INT(3);	// oops...cannot get... bug?
	int col = 0;

	for(int i=0; i<3; i++) {
		if (AG_TableCellSelected(tbl, row, i)) {
			col = i;
			break;
		}
	}

	dlg->click_cell(tbl, row, col);
}

void AG_KEYBIND_DLG::OnKeyDown(AG_Event *event)
{
	AG_Table *tbl = (AG_Table *)AG_SELF();
	AG_KEYBIND_DLG *dlg = (AG_KEYBIND_DLG *)AG_PTR(1);
	int sym = AG_INT(2);
	int mod = AG_INT(3);
	Uint32 unicode = AG_UINT(4);

	dlg->update_key(tbl, sym, mod, (Uint32)unicode);
}
#endif

void AG_KEYBIND_DLG::OnLoadDefault(AG_Event *event)
{
	AG_KEYBIND_DLG *dlg = (AG_KEYBIND_DLG *)AG_PTR(1);

	dlg->load_data(-1);
}

void AG_KEYBIND_DLG::OnLoadPreset(AG_Event *event)
{
	AG_KEYBIND_DLG *dlg = (AG_KEYBIND_DLG *)AG_PTR(1);
	int num = AG_INT(2);

	dlg->load_data(num);
}

void AG_KEYBIND_DLG::OnSavePreset(AG_Event *event)
{
	AG_KEYBIND_DLG *dlg = (AG_KEYBIND_DLG *)AG_PTR(1);
	int num = AG_INT(2);

	dlg->save_data(num);
}

}; /* namespace GUI_AGAR */
