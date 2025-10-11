/** @file qt_vkeyboard.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2016.12.21 -

	@brief [ virtual keyboard ]
*/

#include "qt_vkeyboard.h"

#ifdef QT_VKEYBOARD_H

#include "../../emu.h"
#include "../../emu_osd.h"
#include <QApplication>
#include <QScreen>
#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QMenu>
#include "../../msgs.h"
#include "../../labels.h"

extern EMU *emu;


namespace Vkbd {

//
// for Qt
//
VKeyboard::VKeyboard(QWidget *parent) : OSDBase()
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

	Base::Show(show);

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
		closed = false;
	}
}

void VKeyboard::Close()
{
	if (!dlg) return;

//	delete dlg;
//	dlg = NULL;

	unload_bitmap();

	CloseBase();
}

void VKeyboard::adjust_window_size()
{
	if (dlg) {
		int w = static_cast<int>(static_cast<double>(pSurface->Width()) * 0.25 + 0.5);
		int h = static_cast<int>(static_cast<double>(pSurface->Height()) * 0.25 + 0.5);
		dlg->setMinimumSize(w, h);
		w = pSurface->Width();
		h = pSurface->Height();
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

	need_update_window_base(info, onoff);
	int x = static_cast<int>(magnify_x * info->re.left + 0.5);
	int y = static_cast<int>(magnify_y * info->re.top + 0.5);
	int w = static_cast<int>(magnify_x * (info->re.right - info->re.left) + 0.5);
	int h = static_cast<int>(magnify_y * (info->re.bottom - info->re.top) + 0.5);
	dlg->repaint(x, y, w, h);
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
//	qp.drawImage(re, *pSurface->Get());
	QRect src_re(
		static_cast<double>(re.left()) / magnify_x + 0.5,
		static_cast<double>(re.top()) / magnify_y + 0.5,
		static_cast<double>(re.width()) / magnify_x + 0.5,
		static_cast<double>(re.height()) / magnify_y + 0.5
	);
	qp.drawImage(re, *pSurface->Get(), src_re);
}

void VKeyboard::changing_size()
{
	if (!dlg) return;
	if (!pSurface) return;

	QSize sz = dlg->size();
	magnify_x = static_cast<double>(sz.width()) / pSurface->Width();
	magnify_y = static_cast<double>(sz.height()) / pSurface->Height();
}

void VKeyboard::change_size(double mag)
{
	if (!pSurface) return;

	magnify_x = mag;
	magnify_y = mag;
	int w = static_cast<int>(magnify_x * pSurface->Width() + 0.5);
	int h = static_cast<int>(magnify_y * pSurface->Height() + 0.5);
	dlg->resize(w, h);
}

//void VKeyboard::init_dialog(HWND hDlg)
//{
//}

} /* namespace Vkbd */

//
//
//
MyVKeyboard::MyVKeyboard(Vkbd::VKeyboard *vkbd, QWidget *parent)
	: QDialog(parent, Qt::Window)
{
	this->vkbd = vkbd;
	this->popupMenu = nullptr;

	setWindowTitle("Virtual Keyboard");
}

MyVKeyboard::~MyVKeyboard()
{
	delete popupMenu;
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
	} else if (event->button() & Qt::RightButton) {
		show_popup_menu(event->x(), event->y());
	}
}

void MyVKeyboard::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() & Qt::LeftButton) {
		vkbd->MouseUp();
	}
}

void MyVKeyboard::keyPressEvent(QKeyEvent *event)
{
	if (event->isAutoRepeat()) return;
	int code = event->key();
	uint32_t vk_key = event->nativeVirtualKey();
	uint32_t scan_code = event->nativeScanCode();
	uint32_t mod = static_cast<uint32_t>(event->modifiers());
	(dynamic_cast<EMU_OSD *>(emu))->key_down_up_(0, code, vk_key, scan_code, mod);
}

void MyVKeyboard::keyReleaseEvent(QKeyEvent *event)
{
	if (event->isAutoRepeat()) return;
	int code = event->key();
	uint32_t vk_key = event->nativeVirtualKey();
	uint32_t scan_code = event->nativeScanCode();
	uint32_t mod = static_cast<uint32_t>(event->modifiers());
	(dynamic_cast<EMU_OSD *>(emu))->key_down_up_(1, code, vk_key, scan_code, mod);
}

void MyVKeyboard::resizeEvent(QResizeEvent *event)
{
	vkbd->changing_size();
	repaint();
}

void MyVKeyboard::create_popup_menu()
{
	if (popupMenu) return;
	QAction *act;
	popupMenu = new QMenu(this);
	for(int i=0; LABELS::window_size[i].msg_id != CMsg::End; i++) {
		if (LABELS::window_size[i].msg_id != CMsg::Null) {
			act = popupMenu->addAction(CMSGV(LABELS::window_size[i].msg_id));
			act->setData(QVariant::fromValue(i));
			QObject::connect(act, SIGNAL(triggered()), this, SLOT(selectMenuItem()));
		} else {
			popupMenu->addSeparator();
		}
	}

}

void MyVKeyboard::show_popup_menu(int x, int y)
{
	if (!popupMenu) {
		create_popup_menu();
	}
	QPoint spos = mapToGlobal(QPoint(x, y));
	popupMenu->popup(spos);
}

void MyVKeyboard::selectMenuItem()
{
	QAction *act = dynamic_cast<QAction *>(sender());
	int num = act->data().toInt();
	vkbd->change_size(static_cast<double>(LABELS::window_size[num].percent) / 100.0);
}

#endif /* QT_VKEYBOARD_H */
