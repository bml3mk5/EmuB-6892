/** @file gtk_keybindbox.cpp

	HITACHI BASIC MASTER LEVEL3 Mark5 / MB-S1 Emulator 'EmuB-6892/EmuB-S1'
	Skelton for retropc emulator

	@author Sasaji
	@date   2015.05.10 -

	@brief [ keybind box ]
*/

#include <gdk/gdkx.h>
#include "gtk_keybindbox.h"
#include "gtk_x11_key_trans.h"
#include "../../emu.h"
#include "../../config.h"
#include "../gui_base.h"
#include "../../clocale.h"
#include "../../utility.h"
#include "../../msgs.h"

extern EMU *emu;

namespace GUI_GTK_X11
{

//
//
//
KeybindBox::KeybindBox(GUI *new_gui) : DialogBox(new_gui)
{
	for(int tab=0; tab < KEYBIND_MAX_NUM; tab++) {
		kbdata[tab] = new KeybindData();
		grid[tab] = NULL;
	}
	memset(cells, 0, sizeof(cells));
}

KeybindBox::~KeybindBox()
{
	for(int tab=0; tab < KEYBIND_MAX_NUM; tab++) {
		delete kbdata[tab];
	}
}

bool KeybindBox::Show(GtkWidget *parent_window)
{
	DialogBox::Show(parent_window);

	if (dialog) return true;
#ifndef USE_GTK
	X11_InitKeymap();
#endif
	create_dialog(window, CMsg::Keybind);
	add_accept_button(CMsg::OK);
	add_reject_button(CMsg::Cancel);

	GtkWidget *cont = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	GtkWidget *nb;
	GtkWidget *vboxall;
	GtkWidget *hboxall;
	GtkWidget *scroll;
	GtkWidget *vbox;
	GtkWidget *btn;
	GtkWidget *cell;

	char buf[128];

	const CMsg::Id t[] = {
		CMsg::Keyboard,
		CMsg::Joypad_Key_Assigned,
		CMsg::Joypad_PIA_Type,
		CMsg::End
	};

	// create notebook tab
	nb = create_notebook(cont);

	for(int tab=0; tab < KEYBIND_MAX_NUM; tab++) {
#ifdef USE_PIAJOYSTICKBIT
		kbdata[tab]->Init(emu, tab, tab == 0 ? 0 : 1, tab == 2 ? 2 : 0);
#else
		kbdata[tab]->Init(emu, tab, tab == 0 ? 0 : 1, tab == 2 ? 1 : 0);
#endif

		vboxall = create_vbox(NULL);
		add_note(nb, vboxall, t[tab]);
		hboxall = create_hbox(vboxall);

		scroll = create_scroll_win(hboxall, 360, 300);
		grid[tab] = create_grid(scroll);

#ifdef _MBS1
		attach_to_grid(grid[tab], create_label(NULL, tab == 2 ? CMsg::PIA_on_S1 : CMsg::S1_Key), 0, 0);
#else
		attach_to_grid(grid[tab], create_label(NULL, tab == 2 ? CMsg::PIA_on_L3 : CMsg::Level3_Key), 0, 0);
#endif
		for(int col=1; col < 3; col++) {
			if (tab != 0) sprintf(buf, CMSG(JoypadVDIGIT), col);
			else sprintf(buf, CMSG(BindVDIGIT), col);
			attach_to_grid(grid[tab], create_label(NULL, buf), col, 0);
		}

		for(int row=0; row < kbdata[tab]->GetNumberOfRows(); row++) {
			for(int col=0; col < 3; col++) {
				if (col == 0) {
					cell = create_label(NULL, kbdata[tab]->GetCellString(row, -1));
				} else {
					cell = create_text(NULL, kbdata[tab]->GetCellString(row, col-1), 3);
					g_object_set_data(G_OBJECT(cell), "tab", (gpointer)(intptr_t)tab);
					g_object_set_data(G_OBJECT(cell), "row", (gpointer)(intptr_t)row);
					g_object_set_data(G_OBJECT(cell), "col", (gpointer)(intptr_t)(col-1));
					g_signal_connect(G_OBJECT(cell), "key-press-event", G_CALLBACK(OnKeyDown), (gpointer)this);
					g_signal_connect(G_OBJECT(cell), "button-press-event", G_CALLBACK(OnDoubleClick), (gpointer)this);
					cells[tab][row][col-1]=cell;
				}
				attach_to_grid(grid[tab], cell, col, row + 1);
			}
		}
		vbox = create_vbox(hboxall);
		btn = create_button(vbox, CMsg::Load_Default, G_CALLBACK(OnClickLoadDefault));
		g_object_set_data(G_OBJECT(btn), "tab", (gpointer)(intptr_t)tab);
		create_label(vbox, "");
		for(int i=0; i<KEYBIND_PRESETS; i++) {
			sprintf(buf, CMSG(Load_Preset_VDIGIT), i+1);
			btn = create_button(vbox, buf, G_CALLBACK(OnClickLoadPreset));
			g_object_set_data(G_OBJECT(btn), "tab", (gpointer)(intptr_t)tab);
			g_object_set_data(G_OBJECT(btn), "num", (gpointer)(intptr_t)i);
		}
		create_label(vbox, "");
		for(int i=0; i<KEYBIND_PRESETS; i++) {
			sprintf(buf, CMSG(Save_Preset_VDIGIT), i+1);
			btn = create_button(vbox, buf, G_CALLBACK(OnClickSavePreset));
			g_object_set_data(G_OBJECT(btn), "tab", (gpointer)(intptr_t)tab);
			g_object_set_data(G_OBJECT(btn), "num", (gpointer)(intptr_t)i);
		}

		if (tab == 2) {
			chkCombi[1] = create_check_box(vboxall, CMsg::Signals_are_negative_logic, kbdata[tab]->GetCombi() != 0);
		} else if (tab == 1) {
			chkCombi[0] = create_check_box(vboxall, CMsg::Recognize_as_another_key_when_pressed_two_buttons, kbdata[tab]->GetCombi() != 0);
		}
	}

	//

	gtk_widget_show_all(dialog);
	g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(OnResponse), (gpointer)this);
//	gint rc = gtk_dialog_run(GTK_DIALOG(dialog));

