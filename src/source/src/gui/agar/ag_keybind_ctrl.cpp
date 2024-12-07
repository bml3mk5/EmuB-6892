/** @file ag_keybind_ctrl.cpp

	Skelton for retropc emulator
	SDL + Ager edition

	@author Sasaji
	@date   2024.11.20

	@brief [ ag_keybind_ctrl ]
*/

#include "ag_keybind_ctrl.h"
#include "ag_gui_base.h"
#include "../gui.h"
#include "../../vm/vm.h"
#include "../../utility.h"
#include "../../labels.h"

namespace GUI_AGAR
{

//

AG_KEYBIND_CTRL::AG_KEYBIND_CTRL(AG_DLG *parent, int tab_num, AG_GUI_BASE *parent_gui)
{
	gui = parent_gui;
	m_tab_num = tab_num;
	dlg = parent;

	kbdata = new KeybindData();
	tab = NULL;
	tbl = NULL;

	m_combi = 0;
	chkCombi = NULL;

	tm1 = 0;

	memset(&selected, 0, sizeof(selected));
}

AG_KEYBIND_CTRL::~AG_KEYBIND_CTRL()
{
	delete kbdata;
}

void AG_KEYBIND_CTRL::Init(EMU *emu, AG_Notebook *nb, const char *title)
{
	char labels[3][128];
	char str[128];

	kbdata->Init(emu, m_tab_num);

	int rows = kbdata->GetNumberOfRows();
	int cols = kbdata->GetNumberOfColumns();

	tab = AG_NotebookAddTab(nb, title, AG_BOX_HORIZ);

	// table
	AG_Box *vbox;
	vbox = AG_BoxNewVert(tab, AG_BOX_VFILL);
	if (kbdata->m_devtype == KeybindData::DEVTYPE_JOYPAD) {
		tbl = AG_TableNewPolled(vbox, AG_TABLE_EXPAND | AG_TABLE_NOAUTOSORT, OnUpdate, "%Cp", this);
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

	AG_TableSetCellClickFn(tbl, OnClickCell, "%Cp", this);
	AG_SetEvent(tbl, "key-down", OnKeyDown, "%Cp", this);

	// checkbox for joypad
	AG_LabelNew(vbox, 0, " ");
	if (LABELS::keybind_combi[m_tab_num] != CMsg::Null) {
		m_combi = (int)kbdata->GetCombi();
		chkCombi = AG_CheckboxNewInt(vbox, 0, CMSGV(LABELS::keybind_combi[m_tab_num]), &m_combi);
	} else {
		AG_LabelNew(vbox, 0, " ");
	}
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

void AG_KEYBIND_CTRL::UpdateKey(AG_Table *tbl, int sym, int mod, Uint32 unicode)
{
	int row = selected.row;
	int col = selected.col;
	short scancode = gui->sdl_event.key.keysym.scancode;
	int new_key = 0;

	if (kbdata->m_devtype == KeybindData::DEVTYPE_KEYBOARD && col > 0) {
		// based SDL event
		emu->translate_keysym(0,sym,scancode,&new_key);
		kbdata->ClearVkKeyCode(row, col-1, NULL);
		if (kbdata->m_flags & KeybindData::FLAG_DENY_DUPLICATE) {
#ifndef USE_SDL2
			kbdata->ClearCellByVkKeyCode(new_key, NULL);
#else
			kbdata->ClearCellByVkKeyCode(scancode, NULL);
#endif
		}
#ifndef USE_SDL2
		kbdata->SetVkKeyCode(row, col-1, new_key, NULL);
#else
		kbdata->SetVkKeyCode(row, col-1, scancode, NULL);
#endif
		Update();
	}
}

void AG_KEYBIND_CTRL::UpdateJoy()
{
	int row = selected.row;
	int col = selected.col;

	if (col > 0) {
		Uint32 *joy_stat = emu->joy_real_buffer(col-1);
		if (joy_stat && (joy_stat[0] | joy_stat[1])) {
			kbdata->SetVkJoyCode(row, col-1, joy_stat[0], joy_stat[1], NULL);
			Update();
		}
	}
}

void AG_KEYBIND_CTRL::ClickCell(AG_Table *tbl, int row, int col)
{
	if (selected.cell == AG_TableGetCell(tbl, row, col)) {
		// double click ?
		Uint32 tm2 = AG_GetTicks();
		if (tm2 - tm1 < 500) {
			if (kbdata->m_devtype == KeybindData::DEVTYPE_JOYPAD) {
				kbdata->ClearVkJoyCode(row, col-1, NULL);
			} else {
				kbdata->ClearVkKeyCode(row, col-1, NULL);
			}
			Update();
		}
	}

	AG_TableDeselectRow(tbl, row);
	if (row + 1 < tbl->m) {
		if (col < 1) col = 1;
		AG_TableSelectCell(tbl, row, col);

		selected.cell = AG_TableGetCell(tbl, row, col);
		selected.row = row;
		selected.col = col;
	}
	tm1 = AG_GetTicks();
}

void AG_KEYBIND_CTRL::LoadData(int num)
{
	kbdata->LoadPreset(num);
	Update();
}

void AG_KEYBIND_CTRL::SaveData(int num)
{
	kbdata->SavePreset(num);
}

void AG_KEYBIND_CTRL::OnUpdate(AG_Event *event)
{
	AG_KEYBIND_CTRL *ctrl = (AG_KEYBIND_CTRL *)AG_PTR(1);

	ctrl->UpdateJoy();
}

void AG_KEYBIND_CTRL::OnClickCell(AG_Event *event)
{
	AG_Table *tbl = (AG_Table *)AG_SELF();
	AG_KEYBIND_CTRL *ctrl = (AG_KEYBIND_CTRL *)AG_PTR(1);
	int row = AG_INT(2);
//	int col = AG_INT(3);	// oops...cannot get... bug?
	int col = 0;

	for(int i=0; i<3; i++) {
		if (AG_TableCellSelected(tbl, row, i)) {
			col = i;
			break;
		}
	}

	ctrl->ClickCell(tbl, row, col);
}

void AG_KEYBIND_CTRL::OnKeyDown(AG_Event *event)
{
	AG_Table *tbl = (AG_Table *)AG_SELF();
	AG_KEYBIND_CTRL *ctrl = (AG_KEYBIND_CTRL *)AG_PTR(1);
	int sym = AG_INT(2);
	int mod = AG_INT(3);
	Uint32 unicode = AG_UINT(4);

	ctrl->UpdateKey(tbl, sym, mod, (Uint32)unicode);
}

void AG_KEYBIND_CTRL::OnLoadDefault(AG_Event *event)
{
	AG_KEYBIND_CTRL *ctrl = (AG_KEYBIND_CTRL *)AG_PTR(1);

	ctrl->LoadData(-1);
}

void AG_KEYBIND_CTRL::OnLoadPreset(AG_Event *event)
{
	AG_KEYBIND_CTRL *ctrl = (AG_KEYBIND_CTRL *)AG_PTR(1);
	int num = AG_INT(2);

	ctrl->LoadData(num);
}

void AG_KEYBIND_CTRL::OnSavePreset(AG_Event *event)
{
	AG_KEYBIND_CTRL *ctrl = (AG_KEYBIND_CTRL *)AG_PTR(1);
	int num = AG_INT(2);

	ctrl->SaveData(num);
}

}; /* namespace GUI_AGAR */
