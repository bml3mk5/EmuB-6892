/** @file ag_recaud_dlg.h

	Skelton for retropc emulator
	SDL + Agar edition

	@author Sasaji
	@date   2015.05.21

	@brief [ ag_recaud_dlg ]
*/

#ifndef AG_RECAUDIO_DLG_H
#define AG_RECAUDIO_DLG_H

#include "ag_dlg.h"

namespace GUI_AGAR
{

#define AG_RECAUDIO_LIBS 5

/**
	@brief record video dialog
*/
class AG_RECAUDIO_DLG : public AG_DLG {
	friend class AG_GUI_BASE;

private:
	int typnum;
	int codnums[AG_RECAUDIO_LIBS];
	int quanums[AG_RECAUDIO_LIBS];
	bool enables[AG_RECAUDIO_LIBS];

	AG_Notebook *nb;
	AG_Button *btnOk;

	void ChangeType(int num);
	static void OnOk(AG_Event *);
	static void OnClose(AG_Event *);
	static void OnChangeType(AG_Event *);
	static void OnChangeCodec(AG_Event *);
	static void OnChangeQuality(AG_Event *);

public:
	AG_RECAUDIO_DLG(EMU *, AG_GUI_BASE *);
	~AG_RECAUDIO_DLG();

	void Create();
	void Close(AG_Window *);
	int  SetData(AG_Window *);
};

}; /* namespace GUI_AGAR */

#endif /* AG_RECAUDIO_DLG_H */
