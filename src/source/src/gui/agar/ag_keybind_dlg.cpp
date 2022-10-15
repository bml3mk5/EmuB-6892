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
	for(int tab=0; tab<KEYBIND_MAX_NUM; tab++) {
		kbdata[tab] = new KeybindData();
	}

	tm1 = 0;

	joy_stat = emu->joy_buffer();
}

AG_KEYBIND_DLG::~AG_KEYBIND_DLG()
{
	for(int tab=0; tab<KEYBIND_MAX_NUM; tab++) {
		delete kbdata[tab];
	}
}

/*
 * create keybind dialog
 */
void AG_KEYBIND_DLG::Create()
{
	// create window
	AG_Window *win = AG_WindowNew(AG_WINDOW_MODAL | AG_WINDOW_DIALOG | AG_WINDOW_NORESIZE | AG_WINDOW_MODKEYEVENTS);
	AG_WindowSetCaptionS(win, CMSG(Keybind));

	AG_Notebook *nb;
//	AG_NotebookTab *ntab;
	AG_Table *tbl;
	AG_Box *hbox;
	AG_Box *vbox;

	char label[3][128];
	char str[128];
	int tab;
	int i;

	selected_tab = 0;
	memset(selected, 0, sizeof(selected));

	nb = AG_NotebookNew(win, AG_NOTEBOOK_HFILL);

	for(tab=0; tab<KEYBIND_MAX_NUM; tab++) {
#ifdef USE_PIAJOYSTICKBIT
		kbdata[tab]->Init(emu, tab, tab == 0 ? 0 : 1, tab == 2 ? 2 : 0);
#else
		kbdata[tab]->Init(emu, tab, tab == 0 ? 0 : 1, tab == 2 ? 1 : 0);
#endif

		int rows = kbdata[tab]->GetNumberOfRows();
		int cols = kbdata[tab]->GetNumberOfColumns();

//		tabs[tab] = AG_NotebookAddTab(nb, tab == 1 ? CMSG(Joypad_Key_Assigned) : (tab == 2 ? CMSG(Joypad_PIA_Type) : CMSG(Keyboard)), AG_BOX_HORIZ);
		tabs[tab] = AG_NotebookAddTab(nb, CMSGV(LABELS::keybind_tab[tab]), AG_BOX_HORIZ);

		// table
		vbox = AG_BoxNewVert(tabs[tab], AG_BOX_VFILL);
		if (tab == 1) {
			tbl = AG_TableNewPolled(vbox, AG_TABLE_EXPAND | AG_TABLE_NOAUTOSORT, OnUpdate, "%Cp", this);
			AG_TableSetPollInterval(tbl, 250);
		} else {
			tbl = AG_TableNew(vbox, AG_TABLE_EXPAND | AG_TABLE_NOAUTOSORT);
		}
		tbl->wid.flags |= AG_WIDGET_CATCH_TAB;
		tbls[tab] = tbl;
		AG_TableSetSelectionMode(tbl, AG_TABLE_SEL_CELLS);
		AG_TableSetColMin(tbl, 80);
		AG_TableSetDefaultColWidth(tbl, 120);
//#ifdef _MBS1
//		if (tab == 2) {
//			AG_TableAddCol(tbl, CMSG(PIA_on_S1), "100px", NULL);
//		} else {
//			AG_TableAddCol(tbl, CMSG(S1_Key), "100px", NULL);
//		}
//#else
//		if (tab == 2) {
//			AG_TableAddCol(tbl, CMSG(PIA_on_L3), "100px", NULL);
//		} else {
//			AG_TableAddCol(tbl, CMSG(Level3_Key), "100px", NULL);
//		}
//#endif
		AG_TableAddCol(tbl, CMSGV(LABELS::keybind_col[tab][0]), "100px", NULL);

//		if (tab != 0) {
//			for(int col=0; col<cols; col++) {
//				UTILITY::sprintf_utf8(str, 128, CMSG(JoypadVDIGIT), col+1);
//				AG_TableAddCol(tbl, str, "120px", NULL);
//			}
//		} else {
//			for(int col=0; col<cols; col++) {
//				UTILITY::sprintf_utf8(str, 128, CMSG(BindVDIGIT), col+1);
//				AG_TableAddCol(tbl, str, "120px", NULL);
//			}
//		}
		for(int col=0; col<cols; col++) {
			UTILITY::sprintf_utf8(str, 128, CMSGV(LABELS::keybind_col[tab][1]), col+1);
			AG_TableAddCol(tbl, str, "120px", NULL);
		}

		AG_TableBegin(tbl);
		for(int row=0; row<rows; row++) {
			kbdata[tab]->GetCellString(row, -1, label[0]);

			for(int col=0; col<cols; col++) {
				kbdata[tab]->GetCellString(row, col, label[col + 1]);
			}

			AG_TableAddRow(tbl, "%s:%s:%s",label[0],label[1],label[2]);
		}
		AG_TableAddRow(tbl, "%s:%s:%s","","","");
		AG_TableEnd(tbl);

		AG_TableSetCellClickFn(tbl, OnClickCell, "%Cp", this);
		AG_SetEvent(tbl, "key-down", OnKeyDown, "%Cp", this);

		// checkbox for joypad
		AG_LabelNew(vbox, 0, " ");
//		if (tab == 2) {
//			combi[1] = (int)kbdata[tab]->GetCombi();
//			chkCombi[1] = AG_CheckboxNewInt(vbox, 0, CMSG(Signals_are_negative_logic), &combi[1]);
//		} else if (tab == 1) {
//			combi[0] = (int)kbdata[tab]->GetCombi();
//			chkCombi[0] = AG_CheckboxNewInt(vbox, 0, CMSG(Recognize_as_another_key_when_pressed_two_buttons), &combi[0]);
//		} else {
//			AG_LabelNew(vbox, 0, " ");
//		}
		if (tab > 0 && tab < 3) {
			combi[tab-1] = (int)kbdata[tab]->GetCombi();
			chkCombi[tab-1] = AG_CheckboxNewInt(vbox, 0, CMSGV(LABELS::keybind_combi[tab]), &combi[tab-1]);
		} else {
			AG_LabelNew(vbox, 0, " ");
		}

		// right button
		vbox = AG_BoxNewVert(tabs[tab],AG_BOX_EXPAND);
		AG_ButtonNewFn(vbox, AG_BUTTON_HFILL, CMSG(Load_Default), OnLoadDefault, "%Cp", this);
		AG_LabelNew(vbox, 0, " ");
		for(i=0; i<4; i++) {
			UTILITY::sprintf_utf8(str, 128, CMSG(Load_Preset_VDIGIT), i+1);
			AG_ButtonNewFn(vbox, AG_BUTTON_HFILL, str, OnLoadPreset, "%Cp %i", this, i);
		}
		AG_LabelNew(vbox, 0, " ");
		for(i=0; i<4; i++) {
			UTILITY::sprintf_utf8(str, 128, CMSG(Save_Preset_VDIGIT), i+1);
			AG_ButtonNewFn(vbox, AG_BUTTON_HFILL, str, OnSavePreset, "%Cp %i", this, i);
		}

		AG_LabelNew(vbox, 0, " ");
	}

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
	for(i=1; i<3; i++) {
		tbls[0]->cols[i].x = tbls[0]->cols[i-1].x + tbls[0]->cols[i-1].w + 1;
	}
	for(tab=1; tab<KEYBIND_MAX_NUM; tab++) {
		for(i=0; i<3; i++) {
			tbls[tab]->cols[i].x = tbls[0]->cols[i].x;
			tbls[tab]->cols[i].w = tbls[0]->cols[i].w;
		}
	}
}

