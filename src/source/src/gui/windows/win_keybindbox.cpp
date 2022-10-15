/** @file win_keybindbox.cpp

	HITACHI BASIC MASTER LEVEL3 Mark5 / MB-S1 Emulator 'EmuB-6892/EmuB-S1'
	Skelton for retropc emulator

	@author Sasaji
	@date   2012.03.31 -

	@brief [ keybind box ]
*/

#include "win_keybindbox.h"
#include "../../emu.h"
#include "../../clocale.h"
#include "../../utility.h"
#include "win_gui.h"
#include "../../labels.h"

namespace GUI_WIN
{

KeybindBox::KeybindBox(HINSTANCE hInst, CFont *new_font, EMU *new_emu, GUI *new_gui)
	: CDialogBox(hInst, IDD_KEYBIND, new_font, new_emu, new_gui)
{
}

KeybindBox::~KeybindBox()
{
}

INT_PTR KeybindBox::onInitDialog(UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND hCtrl;
	TCITEM tcitm;
	_TCHAR label[KBLABEL_MAXLEN];

	CDialogBox::onInitDialog(message, wParam, lParam);

	// create dialog
	selected_tabctrl = 0;

	SIZE siz;
	font->GetTextSize(hDlg, NULL, &siz);
	for(int tab_num=0; tab_num<KEYBIND_MAX_NUM; tab_num++) {
		hCtrl =	CreateControl(NULL, _T("KeyBindCtrl"), IDC_CUSTOM0 + tab_num, 100, 340, WS_BORDER | WS_VSCROLL, 0, SM_CXVSCROLL);
		kbctl[tab_num] = KeybindControl::GetPtr(hCtrl);

		kbctl[tab_num]->Init(emu, tab_num, font->GetFont());

		kbctl[tab_num]->SetCellSize(siz.cx * 16 + padding * 2, siz.cy + padding * 2);

		kbctl[tab_num]->MapDefaultVmKey();

		kbctl[tab_num]->Update();
	}
	for(int tab_num=0; tab_num<KEYBIND_MAX_NUM; tab_num++) {
		kbctl[tab_num]->SetTitleLabel(LABELS::keybind_col[tab_num][0], LABELS::keybind_col[tab_num][1]);
	}

	// tab control
	CBox *box_all = new CBox(CBox::VerticalBox, 0, margin, _T("all"));
	CBox *box_tab = AdjustTabControl(box_all, IDC_TAB1, IDC_STATIC_0);

	HWND hTabCtrl = GetDlgItem(hDlg, IDC_TAB1);

	tcitm.mask = TCIF_TEXT;

	for(int tab_num=0; tab_num<KEYBIND_MAX_NUM; tab_num++) {
		UTILITY::tcscpy(label, KBLABEL_MAXLEN, CMSGVM(LABELS::keybind_tab[tab_num]));
		tcitm.pszText = label;
		TabCtrl_InsertItem(hTabCtrl , tab_num , &tcitm);
	}
	TabCtrl_SetCurSel(hTabCtrl, selected_tabctrl);

	//
	// adjust control size
	//

	// kb control
	CBox *box_hall0 = NULL;
	for(int tab_num=0; tab_num<KEYBIND_MAX_NUM; tab_num++) {
		CBox *box_hall = box_tab->AddBox(CBox::HorizontalBox);
		CBox *box_kb = box_hall->AddBox(CBox::VerticalBox);
		AdjustControl(box_kb, IDC_CUSTOM0 + tab_num, kbctl[tab_num]->GetWidth(), 340, SM_CXVSCROLL);

		if (LABELS::keybind_combi[tab_num] != CMsg::Null) {
			int id = get_combi_id(kbctl[tab_num]);
			if (id) {
				CreateCheckBox(box_kb, id, LABELS::keybind_combi[tab_num], false);
				CheckDlgButton(hDlg, id, kbctl[tab_num]->GetCombi() != 0);
			}
		}
		if (tab_num == 0) {
			box_hall0 = box_hall;
		}
	}

	// buttons
	CBox *box_vbtn = new CBox(CBox::VerticalBox, 0, 0, _T("vbtn"));
	box_hall0->AddBox(box_vbtn);
	int n = 0;
	for(int i=0; LABELS::keybind_btn[i] != CMsg::End; i++) {
		box_vbtn->AddSpace(0, margin >> 1);
		if (LABELS::keybind_btn[i] == CMsg::Null) {
			box_vbtn->AddSpace(0, margin);
			continue;
		}
		CreateButton(box_vbtn, IDC_BTN_LOAD_DEFAULT + n, LABELS::keybind_btn[i], 8);
		n++;
	}

	// tab control size

	// ok cancel
	CBox *box_btn = new CBox(CBox::HorizontalBox, CBox::RightPos);
	box_all->AddBox(box_btn);
	CreateButton(box_btn, IDOK, CMsg::OK, 8);
	CreateButton(box_btn, IDCANCEL, CMsg::Cancel, 8);

	RECT prc;
	GetClientRect(hTabCtrl, &prc);
	TabCtrl_AdjustRect(hTabCtrl, FALSE, &prc);
	box_tab->SetTopMargin(prc.top + 8);

	// dialog size
	box_all->Realize(*this);

	select_tabctrl(selected_tabctrl);

	delete box_all;

	return (INT_PTR)TRUE;
}

INT_PTR KeybindBox::onCommand(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(LOWORD(wParam)) {
	case IDOK:
	case IDCANCEL:
		if (LOWORD(wParam) == IDOK) {
			onClickOk();
		}
		::EndDialog(hDlg, LOWORD(wParam));
		return (INT_PTR)TRUE;

	case IDC_BTN_LOAD_DEFAULT:
		return onClickLoadDefault();

	case IDC_BTN_LOAD_PRESET1:
	case IDC_BTN_LOAD_PRESET2:
	case IDC_BTN_LOAD_PRESET3:
	case IDC_BTN_LOAD_PRESET4:
		return onClickLoadPreset(LOWORD(wParam) - IDC_BTN_LOAD_PRESET1);

	case IDC_BTN_SAVE_PRESET1:
	case IDC_BTN_SAVE_PRESET2:
	case IDC_BTN_SAVE_PRESET3:
	case IDC_BTN_SAVE_PRESET4:
		return onClickSavePreset(LOWORD(wParam) - IDC_BTN_SAVE_PRESET1);

	}
	return (INT_PTR)FALSE;
}

INT_PTR KeybindBox::onNotify(UINT message, WPARAM wParam, LPARAM lParam)
{
	// change tab
	LPNMHDR lpNmHdr = (NMHDR *)lParam;
	int i;
	if (lpNmHdr->idFrom == IDC_TAB1) {
		switch (lpNmHdr->code) {
		case TCN_SELCHANGE:
			i = TabCtrl_GetCurSel(lpNmHdr->hwndFrom);
			select_tabctrl(i);
			break;
		}
	}
	return (INT_PTR)TRUE;
}

INT_PTR KeybindBox::onMouseWheel(UINT message, WPARAM wParam, LPARAM lParam)
{
	SendDlgItemMessage(hDlg, IDC_CUSTOM0 + selected_tabctrl, message, wParam, lParam);
	return (INT_PTR)TRUE;
}

#if 0
INT_PTR KeybindBox::onControlColorStatic(UINT message, WPARAM wParam, LPARAM lParam)
{
	HANDLE h = (HANDLE)GetStockObject(NULL_BRUSH);
	SetBkMode((HDC)wParam, TRANSPARENT);
	return (INT_PTR)h;
}
#endif

INT_PTR KeybindBox::onClickOk()
{
	for(int tab_num=0; tab_num<KEYBIND_MAX_NUM; tab_num++) {
		kbctl[tab_num]->SetData();

		int id = get_combi_id(kbctl[tab_num]);
		if (id) {
			kbctl[tab_num]->SetCombi(IsDlgButtonChecked(hDlg, id) ? 1 : 0);
		}
	}
	return (INT_PTR)TRUE;
}

INT_PTR KeybindBox::onClickLoadDefault()
{
	int tab_num = selected_tabctrl;

	kbctl[tab_num]->LoadDefaultPreset();

	int id = get_combi_id(kbctl[tab_num]);
	if (id) {
		CheckDlgButton(hDlg, id, kbctl[tab_num]->GetCombi() != 0);
	}
	return (INT_PTR)TRUE;
}

INT_PTR KeybindBox::onClickLoadPreset(int idx)
{
	int tab_num = selected_tabctrl;

	kbctl[tab_num]->LoadPreset(idx);

	int id = get_combi_id(kbctl[tab_num]);
	if (id) {
		CheckDlgButton(hDlg, id, kbctl[tab_num]->GetCombi() != 0);
	}
	return (INT_PTR)TRUE;
}

INT_PTR KeybindBox::onClickSavePreset(int idx)
{
	int tab_num = selected_tabctrl;

	kbctl[tab_num]->SavePreset(idx);

	int id = get_combi_id(kbctl[tab_num]);
	if (id) {
		kbctl[tab_num]->SetCombi(IsDlgButtonChecked(hDlg, id) ? 1 : 0);
	}
	return (INT_PTR)TRUE;
}

void KeybindBox::select_tabctrl(int tab_num)
{
	HWND hCtrl;

	selected_tabctrl = tab_num;
	for(int i=0; i<KEYBIND_MAX_NUM; i++) {
		hCtrl = GetDlgItem(hDlg, IDC_CUSTOM0 + i);
		ShowWindow(hCtrl, i == selected_tabctrl ? SW_SHOW : SW_HIDE);
	}

	hCtrl = GetDlgItem(hDlg, IDC_CHK_COMBI1);
	ShowWindow(hCtrl, SW_HIDE);
	hCtrl = GetDlgItem(hDlg, IDC_CHK_COMBI2);
	ShowWindow(hCtrl, SW_HIDE);

	int id = get_combi_id(kbctl[tab_num]);
	if (id) {
		hCtrl = GetDlgItem(hDlg, id);
		ShowWindow(hCtrl, SW_SHOW);
	}
}

int KeybindBox::get_combi_id(KeybindControl *kbctl)
{
	int id = 0;
	if (kbctl->devtype == 1) {
		// joystick
		switch(kbctl->vm_type) {
		case 0:
			// key assigned
			id = IDC_CHK_COMBI1;
			break;
		case 2:
			// negative logic
			id = IDC_CHK_COMBI2;
			break;
		}
	}
	return id;
}

}; /* namespace GUI_WIN */
