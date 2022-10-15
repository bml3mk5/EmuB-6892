/** @file wx_volume_dlg.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2014.03.23

	@brief [ wx_volume_dlg ]
*/

#include <wx/wx.h>
#include <wx/statline.h>
#include "wx_volume_dlg.h"
#include "../../emu.h"
#include "../../labels.h"

// Attach Event
BEGIN_EVENT_TABLE(MyVolumeDlg, wxDialog)
	EVT_COMMAND_RANGE(IDC_SLIDER_0, IDC_SLIDER_6, wxEVT_SCROLL_THUMBTRACK , MyVolumeDlg::OnChangeVolume)
	EVT_COMMAND_RANGE(IDC_CHECK_0, IDC_CHECK_6, wxEVT_CHECKBOX, MyVolumeDlg::OnChangeMute)
END_EVENT_TABLE()

MyVolumeDlg::MyVolumeDlg(wxWindow* parent, wxWindowID id, EMU *parent_emu, GUI_BASE *parent_gui)
	: MyDialog(parent, id, CMSG(Volume), parent_emu, parent_gui)
{
	volmax = 100;
	volmin = 0;
}

MyVolumeDlg::~MyVolumeDlg()
{
}

/**
 * create config dialog when ShowModal called
 */
void MyVolumeDlg::InitDialog()
{
	wxSizerFlags flags = wxSizerFlags().Expand().Border(wxALL, 4);

	wxBoxSizer *szrAll    = new wxBoxSizer(wxVERTICAL);

	int h[3] = {32,120,20};

	get_volume();

	wxBoxSizer *szrMain = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *bszr;
	for(int i=0, n=0; ; n++) {
		if (LABELS::volume[n] == CMsg::End) {
			break;
		}
		if (LABELS::volume[n] == CMsg::Null) {
			szrAll->Add(szrMain, flags);
			szrAll->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL), flags);
			szrMain = new wxBoxSizer(wxHORIZONTAL);
			continue;
		}
		wxSize size(-1, -1);
		bszr = new wxBoxSizer(wxVERTICAL);
		size.y = h[0];
		MyStaticText *st = new MyStaticText(this, wxID_ANY, LABELS::volume[n], wxDefaultPosition, size, wxALIGN_CENTRE);
		bszr->Add(st, flags);
		size.y = h[1];
		sl[i] = new wxSlider(this, IDC_SLIDER_0 + i, volume[i], volmin, volmax, wxDefaultPosition, size, wxSL_VERTICAL | wxSL_RIGHT | wxSL_AUTOTICKS);
		sl[i]->SetTickFreq(25);
		bszr->Add(sl[i], flags);
		size.y = h[2];
		ck[i] = new MyCheckBox(this, IDC_CHECK_0 + i, CMsg::Mute, wxDefaultPosition, size);
		bszr->Add(ck[i], flags);
		szrMain->Add(bszr, flags);
		if (i==0) {
			szrMain->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL), flags);
		}
		i++;
	}
	szrAll->Add(szrMain, flags);

	// button
	wxSizer *szrButtons = CreateButtonSizer(wxCLOSE);
	SetAffirmativeId(wxID_CLOSE);
	szrAll->Add(szrButtons, flags);

	UpdateDialog();

	SetSizerAndFit(szrAll);
//	SetSizer(szrAll);
}

void MyVolumeDlg::UpdateDialog()
{
}

int MyVolumeDlg::ShowModal()
{
//	InitDialog();
	int rc = MyDialog::ShowModal();
	if (rc == wxID_OK) {
		ModifyParam();
	}
	return rc;
}

void MyVolumeDlg::ModifyParam()
{
}

void MyVolumeDlg::get_volume()
{
	int i=0;
	volume[i++] = volmax - config.volume;
	volume[i++] = volmax - config.beep_volume;
#if defined(_MBS1)
	volume[i++] = volmax - config.psg_volume;
	volume[i++] = volmax - config.psgexfm_volume;
	volume[i++] = volmax - config.psgexssg_volume;
	volume[i++] = volmax - config.psgexpcm_volume;
	volume[i++] = volmax - config.psgexrhy_volume;
	volume[i++] = volmax - config.opnfm_volume;
	volume[i++] = volmax - config.opnssg_volume;
	volume[i++] = volmax - config.opnpcm_volume;
	volume[i++] = volmax - config.opnrhy_volume;
#endif
	volume[i++] = volmax - config.psg6_volume;
	volume[i++] = volmax - config.psg9_volume;
	volume[i++] = volmax - config.relay_volume;
	volume[i++] = volmax - config.cmt_volume;
	volume[i++] = volmax - config.fdd_volume;

	i = 0;
	mute[i++] = config.mute;
	mute[i++] = config.beep_mute;
#if defined(_MBS1)
	mute[i++] = config.psg_mute;
	mute[i++] = config.psgexfm_mute;
	mute[i++] = config.psgexssg_mute;
	mute[i++] = config.psgexpcm_mute;
	mute[i++] = config.psgexrhy_mute;
	mute[i++] = config.opnfm_mute;
	mute[i++] = config.opnssg_mute;
	mute[i++] = config.opnpcm_mute;
	mute[i++] = config.opnrhy_mute;
#endif
	mute[i++] = config.psg6_mute;
	mute[i++] = config.psg9_mute;
	mute[i++] = config.relay_mute;
	mute[i++] = config.cmt_mute;
	mute[i++] = config.fdd_mute;
}

void MyVolumeDlg::set_volume()
{
	int i = 0;
	config.volume = volmax - volume[i++];
	config.beep_volume = volmax - volume[i++];
#if defined(_MBS1)
	config.psg_volume = volmax - volume[i++];
	config.psgexfm_volume = volmax - volume[i++];
	config.psgexssg_volume = volmax - volume[i++];
	config.psgexpcm_volume = volmax - volume[i++];
	config.psgexrhy_volume = volmax - volume[i++];
	config.opnfm_volume = volmax - volume[i++];
	config.opnssg_volume = volmax - volume[i++];
	config.opnpcm_volume = volmax - volume[i++];
	config.opnrhy_volume = volmax - volume[i++];
#endif
	config.psg6_volume = volmax - volume[i++];
	config.psg9_volume = volmax - volume[i++];
	config.relay_volume = volmax - volume[i++];
	config.cmt_volume = volmax - volume[i++];
	config.fdd_volume = volmax - volume[i++];

	i = 0;
	config.mute = mute[i++];
	config.beep_mute = mute[i++];
#if defined(_MBS1)
	config.psg_mute = mute[i++];
	config.psgexfm_mute = mute[i++];
	config.psgexssg_mute = mute[i++];
	config.psgexpcm_mute = mute[i++];
	config.psgexrhy_mute = mute[i++];
	config.opnfm_mute = mute[i++];
	config.opnssg_mute = mute[i++];
	config.opnpcm_mute = mute[i++];
	config.opnrhy_mute = mute[i++];
#endif
	config.psg6_mute = mute[i++];
	config.psg9_mute = mute[i++];
	config.relay_mute = mute[i++];
	config.cmt_mute = mute[i++];
	config.fdd_mute = mute[i++];

	emu->set_volume(0);
}

/*
 * Event Handler
 */

void MyVolumeDlg::OnChangeVolume(wxCommandEvent &event)
{
	int i = event.GetId() - IDC_SLIDER_0;
	volume[i] = sl[i]->GetValue();
	set_volume();
}

void MyVolumeDlg::OnChangeMute(wxCommandEvent &event)
{
	int i = event.GetId() - IDC_CHECK_0;
	mute[i] = ck[i]->GetValue();
	set_volume();
}
