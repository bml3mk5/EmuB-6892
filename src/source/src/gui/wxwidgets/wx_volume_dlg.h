/** @file wx_volume_dlg.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2014.03.23

	@brief [ wx_volume_dlg ]
*/

#ifndef WX_VOLUME_DLG_H
#define WX_VOLUME_DLG_H

#include "wx_dlg.h"
#include "../../config.h"

/**
	@brief Volume dialog box
*/
class MyVolumeDlg : public MyDialog
{
private:
	int volume[VOLUME_NUMS];
	bool mute[VOLUME_NUMS];

	int volmax;
	int volmin;

	wxSlider *sl[VOLUME_NUMS];
	wxCheckBox *ck[VOLUME_NUMS];

	void InitDialog();
	void UpdateDialog();
	void ModifyParam();
	void get_volume();
	void set_volume();

	void OnChangeVolume(wxCommandEvent &);
	void OnChangeMute(wxCommandEvent &);

public:
	MyVolumeDlg(wxWindow *, wxWindowID, EMU *, GUI_BASE *);
	~MyVolumeDlg();

	int ShowModal();

	enum {
		IDC_SLIDER_0 = 1,
		IDC_SLIDER_1,
		IDC_SLIDER_2,
		IDC_SLIDER_3,
		IDC_SLIDER_4,
		IDC_SLIDER_5,
		IDC_SLIDER_6,
		IDC_SLIDER_7,
		IDC_SLIDER_8,
		IDC_SLIDER_9,
		IDC_SLIDER_10,
		IDC_SLIDER_11,

		IDC_CHECK_0 = 21,
		IDC_CHECK_1,
		IDC_CHECK_2,
		IDC_CHECK_3,
		IDC_CHECK_4,
		IDC_CHECK_5,
		IDC_CHECK_6,
		IDC_CHECK_7,
		IDC_CHECK_8,
		IDC_CHECK_9,
		IDC_CHECK_10,
		IDC_CHECK_11,
	};

	DECLARE_EVENT_TABLE()
};

#endif /* WX_VOLUME_DLG_H */
