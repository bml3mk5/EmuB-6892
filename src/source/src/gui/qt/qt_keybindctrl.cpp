/** @file qt_keybindctrl.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2024.02.04

	@brief [ qt keybind control ]
*/

#include "qt_keybindctrl.h"
#include "qt_dialog.h"
#include "../../emu_osd.h"
#include "../../labels.h"
#include "../../utility.h"
//#include "../../keycode.h"
#include <QLayout>
#include <QKeyEvent>
#include <QTimer>

extern EMU *emu;

MyTableWidget::MyTableWidget(int tab_num, QWidget *parent)
 : QTableWidget(parent)
{
	m_tab_num = tab_num;

	timer = nullptr;
	chkCombi = nullptr;
	kbdata = new KeybindData();
	kbdata->Init(emu, m_tab_num);

	_TCHAR label[128];

	int rows = kbdata->GetNumberOfRows();
	int cols = kbdata->GetNumberOfColumns();

	setColumnCount(cols);
	for(int col=0; col<cols; col++) {
		UTILITY::stprintf(label, 128, CMSGV(LABELS::keybind_col[m_tab_num][1]), col + 1);
		QTableWidgetItem *itm = new QTableWidgetItem(label);
		setHorizontalHeaderItem(col, itm);
	}

	setRowCount(rows);
	for(int row=0; row<rows; row++) {
		kbdata->GetCellString(row, -1, label);
		QTableWidgetItem *itm = new QTableWidgetItem(label);
		setVerticalHeaderItem(row, itm);

		for(int col=0; col<cols; col++) {
			kbdata->GetCellString(row, col, label);
			QTableWidgetItem *itm = new QTableWidgetItem(label);
			itm->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
			setItem(row, col, itm);
		}
	}

	connect(this, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(cellDoubleClick(int, int)));

	if (kbdata->m_devtype == KeybindData::DEVTYPE_JOYPAD) {
		timer = new QTimer();
		connect(timer, SIGNAL(timeout()), this, SLOT(updateJoy()));
		timer->setSingleShot(false);
		timer->setInterval(100);
		timer->start();
	}
}

MyTableWidget::~MyTableWidget()
{
	delete timer;
	delete kbdata;
}

void MyTableWidget::setData()
{
	setCombiCheckData();
	kbdata->SetData();
}

void MyTableWidget::loadPreset(int num)
{
	kbdata->LoadPreset(num);
	updateCombiCheckButton();
}

void MyTableWidget::savePreset(int num)
{
	setCombiCheckData();
	kbdata->SavePreset(num);
}

void MyTableWidget::update()
{
	_TCHAR label[128];

	if (!kbdata) return;

	int rows = kbdata->GetNumberOfRows();
	int cols = kbdata->GetNumberOfColumns();
	for(int row=0; row<rows; row++) {
		for(int col=0; col<cols; col++) {
			kbdata->GetCellString(row, col, label);
			QTableWidgetItem *itm = item(row, col);
			if (itm) {
				itm->setText(label);
			}
		}
	}

	QTableWidget::update();
}

void MyTableWidget::updateJoy()
{
	QTableWidgetItem *itm = currentItem();
	if (!itm) return;
	emu->update_joystick();
	int col = itm->column();
	uint32_t *joy_stat = emu->joy_real_buffer(col);
	uint32_t joy_mask = ~0;
	if ((joy_stat[0] & joy_mask) | joy_stat[1]) {
		setJoyCell(itm, joy_stat[0], joy_stat[1]);
	}
}

MyCheckBox *MyTableWidget::addCombiCheckButton(QLayout *layout)
{
	if (LABELS::keybind_combi[m_tab_num] != CMsg::Null) {
		chkCombi = new MyCheckBox(LABELS::keybind_combi[m_tab_num]);
		layout->addWidget(chkCombi);
		chkCombi->setChecked(kbdata->GetCombi() != 0);
	}
	return chkCombi;
}

void MyTableWidget::setCombiCheckData()
{
	if (chkCombi) {
		kbdata->SetCombi(chkCombi->isChecked() ? 1 : 0);
	}
}

void MyTableWidget::updateCombiCheckButton()
{
	if (chkCombi) {
		chkCombi->setChecked(kbdata->GetCombi() != 0);
	}
}

void MyTableWidget::clearCellByVkCode(int code)
{
	if (kbdata->m_flags == KeybindData::FLAG_DENY_DUPLICATE) {
		_TCHAR label[128];
		int row;
		int col;
		bool rc = kbdata->ClearCellByVkKeyCode(code, label, &row, &col);
		if (rc) {
			QTableWidgetItem *itm = item(row, col);
			if (itm) {
				itm->setText(label);
			}
		}
	}
}

void MyTableWidget::clearCell(QTableWidgetItem *itm)
{
	_TCHAR label[128];
	bool rc = kbdata->ClearVkCode(itm->row(), itm->column(), label);
	if (rc) {
		itm->setText(label);
	}
}

void MyTableWidget::setKeyCell(QTableWidgetItem *itm, int code)
{
	_TCHAR label[128];
	bool rc = kbdata->SetVkCode(itm->row(), itm->column(), code, label);
	if (rc) {
		itm->setText(label);
	}
}

void MyTableWidget::setJoyCell(QTableWidgetItem *itm, uint32_t code0, uint32_t code1)
{
	_TCHAR label[128];
	bool rc = kbdata->SetVkJoyCode(itm->row(), itm->column(), code0, code1, label);
	if (rc) {
		itm->setText(label);
	}
}

/// override keyevent on table
void MyTableWidget::keyPressEvent(QKeyEvent *event)
{
//	if (m_tab_num != 0) return;
	if (kbdata->m_devtype == KeybindData::DEVTYPE_JOYPAD) return;

	int code = event->key();
	uint32_t vk_key = event->nativeVirtualKey();
	uint32_t scan_code = event->nativeScanCode();
	uint32_t mod = static_cast<uint32_t>(event->modifiers());
	(dynamic_cast<EMU_OSD *>(emu))->translate_keysym_(0, code, vk_key, scan_code, mod, &code);

	QTableWidgetItem *itm = currentItem();
	if (!itm) return;
	if (itm->row() < 0 || itm->column() < 0) return;
	clearCell(itm);
	clearCellByVkCode(code);
	setKeyCell(itm, code);
}

//void MyTableWidget::keyReleaseEvent(QKeyEvent *event)
//{
//}

void MyTableWidget::cellDoubleClick(int row, int column)
{
	QTableWidgetItem *itm = this->item(row, column);
	if (!itm) return;

	clearCell(itm);
}
