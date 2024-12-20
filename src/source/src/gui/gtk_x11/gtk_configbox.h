/** @file gtk_configbox.h

	HITACHI BASIC MASTER LEVEL3 Mark5 Emulator 'EmuB-6892'
	HITACHI MB-S1 Emulator 'EmuB-S1'
	Skelton for retropc emulator

	@author Sasaji
	@date   2015.05.10 -

	@brief [ config box ]
*/

#ifndef GUI_GTK_CONFIGBOX_H
#define GUI_GTK_CONFIGBOX_H

#include "../../common.h"
#include <gtk/gtk.h>
#include "gtk_dialogbox.h"
#include "../../vm/vm.h"
#include "../../config.h"
#include "../../cchar.h"
#include "../../cptrlist.h"

namespace GUI_GTK_X11
{

/**
	@brief Configure dialog box
*/
class ConfigBox : public DialogBox
{
private:
	GtkWidget *chkPowerOff;
#if defined(_MBS1)
	GtkWidget *radSYS[2];
#endif
	GtkWidget *chkMODE;
	GtkWidget *radFDD[4];
	GtkWidget *chkIO[IOPORT_NUMS];

	GtkWidget *comUseOpenGL;
	GtkWidget *comGLFilter;

	GtkWidget *comDispSkew;
	GtkWidget *comCurdSkew;

	GtkWidget *comLEDShow;
	GtkWidget *comLEDPos;

	GtkWidget *comCapType;

	GtkWidget *txtSnapPath;
	GtkWidget *txtFontPath;
	GtkWidget *txtMsgFontName;
	GtkWidget *txtMsgFontSize;
	GtkWidget *txtInfoFontName;
	GtkWidget *txtInfoFontSize;

#ifdef USE_DATAREC
	GtkWidget *chkReverseWave;
	GtkWidget *chkHalfWave;
	GtkWidget *radCorrectType[3];
	GtkWidget *txtCorrectAmp[2];

	GtkWidget *comSampleRate;
	GtkWidget *comSampleBits;
#endif

#ifdef USE_FD1
	GtkWidget *chkFDMount[USE_FLOPPY_DISKS];
	GtkWidget *chkDelayFd1;
	GtkWidget *chkDelayFd2;
	GtkWidget *chkFdDensity;
	GtkWidget *chkFdMedia;
	GtkWidget *chkFdSavePlain;
#endif

#ifdef USE_HD1
	GtkWidget *chkHDMount[USE_HARD_DISKS];

	GtkWidget *chkDelayHd2;
#endif

#ifdef MAX_PRINTER
	GtkWidget *txtLPTHost[MAX_PRINTER];
	GtkWidget *txtLPTPort[MAX_PRINTER];
	GtkWidget *txtLPTDelay[MAX_PRINTER];
#endif
#ifdef MAX_COMM
	GtkWidget *txtCOMHost[MAX_COMM];
	GtkWidget *txtCOMPort[MAX_COMM];
	GtkWidget *comCOMBaud[MAX_COMM];
#endif
#ifdef USE_DEBUGGER
	GtkWidget *txtDbgrHost;
	GtkWidget *txtDbgrPort;
#endif
	GtkWidget *comCOMUartBaud;
	GtkWidget *comCOMUartDataBit;
	GtkWidget *comCOMUartParity;
	GtkWidget *comCOMUartStopBit;
	GtkWidget *comCOMUartFlowCtrl;

	GtkWidget *txtROMPath;
	GtkWidget *chkUndefOp;
	GtkWidget *chkClrCPUReg;
#if defined(_MBS1)
	GtkWidget *comExRam;
	GtkWidget *chkMemNoWait;

	GtkWidget *chkFmOpnEn;
	GtkWidget *comFmOpnClk;
	GtkWidget *comFmOpnChip;
	GtkWidget *chkExPsgEn;
	GtkWidget *comExPsgChip;
	GtkWidget *comFmOpnIrq;
#if defined(USE_Z80B_CARD)
	GtkWidget *comZ80CardIrq;
#elif defined(USE_MPC_68008)
	GtkWidget *chkAddrErr;
#endif
#else
	GtkWidget *chkExMem;
#endif

	GtkWidget *comLanguage;

	CPtrList<CTchar> lang_list;

	bool SetData();
	void ShowFolderBox(const char *title, GtkWidget *entry);
	void ShowFontFileBox(const char *title, GtkWidget *entry);
	void ChangeFDDType(int index);
	void ChangeIOPort(int index);
	void ChangeFmOpn();
	void ChangeExPsg();

	static void OnChangedFDD(GtkWidget *widget, gpointer user_data);
	static void OnChangedIO(GtkWidget *widget, gpointer user_data);
	static void OnChangedFmOpn(GtkWidget *widget, gpointer user_data);
	static void OnChangedExPsg(GtkWidget *widget, gpointer user_data);

	static void OnSelectSnapPath(GtkWidget *widget, gpointer user_data);
	static void OnSelectFontPath(GtkWidget *widget, gpointer user_data);
	static void OnSelectROMPath(GtkWidget *widget, gpointer user_data);
	static void OnSelectMessageFont(GtkWidget *widget, gpointer user_data);
	static void OnSelectInfoFont(GtkWidget *widget, gpointer user_data);

	static void OnResponse(GtkWidget *widget, gint response_id, gpointer user_data);

public:
	ConfigBox(GUI *new_gui);
	~ConfigBox();
	bool Show(GtkWidget *parent_window);
	void Hide();
};

}; /* namespace GUI_GTK_X11 */

#endif /* GUI_GTK_CONFIGBOX_H */
