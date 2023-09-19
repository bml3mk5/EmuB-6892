/** @file qt_gui.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2016.11.18

	@brief [ qt_gui ]
*/

#include "../../main.h"
#include "qt_gui.h"
#include "ui_qt_gui.h"
#include <QScreen>
#include <QPainter>
#include <QVariant>
#include <QFileDialog>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QClipboard>
#include "../../emu_osd.h"
#include "../../utils.h"
#include "../../parseopt.h"
#include "../../utility.h"
#include "../../config.h"
#include "../../depend.h"
#include "qt_volumebox.h"
#include "qt_keybindbox.h"
#include "qt_configbox.h"
#include "qt_recvidbox.h"
#include "qt_recaudbox.h"
#include "qt_aboutbox.h"
#ifdef USE_LEDBOX
#include "../ledbox.h"
#endif
#ifdef USE_VKEYBOARD
#include "../vkeyboard.h"
#endif
#ifdef USE_SOCKET
#include "../../osd/qt/qt_socket.h"
#endif
#ifdef USE_UART
#include "../../osd/qt/qt_uart.h"
#endif

MainWindow *mainwindow = nullptr;

//
//
//

GUI::GUI(int argc, char **argv, EMU *new_emu) :
	GUI_BASE(argc, argv, new_emu)
{

}

/// overloaded function
bool GUI::NeedUpdateScreen()
{
	if (need_update_screen > 0) {
		need_update_screen = 0;
		// request update to MainWindow
#ifdef USE_QT_UPDATE
		QWidget *c = mainwindow->centralWidget();
		if (c) c->update();
#endif
		return true;
	} else {
		return false;
	}
}

void GUI::UpdatedScreen()
{
	need_update_screen = 6;
}

void GUI::ScreenModeChanged(bool UNUSED_PARAM(fullscreen))
{
}

void GUI::PreProcessEvent()
{
	if (need_update_screen <= 0) {
		UpdateScreen();
	}
}

void GUI::PostCommandMessage(int id, void *data1, void *data2)
{
	MyUserEvent *e = new MyUserEvent(QEvent::User);
	e->code = id;
	e->data1 = data1;
	e->data2 = data2;
	QApplication::postEvent(mainwindow, e);
}

QString GUI::replaceSemicolon(const QString &str)
{
	QString nstr = str;
	nstr = nstr.replace(QChar(';'), QChar(' '));
	return nstr;
}

bool GUI::ShowLoadStateDialog(void)
{
	QString dir = QTChar::fromTChar(config.initial_state_path.Get());
	QStringList filters;
	filters << replaceSemicolon(CMSG(Supported_Files_l3r))
			<< CMSG(All_Files_);

	QFileDialog dlg(mainwindow, CMSG(Load_Status_Data), dir);
	dlg.setAcceptMode(QFileDialog::AcceptOpen);
	dlg.setNameFilters(filters);
	dlg.setFileMode(QFileDialog::ExistingFile);	// select only one file
	PostEtSystemPause(true);
	int rc = dlg.exec();
	if (rc != QDialog::Accepted) {
		PostEtSystemPause(false);
		return false;
	}
	QTChar path(dlg.selectedFiles().at(0));
	PostEtLoadStatusMessage(path.toTChar());
	return true;
}

bool GUI::ShowSaveStateDialog(bool cont)
{
	QString dir = QTChar::fromTChar(config.initial_state_path.Get());
	QStringList filters;
	filters << replaceSemicolon(CMSG(Supported_Files_l3r))
			<< CMSG(All_Files_);

	QFileDialog dlg(mainwindow, CMSG(Save_Status_Data), dir);
	dlg.setAcceptMode(QFileDialog::AcceptSave);
	dlg.setNameFilters(filters);
	dlg.setFileMode(QFileDialog::ExistingFile);	// select only one file
	PostEtSystemPause(true);
	int rc = dlg.exec();
	if (rc != QDialog::Accepted) {
		PostEtSystemPause(false);
		return false;
	}
	QTChar path(dlg.selectedFiles().at(0));
	PostEtSaveStatusMessage(path.toTChar(), cont);
	return true;
}

bool GUI::ShowOpenAutoKeyDialog(void)
{
	QString dir = QTChar::fromTChar(config.initial_autokey_path.Get());
	QStringList filters;
	filters << replaceSemicolon(CMSG(Supported_Files_txt_bas_lpt))
			<< CMSG(All_Files_);

	QFileDialog dlg(mainwindow, CMSG(Open_Text_File), dir);
	dlg.setAcceptMode(QFileDialog::AcceptOpen);
	dlg.setNameFilters(filters);
	dlg.setFileMode(QFileDialog::ExistingFile);	// select only one file
	PostEtSystemPause(true);
	int rc = dlg.exec();
	if (rc != QDialog::Accepted) {
		PostEtSystemPause(false);
		return false;
	}
	QTChar path(dlg.selectedFiles().at(0));
	PostEtLoadAutoKeyMessage(path.toTChar());
	return true;
}

bool GUI::ShowPlayRecKeyDialog(void)
{
	QString dir = QTChar::fromTChar(config.initial_state_path.Get());
	QStringList filters;
	filters << replaceSemicolon(CMSG(Supported_Files_l3k))
			<< CMSG(All_Files_);

	QFileDialog dlg(mainwindow, CMSG(Play_Recorded_Keys), dir);
	dlg.setAcceptMode(QFileDialog::AcceptOpen);
	dlg.setNameFilters(filters);
	dlg.setFileMode(QFileDialog::ExistingFile);	// select only one file
	PostEtSystemPause(true);
	int rc = dlg.exec();
	if (rc != QDialog::Accepted) {
		PostEtSystemPause(false);
		return false;
	}
	QTChar path(dlg.selectedFiles().at(0));
	PostEtLoadRecKeyMessage(path.toTChar());
	return true;
}

bool GUI::ShowRecordRecKeyDialog(void)
{
	QString dir = QTChar::fromTChar(config.initial_state_path.Get());
	QStringList filters;
	filters << replaceSemicolon(CMSG(Supported_Files_l3k))
			<< CMSG(All_Files_);

	QFileDialog dlg(mainwindow, CMSG(Record_Input_Keys), dir);
	dlg.setAcceptMode(QFileDialog::AcceptSave);
	dlg.setNameFilters(filters);
	dlg.setFileMode(QFileDialog::ExistingFile);	// select only one file
	PostEtSystemPause(true);
	int rc = dlg.exec();
	if (rc != QDialog::Accepted) {
		PostEtSystemPause(false);
		return false;
	}
	QTChar path(dlg.selectedFiles().at(0));
	PostEtSaveRecKeyMessage(path.toTChar(), false);
	return true;
}

bool GUI::ShowRecordStateAndRecKeyDialog(void)
{
	bool rc = ShowSaveStateDialog(true);
	if (!rc) return rc;
	return ShowRecordRecKeyDialog();
}

bool GUI::ShowLoadDataRecDialog(void)
{
	QString dir = QTChar::fromTChar(config.initial_datarec_path.Get());
	QStringList filters;
	filters << replaceSemicolon(CMSG(Supported_Files_l3_l3b_l3c_wav_t9x))
			<< CMSG(All_Files_);

	QFileDialog dlg(mainwindow, CMSG(Play_Data_Recorder_Tape), dir);
	dlg.setAcceptMode(QFileDialog::AcceptOpen);
	dlg.setNameFilters(filters);
	dlg.setFileMode(QFileDialog::ExistingFile);	// select only one file
	PostEtSystemPause(true);
	int rc = dlg.exec();
	if (rc != QDialog::Accepted) {
		PostEtSystemPause(false);
		return false;
	}
	QTChar path(dlg.selectedFiles().at(0));
	PostEtLoadDataRecMessage(path.toTChar());
	return true;
}

bool GUI::ShowSaveDataRecDialog(void)
{
	QString dir = QTChar::fromTChar(config.initial_datarec_path.Get());
	QStringList filters;
	filters << "L3 File (*.l3)"
			<< "L3B File (*.l3b)"
			<< "L3C File (*.l3c)"
			<< "Wave File (*.wav)"
			<< "T9X File (*.t9x)"
			<< "All Files (*.*)";

	QFileDialog dlg(mainwindow, "Record Data Recorder Tape", dir);
	dlg.setAcceptMode(QFileDialog::AcceptSave);
	dlg.setNameFilters(filters);
	dlg.setFileMode(QFileDialog::ExistingFile);	// select only one file
	PostEtSystemPause(true);
	int rc = dlg.exec();
	if (rc != QDialog::Accepted) {
		PostEtSystemPause(false);
		return false;
	}
	QTChar path(dlg.selectedFiles().at(0));
	PostEtSaveDataRecMessage(path.toTChar());
	return true;
}

