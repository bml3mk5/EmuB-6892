/** @file qt_configbox.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2016.03.01

	@brief [ qt config box ]
*/

#ifndef QT_CONFIGBOX_H
#define QT_CONFIGBOX_H

#include <QDialog>
#include <QLabel>
#include <QRadioButton>
#include <QCheckBox>
#include <QLineEdit>
#include <QDialogButtonBox>
#include "../../config.h"
#include "qt_dialog.h"
#include "../../cchar.h"
#include "../../cptrlist.h"

//namespace Ui {
//class MyConfigBox;
//}

/**
	@brief Config dialog box
*/
class MyConfigBox : public QDialog
{
	Q_OBJECT

public:
	explicit MyConfigBox(QWidget *parent = nullptr);
	~MyConfigBox();

public slots:
	int exec();
	void toggledIOPort(bool checked);
	void toggledFmopn(bool checked);
	void toggledExpsg(bool checked);
	void pressedSnapPath();
	void pressedMsgFont();
	void pressedInfoFont();
	void pressedRomPath();

private:
//	Ui::MyConfigBox *ui;
	QLabel *lblFddTypes[4];
	QRadioButton *radFddTypes[4];
	QLabel *lblIOPorts[IOPORT_NUMS];
	QCheckBox *chkIOPorts[IOPORT_NUMS];

	QCheckBox *chkPowerOff;
#if defined(_MBS1)
	QRadioButton *radSysMode[2];
	QCheckBox *chkDipSwitch;
#else
	QCheckBox *chkModeSwitch;
#endif

	MyComboBox *comUseOpenGL;
	MyComboBox *comGLFilter;

	MyComboBox *comCRTCdisptmg;
	MyComboBox *comCRTCcurdisp;

	MyComboBox *comLED;
	MyComboBox *comLEDPos;

	MyComboBox *comCapType;

	QLineEdit *linSnapPath;
	QLineEdit *linFontPath;

	QLineEdit *linMsgFont;
	QLineEdit *linMsgSize;

	QLineEdit *linInfoFont;
	QLineEdit *linInfoSize;

	MyComboBox *comLanguage;
	CPtrList<CTchar> lang_list;

	QCheckBox *chkReverse;
	QCheckBox *chkHalf;
	QRadioButton *radNoCorr;
	QRadioButton *radCOS;
	QRadioButton *radSIN;

	QLineEdit *linCorrAmp[2];

	MyComboBox *comRate;
	MyComboBox *comBits;

	QCheckBox *chkDrive[MAX_DRIVE];
	QCheckBox *chkDelayFd1;
	QCheckBox *chkDelayFd2;
	QCheckBox *chkFdDensity;
	QCheckBox *chkFdMedia;

	QLineEdit *linLPTHost[MAX_PRINTER];
	QLineEdit *linLPTPort[MAX_PRINTER];
	QLineEdit *linLPTDelay[MAX_PRINTER];
	QLineEdit *linCOMHost[MAX_COMM];
	QLineEdit *linCOMPort[MAX_COMM];
	MyComboBox *comCOMBaud[MAX_COMM];

	QLineEdit *linRomPath;
#if defined(_MBS1)
	MyComboBox *comExMem;
	QCheckBox *chkMemNoWait;
#else
	QCheckBox *chkExMem;
#endif
	QCheckBox *chkUndefOp;
	QCheckBox *chkClrCPUReg;

#if defined(_MBS1)
# if defined(USE_Z80B_CARD)
	MyComboBox *comZ80BIntr;
# elif defined(USE_MPC_68008)
	QCheckBox *chkAddrErr;
# endif
	QCheckBox *chkFmopn;
	MyComboBox *comFmopn;
	QCheckBox *chkExpsg;
	MyComboBox *comExpsg;
	MyComboBox *comFMIntr;
#endif

	void setDatas();
};

#endif // QT_CONFIGBOX_H
