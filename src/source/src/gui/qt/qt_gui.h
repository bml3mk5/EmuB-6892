/** @file qt_gui.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2016.11.18

	@brief [ qt_gui ]
*/

#ifndef QT_GUI_H
#define QT_GUI_H

#include "../gui_base.h"

#ifdef Status
#undef Status
#endif

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class EMU;
class EmuThread;
class CRecentPathList;
#ifdef USE_SOCKET
class Connection;
#endif
#ifdef USE_UART
class CommPorts;
#endif
class MyLoggingBox;

/**
	@brief GUI class
*/
class GUI : public GUI_BASE
{
private:
	QString replaceSemicolon(const QString &str);
public:
	GUI(int argc, char **argv, EMU *new_emu);
	virtual ~GUI();

	bool NeedUpdateScreen();
	void UpdatedScreen();
	void ScreenModeChanged(bool fullscreen);
	void PreProcessEvent();
	void PostCommandMessage(int id, void *data1 = nullptr, void *data2 = nullptr);

	bool ShowLoadStateDialog(void);
	bool ShowSaveStateDialog(bool cont);
	bool ShowOpenAutoKeyDialog(void);
	bool ShowPlayRecKeyDialog(void);
	bool ShowRecordRecKeyDialog(void);
	bool ShowRecordStateAndRecKeyDialog(void);
	bool ShowLoadDataRecDialog(void);
	bool ShowSaveDataRecDialog(void);
	bool ShowOpenFloppyDiskDialog(int drv);
	bool ShowOpenBlankFloppyDiskDialog(int drv, uint8_t type);
	bool ShowSavePrinterDialog(int drv);
	bool ShowRecordVideoDialog(int fps_num);
	bool ShowRecordAudioDialog(void);
	bool ShowRecordVideoAndAudioDialog(int fps_num);
	bool ShowVolumeDialog(void);
	bool ShowJoySettingDialog(void);
	bool ShowKeybindDialog(void);
	bool ShowVirtualKeyboard(void);
	bool ShowConfigureDialog(void);
	bool ShowAboutDialog(void);
	bool ShowLoggingDialog(void);
	bool IsShownLoggingDialog(void);

	void CreateLedBoxSub();

	bool GetRecentFileStr(const _TCHAR *file, int num, _TCHAR *str, int trimlen);

	bool StartAutoKey(void);
};

/**
	@brief Main Window
*/
class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = nullptr);
	~MainWindow();

	// inherit
	bool event(QEvent *e);

	// original
	bool isValid();
    void resizeAll(int w, int h);
	void goFullScreen(int x, int y, int w, int h);
	void goNormalScreen(int w, int h);

	bool showLoggingDialog(void);
	bool isShownLoggingDialog(void);

#ifdef USE_SOCKET
	Connection *conn;
#endif
#ifdef USE_UART
	CommPorts *coms;
#endif

private slots:
	void updateScreenSlot();
	void updateTitleSlot();

	// for menu
	void updateMenuControlSlot();

	void resetSlot();
#ifdef USE_SPECIAL_RESET
	void specialResetSlot();
	void warmResetSlot();
#endif
#ifdef USE_DIPSWITCH
	void dipSwitch2Slot();
#endif
#if defined(_MBS1)
	void systemModeSlot();
#endif
	void fddTypeSlot();

	void pauseSlot();

	void cpuPowerSlot();

	void syncIrqSlot();

	void playAutokeySlot();
	void pasteAutokeySlot();
	void stopAutokeySlot();

	void playReckeySlot();
	void stopPlayingReckeySlot();
	void recordReckeySlot();
	void stopRecordingReckeySlot();

	void loadStateSlot();
	void saveStateSlot();

	void exitSlot();

	void updateMenuTapeSlot();

	void playTapeSlot();
	void recordTapeSlot();
	void rewindTapeSlot();
	void fastForwardTapeSlot();
	void ejectTapeSlot();
	void stopTapeSlot();
	void tapeRealModeSlot();
	void updateRecentTapeSlot();
	void recentTapeSlot();

