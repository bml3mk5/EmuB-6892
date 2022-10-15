/** @file gtk_keybindbox.h

	HITACHI BASIC MASTER LEVEL3 Mark5 / MB-S1 Emulator 'EmuB-6892/EmuB-S1'
	Skelton for retropc emulator

	@author Sasaji
	@date   2015.05.10 -

	@brief [ keybind box ]
*/

#ifndef GUI_GTK_KEYBINDBOX_H
#define GUI_GTK_KEYBINDBOX_H

#include <gtk/gtk.h>
#include "gtk_dialogbox.h"
#include "../gui_keybinddata.h"
#include "../../vm/vm.h"

namespace GUI_GTK_X11
{

/**
	@brief Keybind dialog box
*/
class KeybindBox : public DialogBox
{
private:
	KeybindData *kbdata[KEYBIND_MAX_NUM];
	GtkWidget *grid[KEYBIND_MAX_NUM];
	GtkWidget *cells[KEYBIND_MAX_NUM][KBCTRL_MAX_LINES][KBCTRL_MAX_COLS];
	GtkWidget *chkCombi[2];

	void Update();

	bool SetData();

	bool SetKeyCode(int tab, int row, int col, int code, int scancode, char *label);
	void LoadPreset(int tab, int idx);
	void SavePreset(int tab, int idx);

	static gboolean OnKeyDown(GtkWidget *widget, GdkEvent  *event, gpointer user_data);
	static gboolean OnDoubleClick(GtkWidget *widget, GdkEvent  *event, gpointer user_data);
	static void OnClickLoadDefault(GtkButton *button, gpointer user_data);
	static void OnClickLoadPreset(GtkButton *button, gpointer user_data);
	static void OnClickSavePreset(GtkButton *button, gpointer user_data);
	static void OnResponse(GtkWidget *widget, gint response_id, gpointer user_data);

public:
	KeybindBox(GUI *new_gui);
	~KeybindBox();
	bool Show(GtkWidget *parent_window);
	void Hide();

};

}; /* namespace GUI_GTK_X11 */

#endif /* GUI_GTK_KEYBINDBOX_H */
