/** @file wx_keybind_dlg.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2014.03.23

	@brief [ keybind dialog ]
*/

#include <wx/wx.h>
#include "wx_keybind_dlg.h"
#include "../../emu.h"
#include "../../config.h"
#include "wx_gui.h"

// Attach Event
BEGIN_EVENT_TABLE(MyKeybindDlg, wxDialog)
	EVT_COMMAND_RANGE(IDC_BUTTON_LOAD_DEFAULT0, IDC_BUTTON_LOAD_DEFAULT2, wxEVT_BUTTON, MyKeybindDlg::OnLoadDefault)
	EVT_COMMAND_RANGE(IDC_BUTTON_LOAD_PRESET00, IDC_BUTTON_LOAD_PRESET32, wxEVT_BUTTON, MyKeybindDlg::OnLoadPreset)
	EVT_COMMAND_RANGE(IDC_BUTTON_SAVE_PRESET00, IDC_BUTTON_SAVE_PRESET32, wxEVT_BUTTON, MyKeybindDlg::OnSavePreset)
//	EVT_BUTTON(wxID_CANCEL, MyKeybindDlg::OnCancel)
//	EVT_KEY_DOWN(MyKeybindDlg::OnKeyDown)
END_EVENT_TABLE()

MyKeybindDlg::MyKeybindDlg(MyFrame* parent, wxWindowID id, EMU *parent_emu, GUI_BASE *parent_gui)
	: MyDialog(parent, id, CMSG(Keybind), parent_emu, parent_gui)
{
	frame = parent;

	for(int tab=0; tab<KEYBIND_MAX_NUM; tab++) {
		kbdata[tab] = new KeybindData();
	}

	tm1 = 0;

#ifdef USE_JOYSTICK
	// get parameters
	joy_stat = emu->joy_buffer();
#endif
}

MyKeybindDlg::~MyKeybindDlg()
{
	for(int tab=0; tab<KEYBIND_MAX_NUM; tab++) {
		delete kbdata[tab];
	}
}

/**
 * create config dialog when ShowModal called
 */
void MyKeybindDlg::InitDialog()
{
	wxSizerFlags flags = wxSizerFlags().Expand().Border(wxALL, 4);

	wxBoxSizer *szrAll = new wxBoxSizer(wxVERTICAL);
#if 1
	MyNotebook *book = new MyNotebook(this, IDC_NOTEBOOK, wxDefaultPosition, wxDefaultSize);
	book->AddPageById(CreateBook(book, 0), CMsg::Keyboard);
	book->AddPageById(CreateBook(book, 1), CMsg::Joypad_Key_Assigned);
	book->AddPageById(CreateBook(book, 2), CMsg::Joypad_PIA_Type);
	szrAll->Add(book, flags);
#else
	szrAll->Add(CreateBook(this, 0), flags);
#endif

	// ok cancel button
	wxSizer *szrButtons = CreateButtonSizer(wxOK|wxCANCEL);
#if 0
    wxStdDialogButtonSizer *szrButtons = new wxStdDialogButtonSizer();
	wxButton *ok = new wxButton(this, wxID_OK);
	wxButton *cancel = new wxButton(this, wxID_CANCEL);
	szrButtons->AddButton(ok);
	szrButtons->AddButton(cancel);
	ok->SetDefault();
	ok->SetFocus();
	SetAffirmativeId(wxID_NONE);
	SetEscapeId(wxID_NONE);
	szrButtons->Realize();
#endif
	szrAll->Add(szrButtons, flags);

//	cancel->Bind(wxEVT_KEY_DOWN, &MyKeybindDlg::OnKeyDown, cancel, wxID_CANCEL);

	SetSizerAndFit(szrAll);
}