bool GUI::ShowOpenFloppyDiskDialog(int drv)
{
	QString dir = QTChar::fromTChar(config.initial_disk_path.Get());
	QStringList filters;
	filters << replaceSemicolon(CMSG(Supported_Files_d88_d77_td0_imd_dsk_fdi_hdm_tfd_xdf_2d_sf7))
			<< "All Files (*.*)";

	QString title = QString::asprintf("Open Floppy Disk %d", drv);
	QFileDialog dlg(mainwindow, title, dir);
	dlg.setAcceptMode(QFileDialog::AcceptOpen);
	dlg.setNameFilters(filters);
	dlg.setFileMode(QFileDialog::ExistingFile);	// select only one file
	PostEtSystemPause(true);
	int rc = dlg.exec();
	if (rc != QDialog::Accepted) {
		PostEtSystemPause(false);
		return false;
	}
	QTChar path(dlg.selectedFiles().at(0));
	uint32_t flags = dlg.isReadOnly() ? 1 : 0;
	PostEtOpenFloppyMessage(drv, path.toTChar(), 0, flags, true);
	return true;
}

bool GUI::ShowOpenBlankFloppyDiskDialog(int drv, uint8_t type)
{
	QString dir = QTChar::fromTChar(config.initial_disk_path.Get());
	QStringList filters;
	filters << "Supported Files (*.d88 *.d77)"
			<< "All Files (*.*)";

	QString title = QString::asprintf("New Floppy Disk %d", drv);
	QFileDialog dlg(mainwindow, title, dir);
	dlg.setAcceptMode(QFileDialog::AcceptSave);
	dlg.setNameFilters(filters);
	dlg.setFileMode(QFileDialog::ExistingFile);	// select only one file

	_TCHAR file_name[128];
	UTILITY::create_date_file_path(nullptr, file_name, 128, "d88");
	dlg.selectFile(file_name);

	PostEtSystemPause(true);
	int sts = dlg.exec();
	bool rc = (sts == QDialog::Accepted);
	if (rc) {
		QTChar path(dlg.selectedFiles().at(0));
		rc = emu->create_blank_floppy_disk(path.toTChar(), type);
	 }
	if (rc) {
		QTChar path(dlg.selectedFiles().at(0));
		uint32_t flags = dlg.isReadOnly() ? 1 : 0;
		PostEtOpenFloppyMessage(drv, path.toTChar(), 0, flags, true);
	} else {
		PostEtSystemPause(false);
	}
	return rc;
}

bool GUI::ShowSavePrinterDialog(int drv)
{
	QString dir = QTChar::fromTChar(config.initial_disk_path.Get());
	QStringList filters;
	filters << replaceSemicolon(CMSG(Supported_Files_lpt))
			<< "All Files (*.*)";

	QString title = "Save Printing Data";
	QFileDialog dlg(mainwindow, title, dir);
	dlg.setAcceptMode(QFileDialog::AcceptSave);
	dlg.setNameFilters(filters);
	dlg.setFileMode(QFileDialog::ExistingFile);	// select only one file
	PostEtSystemPause(true);
	int rc = dlg.exec();
	if (rc != QDialog::Accepted) {
		PostEtSystemPause(false);
		return false;
	}
	QTChar path(dlg.selectedFiles().at(0));
	PostEtSavePrinterMessage(drv, path.toTChar());
	return true;
}

bool GUI::ShowRecordVideoDialog(int fps_num)
{
	MyRecVidBox dlg(mainwindow);
	SystemPause(true);
	int rc = dlg.exec();
	if (rc != QDialog::Accepted) {
		PostEtSystemPause(false);
		return false;
	}
	PostEtStartRecordVideo(fps_num);
	return true;
}

bool GUI::ShowRecordAudioDialog(void)
{
	MyRecAudBox dlg(mainwindow);
	SystemPause(true);
	int rc = dlg.exec();
	if (rc != QDialog::Accepted) {
		PostEtSystemPause(false);
		return false;
	}
	PostEtStartRecordSound();
	return true;
}

bool GUI::ShowRecordVideoAndAudioDialog(int fps_num)
{
	MyRecVidBox dlg1(mainwindow);
	SystemPause(true);
	int rc = dlg1.exec();
	if (rc != QDialog::Accepted) {
		PostEtSystemPause(false);
		return false;
	}
	MyRecAudBox dlg2(mainwindow);
	SystemPause(true);
	rc = dlg2.exec();
	if (rc != QDialog::Accepted) {
		PostEtSystemPause(false);
		return false;
	}
	PostEtStartRecordVideo(fps_num);
	return true;
}

bool GUI::ShowVolumeDialog(void)
{
	MyVolumeBox dlg(mainwindow);
	dlg.exec();
	return true;
}

bool GUI::ShowKeybindDialog(void)
{
	MyKeybindBox dlg(mainwindow);
	SystemPause(true);
	dlg.exec();
	SystemPause(false);
	return true;
}

bool GUI::ShowVirtualKeyboard(void)
{
#ifdef USE_VKEYBOARD
	if (!vkeyboard) {
		vkeyboard = new Vkbd::VKeyboard(mainwindow);

		uint8_t *buf;
		int siz;
		emu->get_vm_key_status_buffer(&buf, &siz);
		FIFOINT *his = emu->get_vm_key_history();
		vkeyboard->SetStatusBufferPtr(buf, siz, VM_KEY_STATUS_VKEYBOARD);
		vkeyboard->SetHistoryBufferPtr(his);
		vkeyboard->Create(emu->resource_path());
		vkeyboard->Show();
	} else {
		vkeyboard->Close();
	}
	return true;
#else
	return false;
#endif
}

bool GUI::ShowConfigureDialog(void)
{
	MyConfigBox dlg(mainwindow);
	SystemPause(true);
	dlg.exec();
	SystemPause(false);
	return true;
}

bool GUI::ShowAboutDialog(void)
{
	MyAboutBox dlg(mainwindow);
	dlg.exec();
	return true;
}

void GUI::CreateLedBoxSub()
{
	if (ledbox) {
		ledbox->SetParent(mainwindow);
	}
}

bool GUI::GetRecentFileStr(const _TCHAR *file, int num, _TCHAR *str, int trimlen)
{
	if (file == nullptr || file[0] == '\0') return false;

	UTILITY::tcscpy(str, _MAX_PATH, UTILITY::trim_center(file, trimlen));
	if (num > 0) {
		size_t len = _tcslen(str);
		UTILITY::stprintf(&str[len], _MAX_PATH - len, _T(" : %d"), num + 1);
	}
	return true;
}

/// input text automatically from clipboard
bool GUI::StartAutoKey(void)
{
	const QClipboard *clip = QApplication::clipboard();
	if (!clip) return false;

	QString text = clip->text();

	emu->start_auto_key(text.toLocal8Bit().data());
	return true;
}

//
//
//

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent,
	Qt::Window|Qt::WindowTitleHint|Qt::WindowMinimizeButtonHint|Qt::WindowCloseButtonHint|Qt::WindowSystemMenuHint
	/* |Qt::MaximizeUsingFullscreenGeometryHint */
	),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	initialize_ok = false;

	// Window title
	setWindowTitle(DEVICE_NAME);

	// window icon
	QString iconfile = QTChar::fromTChar(options->get_res_path());
	iconfile += CONFIG_NAME;
    iconfile += ".png";
//	QPixmap iconpix(iconfile);
//	QIcon icon(iconpix);
    QIcon icon(iconfile);
    setWindowIcon(icon);

	// create gui menu
//	int need_resize = gui->CreateMenu();
//	if (need_resize == -1) {
//		return;
//	}

	//
	//
	//

	// menu action
	QMenuBar *mb = ui->menuBar;
	QMenu *mn;
	QMenu *ms;
	QAction *act;

	// control menu
	mn = mb->addMenu(CMSG(Control));
	connect(mn, SIGNAL(aboutToShow()), this, SLOT(updateMenuControlSlot()));

	actionPowerOn = mn->addAction(CMSG(PowerOn), this, SLOT(resetSlot()));
	actionPowerOn->setCheckable(true);
#if !defined(_MBS1)
#ifdef USE_DIPSWITCH
	actionModeSwitch = mn->addAction(CMSG(MODE_Switch), this, SLOT(dipSwitch2Slot()));
	actionModeSwitch->setCheckable(true);
#endif
#endif
#ifdef USE_SPECIAL_RESET
	act = mn->addAction(CMSG(Reset_Switch), this, SLOT(warmResetSlot()));
#endif
#if defined(_MBS1)
	mn->addSeparator();
	ms = mn->addMenu(CMSG(System_Mode));
	CMsg::Id sysModeList[] = {
		CMsg::A_Mode_S1,
		CMsg::B_Mode_L3,
		CMsg::End
	};
	for(int i=0; sysModeList[i] != CMsg::End; i++) {
		actionSystemMode[i] = ms->addAction(CMSGV(sysModeList[i]));
		actionSystemMode[i]->setCheckable(true);
		connectWithNumber(actionSystemMode[i], SIGNAL(triggered()), this, SLOT(systemModeSlot()), i);
	}
