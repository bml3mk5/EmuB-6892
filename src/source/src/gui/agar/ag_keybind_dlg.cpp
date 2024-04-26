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

//

AG_KEYBIND_CTRL::AG_KEYBIND_CTRL(AG_KEYBIND_DLG *parent, int tab_num)
{
	m_tab_num = tab_num;
	dlg = parent;

	kbdata = new KeybindData();
	tab = NULL;
	tbl = NULL;

	m_combi = 0;
	chkCombi = NULL;

	memset(&selected, 0, sizeof(selected));
}

AG_KEYBIND_CTRL::~AG_KEYBIND_CTRL()
{
	delete kbdata;
}

void AG_KEYBIND_CTRL::Init(EMU *emu, AG_Notebook *nb)
{
	char labels[3][128];
	char str[128];

	kbdata->Init(emu, m_tab_num);

	int rows = kbdata->GetNumberOfRows();
	int cols = kbdata->GetNumberOfColumns();

	tab = AG_NotebookAddTab(nb, CMSGV(LABELS::keybind_tab[m_tab_num]), AG_BOX_HORIZ);

	// table
	AG_Box *vbox;
	vbox = AG_BoxNewVert(tab, AG_BOX_VFILL);
	if (kbdata->m_devtype == KeybindData::DEVTYPE_JOYPAD) {
		tbl = AG_TableNewPolled(vbox, AG_TABLE_EXPAND | AG_TABLE_NOAUTOSORT, OnUpdate, "%Cp", dlg);
		AG_TableSetPollInterval(tbl, 250);
	} else {
		tbl = AG_TableNew(vbox, AG_TABLE_EXPAND | AG_TABLE_NOAUTOSORT);
	}
	tbl->wid.flags |= AG_WIDGET_CATCH_TAB;

	AG_TableSetSelectionMode(tbl, AG_TABLE_SEL_CELLS);
	AG_TableSetColMin(tbl, 80);
	AG_TableSetDefaultColWidth(tbl, 120);

	AG_TableAddCol(tbl, CMSGV(LABELS::keybind_col[m_tab_num][0]), "100px", NULL);

	for(int col=0; col<cols; col++) {
		UTILITY::sprintf_utf8(str, 128, CMSGV(LABELS::keybind_col[m_tab_num][1]), col+1);
		AG_TableAddCol(tbl, str, "120px", NULL);
	}

	AG_TableBegin(tbl);
	for(int row=0; row<rows; row++) {
		kbdata->GetCellString(row, -1, labels[0]);

			for(int col=0; col<cols; col++) {
				kbdata->GetCellString(row, col, labels[col + 1]);
			}

			AG_TableAddRow(tbl, "%s:%s:%s",labels[0],labels[1],labels[2]);
		}
		AG_TableAddRow(tbl, "%s:%s:%s","","","");
		AG_TableEnd(tbl);

		AG_TableSetCellClickFn(tbl, OnClickCell, "%Cp", dlg);
		AG_SetEvent(tbl, "key-down", OnKeyDown, "%Cp", dlg);

		// checkbox for joypad
		AG_LabelNew(vbox, 0, " ");
		if (LABELS::keybind_combi[m_tab_num] != CMsg::Null) {
			m_combi = (int)kbdata->GetCombi();
			chkCombi = AG_CheckboxNewInt(vbox, 0, CMSGV(LABELS::keybind_combi[m_tab_num]), &m_combi);
		} else {
			AG_LabelNew(vbox, 0, " ");
		}

		// right button
		vbox = AG_BoxNewVert(tab, AG_BOX_EXPAND);
		AG_ButtonNewFn(vbox, AG_BUTTON_HFILL, CMSG(Load_Default), OnLoadDefault, "%Cp", dlg);
		AG_LabelNew(vbox, 0, " ");
		for(int i=0; i<4; i++) {
			UTILITY::sprintf_utf8(str, 128, CMSG(Load_Preset_VDIGIT), i+1);
			AG_ButtonNewFn(vbox, AG_BUTTON_HFILL, str, OnLoadPreset, "%Cp %i", dlg, i);
		}
		AG_LabelNew(vbox, 0, " ");
		for(int i=0; i<4; i++) {
			UTILITY::sprintf_utf8(str, 128, CMSG(Save_Preset_VDIGIT), i+1);
			AG_ButtonNewFn(vbox, AG_BUTTON_HFILL, str, OnSavePreset, "%Cp %i", dlg, i);
		}

		AG_LabelNew(vbox, 0, " ");
}

void AG_KEYBIND_CTRL::Final()
{
	if (kbdata->m_devtype == KeybindData::DEVTYPE_JOYPAD) {
		AG_TableSetPollInterval(tbl, 0);
	}
}

void AG_KEYBIND_CTRL::AdjustColumnPosition()
{
	for(int i=1; i<3; i++) {
		tbl->cols[i].x = tbl->cols[i-1].x + tbl->cols[i-1].w + 1;
	}
}

