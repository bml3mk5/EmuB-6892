/** @file ag_recvid_dlg.h

	Skelton for retropc emulator
	SDL + Agar edition

	@author Sasaji
	@date   2015.05.21

	@brief [ ag_recvid_dlg ]
*/

#ifndef AG_RECVIDEO_DLG_H
#define AG_RECVIDEO_DLG_H

#include "ag_dlg.h"

namespace GUI_AGAR
{

#define AG_RECVIDEO_LIBS 5

/**
	@brief record video dialog
*/
class AG_RECVIDEO_DLG : public AG_DLG {
	friend class AG_GUI_BASE;

private:
	int fpsnum;
	int typnum;
	int codnums[AG_RECVIDEO_LIBS];
	int quanums[AG_RECVIDEO_LIBS];
	bool enables[AG_RECVIDEO_LIBS];
	bool cont;

	AG_Notebook *nb;
	AG_Button *btnOk;

	void ChangeType(int num);
	static void OnOk(AG_Event *);
	static void OnClose(AG_Event *);
	static void OnChangeType(AG_Event *);
	static void OnChangeCodec(AG_Event *);
	static void OnChangeQuality(AG_Event *);

public:
	AG_RECVIDEO_DLG(EMU *, AG_GUI_BASE *);
	~AG_RECVIDEO_DLG();

	void Create(int num, bool continuous = false);
	void Close(AG_Window *);
	int  SetData(AG_Window *);
};

}; /* namespace GUI_AGAR */

#endif /* AG_RECVIDEO_DLG_H */