#endif

	mn->addSeparator();

	ms = mn->addMenu(CMSG(FDD_Type));
		CMsg::Id fddTypeList[] = {
			CMsg::Non_FDD,
			CMsg::FD3inch_compact_FDD,
#if defined(_MBS1)
			CMsg::FD5inch_mini_FDD_2D_Type,
			CMsg::FD5inch_mini_FDD_2HD_Type,
#else
			CMsg::FD5inch_mini_FDD,
			CMsg::FD8inch_standard_FDD,
#endif
			CMsg::End
		};
		for(int i=0; fddTypeList[i] != CMsg::End; i++) {
			actionFddType[i] = ms->addAction(CMSGV(fddTypeList[i]));
			actionFddType[i]->setCheckable(true);
			connectWithNumber(actionFddType[i], SIGNAL(triggered()), this, SLOT(fddTypeSlot()), i);
		}

	mn->addSeparator();

	actionPause = mn->addAction(CMSG(Pause), this, SLOT(pauseSlot()));
	actionPause->setCheckable(true);

	mn->addSeparator();

	ms = mn->addMenu(CMSG(CPU_Speed));
		CMsg::Id cpuSpeedList[] = {
			CMsg::CPU_x0_5,
			CMsg::CPU_x1,
			CMsg::CPU_x2,
			CMsg::CPU_x4,
			CMsg::CPU_x8,
			CMsg::CPU_x16,
			CMsg::End
		};
		for(int i=0; cpuSpeedList[i] != CMsg::End; i++) {
			actionCPUSpeed[i] = ms->addAction(CMSGV(cpuSpeedList[i]));
			actionCPUSpeed[i]->setCheckable(true);
			connectWithNumber(actionCPUSpeed[i], SIGNAL(triggered()), this, SLOT(cpuPowerSlot()), i);
		}
		ms->addSeparator();
		actionSyncIRQ = ms->addAction(CMSG(Sync_With_CPU_Speed), this, SLOT(syncIrqSlot()));
		actionSyncIRQ->setCheckable(true);

	mn->addSeparator();

	ms = mn->addMenu(CMSG(Auto_Key));
		act = ms->addAction(CMSG(Open_), this, SLOT(playAutokeySlot()));
		act = ms->addAction(CMSG(Paste), this, SLOT(pasteAutokeySlot()));
		act = ms->addAction(CMSG(Stop), this, SLOT(stopAutokeySlot()));

	mn->addSeparator();

	ms = mn->addMenu(CMSG(Record_Key));
		actionRecordKeyPlay = ms->addAction(CMSG(Play_), this, SLOT(playReckeySlot()));
		actionRecordKeyPlay->setCheckable(true);
		act = ms->addAction(CMSG(Stop_Playing), this, SLOT(stopPlayingReckeySlot()));
		ms->addSeparator();
		actionRecordKeyRec = ms->addAction(CMSG(Record_), this, SLOT(recordReckeySlot()));
		actionRecordKeyRec->setCheckable(true);
		act = ms->addAction(CMSG(Stop_Recording), this, SLOT(stopRecordingReckeySlot()));

	mn->addSeparator();

	mn->addAction(CMSG(Load_State_), this, SLOT(loadStateSlot()));
	mn->addAction(CMSG(Save_State_), this, SLOT(saveStateSlot()));

	mn->addSeparator();

	mn->addAction(CMSG(Exit_), this, SLOT(exitSlot()));

	// tape menu
	mn = mb->addMenu(CMSG(Tape));
	connect(mn, SIGNAL(aboutToShow()), this, SLOT(updateMenuTapeSlot()));

	actionTapePlay = mn->addAction(CMSG(Play_), this, SLOT(playTapeSlot()));
	actionTapePlay->setCheckable(true);
	actionTapeRecord = mn->addAction(CMSG(Rec_), this, SLOT(recordTapeSlot()));
	actionTapeRecord->setCheckable(true);
	mn->addAction(CMSG(Eject), this, SLOT(ejectTapeSlot()));
	mn->addSeparator();
	mn->addAction(CMSG(Rewind), this, SLOT(rewindTapeSlot()));
	mn->addAction(CMSG(F_F_), this, SLOT(fastForwardTapeSlot()));
	mn->addAction(CMSG(Stop), this, SLOT(stopTapeSlot()));
	mn->addSeparator();
	actionTapeRealMode = mn->addAction(CMSG(Real_Mode), this, SLOT(tapeRealModeSlot()));
	actionTapeRealMode->setCheckable(true);
	mn->addSeparator();
	menuTapeRecentFiles = mn->addMenu(CMSG(Recent_Files));
	connect(menuTapeRecentFiles, SIGNAL(aboutToShow()), this, SLOT(updateRecentTapeSlot()));

	// fdd menu
	for(int drv=0; drv<MAX_DRIVE; drv++) {
		QString str(CMSG(FDD));
		mn = mb->addMenu(str + QString::asprintf("%d", drv));
		mn->setProperty("drv", QVariant::fromValue(drv));
		connect(mn, SIGNAL(aboutToShow()), this, SLOT(updateMenuFddSlot()));

		actionFddOpen[drv] = mn->addAction(CMSG(Insert_));
		actionFddOpen[drv]->setCheckable(true);
		connectWithNumber(actionFddOpen[drv], SIGNAL(triggered()), this, SLOT(openFddSlot()), drv);

		actionFddChangeSide[drv] = mn->addAction(CMSG(Change_Side_to_B));
		connectWithNumber(actionFddChangeSide[drv], SIGNAL(triggered()), this, SLOT(changeSideFddSlot()), drv);

		act = mn->addAction(CMSG(Eject));
		connectWithNumber(act, SIGNAL(triggered()), this, SLOT(closeFddSlot()), drv);

		ms = mn->addMenu(CMSG(New));
			act = ms->addAction(CMSG(Insert_Blank_2D_));
			connectWithNumber(act, SIGNAL(triggered()), this, SLOT(openBlankFddSlot()), drv | (0x00 << 8));
			act = ms->addAction(CMSG(Insert_Blank_2HD_));
			connectWithNumber(act, SIGNAL(triggered()), this, SLOT(openBlankFddSlot()), drv | (0x20 << 8));

		mn->addSeparator();

		actionFddWriteProtect[drv] = mn->addAction(CMSG(Write_Protect));
		actionFddWriteProtect[drv]->setCheckable(true);
		connectWithNumber(actionFddWriteProtect[drv], SIGNAL(triggered()), this, SLOT(writeProtectFddSlot()), drv);

		mn->addSeparator();

		menuFddMultiVolume[drv] = mn->addMenu(CMSG(Multi_Volume));
		menuFddMultiVolume[drv]->setProperty("drv", QVariant::fromValue(drv));
		connect(menuFddMultiVolume[drv], SIGNAL(aboutToShow()), this, SLOT(updateMultiVolumeFddSlot()));

		mn->addSeparator();

		menuFddRecentFiles[drv] = mn->addMenu(CMSG(Recent_Files));
		menuFddRecentFiles[drv]->setProperty("drv", QVariant::fromValue(drv));
		connect(menuFddRecentFiles[drv], SIGNAL(aboutToShow()), this, SLOT(updateRecentFddSlot()));
	}

	// screen menu
	mn = mb->addMenu(CMSG(Screen));
	connect(mn, SIGNAL(aboutToShow()), this, SLOT(updateMenuScreenSlot()));

	menuFrameRate = mn->addMenu(CMSG(Frame_Rate));
	connect(menuFrameRate, SIGNAL(aboutToShow()), this, SLOT(updateMenuScreenFrameRateSlot()));
	CMsg::Id frameRateList[] = {
		CMsg::Auto,
		CMsg::F60fps,
		CMsg::F30fps,
		CMsg::F20fps,
		CMsg::F15fps,
		CMsg::F12fps,
		CMsg::F10fps,
		CMsg::End
	};
	for(int i=0; frameRateList[i] != CMsg::End; i++) {
		act = menuFrameRate->addAction(CMSGV(frameRateList[i]));
		act->setCheckable(true);
		connectWithNumber(act, SIGNAL(triggered()), this, SLOT(selectFrameRateSlot()), i);
	}

	mn->addSeparator();

	menuRecordScreen = ms = mn->addMenu(CMSG(Record_Screen));
	connect(menuRecordScreen, SIGNAL(aboutToShow()), this, SLOT(updateMenuScreenRecordSlot()));
	CMsg::Id recordScreenList[] = {
		CMsg::Rec_60fps,
		CMsg::Rec_30fps,
		CMsg::Rec_20fps,
		CMsg::Rec_15fps,
		CMsg::Rec_12fps,
		CMsg::Rec_10fps,
		CMsg::End
	};
	actionRecordScreenSize[0] = ms->addAction("640x480");
	actionRecordScreenSize[0]->setCheckable(true);
	connectWithNumber(actionRecordScreenSize[0], SIGNAL(triggered()), this, SLOT(selectRecScreenSizeSlot()), 0);
	actionRecordScreenSize[1] = ms->addAction("768x512");
	actionRecordScreenSize[1]->setCheckable(true);
	connectWithNumber(actionRecordScreenSize[1], SIGNAL(triggered()), this, SLOT(selectRecScreenSizeSlot()), 1);
	ms->addSeparator();
	for(int i=0; recordScreenList[i] != CMsg::End; i++) {
		actionStartRecordScreen[i] = ms->addAction(CMSGV(recordScreenList[i]));
		actionStartRecordScreen[i]->setCheckable(true);
		connectWithNumber(actionStartRecordScreen[i], SIGNAL(triggered()), this, SLOT(selectStartRecScreenSlot()), i);
	}
	actionStopRecordScreen = ms->addAction(CMSG(Stop));
	connect(actionStopRecordScreen, SIGNAL(triggered()), this, SLOT(selectStopRecScreenSlot()));
	ms->addSeparator();
	actionCaptureScreen = ms->addAction(CMSG(Capture));
	connect(actionCaptureScreen, SIGNAL(triggered()), this, SLOT(selectCaptureScreenSlot()));

	mn->addSeparator();

	_TCHAR name[64];
	menuWindow = mn->addMenu(CMSG(Window));
	connect(menuWindow, SIGNAL(aboutToShow()), this, SLOT(updateMenuScreenWindowSlot()));

	// menuWindow->clear();
	for(int i = 0; i < gui->GetWindowModeCount(); i++) {
		gui->GetWindowModeStr(i, name);
		act = menuWindow->addAction(QTChar::fromTChar(name));
		act->setData(QVariant::fromValue(i));
		act->setCheckable(true);
		connect(act, SIGNAL(triggered()), this, SLOT(selectWindowModeSlot()));
	}

	menuFullscreen = mn->addMenu(CMSG(Fullscreen));
	connect(menuFullscreen, SIGNAL(aboutToShow()), this, SLOT(updateMenuScreenFullscreenSlot()));

	actionFullscreen[0] = menuFullscreen->addAction(CMSG(Stretch_Screen));
	actionFullscreen[0]->setData(QVariant::fromValue(99999));
	actionFullscreen[0]->setCheckable(true);
	connectWithNumber(actionFullscreen[0], SIGNAL(triggered()), this, SLOT(selectFullscreenStretchSlot()), 1);
	actionFullscreen[1] = menuFullscreen->addAction(CMSG(Cutout_Screen));
	actionFullscreen[1]->setData(QVariant::fromValue(99999));
	actionFullscreen[1]->setCheckable(true);
	connectWithNumber(actionFullscreen[1], SIGNAL(triggered()), this, SLOT(selectFullscreenStretchSlot()), 2);
	menuFullscreen->addSeparator();

	for(int disp_no = 0; disp_no < gui->GetDisplayDeviceCount(); disp_no++) {
		gui->GetDisplayDeviceStr(CMSG(Display), disp_no, name);
		QMenu *mss = menuFullscreen->addMenu(name);
		for(int i = 0; i < gui->GetFullScreenModeCount(disp_no); i++) {
			gui->GetFullScreenModeStr(disp_no, i, name);
			act = mss->addAction(QTChar::fromTChar(name));
			act->setData(QVariant::fromValue(disp_no * VIDEO_MODE_MAX + i));
			act->setCheckable(true);
			connect(act, SIGNAL(triggered()), this, SLOT(selectFullscreenModeSlot()));
		}
	}

	menuAspectRatio = mn->addMenu(CMSG(Aspect_Ratio));
	connect(menuAspectRatio, SIGNAL(aboutToShow()), this, SLOT(updateMenuScreenPixelAspectSlot()));
	// menuAspectRatio->clear();
	for(int i = 0; i < gui->GetPixelAspectModeCount(); i++) {
		gui->GetPixelAspectModeStr(i, name);
		act = menuAspectRatio->addAction(QTChar::fromTChar(name));
		act->setData(QVariant::fromValue(i));
		act->setCheckable(true);
		connect(act, SIGNAL(triggered()), this, SLOT(selectPixelAspectModeSlot()));
	}

	mn->addSeparator();
	menuDrawingMode = mn->addMenu(CMSG(Drawing_Mode));
	connect(menuDrawingMode, SIGNAL(aboutToShow()), this, SLOT(updateMenuScreenDrawModeSlot()));
	CMsg::Id drawingModeList[] = {
		CMsg::Full_Draw,
		CMsg::Scanline,
		CMsg::Stripe,
		CMsg::Checker,
		CMsg::End
	};
	for(int i = 0; drawingModeList[i] != CMsg::End; i++) {
		act = menuDrawingMode->addAction(CMSGV(drawingModeList[i]));
		act->setCheckable(true);
		connectWithNumber(act, SIGNAL(triggered()), this, SLOT(selectDrawModeSlot()), i);
	}

	mn->addSeparator();
	actionAfterimage1 = mn->addAction(CMSG(Afterimage1));
	actionAfterimage1->setCheckable(true);
	connectWithNumber(actionAfterimage1, SIGNAL(triggered()), this, SLOT(selectAfterImageSlot()), 1);
	actionAfterimage2 = mn->addAction(CMSG(Afterimage2));
	actionAfterimage2->setCheckable(true);
	connectWithNumber(actionAfterimage2, SIGNAL(triggered()), this, SLOT(selectAfterImageSlot()), 2);

