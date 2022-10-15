/** @file wx_dlg.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2014.03.23

	@brief [ wx_dlg ]
*/

#ifndef WX_DLG_H
#define WX_DLG_H

#include "../../common.h"
#include "../../depend.h"
#include "../../msgs.h"
#include <wx/dialog.h>
#include <wx/notebook.h>
#include <wx/menu.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/choice.h>
#include <wx/combobox.h>
#include <wx/button.h>
#include <wx/radiobut.h>
#include <wx/checkbox.h>
#include "../../cchar.h"
#include "../../cptrlist.h"

class EMU;
class GUI_BASE;

/**
	@brief dialog base class
*/
class MyDialog : public wxDialog
{
protected:
	EMU         *emu;
	GUI_BASE	*gui;

public:
	MyDialog(wxWindow *parent, wxWindowID id, const wxString &title, EMU *parent_emu, GUI_BASE *parent_gui
		, const wxPoint &pos = wxDefaultPosition
		, const wxSize &size = wxDefaultSize
		, long style = wxDEFAULT_DIALOG_STYLE);
	virtual ~MyDialog() {}
	EMU *GetEMU() { return emu; }

//	DECLARE_ABSTRACT_CLASS(MyDialog)
};

//IMPLEMENT_ABSTRACT_CLASS(MyDialog, wxDialog)

/**
	@brief Tab control
*/
class MyNotebook : public wxNotebook
{
public:
	MyNotebook(wxWindow *parent,
		wxWindowID id,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = 0);
	bool AddPageById(wxWindow *page, CMsg::Id textid, bool bSelect = false, int imageId = NO_IMAGE);
};

/**
	@brief Static group box
*/
class MyStaticBox : public wxStaticBox
{
public:
	MyStaticBox(wxWindow *parent, wxWindowID id,
		CMsg::Id labelid,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = 0);
	void SetLabelById(CMsg::Id labelid);
};

/**
	@brief Static text
*/
class MyStaticText : public wxStaticText
{
public:
	MyStaticText(wxWindow *parent,
		wxWindowID id,
		CMsg::Id labelid,
		const wxPoint &pos = wxDefaultPosition,
		const wxSize &size = wxDefaultSize,
		long style = 0);

	void SetLabelById(CMsg::Id labelid);
};

/**
	@brief Choice control
*/
class MyChoice : public wxChoice
{
public:
	MyChoice(wxWindow *parent, wxWindowID id,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		const _TCHAR *choices[] = NULL,
		long style = 0,
		const wxValidator& validator = wxDefaultValidator);
	MyChoice(wxWindow *parent, wxWindowID id,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		const CMsg::Id choices[] = NULL,
		long style = 0,
		int selidx = -1,
		int appendidx = -1,
		CMsg::Id appendstr = CMsg::End,
		const wxValidator& validator = wxDefaultValidator);
	MyChoice(wxWindow *parent, wxWindowID id,
		const wxPoint& pos,
		const wxSize& size,
		const CPtrList<CTchar> &choices,
		long style = 0,
		const wxValidator& validator = wxDefaultValidator);

	int AppendById(CMsg::Id itemid);
};

/**
	@brief Combo control
*/
class MyComboBox : public wxComboBox
{
public:
	MyComboBox(wxWindow *parent, wxWindowID id,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		const CMsg::Id choices[] = NULL,
		long style = 0,
		const wxValidator& validator = wxDefaultValidator);

	int AppendById(CMsg::Id itemid);
};

/**
	@brief Check button
*/
class MyCheckBox : public wxCheckBox
{
public:
	MyCheckBox(wxWindow *parent, wxWindowID id, CMsg::Id labelid,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize, long style = 0,
		const wxValidator& validator = wxDefaultValidator);

	void SetLabelById(CMsg::Id labelid);
};

/**
	@brief Radio button
*/
class MyRadioButton : public wxRadioButton
{
public:
	MyRadioButton(wxWindow *parent,
		wxWindowID id,
		CMsg::Id labelid,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = 0,
		const wxValidator& validator = wxDefaultValidator);

	void SetLabelById(CMsg::Id labelid);
};

/**
	@brief Button control
*/
class MyButton : public wxButton
{
public:
	MyButton(wxWindow *parent, wxWindowID id,
		CMsg::Id labelid = CMsg::Null,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize, long style = 0,
		const wxValidator& validator = wxDefaultValidator);

	void SetLabelById(CMsg::Id labelid);
};

/**
	@brief Menu control
*/
class MyMenu : public wxMenu
{
public:
	wxMenuItem* AppendById(int itemid,
		CMsg::Id textid,
		const wxString& help = wxEmptyString,
		wxItemKind kind = wxITEM_NORMAL);

	wxMenuItem* AppendCheckItemById(int itemid,
		CMsg::Id textid,
		const wxString& help = wxEmptyString);

	wxMenuItem* AppendRadioItemById(int itemid,
		CMsg::Id textid,
		const wxString& help = wxEmptyString);

	wxMenuItem* AppendSubMenuById(MyMenu *submenu,
		CMsg::Id textid,
		const wxString& help = wxEmptyString);
};

#endif /* WX_DLG_H */