void AG_KEYBIND_DLG::Update()
{
	int tab = selected_tab;
	AG_Table *tbl = tbls[tab];
	char label[3][100];
	int rows = kbdata[tab]->GetNumberOfRows();
	int cols = kbdata[tab]->GetNumberOfColumns();

	AG_TableBegin(tbl);
	for(int row=0; row<rows; row++) {
		kbdata[tab]->GetCellString(row, -1, label[0]);

		for(int col=0; col<cols; col++) {
			kbdata[tab]->GetCellString(row, col, label[col + 1]);
		}

		AG_TableAddRow(tbl, "%s:%s:%s",label[0],label[1],label[2]);
	}
	AG_TableAddRow(tbl, "%s:%s:%s","","","");
	AG_TableEnd(tbl);

	AG_TableDeselectAllRows(tbl);
	AG_TableSelectCell(tbl, selected[tab].row, selected[tab].col);

	if (tab == 1 || tab == 2) {
		combi[tab-1] = (int)kbdata[tab]->GetCombi();
		AG_WidgetUpdate(chkCombi[tab-1]);
	}
}

Uint32 AG_KEYBIND_DLG::translate_vkkey(Uint32 code) {
//	UINT32 new_code = code;

//	emu->translate_sdl_keysym();
	return code;
}

