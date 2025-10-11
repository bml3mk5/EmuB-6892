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
#include "../gui_keybinddata.h"
#include "../../utility.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

extern EMU *emu;

MyJoySettingBox::MyJoySettingBox(QWidget *parent) :
	MyKeybindBaseBox(parent)
{
	_TCHAR str[64];

	setWindowTitle(CMSG(Joypad_Setting));

	QVBoxLayout *vbox_all = new QVBoxLayout(this);

#if defined(USE_JOYSTICK) || defined(USE_KEY2JOYSTICK)
	QHBoxLayout *hbox_all = new QHBoxLayout(this);
	vbox_all->addLayout(hbox_all);

	MyLabel *lbl;
	int lx = 80;
	int sw = 100;
	for(int i=0; i<MAX_JOYSTICKS; i++) {
		QVBoxLayout *vbox = new QVBoxLayout();
		hbox_all->addLayout(vbox);

		UTILITY::stprintf(str, 64, CMSGV(CMsg::JoypadVDIGIT), i + 1);
		lbl = new MyLabel(str);
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

	int tab_offset = KeybindData::JS_TABS_MIN;
	for(int tab_num=tab_offset; tab_num<KeybindData::JS_TABS_MAX; tab_num++) {
		tables.push_back(new MyTableWidget(tab_num));
	}

	QVBoxLayout *vbox_tab = new QVBoxLayout(this);
	hbox_all->addLayout(vbox_tab);
	tabWidget = new MyTabWidget();
	vbox_tab->addWidget(tabWidget);

//	curr_tab = 0;
	for(int tab_num=0; tab_num<(int)tables.size(); tab_num++) {
		QWidget *titmWidget = new QWidget();
		QVBoxLayout *vbox = new QVBoxLayout(titmWidget);
		tables[tab_num]->setMinimumSize(300, 300);
//		tabWidget->addTab(titmWidget, LABELS::joysetting_tab[tab_num]);
		UTILITY::stprintf(str, 64, _T("%d"), tab_num + 1);
		tabWidget->addTab(titmWidget, str);

		vbox->addWidget(new MyLabel(LABELS::joysetting_tab[tab_num]));

		vbox->addWidget(tables[tab_num]);

//		tables[tab_num]->addCombiCheckButton(vbox);

#ifdef USE_PIAJOYSTICK
		if (tab_num + KeybindData::JS_TABS_MIN == Keybind::TAB_JOY2JOY) {
#ifdef USE_JOYSTICKBIT
			// check box
			chkPiaJoyNeg = new MyCheckBox(CMsg::Signals_are_negative_logic);
			chkPiaJoyNeg->setChecked(FLG_PIAJOY_NEGATIVE != 0);
			vbox->addWidget(chkPiaJoyNeg);
			QHBoxLayout *hbox = new QHBoxLayout();
			hbox->addWidget(new MyLabel(CMsg::Connect_to_));
			for(int i=0; LABELS::joysetting_opts[i] != CMsg::End; i++) {
				radPiaJoyConn[i] = new MyRadioButton(LABELS::joysetting_opts[i]);
				radPiaJoyConn[i]->setChecked(pConfig->piajoy_conn_to == i);
				hbox->addWidget(radPiaJoyConn[i]);
			}
			vbox->addLayout(hbox);
#else
			chkPiaJoyNoIrq = new MyCheckBox(CMsg::No_interrupt_caused_by_pressing_the_button);
			chkPiaJoyNoIrq->setChecked(FLG_PIAJOY_NOIRQ != 0);
			vbox->addWidget(chkPiaJoyNoIrq);
#endif
		}
#endif
#ifdef USE_PSGJOYSTICK
		if (tab_num + KeybindData::JS_TABS_MIN == Keybind::TAB_JOY2JOYB) {
#ifdef USE_JOYSTICKBIT
			// check box
			chkPsgJoyNeg = new MyCheckBox(CMsg::Signals_are_negative_logic);
			chkPsgJoyNeg->setChecked(FLG_PSGJOY_NEGATIVE != 0);
			vbox->addWidget(chkPsgJoyNeg);
#endif
		}
#endif
	}
//	connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));


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
	createFooter(vbox_tab);

#endif

	QDialogButtonBox *btn = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	vbox_all->addWidget(btn, Qt::AlignRight);

	connect(btn, SIGNAL(accepted()), this, SLOT(accept()));
	connect(btn, SIGNAL(rejected()), this, SLOT(reject()));
}

MyJoySettingBox::~MyJoySettingBox()
{
}

void MyJoySettingBox::setData()
{
#if defined(USE_PIAJOYSTICK) || defined(USE_KEY2JOYSTICK)
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
#endif
#if defined(USE_PIAJOYSTICK) || defined(USE_KEY2PIAJOYSTICK)
# ifdef USE_JOYSTICKBIT
	BIT_ONOFF(pConfig->misc_flags, MSK_PIAJOY_NEGATIVE, chkPiaJoyNeg->isChecked());
	for(int i=0; LABELS::joysetting_opts[i] != CMsg::End; i++) {
		if (radPiaJoyConn[i]->isChecked()) {
			pConfig->piajoy_conn_to = i;
			break;
		}
	}
# else
	BIT_ONOFF(pConfig->misc_flags, MSK_PIAJOY_NOIRQ, chkPiaJoyNoIrq->isChecked());
# endif
#endif
#if defined(USE_PSGJOYSTICK) || defined(USE_KEY2PSGJOYSTICK)
# ifdef USE_JOYSTICKBIT
	BIT_ONOFF(pConfig->misc_flags, MSK_PSGJOY_NEGATIVE, chkPsgJoyNeg->isChecked());
# endif
#endif
}
