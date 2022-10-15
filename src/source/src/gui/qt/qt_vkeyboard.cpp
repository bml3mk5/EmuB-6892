/** @file qt_vkeyboard.cpp

	HITACHI BASIC MASTER LEVEL3 Mark5 Emulator 'EmuB-6892'
	Skelton for retropc emulator

	@author Sasaji
	@date   2016.12.21 -

	@brief [ virtual keyboard ]
*/

#include "qt_vkeyboard.h"

#ifdef QT_VKEYBOARD_H

#include "../../emu.h"
#include <QApplication>
#include <QScreen>
#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>

extern EMU *emu;

namespace Vkbd {

//
// for Qt
//
VKeyboard::VKeyboard(QWidget *parent) : Base()
{
	this->parent = parent;
	this->dlg = nullptr;
}

VKeyboard::~VKeyboard()
{
	delete dlg;
}

void VKeyboard::Show(bool show)
{
	if (!dlg) return;

	if (show) dlg->show();
	else dlg->hide();
}

void VKeyboard::Create(const _TCHAR *res_path)
{
	if (dlg) return;

	load_bitmap(res_path);

	dlg = new MyVKeyboard(this, parent);
	if (dlg) {
		adjust_window_size();
		set_dist();
		Base::Create();
	}
}

void VKeyboard::Close()
{
	if (!dlg) return;

//	delete dlg;
//	dlg = NULL;

	unload_bitmap();

	Base::Close();
}

void VKeyboard::adjust_window_size()
{
	if (dlg) {
		int w = pSurface->Width();
		int h = pSurface->Height();

		dlg->resize(w, h);
	}
}

void VKeyboard::set_dist()
{
	if (!dlg) return;

	const QScreen *current_desktop = qApp->screens().at(0);
//	int desktop_width = current_desktop->size().width();
	int desktop_height = current_desktop->size().height();

	int wp = parent->width();
//	int hp = parent->height();
	int w = dlg->width();
	int h = dlg->height();

	int x = (wp - w) / 2 + parent->x();
	int y = parent->y() + parent->frameGeometry().height();

	if (y + h > desktop_height) {
		y = (desktop_height - h);
	}

	dlg->move(x, y);
}

void VKeyboard::need_update_window(PressedInfo_t *info, bool onoff)
{
	if (!dlg) return;

	Base::need_update_window(info, onoff);

	dlg->repaint(info->re.left, info->re.top, info->re.right - info->re.left, info->re.bottom - info->re.top);
}

void VKeyboard::update_window()
{
	if (!dlg) return;

	dlg->update();
}

void VKeyboard::paint_window(QPaintDevice *dev, const QRect &re)
{
	if (!pSurface) return;

	QPainter qp(dev);
	qp.drawImage(re, *pSurface->Get());
}

//void VKeyboard::init_dialog(HWND hDlg)
//{
//}

} /* namespace Vkbd */

//
//
//
MyVKeyboard::MyVKeyboard(Vkbd::VKeyboard *vkbd, QWidget *parent)
	: QDialog(parent, Qt::Dialog)
{
	this->vkbd = vkbd;
}

MyVKeyboard::~MyVKeyboard()
{
	vkbd->dlg = nullptr;
}

void MyVKeyboard::closeEvent(QCloseEvent *UNUSED_PARAM(event))
{
	vkbd->Close();
}

void MyVKeyboard::paintEvent(QPaintEvent *event)
{
	vkbd->paint_window(this, event->rect());
}

void MyVKeyboard::mousePressEvent(QMouseEvent *event)
{
	if (event->button() & Qt::LeftButton) {
		vkbd->MouseDown(event->x(), event->y());
	}
}

void MyVKeyboard::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() & Qt::LeftButton) {
		vkbd->MouseUp();
	}
}

#endif /* QT_VKEYBOARD_H */