#ifdef USE_FD1
	void updateMenuFddSlot();
	void updateMenuFddSlot(int drv, QAction *actOpen, QAction *actProtect, QAction *actSide);
	void updateMultiVolumeFddSlot();
	void updateMultiVolumeFddSlot(int drv, QMenu *multi);
	void updateRecentFddSlot();
	void updateRecentFddSlot(int drv, QMenu *recent);
#endif

#ifdef USE_FD1
	void openFddSlot();
	void changeSideFddSlot();
	void closeFddSlot();
	void openBlankFddSlot();
	void writeProtectFddSlot();
	void openVolumeFddSlot();
	void recentFddSlot();
#endif

	void updateMenuScreenSlot();
	void updateMenuScreenRecordSlot();
	void updateMenuScreenFrameRateSlot();
	void updateMenuScreenWindowSlot();
	void updateMenuScreenFullscreenSlot();
	void updateMenuScreenPixelAspectSlot();
	void updateMenuScreenDrawModeSlot();

	void selectFrameRateSlot();
	void selectRecScreenSizeSlot();
	void selectStartRecScreenSlot();
	void selectStopRecScreenSlot();
	void selectCaptureScreenSlot();

	void selectWindowModeSlot();
	void selectFullscreenModeSlot();
	void selectFullscreenStretchSlot();
	void selectPixelAspectModeSlot();
	void selectDrawModeSlot();

	void selectAfterImageSlot();
#if defined(_MBS1)
	void selectRGBTypeSlot();
#endif

	void selectOpenGLSlot();
	void selectOpenGLFilterSlot();

	void updateMenuSoundSlot();
	void updateMenuSoundRecordSlot();
	void selectVolumeSlot();

	void selectStartRecSndSlot();
	void selectStopRecSndSlot();

	void selectSoundFrequencySlot();
	void selectSoundLatencySlot();

	void updateMenuDevicesSlot();
	void updateMenuDevicesLptSlot();
	void updateMenuDevicesCommSlot();

	void selectLptSaveSlot();
	void selectLptPrintSlot();
	void selectLptClearSlot();
	void selectLptDirectSlot();
	void selectLptOnlineSlot();

	void selectComEnableServerSlot();
	void updateComConnectSlot();
	void selectComConnectSlot();
	void selectComByteDataSlot();
	void selectComBinaryModeSlot();
	void selectComSendWillEchoSlot();

	void updateMenuOptionsSlot();
	void selectShowLEDSlot();
	void selectInsideLEDSlot();
	void selectShowMessageSlot();
	void selectShowLoggingSlot();
#ifdef USE_PERFORMANCE_METER
	void selectShowPMeterSlot();
#endif
	void selectUseJoypadSlot();
	void selectUsePIAJoypadSlot();
#ifdef USE_KEY2JOYSTICK
	void selectKey2JoypadSlot();
#endif
	void selectJoySettingSlot();
#ifdef USE_LIGHTPEN
	void selectEnableLightpenSlot();
#endif
#ifdef USE_MOUSE
	void selectEnableMouseSlot();
#endif
	void selectLoosenKeyStrokeSlot();
	void selectKeybindSlot();
	void selectVirtualKeyboardSlot();
	void selectConfigureSlot();

#ifdef USE_DEBUGGER
	void selectStartDebuggerSlot();
	void selectStopDebuggerSlot();
#endif

	void aboutSlot();

#ifdef USE_UART
	void writeSerialDataSlot(int ch);
#endif

