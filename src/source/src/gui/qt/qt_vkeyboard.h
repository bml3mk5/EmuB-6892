/** @file qt_vkeyboard.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2016.12.21 -

	@brief [ virtual keyboard ]
*/

#ifndef QT_VKEYBOARD_H
#define QT_VKEYBOARD_H

#include "../../res/resource.h"
#include "../vkeyboard.h"
#include <QDialog>

class MyVKeyboard;
class QMenu;

namespace Vkbd {

/**
	@brief Virtual keyboard
*/
class VKeyboard : public OSDBase
{
    friend class ::MyVKeyboard;

private:
	QWidget *parent;
	MyVKeyboard *dlg;

	void adjust_window_size();
	void set_dist();
	void need_update_window(PressedInfo_t *, bool);
	void update_window();
//	void init_dialog(HWND);
	void paint_window(QPaintDevice *, const QRect &);
	void changing_size();
	void change_size(double mag);

public:
	VKeyboard(QWidget *);
	~VKeyboard();

	void Create(const _TCHAR *res_path);
	void Show(bool = true);
	void Close();
};

} /* namespace Vkbd */

/**
	@brief Virtual keyboard window
*/
class MyVKeyboard : public QDialog
{
	Q_OBJECT

private:
	Vkbd::VKeyboard *vkbd;
	QMenu *popupMenu;

	void closeEvent(QCloseEvent *);
	void paintEvent(QPaintEvent *);
	void mousePressEvent(QMouseEvent *);
	void mouseReleaseEvent(QMouseEvent *);
	void keyPressEvent(QKeyEvent *);
	void keyReleaseEvent(QKeyEvent *);
	void resizeEvent(QResizeEvent *);

	void create_popup_menu();
	void show_popup_menu(int x, int y);

private slots:
	void selectMenuItem();

public:
	explicit MyVKeyboard(Vkbd::VKeyboard *vkbd, QWidget *parent = nullptr);
	~MyVKeyboard();

	// inherit
//	bool event(QEvent *e);
};

#endif /* QT_VKEYBOARD_H */
