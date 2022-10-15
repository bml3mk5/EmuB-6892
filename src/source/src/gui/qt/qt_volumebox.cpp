/** @file qt_volumebox.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2016.03.01

	@brief [ qt volume box ]
*/

#include "qt_volumebox.h"
//#include "ui_qt_volumebox.h"
#include "qt_dialog.h"
#include "../../emu.h"
#include "../../utils.h"
#include "../../labels.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

extern EMU *emu;

MyVolumeBox::MyVolumeBox(QWidget *parent) :
	QDialog(parent)
//	ui(new Ui::MyVolumeBox)
{
//	ui->setupUi(this);

	setWindowTitle(CMSG(Volume));

	setPtr();

//	ui->verticalSlider1->setRange(0, 100);
//	ui->verticalSlider1->setValue(config.volume);
//	ui->checkMute1->setChecked(config.mute);
//	connect(ui->verticalSlider1, SIGNAL(sliderMoved(int)), this, SLOT(moveSlider1(int)));
//	connect(ui->checkMute1, SIGNAL(stateChanged(int)), this, SLOT(changeMute1(int)));

	QVBoxLayout *vbox_all = new QVBoxLayout(this);
	QHBoxLayout *hbox = new QHBoxLayout();
	for(int i=0, n=0; ; n++) {
		if (LABELS::volume[n] == CMsg::End) {
			break;
		}
		if (LABELS::volume[n] == CMsg::Null) {
			vbox_all->addLayout(hbox);
			QFrame *fr = new QFrame();
			fr->setFrameStyle(QFrame::HLine | QFrame::Sunken);
			vbox_all->addWidget(fr);
			hbox = new QHBoxLayout();
			continue;
		}
		QVBoxLayout *vbox = new QVBoxLayout();
		hbox->addLayout(vbox);

		MyLabel *lblTxt = new MyLabel(LABELS::volume[n]);
		lblTxt->setMinimumSize(64, 32);
		lblTxt->setAlignment(Qt::AlignHCenter);
		vbox->addWidget(lblTxt);
		vbox->setAlignment(lblTxt, Qt::AlignHCenter);

		QSlider *sliVol = new QSlider(Qt::Orientation::Vertical);
		sliVol->setRange(0, 100);
		sliVol->setProperty("index", i);
		vbox->addWidget(sliVol);
		vbox->setAlignment(sliVol, Qt::AlignHCenter);

		lblVol[i] = new QLabel("00");
		vbox->addWidget(lblVol[i]);
		vbox->setAlignment(lblVol[i], Qt::AlignHCenter);

		MyCheckBox *chkMut = new MyCheckBox(CMsg::Mute);
		chkMut->setProperty("index", i);
		vbox->addWidget(chkMut);
		vbox->setAlignment(chkMut, Qt::AlignHCenter);

		sliVol->setValue(*p_volume[i]);
		lblVol[i]->setText(QString::asprintf("%02d", *p_volume[i]));
		chkMut->setChecked(*p_mute[i]);

		connect(sliVol, SIGNAL(sliderMoved(int)), this, SLOT(moveSlider1(int)));
		connect(chkMut, SIGNAL(stateChanged(int)), this, SLOT(changeMute1(int)));

		if (i==0) {
			QFrame *fr = new QFrame();
			fr->setFrameStyle(QFrame::VLine | QFrame::Sunken);
			hbox->addWidget(fr);
		}
		i++;
	}
	hbox->addSpacerItem(new QSpacerItem(1,1,QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding));
	vbox_all->addLayout(hbox);

//	hbox = new QHBoxLayout(this);
	QDialogButtonBox *btn = new QDialogButtonBox(QDialogButtonBox::Close);
	vbox_all->addWidget(btn, Qt::AlignRight);

	connect(btn, SIGNAL(accepted()), this, SLOT(accept()));
	connect(btn, SIGNAL(rejected()), this, SLOT(reject()));
}

MyVolumeBox::~MyVolumeBox()
{
//	delete ui;
}

void MyVolumeBox::setPtr()
{
	int i = 0;
	p_volume[i++] = &config.volume;
	p_volume[i++] = &config.beep_volume;
#if defined(_MBS1)
	p_volume[i++] = &config.psg_volume;
	p_volume[i++] = &config.psgexfm_volume;
	p_volume[i++] = &config.psgexssg_volume;
	p_volume[i++] = &config.psgexpcm_volume;
	p_volume[i++] = &config.psgexrhy_volume;
	p_volume[i++] = &config.opnfm_volume;
	p_volume[i++] = &config.opnssg_volume;
	p_volume[i++] = &config.opnpcm_volume;
	p_volume[i++] = &config.opnrhy_volume;
#endif
	p_volume[i++] = &config.psg6_volume;
	p_volume[i++] = &config.psg9_volume;
	p_volume[i++] = &config.relay_volume;
	p_volume[i++] = &config.cmt_volume;
	p_volume[i++] = &config.fdd_volume;

	i = 0;
	p_mute[i++] = &config.mute;
	p_mute[i++] = &config.beep_mute;
#if defined(_MBS1)
	p_mute[i++] = &config.psg_mute;
	p_mute[i++] = &config.psgexfm_mute;
	p_mute[i++] = &config.psgexssg_mute;
	p_mute[i++] = &config.psgexpcm_mute;
	p_mute[i++] = &config.psgexrhy_mute;
	p_mute[i++] = &config.opnfm_mute;
	p_mute[i++] = &config.opnssg_mute;
	p_mute[i++] = &config.opnpcm_mute;
	p_mute[i++] = &config.opnrhy_mute;
#endif
	p_mute[i++] = &config.psg6_mute;
	p_mute[i++] = &config.psg9_mute;
	p_mute[i++] = &config.relay_mute;
	p_mute[i++] = &config.cmt_mute;
	p_mute[i++] = &config.fdd_mute;
}

void MyVolumeBox::moveSlider1(int num)
{
	QSlider *sli = dynamic_cast<QSlider *>(sender());
	int idx = sli->property("index").toInt();

	*p_volume[idx] = num;
	lblVol[idx]->setText(QString::asprintf("%02d", *p_volume[idx]));

	emu->set_volume(0);
}

void MyVolumeBox::changeMute1(int num)
{
	QCheckBox *chk = dynamic_cast<QCheckBox *>(sender());
	int idx = chk->property("index").toInt();

	*p_mute[idx] = (num == Qt::Checked);

	emu->set_volume(0);
}