#if defined(_MBS1)
	mn->addSeparator();

	actionDigitalRGB = mn->addAction(CMSG(Digital_RGB));
	actionDigitalRGB->setCheckable(true);
	connectWithNumber(actionDigitalRGB, SIGNAL(triggered()), this, SLOT(selectRGBTypeSlot()), 0);
	actionAnalogRGB = mn->addAction(CMSG(Analog_RGB));
	actionAnalogRGB->setCheckable(true);
	connectWithNumber(actionAnalogRGB, SIGNAL(triggered()), this, SLOT(selectRGBTypeSlot()), 1);
#endif

	mn->addSeparator();
	actionOpenGLSync = mn->addAction(CMSG(Use_OpenGL_Sync));
	actionOpenGLSync->setCheckable(true);
	connectWithNumber(actionOpenGLSync, SIGNAL(triggered()), this, SLOT(selectOpenGLSlot()), 1);
	actionOpenGLAsync = mn->addAction(CMSG(Use_OpenGL_Async));
	actionOpenGLAsync->setCheckable(true);
	connectWithNumber(actionOpenGLAsync, SIGNAL(triggered()), this, SLOT(selectOpenGLSlot()), 2);

	ms = mn->addMenu(CMSG(OpenGL_Filter));
	actionGLFNear = ms->addAction(CMSG(Nearest_Neighbour));
	actionGLFNear->setCheckable(true);
	connectWithNumber(actionGLFNear, SIGNAL(triggered()), this, SLOT(selectOpenGLFilterSlot()), 0);
	actionGLFLinear = ms->addAction(CMSG(Linear));
	actionGLFLinear->setCheckable(true);
	connectWithNumber(actionGLFLinear, SIGNAL(triggered()), this, SLOT(selectOpenGLFilterSlot()), 1);

	// sound menu
	mn = mb->addMenu(CMSG(Sound));
	connect(mn, SIGNAL(aboutToShow()), this, SLOT(updateMenuSoundSlot()));

	mn->addAction(CMSG(Volume),	this, SLOT(selectVolumeSlot()));

	mn->addSeparator();
	menuRecordSound = ms = mn->addMenu(CMSG(Record_Sound));
	connect(menuRecordSound, SIGNAL(aboutToShow()), this, SLOT(updateMenuSoundRecordSlot()));
	actionStartRecordSound = ms->addAction(CMSG(Start_));
	actionStartRecordSound->setCheckable(true);
	connect(actionStartRecordSound, SIGNAL(triggered()), this, SLOT(selectStartRecSndSlot()));
	actionStopRecordSound = ms->addAction(CMSG(Stop));
	connect(actionStopRecordSound, SIGNAL(triggered()), this, SLOT(selectStopRecSndSlot()));

	mn->addSeparator();
	CMsg::Id sampleRateList[] = {
		CMsg::F8000Hz,
		CMsg::F11025Hz,
		CMsg::F22050Hz,
		CMsg::F44100Hz,
		CMsg::F48000Hz,
		CMsg::F96000Hz,
		CMsg::End
	};
	for(int i = 0; i < 6; i++) {
		actionSampleRate[i] = mn->addAction(CMSGV(sampleRateList[i]));
		actionSampleRate[i]->setCheckable(true);
		connectWithNumber(actionSampleRate[i], SIGNAL(triggered()), this, SLOT(selectSoundFrequencySlot()), i + 2);
	}

	mn->addSeparator();
	CMsg::Id soundLateList[] = {
		CMsg::S50msec,
		CMsg::S75msec,
		CMsg::S100msec,
		CMsg::S200msec,
		CMsg::S300msec,
		CMsg::S400msec,
		CMsg::End
	};
	for(int i = 0; i < 6; i++) {
		actionSoundLate[i] = mn->addAction(CMSGV(soundLateList[i]));
		actionSoundLate[i]->setCheckable(true);
		connectWithNumber(actionSoundLate[i], SIGNAL(triggered()), this, SLOT(selectSoundLatencySlot()), i);
	}

	// devices menu
	mn = mb->addMenu(CMSG(Devices));
