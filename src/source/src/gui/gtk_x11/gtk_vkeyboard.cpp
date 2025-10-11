/** @file gtk_vkeyboard.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2017.01.21 -

	@brief [ virtual keyboard ]
*/

#include "gtk_vkeyboard.h"

#if defined(USE_GTK_VKEYBOARD)

#include "../gui.h"
#include "../../labels.h"

extern GUI *gui;

namespace Vkbd {

//
// for GTK+
//

VKeyboard::VKeyboard() : OSDBase()
{
	parent = NULL;
	window = NULL;
	drawing = NULL;
//	surface = NULL;
	RECT_IN(reWindow, 0, 0, 0, 0);
	popupMenu = NULL;
}

VKeyboard::~VKeyboard()
{
}

void VKeyboard::Show(bool show)
{
	if (window) {
		Base::Show(show);
		if (show) {
			// show
			gtk_widget_show_all(window);
			calc_client_size();
//			need_update_window();
		} else {
			// hide
			gtk_widget_hide(window);
		}
	}
}

bool VKeyboard::Create(const char *res_path)
{
	if (window) return true;

	if (!load_bitmap(res_path)) {
		closed = true;
		return false;
	}

	parent = gui->GetWindow();

//	int flags = GTK_DIALOG_DESTROY_WITH_PARENT;
//	window = gtk_dialog_new_with_buttons("Virtual Keyboard"
//		, GTK_WINDOW(parent)
//		, (GtkDialogFlags)flags
//		, NULL
//	);
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	if (!window) {
		closed = true;
		return false;
	}

//	gtk_window_set_decorated(GTK_WINDOW(window), TRUE);
	gtk_window_set_title(GTK_WINDOW(window), "Virtual Keyboard");
//	gtk_window_set_type_hint(GTK_WINDOW(window), 1);
//	gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(parent));
//	gtk_window_set_destroy_with_parent(GTK_WINDOW(window), TRUE);
	GdkGeometry geometry;
	geometry.min_width = (gint)((double)pSurface->Width() * 0.25 + 0.5);
	geometry.min_height = (gint)((double)pSurface->Height() * 0.25 + 0.5);
//	geometry.base_width = pSurface->Width();
//	geometry.base_height = pSurface->Height();
//	int hints = (GDK_HINT_MIN_SIZE | GDK_HINT_BASE_SIZE);
//	gtk_window_set_geometry_hints(GTK_WINDOW(window), NULL, &geometry, (GdkWindowHints)hints);
	gtk_window_set_default_size(GTK_WINDOW(window), pSurface->Width(), pSurface->Height());
	reWindow.w = pSurface->Width();
	reWindow.h = pSurface->Height();
	drawing = gtk_drawing_area_new();
	gtk_widget_set_size_request(drawing, geometry.min_width, geometry.min_height);
//	gtk_widget_set_size_request(drawing, pSurface->Width(), pSurface->Height());
	gtk_container_add(GTK_CONTAINER(window), drawing);

	gtk_widget_set_app_paintable(window, TRUE);
	gtk_window_set_resizable(GTK_WINDOW(window), TRUE);

#if !GTK_CHECK_VERSION(3,0,0)
	gint mask = (GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_EXPOSURE_MASK);
	gtk_widget_add_events(window, mask);
#endif

//	surface = cairo_image_surface_create_for_data((unsigned char *)pSurface->GetBuffer()
//		, CAIRO_FORMAT_RGB24
//		, pSurface->Width(), pSurface->Height()
//		, cairo_format_stride_for_width(CAIRO_FORMAT_RGB24, pSurface->Width())
//	);
	surface.CreateC(*pSurface, pSurface->Width(), pSurface->Height());

//	g_signal_connect(G_OBJECT(window), "button-press-event", G_CALLBACK(OnMouseDown), (gpointer)this);
//	g_signal_connect(G_OBJECT(window), "button-release-event", G_CALLBACK(OnMouseUp), (gpointer)this);
	g_signal_connect(G_OBJECT(drawing), "button-press-event", G_CALLBACK(OnMouseDown), (gpointer)this);
	g_signal_connect(G_OBJECT(drawing), "button-release-event", G_CALLBACK(OnMouseUp), (gpointer)this);
	g_signal_connect(G_OBJECT(window), "key-press-event", G_CALLBACK(OnKeyDown), (gpointer)this);
	g_signal_connect(G_OBJECT(window), "key-release-event", G_CALLBACK(OnKeyUp), (gpointer)this);
#if GTK_CHECK_VERSION(3,0,0)
//	g_signal_connect(G_OBJECT(window), "draw", G_CALLBACK(OnDraw), (gpointer)this);
	g_signal_connect(G_OBJECT(drawing), "draw", G_CALLBACK(OnDraw), (gpointer)this);
#else
	g_signal_connect(G_OBJECT(window), "expose-event", G_CALLBACK(OnExpose), (gpointer)this);
#endif
	g_signal_connect(G_OBJECT(window), "delete-event", G_CALLBACK(OnDelete), (gpointer)this);
//	g_signal_connect(G_OBJECT(window), "response", G_CALLBACK(OnResponse), (gpointer)this);
	g_signal_connect(G_OBJECT(window), "size-allocate", G_CALLBACK(OnSizeAllocate), (gpointer)this);

	gtk_widget_set_events(drawing,
			gtk_widget_get_events(drawing) | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);

//	adjust_window_size();
	set_dist();
	closed = false;

	return true;
}

void VKeyboard::Close()
{
#ifdef VKEYBOARD_DEBUG
	fprintf(stderr, "VKeyboard::Close()\n");
#endif

	if (popupMenu) {
		gtk_widget_destroy(popupMenu);
		popupMenu = NULL;
	}

//	if (surface) {
//		cairo_surface_destroy(surface);
//		surface = NULL;
//	}
	surface.ReleaseC();

	if (window) {
		gtk_widget_destroy(window);
		window = NULL;
	}

	unload_bitmap();

	CloseBase();
}

void VKeyboard::set_dist()
{
	if (!window) return;

	gint xp = 0;
	gint yp = 0;
	gint wp = 1;
	gint hp = 1;
	gint w = 1;
	gint h = 1;
	gtk_window_get_position(GTK_WINDOW(parent), &xp, &yp);
	gtk_window_get_size(GTK_WINDOW(parent), &wp, &hp);
	gtk_window_get_default_size(GTK_WINDOW(window), &w, &h);
	if (w < 0 || h < 0) {
		gtk_window_get_size(GTK_WINDOW(window), &w, &h);
	}

	int x = (wp - w) / 2 + xp;
	int y = yp;

	gtk_window_move(GTK_WINDOW(window), x, y);
}

void VKeyboard::changing_size()
{
	calc_client_size();
}

void VKeyboard::need_update_window(PressedInfo_t *info, bool onoff)
{
	if (!window) return;

	need_update_window_base(info, onoff);

	gint x = (gint)(magnify_x * info->re.left);
	gint y = (gint)(magnify_y * info->re.top);
	gint w = (gint)(magnify_x *(info->re.right - info->re.left));
	gint h = (gint)(magnify_y *(info->re.bottom - info->re.top));
//	gtk_widget_queue_draw_area(window
	gtk_widget_queue_draw_area(drawing, x, y, w, h);
}

//void VKeyboard::need_update_window()
//{
//	if (!window) return;
//
//	gtk_widget_queue_draw(window);
//}

void VKeyboard::draw_window(cairo_t *cr)
{
//	GdkRectangle re;
//	gdk_cairo_get_clip_rectangle(cr, &re);
//printf("OnDraw: re:%d:%d:%d:%d\n",re.x,re.y,re.width,re.height);
//	cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);

//	cairo_set_source_surface(cr, surface, 0.0, 0.0);
	surface.StretchBlitC(cr, reWindow, pConfig->filter_type);
	cairo_paint(cr);
}

void VKeyboard::calc_client_size()
{
	gtk_window_get_size(GTK_WINDOW(window), &reWindow.w, &reWindow.h);
	magnify_x = (double)reWindow.w / pSurface->Width();
	magnify_y = (double)reWindow.h / pSurface->Height();
}

void VKeyboard::adjust_size(double mag)
{
	magnify_x = mag;
	magnify_y = mag;
	reWindow.w = (gint)(magnify_x * pSurface->Width() + 0.5);
	reWindow.h = (gint)(magnify_y * pSurface->Height() + 0.5);
	gtk_window_resize(GTK_WINDOW(window), reWindow.w, reWindow.h);
}

void VKeyboard::create_popup_menu()
{
	if (popupMenu) return;
	popupMenu = gtk_menu_new();
	for(int i=0; LABELS::window_size[i].msg_id != CMsg::End; i++) {
		if (LABELS::window_size[i].msg_id != CMsg::Null) {
			GUI::CreateMenuItem(popupMenu, LABELS::window_size[i].msg_id, OnSelectPopupMenu, NULL, 0, i, (gpointer)this);
		} else {
			GUI::CreateSeparatorMenu(popupMenu);
		}
	}
}

void VKeyboard::show_popup_menu(const GdkEvent *event)
{
	if (!popupMenu) {
		create_popup_menu();
	}
	gtk_widget_show_all(popupMenu);
#if GTK_CHECK_VERSION(3,22,0)
	gtk_menu_popup_at_pointer(GTK_MENU(popupMenu), event);
#else
	const GdkEventButton *e = (const GdkEventButton *)event;
	gtk_menu_popup(GTK_MENU(popupMenu), NULL, NULL, NULL, NULL, e->button, e->time);
#endif
}

void VKeyboard::OnSelectPopupMenu(GtkWidget *widget, gpointer user_data)
{
	VKeyboard *obj = (VKeyboard *)user_data;
	int num = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"num");
	obj->adjust_size((double)LABELS::window_size[num].percent / 100.0);
}

