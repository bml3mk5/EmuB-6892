/** @file wx_keybind_ctrl.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2024.02.04

	@brief [ keybind control ]
*/

#include <wx/wx.h>
#include "wx_keybind_ctrl.h"
#include "../../emu.h"
#include "../../config.h"
#include "../../labels.h"
#include "../../keycode.h"
#include "wx_gui.h"

extern EMU *emu;

/**
 * List Control
 */
MyKeybindListWindow::MyKeybindListWindow(int tab_no, wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size, int style)
		: wxScrolledWindow(parent, wxID_ANY, pos, size, style)
{
//	dlg = parent_dlg;
	m_tab_num = tab_no;

	ctrl = new MyKeybindListCtrl(this, tab_no, id);
	ctrl->Show(false);

	kbdata = new KeybindData();
	kbdata->Init(emu, tab_no);

	int rows = kbdata->GetNumberOfRows();
	int cols = kbdata->GetNumberOfColumns();

	_TCHAR clabel[128];
	wxString labels[3];

	// set header
	labels[0] = CMSGV(LABELS::keybind_col[m_tab_num][0]);
	for(int col=1; col<=cols; col++) {
		labels[col] = wxString::Format(CMSGV(LABELS::keybind_col[m_tab_num][1]), col);
	}
	ctrl->InsertColumns(3, labels);

	// set datas
	for(int row=0; row<rows; row++) {
		kbdata->GetCellString(row, -1, clabel);
		labels[0] = wxString(clabel);
		for(int col=0; col<cols; col++) {
			kbdata->GetCellString(row, col, clabel);
			labels[col + 1] = wxString(clabel);
		}
		ctrl->AppendRow(row, labels, kbdata);
	}

	// joypad 
	chkCombi = NULL;
	if (LABELS::keybind_combi[m_tab_num] != CMsg::Null) {
		chkCombi = new MyCheckBox(parent, wxID_ANY, LABELS::keybind_combi[m_tab_num]);
		int combi1 = (int)kbdata->GetCombi();
		chkCombi->SetValue(combi1 != 0);
	}

	ctrl->Show(true);
}
MyKeybindListWindow::~MyKeybindListWindow()
{
	delete kbdata;
}
#if 0
void MyKeybindListWindow::InsertColumns(int cols, const wxString *labels)
{
	ctrl->InsertColumns(cols, labels);
}
void MyKeybindListWindow::AppendRow(int row, const wxString *labels, KeybindData *items)
{
	list->AppendRow(row, labels, items);
}
void MyKeybindListWindow::UpdateItems() {
	list->UpdateItems();
}
#endif
void MyKeybindListWindow::SetScrollSize(int px, int py)
{
	int unit = 16;
	int ux = px / unit;
	int uy = py / unit;
	SetScrollbars(ux,uy,unit,unit);
}
#if 0
void MyKeybindListWindow::OnKeyDown(wxKeyEvent &event)
{
	int code = event.GetKeyCode();
	wxChar unicode = event.GetUnicodeKey();
	wxUint32_t rawcode = event.GetRawKeyCode();
	frame->TranslateKeyCode(code, unicode, rawcode);


}
#endif
void MyKeybindListWindow::SetKeybindData()
{
	kbdata->SetData();
}
void MyKeybindListWindow::LoadPreset(int num)
{
	kbdata->LoadPreset(num);
	ctrl->UpdateItems();
}
void MyKeybindListWindow::SavePreset(int num)
{
	kbdata->SavePreset(num);
}

//
MyKeybindListCtrl::MyKeybindListCtrl(MyKeybindListWindow *parent, int tab_no, wxWindowID id, const wxPoint &pos, const wxSize &size, int style)
		: wxControl(parent, id, pos, size, style)
{
//	dlg = parent->GetDlg();
	owner = parent;
	tab = tab_no;
	timer = NULL;
	now_textctl = NULL;

	// event
//	Bind(wxEVT_PAINT, &MyKeybindListCtrl::OnPaint, this, id);
//	Bind(wxEVT_CHILD_FOCUS, &MyKeybindListCtrl::OnChildFocus, this, id);
	Bind(wxEVT_TIMER, &MyKeybindListCtrl::OnTimer, this, IDC_TIMER);

	max_size = wxSize(0,0);

	border = 2;
	now_pos = wxPoint(border,border);
	btn_size0 = wxSize(80,24);
	btn_size = wxSize(120,24);
	max_rows = 0;

	max_btn_ids = 0;

	if (tab != 0) {
		timer = new wxTimer(this, IDC_TIMER);
		timer->Start(250);
	}

	textctl.clear();
//	btn_size = wxSize(120,-1);
//	now_pos = wxDefaultPosition;

//	flags = wxSizerFlags().Expand().Border(wxALL, 1);
//	szrAll = new wxBoxSizer(wxVERTICAL);

//	SetSizer(szrAll);
}