wxWindow *MyKeybindDlg::CreateBook(wxWindow *parent, int tab)
{
	wxSizerFlags flags = wxSizerFlags().Expand().Border(wxALL, 2);

	wxPanel *page = new wxPanel(parent);

//	wxBoxSizer *szrLeft   = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *szrRight  = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *szrMain   = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *szrAll    = new wxBoxSizer(wxVERTICAL);

	_TCHAR clabel[128];
	wxString label[3];

#ifdef USE_PIAJOYSTICKBIT
	kbdata[tab]->Init(emu, tab, tab == 0 ? 0 : 1, tab == 2 ? 2 : 0);
#else
	kbdata[tab]->Init(emu, tab, tab == 0 ? 0 : 1, tab == 2 ? 1 : 0);
#endif

	int rows = kbdata[tab]->GetNumberOfRows();
	int cols = kbdata[tab]->GetNumberOfColumns();

	// table
	list[tab] = new MyKeybindListWindow(this, tab, page, IDC_LIST_0 + tab, wxDefaultPosition, wxSize(-1,400), wxBORDER_SIMPLE | wxVSCROLL);
#if 1
	list[tab]->Show(false);
	label[0] = CMSG(Level3_Key);
	if (tab != 0) {
		// joypad
		for(int col=0; col<cols; col++) {
			label[col+1] = wxString::Format(CMSG(JoypadVDIGIT), col+1);
		}
	} else {
		// keyboard
		for(int col=0; col<cols; col++) {
			label[col+1] = wxString::Format(CMSG(BindVDIGIT), col+1);
		}
	}
	list[tab]->InsertColumns(3, label);

	// set datas
	for(int row=0; row<rows; row++) {
		kbdata[tab]->GetCellString(row, -1, clabel);
		label[0] = wxString(clabel);
		for(int col=0; col<cols; col++) {
			kbdata[tab]->GetCellString(row, col, clabel);
			label[col + 1] = wxString(clabel);
		}
		list[tab]->AppendRow(row, label, kbdata[tab]);
	}
	//	bszr->Add(list, flags);
//	scr->SetSizerAndFit(bszr);
//	szrLeft->Add(scr, flags);
	list[tab]->Show(true);
#endif
	// load and save buttons
	wxButton *btn;
	btn = new MyButton(page, IDC_BUTTON_LOAD_DEFAULT0 + tab, CMsg::Load_Default);
	szrRight->Add(btn, flags);
	szrRight->Add(new wxStaticText(page, wxID_ANY, wxT(" ")), flags);
	for(int i=0; i<KEYBIND_PRESETS; i++) {
		wxString str = wxString::Format(CMSG(Load_Preset_VDIGIT), i + 1);
		btn = new wxButton(page, IDC_BUTTON_LOAD_PRESET00 + tab + i * KEYBIND_MAX_NUM, str);
		szrRight->Add(btn, flags);
	}
	szrRight->Add(new wxStaticText(page, wxID_ANY, wxT(" ")), flags);
	for(int i=0; i<KEYBIND_PRESETS; i++) {
		wxString str = wxString::Format(CMSG(Save_Preset_VDIGIT), i + 1);
		btn = new wxButton(page, IDC_BUTTON_SAVE_PRESET00 + tab + i * KEYBIND_MAX_NUM, str);
		szrRight->Add(btn, flags);
	}

//	szrMain->Add(scr, flags);
	szrMain->Add(list[tab], flags);
	szrMain->Add(szrRight, flags);
	szrAll->Add(szrMain, flags);

	// joypad 
	if (tab == 1) {
		chkCombi = new MyCheckBox(page, IDC_CHECK_COMBI, CMsg::Recognize_as_another_key_when_pressed_two_buttons);
		int combi1 = (int)kbdata[tab]->GetCombi();
		chkCombi->SetValue(combi1 != 0);
		szrAll->Add(chkCombi, flags);
	}

    page->SetSizer(szrAll);

	return page;
}

void MyKeybindDlg::UpdateDialog()
{
}

int MyKeybindDlg::ShowModal()
{
//	Init_Dialog();
	int rc = MyDialog::ShowModal();
	if (rc == wxID_OK) {
		ModifyParam();
	}
	return rc;
}

void MyKeybindDlg::ModifyParam()
{
	for(int tab=0; tab<KEYBIND_MAX_NUM; tab++) {
		kbdata[tab]->SetData();
	}
}

#if 0
void MyKeybindDlg::SetVmKeyMap(int tab_num, uint16_t *vmKeyMap, int size)
{
	if (vmKeyMap == NULL) return;
	if (tab_num < 0 || tab_num >= KEYBIND_MAX_NUM) return;

	for(int i=0; i<KBCTRL_MAX_LINES && i<size; i++) {
//		vmkey_map[tab_num][i] = vmKeyMap[i];
		SetVmKeyCode(tab_num, i, vmKeyMap[i]);
	}
	vmkey_map_size[tab_num] = (size > KBCTRL_MAX_LINES) ? KBCTRL_MAX_LINES : size;
}

