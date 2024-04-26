/** @file qt_joysetbox.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2023.01.08

	@brief [ qt joyset box ]
*/

#include "qt_joysetbox.h"
#include "qt_dialog.h"
#include "../../config.h"
//#include "../../utils.h"
#include "../../msgs.h"
#include "../../labels.h"
#include "../../keycode.h"
#include "../gui_keybinddata.h"
#include "../../utility.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

extern EMU *emu;

MyJoySettingBox::MyJoySettingBox(QWidget *parent) :
	QDialog(parent)
{
	_TCHAR str[64];

	setWindowTitle(CMSG(Joypad_Setting));

	QVBoxLayout *vbox_all = new QVBoxLayout(this);

#if defined(USE_JOYSTICK) || defined(USE_KEY2JOYSTICK)
	QHBoxLayout *hbox_all = new QHBoxLayout(this);
	vbox_all->addLayout(hbox_all);

	int lx = 80;
	int sw = 100;
	for(int i=0; i<MAX_JOYSTICKS; i++) {
		QVBoxLayout *vbox = new QVBoxLayout();
		hbox_all->addLayout(vbox);

		UTILITY::stprintf(str, 64, CMSGV(CMsg::JoypadVDIGIT), i + 1);
		MyLabel *lbl = new MyLabel(str);
		vbox->addWidget(lbl);


		QHBoxLayout *hbox = new QHBoxLayout();
		vbox->addLayout(hbox);
		lbl = new MyLabel(CMsg::Button_Mashing_Speed);
		hbox->addWidget(lbl);
		hbox->addSpacerItem(new QSpacerItem(32, 1));
		lbl = new MyLabel(_T("0 <-> 3"));
		hbox->addWidget(lbl);

		for(int k=0; k<KEYBIND_JOY_BUTTONS; k++) {
			int kk = k + VM_JOY_LABEL_BUTTON_A;
			if (kk >= VM_JOY_LABELS_MAX) {
				mash[i][k] = nullptr;
				continue;
			}

			CMsg::Id id = (CMsg::Id)cVmJoyLabels[kk].id;

			hbox = new QHBoxLayout();
			vbox->addLayout(hbox);

			lbl = new MyLabel(id, lx, 1, Qt::AlignHCenter);
			hbox->addWidget(lbl);

			int val = pConfig->joy_mashing[i][k];
			MySlider *sli = new MySlider(Qt::Orientation::Horizontal, 0, 3, val, sw, 1);
			sli->setTickInterval(1);
			sli->setTickPosition(QSlider::TickPosition::TicksAbove);
			sli->setProperty("idx", i);
			sli->setProperty("pos", k);
			hbox->addWidget(sli);
			hbox->setAlignment(sli, Qt::AlignHCenter);
			mash[i][k] = sli;

//			connect(sli, SIGNAL(sliderMoved(int)), this, SLOT(moveSlider1(int)));
		}

		hbox = new QHBoxLayout();
		vbox->addLayout(hbox);
		lbl = new MyLabel(CMsg::Analog_to_Digital_Sensitivity);
		hbox->addWidget(lbl);
		hbox->addSpacerItem(new QSpacerItem(32, 1));
		lbl = new MyLabel(_T("0 <-> 10"));
		hbox->addWidget(lbl);

		for(int k=0; k<6; k++) {
			CMsg::Id id = LABELS::joypad_axis[k];

			hbox = new QHBoxLayout();
			vbox->addLayout(hbox);

			lbl = new MyLabel(id, lx, 1, Qt::AlignHCenter);
			hbox->addWidget(lbl);

			int val = 10 - pConfig->joy_axis_threshold[i][k];
			MySlider *sli = new MySlider(Qt::Orientation::Horizontal, 0, 10, val, sw, 1);
			sli->setTickInterval(1);
			sli->setTickPosition(QSlider::TickPosition::TicksAbove);
			hbox->addWidget(sli);
			hbox->setAlignment(sli, Qt::AlignHCenter);
			axis[i][k] = sli;
		}
	}

	//

	int tab_offset = KeybindData::TAB_JOY2JOY;
	for(int tab_num=tab_offset; tab_num<KeybindData::TABS_MAX; tab_num++) {
		tables.push_back(new MyTableWidget(tab_num));
	}

	QVBoxLayout *vbox_tab = new QVBoxLayout(this);
	hbox_all->addLayout(vbox_tab);
	MyTabWidget *tabWidget = new MyTabWidget();
	vbox_tab->addWidget(tabWidget);

	curr_tab = 0;
	for(int tab=0; tab<(int)tables.size(); tab++) {
		QWidget *titmWidget = new QWidget();
		QVBoxLayout *vbox = new QVBoxLayout(titmWidget);
		tables[tab]->setMinimumSize(300, 300);
		vbox->addWidget(tables[tab]);
		tabWidget->addTab(titmWidget, LABELS::keybind_tab[tab + tab_offset]);
	}
	connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));

	// right button
	char label[128];

	MyPushButton *btn_d = new MyPushButton(CMsg::Load_Default);
	vbox_tab->addWidget(btn_d);
	connect(btn_d, SIGNAL(released()), this, SLOT(loadPreset()));
	btn_d->setProperty("num", QVariant::fromValue(-1));
	QHBoxLayout *hbox;
	for(int i=0; i<4; i++) {
		hbox = new QHBoxLayout();
		vbox_tab->addLayout(hbox);

		gMessages.Sprintf(label, 128, CMsg::Load_Preset_VDIGIT, i+1);
		QPushButton *btn_lp = new QPushButton(label);
		hbox->addWidget(btn_lp);
		connect(btn_lp, SIGNAL(released()), this, SLOT(loadPreset()));
		btn_lp->setProperty("num", QVariant::fromValue(i));

		gMessages.Sprintf(label, 128, CMsg::Save_Preset_VDIGIT, i+1);
		QPushButton *btn_sp = new QPushButton(label);
		hbox->addWidget(btn_sp);
		connect(btn_sp, SIGNAL(released()), this, SLOT(savePreset()));
		btn_sp->setProperty("num", QVariant::fromValue(i));
	}

	//

	joy_mask = ~0;
	hbox = new QHBoxLayout();
	vbox_tab->addLayout(hbox);
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