//	connect(mn, SIGNAL(aboutToShow()), this, SLOT(updateMenuDevicesSlot()));

	for(int i = 0; i < MAX_PRINTER; i++) {
		ms = mn->addMenu(tr("LPT") + QString::asprintf("%d", i));
		ms->setProperty("num", QVariant::fromValue(i));
		connect(ms, SIGNAL(aboutToShow()), this, SLOT(updateMenuDevicesLptSlot()));

		actionLptSave[i] = ms->addAction(CMSG(Save_));
		connectWithNumber(actionLptSave[i], SIGNAL(triggered()), this, SLOT(selectLptSaveSlot()), i);
		actionLptPrint[i] = ms->addAction(CMSG(Print_to_mpprinter));
		connectWithNumber(actionLptPrint[i], SIGNAL(triggered()), this, SLOT(selectLptPrintSlot()), i);
		act = ms->addAction(CMSG(Clear));
		connectWithNumber(act, SIGNAL(triggered()), this, SLOT(selectLptClearSlot()), i);
		ms->addSeparator();
		actionLptOnline[i] = ms->addAction(CMSG(Online));
		actionLptOnline[i]->setCheckable(true);
		connectWithNumber(actionLptOnline[i], SIGNAL(triggered()), this, SLOT(selectLptOnlineSlot()), i);
		ms->addSeparator();
		actionLptDirect[i] = ms->addAction(CMSG(Send_to_mpprinter_concurrently));
		actionLptDirect[i]->setCheckable(true);
		connectWithNumber(actionLptDirect[i], SIGNAL(triggered()), this, SLOT(selectLptDirectSlot()), i);
	}
	mn->addSeparator();
	for(int i = 0; i < MAX_COMM; i++) {
		ms = mn->addMenu(tr("COM") + QString::asprintf("%d", i));
		ms->setProperty("num", QVariant::fromValue(i));
		connect(ms, SIGNAL(aboutToShow()), this, SLOT(updateMenuDevicesCommSlot()));

		actionCommEnableServer[i] = ms->addAction(CMSG(Enable_Server));
		actionCommEnableServer[i]->setCheckable(true);
		connectWithNumber(actionCommEnableServer[i], SIGNAL(triggered()), this, SLOT(selectComEnableServerSlot()), i);
//		actionCommConnect[i] = ms->addAction(CMSG(Connect));
//		actionCommConnect[i]->setCheckable(true);
//		connectWithNumber(actionCommConnect[i], SIGNAL(triggered()), this, SLOT(selectComConnectSlot()), i);
		menuCommConnect[i] = ms->addMenu(CMSG(Connect));
		menuCommConnect[i]->setProperty("drv", QVariant::fromValue(i));
		connect(menuCommConnect[i], SIGNAL(aboutToShow()), this, SLOT(updateComConnectSlot()));
		ms->addSeparator();
		actionCommByteData[i] = ms->addAction(CMSG(Comm_With_Byte_Data));
		actionCommByteData[i]->setCheckable(true);
		connectWithNumber(actionCommByteData[i], SIGNAL(triggered()), this, SLOT(selectComByteDataSlot()), i);
		ms->addSeparator();
		QMenu *mss = ms->addMenu(CMSG(Options_For_Telnet));
		actionCommTelnetBinary[i] = mss->addAction(CMSG(Binary_Mode));
		actionCommTelnetBinary[i]->setCheckable(true);
		connectWithNumber(actionCommTelnetBinary[i], SIGNAL(triggered()), this, SLOT(selectComBinaryModeSlot()), i);
		actionCommTelnetEcho[i] = mss->addAction(CMSG(Send_WILL_ECHO));
		connectWithNumber(actionCommTelnetEcho[i], SIGNAL(triggered()), this, SLOT(selectComSendWillEchoSlot()), i);
	}

	// options menu
	mn = mb->addMenu(CMSG(Options));
	connect(mn, SIGNAL(aboutToShow()), this, SLOT(updateMenuOptionsSlot()));

	actionShowLED = mn->addAction(CMSG(Show_LED), this, SLOT(selectShowLEDSlot()));
	actionShowLED->setCheckable(true);
	actionInsideLED = mn->addAction(CMSG(Inside_LED), this, SLOT(selectInsideLEDSlot()));
	actionInsideLED->setCheckable(true);
	actionShowMessage = mn->addAction(CMSG(Show_Message), this, SLOT(selectShowMessageSlot()));
	actionShowMessage->setCheckable(true);
#ifdef USE_PERFORMANCE_METER
	actionShowPMeter = mn->addAction(CMSG(Show_Performance_Meter), this, SLOT(selectShowPMeterSlot()));
	actionShowPMeter->setCheckable(true);
#endif
	mn->addSeparator();
	actionUseJoypad[0] = mn->addAction(CMSG(Use_Joypad_Key_Assigned), this, SLOT(selectUseJoypadSlot()));
	actionUseJoypad[0]->setCheckable(true);
#ifdef USE_PIAJOYSTICK
	actionUseJoypad[1] = mn->addAction(CMSG(Use_Joypad_PIA_Type), this, SLOT(selectUseJoypadSlot()));
	actionUseJoypad[1]->setCheckable(true);
#else
	actionUseJoypad[1] = nullptr;
#endif
#ifdef USE_LIGHTPEN
	actionLightPen = mn->addAction(CMSG(Enable_Lightpen), this, SLOT(selectEnableLightpenSlot()));
	actionLightPen->setCheckable(true);
#endif
#ifdef USE_MOUSE
	actionMouse = mn->addAction(CMSG(Enable_Mouse), this, SLOT(selectEnableMouseSlot()));
	actionMouse->setCheckable(true);
#endif
	actionLoosenKeyStroke = mn->addAction(CMSG(Loosen_Key_Stroke_Game), this, SLOT(selectLoosenKeyStrokeSlot()));
	actionLoosenKeyStroke->setCheckable(true);
	mn->addSeparator();
	actionKeybind = mn->addAction(CMSG(Keybind_), this, SLOT(selectKeybindSlot()));
	actionVirtualKeyboard = mn->addAction(CMSG(Virtual_Keyboard), this, SLOT(selectVirtualKeyboardSlot()));

#ifdef USE_DEBUGGER
	mn->addSeparator();
	actionStartDebugger = mn->addAction(CMSG(Start_Debugger),this, SLOT(selectStartDebuggerSlot()));
	actionStopDebugger = mn->addAction(CMSG(Stop_Debugger), this, SLOT(selectStopDebuggerSlot()));
#else
	actionStartDebugger = nullptr;
	actionStopDebugger = nullptr;
#endif

	mn->addSeparator();
	actionConfigure = mn->addAction(CMSG(Configure_), this, SLOT(selectConfigureSlot()));

	// help menu
	mn = mb->addMenu(CMSG(Help));
	mn->addAction(CMSG(About_), this, SLOT(aboutSlot()));


#ifdef USE_SOCKET
	conn = new Connection();
#endif
#ifdef USE_UART
	coms = new CommPorts();
#endif

	initialize_ok = true;
}

MainWindow::~MainWindow()
{
#ifdef USE_SOCKET
	delete conn;
#endif
#ifdef USE_UART
	delete coms;
#endif
	delete ui;
}

bool MainWindow::isValid()
{
	return initialize_ok;
}

void MainWindow::resizeAll(int w, int h)
{
//	QWidget *c = centralWidget();
//	if (c) {
//		c->resize(x, y);
//	}
	QWidget *m = menuWidget();
	int mh = 0;
#if !defined(Q_OS_MAC)
	if (m) {
		mh = m->height();
	}
#endif
	setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
	setMinimumSize(0, 0);
    resize(w, h + mh);
#ifdef Q_OS_UNIX
	setMaximumSize(w, h + mh);
    setMinimumSize(w, h + mh);
#endif
}

void MainWindow::goFullScreen(int x, int y, int w, int h)
{
	QWidget *m = menuWidget();
	if (m) {
		m->hide();
	}
	setMaximumSize(w + 1, h + 1);
	setMinimumSize(w, h);
	move(x, y);
	showFullScreen();
}

void MainWindow::goNormalScreen(int w, int h)
{
    showNormal();

	QWidget *m = menuWidget();
	if (m) {
		m->show();
    }
    resizeAll(w, h);
}

bool MainWindow::event(QEvent *e)
{
	if (e->type() == QEvent::User) {
		// process command
		gui->ProcessEvent(dynamic_cast<MyUserEvent *>(e));
		return true;
	}
	return QMainWindow::event(e);
}


