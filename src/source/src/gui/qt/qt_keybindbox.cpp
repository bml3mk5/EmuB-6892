/** @file qt_keybindbox.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2016.03.01

	@brief [ qt keybind box ]
*/

#include "qt_keybindbox.h"
#include "ui_qt_keybindbox.h"
#include "qt_dialog.h"
#include "../../emu_osd.h"
#include <QKeyEvent>

extern EMU *emu;

MyKeybindBox::MyKeybindBox(QWidget *parent) :
	QDialog(parent)
//	ui(new Ui::MyKeybindBox)
{
	setWindowTitle(CMSG(Keybind));

	int tab;

//	ui->setupUi(this);

	for(tab=0; tab<KEYBIND_MAX_NUM; tab++) {
		tables[tab] = nullptr;
		kbdata[tab] = new KeybindData();
	}

	QVBoxLayout *vbox_all = new QVBoxLayout(this);
	QHBoxLayout *hbox = new QHBoxLayout();
	vbox_all->addLayout(hbox);
	QVBoxLayout *vbox_tab = new QVBoxLayout();
	hbox->addLayout(vbox_tab);
	QVBoxLayout *vbox_btn = new QVBoxLayout();
	hbox->addLayout(vbox_btn);

	MyTabWidget *tabWidget = new MyTabWidget();
	vbox_tab->addWidget(tabWidget);


	curr_tab = 0;
	for(tab=0; tab<KEYBIND_MAX_NUM; tab++) {
		QWidget *titmWidget = new QWidget();
		QVBoxLayout *vbox = new QVBoxLayout(titmWidget);
		tables[tab] = new MyTableWidget(tab, kbdata[tab]);
		tables[tab]->setMinimumSize(400, 400);
		vbox->addWidget(tables[tab]);
		tabWidget->addTab(titmWidget, tab == 1 ? CMsg::Joypad_Key_Assigned : (tab == 2 ? CMsg::Joypad_PIA_Type : CMsg::Keyboard));
	}

	// right button
	char label[128];

	MyPushButton *btn_d = new MyPushButton(CMsg::Load_Default);
	vbox_btn->addWidget(btn_d);
	connect(btn_d, SIGNAL(released()), this, SLOT(loadPreset()));
	btn_d->setProperty("num", QVariant::fromValue(-1));
	QWidget *spc1 = new QWidget();
	spc1->setMinimumHeight(32);
	vbox_btn->addWidget(spc1);
	for(int i=0; i<4; i++) {
		gMessages.Sprintf(label, 128, CMsg::Load_Preset_VDIGIT, i+1);
		QPushButton *btn_lp = new QPushButton(label);
		vbox_btn->addWidget(btn_lp);
		connect(btn_lp, SIGNAL(released()), this, SLOT(loadPreset()));
		btn_lp->setProperty("num", QVariant::fromValue(i));
	}
	spc1 = new QWidget();
	spc1->setMinimumHeight(32);
	vbox_btn->addWidget(spc1);
	for(int i=0; i<4; i++) {
		gMessages.Sprintf(label, 128, CMsg::Save_Preset_VDIGIT, i+1);
		QPushButton *btn_sp = new QPushButton(label);
		vbox_btn->addWidget(btn_sp);
		connect(btn_sp, SIGNAL(released()), this, SLOT(savePreset()));
		btn_sp->setProperty("num", QVariant::fromValue(i));
	}
	QSpacerItem *spc = new QSpacerItem(1,1,QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
	vbox_btn->addSpacerItem(spc);

	QDialogButtonBox *btn = new QDialogButtonBox(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
	vbox_all->addWidget(btn, Qt::AlignRight);
	connect(btn, SIGNAL(accepted()), this, SLOT(accept()));
	connect(btn, SIGNAL(rejected()), this, SLOT(reject()));
}

MyKeybindBox::~MyKeybindBox()
{
	for(int tab=0; tab<KEYBIND_MAX_NUM; tab++) {
		delete kbdata[tab];
	}

//	delete ui;
}

void MyKeybindBox::setData()
{
	for(int tab=0; tab<KEYBIND_MAX_NUM; tab++) {
		kbdata[tab]->SetData();
	}

	emu->save_keybind();
}

void MyKeybindBox::accept()
{
	setData();

	QDialog::accept();
}

void MyKeybindBox::update()
{
	if (tables[curr_tab]) {
		tables[curr_tab]->update();
	}

	QDialog::update();
}

void MyKeybindBox::loadPreset()
{
	int num = sender()->property("num").toInt();

	kbdata[curr_tab]->LoadPreset(num);
	update();
}

void MyKeybindBox::savePreset()
{
	int num = sender()->property("num").toInt();

	kbdata[curr_tab]->SavePreset(num);

	update();
}

//

MyTableWidget::MyTableWidget(int tab, KeybindData *data, QWidget *parent)
 : QTableWidget(parent)
{
	kbdata = data;
	tab_num = tab;

#ifdef USE_PIAJOYSTICKBIT
	kbdata->Init(emu, tab, tab == 0 ? 0 : 1, tab == 2 ? 2 : 0);
#else
	kbdata->Init(emu, tab, tab == 0 ? 0 : 1, tab == 2 ? 1 : 0);
#endif

	_TCHAR label[128];

	int rows = kbdata->GetNumberOfRows();
	int cols = kbdata->GetNumberOfColumns();

	setColumnCount(cols);
	for(int col=0; col<cols; col++) {
		QString qlabel = tr("Key%1").arg(col + 1);
		QTableWidgetItem *itm = new QTableWidgetItem(qlabel);
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

/// override keyevent on table
void MyTableWidget::keyPressEvent(QKeyEvent *event)
{
	_TCHAR label[128];

//	if (event->isAutoRepeat()) return;
    int code = event->key();
    uint32_t vk_key = event->nativeVirtualKey();
    uint32_t scan_code = event->nativeScanCode();
	uint32_t mod = static_cast<uint32_t>(event->modifiers());
	(dynamic_cast<EMU_OSD *>(emu))->translate_keysym_(0, code, vk_key, scan_code, mod, &code);

	QTableWidgetItem *itm = currentItem();
	if (!itm) return;
	if (itm->row() < 0 || itm->column() < 0) return;

	kbdata->SetVkCode(itm->row(), itm->column(), static_cast<uint32_t>(code), label);
	itm->setText(label);
}

//void MyTableWidget::keyReleaseEvent(QKeyEvent *event)
//{
//}

void MyTableWidget::cellDoubleClick(int row, int column)
{
	_TCHAR label[128];

	QTableWidgetItem *itm = this->item(row, column);
	if (!itm) return;

	kbdata->SetVkCode(itm->row(), itm->column(), 0, label);
	itm->setText(label);
}
