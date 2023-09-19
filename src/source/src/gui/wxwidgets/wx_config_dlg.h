/** @file wx_config_dlg.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2014.03.23

	@brief [ wx_config_dlg ]
*/

#ifndef WX_CONFIG_DLG_H
#define WX_CONFIG_DLG_H

#include "../../common.h"
#include "../../config.h"
#include "wx_dlg.h"
#include "../../cchar.h"
#include "../../cptrlist.h"

/**
	@brief Config dialog box
*/
class MyConfigDlg : public MyDialog
{
private:
	MyCheckBox *chkPowerOff;
#if defined(_MBS1)
	wxStaticText *staSysMode[2];
	MyRadioButton *radSysMode[2];
	MyCheckBox *chkDIPSwitch;
#else
	wxStaticText *staDIPSwitch[1];
	MyCheckBox *chkDIPSwitch[1];
#endif
	wxStaticText *staFDDType[4];
	MyRadioButton *radFDDType[4];
	wxStaticText *staIOPort[IOPORT_NUMS];
	MyCheckBox *chkIOPort[IOPORT_NUMS];
	wxCheckBox *chkFDMount[4];
	MyChoice *comGLUse;
	MyChoice *comGLFilter;
	MyChoice *comLedShow;
	MyChoice *comLedPos;
	wxChoice *comDisptmg;
	wxChoice *comCurdisp;
	MyChoice *comCapType;
	wxTextCtrl *txtSnapPath;
	MyButton   *btnSnapPath;
	wxTextCtrl *txtFontPath;
	MyButton   *btnFontPath;
	wxTextCtrl *txtMsgFont;
	wxTextCtrl *txtMsgSize;
	MyButton   *btnMsgFont;
	wxTextCtrl *txtInfoFont;
	wxTextCtrl *txtInfoSize;
	MyButton   *btnInfoFont;
	MyCheckBox *chkWavReverse;
	MyCheckBox *chkWavHalf;
	MyRadioButton *radWavNoCorrect;
	MyRadioButton *radWavCorrectCos;
	MyRadioButton *radWavCorrectSin;
	wxTextCtrl *txtCorrectAmp[2];
	wxChoice *comWavSampleRate;
	wxChoice *comWavSampleBits;
	MyCheckBox *chkDelayFd1;
	MyCheckBox *chkDelayFd2;
	MyCheckBox *chkFdDensity;
	MyCheckBox *chkFdMedia;
	MyCheckBox *chkFdSavePlain;
	wxTextCtrl *txtHostLPT[3];
	wxTextCtrl *txtPortLPT[3];
	wxTextCtrl *txtDelayLPT[3];
	wxTextCtrl *txtHostCOM[2];
	wxTextCtrl *txtPortCOM[2];
	MyChoice *comCOMBaud[2];
	MyChoice *comUartBaud;
	MyChoice *comUartDataBit;
	MyChoice *comUartParity;
	MyChoice *comUartStopBit;
	MyChoice *comUartFlowCtrl;
	wxTextCtrl *txtROMPath;
	wxButton   *btnROMPath;
#if defined(_MBS1)
	MyChoice *comExMem;
	MyCheckBox *chkMemNoWait;
#else
	MyCheckBox *chkUseExMem;
#endif
	MyCheckBox *chkUndefOp;
	MyCheckBox *chkClrCPUReg;
#if defined(_MBS1)
# if defined(USE_Z80B_CARD)
	MyChoice *comZ80BIntr;
# elif defined(USE_MPC_68008)
	MyCheckBox *chkAddrErr;
# endif
	MyCheckBox *chkFMOPN;
	MyChoice *comFMOPN;
	MyCheckBox *chkEXPSG;
	MyChoice *comEXPSG;
	MyChoice *comFMIntr;
#endif

	MyChoice *comLanguage;

	CPtrList<CTchar> lang_list;

	void InitDialog();
	void UpdateDialog();
	void ModifyParam();
	void change_fdd_type(int);
	void change_io_port(int);

	void OnChangeCorrect(wxCommandEvent &);
	void OnChangeFddType(wxCommandEvent &);
	void OnChangeIOPort(wxCommandEvent &);
	void OnChangeSoundCard(wxCommandEvent &);
	void OnShowFolderDialog(wxCommandEvent &);
	void OnShowFontDialog(wxCommandEvent &);

public:
	MyConfigDlg(wxWindow *, wxWindowID, EMU *, GUI_BASE *);
	~MyConfigDlg();

	int ShowModal();

	enum {
		IDC_NOTEBOOK = 1,
		IDC_STATIC_NOFDD,
		IDC_STATIC_3FDD,
		IDC_STATIC_5FDD,
	};

	DECLARE_EVENT_TABLE()
};

#endif /* WX_CONFIG_DLG_H */