#if 0
void MainWindow::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);
	emu->update_screen(&painter);
	painter.end();
	gui->UpdateScreen();
}
#endif

void MainWindow::keyPressEvent(QKeyEvent *event)
{
	if (event->isAutoRepeat()) return;
    int code = event->key();
    uint32_t vk_key = event->nativeVirtualKey();
    uint32_t scan_code = event->nativeScanCode();
	uint32_t mod = static_cast<uint32_t>(event->modifiers());
	(dynamic_cast<EMU_OSD *>(emu))->key_down_up_(0, code, vk_key, scan_code, mod);
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
	if (event->isAutoRepeat()) return;
    int code = event->key();
    uint32_t vk_key = event->nativeVirtualKey();
    uint32_t scan_code = event->nativeScanCode();
	uint32_t mod = static_cast<uint32_t>(event->modifiers());
	(dynamic_cast<EMU_OSD *>(emu))->key_down_up_(1, code, vk_key, scan_code, mod);
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
//	Qt::MouseButtons btns = event->buttons();

	(dynamic_cast<EMU_OSD *>(emu))->mouse_down_up(0,
	 static_cast<int>(event->buttons()),
	 static_cast<int>(event->x()),
	 static_cast<int>(event->y())
	);
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
	Qt::MouseButtons btns = event->buttons();
	(dynamic_cast<EMU_OSD *>(emu))->mouse_down_up(1,
	 static_cast<int>(btns),
	 static_cast<int>(event->x()),
	 static_cast<int>(event->y())
	);
}

void MainWindow::mouseMoveEvent(QMouseEvent *UNUSED_PARAM(event))
{
//	emu->mouse_move();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	if (emu) emu->resume_window_placement();
	event->accept();
}

void MainWindow::updateScreenSlot()
{
	QWidget *c = centralWidget();
	if (c) {
		c->repaint();
	}
}

//
void MainWindow::updateTitleSlot()
{
	if (need_update_title) {
		need_update_title = false;
//		int ratio = 0;
//		if (total_frames) ratio = (int)(100.0 * (double)draw_frames / (double)total_frames + 0.5);
		char buf[128];
		UTILITY::sprintf(buf, 128, "%s - %d/%dfps", DEVICE_NAME, frames_result.draw, frames_result.total);

		setWindowTitle(QString::fromUtf8(buf));
	}
}

void MainWindow::connectWithNumber(QAction *sender, const char *signal, MainWindow *recv, const char *slot, int num)
{
	sender->setData(QVariant::fromValue(num));
	QObject::connect(sender, signal, recv, slot);
}

#define TRIM_STRING_SIZE 64

//
void MainWindow::updateRecentMenu(QMenu *menu, CRecentPathList &list, const char *slot, int drv)
{
	_TCHAR str[_MAX_PATH];
	bool flag = false;

	if (list.updated) {
		menu->clear();
		for(int i = 0; i < MAX_HISTORY && i < list.Count(); i++) {
			if (!gui->GetRecentFileStr(list[i]->path, list[i]->num, str, TRIM_STRING_SIZE)) break;
			QAction *act = menu->addAction(QTChar::fromTChar(str));
			act->setData(QVariant::fromValue(drv << 16 | i));
			connect(act, SIGNAL(triggered()), this, slot);

			flag = true;
		}
		if(!flag) {
			menu->addAction("None");
		}
	}
	list.updated = false;
}

//
void MainWindow::updateMultiVolumeMenu(QMenu *menu, int drv, const char *slot)
{
	_TCHAR str[_MAX_PATH];
	bool flag = false;

	menu->clear();
	if (gui->InsertedFloppyDisk(drv)) {
		D88File *d88_file = gui->GetD88File(drv);
		int bank_nums = d88_file->GetBanks().Count();
		if(bank_nums >= 1) {
			for(int i = 0; i < bank_nums; i++) {
				const D88Bank *d88_bank = d88_file->GetBank(i);
				gui->GetMultiVolumeStr(i, d88_bank->GetName(), str, _MAX_PATH);
				QAction *act = menu->addAction(QTChar::fromTChar(str));
				act->setData(QVariant::fromValue(drv << 16 | i));
				connect(act, SIGNAL(triggered()), this, slot);
				act->setDisabled(bank_nums <= 1);
				act->setCheckable(true);
				act->setChecked(i == d88_file->GetCurrentBank());
			}
			flag = true;
		}
	}
	if(!flag) {
		menu->addAction("None");
	}
}

//
void MainWindow::updateMenuControlSlot()
{
	actionPowerOn->setChecked(!gui->NowPowerOff());
#if defined(_MBS1)
	for(int i=0; i<2; i++) {
		actionSystemMode[i]->setChecked(gui->GetSystemMode() == (1 - i));
	}
#else
#ifdef USE_DIPSWITCH
	actionModeSwitch->setChecked((gui->GetDipswitch() & (1 << 2)) != 0);
#endif
#endif
	for(int i=0; i<4; i++) {
		actionFddType[i]->setChecked(gui->NextFddType() == i);
	}
	actionPause->setChecked(gui->NowPause());

	for(int i=0; i<6; i++) {
		actionCPUSpeed[i]->setChecked(gui->GetCPUPower() == i);
	}
	actionSyncIRQ->setChecked(gui->NowSyncIRQ());
}

//
void MainWindow::resetSlot()
{
	gui->PostEtReset();
}
#ifdef USE_SPECIAL_RESET
void MainWindow::specialResetSlot()
{
	gui->PostEtSpecialReset();
}
void MainWindow::warmResetSlot()
{
	gui->PostEtWarmReset(-1);
}
#endif
#ifdef USE_DIPSWITCH
void MainWindow::dipSwitch2Slot()
{
	gui->Dipswitch(2);
}
#endif
#if defined(_MBS1)
void MainWindow::systemModeSlot()
{
	QAction *act = dynamic_cast<QAction *>(sender());
	int num = act->data().toInt();
	gui->ChangeSystemMode(1 - num);
}
#endif
void MainWindow::fddTypeSlot()
{
	QAction *act = dynamic_cast<QAction *>(sender());
	int num = act->data().toInt();
	gui->ChangeFddType(num);
}
void MainWindow::pauseSlot()
{
	gui->PostEtTogglePause();
}
void MainWindow::cpuPowerSlot()
{
	QAction *act = dynamic_cast<QAction *>(sender());
	int num = act->data().toInt();
	gui->PostEtCPUPower(num);
}
void MainWindow::syncIrqSlot()
{
	gui->PostEtToggleSyncIRQ();
}
void MainWindow::playAutokeySlot()
{
	gui->ShowOpenAutoKeyDialog();
}
void MainWindow::pasteAutokeySlot()
{
	gui->PostEtStartAutoKeyMessage();
}
void MainWindow::stopAutokeySlot()
{
	gui->PostEtStopAutoKeyMessage();
}
void MainWindow::playReckeySlot()
{
	gui->ShowPlayRecKeyDialog();
}
void MainWindow::stopPlayingReckeySlot()
{
	gui->StopPlayRecKey();
}
void MainWindow::recordReckeySlot()
{
	gui->ShowRecordStateAndRecKeyDialog();
}
void MainWindow::stopRecordingReckeySlot()
{
	gui->StopRecordRecKey();
}
void MainWindow::loadStateSlot()
{
	gui->ShowLoadStateDialog();
}
void MainWindow::saveStateSlot()
{
	gui->ShowSaveStateDialog(false);
}
void MainWindow::exitSlot()
{
	gui->Exit();
	close();
}

//
void MainWindow::updateMenuTapeSlot()
{
	actionTapePlay->setChecked(gui->IsOpenedLoadDataRecFile());
	actionTapeRecord->setChecked(gui->IsOpenedSaveDataRecFile());
	actionTapeRealMode->setChecked(gui->NowRealModeDataRec());
}
void MainWindow::playTapeSlot()
{
	gui->ShowLoadDataRecDialog();
}
void MainWindow::recordTapeSlot()
{
	gui->ShowSaveDataRecDialog();
}
void MainWindow::ejectTapeSlot()
{
	gui->PostEtCloseDataRecMessage();
}
void MainWindow::stopTapeSlot()
{
	gui->PostEtStopDataRecMessage();
}
void MainWindow::rewindTapeSlot()
{
	gui->PostEtRewindDataRecMessage();
}
void MainWindow::fastForwardTapeSlot()
{
	gui->PostEtFastForwardDataRecMessage();
}
void MainWindow::tapeRealModeSlot()
{
	gui->PostEtToggleRealModeDataRecMessage();
}
void MainWindow::updateRecentTapeSlot()
{
	updateRecentMenu(menuTapeRecentFiles, config.recent_datarec_path, SLOT(recentTapeSlot()));
}
void MainWindow::recentTapeSlot()
{
	QAction *act = dynamic_cast<QAction *>(sender());
	int num = act->data().toInt();

	gui->PostEtLoadRecentDataRecMessage(num);
}