#endif

	QDialogButtonBox *btn = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	vbox_all->addWidget(btn, Qt::AlignRight);

	connect(btn, SIGNAL(accepted()), this, SLOT(accept()));
	connect(btn, SIGNAL(rejected()), this, SLOT(reject()));
}

MyJoySettingBox::~MyJoySettingBox()
{
}

void MyJoySettingBox::SetData()
{
	for(int i=0; i<MAX_JOYSTICKS; i++) {
		for(int k=0; k<KEYBIND_JOY_BUTTONS; k++) {
			if (!mash[i][k]) continue;
			pConfig->joy_mashing[i][k] = mash[i][k]->value();
		}
		for(int k=0; k<6; k++) {
			if (!axis[i][k]) continue;
			pConfig->joy_axis_threshold[i][k] = 10 - axis[i][k]->value();
		}
	}
	emu->modify_joy_mashing();
	emu->modify_joy_threshold();

	for(int tab=0; tab<(int)tables.size(); tab++) {
		tables[tab]->setData();
	}

	emu->save_keybind();
}

void MyJoySettingBox::accept()
{
	SetData();
	QDialog::accept();
}

void MyJoySettingBox::update()
{
	if (tables[curr_tab]) {
		tables[curr_tab]->update();
	}

	QDialog::update();
}

void MyJoySettingBox::loadPreset()
{
	int num = sender()->property("num").toInt();

	tables[curr_tab]->loadPreset(num);
	update();
}

void MyJoySettingBox::savePreset()
{
	int num = sender()->property("num").toInt();

	tables[curr_tab]->savePreset(num);

	update();
}

void MyJoySettingBox::tabChanged(int index)
{
	curr_tab = index;
}

void MyJoySettingBox::toggleAxis(bool checked)
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

#if 0
void MyJoySettingBox::moveSlider1(int num)
{
#if defined(USE_JOYSTICK) || defined(USE_KEY2JOYSTICK)
	QSlider *sli = dynamic_cast<QSlider *>(sender());
	int i = sli->property("idx").toInt();
	int k = sli->property("pos").toInt();

	pConfig->joy_mashing[i][k] = num;
#endif
}
#endif
