/** @file win_keybindbox.cpp

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
#include "../../keycode.h"

namespace GUI_WIN
{

KeybindBox::KeybindBox(HINSTANCE hInst, CFont *new_font, EMU *new_emu, GUI *new_gui)
	: KeybindBaseBox(hInst, IDD_KEYBIND, new_emu, new_gui)
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
	int tab_offset = KeybindData::KB_TABS_MIN;

	CDialogBox::onInitDialog(message, wParam, lParam);

	// create dialog
	m_selected_tabctrl = 0;

	SIZE siz;
	font->GetTextSize(hDlg, NULL, &siz);
	// calculate number of tabs
	for(int tab_num=tab_offset; tab_num < KeybindData::KB_TABS_MAX; tab_num++) {
		hCtrl =	CreateControl(NULL, _T("KeyBindCtrl"), IDC_CUSTOM0 + tab_num, 100, 380, WS_BORDER | WS_VSCROLL, 0, SM_CXVSCROLL);
		KeybindControl *kc = KeybindControl::GetPtr(hCtrl);

		kc->Init(emu, tab_num, font->GetFont());
		kc->SetCellSize(siz.cx * 16 + padding * 2, siz.cy + padding * 2);
		kc->MapDefaultVmKey();
		kc->SetJoyMask(&m_joy_mask);
		kc->Update();

		kc->SetTitleLabel(LABELS::keybind_col[tab_num][0], LABELS::keybind_col[tab_num][1]);

		m_kbctl.push_back(kc);
	}

	// tab control
	CBox *box_all = new CBox(CBox::VerticalBox, 0, margin, _T("all"));
	CBox *box_tab = AdjustTabControl(box_all, IDC_TAB1, IDC_STATIC_0);

	HWND hTabCtrl = GetDlgItem(hDlg, IDC_TAB1);

	tcitm.mask = TCIF_TEXT;

	for(int tab_num=0; tab_num<(int)m_kbctl.size(); tab_num++) {
		UTILITY::tcscpy(label, KBLABEL_MAXLEN, CMSGVM(LABELS::keybind_tab[tab_num]));
		tcitm.pszText = label;
		TabCtrl_InsertItem(hTabCtrl , tab_num , &tcitm);
	}
	TabCtrl_SetCurSel(hTabCtrl, m_selected_tabctrl);

	//
	// adjust control size
	//

	// kb control
	CBox *box_hall0 = NULL;
	for(int tab_num=0; tab_num<(int)m_kbctl.size(); tab_num++) {
		CBox *box_hall = box_tab->AddBox(CBox::HorizontalBox);
		CBox *box_kb = box_hall->AddBox(CBox::VerticalBox);
		AdjustControl(box_kb, IDC_CUSTOM0 + tab_num, m_kbctl[tab_num]->GetWidth(), 380, SM_CXVSCROLL);

		m_kbctl[tab_num]->AddCombiCheckButton(this, box_kb);

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

	// joypad
	create_dialog_footer(box_all);

	// tab control size

	// ok cancel
	CBox *box_btn = box_all->AddBox(CBox::HorizontalBox, CBox::RightPos);
	CreateButton(box_btn, IDOK, CMsg::OK, 8);
	CreateButton(box_btn, IDCANCEL, CMsg::Cancel, 8);

	RECT prc;
	GetClientRect(hTabCtrl, &prc);
	TabCtrl_AdjustRect(hTabCtrl, FALSE, &prc);
	box_tab->SetTopMargin(prc.top + 8);

	// dialog size
	box_all->Realize(*this);

	select_tabctrl(m_selected_tabctrl);

	delete box_all;

	return (INT_PTR)TRUE;
}

void KeybindBox::select_tabctrl(int tab_num)
{
	HWND hCtrl;

	m_selected_tabctrl = tab_num;
	for(int i=0; i<(int)m_kbctl.size(); i++) {
		hCtrl = GetDlgItem(hDlg, IDC_CUSTOM0 + i);
		ShowWindow(hCtrl, i == m_selected_tabctrl ? SW_SHOW : SW_HIDE);
	}

	for(int id=IDC_CHK_COMBI1; id<=IDC_CHK_COMBI3; id++) {
		hCtrl = GetDlgItem(hDlg, id);
		if (hCtrl) {
			ShowWindow(hCtrl, SW_HIDE);
		}
	}
	hCtrl = m_kbctl[tab_num]->GetCombiCheckButton();
	if (hCtrl) {
		ShowWindow(hCtrl, SW_SHOW);
	}
}

}; /* namespace GUI_WIN */