//
#ifdef USE_FD1
void MainWindow::updateMenuFddSlot()
{
	int drv = sender()->property("drv").toInt();
	updateMenuFddSlot(drv, actionFddOpen[drv], actionFddWriteProtect[drv], actionFddChangeSide[drv]);
}
void MainWindow::updateMenuFddSlot(int drv, QAction *actOpen, QAction *actProtect, QAction *actSide)
{
	actOpen->setChecked(gui->InsertedFloppyDisk(drv));
	actProtect->setChecked(gui->WriteProtectedFloppyDisk(drv));

	int side = gui->GetSideFloppyDisk(drv);
	CMsg::Id msg_id = CMsg::Change_Side_to_B;
	if (side == 1 && config.fdd_type == FDD_TYPE_3FDD) {
		msg_id = CMsg::Change_Side_to_A;
	}
	actSide->setText(CMSGV(msg_id));
	actSide->setEnabled(config.fdd_type == FDD_TYPE_3FDD);
}
void MainWindow::updateMultiVolumeFddSlot()
{
	int drv = sender()->property("drv").toInt();
	updateMultiVolumeFddSlot(drv, menuFddMultiVolume[drv]);
}
void MainWindow::updateMultiVolumeFddSlot(int drv, QMenu *multi)
{
	updateMultiVolumeMenu(multi, drv, SLOT(openVolumeFddSlot()));
}
void MainWindow::updateRecentFddSlot()
{
	int drv = sender()->property("drv").toInt();
	updateRecentFddSlot(drv, menuFddRecentFiles[drv]);
}
void MainWindow::updateRecentFddSlot(int drv, QMenu *recent)
{
	updateRecentMenu(recent, config.recent_disk_path[drv], SLOT(recentFddSlot()), drv);
}
#endif

#ifdef USE_FD1
void MainWindow::openFddSlot()
{
	QAction *act = dynamic_cast<QAction *>(sender());
	int drv = act->data().toInt();
	gui->ShowOpenFloppyDiskDialog(drv);
}
void MainWindow::changeSideFddSlot()
{
	QAction *act = dynamic_cast<QAction *>(sender());
	int drv = act->data().toInt();
	gui->PostEtChangeSideFloppyDisk(drv);
}
void MainWindow::closeFddSlot()
{
	QAction *act = dynamic_cast<QAction *>(sender());
	int drv = act->data().toInt();
	gui->PostEtCloseFloppyMessage(drv);
}
void MainWindow::openBlankFddSlot()
{
	QAction *act = dynamic_cast<QAction *>(sender());
	int drv = act->data().toInt();
	int type = (drv >> 8);
	drv &= 0xff;
	gui->ShowOpenBlankFloppyDiskDialog(drv, type & 0xff);
}
void MainWindow::writeProtectFddSlot()
{
	QAction *act = dynamic_cast<QAction *>(sender());
	int drv = act->data().toInt();
	gui->PostEtToggleWriteProtectFloppyDisk(drv);
}
void MainWindow::openVolumeFddSlot()
{
	QAction *act = dynamic_cast<QAction *>(sender());
	int num = act->data().toInt();
	int drv = (num >> 16);
	num = (num & 0xffff);
	gui->PostEtOpenFloppySelectedVolume(drv, num);
}
void MainWindow::recentFddSlot()
{
	QAction *act = dynamic_cast<QAction *>(sender());
	int num = act->data().toInt();
	int drv = (num >> 16);
	num = (num & 0xffff);
	gui->PostEtOpenRecentFloppyMessage(drv, num);
}
#endif

//
void MainWindow::updateMenuScreenSlot()
{
	int num;

	num = gui->GetAfterImageMode();
	actionAfterimage1->setChecked(num == 1);
	actionAfterimage2->setChecked(num == 2);

#if defined(_MBS1)
	num = gui->GetRGBTypeMode();
	actionDigitalRGB->setChecked(num == 0);
	actionAnalogRGB->setChecked(num == 1);
#endif

	num = gui->GetOpenGLMode();
	actionOpenGLSync->setChecked(num == 1);
	actionOpenGLAsync->setChecked(num == 2);

	num = gui->GetOpenGLFilter();
	actionGLFNear->setChecked(num == 0);
	actionGLFLinear->setChecked(num == 1);
}
void MainWindow::updateMenuScreenRecordSlot()
{
	bool now_rec = gui->NowRecordingVideo() | gui->NowRecordingSound();
	int siz_num = gui->GetRecordVideoSurfaceNum();
	int fre_num = gui->GetRecordVideoFrameNum();
	for(int i=0; i<2; i++) {
		actionRecordScreenSize[i]->setChecked(i == siz_num);
		actionRecordScreenSize[i]->setEnabled(!now_rec);
	}
	for(int i=0; i<6; i++) {
		actionStartRecordScreen[i]->setChecked(i == fre_num);
		actionStartRecordScreen[i]->setEnabled(!now_rec);
	}
}
void MainWindow::updateMenuScreenFrameRateSlot()
{
	int num;
	num = gui->GetFrameRateNum();
	foreach (QAction *act, menuFrameRate->actions()) {
		int i = act->data().toInt();
		act->setChecked(i == 0 ? num < 0 : num == (i-1));
	}
}
void MainWindow::updateMenuScreenWindowSlot()
{
	int num = gui->GetWindowMode();
	foreach (QAction *act, menuWindow->actions()) {
		int i = act->data().toInt();
		act->setChecked(num == i);
	}
}
void MainWindow::updateMenuScreenFullscreenSlot()
{
	int num = gui->GetFullScreenMode();
	foreach (QAction *act, menuFullscreen->actions()) {
		int i = act->data().toInt();
		act->setChecked(num == i);
	}
	num = gui->GetStretchScreen();
	actionFullscreen[0]->setChecked(num == 1);
	actionFullscreen[1]->setChecked(num == 2);
}
void MainWindow::updateMenuScreenPixelAspectSlot()
{
	int num = gui->GetPixelAspectMode();
	foreach (QAction *act, menuAspectRatio->actions()) {
		int i = act->data().toInt();
		act->setChecked(num == i);
	}
}
void MainWindow::updateMenuScreenDrawModeSlot()
{
	int num = gui->GetDrawMode();
	foreach (QAction *act, menuDrawingMode->actions()) {
		int i = act->data().toInt();
		act->setChecked(num == i);
	}
}

void MainWindow::selectFrameRateSlot()
{
	QAction *act = dynamic_cast<QAction *>(sender());
	int num = act->data().toInt();
	gui->ChangeFrameRate(num - 1);
}
void MainWindow::selectRecScreenSizeSlot()
{
	QAction *act = dynamic_cast<QAction *>(sender());
	int num = act->data().toInt();
	gui->PostEtResizeRecordVideoSurface(num);
}
void MainWindow::selectStartRecScreenSlot()
{
	QAction *act = dynamic_cast<QAction *>(sender());
	int num = act->data().toInt();
	gui->ShowRecordVideoAndAudioDialog(num);
}
void MainWindow::selectStopRecScreenSlot()
{
	gui->PostEtStopRecordVideo();
}
void MainWindow::selectCaptureScreenSlot()
{
	gui->PostEtCaptureScreen();
}
void MainWindow::selectWindowModeSlot()
{
	QAction *act = dynamic_cast<QAction *>(sender());
	int num = act->data().toInt();
	gui->ChangeWindowMode(num);
}
void MainWindow::selectFullscreenModeSlot()
{
	QAction *act = dynamic_cast<QAction *>(sender());
	int num = act->data().toInt();
	gui->ChangeFullScreenMode(num);
}
void MainWindow::selectFullscreenStretchSlot()
{
	QAction *act = dynamic_cast<QAction *>(sender());
	int num = act->data().toInt();
	gui->ChangeStretchScreen(num);
}
void MainWindow::selectPixelAspectModeSlot()
{
	QAction *act = dynamic_cast<QAction *>(sender());
	int num = act->data().toInt();
	gui->ChangePixelAspect(num);
}
void MainWindow::selectDrawModeSlot()
{
	QAction *act = dynamic_cast<QAction *>(sender());
	int num = act->data().toInt();
	gui->PostEtChangeDrawMode(num);
}
void MainWindow::selectAfterImageSlot()
{
	QAction *act = dynamic_cast<QAction *>(sender());
	int num = act->data().toInt();
	gui->PostEtChangeAfterImage(num);
}
#if defined(_MBS1)
void MainWindow::selectRGBTypeSlot()
{
	QAction *act = dynamic_cast<QAction *>(sender());
	int num = act->data().toInt();
	gui->ChangeRGBType(num);
}
#endif
void MainWindow::selectOpenGLSlot()
{
	QAction *act = dynamic_cast<QAction *>(sender());
	int num = act->data().toInt();
	gui->ChangeUseOpenGL(num);
}
void MainWindow::selectOpenGLFilterSlot()
{
	QAction *act = dynamic_cast<QAction *>(sender());
	int num = act->data().toInt();
	gui->ChangeOpenGLFilter(num);
}

