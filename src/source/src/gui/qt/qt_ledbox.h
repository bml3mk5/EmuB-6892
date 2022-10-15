/** @file qt_ledbox.h

	HITACHI BASIC MASTER LEVEL3 Mark5 Emulator 'EmuB-6892'
	Skelton for retropc emulator

	@author Sasaji
	@date   2015.12.21 -

	@brief [ led box ]
*/

#ifndef QT_LEDBOX_H
#define QT_LEDBOX_H

#include "../ledbox.h"
#include <QDialog>

#define NO_TITLEBAR

class MyLedBox;

/**
	@brief LedBox is the window that display the access indicator outside the main window.
*/
class LedBox : public LedBoxBase
{
private:
	VmPoint pStart;
	MyLedBox *dlg;
	QWidget *parent;

	void show_dialog();
	void move_in_place(int place);
	void need_update_dialog();

#ifdef NO_TITLEBAR
	void mouse_press(int x, int y);
	void mouse_move(int x, int y);
#else
	void set_dist();
#endif
	void adjust_dialog_size();
	void update_dialog(QPaintDevice *dev, const QRect &re);

public:
	friend class MyLedBox;

	LedBox();
	~LedBox();

	void CreateDialogBox();
	void Move();
	void SetParent(QWidget *parent);
};

/**
	@brief Led dialog
*/
class MyLedBox : public QDialog
{
	Q_OBJECT

private:
	LedBox *ledbox;

	void paintEvent(QPaintEvent *);
	void mousePressEvent(QMouseEvent *);
	void mouseReleaseEvent(QMouseEvent *);
	void mouseMoveEvent(QMouseEvent *);

public:
	explicit MyLedBox(LedBox *ledbox, QWidget *parent = nullptr);
	~MyLedBox();

	// inherit
//	bool event(QEvent *e);
};

#endif /* QT_LEDBOX_H */
