/** @file gtk_vkeyboard.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2017.01.21 -

	@brief [ virtual keyboard ]
*/

#ifndef GTK_VKEYBOARD_H
#define GTK_VKEYBOARD_H

#include "../vkeyboard.h"

#if defined(USE_GTK_VKEYBOARD)

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>

namespace Vkbd {

/**
	@brief Virtual keyboard
*/
class VKeyboard : public OSDBase
{
private:
	GtkWidget	*parent;
	GtkWidget	*window;
	GtkWidget   *drawing;
	CCairoSurface surface;
	VmRectWH	reWindow;
	GtkWidget *popupMenu;

	void set_dist();
	void changing_size();

	void need_update_window(PressedInfo_t *, bool);
//	void need_update_window();
	void draw_window(cairo_t *);
	void calc_client_size();
	void adjust_size(double mag);

	void create_popup_menu();
	void show_popup_menu(const GdkEvent *);

	static gboolean OnMouseDown(GtkWidget *, GdkEvent *, gpointer);
	static gboolean OnMouseUp(GtkWidget *, GdkEvent *, gpointer);
	static gboolean OnKeyDown(GtkWidget *, GdkEvent *, gpointer);
	static gboolean OnKeyUp(GtkWidget *, GdkEvent *, gpointer);
#if GTK_CHECK_VERSION(3,0,0)
	static gboolean OnDraw(GtkWidget *, cairo_t *, gpointer);
#else
	static gboolean OnExpose(GtkWidget *, GdkEvent *, gpointer);
#endif
	static gboolean OnDelete(GtkWidget *, GdkEvent *, gpointer);
//	static void OnResponse(GtkWidget *, gint, gpointer);
	static void OnSizeAllocate(GtkWidget *, GdkRectangle *, gpointer);

	static void OnSelectPopupMenu(GtkWidget *widget, gpointer user_data);

public:
	VKeyboard();
	~VKeyboard();

	bool Create(const char *);
	void Show(bool = true);
	void Close();

	void ProcessEvent();
	void PostProcessEvent();

};

} /* namespace Vkbd */

#endif /* USE_GTK_VKEYBOARD */
#endif /* GTK_VKEYBOARD_H */

