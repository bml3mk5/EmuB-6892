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
#include "../gui_keybinddata.h"
#include "../../labels.h"
#include "../../utility.h"

namespace GUI_AGAR
{

AG_JOYSET_DLG::AG_JOYSET_DLG(EMU *parent_emu, AG_GUI_BASE *parent_gui) : AG_DLG(parent_emu, parent_gui)
{
}

AG_JOYSET_DLG::~AG_JOYSET_DLG()
{
}
/*
 * create volume dialog
 */
void AG_JOYSET_DLG::Create()
{
	char label[64];

	AG_Window *win = AG_WindowNew(AG_WINDOW_MODAL | AG_WINDOW_DIALOG | AG_WINDOW_NORESIZE);
	AG_WindowSetCaptionS(win, CMSG(Joypad_Setting));

	AG_Box *hbox_base;
	AG_Box *vbox;
	AG_Box *hbox;
	AG_Label *lbl;
	AG_Slider *sli;

	hbox_base = AG_BoxNewHoriz(win, 0);

	for(int i=0; i<MAX_JOYSTICKS; i++) {
		vbox = AG_BoxNewVert(hbox_base, 0);

		UTILITY::sprintf_utf8(label, 64, CMSG(JoypadVDIGIT), i + 1);

		lbl = AG_LabelNewS(vbox, 0, label);

		hbox = AG_BoxNewHoriz(vbox, AG_BOX_HFILL);
		lbl = AG_LabelNewS(hbox, 0, CMSG(Button_Mashing_Speed));
		lbl = AG_LabelNewS(hbox, 0, "0 <-> 3");

		for(int k=0; k < KEYBIND_JOY_BUTTONS; k++) {
			int kk = k + VM_JOY_LABEL_BUTTON_A;
			if (kk >= VM_JOY_LABELS_MAX) break;

			hbox = AG_BoxNewHoriz(vbox, AG_BOX_HFILL);

			CMsg::Id id = (CMsg::Id)cVmJoyLabels[kk].id;
			lbl = AG_LabelNewS(hbox, 0, CMSGV(id));
			AG_LabelJustify(lbl, AG_TEXT_CENTER);
			mash[i][k] = pConfig->joy_mashing[i][k];
			sli = AG_SliderNewIntR(hbox, AG_SLIDER_HORIZ, AG_SLIDER_HFILL, &mash[i][k], 0, 3);
//			AG_SliderSetControlSize(sli, 200);
		}

		hbox = AG_BoxNewHoriz(vbox, AG_BOX_HFILL);
		lbl = AG_LabelNewS(hbox, 0, CMSG(Analog_to_Digital_Sensitivity));
		lbl = AG_LabelNewS(hbox, 0, "0 <-> 10");

		for(int k=0; k < 6; k++) {
			hbox = AG_BoxNewHoriz(vbox, AG_BOX_HFILL);

			CMsg::Id id = LABELS::joypad_axis[k];
			lbl = AG_LabelNewS(hbox, 0, CMSGV(id));
			AG_LabelJustify(lbl, AG_TEXT_CENTER);
			axis[i][k] = pConfig->joy_axis_threshold[i][k];
			sli = AG_SliderNewIntR(hbox, AG_SLIDER_HORIZ, AG_SLIDER_HFILL, &axis[i][k], 0, 10);
//			AG_SliderSetControlSize(sli, 200);
		}
	}

//	AG_WindowSetGeometryAligned(win, AG_WINDOW_MC, 640, 480);

	// button
	hbox_base = AG_BoxNewHoriz(win, 0);
	AG_ButtonNewFn(hbox_base, 0, CMSG(OK), OnOk, "%Cp %Cp", this, win);
	AG_ButtonNewFn(hbox_base, 0, CMSG(Cancel), OnClose, "%Cp %Cp", this, win);

	AG_SetEvent(win, "window-close", OnClose, "%Cp %Cp", this, win);

	gui->PostEtSystemPause(true);
	AG_WindowShow(win);
}

void AG_JOYSET_DLG::Close(AG_Window *win)
{
	AG_ObjectDetach(win);
	gui->PostEtSystemPause(false);
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

}; /* namespace GUI_AGAR */