	emu->set_pause(1, true);

	return true;
}

void KeybindBox::Hide()
{
	DialogBox::Hide();
	for(int tab=0; tab < KEYBIND_MAX_NUM; tab++) {
		grid[tab] = NULL;
	}
	memset(cells, 0, sizeof(cells));
	emu->set_pause(1, false);
}

void KeybindBox::Update()
{
	GtkWidget *entry;
	for(int tab=0; tab < KEYBIND_MAX_NUM; tab++) {
		for(int row=0; row < kbdata[tab]->GetNumberOfRows(); row++) {
			for(int col=0; col < 2; col++) {
				entry = cells[tab][row][col];
				set_text(entry, kbdata[tab]->GetCellString(row, col));
			}
		}
		if (tab == 1 || tab == 2) {
			set_check_state(chkCombi[tab-1], kbdata[tab]->GetCombi() != 0);
		}
	}
}

bool KeybindBox::SetData()
{
	for(int tab=0; tab<KEYBIND_MAX_NUM; tab++) {
		kbdata[tab]->SetData();
		if (tab == 1 || tab == 2) {
			kbdata[tab]->SetCombi(get_check_state(chkCombi[tab-1]) ? 1 : 0);
		}
	}

	emu->save_keybind();

	return true;
}

bool KeybindBox::SetKeyCode(int tab, int row, int col, int code, int scancode, char *label)
{
	if (!kbdata[tab]) return false;
#ifndef USE_GTK
	Display *display = gdk_x11_display_get_xdisplay(gdk_display_get_default());
	code = X11_TranslateKeycode(display, scancode);
#endif
	emu->translate_keysym(0,code,(short)scancode,&code);
	return kbdata[tab]->SetVkKeyCode(row, col, code, label);
}

void KeybindBox::LoadPreset(int tab, int idx)
{
	if (!kbdata[tab]) return;
	kbdata[tab]->LoadPreset(idx);
	Update();
}

void KeybindBox::SavePreset(int tab, int idx)
{
	if (!kbdata[tab]) return;
	kbdata[tab]->SavePreset(idx);
}

gboolean KeybindBox::OnKeyDown(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	KeybindBox *obj = (KeybindBox *)user_data;
	if (event->type != GDK_KEY_PRESS) return FALSE;
	char label[128];
	int tab = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"tab");
	int row = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"row");
	int col = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"col");
	GdkEventKey *key_event = (GdkEventKey *)event;
	int code = (int)key_event->keyval;
	int scancode =  key_event->hardware_keycode;
	obj->SetKeyCode(tab, row, col, code, scancode, label);
	gtk_entry_set_text(GTK_ENTRY(widget), label);
	return TRUE;
}

gboolean KeybindBox::OnDoubleClick(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	KeybindBox *obj = (KeybindBox *)user_data;
	if (event->type != GDK_2BUTTON_PRESS) return FALSE;
	char label[128];
	int tab = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"tab");
	int row = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"row");
	int col = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"col");
	obj->SetKeyCode(tab, row, col, 0, 0, label);
	gtk_entry_set_text(GTK_ENTRY(widget), label);
	return TRUE;
}

void KeybindBox::OnClickLoadDefault(GtkButton *button, gpointer user_data)
{
	KeybindBox *obj = (KeybindBox *)user_data;
	int tab = (int)(intptr_t)g_object_get_data(G_OBJECT(button),"tab");
	obj->LoadPreset(tab, -1);
}

void KeybindBox::OnClickLoadPreset(GtkButton *button, gpointer user_data)
{
	KeybindBox *obj = (KeybindBox *)user_data;
	int tab = (int)(intptr_t)g_object_get_data(G_OBJECT(button),"tab");
	int num = (int)(intptr_t)g_object_get_data(G_OBJECT(button),"num");
	obj->LoadPreset(tab, num);
}

void KeybindBox::OnClickSavePreset(GtkButton *button, gpointer user_data)
{
	KeybindBox *obj = (KeybindBox *)user_data;
	int tab = (int)(intptr_t)g_object_get_data(G_OBJECT(button),"tab");
	int num = (int)(intptr_t)g_object_get_data(G_OBJECT(button),"num");
	obj->SavePreset(tab, num);
}

void KeybindBox::OnResponse(GtkWidget *widget, gint response_id, gpointer user_data)
{
	KeybindBox *obj = (KeybindBox *)user_data;
	if (response_id == GTK_RESPONSE_ACCEPT) {
		obj->SetData();
	}
//	g_print("OnResponse: %d\n",response_id);
	obj->Hide();
}

}; /* namespace GUI_GTK_X11 */