void MainWindow::updateMenuSoundSlot()
{
	int num = gui->GetSoundFrequencyNum();
	for(int i = 0; i < 6; i++) {
		actionSampleRate[i]->setChecked(num == i + 2);
	}
	num = gui->GetSoundLatencyNum();
	for(int i = 0; i < 6; i++) {
		actionSoundLate[i]->setChecked(num == i);
	}
}
void MainWindow::updateMenuSoundRecordSlot()
{
	bool now_rec = gui->NowRecordingVideo() | gui->NowRecordingSound();
	actionStartRecordSound->setChecked(now_rec);
	actionStartRecordSound->setEnabled(!now_rec);
}
void MainWindow::selectVolumeSlot()
{
	gui->ShowVolumeDialog();
}

void MainWindow::selectStartRecSndSlot()
{
	gui->ShowRecordAudioDialog();
}

void MainWindow::selectStopRecSndSlot()
{
	gui->PostEtStopRecordSound();
}

void MainWindow::selectSoundFrequencySlot()
{
	QAction *act = dynamic_cast<QAction *>(sender());
	int num = act->data().toInt();

	gui->ChangeSoundFrequency(num);
}

void MainWindow::selectSoundLatencySlot()
{
	QAction *act = dynamic_cast<QAction *>(sender());
	int num = act->data().toInt();

	gui->ChangeSoundLatency(num);
}

void MainWindow::updateMenuDevicesSlot()
{
}

void MainWindow::updateMenuDevicesLptSlot()
{
	QMenu *mn = dynamic_cast<QMenu *>(sender());
	int num = mn->property("num").toInt();

	actionLptSave[num]->setEnabled(gui->GetPrinterBufferSize(num) > 0);
	actionLptPrint[num]->setEnabled(gui->GetPrinterBufferSize(num) > 0);
	actionLptDirect[num]->setChecked(gui->IsEnablePrinterDirect(num));
	actionLptOnline[num]->setChecked(gui->IsOnlinePrinter(num));
}

void MainWindow::selectLptSaveSlot()
{
	QAction *act = dynamic_cast<QAction *>(sender());
	int drv = act->data().toInt();
	gui->ShowSavePrinterDialog(drv);
}

void MainWindow::selectLptPrintSlot()
{
	QAction *act = dynamic_cast<QAction *>(sender());
	int drv = act->data().toInt();
//	gui->PostEtPrintPrinterMessage(drv);
	gui->PrintPrinter(drv);
}

void MainWindow::selectLptClearSlot()
{
	QAction *act = dynamic_cast<QAction *>(sender());
	int drv = act->data().toInt();
	gui->PostEtClearPrinterBufferMessage(drv);
}

void MainWindow::selectLptDirectSlot()
{
	QAction *act = dynamic_cast<QAction *>(sender());
	int drv = act->data().toInt();
//	gui->PostEtEnablePrinterDirectMessage(drv);
	gui->EnablePrinterDirect(drv);
}

void MainWindow::selectLptOnlineSlot()
{
	QAction *act = dynamic_cast<QAction *>(sender());
	int drv = act->data().toInt();
	gui->PostEtTogglePrinterOnlineMessage(drv);
}

void MainWindow::updateMenuDevicesCommSlot()
{
	QMenu *mn = dynamic_cast<QMenu *>(sender());
	int num = mn->property("num").toInt();

	actionCommEnableServer[num]->setChecked(gui->IsEnableCommServer(num));
//	actionCommConnect[num]->setChecked(gui->NowConnectingComm(num, 0));
	actionCommByteData[num]->setChecked(gui->NowCommThroughMode(num));
	actionCommTelnetBinary[num]->setChecked(gui->NowCommBinaryMode(num));
}

void MainWindow::selectComEnableServerSlot()
{
	QAction *act = dynamic_cast<QAction *>(sender());
	int drv = act->data().toInt();
	gui->ToggleEnableCommServer(drv);
}

void MainWindow::updateComConnectSlot()
{
	int drv = sender()->property("drv").toInt();
	menuCommConnect[drv]->clear();

	QAction *a = menuCommConnect[drv]->addAction(CMSG(Ethernet));
	a->setCheckable(true);
	a->setChecked(gui->NowConnectingComm(drv, 0));
	connectWithNumber(a, SIGNAL(triggered()), this, SLOT(selectComConnectSlot()), drv << 16);

	int uarts = gui->EnumUarts();

	if (uarts > 0) {
		menuCommConnect[drv]->addSeparator();
	}
	for(int i=0; i<uarts; i++) {
		char str[128];
		gui->GetUartDescription(i, str, sizeof(str));
		a = menuCommConnect[drv]->addAction(str);
		a->setCheckable(true);
		a->setChecked(gui->NowConnectingComm(drv, i + 1));
		connectWithNumber(a, SIGNAL(triggered()), this, SLOT(selectComConnectSlot()), (drv << 16) | (i + 1));
	}
}

void MainWindow::selectComConnectSlot()
{
	QAction *act = dynamic_cast<QAction *>(sender());
	int drv = act->data().toInt();
	int num = (drv & 0xffff);
	drv >>= 16;
	gui->ToggleConnectComm(drv, num);
//	gui->PostEtToggleConnectCommMessage(drv, num);
}

void MainWindow::selectComByteDataSlot()
{
	QAction *act = dynamic_cast<QAction *>(sender());
	int drv = act->data().toInt();
	gui->ToggleCommThroughMode(drv);
}

void MainWindow::selectComBinaryModeSlot()
{
	QAction *act = dynamic_cast<QAction *>(sender());
	int drv = act->data().toInt();
	gui->ToggleCommBinaryMode(drv);
}

void MainWindow::selectComSendWillEchoSlot()
{
	QAction *act = dynamic_cast<QAction *>(sender());
	int drv = act->data().toInt();
	gui->SendCommTelnetCommand(drv, 1);
}

void MainWindow::updateMenuOptionsSlot()
{
	bool show = gui->IsShownLedBox();
	actionShowLED->setChecked(show);
	show = gui->IsInsidedLedBox();
	actionInsideLED->setChecked(show);
	show = gui->IsShownMessageBoard();
	actionShowMessage->setChecked(show);
#ifdef USE_PERFORMANCE_METER
	show = gui->IsShownPMeter();
	actionShowPMeter->setChecked(show);
#endif
	show = gui->IsEnableJoypad(1);
	actionUseJoypad[0]->setChecked(show);
#ifdef USE_LIGHTPEN
	show = gui->IsEnableLightpen();
	actionLightPen->setChecked(show);
#endif
#ifdef USE_MOUSE
	show = gui->IsEnableMouse();
	actionMouse->setChecked(show);
#endif
	show = gui->IsLoosenKeyStroke();
	actionLoosenKeyStroke->setChecked(show);
	show = gui->IsShownVirtualKeyboard();
	actionVirtualKeyboard->setChecked(show);
#ifdef USE_DEBUGGER
	show = gui->IsDebuggerOpened();
	actionStartDebugger->setEnabled(!show);
//	actionStopDebugger->setEnabled(show);
#endif
}
void MainWindow::selectShowLEDSlot()
{
	gui->ToggleShowLedBox();
}
void MainWindow::selectInsideLEDSlot()
{
	gui->ToggleInsideLedBox();
}
void MainWindow::selectShowMessageSlot()
{
	gui->ToggleMessageBoard();
}
#ifdef USE_PERFORMANCE_METER
void MainWindow::selectShowPMeterSlot()
{
	gui->TogglePMeter();
}
#endif
void MainWindow::selectUseJoypadSlot()
{
	gui->ChangeUseJoypad(1);
}
void MainWindow::selectUsePIAJoypadSlot()
{
	gui->ChangeUseJoypad(2);
}
#ifdef USE_LIGHTPEN
void MainWindow::selectEnableLightpenSlot()
{
	gui->ToggleEnableLightpen();
}
#endif
#ifdef USE_MOUSE
void MainWindow::selectEnableMouseSlot()
{
	gui->ToggleUseMouse();
}
#endif
void MainWindow::selectLoosenKeyStrokeSlot()
{
	gui->ToggleLoosenKeyStroke();
}
void MainWindow::selectKeybindSlot()
{
	gui->ShowKeybindDialog();
}
void MainWindow::selectVirtualKeyboardSlot()
{
	gui->ShowVirtualKeyboard();
}
void MainWindow::selectConfigureSlot()
{
	gui->ShowConfigureDialog();
}
#ifdef USE_DEBUGGER
void MainWindow::selectStartDebuggerSlot()
{
	gui->OpenDebugger();
}
void MainWindow::selectStopDebuggerSlot()
{
	gui->CloseDebugger();
}
#endif

//
void MainWindow::aboutSlot()
{
	gui->ShowAboutDialog();
}

#ifdef USE_UART
void MainWindow::writeSerialDataSlot(int ch)
{

}
#endif