protected:
//	void paintEvent(QPaintEvent * event);
	void keyPressEvent(QKeyEvent * event);
	void keyReleaseEvent(QKeyEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void closeEvent(QCloseEvent *event);

	void updateRecentMenu(QMenu *menu, CRecentPathList &list, const char *slot, int drv = 0);
	void updateMultiVolumeMenu(QMenu *menu, int drv, const char *slot);

private:
	Ui::MainWindow *ui;
	bool initialize_ok;

	MyLoggingBox *loggingbox;

	QAction *actionPowerOn;
#if defined(_MBS1)
	QAction *actionSystemMode[2];
#else
	QAction *actionModeSwitch;
#endif
	QAction *actionPause;
	QAction *actionFddType[4];
	QAction *actionCPUSpeed[6];
	QAction *actionSyncIRQ;
	QAction *actionRecordKeyPlay;
	QAction *actionRecordKeyRec;
	QAction *actionTapePlay;
	QAction *actionTapeRecord;
	QAction *actionTapeRealMode;
	QMenu *menuTapeRecentFiles;
	QAction *actionFddOpen[USE_FLOPPY_DISKS];
	QAction *actionFddChangeSide[USE_FLOPPY_DISKS];
	QAction *actionFddWriteProtect[USE_FLOPPY_DISKS];
	QMenu *menuFddMultiVolume[USE_FLOPPY_DISKS];
	QMenu *menuFddRecentFiles[USE_FLOPPY_DISKS];
	QMenu *menuFrameRate;
	QMenu *menuRecordScreen;
	QAction *actionRecordScreenSize[2];
	QAction *actionStartRecordScreen[6];
	QAction *actionStopRecordScreen;
	QAction *actionCaptureScreen;
	QAction *actionFullscreen[2];
	QMenu *menuWindow;
	QMenu *menuFullscreen;
	QMenu *menuAspectRatio;
	QMenu *menuDrawingMode;
	QAction *actionAfterimage1;
	QAction *actionAfterimage2;
#if defined(_MBS1)
	QAction *actionDigitalRGB;
	QAction *actionAnalogRGB;
#endif
	QAction *actionOpenGLSync;
	QAction *actionOpenGLAsync;
	QAction *actionGLFNear;
	QAction *actionGLFLinear;
	QMenu *menuRecordSound;
	QAction *actionStartRecordSound;
	QAction *actionStopRecordSound;
	QAction *actionSampleRate[6];
	QAction *actionSoundLate[6];
	QAction *actionLptSave[MAX_PRINTER];
	QAction *actionLptPrint[MAX_PRINTER];
	QAction *actionLptDirect[MAX_PRINTER];
	QAction *actionLptOnline[MAX_PRINTER];
	QAction *actionCommEnableServer[MAX_COMM];
	QMenu *menuCommConnect[MAX_COMM];
	QAction *actionCommByteData[MAX_COMM];
	QAction *actionCommTelnetBinary[MAX_COMM];
	QAction *actionCommTelnetEcho[MAX_COMM];
	QAction *actionShowLED;
	QAction *actionInsideLED;
	QAction *actionShowMessage;
	QAction *actionShowLogging;
#ifdef USE_PERFORMANCE_METER
	QAction *actionShowPMeter;
#endif
#ifdef USE_JOYSTICK
	QAction *actionUseJoypad[2];
#ifdef USE_KEY2JOYSTICK
	QAction *actionKey2Joypad;
#endif
	QAction *actionJoySetting;
#endif
#ifdef USE_LIGHTPEN
	QAction *actionLightPen;
#endif
#ifdef USE_MOUSE
	QAction *actionMouse;
#endif
	QAction *actionLoosenKeyStroke;
	QAction *actionKeybind;
	QAction *actionVirtualKeyboard;
#ifdef USE_DEBUGGER
	QAction *actionStartDebugger;
	QAction *actionStopDebugger;
#endif
	QAction *actionConfigure;

	static void connectWithNumber(QAction *sender, const char *signal, MainWindow *recv, const char *slot, int num);
};

extern MainWindow *mainwindow;

#endif /* QT_GUI_H */
;
