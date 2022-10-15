/** @file qt_recvidbox.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2019.03.01

	@brief [ qt record video box ]
*/

#include "qt_recvidbox.h"
//#include "ui_qt_recvidbox.h"
#include "qt_dialog.h"
#include "../../emu.h"
#include "../../utils.h"
#include "../../video/rec_video.h"
#include "../../clocale.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

extern EMU *emu;

// list
static const _TCHAR *video_type_label[] = {
#ifdef USE_REC_VIDEO_VFW
	_TX("video for windows"),
#endif
#ifdef USE_REC_VIDEO_AVKIT
	_TX("avkit"),
#endif
#ifdef USE_REC_VIDEO_QTKIT
	_TX("qtkit"),
#endif
#ifdef USE_REC_VIDEO_FFMPEG
	_TX("ffmpeg"),
#endif
	nullptr };

static const int video_type_ids[] = {
#ifdef USE_REC_VIDEO_VFW
	RECORD_VIDEO_TYPE_VFW,
#endif
#ifdef USE_REC_VIDEO_AVKIT
	RECORD_VIDEO_TYPE_AVKIT,
#endif
#ifdef USE_REC_VIDEO_QTKIT
	RECORD_VIDEO_TYPE_QTKIT,
#endif
#ifdef USE_REC_VIDEO_FFMPEG
	RECORD_VIDEO_TYPE_FFMPEG,
#endif
	0 };

MyRecVidBox::MyRecVidBox(QWidget *parent) :
	QDialog(parent)
//	ui(new Ui::MyRecVidBox)
{
//	ui->setupUi(this);

	setWindowTitle(CMSG(Record_Screen));

	QVBoxLayout *vbox_all = new QVBoxLayout(this);

	tab = new QTabWidget();
	vbox_all->addWidget(tab);

	QHBoxLayout *hbox;
	MyLabel *lbl;
	QComboBox *com;

	//
	int i;
	for(i=0; video_type_ids[i] != 0; i++) {
		QWidget *tab0 = new QWidget();
		tab->addTab(tab0, video_type_label[i]);

		codnums.append(0);
		quanums.append(0);

		QVBoxLayout *vbox = new QVBoxLayout(tab0);

		enables.append(emu->rec_video_enabled(video_type_ids[i]) ? 1 : 0);

		switch(video_type_ids[i]) {
			case RECORD_VIDEO_TYPE_VFW:
			{
				hbox = new QHBoxLayout();
				vbox->addLayout(hbox);
				lbl = new MyLabel(CMsg::You_can_set_properties_after_pressing_start_button);
				hbox->addWidget(lbl);
				break;
			}
			default:
			{
				int count = 0;
				// Codec
				hbox = new QHBoxLayout();
				vbox->addLayout(hbox);
				lbl = new MyLabel(CMsg::Codec_Type);
				hbox->addWidget(lbl);
				const _TCHAR **codlbl = emu->get_rec_video_codec_list(video_type_ids[i]);
				com = new QComboBox();
				hbox->addWidget(com);
				for(count = 0; codlbl[count] != nullptr; count++) {
					com->addItem(codlbl[count]);
				}
				com->setCurrentIndex(0);
				connect(com, SIGNAL(currentIndexChanged()), this, SLOT(codeChangedSlot()));

				// Quality
				hbox = new QHBoxLayout();
				vbox->addLayout(hbox);
				lbl = new MyLabel(CMsg::Quality);
				hbox->addWidget(lbl);
				const CMsg::Id *qualbl = emu->get_rec_video_quality_list(video_type_ids[i]);
				com = new QComboBox();
				hbox->addWidget(com);
				for(count = 0; qualbl[count] != 0 && qualbl[count] != CMsg::End; count++) {
					com->addItem(CMSGV(qualbl[count]));
				}
				com->setCurrentIndex(0);
				connect(com, SIGNAL(currentIndexChanged()), this, SLOT(qualityChangedSlot()));

				break;
			}
		}
		if (!enables[i]) {
			lbl = new MyLabel(CMsg::Need_install_library);
			vbox->addWidget(lbl);
		}
	}

	// button
	QDialogButtonBox *btn = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	vbox_all->addWidget(btn);

	connect(btn, SIGNAL(accepted()), this, SLOT(accept()));
	connect(btn, SIGNAL(rejected()), this, SLOT(reject()));
}

MyRecVidBox::~MyRecVidBox()
{
//	delete ui;
}

void MyRecVidBox::codeChangedSlot(int idx)
{
	int tab_num = tab->currentIndex();

	codnums[tab_num] = idx;
}

void MyRecVidBox::qualityChangedSlot(int idx)
{
	int tab_num = tab->currentIndex();

	quanums[tab_num] = idx;
}

int MyRecVidBox::exec()
{
	int rc = QDialog::exec();
	if (rc == QDialog::Accepted) {
		// set datas
		setDatas();
	}
	return rc;
}

void MyRecVidBox::setDatas()
{
	int tab_num = tab->currentIndex();

	emu->set_parami(VM::ParamRecVideoType, video_type_ids[tab_num]);
	emu->set_parami(VM::ParamRecVideoCodec, codnums[tab_num]);
	emu->set_parami(VM::ParamRecVideoQuality, quanums[tab_num]);
}