gboolean VKeyboard::OnMouseDown(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	GdkEventButton *e = (GdkEventButton *)event;
	VKeyboard *obj = (VKeyboard *)user_data;
	switch(e->button) {
	case 1:
		// left button
		obj->MouseDown(e->x, e->y);
		break;
	case 3:
		// right button
		obj->show_popup_menu(event);
		break;
	default:
		break;
	}
	return TRUE;
}

gboolean VKeyboard::OnMouseUp(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	GdkEventButton *e = (GdkEventButton *)event;
	VKeyboard *obj = (VKeyboard *)user_data;
	switch(e->button) {
	case 1:
		// left button
		obj->MouseUp();
		break;
	default:
		break;
	}
	return TRUE;
}

gboolean VKeyboard::OnKeyDown(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	GdkEventKey *key_event = (GdkEventKey *)event;
	emu->key_down_up(0, (int)key_event->keyval
		, (short)key_event->hardware_keycode);
	return FALSE;
}

gboolean VKeyboard::OnKeyUp(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	GdkEventKey *key_event = (GdkEventKey *)event;
	emu->key_down_up(1, (int)key_event->keyval
		, (short)key_event->hardware_keycode);
	return FALSE;
}

#if GTK_CHECK_VERSION(3,0,0)
gboolean VKeyboard::OnDraw(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
	VKeyboard *obj = (VKeyboard *)user_data;
	obj->draw_window(cr);
	return FALSE;
}
#else
gboolean VKeyboard::OnExpose(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	GdkEventExpose *e = (GdkEventExpose *)event;
	VKeyboard *obj = (VKeyboard *)user_data;
	cairo_t *cr = gdk_cairo_create(widget->window);
	obj->draw_window(cr);
	cairo_destroy(cr);
	return FALSE;
}
#endif

//void VKeyboard::OnResponse(GtkWidget *widget, gint response_id, gpointer user_data)
//{
//	g_print("OnResponse\n");
//	VKeyboard *obj = (VKeyboard *)user_data;
//	obj->Close();
//}

gboolean VKeyboard::OnDelete(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	VKeyboard *obj = (VKeyboard *)user_data;
	obj->Close();
	return FALSE;
}

void VKeyboard::OnSizeAllocate(GtkWidget *widget, GdkRectangle *rect, gpointer user_data)
{
	// rect size is the window included title bar and border etc.
	VKeyboard *obj = (VKeyboard *)user_data;
	obj->changing_size();
}

} /* namespace Vkbd */

#endif /* USE_GTK_VKEYBOARD */

