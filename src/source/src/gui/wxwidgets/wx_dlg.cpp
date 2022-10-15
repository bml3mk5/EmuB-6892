/** @file wx_dlg.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2014.03.231

	@brief [ dialog ]
*/

//#include <wx/wx.h>
#include "wx_dlg.h"

MyDialog::MyDialog(wxWindow *parent, wxWindowID id, const wxString &title, EMU *parent_emu, GUI_BASE *parent_gui
		, const wxPoint &pos
		, const wxSize &size
		, long style)
	: wxDialog(parent, id, title, pos, size, style)
{
	emu = parent_emu;
	gui = parent_gui;
}

//

MyNotebook::MyNotebook(wxWindow *parent,
	wxWindowID id,
	const wxPoint& pos,
	const wxSize& size,
	long style)
	: wxNotebook(parent, id, pos, size, style)
{
}

bool MyNotebook::AddPageById(wxWindow *page, CMsg::Id textid, bool bSelect, int imageId)
{
	const _TCHAR *text = CMSGV(textid);
	return AddPage(page, text, bSelect, imageId);
}

//

MyStaticBox::MyStaticBox(wxWindow *parent, wxWindowID id,
		CMsg::Id labelid,
		const wxPoint& pos,
		const wxSize& size,
		long style)
	: wxStaticBox(parent, id, wxEmptyString, pos, size, style)
{
	SetLabelById(labelid);
}

void MyStaticBox::SetLabelById(CMsg::Id labelid)
{
	const _TCHAR *label = CMSGV(labelid);
	SetLabel(label);
}

//

MyStaticText::MyStaticText(wxWindow *parent,
		wxWindowID id,
		CMsg::Id labelid,
		const wxPoint &pos,
		const wxSize &size,
		long style)
	: wxStaticText(parent, id, wxEmptyString, pos, size, style)
{
	SetLabelById(labelid);
}

void MyStaticText::SetLabelById(CMsg::Id labelid)
{
	const _TCHAR *label = CMSGV(labelid);
	SetLabel(label);
}

//

MyChoice::MyChoice(wxWindow *parent, wxWindowID id,
		const wxPoint& pos,
		const wxSize& size,
		const _TCHAR *choices[],
		long style,
		const wxValidator& validator)
	: wxChoice(parent, id, pos, size, 0, NULL, style, validator)
{
	if (choices) {
		for(int i=0; choices[i] != NULL; i++) {
			Append(choices[i]);
		}
	}
}

MyChoice::MyChoice(wxWindow *parent, wxWindowID id,
		const wxPoint& pos,
		const wxSize& size,
		const CMsg::Id choices[],
		long style,
		int selidx,
		int appendidx,
		CMsg::Id appendstr,
		const wxValidator& validator)
	: wxChoice(parent, id, pos, size, 0, NULL, style, validator)
{
	if (choices) {
		for(int i=0; choices[i] != CMsg::End; i++) {
			if (i == appendidx) {
				wxString label;
				label += CMSGV(choices[i]);
				label += CMSGV(appendstr);
				Append(label);
			} else {
				AppendById(choices[i]);
			}
		}
	}
}

MyChoice::MyChoice(wxWindow *parent, wxWindowID id,
		const wxPoint& pos,
		const wxSize& size,
		const CPtrList<CTchar> &choices,
		long style,
		const wxValidator& validator)
	: wxChoice(parent, id, pos, size, 0, NULL, style, validator)
{
	for(int i=0; i<choices.Count(); i++) {
		Append(choices.Item(i)->Get());
	}
}

int MyChoice::AppendById(CMsg::Id itemid)
{
	const _TCHAR *item = CMSGV(itemid);
	return Append(item);
}

//

MyComboBox::MyComboBox(wxWindow *parent, wxWindowID id,
		const wxPoint& pos,
		const wxSize& size,
		const CMsg::Id choices[],
		long style,
		const wxValidator& validator)
	: wxComboBox(parent, id, wxEmptyString, pos, size, 0, NULL, style, validator)
{
	if (choices) {
		for(int i=0; choices[i] != CMsg::End; i++) {
			AppendById(choices[i]);
		}
	}
}

int MyComboBox::AppendById(CMsg::Id itemid)
{
	const _TCHAR *item = CMSGV(itemid);
	return Append(item);
}

//

MyCheckBox::MyCheckBox(wxWindow *parent, wxWindowID id, CMsg::Id labelid,
		const wxPoint& pos,
		const wxSize& size, long style,
		const wxValidator& validator)
	: wxCheckBox(parent, id, wxEmptyString, pos, size, style, validator)
{
	SetLabelById(labelid);
}

void MyCheckBox::SetLabelById(CMsg::Id labelid)
{
	const _TCHAR *label = CMSGV(labelid);
	SetLabel(label);
}

//

MyRadioButton::MyRadioButton(wxWindow *parent,
		wxWindowID id,
		CMsg::Id labelid,
		const wxPoint& pos,
		const wxSize& size,
		long style,
		const wxValidator& validator)
	: wxRadioButton(parent, id, wxEmptyString, pos, size, style, validator)
{
	SetLabelById(labelid);
}

void MyRadioButton::SetLabelById(CMsg::Id labelid)
{
	const _TCHAR *label = CMSGV(labelid);
	SetLabel(label);
}

//

MyButton::MyButton(wxWindow *parent, wxWindowID id,
	CMsg::Id labelid,
	const wxPoint& pos,
	const wxSize& size, long style,
	const wxValidator& validator)
	: wxButton(parent, id, wxEmptyString, pos, size, style, validator)
{
	SetLabelById(labelid);
}

void MyButton::SetLabelById(CMsg::Id labelid)
{
	const _TCHAR *label = CMSGV(labelid);
	SetLabel(label);
}

//

wxMenuItem* MyMenu::AppendById(int itemid, CMsg::Id textid, const wxString& help, wxItemKind kind)
{
	const _TCHAR *text = CMSGV(textid);
	return wxMenu::Append(itemid, text, help, kind);
}

wxMenuItem* MyMenu::AppendCheckItemById(int itemid, CMsg::Id textid, const wxString& help)
{
	return AppendById(itemid, textid, help, wxITEM_CHECK);
}

wxMenuItem* MyMenu::AppendRadioItemById(int itemid, CMsg::Id textid, const wxString& help)
{
	return AppendById(itemid, textid, help, wxITEM_RADIO);
}

wxMenuItem* MyMenu::AppendSubMenuById(MyMenu *submenu, CMsg::Id textid, const wxString& help)
{
	const _TCHAR *text = CMSGV(textid);
	return wxMenu::AppendSubMenu(submenu, text, help);
}
