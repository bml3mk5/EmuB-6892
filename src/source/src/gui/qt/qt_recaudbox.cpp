/** @file qt_recaudbox.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2019.03.01

	@brief [ qt record audio box ]
*/

#include "qt_recaudbox.h"
//#include "ui_qt_recaudbox.h"
#include "qt_dialog.h"
#include "../../emu.h"
#include "../../utils.h"
#include "../../video/rec_audio.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

extern EMU *emu;

// list
static const char *audio_type_label[] = {
#ifdef USE_REC_AUDIO
#ifdef USE_REC_AUDIO_AVKIT
    _T("avkit"),
#endif
#ifdef USE_REC_AUDIO_WAVE
	_T("wave"),
#endif
#ifdef USE_REC_AUDIO_MMF
	_T("media foundation"),
#endif
#ifdef USE_REC_AUDIO_FFMPEG
	_T("ffmpeg"),
#endif
#endif
	nullptr };

static const int audio_type_ids[] = {
#ifdef USE_REC_AUDIO
#ifdef USE_REC_AUDIO_AVKIT
    RECORD_AUDIO_TYPE_AVKIT,
#endif
#ifdef USE_REC_AUDIO_WAVE
	RECORD_AUDIO_TYPE_WAVE,
#endif
#ifdef USE_REC_AUDIO_MMF
	RECORD_AUDIO_TYPE_MMF,
#endif
#ifdef USE_REC_AUDIO_FFMPEG
	RECORD_AUDIO_TYPE_FFMPEG,
#endif
#endif
	0 };

MyRecAudBox::MyRecAudBox(QWidget *parent) :
	QDialog(parent)
//	ui(new Ui::MyRecAudBox)
{
//	ui->setupUi(this);

	setWindowTitle(CMSG(Record_Sound));

	QVBoxLayout *vbox_all = new QVBoxLayout(this);

	tab = new QTabWidget();
	vbox_all->addWidget(tab);

	QHBoxLayout *hbox;
	MyLabel *lbl;
	QComboBox *com;

	//
	int i;
	for(i=0; audio_type_ids[i] != 0; i++) {
		QWidget *tab0 = new QWidget();
		tab->addTab(tab0, audio_type_label[i]);

		codnums.append(0);
//		quanums.append(0);

		QVBoxLayout *vbox = new QVBoxLayout(tab0);

		enables.append(emu->rec_sound_enabled(audio_type_ids[i]) ? 1 : 0);

		switch(audio_type_ids[i]) {
			case RECORD_AUDIO_TYPE_WAVE:
			{
				hbox = new QHBoxLayout();
				vbox->addLayout(hbox);
				lbl = new MyLabel(CMsg::Select_a_sample_rate_on_sound_menu_in_advance);
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
				const _TCHAR **codlbl = emu->get_rec_sound_codec_list(audio_type_ids[i]);
				com = new QComboBox();
				hbox->addWidget(com);
				for(count = 0; codlbl[count] != nullptr; count++) {
					com->addItem(codlbl[count]);
				}
				com->setCurrentIndex(0);
				connect(com, SIGNAL(currentIndexChanged()), this, SLOT(codeChangedSlot()));

#if 0
				// Quality
				hbox = new QHBoxLayout();
				vbox->addLayout(hbox);
				lbl = new MyLabel(CMsg::Quality);
				hbox->addWidget(lbl);
				const CMsg::Id *qualbl = emu->get_rec_video_quality_list(type_ids[i]);
				com = new QComboBox();
				hbox->addWidget(com);
				for(count = 0; qualbl[count] != 0 && qualbl[count] != CMsg::End; count++) {
					com->addItem(CMSGV(qualbl[count]));
				}
				com->setCurrentIndex(0);
				connect(com, SIGNAL(currentIndexChanged()), this, SLOT(qualityChangedSlot()));
#endif

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

MyRecAudBox::~MyRecAudBox()
{
//	delete ui;
}

void MyRecAudBox::codeChangedSlot(int idx)
{
	int tab_num = tab->currentIndex();

	codnums[tab_num] = idx;
}

void MyRecAudBox::qualityChangedSlot(int UNUSED_PARAM(idx))
{
//	int tab_num = tab->currentIndex();
//	quanums[tab_num] = idx;
}

int MyRecAudBox::exec()
{
	int rc = QDialog::exec();
	if (rc == QDialog::Accepted) {
		// set datas
		setDatas();
	}
	return rc;
}

void MyRecAudBox::setDatas()
{
	int tab_num = tab->currentIndex();

	emu->set_parami(VM::ParamRecAudioType, audio_type_ids[tab_num]);
	emu->set_parami(VM::ParamRecAudioCodec, codnums[tab_num]);
//	emu->set_parami(VM::ParamRecVideoQuality, quanums[tab_num]);
}