void AG_KEYBIND_DLG::click_cell(AG_Table *tbl, int row, int col)
{
	int tab = selected_tab;

	if (selected[tab].cell == AG_TableGetCell(tbl, row, col)) {
		// double click ?
		Uint32 tm2 = AG_GetTicks();
		if (tm2 - tm1 < 500) {
			if (tab == 1) kbdata[tab]->SetVkJoyCode(row, col-1, 0, NULL);
			else kbdata[tab]->SetVkKeyCode(row, col-1, 0, NULL);
			Update();
		}
	}

	AG_TableDeselectRow(tbl, row);
	if (row + 1 < tbl->m) {
		if (col < 1) col = 1;
		AG_TableSelectCell(tbl, row, col);

		selected[tab].cell = AG_TableGetCell(tbl, row, col);
		selected[tab].row = row;
		selected[tab].col = col;
	}
	tm1 = AG_GetTicks();
}

void AG_KEYBIND_DLG::load_data(int num)
{
	int tab = selected_tab;
	kbdata[tab]->LoadPreset(num);
	Update();
}

void AG_KEYBIND_DLG::save_data(int num)
{
	int tab = selected_tab;
	kbdata[tab]->SavePreset(num);
}

void AG_KEYBIND_DLG::Close(AG_Window *win)
{
	AG_ObjectDetach(win);
	gui->PostEtSystemPause(false);
}

int AG_KEYBIND_DLG::SetData(AG_Window *win)
{
	AG_TableSetPollInterval(tbls[1], 0);

	for(int tab=0; tab<KEYBIND_MAX_NUM; tab++) {
		kbdata[tab]->SetData();
		if (tab == 1 || tab == 2) {
			kbdata[tab]->SetCombi(combi[tab-1]);
		}
	}

	emu->save_keybind();

	return 1;
}

void AG_KEYBIND_DLG::click_cancel(AG_Window *win)
{
	AG_TableSetPollInterval(tbls[1], 0);
	Close(win);
}

void AG_KEYBIND_DLG::update_key(AG_Table *tbl, int sym, int mod, Uint32 unicode)
{
	int tab = selected_tab;
	int row = selected[tab].row;
	int col = selected[tab].col;
	short scancode = gui->sdl_event.key.keysym.scancode;
	int new_key = 0;

	if (tab == 0 && col > 0) {
		// based SDL event
		emu->translate_keysym(0,sym,scancode,&new_key);
#ifndef USE_SDL2
		kbdata[tab]->SetVkKeyCode(row, col-1, new_key, NULL);
#else
		kbdata[tab]->SetVkKeyCode(row, col-1, scancode, NULL);
#endif
		Update();
	}
}

void AG_KEYBIND_DLG::update_joy()
{
	int tab = selected_tab;
	int row = selected[tab].row;
	int col = selected[tab].col;

	if (tab != 0 && col > 0) {
		emu->reset_joystick();
//		emu->update_joystick();
		if (joy_stat[col-1]) {
			kbdata[tab]->SetVkJoyCode(row, col-1, joy_stat[col-1], NULL);
			Update();
		}
	}
}

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

void AG_KEYBIND_DLG::OnUpdate(AG_Event *event)
{
//	AG_Table *tbl = (AG_Table *)AG_SELF();
	AG_KEYBIND_DLG *dlg = (AG_KEYBIND_DLG *)AG_PTR(1);

	dlg->update_joy();
}

void AG_KEYBIND_DLG::OnClickTab(AG_Event *event)
{
	AG_Notebook *tab = (AG_Notebook *)AG_SELF();
	AG_KEYBIND_DLG *dlg = (AG_KEYBIND_DLG *)AG_PTR(1);

	for(int i=0; i<KEYBIND_MAX_NUM; i++) {
		if (dlg->tabs[i] == tab->sel_tab) {
			dlg->selected_tab = i;
			break;
		}
	}
}

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

void AG_KEYBIND_DLG::OnLoadDefault(AG_Event *event)
{
//	AG_Table *tbl = (AG_Table *)AG_SELF();
	AG_KEYBIND_DLG *dlg = (AG_KEYBIND_DLG *)AG_PTR(1);

	dlg->load_data(-1);
}

void AG_KEYBIND_DLG::OnLoadPreset(AG_Event *event)
{
//	AG_Table *tbl = (AG_Table *)AG_SELF();
	AG_KEYBIND_DLG *dlg = (AG_KEYBIND_DLG *)AG_PTR(1);
	int num = AG_INT(2);

	dlg->load_data(num);
}

void AG_KEYBIND_DLG::OnSavePreset(AG_Event *event)
{
//	AG_Table *tbl = (AG_Table *)AG_SELF();
	AG_KEYBIND_DLG *dlg = (AG_KEYBIND_DLG *)AG_PTR(1);
	int num = AG_INT(2);

	dlg->save_data(num);
}

}; /* namespace GUI_AGAR */
