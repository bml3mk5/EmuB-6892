/** @file ag_dlg.cpp

	Skelton for retropc emulator
	SDL + Agar edition

	@author Sasaji
	@date   2012.04.08

	@brief [ ag_dlg ]
*/

#include "ag_dlg.h"
#include "../../utility.h"

namespace GUI_AGAR
{

AG_Label *AG_DLG::LabelNew(void *parent, const char *label)
{
	AG_Label *lbl = AG_LabelNewS(parent, AG_LABEL_HFILL, label);
	return lbl;
}

AG_Textbox *AG_DLG::TextboxNew(void *parent, const char *label, int box_len, char *buf, int buf_len)
{
	int w,h;
	AG_Textbox *txt = AG_TextboxNewS(parent, 0, label);
	AG_TextSize("w", &w, &h);
	AG_TextboxSizeHintPixels(txt, w * box_len, h);
	AG_TextboxBindUTF8(txt, buf, buf_len);
	return txt;
}

AG_UCombo *AG_DLG::UComboNew(void *parent, const char **list, int selnum, AG_EventFn cb, int index)
{
	int w,h,maxw,i,maxi;
	maxw = maxi = 0;

	AG_UCombo *ucom = AG_UComboNew(parent, AG_UCOMBO_HFILL);
	for(i=0; list[i] != NULL; i++) {
		AG_TextSize(list[i], &w, &h);
		w += 4;
		h += 2;
		AG_TlistAdd(ucom->list, NULL, list[i]);
		if (maxw < w) {
			maxw = w;
			maxi = i;
		}
	}
	AG_UComboSizeHintPixels(ucom, maxw, i);
	if (selnum >= 0 && selnum < i) {
		AG_TlistSelectText(ucom->list, list[selnum]);
	}
	if (index >= 0) {
		AG_TlistSetChangedFn(ucom->list, cb, "%Cp %i", this, index);
	} else {
		AG_TlistSetChangedFn(ucom->list, cb, "%Cp", this);
	}
	return ucom;
}


AG_UCombo *AG_DLG::UComboNew(void *parent, const CMsg::Id *list, int selnum, AG_EventFn cb, int index, int appendnum, CMsg::Id appendstr)
{
	int w,h,maxw,i,maxi;
	maxw = maxi = 0;

	AG_UCombo *ucom = AG_UComboNew(parent, AG_UCOMBO_HFILL);
	for(i=0; list[i] != 0 && list[i] != CMsg::End; i++) {
		if (i == appendnum) {
			char p[_MAX_PATH];
			UTILITY::tcscpy(p, _MAX_PATH, CMSGV(list[i]));
			UTILITY::tcscat(p, _MAX_PATH, CMSGV(appendstr));
			AG_TextSize(p, &w, &h);
			w += 4;
			h += 2;
			AG_TlistAdd(ucom->list, NULL, p);
		} else {
			const char *p = CMSGV(list[i]);
			AG_TextSize(p, &w, &h);
			w += 4;
			h += 2;
			AG_TlistAdd(ucom->list, NULL, p);
		}
		if (maxw < w) {
			maxw = w;
			maxi = i;
		}
	}
	AG_UComboSizeHintPixels(ucom, maxw, i);
	if (selnum >= 0 && selnum < i) {
		AG_TlistSelectText(ucom->list, CMSGV(list[selnum]));
	}
	if (index >= 0) {
		AG_TlistSetChangedFn(ucom->list, cb, "%Cp %i", this, index);
	} else {
		AG_TlistSetChangedFn(ucom->list, cb, "%Cp", this);
	}
	return ucom;
}

AG_UCombo *AG_DLG::UComboNew(void *parent, const CPtrList<CTchar> &list, int selnum, AG_EventFn cb, int index)
{
	int w,h,maxw,i,maxi;
	maxw = maxi = 0;

	AG_UCombo *ucom = AG_UComboNew(parent, AG_UCOMBO_HFILL);
	for(i=0; i<list.Count(); i++) {
		const char *p = list.Item(i)->GetN();
		AG_TextSize(p, &w, &h);
		w += 4;
		h += 2;
		AG_TlistAdd(ucom->list, NULL, p);
		if (maxw < w) {
			maxw = w;
			maxi = i;
		}
	}
	AG_UComboSizeHintPixels(ucom, maxw, i);
	if (selnum >= 0 && selnum < i) {
		AG_TlistSelectText(ucom->list, list.Item(selnum)->GetN());
	}
	if (index >= 0) {
		AG_TlistSetChangedFn(ucom->list, cb, "%Cp %i", this, index);
	} else {
		AG_TlistSetChangedFn(ucom->list, cb, "%Cp", this);
	}
	return ucom;
}

AG_Radio *AG_DLG::RadioNewInt(void *parent, unsigned int flags, const CMsg::Id *list, int *ret)
{
	int i;
	AG_Radio *rad = AG_RadioNewInt(parent, flags, NULL, ret);
	for(i=0; list[i] != 0 && list[i] != CMsg::End; i++) {
		const char *p = CMSGV(list[i]);
		AG_RadioAddItemS(rad, p);
	}
	return rad;
}

}; /* namespace GUI_AGAR */
