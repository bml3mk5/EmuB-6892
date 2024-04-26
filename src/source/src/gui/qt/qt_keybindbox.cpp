/** @file qt_keybindbox.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2016.03.01

	@brief [ qt keybind box ]
*/

#include "qt_keybindbox.h"
#include "ui_qt_keybindbox.h"
#include "qt_dialog.h"
//#include "../../emu_osd.h"
#include "../../labels.h"
//#include "../../utility.h"
#include "../../keycode.h"
#include <QKeyEvent>
#include <QTimer>

extern EMU *emu;

MyKeybindBox::MyKeybindBox(QWidget *parent) :
	QDialog(parent)
//	ui(new Ui::MyKeybindBox)
{
	setWindowTitle(CMSG(Keybind));

	int tab_num;

//	ui->setupUi(this);

	for(tab_num=0; tab_num<KeybindData::TABS_MAX; tab_num++) {
		tables.push_back(new MyTableWidget(tab_num));
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
	for(tab_num=0; tab_num<(int)tables.size(); tab_num++) {
		QWidget *titmWidget = new QWidget();
		QVBoxLayout *vbox = new QVBoxLayout(titmWidget);
		tables[tab_num]->setMinimumSize(400, 400);
		vbox->addWidget(tables[tab_num]);
		tabWidget->addTab(titmWidget, LABELS::keybind_tab[tab_num]);
	}
	connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));

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

	//

	joy_mask = ~0;
	hbox = new QHBoxLayout();
	vbox_all->addLayout(hbox);
	MyCheckBox *chk;
	chk = new MyCheckBox(CMsg::Enable_Z_axis);
	connect(chk, SIGNAL(toggled()), this, SLOT(clickAxis()));
	chk->setProperty("num", 0);
	chk->setChecked((joy_mask & (JOYCODE_Z_LEFT | JOYCODE_Z_RIGHT)) != 0);
	hbox->addWidget(chk);
	chk = new MyCheckBox(CMsg::Enable_R_axis);
	connect(chk, SIGNAL(toggled()), this, SLOT(clickAxis()));
	chk->setProperty("num", 1);
	chk->setChecked((joy_mask & (JOYCODE_R_UP | JOYCODE_R_DOWN)) != 0);
	hbox->addWidget(chk);
	chk = new MyCheckBox(CMsg::Enable_U_axis);
	connect(chk, SIGNAL(toggled()), this, SLOT(clickAxis()));
	chk->setProperty("num", 2);
	chk->setChecked((joy_mask & (JOYCODE_U_LEFT | JOYCODE_U_RIGHT)) != 0);
	hbox->addWidget(chk);
	chk = new MyCheckBox(CMsg::Enable_V_axis);
	connect(chk, SIGNAL(toggled()), this, SLOT(clickAxis()));
	chk->setProperty("num", 3);
	chk->setChecked((joy_mask & (JOYCODE_V_UP | JOYCODE_V_DOWN)) != 0);
	hbox->addWidget(chk);

	QDialogButtonBox *btn = new QDialogButtonBox(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
	vbox_all->addWidget(btn, Qt::AlignRight);
	connect(btn, SIGNAL(accepted()), this, SLOT(accept()));
	connect(btn, SIGNAL(rejected()), this, SLOT(reject()));
}

MyKeybindBox::~MyKeybindBox()
{
}

void MyKeybindBox::setData()
{
	for(int tab_num=0; tab_num<(int)tables.size(); tab_num++) {
		tables[tab_num]->setData();
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

	tables[curr_tab]->loadPreset(num);
	update();
}

void MyKeybindBox::savePreset()
{
	int num = sender()->property("num").toInt();

	tables[curr_tab]->savePreset(num);

	update();
}

void MyKeybindBox::tabChanged(int index)
{
	curr_tab = index;
}

void MyKeybindBox::toggleAxis(bool checked)
{
	int num = sender()->property("num").toInt();
	uint32_t mask = 0;
	switch(num) {
	case 0:
		mask = (JOYCODE_Z_LEFT | JOYCODE_Z_RIGHT);
		break;
	case 1:
		mask = (JOYCODE_R_UP | JOYCODE_R_DOWN);
		break;
	case 2:
		mask = (JOYCODE_U_LEFT | JOYCODE_U_RIGHT);
		break;
	case 3:
		mask = (JOYCODE_V_UP | JOYCODE_V_DOWN);
		break;
	}
	BIT_ONOFF(joy_mask, mask, checked);
}
