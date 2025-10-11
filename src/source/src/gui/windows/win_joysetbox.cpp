/** @file win_joysetbox.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2023.01.07 -

	@brief [ joypad setting box ]
*/
#include "win_joysetbox.h"
#include "../../emu.h"
#include "../../emumsg.h"
#include "win_gui.h"
#include "../../msgs.h"
#include "../../labels.h"
#include "../gui_keybinddata.h"
#include "../../keycode.h"
#include "../../utility.h"

namespace GUI_WIN
{

JoySettingBox::JoySettingBox(HINSTANCE hInst, CFont *new_font, EMU *new_emu, GUI *new_gui)
	: KeybindBaseBox(hInst, IDD_JOYSETTING, new_emu, new_gui)
{
}

JoySettingBox::~JoySettingBox()
{
}

INT_PTR JoySettingBox::onInitDialog(UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND hCtrl;
	TCITEM tcitm;
	_TCHAR label[KBLABEL_MAXLEN];
	int tab_offset = KeybindData::JS_TABS_MIN;

	CDialogBox::onInitDialog(message, wParam, lParam);

	// create dialog
	m_selected_tabctrl = 0;

	SIZE siz;
	font->GetTextSize(hDlg, NULL, &siz);

	// calculate number of tabs
	int max_rows = 6;
	for(int tab_num=tab_offset; tab_num < KeybindData::JS_TABS_MAX; tab_num++) {
		hCtrl =	CreateControl(NULL, _T("KeyBindCtrl"), IDC_CUSTOM0 + tab_num - tab_offset, 100, 180, WS_BORDER | WS_VSCROLL, 0, SM_CXVSCROLL);
		KeybindControl *kc = KeybindControl::GetPtr(hCtrl);

		kc->Init(emu, tab_num, font->GetFont());
		kc->SetCellSize(siz.cx * 18 + padding * 2, siz.cy + padding * 2);
		kc->MapDefaultVmKey();
		kc->SetJoyMask(&m_joy_mask);
		kc->Update();

		kc->SetTitleLabel(LABELS::keybind_col[tab_num][0], LABELS::keybind_col[tab_num][1]);

		if (max_rows < kc->GetNumberOfRows()) {
			max_rows = kc->GetNumberOfRows();
		}

		m_kbctl.push_back(kc);
	}

	//

	CBox *box_all = new CBox(CBox::VerticalBox, 0, margin);
	CBox *box_hall = box_all->AddBox(CBox::HorizontalBox);

	//
	// controller type and button mashing 
	//
	SIZE sz;
	sz.cx = 80; sz.cy = 24;
	int tx = 70;

#if defined(USE_PIAJOYSTICK) || defined(USE_PSGJOYSTICK) || defined(USE_KEY2JOYSTICK)
//	CBox *hbox_joy = box_hall->AddBox(CBox::HorizontalBox, 0, margin);

	int val = 0;
	for(int i=0; i<MAX_JOYSTICKS; i++) {
		CBox *vbox = box_hall->AddBox(CBox::VerticalBox);

		UTILITY::stprintf(label, 64, CMSGM(JoypadVDIGIT), i + 1);
		CreateStatic(vbox, IDC_STATIC, label);
#ifdef USE_JOYSTICK_TYPE
		val = pConfig->joy_type[i];
		CreateComboBox(vbox, IDC_COMBO_JOY1 + i, LABELS::joypad_type, val, 8);
#endif
		CreateStatic(vbox, IDC_STATIC, CMsg::Button_Mashing_Speed);
		CBox *hbox = vbox->AddBox(CBox::HorizontalBox);
		hbox->AddSpace(tx, 0);
		CreateStatic(hbox, IDC_STATIC, _T("0 <-> 3"));

		for(int k=0; k < KEYBIND_JOY_BUTTONS; k++) {
			int kk = k + VM_JOY_LABEL_BUTTON_A;
			if (kk >= VM_JOY_LABELS_MAX) break;

			CBox *box = vbox->AddBox(CBox::HorizontalBox);

			CMsg::Id id = (CMsg::Id)cVmJoyLabels[kk].id;
			CreateStatic(box, IDC_STATIC, CMSGVM(id), tx, 0, SS_CENTER);
			val = pConfig->joy_mashing[i][k];
			CreateSlider(box, IDC_SLIDER_JOY1 + i * KEYBIND_JOY_BUTTONS + k,
				sz.cx, sz.cy, 0, 3, 1, val, false);
		}

		CreateStatic(vbox, IDC_STATIC, CMsg::Analog_to_Digital_Sensitivity);
		hbox = vbox->AddBox(CBox::HorizontalBox);
		hbox->AddSpace(tx, 0);
		CreateStatic(hbox, IDC_STATIC, _T("0 <-> 10"));

		for(int k=0; k < 6; k++) {
			CBox *box = vbox->AddBox(CBox::HorizontalBox);

			CMsg::Id id = LABELS::joypad_axis[k];
			CreateStatic(box, IDC_STATIC, CMSGVM(id), tx, 0, SS_CENTER);
			val = 10 - pConfig->joy_axis_threshold[i][k];
			CreateSlider(box, IDC_SLIDER_AXIS1 + i * 6 + k,
				sz.cx, sz.cy, 0, 10, 1, val, false);
		}
	}
#endif

	//
	// tab control for button assining
	//
	CBox *box_vall = box_hall->AddBox(CBox::VerticalBox);
	CBox *box_tab = AdjustTabControl(box_vall, IDC_TAB1, IDC_STATIC_0);

	HWND hTabCtrl = GetDlgItem(hDlg, IDC_TAB1);

	tcitm.mask = TCIF_TEXT;

	for(int tab_num=0; tab_num<(int)m_kbctl.size(); tab_num++) {
//		UTILITY::tcscpy(label, KBLABEL_MAXLEN, CMSGVM(LABELS::joysetting_tab[tab_num]));
		UTILITY::stprintf(label, KBLABEL_MAXLEN, _T("%d"), tab_num + 1);
		tcitm.pszText = label;
		TabCtrl_InsertItem(hTabCtrl , tab_num , &tcitm);
	}
	TabCtrl_SetCurSel(hTabCtrl, m_selected_tabctrl);

	//
	// adjust control size
	//

	// kb control
	CBox *box_vall0 = NULL;
	for(int tab_num=0; tab_num<(int)m_kbctl.size(); tab_num++) {
		CBox *box_v = box_tab->AddBox(CBox::VerticalBox);
		CBox *box_kb = box_v->AddBox(CBox::VerticalBox);

		CreateStatic(box_kb, IDC_STATIC_10 + tab_num, LABELS::joysetting_tab[tab_num]);

		AdjustControl(box_kb, IDC_CUSTOM0 + tab_num, m_kbctl[tab_num]->GetWidth(), (max_rows + 1) * (siz.cy + padding * 2 + 4), SM_CXVSCROLL);

//		kbctl[tab_num]->AddCombiCheckButton(this, box_kb);

#if defined(USE_PIAJOYSTICK) || defined(USE_KEY2PIAJOYSTICK)
		if (tab_num + KeybindData::JS_TABS_MIN == Keybind::TAB_JOY2JOY) {
# ifdef USE_JOYSTICKBIT
			// check button
			CBox *box_cbtn = box_kb->AddBox(CBox::VerticalBox);
			CreateCheckBox(box_cbtn, IDC_CHK_PIAJOY_NEGATIVE, CMSGM(Signals_are_negative_logic), FLG_PIAJOY_NEGATIVE != 0);	
//			CreateCheckBox(box_cbtn, IDC_COMBO_PIAJOY_CONNTO, CMSGM(Connect_to_standard_PIA_A_port), pConfig->piajoy_conn_to != 0);
			CBox *hbox = box_cbtn->AddBox(CBox::HorizontalBox);
			CreateStatic(hbox, IDC_STATIC_20, CMsg::Connect_to_);
			for(int i=0; LABELS::joysetting_opts[i] != CMsg::End; i++) {
				CreateRadioButton(hbox, IDC_COMBO_PIAJOY_CONNTO + i, LABELS::joysetting_opts[i], (i == 0));
				CheckDlgButton(hDlg, IDC_COMBO_PIAJOY_CONNTO + i, pConfig->piajoy_conn_to == i);
			}
# else
			CBox *box_cbtn = box_kb->AddBox(CBox::VerticalBox);
			CreateCheckBox(box_cbtn, IDC_CHK_PIAJOY_NOIRQ, CMSGM(No_interrupt_caused_by_pressing_the_button), FLG_PIAJOY_NOIRQ != 0);	
# endif
		}
#endif
#if defined(USE_PSGJOYSTICK) || defined(USE_KEY2PSGJOYSTICK)
		if (tab_num + KeybindData::JS_TABS_MIN == Keybind::TAB_JOY2JOYB) {
# ifdef USE_JOYSTICKBIT
			// check button
			CBox *box_cbtn = box_kb->AddBox(CBox::VerticalBox);
			CreateCheckBox(box_cbtn, IDC_CHK_PSGJOY_NEGATIVE, CMSGM(Signals_are_negative_logic), FLG_PSGJOY_NEGATIVE != 0);	
# endif
		}
#endif
		if (tab_num == 0) {
			box_vall0 = box_v;
		}
	}

	// buttons
	CBox *box_hbtn = box_vall0->AddBox(CBox::HorizontalBox);
	CBox *box_vbtn = box_hbtn->AddBox(CBox::VerticalBox);
	int n = 0;
	for(int i=0; LABELS::keybind_btn[i] != CMsg::End; i++) {
		if (LABELS::keybind_btn[i] == CMsg::Null) {
			box_vbtn = box_hbtn->AddBox(CBox::VerticalBox);
			continue;
		}
		CreateButton(box_vbtn, IDC_BTN_LOAD_DEFAULT + n, LABELS::keybind_btn[i], 8);
		n++;
	}

	// joypad
	create_dialog_footer(box_vall);

	// tab control size

	// ok cancel
	CBox *box_btn = box_all->AddBox(CBox::HorizontalBox, CBox::RightPos);
	CreateButton(box_btn, IDOK, CMsg::OK, 8, true);
	CreateButton(box_btn, IDCANCEL, CMsg::Cancel, 8, false);

//	RECT prc;
//	GetClientRect(hTabCtrl, &prc);
//	TabCtrl_AdjustRect(hTabCtrl, FALSE, &prc);
//	box_tab->SetTopMargin(prc.top + 8);


	box_all->Realize(*this);

	select_tabctrl(m_selected_tabctrl);

	delete box_all;

	return (INT_PTR)TRUE;
}

INT_PTR JoySettingBox::onHScroll(UINT message, WPARAM wParam, LPARAM lParam)
{
//	SetValue((HWND)lParam);

	return (INT_PTR)TRUE;
}

INT_PTR JoySettingBox::onVScroll(UINT message, WPARAM wParam, LPARAM lParam)
{
//	SetValue((HWND)lParam);

	return (INT_PTR)TRUE;
}

INT_PTR JoySettingBox::onClickOk()
{
	KeybindBaseBox::onClickOk();

	SetValue();

	return (INT_PTR)TRUE;
}

void JoySettingBox::select_tabctrl(int tab_num)
{
	HWND hCtrl;

	m_selected_tabctrl = tab_num;
	for(int i=0; i<(int)m_kbctl.size(); i++) {
		hCtrl = GetDlgItem(hDlg, IDC_CUSTOM0 + i);
		ShowWindow(hCtrl, i == m_selected_tabctrl ? SW_SHOW : SW_HIDE);
		hCtrl = GetDlgItem(hDlg, IDC_STATIC_10 + i);
		if (hCtrl) {
			ShowWindow(hCtrl, i == m_selected_tabctrl ? SW_SHOW : SW_HIDE);
		}
	}

#ifdef USE_PIAJOYSTICK
	const int arr0[] = {
# ifdef USE_JOYSTICKBIT
		IDC_CHK_PIAJOY_NEGATIVE,
		IDC_STATIC_20,
		IDC_COMBO_PIAJOY_CONNTO,
		IDC_COMBO_PIAJOY_CONNT1,
# else
		IDC_CHK_PIAJOY_NOIRQ,
# endif
		-1
	};
	for(int i=0; arr0[i]>=0; i++) {
		hCtrl = GetDlgItem(hDlg, arr0[i]);
		if (hCtrl) {
			ShowWindow(hCtrl, m_selected_tabctrl + KeybindData::JS_TABS_MIN == Keybind::TAB_JOY2JOY ? SW_SHOW : SW_HIDE);
		}
	}
#endif
#ifdef USE_PSGJOYSTICK
# ifdef USE_JOYSTICKBIT
	hCtrl = GetDlgItem(hDlg, IDC_CHK_PSGJOY_NEGATIVE);
	if (hCtrl) {
		ShowWindow(hCtrl, m_selected_tabctrl + KeybindData::JS_TABS_MIN == Keybind::TAB_JOY2JOYB ? SW_SHOW : SW_HIDE);
	}
# endif
#endif
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

void JoySettingBox::SetValue()
{
#if defined(USE_JOYSTICK) || defined(USE_KEY2JOYSTICK)
	for(int i=0; i<MAX_JOYSTICKS; i++) {
		for(int k = 0; k < KEYBIND_JOY_BUTTONS; k++) {
			int id = IDC_SLIDER_JOY1 + i * KEYBIND_JOY_BUTTONS + k;
			pConfig->joy_mashing[i][k] = (int)SendDlgItemMessage(hDlg, id, TBM_GETPOS, 0, 0);
		}
		for(int k = 0; k < 6; k++) {
			int id = IDC_SLIDER_AXIS1 + i * 6 + k;
			pConfig->joy_axis_threshold[i][k] = 10 - (int)SendDlgItemMessage(hDlg, id, TBM_GETPOS, 0, 0);
		}
# ifdef USE_JOYSTICK_TYPE
		pConfig->joy_type[i] = (int)::SendDlgItemMessage(hDlg, IDC_COMBO_JOY1 + i, CB_GETCURSEL, 0, 0);
# endif
	}
	emu->modify_joy_mashing();
	emu->modify_joy_threshold();
# ifdef USE_JOYSTICK_TYPE
	// will change joypad type in emu thread
	emumsg.Send(EMUMSG_ID_MODIFY_JOYTYPE);
# endif
# if defined(USE_PIAJOYSTICK) || defined(USE_KEY2PIAJOYSTICK)
#  ifdef USE_JOYSTICKBIT
	BIT_ONOFF(pConfig->misc_flags, MSK_PIAJOY_NEGATIVE, IsDlgButtonChecked(hDlg, IDC_CHK_PIAJOY_NEGATIVE) != 0);
	for(int i=0; LABELS::joysetting_opts[i] != CMsg::End; i++) {
		if (IsDlgButtonChecked(hDlg, IDC_COMBO_PIAJOY_CONNTO + i) == BST_CHECKED) {
			pConfig->piajoy_conn_to = i;
			break;
		}
	}
#  else
	BIT_ONOFF(pConfig->misc_flags, MSK_PIAJOY_NOIRQ, IsDlgButtonChecked(hDlg, IDC_CHK_PIAJOY_NOIRQ) != 0);	
#  endif
# endif
# if defined(USE_PSGJOYSTICK) || defined(USE_KEY2PSGJOYSTICK)
#  ifdef USE_JOYSTICKBIT
	BIT_ONOFF(pConfig->misc_flags, MSK_PSGJOY_NEGATIVE, IsDlgButtonChecked(hDlg, IDC_CHK_PSGJOY_NEGATIVE) != 0);
#  endif
# endif
#endif
}

#if 0
void JoySettingBox::SetValue(HWND ctrl)
{
#if defined(USE_JOYSTICK) || defined(USE_KEY2JOYSTICK)
	int i = (int)::GetDlgCtrlID(ctrl);
	i -= IDC_SLIDER_JOY1;
	int k = i % KEYBIND_JOY_BUTTONS;
	i /= KEYBIND_JOY_BUTTONS;

	pConfig->joy_mashing[i][k] = (int)SendMessage(ctrl, TBM_GETPOS, 0, 0);
	emu->set_joy_mashing();
#endif
}
#endif

}; /* namespace GUI_WIN */