MyKeybindListCtrl::~MyKeybindListCtrl()
{
	delete timer;
	textctl.clear();
}

void MyKeybindListCtrl::InsertColumns(int cols, const wxString *labels)
{
//	wxBoxSizer *szr = new wxBoxSizer(wxHORIZONTAL);
	max_cols = cols;

	wxStaticText *st;
	st = new wxStaticText(this, wxID_ANY, labels[0], now_pos, btn_size0, wxALIGN_CENTRE | wxALIGN_CENTRE_HORIZONTAL);
	st->SetBackgroundColour("WHITE");
	now_pos.x += btn_size0.x + border;
//	szr->Add(st, flags);
	for(int i=1; i<cols; i++) {
		st = new wxStaticText(this, wxID_ANY, labels[i], now_pos, btn_size, wxALIGN_CENTRE | wxALIGN_CENTRE_HORIZONTAL);
		st->SetBackgroundColour("WHITE");
		now_pos.x += btn_size.x + border;
//		szr->Add(st, flags);
	}

	now_pos.y += btn_size.y + border;

	SetMaxSize(now_pos.x, now_pos.y);

	now_pos.x = border;
//	szrAll->Add(szr,flags);
}

void MyKeybindListCtrl::AppendRow(int row, const wxString *labels, KeybindData *items)
{
//	wxBoxSizer *szr = new wxBoxSizer(wxHORIZONTAL);
	max_rows++;
//	return;

	wxStaticText *st;
	st = new wxStaticText(this, wxID_ANY, labels[0], now_pos, btn_size0, wxALIGN_CENTRE_HORIZONTAL);
	st->SetBackgroundColour("WHITE");
	now_pos.x += btn_size0.x + border;
//	/szr->Add(st, flags);
	MyKeybindListEdit *tc;
	for(int i=1; i<max_cols; i++) {
		max_btn_ids++;
#if 1
		tc = new MyKeybindListEdit(this, max_btn_ids, labels[i], tab, row, i-1, items, now_pos, btn_size, wxALIGN_CENTRE_HORIZONTAL);

		textctl.push_back(tc);
#endif
		now_pos.x += btn_size.x + border;
//		szr->Add(tc, flags);
	}

	now_pos.y += btn_size.y + border;

	SetMaxSize(now_pos.x, now_pos.y);

	now_pos.x = border;
//	szrAll->Add(szr,flags);
}

void MyKeybindListCtrl::UpdateItems() {
	wxVector<MyKeybindListEdit *>::iterator it;

	for(it = textctl.begin(); it != textctl.end(); it++) {
		(*it)->UpdateItem();
	}
}

void MyKeybindListCtrl::ScrollWindow(int dx, int dy, const wxRect *rect)
{
 //   wxControl::ScrollWindow( dx, dy, rect );
}
void MyKeybindListCtrl::GetMaxSize(int &w, int &h)
{
	w = max_size.x;
	h = max_size.y;
}
void MyKeybindListCtrl::SetMaxSize(int w, int h)
{
	max_size.x = w;
	max_size.y = h;
	SetClientSize(w, h);
	owner->SetScrollSize(w, h);
}

void MyKeybindListCtrl::OnPaint(wxPaintEvent& WXUNUSED(event))
{
}
void MyKeybindListCtrl::OnTimer(wxTimerEvent &event)
{
	if (now_textctl != NULL && now_textctl->HasFocus()) {
		emu->update_joystick();

		now_textctl->SetJoyLabel();
	}
}
#if 0
void MyKeybindListCtrl::OnChildFocus(wxChildFocusEvent &event)
{
	now_focus_id = event.GetId();
}
#endif
MyKeybindListEdit *MyKeybindListCtrl::GetCell(int row, int col)
{
	return textctl.at(row * (max_cols - 1) + col);
}
uint32_t *MyKeybindListCtrl::GetJoyStat(int num)
{
	return emu->joy_real_buffer(num);
}
uint32_t MyKeybindListCtrl::GetJoyMask() const
{
	return owner->GetJoyMask();
}