void AG_KEYBIND_CTRL::CopyColumnPositionAndSize(const AG_KEYBIND_CTRL *src_ctrl)
{
	for(int i=0; i<3; i++) {
		tbl->cols[i].x = src_ctrl->tbl->cols[i].x;
		tbl->cols[i].w = src_ctrl->tbl->cols[i].w;
	}
}

void AG_KEYBIND_CTRL::Update()
{
	char labels[3][100];
	int rows = kbdata->GetNumberOfRows();
	int cols = kbdata->GetNumberOfColumns();

	AG_TableBegin(tbl);
	for(int row=0; row<rows; row++) {
		kbdata->GetCellString(row, -1, labels[0]);

		for(int col=0; col<cols; col++) {
			kbdata->GetCellString(row, col, labels[col + 1]);
		}

		AG_TableAddRow(tbl, "%s:%s:%s",labels[0],labels[1],labels[2]);
	}
	AG_TableAddRow(tbl, "%s:%s:%s","","","");
	AG_TableEnd(tbl);

	AG_TableDeselectAllRows(tbl);
	AG_TableSelectCell(tbl, selected.row, selected.col);

	if (chkCombi) {
		m_combi = (int)kbdata->GetCombi();
		AG_WidgetUpdate(chkCombi);
	}
}

void AG_KEYBIND_CTRL::SetData()
{
	kbdata->SetData();

	if (chkCombi) {
		kbdata->SetCombi(m_combi);
	}
}

void AG_KEYBIND_CTRL::OnUpdate(AG_Event *event)
{
	AG_KEYBIND_DLG *dlg = (AG_KEYBIND_DLG *)AG_PTR(1);

	dlg->UpdateJoy();
}

void AG_KEYBIND_CTRL::OnClickCell(AG_Event *event)
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

	dlg->ClickCell(tbl, row, col);
}

void AG_KEYBIND_CTRL::OnKeyDown(AG_Event *event)
{
	AG_Table *tbl = (AG_Table *)AG_SELF();
	AG_KEYBIND_DLG *dlg = (AG_KEYBIND_DLG *)AG_PTR(1);
	int sym = AG_INT(2);
	int mod = AG_INT(3);
	Uint32 unicode = AG_UINT(4);

	dlg->UpdateKey(tbl, sym, mod, (Uint32)unicode);
}

void AG_KEYBIND_CTRL::OnLoadDefault(AG_Event *event)
{
	AG_KEYBIND_DLG *dlg = (AG_KEYBIND_DLG *)AG_PTR(1);

	dlg->LoadData(-1);
}

void AG_KEYBIND_CTRL::OnLoadPreset(AG_Event *event)
{
	AG_KEYBIND_DLG *dlg = (AG_KEYBIND_DLG *)AG_PTR(1);
	int num = AG_INT(2);

	dlg->LoadData(num);
}

void AG_KEYBIND_CTRL::OnSavePreset(AG_Event *event)
{
	AG_KEYBIND_DLG *dlg = (AG_KEYBIND_DLG *)AG_PTR(1);
	int num = AG_INT(2);

	dlg->SaveData(num);
}

//

AG_KEYBIND_DLG::AG_KEYBIND_DLG(EMU *parent_emu, AG_GUI_BASE *parent_gui) : AG_DLG(parent_emu, parent_gui)
{
	for(int tab=0; tab<KeybindData::TABS_MAX; tab++) {
		AG_KEYBIND_CTRL *kc = new AG_KEYBIND_CTRL(this, tab);
		ctrls.Add(kc);
	}

	tm1 = 0;
}

AG_KEYBIND_DLG::~AG_KEYBIND_DLG()
{
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
	AG_Box *hbox;

	selected_tab = 0;

	nb = AG_NotebookNew(win, AG_NOTEBOOK_HFILL);

	for(int tab_num=0; tab_num<ctrls.Count(); tab_num++) {
		AG_KEYBIND_CTRL *ctrl = ctrls[tab_num];

		ctrl->Init(emu, nb);
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
	ctrls[0]->AdjustColumnPosition();

	for(int tab_num=1; tab_num<ctrls.Count(); tab_num++) {
		ctrls[tab_num]->CopyColumnPositionAndSize(ctrls[0]);
	}
}

void AG_KEYBIND_DLG::Update(int tab_num)
{
	AG_KEYBIND_CTRL *kc = ctrls[tab_num];
	kc->Update();
}

Uint32 AG_KEYBIND_DLG::translate_vkkey(Uint32 code)
{
//	UINT32 new_code = code;

//	emu->translate_sdl_keysym();
	return code;
}

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

void AG_KEYBIND_DLG::LoadData(int num)
{
	int tab = selected_tab;
	AG_KEYBIND_CTRL *kc = ctrls[tab];
	kc->kbdata->LoadPreset(num);
	Update(tab);
}

void AG_KEYBIND_DLG::SaveData(int num)
{
	int tab = selected_tab;
	AG_KEYBIND_CTRL *kc = ctrls[tab];
	kc->kbdata->SavePreset(num);
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
#endif

}; /* namespace GUI_AGAR */
