/** @file qt_configbox.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2016.03.01

	@brief [ qt config box ]
*/

#include "qt_configbox.h"
//#include "ui_qt_configbox.h"
#include "qt_dialog.h"
#include <QFontDialog>
#include "../../emu.h"
#include "../gui.h"
#include "../../utils.h"
#include "../../clocale.h"
#include "../../labels.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <math.h>

extern EMU *emu;
extern GUI *gui;
extern CLocale *clocale;

#ifdef USE_IOPORT_FDD
#define IOPORT_STARTNUM 0
#else
#define IOPORT_STARTNUM 2
#endif

MyConfigBox::MyConfigBox(QWidget *parent) :
	QDialog(parent)
//	ui(new Ui::MyConfigBox)
{
	setWindowTitle(CMSG(Configure));

//	ui->setupUi(this);
//	QHBoxLayout *hbox;
	QSpacerItem *spc;
	QLabel *lbl;

	int fdd_type = emu->get_parami(VM::ParamFddType);
	int io_port = emu->get_parami(VM::ParamIOPort);

	QVBoxLayout *vbox_all = new QVBoxLayout(this);

	MyTabWidget *tab = new MyTabWidget();
	vbox_all->addWidget(tab);

	// Mode Tab
	QWidget *tab0 = new QWidget();
	tab->addTab(tab0, CMsg::Mode);

	QVBoxLayout *vbox0 = new QVBoxLayout(tab0);

	QHBoxLayout *hbox0 = new QHBoxLayout();
	vbox0->addLayout(hbox0);

	QVBoxLayout *vbox0l = new QVBoxLayout();
	hbox0->addLayout(vbox0l);

	// MODE Switch
#if defined(_MBS1)
	const CMsg::Id sys_mode_labels[] = {
		CMsg::A_Mode_S1,
		CMsg::B_Mode_L3,
		CMsg::End
	};
	MyGroupBox *grpSysMode = new MyGroupBox(CMsg::System_Mode_ASTERISK);
	vbox0l->addWidget(grpSysMode);
	QGridLayout *griSysMode = new QGridLayout(grpSysMode);
	for(int row=0; row<2; row++) {
		QLabel *lblSysMode;
		lblSysMode = new QLabel(">");
		lblSysMode->setMinimumSize(16, 0);
		griSysMode->addWidget(lblSysMode, row, 0);
		lblSysMode->setVisible((emu->get_parami(VM::ParamSysMode) & 1) == (1 - row));
		radSysMode[row] = new MyRadioButton(sys_mode_labels[row]);
		radSysMode[row]->setMinimumSize(16, 0);
		griSysMode->addWidget(radSysMode[row], row, 1);
		radSysMode[row]->setChecked((config.sys_mode & 1) == (1 - row));
	}
	QHBoxLayout *hbox_dip = new QHBoxLayout();
	chkDipSwitch = new MyCheckBox(CMsg::NEWON7);
	chkDipSwitch->setChecked((config.dipswitch & 4) != 0);
	hbox_dip->addItem(new QSpacerItem(16,1));
	hbox_dip->addWidget(chkDipSwitch);
	griSysMode->addLayout(hbox_dip, 2, 1);

	spc = new QSpacerItem(1,1,QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
	griSysMode->addItem(spc, 0, 3);
#else
	MyGroupBox *grpDipSwitch = new MyGroupBox(CMsg::DIP_Switch_ASTERISK);
	vbox0l->addWidget(grpDipSwitch);
	QGridLayout *griDipSwitch = new QGridLayout(grpDipSwitch);
	QLabel *lblModeSwitch = new QLabel(">");
	lblModeSwitch->setMinimumSize(16, 0);
	griDipSwitch->addWidget(lblModeSwitch, 0, 0);
	chkModeSwitch = new MyCheckBox(CMsg::MODE_Switch);
	chkModeSwitch->setChecked((config.dipswitch & 4) != 0);
	griDipSwitch->addWidget(chkModeSwitch, 0, 1);
	spc = new QSpacerItem(1,1,QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
	griDipSwitch->addItem(spc, 0, 2);
#endif

	// FDD Type
	MyGroupBox *grpFddType = new MyGroupBox(CMsg::FDD_Type_ASTERISK);
	vbox0l->addWidget(grpFddType);
	QGridLayout *griFddType = new QGridLayout(grpFddType);
	for(int row=0; row<4; row++) {
		lblFddTypes[row] = new QLabel(">");
		lblFddTypes[row]->setMinimumSize(16, 0);
		griFddType->addWidget(lblFddTypes[row], row, 0);
		lblFddTypes[row]->setVisible(config.fdd_type == row);
		radFddTypes[row] = new MyRadioButton(LABELS::fdd_type[row]);
		griFddType->addWidget(radFddTypes[row], row, 1);
		radFddTypes[row]->setChecked(fdd_type == row);
	}
	spc = new QSpacerItem(1,1,QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
	griFddType->addItem(spc, 0, 3);

	// Power Off
	chkPowerOff = new MyCheckBox(CMsg::Enable_the_state_of_power_off);
	chkPowerOff->setChecked(config.use_power_off);
	vbox0l->addWidget(chkPowerOff);

	spc = new QSpacerItem(1,1,QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
	vbox0l->addSpacerItem(spc);

	QVBoxLayout *vbox0r = new QVBoxLayout();
	hbox0->addLayout(vbox0r);

	// I/O Port
	MyGroupBox *grpIOPort = new MyGroupBox(CMsg::I_O_Port_Address_ASTERISK);
	vbox0r->addWidget(grpIOPort);
	QGridLayout *griIOPort = new QGridLayout(grpIOPort);
	int val = emu->get_parami(VM::ParamIOPort);
	for(int i = 0; LABELS::io_port[i] != CMsg::End; i++) {
		int pos = LABELS::io_port_pos[i];
		if ((1 << pos) & IOPORT_MSK_ALL) {
			lblIOPorts[pos] = new QLabel(">");
			lblIOPorts[pos]->setMinimumSize(16, 0);
			griIOPort->addWidget(lblIOPorts[pos], pos, 0);
			lblIOPorts[pos]->setVisible(val & (1 << pos));
			chkIOPorts[pos] = new MyCheckBox(LABELS::io_port[i]);
			griIOPort->addWidget(chkIOPorts[pos], pos, 1);
			chkIOPorts[pos]->setChecked(io_port & (1 << pos));
		}
	}
	spc = new QSpacerItem(1,1,QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
	griIOPort->addItem(spc, 0, 3);

	spc = new QSpacerItem(1,1,QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
	vbox0r->addSpacerItem(spc);

	// exclusive ioports
	connect(chkIOPorts[5], SIGNAL(toggled(bool)), this, SLOT(toggledIOPort(bool)));
	connect(chkIOPorts[6], SIGNAL(toggled(bool)), this, SLOT(toggledIOPort(bool)));
#if defined(_MBS1)
	connect(chkIOPorts[IOPORT_POS_EXPSG], SIGNAL(toggled(bool)), this, SLOT(toggledIOPort(bool)));
	connect(chkIOPorts[IOPORT_POS_FMOPN], SIGNAL(toggled(bool)), this, SLOT(toggledIOPort(bool)));
#endif

	lbl = new MyLabel(CMsg::Need_restart_program_or_PowerOn);
	vbox0->addWidget(lbl);

	spc = new QSpacerItem(1,1,QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
	vbox0->addSpacerItem(spc);

	// screen tab
	QWidget *tab1 = new QWidget();
	tab->addTab(tab1, CMsg::Screen);

	QVBoxLayout *vbox1 = new QVBoxLayout(tab1);

	QHBoxLayout *hbox1 = new QHBoxLayout();
	vbox1->addLayout(hbox1);

	// opengl group
	MyGroupBox *grpOpenGL = new MyGroupBox(CMsg::Drawing);
	hbox1->addWidget(grpOpenGL);

	QVBoxLayout *vboxOpenGL = new QVBoxLayout(grpOpenGL);

	// use opengl
	QHBoxLayout *hboxUseOpenGL = new QHBoxLayout();
	vboxOpenGL->addLayout(hboxUseOpenGL);
	lbl = new MyLabel(CMsg::Method_ASTERISK);
	hboxUseOpenGL->addWidget(lbl);
	comUseOpenGL = new MyComboBox(nullptr, LABELS::opengl_use, config.use_opengl);
	hboxUseOpenGL->addWidget(comUseOpenGL);
	spc = new QSpacerItem(1,1,QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
	hboxUseOpenGL->addSpacerItem(spc);
	// opengl filter
	QHBoxLayout *hboxGLFilter = new QHBoxLayout();
	vboxOpenGL->addLayout(hboxGLFilter);
	lbl = new MyLabel(CMsg::Filter_Type);
	hboxGLFilter->addWidget(lbl);
	comGLFilter = new MyComboBox(nullptr, LABELS::opengl_filter, config.gl_filter_type);
	hboxGLFilter->addWidget(comGLFilter);
	spc = new QSpacerItem(1,1,QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
	hboxGLFilter->addSpacerItem(spc);

	// CRTC group
	QGroupBox *grpCRTC = new MyGroupBox(CMsg::CRTC);
	hbox1->addWidget(grpCRTC);

	QVBoxLayout *vboxCRTC = new QVBoxLayout(grpCRTC);

	// CRTC disptmg
	QHBoxLayout *hboxCRTCdisptmg = new QHBoxLayout();
	vboxCRTC->addLayout(hboxCRTCdisptmg);
	lbl = new MyLabel(CMsg::Disptmg_Skew);
	hboxCRTCdisptmg->addWidget(lbl);
	comCRTCdisptmg = new MyComboBox(nullptr, LABELS::disp_skew);
#if defined(_MBS1)
	comCRTCdisptmg->setCurrentIndex(config.disptmg_skew + 2);
#else
	comCRTCdisptmg->setCurrentIndex(config.disptmg_skew);
#endif
	hboxCRTCdisptmg->addWidget(comCRTCdisptmg);
	spc = new QSpacerItem(1,1,QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
	hboxCRTCdisptmg->addSpacerItem(spc);
	// CRTC curdisp
	QHBoxLayout *hboxCRTCcurdisp = new QHBoxLayout();
	vboxCRTC->addLayout(hboxCRTCcurdisp);
#if defined(_MBS1)
	lbl = new MyLabel(CMsg::Curdisp_Skew_L3);
#else
	lbl = new MyLabel(CMsg::Curdisp_Skew);
#endif
	hboxCRTCcurdisp->addWidget(lbl);
	comCRTCcurdisp = new MyComboBox(nullptr, LABELS::disp_skew,
#if defined(_MBS1)
		config.curdisp_skew + 2
#else
		config.curdisp_skew
#endif
	);
	hboxCRTCcurdisp->addWidget(comCRTCcurdisp);
	spc = new QSpacerItem(1,1,QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
	hboxCRTCcurdisp->addSpacerItem(spc);

	// LED
	QHBoxLayout *hboxLED = new QHBoxLayout();
	vbox1->addLayout(hboxLED);
	lbl = new MyLabel(CMsg::LED);
	hboxLED->addWidget(lbl);
	comLED = new MyComboBox(nullptr, LABELS::led_show, gui->GetLedBoxPhase(-1));
	hboxLED->addWidget(comLED);
	// LED position
	lbl = new MyLabel(CMsg::Position);
	hboxLED->addWidget(lbl);
	comLEDPos = new MyComboBox(nullptr, LABELS::led_pos, config.led_pos);
	hboxLED->addWidget(comLEDPos);
	spc = new QSpacerItem(1,1,QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
	hboxLED->addSpacerItem(spc);

	// Capture type
	QHBoxLayout *hboxCapType = new QHBoxLayout();
	vbox1->addLayout(hboxCapType);
	lbl = new MyLabel(CMsg::Capture_Type);
	hboxCapType->addWidget(lbl);
	comCapType = new MyComboBox(nullptr, LABELS::capture_fmt);
	comCapType->setCurrentIndex(config.capture_type);
	hboxCapType->addWidget(comCapType);
	spc = new QSpacerItem(1,1,QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
	hboxCapType->addSpacerItem(spc);

	// Snapshot path
	QHBoxLayout *hboxSnapPath = new QHBoxLayout();
	vbox1->addLayout(hboxSnapPath);
	lbl = new MyLabel(CMsg::Snapshot_Path);
	hboxSnapPath->addWidget(lbl);
	linSnapPath = new QLineEdit();
	linSnapPath->setText(QTChar::fromTChar(config.snapshot_path.Get()));
	linSnapPath->setMinimumSize(368, 0);
	hboxSnapPath->addWidget(linSnapPath);
	MyPushButton *btnSnapPath = new MyPushButton(CMsg::Folder_);
	connect(btnSnapPath, SIGNAL(pressed()), this, SLOT(pressedSnapPath()));
	hboxSnapPath->addWidget(btnSnapPath);
	spc = new QSpacerItem(1,1,QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
	hboxSnapPath->addSpacerItem(spc);

	// Font path
#if 0
	QHBoxLayout *hboxFontPath = new QHBoxLayout();
	vbox1->addLayout(hboxFontPath);
	lbl = new MyLabel(CMsg::Font_Path);
	hboxFontPath->addWidget(lbl);
	linFontPath = new QLineEdit();
	linFontPath->setText(QTChar::fromTChar(config.font_path.Get()));
	linFontPath->setMinimumSize(256, 0);
	hboxFontPath->addWidget(linFontPath);
	MyPushButton *btnFontPath = new MyPushButton(CMsg::Folder_);
	connect(btnFontPath, SIGNAL(pressed()), this, SLOT(pressedFontPath()));
	hboxFontPath->addWidget(btnFontPath);
	spc = new QSpacerItem(1,1,QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
	hboxFontPath->addSpacerItem(spc);
#endif

	// message font
	QHBoxLayout *hboxMsgFont = new QHBoxLayout();
	vbox1->addLayout(hboxMsgFont);
	lbl = new MyLabel(CMsg::Message_Font);
	hboxMsgFont->addWidget(lbl);
	linMsgFont = new QLineEdit();
	linMsgFont->setText(QTChar::fromTChar(config.msgboard_msg_fontname.Get()));
	linMsgFont->setMinimumSize(192, 0);
	hboxMsgFont->addWidget(linMsgFont);
	lbl = new MyLabel(CMsg::_Size);
	hboxMsgFont->addWidget(lbl);
	linMsgSize = new QLineEdit();
	linMsgSize->setText(QString::number(config.msgboard_msg_fontsize));
	linMsgSize->setMinimumSize(32, 0);
	hboxMsgFont->addWidget(linMsgSize);
	MyPushButton *btnMsgFont = new MyPushButton(CMsg::Font_);
	connect(btnMsgFont, SIGNAL(pressed()), this, SLOT(pressedMsgFont()));
	hboxMsgFont->addWidget(btnMsgFont);
	spc = new QSpacerItem(1,1,QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
	hboxMsgFont->addSpacerItem(spc);

	// info font
	QHBoxLayout *hboxInfoFont = new QHBoxLayout();
	vbox1->addLayout(hboxInfoFont);
	lbl = new MyLabel(CMsg::Info_Font);
	hboxInfoFont->addWidget(lbl);
	linInfoFont = new QLineEdit();
	linInfoFont->setText(QTChar::fromTChar(config.msgboard_info_fontname.Get()));
	linInfoFont->setMinimumSize(192, 0);
	hboxInfoFont->addWidget(linInfoFont);
	lbl = new MyLabel(CMsg::_Size);
	hboxInfoFont->addWidget(lbl);
	linInfoSize = new QLineEdit();
	linInfoSize->setText(QString::number(config.msgboard_info_fontsize));
	linInfoSize->setMinimumSize(32, 0);
	hboxInfoFont->addWidget(linInfoSize);
	MyPushButton *btnInfoFont = new MyPushButton(CMsg::Font_);
	connect(btnInfoFont, SIGNAL(pressed()), this, SLOT(pressedInfoFont()));
	hboxInfoFont->addWidget(btnInfoFont);
	spc = new QSpacerItem(1,1,QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
	hboxInfoFont->addSpacerItem(spc);

	// language
	lang_list.Clear();
	clocale->GetLocaleNamesWithDefault(lang_list);
	int lang_selidx = clocale->SelectLocaleNameIndex(lang_list, config.language);

	QHBoxLayout *hboxLanguage = new QHBoxLayout();
	vbox1->addLayout(hboxLanguage);
	lbl = new MyLabel(CMsg::Language_ASTERISK);
	hboxLanguage->addWidget(lbl);
	comLanguage = new MyComboBox(nullptr, lang_list);
	comLanguage->setCurrentIndex(lang_selidx);
	hboxLanguage->addWidget(comLanguage);
	spc = new QSpacerItem(1,1,QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
	hboxLanguage->addSpacerItem(spc);

	lbl = new MyLabel(CMsg::Need_restart_program);
	vbox1->addWidget(lbl);

	spc = new QSpacerItem(1,1,QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
	vbox1->addSpacerItem(spc);

	// Tape, FDD tab
	QWidget *tab2 = new QWidget();
	tab->addTab(tab2, CMsg::Tape_FDD);

	QVBoxLayout *vbox2 = new QVBoxLayout(tab2);

	QHBoxLayout *hbox2 = new QHBoxLayout();
	vbox2->addLayout(hbox2);

	// load wav group
	MyGroupBox *grpLoadWav = new MyGroupBox(CMsg::Load_Wav_File_from_Tape);
	hbox2->addWidget(grpLoadWav);

	QVBoxLayout *vboxLoadWav = new QVBoxLayout(grpLoadWav);

	QHBoxLayout *hboxLoadWav = new QHBoxLayout();
	vboxLoadWav->addLayout(hboxLoadWav);
	chkReverse = new MyCheckBox(CMsg::Reverse_Wave);
	chkReverse->setChecked(config.wav_reverse);
	hboxLoadWav->addWidget(chkReverse);
	chkHalf = new MyCheckBox(CMsg::Half_Wave);
	chkHalf->setChecked(config.wav_half);
	hboxLoadWav->addWidget(chkHalf);
	spc = new QSpacerItem(1,1,QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
	hboxLoadWav->addSpacerItem(spc);

	QHBoxLayout *hboxCorrect = new QHBoxLayout();
	vboxLoadWav->addLayout(hboxCorrect);
	lbl = new MyLabel(CMsg::Correct);
	hboxCorrect->addWidget(lbl);
	int correct_type = (config.wav_correct ? config.wav_correct_type + 1 : 0);
	radNoCorr = new MyRadioButton(CMsg::None_);
	radNoCorr->setChecked(correct_type == 0);
	hboxCorrect->addWidget(radNoCorr);
	radCOS = new MyRadioButton(CMsg::COS_Wave);
	radCOS->setChecked(correct_type == 1);
	hboxCorrect->addWidget(radCOS);
	radSIN = new MyRadioButton(CMsg::SIN_Wave);
	radSIN->setChecked(correct_type == 2);
	hboxCorrect->addWidget(radSIN);
	spc = new QSpacerItem(1,1,QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
	hboxCorrect->addSpacerItem(spc);

	QHBoxLayout *hboxCorrAmp = new QHBoxLayout();
	vboxLoadWav->addLayout(hboxCorrAmp);
	for(int i=0; i<2; i++) {
		lbl = new QLabel(i == 0 ? "1200Hz" : "2400Hz");
		hboxCorrAmp->addWidget(lbl);
		linCorrAmp[i] = new QLineEdit();
		linCorrAmp[i]->setText(QString::number(config.wav_correct_amp[i]));
        linCorrAmp[i]->setMaxLength(5);
        linCorrAmp[i]->setFixedWidth(64);
		hboxCorrAmp->addWidget(linCorrAmp[i]);
	}
	spc = new QSpacerItem(1,1,QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
	hboxCorrAmp->addSpacerItem(spc);

	// save wav group
	MyGroupBox *grpSaveWav = new MyGroupBox(CMsg::Save_Wav_File_to_Tape);
	hbox2->addWidget(grpSaveWav);

	QVBoxLayout *vboxSaveWav = new QVBoxLayout(grpSaveWav);

	QHBoxLayout *hboxRate = new QHBoxLayout();
	vboxSaveWav->addLayout(hboxRate);
	lbl = new MyLabel(CMsg::Sample_Rate);
	hboxRate->addWidget(lbl);
	comRate = new MyComboBox(nullptr, LABELS::sound_rate);
	comRate->setCurrentIndex(config.wav_sample_rate);
	hboxRate->addWidget(comRate);
	spc = new QSpacerItem(1,1,QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
	hboxRate->addSpacerItem(spc);

	QHBoxLayout *hboxBits = new QHBoxLayout();
	vboxSaveWav->addLayout(hboxBits);
	lbl = new MyLabel(CMsg::Sample_Bits);
	hboxBits->addWidget(lbl);
	comBits = new MyComboBox(nullptr, LABELS::sound_bits);
	comBits->setCurrentIndex(config.wav_sample_bits);
	hboxBits->addWidget(comBits);
	spc = new QSpacerItem(1,1,QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
	hboxBits->addSpacerItem(spc);

	// fdd group
	MyGroupBox *grpFdd = new MyGroupBox(CMsg::Floppy_Disk_Drive);
	vbox2->addWidget(grpFdd);

	QVBoxLayout *vboxFdd = new QVBoxLayout(grpFdd);

	// mount drive
	QHBoxLayout *hboxMountFdd = new QHBoxLayout();
	vboxFdd->addLayout(hboxMountFdd);

	lbl = new MyLabel(CMsg::When_start_up_mount_disk_at_);
	hboxMountFdd->addWidget(lbl);
	for(int i=0; i<MAX_DRIVE; i++) {
		chkDrive[i] = new QCheckBox(QString::number(i));
		chkDrive[i]->setChecked((config.mount_fdd & (1 << i)) != 0);
		hboxMountFdd->addWidget(chkDrive[i]);
	}
	spc = new QSpacerItem(1,1,QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
	hboxMountFdd->addSpacerItem(spc);

	chkDelayFd1 = new MyCheckBox(CMsg::Ignore_delays_to_find_sector);
	chkDelayFd1->setChecked(FLG_DELAY_FDSEARCH != 0);
	vboxFdd->addWidget(chkDelayFd1);
	chkDelayFd2 = new MyCheckBox(CMsg::Ignore_delays_to_seek_track);
	chkDelayFd2->setChecked(FLG_DELAY_FDSEEK != 0);
	vboxFdd->addWidget(chkDelayFd2);
	chkFdDensity = new MyCheckBox(CMsg::Suppress_checking_for_density);
	chkFdDensity->setChecked(FLG_CHECK_FDDENSITY == 0);
	vboxFdd->addWidget(chkFdDensity);
	chkFdMedia = new MyCheckBox(CMsg::Suppress_checking_for_media_type);
	chkFdMedia->setChecked(FLG_CHECK_FDMEDIA == 0);
	vboxFdd->addWidget(chkFdMedia);
	spc = new QSpacerItem(1,1,QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
	vboxFdd->addSpacerItem(spc);

	spc = new QSpacerItem(1,1,QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
	vbox2->addSpacerItem(spc);

	// network tab
	QWidget *tab3 = new QWidget();
	tab->addTab(tab3, CMsg::Network);

	QVBoxLayout *vbox3 = new QVBoxLayout(tab3);

	QGridLayout *gbox3 = new QGridLayout();
	vbox3->addLayout(gbox3);

	lbl = new MyLabel(CMsg::Hostname);
	gbox3->addWidget(lbl, 0, 1);
	lbl = new MyLabel(CMsg::_Port);
	gbox3->addWidget(lbl, 0, 2);

	int row = 1;
	for(int i=0; i<MAX_PRINTER; i++) {
		lbl = new QLabel(tr("LPT") + QString::number(i));
		gbox3->addWidget(lbl, row, 0);
		linLPTHost[i] = new QLineEdit();
		linLPTHost[i]->setText(QTChar::fromTChar(config.printer_server_host[i].Get()));
		linLPTHost[i]->setMinimumSize(128, 0);
		gbox3->addWidget(linLPTHost[i], row, 1);
		linLPTPort[i] = new QLineEdit();
		linLPTPort[i]->setText(QString::number(config.printer_server_port[i]));
		linLPTPort[i]->setMinimumSize(64, 0);
		gbox3->addWidget(linLPTPort[i], row, 2);
		QHBoxLayout *hbox = new QHBoxLayout();
		lbl = new MyLabel(CMsg::_Print_delay);
		hbox->addWidget(lbl);
		linLPTDelay[i] = new QLineEdit();
		linLPTDelay[i]->setText(QString::number(config.printer_delay[i], 'f', 1));
		linLPTDelay[i]->setMinimumSize(32, 0);
		linLPTDelay[i]->setFixedWidth(32);
		hbox->addWidget(linLPTDelay[i]);
		lbl = new MyLabel(CMsg::msec);
		hbox->addWidget(lbl);
		gbox3->addLayout(hbox, row, 3);

		row++;
	}

	for(int i=0; i<MAX_COMM; i++) {
		lbl = new QLabel(tr("COM") + QString::number(i));
		gbox3->addWidget(lbl, row, 0);
		linCOMHost[i] = new QLineEdit();
		linCOMHost[i]->setText(QTChar::fromTChar(config.comm_server_host[i].Get()));
		linCOMHost[i]->setMinimumSize(128, 0);
		gbox3->addWidget(linCOMHost[i], row, 1);
		linCOMPort[i] = new QLineEdit();
		linCOMPort[i]->setText(QString::number(config.comm_server_port[i]));
		linCOMPort[i]->setMinimumSize(64, 0);
		gbox3->addWidget(linCOMPort[i], row, 2);
		comCOMBaud[i] = new MyComboBox(nullptr, LABELS::comm_baud, config.comm_dipswitch[i] - 1);
		gbox3->addWidget(comCOMBaud[i], row, 3);

		row++;
	}
	spc = new QSpacerItem(1,1, QSizePolicy::MinimumExpanding);
	gbox3->addItem(spc, row, 4);

#ifdef USE_DEBUGGER
	lbl = new MyLabel(CMsg::Connectable_host_to_Debugger);
	vbox3->addWidget(lbl);

	QHBoxLayout *hbox3 = new QHBoxLayout();
	vbox3->addLayout(hbox3);
	lbl = new MyLabel(CMsg::Hostname);
	hbox3->addWidget(lbl);
	QLineEdit *linHost = new QLineEdit();
	linHost->setText(QTChar::fromTChar(config.debugger_server_host.Get()));
	linHost->setMinimumSize(128, 0);
	hbox3->addWidget(linHost);
	lbl = new MyLabel(CMsg::_Port);
	hbox3->addWidget(lbl);
	QLineEdit *linPort = new QLineEdit();
	linPort->setText(QString::number(config.debugger_server_port));
	linPort->setMinimumSize(64, 0);
	hbox3->addWidget(linPort);
	spc = new QSpacerItem(1,1, QSizePolicy::MinimumExpanding);
	hbox3->addSpacerItem(spc);
#endif
	// uart
	QHBoxLayout *hbox31 = new QHBoxLayout();
	vbox3->addLayout(hbox31);
	MyGroupBox *grpUart = new MyGroupBox(CMsg::Settings_of_serial_ports_on_host);
	hbox31->addWidget(grpUart);
	QVBoxLayout *vboxUart = new QVBoxLayout(grpUart);
	QHBoxLayout *hboxUart = new QHBoxLayout();
	vboxUart->addLayout(hboxUart);
	lbl = new MyLabel(CMsg::Baud_Rate);
	hboxUart->addWidget(lbl);
	MyComboBox *comUartBaud = new MyComboBox(nullptr, LABELS::comm_uart_baudrate);
	int match = 0;
	for(int i=0; LABELS::comm_uart_baudrate[i] != NULL; i++) {
		if (config.comm_uart_baudrate == (int)_tcstol(LABELS::comm_uart_baudrate[i], NULL, 10)) {
			match = i;
			break;
		}
	}
	comUartBaud->setCurrentIndex(match);
	hboxUart->addWidget(comUartBaud);
	hboxUart = new QHBoxLayout();
	vboxUart->addLayout(hboxUart);
	lbl = new MyLabel(CMsg::Data_Bit);
	hboxUart->addWidget(lbl);
	MyComboBox *comUartDataBit = new MyComboBox(nullptr, LABELS::comm_uart_databit);
	comUartDataBit->setCurrentIndex(config.comm_uart_databit - 7);
	hboxUart->addWidget(comUartDataBit);
	hboxUart = new QHBoxLayout();
	vboxUart->addLayout(hboxUart);
	lbl = new MyLabel(CMsg::Parity);
	hboxUart->addWidget(lbl);
	MyComboBox *comUartParity = new MyComboBox(nullptr, LABELS::comm_uart_parity, config.comm_uart_parity);
	hboxUart->addWidget(comUartParity);
	hboxUart = new QHBoxLayout();
	vboxUart->addLayout(hboxUart);
	lbl = new MyLabel(CMsg::Stop_Bit);
	hboxUart->addWidget(lbl);
	MyComboBox *comUartStopBit = new MyComboBox(nullptr, LABELS::comm_uart_stopbit);
	comUartStopBit->setCurrentIndex(config.comm_uart_stopbit);
	hboxUart->addWidget(comUartStopBit);
	hboxUart = new QHBoxLayout();
	vboxUart->addLayout(hboxUart);
	lbl = new MyLabel(CMsg::Flow_Control);
	hboxUart->addWidget(lbl);
	MyComboBox *comUartFlowCtrl = new MyComboBox(nullptr, LABELS::comm_uart_flowctrl, config.comm_uart_flowctrl);
	hboxUart->addWidget(comUartFlowCtrl);
	lbl = new MyLabel(CMsg::Need_re_connect_to_serial_port_when_modified_this);
	vboxUart->addWidget(lbl);
	spc = new QSpacerItem(1,1, QSizePolicy::MinimumExpanding);
	hbox31->addSpacerItem(spc);

	spc = new QSpacerItem(1,1,QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
	vbox3->addSpacerItem(spc);

	// CPU, memory tab
	QWidget *tab4 = new QWidget();
	tab->addTab(tab4, CMsg::CPU_Memory);

	QVBoxLayout *vbox4 = new QVBoxLayout(tab4);

	// ROM
	QHBoxLayout *hboxRomPath = new QHBoxLayout();
	vbox4->addLayout(hboxRomPath);
	lbl = new MyLabel(CMsg::ROM_Path_ASTERISK);
	hboxRomPath->addWidget(lbl);
	linRomPath = new QLineEdit();
	linRomPath->setText(QTChar::fromTChar(config.rom_path.Get()));
	linRomPath->setMinimumSize(368, 0);
	hboxRomPath->addWidget(linRomPath);
	MyPushButton *btnRomPath = new MyPushButton(CMsg::Folder_);
	connect(btnRomPath, SIGNAL(pressed()), this, SLOT(pressedRomPath()));
	hboxRomPath->addWidget(btnRomPath);
	spc = new QSpacerItem(1,1, QSizePolicy::MinimumExpanding);
	hboxRomPath->addSpacerItem(spc);

	// ex ram
#if defined(_MBS1)
	const CMsg::Id exram_labels[] = { CMsg::None_, CMsg::S64KB, CMsg::S128KB, CMsg::S256KB, CMsg::S512KB, CMsg::End };
	QHBoxLayout *hboxExRam = new QHBoxLayout();
	vbox4->addLayout(hboxExRam);
	lbl = new MyLabel(CMsg::Extended_RAM_ASTERISK);
	hboxExRam->addWidget(lbl);
	comExMem = new MyComboBox();
	for(int n=0; exram_labels[n] != CMsg::End; n++) {
		comExMem->addItemById(exram_labels[n]);
	}
	comExMem->setCurrentIndex(emu->get_parami(VM::ParamExMemNum));
	hboxExRam->addWidget(comExMem);
	lbl = new MyLabel(CMsg::LB_Now_SP);
	hboxExRam->addWidget(lbl);
	lbl = new MyLabel(exram_labels[config.exram_size_num]);
	hboxExRam->addWidget(lbl);
	lbl = new QLabel(")");
	hboxExRam->addWidget(lbl);
	spc = new QSpacerItem(1,1, QSizePolicy::MinimumExpanding);
	hboxExRam->addSpacerItem(spc);

	chkMemNoWait = new MyCheckBox(CMsg::No_wait_to_access_the_main_memory);
	chkMemNoWait->setChecked(config.mem_nowait);
	vbox4->addWidget(chkMemNoWait);
#else
	chkExMem = new MyCheckBox(CMsg::Use_Extended_Memory_64KB);
	chkExMem->setChecked(config.exram_size_num != 0);
	vbox4->addWidget(chkExMem);
#endif

	// undef op
	chkUndefOp = new MyCheckBox(CMsg::Show_message_when_the_CPU_fetches_undefined_opcode);
	chkUndefOp->setChecked(FLG_SHOWMSG_UNDEFOP != 0);
	vbox4->addWidget(chkUndefOp);

#if defined(_MBS1)
# if defined(USE_Z80B_CARD)
	// Interrupt of FM OPN
	const CMsg::Id z80b_card_irq_labels[] = { CMsg::IRQ, CMsg::NMI, CMsg::End };
	QHBoxLayout *hboxZ80BIntr = new QHBoxLayout();
	vbox4->addLayout(hboxZ80BIntr);
	lbl = new MyLabel(CMsg::Connect_interrupt_signal_of_Z80B_Card_to_ASTERISK);
	hboxZ80BIntr->addWidget(lbl);
	comZ80BIntr = new MyComboBox();
	for(int n=0; z80b_card_irq_labels[n] != CMsg::End; n++) {
		comZ80BIntr->addItemById(z80b_card_irq_labels[n]);
	}
	comZ80BIntr->setCurrentIndex(config.z80b_card_out_irq);
	hboxZ80BIntr->addWidget(comZ80BIntr);
	spc = new QSpacerItem(1,1, QSizePolicy::MinimumExpanding);
	hboxZ80BIntr->addSpacerItem(spc);
# elif defined(USE_MPC_68008)
	chkAddrErr = new MyCheckBox(CMsg::Show_message_when_the_address_error_occured_in_MC68008);
	chkAddrErr->setChecked(FLG_SHOWMSG_ADDRERR != 0);
	vbox4->addWidget(chkAddrErr);
# endif
#endif

	lbl = new MyLabel(CMsg::Need_restart_program_or_PowerOn);
	vbox4->addWidget(lbl);

	spc = new QSpacerItem(1,1,QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
	vbox4->addSpacerItem(spc);

#if defined(_MBS1)
	// Sound tab
	QWidget *tab5 = new QWidget();
	tab->addTab(tab5, CMsg::Sound);

	QVBoxLayout *vbox5 = new QVBoxLayout(tab5);

	// FM OPN
	MyGroupBox *grpFmopn = new MyGroupBox(CMsg::FM_Synthesis_Card_ASTERISK);
	vbox5->addWidget(grpFmopn);
	QVBoxLayout *vboxFmopn = new QVBoxLayout(grpFmopn);
	chkFmopn = new MyCheckBox(CMsg::Enable);
	chkFmopn->setChecked(IOPORT_USE_FMOPN != 0);
	vboxFmopn->addWidget(chkFmopn);
	lbl = new MyLabel(CMsg::IO_ports_are_FF1E_FF1F_FF16_FF17);
	vboxFmopn->addWidget(lbl);
	QHBoxLayout *hboxFmopn = new QHBoxLayout();
	lbl = new MyLabel(CMsg::Sound_chip);
	hboxFmopn->addWidget(lbl);
	comFmopn = new MyComboBox(nullptr, LABELS::type_of_soundcard, emu->get_parami(VM::ParamChipTypeOnFmOpn), config.type_of_fmopn, CMsg::LB_Now_RB);
	hboxFmopn->addWidget(comFmopn);
	spc = new QSpacerItem(1, 1, QSizePolicy::MinimumExpanding);
	hboxFmopn->addSpacerItem(spc);
	vboxFmopn->addLayout(hboxFmopn);
	connect(chkFmopn, SIGNAL(toggled(bool)), this, SLOT(toggledFmopn(bool)));

	// Ex PSG
	MyGroupBox *grpExpsg = new MyGroupBox(CMsg::Extended_PSG_Port_ASTERISK);
	vbox5->addWidget(grpExpsg);
	QVBoxLayout *vboxExpsg = new QVBoxLayout(grpExpsg);
	chkExpsg = new MyCheckBox(CMsg::Enable);
	chkExpsg->setChecked(IOPORT_USE_EXPSG != 0);
	vboxExpsg->addWidget(chkExpsg);
	lbl = new MyLabel(CMsg::IO_ports_are_FFE6_FFE7_FFEE_FFEF);
	vboxExpsg->addWidget(lbl);
	QHBoxLayout *hboxExpsg = new QHBoxLayout();
	lbl = new MyLabel(CMsg::Sound_chip);
	hboxExpsg->addWidget(lbl);
	comExpsg = new MyComboBox(nullptr, LABELS::type_of_soundcard, emu->get_parami(VM::ParamChipTypeOnExPsg), config.type_of_expsg, CMsg::LB_Now_RB);
	hboxExpsg->addWidget(comExpsg);
	spc = new QSpacerItem(1, 1, QSizePolicy::MinimumExpanding);
	hboxExpsg->addSpacerItem(spc);
	vboxExpsg->addLayout(hboxExpsg);
	connect(chkExpsg, SIGNAL(toggled(bool)), this, SLOT(toggledExpsg(bool)));

	// Interrupt of FM OPN
	QHBoxLayout *hboxFMIntr = new QHBoxLayout();
	vbox5->addLayout(hboxFMIntr);
	lbl = new MyLabel(CMsg::Connect_interrupt_signal_of_FM_synthesis_to_ASTERISK_ASTERISK);
	hboxFMIntr->addWidget(lbl);
	comFMIntr = new MyComboBox();
	comFMIntr->addItemsById(LABELS::fmopn_irq);
	comFMIntr->setCurrentIndex(config.opn_irq);
	hboxFMIntr->addWidget(comFMIntr);
	spc = new QSpacerItem(1,1, QSizePolicy::MinimumExpanding);
	hboxFMIntr->addSpacerItem(spc);
	//
	lbl = new MyLabel(CMsg::Need_restart_program);
	vbox5->addWidget(lbl);
	lbl = new MyLabel(CMsg::This_is_the_common_setting_both_FM_synthesis_card_and_extended_PSG_port);
	vbox5->addWidget(lbl);

	spc = new QSpacerItem(1,1,QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
	vbox5->addSpacerItem(spc);
#endif

	// button

	QDialogButtonBox *btn = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	vbox_all->addWidget(btn);

	connect(btn, SIGNAL(accepted()), this, SLOT(accept()));
	connect(btn, SIGNAL(rejected()), this, SLOT(reject()));
}

MyConfigBox::~MyConfigBox()
{
//	delete ui;
}

void MyConfigBox::toggledIOPort(bool checked)
{
	QCheckBox *chk = dynamic_cast<QCheckBox *>(sender());
	if (checked) {
		// 9PSG and KANJI are exclusive.
		if (chk == chkIOPorts[5]) {
			chkIOPorts[6]->setChecked(false);
		} else if (chk == chkIOPorts[6]) {
			chkIOPorts[5]->setChecked(false);
		}
	}
#if defined(_MBS1)
	if (chk == chkIOPorts[IOPORT_POS_EXPSG]) {
		chkExpsg->setChecked(chk->isChecked());
	}
	if (chk == chkIOPorts[IOPORT_POS_FMOPN]) {
		chkFmopn->setChecked(chk->isChecked());
	}
#endif
}

void MyConfigBox::toggledFmopn(bool checked)
{
	chkIOPorts[IOPORT_POS_FMOPN]->setChecked(checked);
}

void MyConfigBox::toggledExpsg(bool checked)
{
	chkIOPorts[IOPORT_POS_EXPSG]->setChecked(checked);
}

void MyConfigBox::pressedSnapPath()
{
	QString dir = QTChar::fromTChar(config.snapshot_path.Get());
	QFileDialog dlg(this, CMSG(Select_a_folder_to_save_snapshot_images), dir);
	dlg.setAcceptMode(QFileDialog::AcceptOpen);
	dlg.setFileMode(QFileDialog::Directory);	// select a folder only
	int rc = dlg.exec();
	if (rc != QDialog::Accepted) {
		return;
	}
	linSnapPath->setText(dlg.directory().absolutePath());
}

void MyConfigBox::pressedMsgFont()
{
	QFont font;
	font.setFamily(linMsgFont->text());
	font.setPointSize(linMsgSize->text().toInt());

	QFontDialog dlg(font, this);
	int rc = dlg.exec();
	if (rc != QDialog::Accepted) {
		return;
	}
	font = dlg.selectedFont();
	linMsgFont->setText(font.family());
	linMsgSize->setText(QString::number(font.pointSize()));
}

void MyConfigBox::pressedInfoFont()
{
	QFont font;
	font.setFamily(linInfoFont->text());
	font.setPointSize(linInfoSize->text().toInt());

	QFontDialog dlg(font, this);
	int rc = dlg.exec();
	if (rc != QDialog::Accepted) {
		return;
	}
	font = dlg.selectedFont();
	linInfoFont->setText(font.family());
	linInfoSize->setText(QString::number(font.pointSize()));
}

void MyConfigBox::pressedRomPath()
{
	QString dir = QTChar::fromTChar(config.rom_path.Get());
	QFileDialog dlg(this, "Select a folder containing the rom images.", dir);
	dlg.setAcceptMode(QFileDialog::AcceptOpen);
	dlg.setFileMode(QFileDialog::Directory);	// select a folder only
	int rc = dlg.exec();
	if (rc != QDialog::Accepted) {
		return;
	}
	linRomPath->setText(dlg.directory().absolutePath());
}

int MyConfigBox::exec()
{
	int rc = QDialog::exec();
	if (rc == QDialog::Accepted) {
		// set datas
		setDatas();
	}
	return rc;
}

void MyConfigBox::setDatas()
{
	int fdd_type = 0;
	int io_port = 0;
	int mount_fdd = 0;
	int value = 0;

	// Mode Tab

	// Power Off
	config.use_power_off = chkPowerOff->isChecked();
	// MODE Switch
#if defined(_MBS1)
	int sys_mode = (config.sys_mode & ~1);
	sys_mode |= (radSysMode[0]->isChecked() ? 1 : 0);
	emu->set_parami(VM::ParamSysMode, sys_mode);
	config.dipswitch = (chkDipSwitch->isChecked() ? config.dipswitch | 4 : config.dipswitch & ~4);
#else
	config.dipswitch = (chkModeSwitch->isChecked() ? config.dipswitch | 4 : config.dipswitch & ~4);
#endif
	// FDD Type
	for(int i=0; i<4; i++) {
		if (radFddTypes[i]->isChecked()) {
			fdd_type = i;
			break;
		}
	}
	emu->set_parami(VM::ParamFddType, fdd_type);
	// I/O Port
	for(int i = 0; LABELS::io_port[i] != CMsg::End; i++) {
		int pos = LABELS::io_port_pos[i];
		if ((1 << pos) & IOPORT_MSK_ALL) {
			io_port |= chkIOPorts[pos]->isChecked() ? (1 << pos) : 0;
		}
	}
	emu->set_parami(VM::ParamIOPort, io_port);

	// Screen tab

	// use opengl
	config.use_opengl = comUseOpenGL->currentIndex() & 0xff;
	// opengl filter
	config.gl_filter_type = comGLFilter->currentIndex() & 0xff;
#if defined(_MBS1)
	// CRTC disptmg
	config.disptmg_skew = static_cast<int8_t>(comCRTCdisptmg->currentIndex()) - 2;
	// CRTC curdisp
	config.curdisp_skew = static_cast<int8_t>(comCRTCcurdisp->currentIndex()) - 2;
#else
	// CRTC disptmg
	config.disptmg_skew = static_cast<int8_t>(comCRTCdisptmg->currentIndex());
	// CRTC curdisp
	config.curdisp_skew = static_cast<int8_t>(comCRTCcurdisp->currentIndex());
#endif
	// LED
	int led_show = comLED->currentIndex();
	// LED position
	config.led_pos = comLEDPos->currentIndex() & 0xff;
	// capture type
	config.capture_type = comCapType->currentIndex() & 0xff;
	// Snapshot path
	QTChar snapshot_path(linSnapPath->text());
	config.snapshot_path.Set(snapshot_path.toTChar());
	// message font
	QTChar msg_fontname(linMsgFont->text());
	config.msgboard_msg_fontname.Set(msg_fontname.toTChar());
	value = linMsgSize->text().toInt();
	if (1 <= value && value <= 60) {
		config.msgboard_msg_fontsize = value & 0xff;
	}
	// info font
	QTChar info_fontname(linInfoFont->text());
	config.msgboard_info_fontname.Set(info_fontname.toTChar());
	value = linInfoSize->text().toInt();
	if (1 <= value && value <= 60) {
		config.msgboard_info_fontsize = value & 0xff;
	}
	// language
	value = comLanguage->currentIndex();
	clocale->ChooseLocaleName(lang_list, value, config.language);

	// Tape, FDD tab

	config.wav_reverse = chkReverse->isChecked();
	config.wav_half = chkHalf->isChecked();
	config.wav_correct = !radNoCorr->isChecked();
	config.wav_correct_type = radCOS->isChecked() ? 0 :
		radSIN->isChecked() ? 1 : 0;
	for(int i=0; i<2; i++) {
		value = linCorrAmp[i]->text().toInt();
		if (100 <= value && value <= 5000) {
			config.wav_correct_amp[i] = value;
		}
	}
	config.wav_sample_rate = comRate->currentIndex() & 0xff;
	config.wav_sample_bits = comBits->currentIndex() & 0xff;

	// mount drive
	for(int i=0; i<MAX_DRIVE; i++) {
		mount_fdd |= chkDrive[i]->isChecked() ? (1 << i) : 0;
	}
	config.mount_fdd = mount_fdd;

	config.delay_fdd = (chkDelayFd1->isChecked() ? MSK_DELAY_FDSEARCH : 0)
		| (chkDelayFd2->isChecked() ? MSK_DELAY_FDSEEK : 0);
	config.check_fdmedia = (chkFdDensity->isChecked() ? 0 : MSK_CHECK_FDDENSITY)
		| (chkFdMedia->isChecked() ? 0 : MSK_CHECK_FDMEDIA);

	// Network tab

	for(int i=0; i<MAX_PRINTER; i++) {
		QTChar lpt_host(linLPTHost[i]->text());
		config.printer_server_host[i].Set(lpt_host.toTChar());
		value = linLPTPort[i]->text().toInt();
		if (0 <= value && value <= 65535) {
			config.printer_server_port[i] = value;
		}
		double valued = 0.0;
		valued = linLPTDelay[i]->text().toDouble();
		if (valued < 0.1) valued = 0.1;
		if (valued > 1000.0) valued = 1000.0;
		valued = floor(valued * 10.0 + 0.5) / 10.0;
		config.printer_delay[i] = valued;
	}
	for(int i=0; i<MAX_COMM; i++) {
		QTChar com_host(linCOMHost[i]->text());
		config.comm_server_host[i].Set(com_host.toTChar());
		value = linCOMPort[i]->text().toInt();
		if (0 <= value && value <= 65535) {
			config.comm_server_port[i] = value;
		}
		config.comm_dipswitch[i] = comCOMBaud[i]->currentIndex() + 1;
	}

	// CPU, Memory tab

	// ROM
	QTChar rom_path(linRomPath->text());
	config.rom_path.Set(rom_path.toTChar());
	// ex ram
#if defined(_MBS1)
	emu->set_parami(VM::ParamExMemNum, comExMem->currentIndex() & 0xff);
	config.mem_nowait = chkMemNoWait->isChecked();
#else
	config.exram_size_num = chkExMem->isChecked() ? 1 : 0;
#endif
	//
	BIT_ONOFF(config.misc_flags, MSK_SHOWMSG_UNDEFOP, chkUndefOp->isChecked());

#if defined(_MBS1)
# if defined(USE_Z80B_CARD)
	config.z80b_card_out_irq = comZ80BIntr->currentIndex();
# elif defined(USE_MPC_68008)
	BIT_ONOFF(config.misc_flags, MSK_SHOWMSG_ADDRERR, chkAddrErr->isChecked());
# endif
	config.opn_irq = comFMIntr->currentIndex();
	emu->set_parami(VM::ParamChipTypeOnFmOpn, comFmopn->currentIndex());
	emu->set_parami(VM::ParamChipTypeOnExPsg, comExpsg->currentIndex());
#endif

	gui->ChangeLedBox(led_show);
	gui->ChangeLedBoxPosition(config.led_pos);
}
