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
#include <QKeyEvent>
#include <QTimer>

extern EMU *emu;

MyKeybindBox::MyKeybindBox(QWidget *parent) :
	MyKeybindBaseBox(parent)
//	ui(new Ui::MyKeybindBox)
{
	setWindowTitle(CMSG(Keybind));

//	ui->setupUi(this);

	int tab_offset = KeybindData::KB_TABS_MIN;
	for(int tab_num=tab_offset; tab_num<KeybindData::KB_TABS_MAX; tab_num++) {
		tables.push_back(new MyTableWidget(tab_num));
	}

	QVBoxLayout *vbox_all = new QVBoxLayout(this);
	QHBoxLayout *hbox = new QHBoxLayout();
	vbox_all->addLayout(hbox);
	QVBoxLayout *vbox_tab = new QVBoxLayout();
	hbox->addLayout(vbox_tab);
	QVBoxLayout *vbox_btn = new QVBoxLayout();
	hbox->addLayout(vbox_btn);

	tabWidget = new MyTabWidget();
	vbox_tab->addWidget(tabWidget);


//	curr_tab = 0;
	for(int tab_num=0; tab_num<(int)tables.size(); tab_num++) {
		QWidget *titmWidget = new QWidget();
		QVBoxLayout *vbox = new QVBoxLayout(titmWidget);
		tables[tab_num]->setMinimumSize(400, 400);
		vbox->addWidget(tables[tab_num]);
		tabWidget->addTab(titmWidget, LABELS::keybind_tab[tab_num]);

		tables[tab_num]->addCombiCheckButton(vbox);
	}
//	connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));

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
	createFooter(vbox_all);

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
