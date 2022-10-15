/** @file wx_recaud_dlg.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.09.02

	@brief [ wx_recaud_dlg ]
*/

#include <wx/wx.h>
#include "wx_recaud_dlg.h"
#include "../../common.h"
#include "../../emu.h"
#include "../gui.h"
#include "../../video/rec_audio.h"
#include "../../clocale.h"

// list
static const _TCHAR *audio_type_label[] = {
#ifdef USE_REC_AUDIO
#ifdef USE_REC_AUDIO_WAVE
	_T("wave"),
#endif
#ifdef USE_REC_AUDIO_MMF
	_T("media foundation"),
#endif
#ifdef USE_REC_AUDIO_FFMPEG
	_T("ffmpeg"),
#endif
#endif
	NULL };
static const int audio_type_ids[] = {
#ifdef USE_REC_AUDIO
#ifdef USE_REC_AUDIO_WAVE
	RECORD_AUDIO_TYPE_WAVE,
#endif
#ifdef USE_REC_AUDIO_MMF
	RECORD_AUDIO_TYPE_MMF,
#endif
#ifdef USE_REC_AUDIO_FFMPEG
	RECORD_AUDIO_TYPE_FFMPEG,
#endif
#endif
	0 };

// Attach Event
BEGIN_EVENT_TABLE(MyRecAudioDlg, wxDialog)
	EVT_NOTEBOOK_PAGE_CHANGED(IDC_NOTEBOOK, MyRecAudioDlg::OnChangeBook)
	EVT_COMMAND_RANGE(IDC_CODEC, IDC_CODEC + 1, wxEVT_COMBOBOX, MyRecAudioDlg::OnChangeCodec)
	EVT_COMMAND_RANGE(IDC_QUALITY, IDC_QUALITY+ 1, wxEVT_COMBOBOX, MyRecAudioDlg::OnChangeQuality)
END_EVENT_TABLE()

MyRecAudioDlg::MyRecAudioDlg(wxWindow* parent, wxWindowID id, EMU *parent_emu, GUI_BASE *parent_gui)
	: MyDialog(parent, id, CMSG(Record_Sound), parent_emu, parent_gui)
{
}

MyRecAudioDlg::~MyRecAudioDlg()
{
}

/**
 * create config dialog when ShowModal called
 */
void MyRecAudioDlg::InitDialog()
{
	wxSizerFlags flags = wxSizerFlags().Expand().Border(wxALL, 4);

	wxBoxSizer *szrMain;
	wxBoxSizer *szrSub;
	wxBoxSizer *szrAll = new wxBoxSizer(wxVERTICAL);

	book = new wxNotebook(this, IDC_NOTEBOOK, wxDefaultPosition, wxDefaultSize);
	wxPanel *page;

	//
	int i;
	for(i=0; audio_type_ids[i] != 0; i++) {
		page = new wxPanel(book);
		book->AddPage(page, _(audio_type_label[i]));
		codnums.Add(0);
		quanums.Add(0);

		szrMain = new wxBoxSizer(wxVERTICAL);
		enables.Add(emu->rec_video_enabled(audio_type_ids[i]) ? 1 : 0);

		switch(audio_type_ids[i]) {
			case RECORD_AUDIO_TYPE_WAVE:
			{
				szrSub = new wxBoxSizer(wxHORIZONTAL);
				szrSub->Add(new MyStaticText(page, wxID_ANY, CMsg::Select_a_sample_rate_on_sound_menu_in_advance), flags);
				szrMain->Add(szrSub, flags);
				break;
			}
			default:
			{
				int count = 0;
				// Codec
				szrSub = new wxBoxSizer(wxHORIZONTAL);
				szrSub->Add(new MyStaticText(page, wxID_ANY, CMsg::Codec_Type), flags);
				const _TCHAR **codlbl = emu->get_rec_video_codec_list(audio_type_ids[i]);
				wxArrayString codlbls;
				for(count = 0; codlbl[count] != NULL; count++) {
					codlbls.Add(wxString(codlbl[count]));
				}
				wxComboBox *comCodec = new wxComboBox(page, IDC_CODEC + i, wxT(""), wxDefaultPosition, wxDefaultSize, codlbls, wxCB_DROPDOWN | wxCB_READONLY);
				comCodec->SetSelection(0);
				szrSub->Add(comCodec, flags);
				szrMain->Add(szrSub, flags);

#if 0
				// Quality
				szrSub = new wxBoxSizer(wxHORIZONTAL);
				szrSub->Add(new MyStaticText(page, wxID_ANY, CMsg::Quality), flags);
				const CMsg::Id *qualbl = emu->get_rec_video_quality_list(audio_type_ids[i]);
				wxArrayString qualbls;
				for(count = 0; qualbl[count] != 0 && qualbl[count] != CMsg::End; count++) {
					qualbls.Add(gMessages.Get(qualbl[count]));
				}
				wxComboBox *comQuality = new wxComboBox(page, IDC_QUALITY + i, wxT(""), wxDefaultPosition, wxDefaultSize, qualbls, wxCB_DROPDOWN | wxCB_READONLY);
				comQuality->SetSelection(0);
				szrSub->Add(comQuality, flags);
				szrMain->Add(szrSub, flags);
#endif

				break;
			}
		}
		if (!enables[i]) {
			szrMain->Add(new MyStaticText(page, wxID_ANY, CMsg::Need_install_library), flags);
		}

		page->SetSizer(szrMain);
	}

	szrAll->Add(book, flags);

	// button
	wxSizer *szrButtons = CreateButtonSizer(wxOK|wxCANCEL);
	szrAll->Add(szrButtons, flags);

	UpdateDialog();

	SetSizerAndFit(szrAll);
//	SetSizer(szrAll);
}

void MyRecAudioDlg::UpdateDialog()
{
	wxButton *btnOk = (wxButton *)FindWindow(wxID_OK);
	if (btnOk) {
		btnOk->Enable(enables[0] != 0);
	}
}

int MyRecAudioDlg::ShowModal()
{
//	InitDialog();
	int rc = MyDialog::ShowModal();
	if (rc == wxID_OK) {
		ModifyParam();
	}
	return rc;
}

void MyRecAudioDlg::ModifyParam()
{
	int selnum = book->GetSelection();

	emu->set_parami(VM::ParamRecAudioType, audio_type_ids[selnum]);
	emu->set_parami(VM::ParamRecAudioCodec, codnums[selnum]);
//	emu->set_parami(VM::ParamRecAudioQuality, quanums[selnum]);
}

/*
 * Event Handler
 */
void MyRecAudioDlg::OnChangeBook(wxBookCtrlEvent &event)
{
	int selnum = event.GetSelection();
	wxButton *btnOk = (wxButton *)FindWindow(wxID_OK);
	if (btnOk) {
		btnOk->Enable(enables[selnum] != 0);
	}
}

void MyRecAudioDlg::OnChangeCodec(wxCommandEvent &event)
{
	int selnum = event.GetId() - IDC_CODEC;
	wxComboBox *com = (wxComboBox *)FindWindow(event.GetId());
	if (com && codnums.Count() > (size_t)selnum) {
		int num = com->GetSelection();
		codnums[selnum] = num;
	}
}

void MyRecAudioDlg::OnChangeQuality(wxCommandEvent &event)
{
	int selnum = event.GetId() - IDC_QUALITY;
	wxComboBox *com = (wxComboBox *)FindWindow(event.GetId());
	if (com && quanums.Count() > (size_t)selnum) {
		int num = com->GetSelection();
		quanums[selnum] = num;
	}
}

