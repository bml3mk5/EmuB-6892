/** @file qt_ledbox.cpp

	HITACHI BASIC MASTER LEVEL3 Mark5 Emulator 'EmuB-6892'
	Skelton for retropc emulator

	@author Sasaji
	@date   2015.12.21 -

	@brief [ led box ]
*/

#if defined(USE_QT)

#include "qt_ledbox.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QMainWindow>

//
// for Qt
//
LedBox::LedBox() : LedBoxBase()
{
	dlg = nullptr;
	parent = nullptr;
	pStart.x = 0;
	pStart.y = 0;
}

LedBox::~LedBox()
{
	delete dlg;
}

void LedBox::show_dialog()
{
	if (dlg) {
		if (visible && !inside) dlg->show();
		else dlg->hide();
	}
}

void LedBox::CreateDialogBox()
{
	if (!enable) return;

	dlg = new MyLedBox(this, parent);
	if (dlg) {
		adjust_dialog_size();
	}
}

void LedBox::SetParent(QWidget *parent)
{
	this->parent = parent;
}

void LedBox::Move()
{
	if (dlg) {
		int x = parent->x() + dist.x;
		int y = parent->y() + dist.y;
		dlg->move(x, y);
	}
}

void LedBox::move_in_place(int place)
{
	if (dlg) {
//		QDesktopWidget *desk = qApp->desktop();
//		QRect rf = desk->availableGeometry();
		int w = dlg->width();
		int h = dlg->height();
		QRect frame = parent->frameGeometry();
		if (win_pt.place != place) {
			if (place & 1) {
				dist.x = frame.width() - w;
			} else {
				dist.x = 0;
			}
			if (place & 2) {
				dist.y = frame.height() - (place & 0x10 ? h : 4);
			} else {
				dist.y = 0 - (place & 0x10 ? 0 : h - 4);
			}
		}
		int x = parent->x() + dist.x;
		int y = parent->y() + dist.y;
//		if (y < 0) y = 0;
//		if (y + h > rf.height()) y = rf.height() - h;
		dlg->move(x, y);
	}
}

#ifdef NO_TITLEBAR
void LedBox::mouse_press(int x, int y)
{
	pStart.x = x;
	pStart.y = y;
}

void LedBox::mouse_move(int x, int y)
{
	if (dlg) {
		int px = dlg->parentWidget()->x();
		int py = dlg->parentWidget()->y();
		int wx = dlg->x();
		int wy = dlg->y();

		int nx = x - pStart.x + wx;
		int ny = y - pStart.y + wy;
		dist.x = wx - px;
		dist.y = wy - py;

		dlg->move(nx, ny);
	}
}
#endif

#ifndef NO_TITLEBAR
void LedBox::set_dist()
{
	WINDOWINFO wi;
	WINDOWINFO wid;

	GetWindowInfo(hParent, &wi);
	GetWindowInfo(hDlg, &wid);
	dist.x = wid.rcWindow.left - wi.rcWindow.left;
	dist.y = wid.rcWindow.top - wi.rcWindow.top;
}
#endif

void LedBox::adjust_dialog_size()
{
	if (dlg) {
		dlg->resize(Width() + 2, Height() + 2);
	}
}

void LedBox::need_update_dialog()
{
	if (dlg) {
		dlg->repaint();
	}
}

void LedBox::update_dialog(QPaintDevice *dev, const QRect &UNUSED_PARAM(re))
{
	QPainter qp(dev);
	qp.setPen(QColor(0,0,0));
	qp.drawImage(1, 1, *suf);
	qp.drawRect(0,0,Width()+1,Height()+1);
}

//
//
//
MyLedBox::MyLedBox(LedBox *ledbox, QWidget *parent)
	: QDialog(parent, Qt::Tool | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint)
{
	this->ledbox = ledbox;
}

MyLedBox::~MyLedBox()
{
}

void MyLedBox::paintEvent(QPaintEvent *event)
{
	ledbox->update_dialog(this, event->rect());
}

void MyLedBox::mousePressEvent(QMouseEvent *event)
{
#ifdef NO_TITLEBAR
	if (event->button() & Qt::LeftButton) {
		ledbox->mouse_press(event->x(), event->y());
	}
#endif
}

void MyLedBox::mouseReleaseEvent(QMouseEvent *)
{
	QMainWindow *w = dynamic_cast<QMainWindow *>(parent());
	w->activateWindow();
}

void MyLedBox::mouseMoveEvent(QMouseEvent *event)
{
#ifdef NO_TITLEBAR
	ledbox->mouse_move(event->x(), event->y());
#else
	ledbox->set_dist();
#endif
}

#endif
