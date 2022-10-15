/** @file wx_recvid_dlg.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.09.02

	@brief [ wx_recvid_dlg ]
*/

#include <wx/wx.h>
#include "wx_recvid_dlg.h"
#include "../../common.h"
#include "../../emu.h"
#include "../gui.h"
#include "../../video/rec_video.h"
#include "../../clocale.h"

// list
static const _TCHAR *video_type_label[] = {
#ifdef USE_REC_VIDEO_VFW
	_TX("video for windows"),
#endif
#ifdef USE_REC_VIDEO_QTKIT
	_TX("qtkit"),
#endif
#ifdef USE_REC_VIDEO_FFMPEG
	_TX("ffmpeg"),
#endif
	NULL };
static const int video_type_ids[] = {
#ifdef USE_REC_VIDEO_VFW
	RECORD_VIDEO_TYPE_VFW,
#endif
#ifdef USE_REC_VIDEO_QTKIT
	RECORD_VIDEO_TYPE_QTKIT,
#endif
#ifdef USE_REC_VIDEO_FFMPEG
	RECORD_VIDEO_TYPE_FFMPEG,
#endif
	0 };

// Attach Event
BEGIN_EVENT_TABLE(MyRecVideoDlg, wxDialog)
	EVT_NOTEBOOK_PAGE_CHANGED(IDC_NOTEBOOK, MyRecVideoDlg::OnChangeBook)
	EVT_COMMAND_RANGE(IDC_CODEC, IDC_CODEC + 1, wxEVT_COMBOBOX, MyRecVideoDlg::OnChangeCodec)
	EVT_COMMAND_RANGE(IDC_QUALITY, IDC_QUALITY+ 1, wxEVT_COMBOBOX, MyRecVideoDlg::OnChangeQuality)
END_EVENT_TABLE()

MyRecVideoDlg::MyRecVideoDlg(wxWindow* parent, wxWindowID id, EMU *parent_emu, GUI_BASE *parent_gui)
	: MyDialog(parent, id, CMSG(Record_Screen), parent_emu, parent_gui)
{
}

MyRecVideoDlg::~MyRecVideoDlg()
{
}

/**
 * create config dialog when ShowModal called
 */
void MyRecVideoDlg::InitDialog()
{
	wxSizerFlags flags = wxSizerFlags().Expand().Border(wxALL, 4);

	wxBoxSizer *szrMain;
	wxBoxSizer *szrSub;
	wxBoxSizer *szrAll = new wxBoxSizer(wxVERTICAL);

	book = new wxNotebook(this, IDC_NOTEBOOK, wxDefaultPosition, wxDefaultSize);
	wxPanel *page;

	//
	int i;
	for(i=0; video_type_ids[i] != 0; i++) {
		page = new wxPanel(book);
		book->AddPage(page, _(video_type_label[i]));
		codnums.Add(0);
		quanums.Add(0);

		szrMain = new wxBoxSizer(wxVERTICAL);
		enables.Add(emu->rec_video_enabled(video_type_ids[i]) ? 1 : 0);

		switch(video_type_ids[i]) {
			case RECORD_VIDEO_TYPE_VFW:
			{
				szrSub = new wxBoxSizer(wxHORIZONTAL);
				szrSub->Add(new MyStaticText(page, wxID_ANY, CMsg::You_can_set_properties_after_pressing_start_button), flags);
				szrMain->Add(szrSub, flags);
				break;
			}
			default:
			{
				int count = 0;
				// Codec
				szrSub = new wxBoxSizer(wxHORIZONTAL);
				szrSub->Add(new MyStaticText(page, wxID_ANY, CMsg::Codec_Type), flags);
				const _TCHAR **codlbl = emu->get_rec_video_codec_list(video_type_ids[i]);
				wxArrayString codlbls;
				for(count = 0; codlbl[count] != NULL; count++) {
					codlbls.Add(wxString(codlbl[count]));
				}
				wxComboBox *comCodec = new wxComboBox(page, IDC_CODEC + i, wxT(""), wxDefaultPosition, wxDefaultSize, codlbls, wxCB_DROPDOWN | wxCB_READONLY);
				comCodec->SetSelection(0);
				szrSub->Add(comCodec, flags);
				szrMain->Add(szrSub, flags);

				// Quality
				szrSub = new wxBoxSizer(wxHORIZONTAL);
				szrSub->Add(new MyStaticText(page, wxID_ANY, CMsg::Quality), flags);
				const CMsg::Id *qualbl = emu->get_rec_video_quality_list(video_type_ids[i]);
				wxArrayString qualbls;
				for(count = 0; qualbl[count] != 0 && qualbl[count] != CMsg::End; count++) {
					qualbls.Add(gMessages.Get(qualbl[count]));
				}
				wxComboBox *comQuality = new wxComboBox(page, IDC_QUALITY + i, wxT(""), wxDefaultPosition, wxDefaultSize, qualbls, wxCB_DROPDOWN | wxCB_READONLY);
				comQuality->SetSelection(0);
				szrSub->Add(comQuality, flags);
				szrMain->Add(szrSub, flags);

				break;
			}
		}
		if (!enables[i]) {
			szrMain->Add(new wxStaticText(page, wxID_ANY, _("! Need install library.")), flags);
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

void MyRecVideoDlg::UpdateDialog()
{
	wxButton *btnOk = (wxButton *)FindWindow(wxID_OK);
	if (btnOk) {
		btnOk->Enable(enables[0] != 0);
	}
}

int MyRecVideoDlg::ShowModal()
{
//	InitDialog();
	int rc = MyDialog::ShowModal();
	if (rc == wxID_OK) {
		ModifyParam();
	}
	return rc;
}

void MyRecVideoDlg::ModifyParam()
{
	int selnum = book->GetSelection();

	emu->set_parami(VM::ParamRecVideoType, video_type_ids[selnum]);
	emu->set_parami(VM::ParamRecVideoCodec, codnums[selnum]);
	emu->set_parami(VM::ParamRecVideoQuality, quanums[selnum]);
}

/*
 * Event Handler
 */
void MyRecVideoDlg::OnChangeBook(wxBookCtrlEvent &event)
{
	int selnum = event.GetSelection();
	wxButton *btnOk = (wxButton *)FindWindow(wxID_OK);
	if (btnOk) {
		btnOk->Enable(enables[selnum] != 0);
	}
}

void MyRecVideoDlg::OnChangeCodec(wxCommandEvent &event)
{
	int selnum = event.GetId() - IDC_CODEC;
	wxComboBox *com = (wxComboBox *)FindWindow(event.GetId());
	if (com && codnums.Count() > (size_t)selnum) {
		int num = com->GetSelection();
		codnums[selnum] = num;
	}
}

void MyRecVideoDlg::OnChangeQuality(wxCommandEvent &event)
{
	int selnum = event.GetId() - IDC_QUALITY;
	wxComboBox *com = (wxComboBox *)FindWindow(event.GetId());
	if (com && quanums.Count() > (size_t)selnum) {
		int num = com->GetSelection();
		quanums[selnum] = num;
	}
}