void MyKeybindDlg::SetVmKey(int tab_num, int idx, uint16_t code)
{
	if (tab_num < 0 || tab_num >= KEYBIND_MAX_NUM) return;

//	vmkey_map[tab_num][idx] = code;
	SetVmKeyCode(tab_num, idx, code);
}

bool MyKeybindDlg::SetVmKeyCode(int tab_num, int idx, uint16_t code)
{
	if (idx < 0 || idx >= KBCTRL_MAX_LINES) {
		return false;
	}
	if (code >= 0xff) {
		table[tab_num][idx].enabled = false;
	} else {
		table[tab_num][idx].enabled = true;
	}

	table[tab_num][idx].vm_keycode = code;

	return true;
}

void MyKeybindDlg::SetVkKeyMap(int tab_num, uint32_t *vkKeyMap)
{
	if (tab_num < 0 || tab_num >= KEYBIND_MAX_NUM) return;

	vkkey_map[tab_num] = vkKeyMap;
}

void MyKeybindDlg::SetVkKeyDefMap(int tab_num, uint32_t *vkKeyDefMap, int rows, int cols)
{
	if (tab_num < 0 || tab_num >= KEYBIND_MAX_NUM) return;

	vkkey_defmap[tab_num] = vkKeyDefMap;
	vkkey_defmap_rows[tab_num] = rows;
	vkkey_defmap_cols[tab_num] = cols;
}

void MyKeybindDlg::SetVkKeyPresetMap(int tab_num, uint32_t *vkKeyMap, int idx)
{
	if (tab_num < 0 || tab_num >= KEYBIND_MAX_NUM) return;

	if (idx < MAX_PRESET) {
		vkkey_preset_map[tab_num][idx] = vkKeyMap;
	}
}

bool MyKeybindDlg::SetVkKeyCode(codecols_t *obj, uint32_t code, wxString &label)
{
	uint32_t new_code = 0;

	new_code = translate_vkkey(code);
	get_vkkeylabel(new_code, label);
	obj->vk_prev_keycode = obj->vk_keycode;
	obj->vk_keycode = new_code;

	return true;
}

bool MyKeybindDlg::SetVkJoyCode(codecols_t *obj, uint32_t code, wxString &label)
{
	bool rc = false;

	rc = get_vkjoylabel(code, label);
	if (rc == true) {
		obj->vk_prev_keycode = obj->vk_keycode;
		obj->vk_keycode = code;
	}
	return rc;
}
#endif

void MyKeybindDlg::SetTitleLabel(const _TCHAR *vm, const _TCHAR *vk, const _TCHAR *vj)
{
}

/// load keybind data to dialog from buffer
void MyKeybindDlg::load_data(int tab, int num)
{
	kbdata[tab]->LoadPreset(num);
	list[tab]->UpdateItems();
}

/// save keybind data to buffer from dialog
void MyKeybindDlg::save_data(int tab, int num)
{
	kbdata[tab]->SavePreset(num);
}

bool MyKeybindDlg::get_vmkeylabel(int code, wxString &label)
{
	_TCHAR clabel[128];
	bool rc = KeybindData::GetVmKeyLabel(code, clabel);
	label = wxString(clabel);
	return rc;
}

uint32_t MyKeybindDlg::translate_vkkey(uint32_t code) {
//	UINT32 new_code = code;

//	emu->translate_sdl_keysym();
	return code;
}

bool MyKeybindDlg::get_vkkeylabel(uint32_t code, wxString &label)
{
	_TCHAR clabel[128];
	bool rc = KeybindData::GetVkKeyLabel(code, clabel);
	label = wxString(clabel);
	return rc;
}

bool MyKeybindDlg::get_vkjoylabel(uint32_t code, wxString &label)
{
	_TCHAR clabel[128];
	bool rc = KeybindData::GetVkJoyLabel(code, clabel);
	label = wxString(clabel);
	return rc;
}

/*
 * Event Handler
 */
