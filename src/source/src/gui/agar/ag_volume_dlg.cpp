/** @file ag_volume_dlg.cpp

	Skelton for retropc emulator
	SDL + Ager edition

	@author Sasaji
	@date   2012.4.5

	@brief [ ag_volume_dlg ]
*/

#include "ag_volume_dlg.h"
#include "../gui.h"
#include "../../labels.h"

namespace GUI_AGAR
{

AG_VOLUME_DLG::AG_VOLUME_DLG(EMU *parent_emu, AG_GUI_BASE *parent_gui) : AG_DLG(parent_emu, parent_gui)
{
	// volume dialog
	volmax = 100;
	volmin = 0;
}

AG_VOLUME_DLG::~AG_VOLUME_DLG()
{
}
/*
 * create volume dialog
 */
void AG_VOLUME_DLG::Create()
{
	AG_Window *win = AG_WindowNew(AG_WINDOW_MODAL | AG_WINDOW_DIALOG | AG_WINDOW_NORESIZE);
	AG_WindowSetCaptionS(win, CMSG(Volume));

	AG_Box *hbox_base;
	AG_Fixed *fix;
	AG_Fixed *fix2;
	AG_Label *lbl;
	AG_Slider *sli;
	AG_Checkbox *cbox;
	AG_Separator *sep;

	int w = 65;
	int h[4] = {32,100,16,20};
	int padd = 5;

	get_volume();


	hbox_base = AG_BoxNewHoriz(win, AG_BOX_EXPAND);

	fix = AG_FixedNew(hbox_base, AG_FIXED_EXPAND);

	int x = 0;
	int y = 0;
	int top = padd;
	int right = 0;
	for(int i=0, n=0; LABELS::volume[i] != CMsg::End; i++) {
		if (LABELS::volume[i] == CMsg::Null) {
			top = y;
			x = 0;
			continue;
		}
		y = top;
		lbl = AG_LabelNewS(fix, AG_LABEL_HFILL, CMSGV(LABELS::volume[i]));
		AG_LabelJustify(lbl, AG_TEXT_CENTER);
		AG_FixedMove(fix, lbl, x, y);
		AG_FixedSize(fix, lbl, w, h[0]);
		y += padd + h[0];
		fix2 = AG_FixedNew(fix, AG_FIXED_HFILL | AG_FIXED_FRAME);
		AG_FixedMove(fix, fix2, x, y+h[1]/2);
		AG_FixedSize(fix, fix2, w, 1);
		sli = AG_SliderNewInt(fix, AG_SLIDER_VERT, AG_SLIDER_EXPAND, &volume[n], &volmin, &volmax);
		AG_SetEvent(sli, "slider-changed", OnChange, "%Cp %i", this, n);
		AG_FixedMove(fix, sli, x+4, y);
		AG_FixedSize(fix, sli, w-8, h[1]);
		y += padd + h[1];
		voltxt[n] = AG_LabelNewS(fix, AG_LABEL_HFILL, "0000");
		AG_LabelJustify(voltxt[n], AG_TEXT_CENTER);
		SetVolumeText(n);
		AG_FixedMove(fix, voltxt[n], x, y);
		AG_FixedSize(fix, voltxt[n], w, h[2]);
		y += padd + h[2];
		cbox = AG_CheckboxNewInt(fix, 0, CMSG(Mute), &mute[n]);
		AG_SetEvent(cbox, "checkbox-changed", OnChange, "%Cp %i", this, n);
		AG_FixedMove(fix, cbox, x, y);
		AG_FixedSize(fix, cbox, w, h[3]);
		y += padd + h[3];

		x += (padd + w);

		if (right < x) {
			right = x;
		}

		if (i==0) {
			// sep
			sep = AG_SeparatorNew(fix, AG_SEPARATOR_VERT);
			AG_FixedMove(fix, sep, x, padd);
			AG_FixedSize(fix, sep, 12, y - padd);
			x += padd + 12;
		}

		n++;
	}
	x = right;
	x += padd;
	y += padd;

	AG_WindowSetGeometryAligned(win, AG_WINDOW_MC, x, y + 60);

	AG_ButtonNewFn(win, AG_BUTTON_HFILL, CMSG(Close), OnClose, "%Cp %Cp", this, win);

	AG_SetEvent(win, "window-close", OnClose, "%Cp %Cp", this, win);
	AG_AddEvent(win, "key-up", OnCancel, "%Cp", win);

	AG_WindowShow(win);
}

void AG_VOLUME_DLG::Change()
{
	set_volume();
	emu->set_volume(0);
}

void AG_VOLUME_DLG::Close(AG_Window *win)
{
	Change();
	AG_ObjectDetach(win);
}

void AG_VOLUME_DLG::get_volume()
{
	int i = 0;
	volume[i++] = volmax - pConfig->volume;
	volume[i++] = volmax - pConfig->beep_volume;
#if defined(_MBS1)
	volume[i++] = volmax - pConfig->psg_volume;
	volume[i++] = volmax - pConfig->psgexfm_volume;
	volume[i++] = volmax - pConfig->psgexssg_volume;
	volume[i++] = volmax - pConfig->psgexpcm_volume;
	volume[i++] = volmax - pConfig->psgexrhy_volume;
	volume[i++] = volmax - pConfig->opnfm_volume;
	volume[i++] = volmax - pConfig->opnssg_volume;
	volume[i++] = volmax - pConfig->opnpcm_volume;
	volume[i++] = volmax - pConfig->opnrhy_volume;
#endif
	volume[i++] = volmax - pConfig->psg6_volume;
	volume[i++] = volmax - pConfig->psg9_volume;
	volume[i++] = volmax - pConfig->relay_volume;
	volume[i++] = volmax - pConfig->cmt_volume;
	volume[i++] = volmax - pConfig->fdd_volume;

	i = 0;
	mute[i++] = pConfig->mute ? 1 : 0;
	mute[i++] = pConfig->beep_mute ? 1 : 0;
#if defined(_MBS1)
	mute[i++] = pConfig->psg_mute ? 1 : 0;
	mute[i++] = pConfig->psgexfm_mute ? 1 : 0;
	mute[i++] = pConfig->psgexssg_mute ? 1 : 0;
	mute[i++] = pConfig->psgexpcm_mute ? 1 : 0;
	mute[i++] = pConfig->psgexrhy_mute ? 1 : 0;
	mute[i++] = pConfig->opnfm_mute ? 1 : 0;
	mute[i++] = pConfig->opnssg_mute ? 1 : 0;
	mute[i++] = pConfig->opnpcm_mute ? 1 : 0;
	mute[i++] = pConfig->opnrhy_mute ? 1 : 0;
#endif
	mute[i++] = pConfig->psg6_mute ? 1 : 0;
	mute[i++] = pConfig->psg9_mute ? 1 : 0;
	mute[i++] = pConfig->relay_mute ? 1 : 0;
	mute[i++] = pConfig->cmt_mute ? 1 : 0;
	mute[i++] = pConfig->fdd_mute ? 1 : 0;
}

void AG_VOLUME_DLG::set_volume()
{
	int i = 0;
	pConfig->volume = volmax - volume[i++];
	pConfig->beep_volume = volmax - volume[i++];
#if defined(_MBS1)
	pConfig->psg_volume = volmax - volume[i++];
	pConfig->psgexfm_volume = volmax - volume[i++];
	pConfig->psgexssg_volume = volmax - volume[i++];
	pConfig->psgexpcm_volume = volmax - volume[i++];
	pConfig->psgexrhy_volume = volmax - volume[i++];
	pConfig->opnfm_volume = volmax - volume[i++];
	pConfig->opnssg_volume = volmax - volume[i++];
	pConfig->opnpcm_volume = volmax - volume[i++];
	pConfig->opnrhy_volume = volmax - volume[i++];
#endif
	pConfig->psg6_volume = volmax - volume[i++];
	pConfig->psg9_volume = volmax - volume[i++];
	pConfig->relay_volume = volmax - volume[i++];
	pConfig->cmt_volume = volmax - volume[i++];
	pConfig->fdd_volume = volmax - volume[i++];

	i = 0;
	pConfig->mute = (mute[i++] != 0);
	pConfig->beep_mute = (mute[i++] != 0);
#if defined(_MBS1)
	pConfig->psg_mute = (mute[i++] != 0);
	pConfig->psgexfm_mute = (mute[i++] != 0);
	pConfig->psgexssg_mute = (mute[i++] != 0);
	pConfig->psgexpcm_mute = (mute[i++] != 0);
	pConfig->psgexrhy_mute = (mute[i++] != 0);
	pConfig->opnfm_mute = (mute[i++] != 0);
	pConfig->opnssg_mute = (mute[i++] != 0);
	pConfig->opnpcm_mute = (mute[i++] != 0);
	pConfig->opnrhy_mute = (mute[i++] != 0);
#endif
	pConfig->psg6_mute = (mute[i++] != 0);
	pConfig->psg9_mute = (mute[i++] != 0);
	pConfig->relay_mute = (mute[i++] != 0);
	pConfig->cmt_mute = (mute[i++] != 0);
	pConfig->fdd_mute = (mute[i++] != 0);
}

void AG_VOLUME_DLG::SetVolumeText(int idx)
{
	sprintf(voltxt[idx]->text, "%02d", volmax - volume[idx]);
	voltxt[idx]->flags |= AG_LABEL_REGEN;
	AG_Redraw(voltxt[idx]);
}

/*
 * Event Handler (static)
 */
void AG_VOLUME_DLG::OnClose(AG_Event *event)
{
	AG_VOLUME_DLG *dlg = (AG_VOLUME_DLG *)AG_PTR(1);
	AG_Window *win = (AG_Window *)AG_PTR(2);

	dlg->Close(win);
}

// close dialog
void AG_VOLUME_DLG::OnCancel(AG_Event *event)
{
	AG_Window *win = (AG_Window *)AG_PTR(1);

	if (strcmp(event->name, "key-up") == 0) {
		int key = AG_INT(2);
		if (key != AG_KEY_ESCAPE) {
			return;
		}
	}

	AG_ObjectDetach(win);
}

// slider or checkbox changed
void AG_VOLUME_DLG::OnChange(AG_Event *event)
{
	AG_VOLUME_DLG *dlg = (AG_VOLUME_DLG *)AG_PTR(1);
	int idx = AG_INT(2);

	dlg->Change();
	dlg->SetVolumeText(idx);
}

}; /* namespace GUI_AGAR */
