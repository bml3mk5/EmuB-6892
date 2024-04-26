/** @file wx_logging_dlg.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2024.04.13 -

	@brief [ wx_logging_dlg ]
*/

#include <wx/wx.h>
#include <wx/statline.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include "wx_logging_dlg.h"
#include "../../emu.h"
#include "../../logging.h"

// Attach Event
BEGIN_EVENT_TABLE(MyLoggingDlg, wxDialog)
	EVT_CLOSE(MyLoggingDlg::OnClose)
	EVT_SIZE(MyLoggingDlg::OnSize)
	EVT_BUTTON(IDC_BTN_UPDATE, MyLoggingDlg::OnUpdateButton)
END_EVENT_TABLE()

MyLoggingDlg::MyLoggingDlg(wxWindow* parent, wxWindowID id, EMU *parent_emu, GUI_BASE *parent_gui)
	: MyDialog(parent, id, CMSG(Log), parent_emu, parent_gui, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
	m_initialized = false;
	p_buffer = NULL;
	m_buffer_size = 0;
}

MyLoggingDlg::~MyLoggingDlg()
{
	delete [] p_buffer;
	p_buffer = NULL;
	m_buffer_size = 0;
}

/**
 * create config dialog when ShowModal called
 */
void MyLoggingDlg::InitDialog()
{
	if (m_initialized) return;

	wxSizerFlags flags = wxSizerFlags().Expand().Border(wxALL, 1);

	wxBoxSizer *szrAll = new wxBoxSizer(wxVERTICAL);

	wxSize siz(480, -1);
	txtPath = new wxTextCtrl(this, IDC_TEXT_LOGPATH, wxT(""), wxDefaultPosition, siz, wxTE_READONLY);
	szrAll->Add(txtPath, flags);

	siz.Set(480, 320);
	txtLog = new wxTextCtrl(this, IDC_TEXT_LOG, wxT(""), wxDefaultPosition, siz, wxTE_READONLY | wxTE_MULTILINE);
	szrAll->Add(txtLog, flags);

	// button
	wxSizer *szrH = new wxBoxSizer(wxHORIZONTAL);

	btnUpdate = new wxButton(this, IDC_BTN_UPDATE, CMSG(Update));
	szrH->Add(btnUpdate, flags);
	btnClose = new wxButton(this, wxID_CLOSE, CMSG(Close));
	szrH->Add(btnClose, flags);
	szrAll->Add(szrH, flags);

	SetAffirmativeId(wxID_CLOSE);

	SetSizerAndFit(szrAll);

	m_client_size = GetClientSize();

	m_initialized = true;

	// path
	txtPath->SetValue(logging->get_log_path());

	AdjustButtonPosition();
}

/*
 * Event Handler
 */

void MyLoggingDlg::OnClose(wxCloseEvent &event)
{
	Show(false);
}

void MyLoggingDlg::OnSize(wxSizeEvent &event)
{
	if (!m_initialized) {
		event.Skip();
		return;
	}
	wxSize siz;

	siz = GetClientSize();
	int s_w = siz.GetWidth() - m_client_size.GetWidth();
	int s_h = siz.GetHeight() - m_client_size.GetHeight();

	m_client_size = siz;

	siz = txtPath->GetSize();
	siz.SetWidth(siz.GetWidth() + s_w);
	txtPath->SetSize(siz);

	siz = txtLog->GetSize();
	siz.SetWidth(siz.GetWidth() + s_w);
	siz.SetHeight(siz.GetHeight() + s_h);
	txtLog->SetSize(siz);

	AdjustButtonPosition();
}

void MyLoggingDlg::AdjustButtonPosition()
{
	wxPoint pos;
	wxSize siz;
	siz = btnUpdate->GetSize();
	pos.x = 4;
	pos.y = m_client_size.GetHeight() - siz.GetHeight() - 4;
	btnUpdate->SetPosition(pos);

	siz = btnClose->GetSize();
	pos.x = m_client_size.GetWidth() - siz.GetWidth() - 4;
	pos.y = m_client_size.GetHeight() - siz.GetHeight() - 4;
	btnClose->SetPosition(pos);
}

void MyLoggingDlg::OnUpdateButton(wxCommandEvent &event)
{
	if (!p_buffer) {
		m_buffer_size = 1024 * 1024;
		p_buffer = new TCHAR[m_buffer_size];
	}

	p_buffer[0] = 0;
	logging->get_log(p_buffer, m_buffer_size);
	txtLog->SetValue(p_buffer);
}
