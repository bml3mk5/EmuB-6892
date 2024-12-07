/** @file ag_joyset_dlg.cpp

	Skelton for retropc emulator
	SDL + Ager edition

	@author Sasaji
	@date   2023.01.29

	@brief [ ag_joyset_dlg ]
*/

#include "ag_joyset_dlg.h"
#include "../gui.h"
#include "../../config.h"
#include "../../msgs.h"
#include "../../labels.h"
#include "../../utility.h"

namespace GUI_AGAR
{

AG_JOYSET_DLG::AG_JOYSET_DLG(EMU *parent_emu, AG_GUI_BASE *parent_gui) : AG_DLG(parent_emu, parent_gui)
{
	int tab_offset = KeybindData::JS_TABS_MIN;
	for(int tab_num=tab_offset; tab_num<KeybindData::JS_TABS_MAX; tab_num++) {
		AG_KEYBIND_CTRL *kc = new AG_KEYBIND_CTRL(this, tab_num, parent_gui);
		ctrls.Add(kc);
	}
}

AG_JOYSET_DLG::~AG_JOYSET_DLG()
{
}
/*
 * create volume dialog
 */
void AG_JOYSET_DLG::Create()
{
	char str[128];

	AG_Window *win = AG_WindowNew(AG_WINDOW_MODAL | AG_WINDOW_DIALOG | AG_WINDOW_NORESIZE);
	AG_WindowSetCaptionS(win, CMSG(Joypad_Setting));

	AG_Box *vboxall;
	AG_Box *hboxall;
	AG_Box *hbox_mash;
	AG_Box *vbox;
	AG_Box *hbox;
	AG_Label *lbl;
	AG_Slider *sli;
	AG_Notebook *nb;
	AG_Box *vbox_nb;

	vboxall = AG_BoxNewVert(win, 0);
	hboxall = AG_BoxNewHoriz(vboxall, 0);

#if defined(USE_PIAJOYSTICK) || defined(USE_KEY2JOYSTICK)
	hbox_mash = AG_BoxNewHoriz(hboxall, 0);
	for(int i=0; i<MAX_JOYSTICKS; i++) {
		vbox = AG_BoxNewVert(hbox_mash, 0);

		UTILITY::sprintf_utf8(str, 64, CMSG(JoypadVDIGIT), i + 1);

		lbl = AG_LabelNewS(vbox, 0, str);

//		hbox = AG_BoxNewHoriz(vbox, 0);
		lbl = AG_LabelNewS(vbox, 0, CMSG(Button_Mashing_Speed));
		lbl = AG_LabelNewS(vbox, 0, "0 <-> 3");

		for(int k=0; k < KEYBIND_JOY_BUTTONS; k++) {
			int kk = k + VM_JOY_LABEL_BUTTON_A;
			if (kk >= VM_JOY_LABELS_MAX) break;

			hbox = AG_BoxNewHoriz(vbox, 0);

			CMsg::Id id = (CMsg::Id)cVmJoyLabels[kk].id;
			lbl = AG_LabelNewS(hbox, 0, CMSGV(id));
			AG_LabelJustify(lbl, AG_TEXT_CENTER);
			mash[i][k] = pConfig->joy_mashing[i][k];
			sli = AG_SliderNewIntR(hbox, AG_SLIDER_HORIZ, AG_SLIDER_HFILL, &mash[i][k], 0, 3);
			AG_SliderSetControlSize(sli, 12);
		}

//		hbox = AG_BoxNewHoriz(vbox, 0);
		lbl = AG_LabelNewS(vbox, 0, CMSG(Analog_to_Digital_Sensitivity));
		lbl = AG_LabelNewS(vbox, 0, "0 <-> 10");

		for(int k=0; k < 6; k++) {
			hbox = AG_BoxNewHoriz(vbox, 0);

			CMsg::Id id = LABELS::joypad_axis[k];
			lbl = AG_LabelNewS(hbox, 0, CMSGV(id));
			AG_LabelJustify(lbl, AG_TEXT_CENTER);
			axis[i][k] = pConfig->joy_axis_threshold[i][k];
			sli = AG_SliderNewIntR(hbox, AG_SLIDER_HORIZ, AG_SLIDER_HFILL, &axis[i][k], 0, 10);
			AG_SliderSetControlSize(sli, 12);
		}
	}
#endif

//	AG_WindowSetGeometryAligned(win, AG_WINDOW_MC, 640, 480);

	vbox_nb = AG_BoxNewVert(hboxall, AG_BOX_VFILL);

	// right notebook
	selected_tab = 0;
	nb = AG_NotebookNew(vbox_nb, AG_NOTEBOOK_VFILL);

	for(int tab_num=0; tab_num<ctrls.Count(); tab_num++) {
		AG_KEYBIND_CTRL *ctrl = ctrls[tab_num];
		ctrl->Init(emu, nb, CMSGV(LABELS::joysetting_tab[tab_num]));
	}

	// check button
#ifdef USE_PIAJOYSTICKBIT
	iChkPiaJoyNeg = (FLG_PIAJOY_NEGATIVE != 0 ? 1 : 0);
	AG_CheckboxNewInt(vbox_nb, 0, CMSG(Signals_are_negative_logic), &iChkPiaJoyNeg);
	iChkPiaJoyConn = (pConfig->piajoy_conn_to != 0 ? 1 : 0);
	AG_CheckboxNewInt(vbox_nb, 0, CMSG(Connect_to_standard_PIA_A_port), &iChkPiaJoyConn);
#else
	iChkPiaJoyNoIrq = (FLG_PIAJOY_NOIRQ != 0 ? 1 : 0);
	AG_CheckboxNewInt(vbox_nb, 0, CMSG(No_interrupt_caused_by_pressing_the_button), &iChkPiaJoyNoIrq);
#endif

	// right button
	AG_ButtonNewFn(vbox_nb, AG_BUTTON_HFILL, CMSG(Load_Default), OnLoadDefault, "%Cp", this);

	hbox = AG_BoxNewHoriz(vbox_nb, 0);
	vbox = AG_BoxNewVert(hbox, 0);
	for(int i=0; i<4; i++) {
		UTILITY::sprintf_utf8(str, 128, CMSG(Load_Preset_VDIGIT), i+1);
		AG_ButtonNewFn(vbox, AG_BUTTON_HFILL, str, OnLoadPreset, "%Cp %i", this, i);
	}
	vbox = AG_BoxNewVert(hbox, 0);
	for(int i=0; i<4; i++) {
		UTILITY::sprintf_utf8(str, 128, CMSG(Save_Preset_VDIGIT), i+1);
		AG_ButtonNewFn(vbox, AG_BUTTON_HFILL, str, OnSavePreset, "%Cp %i", this, i);
	}

	// add event
	AG_AddEvent(nb, "mouse-button-down", OnClickTab, "%Cp", this);

	// button
	hbox = AG_BoxNewHoriz(vboxall, 0);
	AG_ButtonNewFn(hbox, 0, CMSG(OK), OnOk, "%Cp %Cp", this, win);
	AG_ButtonNewFn(hbox, 0, CMSG(Cancel), OnClose, "%Cp %Cp", this, win);

	AG_SetEvent(win, "window-close", OnClose, "%Cp %Cp", this, win);

	gui->PostEtSystemPause(true);
	AG_WindowShow(win);
}

void AG_JOYSET_DLG::Close(AG_Window *win)
{
	AG_ObjectDetach(win);
	gui->PostEtSystemPause(false);
}

void AG_JOYSET_DLG::load_data(int num)
{
	int tab = selected_tab;
	AG_KEYBIND_CTRL *kc = ctrls[tab];
	kc->LoadData(num);
}

void AG_JOYSET_DLG::save_data(int num)
{
	int tab = selected_tab;
	AG_KEYBIND_CTRL *kc = ctrls[tab];
	kc->SaveData(num);
}

void AG_JOYSET_DLG::SetData()
{
#if defined(USE_JOYSTICK) || defined(USE_KEY2JOYSTICK)
	for(int i=0; i<MAX_JOYSTICKS; i++) {
		for(int k = 0; k < KEYBIND_JOY_BUTTONS; k++) {
			pConfig->joy_mashing[i][k] = mash[i][k];
		}
		for(int k = 0; k < 6; k++) {
			pConfig->joy_axis_threshold[i][k] = 10 - axis[i][k];
		}
	}
	emu->modify_joy_mashing();
	emu->modify_joy_threshold();
#endif

	for(int tab_num=0; tab_num<ctrls.Count(); tab_num++) {
		AG_KEYBIND_CTRL *kc = ctrls[tab_num];
		kc->SetData();

		kc->Final();
	}

	emu->save_keybind();

#ifdef USE_PIAJOYSTICKBIT
	BIT_ONOFF(pConfig->misc_flags, MSK_PIAJOY_NEGATIVE, iChkPiaJoyNeg != 0);
	pConfig->piajoy_conn_to = (iChkPiaJoyConn != 0 ? 1 : 0);
#else
	BIT_ONOFF(pConfig->misc_flags, MSK_PIAJOY_NOIRQ, iChkPiaJoyNoIrq != 0);
#endif
}

/*
 * Event Handler (static)
 */
void AG_JOYSET_DLG::OnOk(AG_Event *event)
{
	AG_JOYSET_DLG *dlg = (AG_JOYSET_DLG *)AG_PTR(1);
	AG_Window *win = (AG_Window *)AG_PTR(2);

	dlg->SetData();
	dlg->Close(win);
}

void AG_JOYSET_DLG::OnClose(AG_Event *event)
{
	AG_JOYSET_DLG *dlg = (AG_JOYSET_DLG *)AG_PTR(1);
	AG_Window *win = (AG_Window *)AG_PTR(2);

	dlg->Close(win);
}

void AG_JOYSET_DLG::OnClickTab(AG_Event *event)
{
	AG_Notebook *tab = (AG_Notebook *)AG_SELF();
	AG_JOYSET_DLG *dlg = (AG_JOYSET_DLG *)AG_PTR(1);

	for(int tab_num=0; tab_num<dlg->ctrls.Count(); tab_num++) {
		if (dlg->ctrls[tab_num]->tab == tab->sel_tab) {
			dlg->selected_tab = tab_num;
			break;
		}
	}
}

void AG_JOYSET_DLG::OnLoadDefault(AG_Event *event)
{
	AG_JOYSET_DLG *dlg = (AG_JOYSET_DLG *)AG_PTR(1);

	dlg->load_data(-1);
}

void AG_JOYSET_DLG::OnLoadPreset(AG_Event *event)
{
	AG_JOYSET_DLG *dlg = (AG_JOYSET_DLG *)AG_PTR(1);
	int num = AG_INT(2);

	dlg->load_data(num);
}

void AG_JOYSET_DLG::OnSavePreset(AG_Event *event)
{
	AG_JOYSET_DLG *dlg = (AG_JOYSET_DLG *)AG_PTR(1);
	int num = AG_INT(2);

	dlg->save_data(num);
}

}; /* namespace GUI_AGAR */