void MyKeybindDlg::OnLoadDefault(wxCommandEvent &event) {
	int id = event.GetId() - IDC_BUTTON_LOAD_DEFAULT0;
	int tab = id % KEYBIND_MAX_NUM;

	load_data(tab, -1);
}
void MyKeybindDlg::OnLoadPreset(wxCommandEvent &event) {
	int id = event.GetId() - IDC_BUTTON_LOAD_PRESET00;
	int num = id / KEYBIND_MAX_NUM;
	int tab = id % KEYBIND_MAX_NUM;

	load_data(tab, num);
}
void MyKeybindDlg::OnSavePreset(wxCommandEvent &event) {
	int id = event.GetId() - IDC_BUTTON_SAVE_PRESET00;
	int num = id / KEYBIND_MAX_NUM;
	int tab = id % KEYBIND_MAX_NUM;

	save_data(tab, num);
}
#if 0
void MyKeybindDlg::OnCancel(wxCommandEvent &event) {
	int id = event.GetId();
	int i= 0;
}

void MyKeybindDlg::OnKeyDown(wxKeyEvent &event)
{
	int code = event.GetKeyCode();
	wxChar unicode = event.GetUnicodeKey();
	wxUint32_t rawcode = event.GetRawKeyCode();
	frame->TranslateKeyCode(code, unicode, rawcode);


}
#endif
/**
 * List Control
 */
MyKeybindListWindow::MyKeybindListWindow(MyKeybindDlg *parent_dlg, int tab_no, wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size, int style)
		: wxScrolledWindow(parent, wxID_ANY, pos, size, style)
{
	dlg = parent_dlg;
	tab = tab_no;
	list = new MyKeybindListCtrl(this, tab_no, id);
//	Bind(wxEVT_KEY_DOWN, &MyKeybindListWindow::OnKeyDown, this, id);
//	wxBoxSizer *szrAll = new wxBoxSizer(wxVERTICAL);
//	szrAll->Add(list, 0);
//	SetSizer(szrAll);
}
MyKeybindListWindow::~MyKeybindListWindow()
{
}
void MyKeybindListWindow::InsertColumns(int cols, const wxString *labels)
{
	list->InsertColumns(cols, labels);
}
void MyKeybindListWindow::AppendRow(int row, const wxString *labels, KeybindData *items)
{
	list->AppendRow(row, labels, items);
}
void MyKeybindListWindow::UpdateItems() {
	list->UpdateItems();
}
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
//
MyKeybindListCtrl::MyKeybindListCtrl(MyKeybindListWindow *parent, int tab_no, wxWindowID id, const wxPoint &pos, const wxSize &size, int style)
		: wxControl(parent, id, pos, size, style)
{
	dlg = parent->GetDlg();
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
		dlg->GetEMU()->update_joystick();

		now_textctl->SetJoyLabel();
	}
}
#if 0
void MyKeybindListCtrl::OnChildFocus(wxChildFocusEvent &event)
{
	now_focus_id = event.GetId();
}
#endif

//
MyKeybindListEdit::MyKeybindListEdit(MyKeybindListCtrl *parent, wxWindowID id, const wxString &label,int tab_num, int row, int col, KeybindData *kbdata, const wxPoint &pos, const wxSize &size, int style)
		: wxTextCtrl(parent, id, label, pos, size, style | wxTE_READONLY | wxTE_PROCESS_ENTER)
{
	dlg = parent->GetDlg();
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
	uint32_t code = dlg->GetJoyStat(col);
	// no update if code is 0
	if (code != 0 && kbdata->SetVkJoyCode(row, col, code, clabel)) {
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
void MyKeybindListEdit::OnCharHook(wxKeyEvent &event)
{
	if (tab_num != 0) return;

	int code = event.GetKeyCode();
//	int unicode = (int)event.GetUnicodeKey();
	short rawcode = (short)event.GetRawKeyCode();
	long  rawflag = (long)event.GetRawKeyFlags();
	dlg->GetEMU()->translate_keysym(0, rawcode, rawflag, &code);
	SetKeyLabel(code);
}
void MyKeybindListEdit::OnMouseLeftDClick(wxMouseEvent & WXUNUSED(event))
{
	SetLabel(0);
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