//
MyKeybindListEdit::MyKeybindListEdit(MyKeybindListCtrl *parent, wxWindowID id, const wxString &label,int tab_num, int row, int col, KeybindData *kbdata, const wxPoint &pos, const wxSize &size, int style)
		: wxTextCtrl(parent, id, label, pos, size, style | wxTE_READONLY | wxTE_PROCESS_ENTER)
{
//	dlg = parent->GetDlg();
	owner = parent;
	this->tab_num = tab_num;
	this->row = row;
	this->col = col;
	this->kbdata = kbdata;
	// event
//	Bind(wxEVT_KEY_DOWN, &MyKeybindListEdit::OnKeyDown, this, id);
	Bind(wxEVT_CHAR_HOOK, &MyKeybindListEdit::OnCharHook, this, id);
//	Bind(wxEVT_TEXT_ENTER, &MyKeybindListEdit::OnTextEnter, this, id);
	Bind(wxEVT_LEFT_DCLICK, &MyKeybindListEdit::OnMouseLeftDClick, this, id);
//	Bind(wxEVT_JOY_BUTTON_DOWN, &MyKeybindListEdit::OnJoyButtonDown, this, id);
	Bind(wxEVT_SET_FOCUS, &MyKeybindListEdit::OnSetFocus, this, id);
	Bind(wxEVT_KILL_FOCUS, &MyKeybindListEdit::OnKillFocus, this, id);
}
MyKeybindListEdit::~MyKeybindListEdit()
{
//	Unbind(wxEVT_KEY_DOWN, &MyKeybindListEdit::OnKeyDown, this);
}
void MyKeybindListEdit::UpdateItem()
{
	_TCHAR clabel[128];
	kbdata->GetCellString(row, col, clabel);
	this->SetValue(wxString(clabel));
}
void MyKeybindListEdit::SetJoyLabel()
{
	_TCHAR clabel[128];
	uint32_t *code = owner->GetJoyStat(col);
	uint32_t joy_mask = owner->GetJoyMask();
	// no update if code is 0
	if (((code[0] & joy_mask) | code[1]) != 0 && kbdata->SetVkJoyCode(row, col, code[0] & joy_mask, code[1], clabel)) {
		SetValue(wxString(clabel));
	}
}
void MyKeybindListEdit::SetKeyLabel(uint32_t code)
{
	_TCHAR clabel[128];
	kbdata->SetVkKeyCode(row, col, code, clabel);
	this->SetValue(wxString(clabel));
}
void MyKeybindListEdit::SetLabel(uint32_t code)
{
	_TCHAR clabel[128];
	kbdata->SetVkCode(row, col, code, clabel);
	this->SetValue(wxString(clabel));
}
void MyKeybindListEdit::ClearLabel()
{
	_TCHAR clabel[128];
	kbdata->ClearVkCode(row, col, clabel);
	this->SetValue(wxString(clabel));
}
void MyKeybindListEdit::ClearCellByCode(uint32_t code)
{
	_TCHAR clabel[128];
	int nrow;
	int ncol;
	bool rc = kbdata->ClearCellByVkKeyCode(code, clabel, &nrow, &ncol);
	if (rc) {
		MyKeybindListEdit *item = owner->GetCell(nrow, ncol);
		if (item) {
			item->SetValue(wxString(clabel));
		}
	}
}
void MyKeybindListEdit::OnCharHook(wxKeyEvent &event)
{
	if (kbdata->m_devtype == KeybindData::DEVTYPE_JOYPAD) return;

	int code = event.GetKeyCode();
//	int unicode = (int)event.GetUnicodeKey();
	short rawcode = (short)event.GetRawKeyCode();
	long  rawflag = (long)event.GetRawKeyFlags();
	emu->translate_keysym(0, rawcode, rawflag, &code);
	ClearCellByCode(code);
	SetKeyLabel(code);
}
void MyKeybindListEdit::OnMouseLeftDClick(wxMouseEvent & WXUNUSED(event))
{
	ClearLabel();
}
void MyKeybindListEdit::OnSetFocus(wxFocusEvent &event)
{
	owner->SetEditPtr(this);
	event.Skip();
}
void MyKeybindListEdit::OnKillFocus(wxFocusEvent &event)
{
	owner->SetEditPtr(NULL);
	event.Skip();
}
#if 0
void MyKeybindListEdit::OnJoyButtonDown(wxJoystickEvent &event)
{
	if (tab_num == 0) return;

	dlg->GetEMU()->update_joystick();

	SetLabel(dlg->GetJoyStat(col));
}
void MyKeybindListEdit::OnKeyDown(wxKeyEvent &event)
{
	if (val->tab != 0) return;

	int code = event.GetKeyCode();
	wxChar unicode = event.GetUnicodeKey();
	wxUint32_t rawcode = event.GetRawKeyCode();
 	dlg->GetFrame()->TranslateKeyCode(code, unicode, rawcode);
	SetLabel(code);
}
void MyKeybindListEdit::OnTextEnter(wxCommandEvent &event)
{
	if (val->tab != 0) return;

}
#endif
