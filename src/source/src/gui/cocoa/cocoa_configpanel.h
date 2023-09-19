/** @file cocoa_configpanel.h

 HITACHI BASIC MASTER LEVEL3 Mark5 Emulator 'EmuB-6892'
 HITACHI MB-S1 Emulator 'EmuB-S1'
 Skelton for retropc emulator
 SDL edition + Cocoa GUI

 @author Sasaji
 @date   2015.04.30 -

 @brief [ config panel ]
 */

#ifndef COCOA_CONFIGPANEL_H
#define COCOA_CONFIGPANEL_H

#import <Cocoa/Cocoa.h>
#import "cocoa_basepanel.h"
#import "../../vm/vm.h"
#import "../../config.h"
#import "../../cchar.h"
#import "../../cptrlist.h"

/**
	@brief Config dialog box
*/
@interface CocoaConfigPanel : CocoaBasePanel
{
	CocoaCheckBox *chkPowerOff;
#if defined(_MBS1)
	CocoaRadioButton *radSysMode[2];
#endif
	CocoaCheckBox *chkDipswitch;
	CocoaRadioButton *radFddType[4];
	CocoaCheckBox *chkFddMount[MAX_DRIVE];
	CocoaCheckBox *chkIOPort[IOPORT_NUMS];

	CocoaPopUpButton *popUseOpenGL;
	CocoaPopUpButton *popGLFilter;
	CocoaPopUpButton *popLEDShow;
	CocoaPopUpButton *popLEDPosition;
	CocoaPopUpButton *popCurdispSkew;
	CocoaPopUpButton *popDisptmgSkew;
	CocoaPopUpButton *popCaptureType;

	CocoaTextField *txtSnapPath;
	CocoaTextField *txtFontPath;
	CocoaTextField *txtMsgFontName;
	CocoaTextField *txtMsgFontSize;
	CocoaTextField *txtInfoFontName;
	CocoaTextField *txtInfoFontSize;

#ifdef USE_DATAREC
	CocoaCheckBox *chkReverseWave;
	CocoaCheckBox *chkHalfWave;
//	CocoaCheckBox *chkCorrectWave;
	CocoaRadioGroup *radCorrect;
	CocoaTextField *txtCorrectAmp[2];
	CocoaPopUpButton *popSampleRate;
	CocoaPopUpButton *popSampleBits;
#endif

#ifdef USE_FD1
	CocoaCheckBox *chkDelayFd1;
	CocoaCheckBox *chkDelayFd2;
	CocoaCheckBox *chkFdDensity;
	CocoaCheckBox *chkFdMedia;
	CocoaCheckBox *chkFdSavePlain;
#endif

#ifdef MAX_PRINTER
	CocoaTextField *txtLPTHost[MAX_PRINTER];
	CocoaTextField *txtLPTPort[MAX_PRINTER];
	CocoaTextField *txtLPTDelay[MAX_PRINTER];
#endif
#ifdef MAX_COMM
	CocoaTextField *txtCOMHost[MAX_COMM];
	CocoaTextField *txtCOMPort[MAX_COMM];
	CocoaPopUpButton *popCOMDipswitch[MAX_COMM];
#endif
#ifdef USE_DEBUGGER
	CocoaTextField *txtDbgrHost;
	CocoaTextField *txtDbgrPort;
#endif
	CocoaPopUpButton *popCOMUartBaud;
	CocoaPopUpButton *popCOMUartDataBit;
	CocoaPopUpButton *popCOMUartParity;
	CocoaPopUpButton *popCOMUartStopBit;
	CocoaPopUpButton *popCOMUartFlowCtrl;

	CocoaTextField *txtROMPath;

	CocoaCheckBox *chkUndefOp;
	CocoaCheckBox *chkClrCPUReg;

#if defined(_MBS1)
# if defined(USE_Z80B_CARD)
	CocoaPopUpButton *popZ80BCardIrq;
# elif defined(USE_MPC_68008)
	CocoaCheckBox *chkAddrErr;
# endif

	CocoaPopUpButton *popExtRam;
	CocoaCheckBox *chkMemNoWait;

	CocoaCheckBox *chkFmOpnEn;
//	CocoaPopUpButton *popFmOpnClk;
	CocoaPopUpButton *popFmOpnChip;
	CocoaCheckBox *chkExPsgEn;
	CocoaPopUpButton *popExPsgChip;
	CocoaPopUpButton *popFmOpnIrq;
#else
	CocoaCheckBox *chkExMem;
#endif

	CocoaPopUpButton *popLanguage;
	CPtrList<CTchar> lang_list;
}
- (id)init;
- (NSInteger)runModal;
- (void)close;
- (void)dialogCancel:(id)sender;
- (void)dialogOk:(id)sender;
- (void)showFolderPanel:(id)sender;
- (void)showFilePanel:(id)sender;
- (void)showFontPanel:(id)sender;
- (void)changeFont:(id)sender;

#if defined(_MBS1)
- (void)selectSysMode:(CocoaRadioButton *)sender;
- (void)selectFmOpn:(CocoaCheckBox *)sender;
- (void)selectExPsg:(CocoaCheckBox *)sender;
#endif
- (void)selectFddType:(CocoaRadioButton *)sender;
- (void)selectIO:(CocoaCheckBox *)sender;
- (void)selectCorrect:(CocoaCheckBox *)sender;
@end

#endif /* COCOA_CONFIGPANEL_H */
