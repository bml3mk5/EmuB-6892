/** @file ag_recaud_dlg.cpp

	Skelton for retropc emulator
	SDL + Ager edition

	@author Sasaji
	@date   2015.05.21

	@brief [ ag_recaud_dlg ]
*/

#include "ag_recaud_dlg.h"
#include "../../video/rec_audio.h"
#include <agar/core/types.h>
#include "../gui.h"
#include "ag_gui_base.h"
#include "../../clocale.h"

namespace GUI_AGAR
{

// list
static const char *type_label[] = {
#ifdef USE_REC_AUDIO_AVKIT
	_T("avkit"),
#endif
#ifdef USE_REC_AUDIO_WAVE
	_T("wave"),
#endif
#ifdef USE_REC_AUDIO_FFMPEG
	_T("ffmpeg"),
#endif
	NULL };
static const int type_ids[] = {
#ifdef USE_REC_AUDIO_AVKIT
	RECORD_AUDIO_TYPE_AVKIT,
#endif
#ifdef USE_REC_AUDIO_WAVE
	RECORD_AUDIO_TYPE_WAVE,
#endif
#ifdef USE_REC_AUDIO_FFMPEG
	RECORD_AUDIO_TYPE_FFMPEG,
#endif
	0 };

AG_RECAUDIO_DLG::AG_RECAUDIO_DLG(EMU *parent_emu, AG_GUI_BASE *parent_gui) : AG_DLG(parent_emu, parent_gui)
{
	typnum = 0;
	memset(codnums, 0, sizeof(codnums));
	memset(quanums, 0, sizeof(quanums));
	memset(enables, 0, sizeof(enables));
	nb = NULL;
	btnOk = NULL;
}

AG_RECAUDIO_DLG::~AG_RECAUDIO_DLG()
{
}

/**
 * create recaud dialog
 */
void AG_RECAUDIO_DLG::Create()
{
	int codnum = emu->get_parami(VM::ParamRecAudioCodec);
//	int quanum = emu->get_parami(VM::ParamRecAudioQuality);
	int i;
	typnum = 0;
	for(i=0; type_ids[i] != 0; i++) {
		if (type_ids[i] == emu->get_parami(VM::ParamRecAudioType)) {
			typnum = i;
			codnums[i] = codnum;
//			quanums[i] = quanum;
			break;
		}
	}

	AG_Window *win = AG_WindowNew(AG_WINDOW_MODAL | AG_WINDOW_DIALOG | AG_WINDOW_NORESIZE);
	AG_WindowSetCaptionS(win, CMSG(Record_Sound));

	AG_Box *vbox;
	AG_Box *hbox;

	AG_NotebookTab *tabs[AG_RECAUDIO_LIBS];
	memset(tabs, 0, sizeof(tabs));

	nb = AG_NotebookNew(win, AG_NOTEBOOK_HFILL);
	AG_AddEvent(nb, "mouse-button-down", OnChangeType, "%Cp", this);

	//
	for(i=0; type_ids[i] != 0; i++) {
		tabs[i] = AG_NotebookAddTab(nb, _tgettext(type_label[i]), AG_BOX_HORIZ);
		enables[i] = emu->rec_sound_enabled(type_ids[i]);

		vbox = AG_BoxNewVert(tabs[i], AG_BOX_EXPAND);

		switch(type_ids[i]) {
			case RECORD_AUDIO_TYPE_WAVE:
			{
				AG_LabelNewS(vbox, AG_LABEL_EXPAND, CMSG(Select_a_sample_rate_on_sound_menu_in_advance));
				break;
			}
			default:
			{
				hbox = AG_BoxNewHoriz(vbox, AG_BOX_HFILL);

				AG_LabelNewS(hbox, 0, CMSG(Codec_Type));
				const char **codlbl = emu->get_rec_sound_codec_list(type_ids[i]);
				UComboNew(hbox, codlbl, codnums[i], OnChangeCodec, i);

//				hbox = AG_BoxNewHoriz(vbox, AG_BOX_HFILL);
//				AG_LabelNewS(hbox, 0, CMSG(Quality));
//				const char **qualbl = emu->get_rec_sound_quality_list(type_ids[i]);
//				UComboNew(hbox, qualbl, quanums[i], OnChangeQuality, i);
				break;
			}
		}

		if (!enables[i]) {
			AG_LabelNewS(vbox, 0, CMSG(Need_install_library));
		}
	}

	// button
	hbox = AG_BoxNewHoriz(win, AG_BOX_HFILL);
	btnOk = AG_ButtonNewFn(hbox, 0, CMSG(Start), OnOk, "%Cp %Cp", this, win);
	AG_ButtonNewFn(hbox, 0, CMSG(Cancel), OnClose, "%Cp %Cp", this, win);

	AG_SetEvent(win, "window-close", OnClose, "%Cp %Cp", this, win);

	ChangeType(0);

	gui->PostEtSystemPause(true);
	AG_WindowShow(win);

	AG_SizeReq req;
	AG_WidgetSizeReq(win, &req);
	if (req.w < 240) req.w = 240;
	AG_WindowSetGeometryAligned(win, AG_WINDOW_MC, req.w, req.h);
}

void AG_RECAUDIO_DLG::Close(AG_Window *win)
{
	AG_ObjectDetach(win);
	gui->PostEtSystemPause(false);
}

int AG_RECAUDIO_DLG::SetData(AG_Window *win)
{
	// get current displayed tab on notebook
	int selnum = 0;
	AG_NotebookTab *tab;
	AG_TAILQ_FOREACH(tab, &nb->tabs, tabs) {
		if (tab == nb->sel_tab) {
			break;
		}
		selnum++;
	}
	emu->set_parami(VM::ParamRecAudioType, type_ids[selnum]);
	emu->set_parami(VM::ParamRecAudioCodec, codnums[selnum]);
//	emu->set_parami(VM::ParamRecAudioQuality, quanums[selnum]);

	gui->PostEtStartRecordSound();

	return 1;
}

void AG_RECAUDIO_DLG::ChangeType(int num)
{
	if (enables[num]) {
		AG_WidgetEnable(btnOk);
	} else {
		AG_WidgetDisable(btnOk);
	}
}

/*
 * Event Handler (static)
 */
void AG_RECAUDIO_DLG::OnOk(AG_Event *event)
{
	AG_RECAUDIO_DLG *dlg = (AG_RECAUDIO_DLG *)AG_PTR(1);
	AG_Window *win = (AG_Window *)AG_PTR(2);

	if (dlg->SetData(win)) {
		dlg->Close(win);
	}
}

void AG_RECAUDIO_DLG::OnClose(AG_Event *event)
{
	AG_RECAUDIO_DLG *dlg = (AG_RECAUDIO_DLG *)AG_PTR(1);
	AG_Window *win = (AG_Window *)AG_PTR(2);

	dlg->Close(win);
}

void AG_RECAUDIO_DLG::OnChangeType(AG_Event *event)
{
	AG_Notebook *nb = (AG_Notebook *)AG_SELF();
	AG_RECAUDIO_DLG *dlg = (AG_RECAUDIO_DLG *)AG_PTR(1);
	int selnum = 0;
	AG_NotebookTab *tab;
	AG_TAILQ_FOREACH(tab, &nb->tabs, tabs) {
		if (tab == nb->sel_tab) {
			break;
		}
		selnum++;
	}
	dlg->ChangeType(selnum);
}

void AG_RECAUDIO_DLG::OnChangeCodec(AG_Event *event)
{
	AG_RECAUDIO_DLG *dlg = (AG_RECAUDIO_DLG *)AG_PTR(1);
	int typenum = AG_INT(2);
	AG_TlistItem *item = (AG_TlistItem *)AG_PTR(3);
	int state = AG_INT(4);
	if (state) {
		dlg->codnums[typenum] = item->label;
	}
}

void AG_RECAUDIO_DLG::OnChangeQuality(AG_Event *event)
{
	AG_RECAUDIO_DLG *dlg = (AG_RECAUDIO_DLG *)AG_PTR(1);
	int typenum = AG_INT(2);
	AG_TlistItem *item = (AG_TlistItem *)AG_PTR(3);
	int state = AG_INT(4);
	if (state) {
		dlg->quanums[typenum] = item->label;
	}
}

}; /* namespace GUI_AGAR */
