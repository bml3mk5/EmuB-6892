/** @file cocoa_configpanel.mm

 HITACHI BASIC MASTER LEVEL3 Mark5 Emulator 'EmuB-6892'
 HITACHI MB-S1 Emulator 'EmuB-S1'
 Skelton for retropc emulator
 SDL edition + Cocoa GUI

 @author Sasaji
 @date   2015.04.30 -

 @brief [ config panel ]
 */

#import "cocoa_gui.h"
#import "cocoa_configpanel.h"
#import "../../config.h"
#import "../../emu.h"
#import "../gui.h"
#import "../../clocale.h"
#import "../../msgboard.h"
#import "../../msgs.h"
#import "../../rec_video_defs.h"
#import "cocoa_fontpanel.h"
#import "../../labels.h"

extern EMU *emu;
extern GUI *gui;

#ifdef USE_IOPORT_FDD
#define IOPORT_STARTNUM 0
#else
#define IOPORT_STARTNUM 2
#endif

@implementation CocoaConfigPanel
- (id)init
{
	[super init];

	[self setTitleById:CMsg::Configure];
	[self setShowsResizeIndicator:FALSE];

	CocoaView *view = [self contentView];

	CocoaLayout *box_all = [CocoaLayout create:VerticalBox :0 :COCOA_DEFAULT_MARGIN :_T("box_all")];
	CocoaLayout *box_tab = [box_all addBox:TabViewBox :0 :COCOA_DEFAULT_MARGIN :_T("box_tab")];

	CocoaTabView *tabView = [CocoaTabView createI:LABELS::tabs];
	[box_tab addControl:tabView :300 :200];
	[view addSubview:tabView];

	NSTabViewItem *tab;
	CocoaView *tab_view;
	CocoaBox *box;
	CocoaView *box_view;
	CocoaLabel *lbl;
	CocoaButton *btn;

	CocoaLayout *box_one;
	CocoaLayout *sbox;
	CocoaLayout *lbox;
	CocoaLayout *rbox;
	CocoaLayout *bbox;
	CocoaLayout *vbox;
	CocoaLayout *hbox;

	int i;

	_TCHAR bname[10];

	// Mode tab
	tab = [tabView tabViewItemAtIndex:0];
	tab_view = (CocoaView *)[tab view];

	box_one = [box_tab addBox:VerticalBox :0 :0 :_T("mode")];
	sbox = [box_one addBox:HorizontalBox :0 :0 :_T("modeS")];
	lbox = [sbox addBox:VerticalBox :0 :0 :_T("modeL")];

#if defined(_MBS1)
	//
	bbox = [lbox addBox:BoxViewBox :0 :COCOA_DEFAULT_MARGIN :_T("SysModeB")];

	box = [CocoaBox createI:CMsg::System_Mode_ASTERISK];
	[bbox addControl:box :260 :1];
	box_view = [box contentView];

	vbox = [bbox addBox:VerticalBox :0 :0 :_T("SysModA")];

	for(i = 0; i < 2; i++) {
		hbox = [vbox addBox:HorizontalBox :0 :0 :_T("SysModH")];

		lbl = [CocoaLabel createT:(config.sys_mode & 1) == (1 - i) ? ">" : " "];
		[hbox addControl:lbl width:16];
		[box_view addSubview:lbl];

		radSysMode[i] = [CocoaRadioButton createI:LABELS::sys_mode[i] index:i action:@selector(selectSysMode:) value:(emu->get_parami(VM::ParamSysMode) & 1) == (1 - i)];
		[hbox addControl:radSysMode[i]];
		[box_view addSubview:radSysMode[i]];
	}
	hbox = [vbox addBox:HorizontalBox :0 :0 :_T("Newon7H")];
	[hbox addSpace:32 :20];
	chkDipswitch = [CocoaCheckBox createI:CMsg::NEWON7 action:nil value:(config.dipswitch & 4)];
	[hbox addControl:chkDipswitch];
	[box_view addSubview:chkDipswitch];
#else
	//
	bbox = [lbox addBox:BoxViewBox :0 :COCOA_DEFAULT_MARGIN :_T("DipSwB")];

	box = [CocoaBox createI:CMsg::DIP_Switch_ASTERISK];
	[bbox addControl:box :260 :1];
	box_view = [box contentView];

	vbox = [bbox addBox:VerticalBox :0 :0 :_T("DipSwA")];
	hbox = [vbox addBox:HorizontalBox :0 :0 :_T("DipSwH")];

	lbl = [CocoaLabel createT:(config.dipswitch & 4) ? ">" : " "];
	[hbox addControl:lbl width:16];
	[box_view addSubview:lbl];

	chkDipswitch = [CocoaCheckBox createI:CMsg::MODE_Switch action:nil value:(config.dipswitch & 4)];
	[hbox addControl:chkDipswitch];
	[box_view addSubview:chkDipswitch];
#endif

	[tab_view addSubview:box];

	//
	bbox = [lbox addBox:BoxViewBox :0 :COCOA_DEFAULT_MARGIN :_T("FddTypeB")];

	box = [CocoaBox createI:CMsg::FDD_Type_ASTERISK];
	[bbox addControl:box :260 :1];
	box_view = [box contentView];

	vbox = [bbox addBox:VerticalBox :0 :0 :_T("FddTypeA")];

	for(i = 0; i < 4; i++) {
		hbox = [vbox addBox:HorizontalBox :0 :0 :_T("FddTypeH")];

		lbl = [CocoaLabel createT:(config.fdd_type == i) ? ">" : " "];
		[hbox addControl:lbl width:16];
		[box_view addSubview:lbl];

		radFddType[i] = [CocoaRadioButton createI:LABELS::fdd_type[i] index:i action:@selector(selectFddType:) value:emu->get_parami(VM::ParamFddType) == i];
		[hbox addControl:radFddType[i]];
		[box_view addSubview:radFddType[i]];
	}

	[tab_view addSubview:box];

	chkPowerOff = [CocoaCheckBox createI:CMsg::Enable_the_state_of_power_off action:nil value:config.use_power_off];
	[lbox addControl:chkPowerOff];
	[tab_view addSubview:chkPowerOff];

	//
	rbox = [sbox addBox:VerticalBox :0 :0 :_T("modeR")];

	bbox = [rbox addBox:BoxViewBox :0 :COCOA_DEFAULT_MARGIN :_T("IOBOX")];

	box = [CocoaBox createI:CMsg::I_O_Port_Address_ASTERISK];
	[bbox addControl:box :320 :1];
	box_view = [box contentView];

	vbox = [bbox addBox:VerticalBox :0 :0 :_T("IOVER")];
	for(i=0; LABELS::io_port[i] != CMsg::End; i++) {
		int pos = LABELS::io_port_pos[i];
		if ((1 << pos) & IOPORT_MSK_ALL) {
			_stprintf(bname, _T("IO%dH"), pos);
			hbox = [vbox addBox:HorizontalBox :0 :0 :bname];
			lbl = [CocoaLabel createT:(config.io_port & (1 << pos)) ? ">" : " "];
			[hbox addControl:lbl width:16];
			[box_view addSubview:lbl];

			chkIOPort[pos] = [CocoaCheckBox createI:LABELS::io_port[i] index:pos action:@selector(selectIO:) value:(emu->get_parami(VM::ParamIOPort) & (1 << pos))];
			[hbox addControl:chkIOPort[pos]];
			[box_view addSubview:chkIOPort[pos]];
			[self selectIO:chkIOPort[pos]];
		}
	}
	[tab_view addSubview:box];

	//
	lbl = [CocoaLabel createI:CMsg::Need_restart_program_or_PowerOn];
	[box_one addControl:lbl];
	[tab_view addSubview:lbl];


	// Screen tab
	tab = [tabView tabViewItemAtIndex:1];
	tab_view = (CocoaView *)[tab view];

	box_one = [box_tab addBox:VerticalBox :0 :0 :_T("Scrn")];
	sbox = [box_one addBox:HorizontalBox :0 :0 :_T("ScrnS")];
	lbox = [sbox addBox:VerticalBox :0 :0 :_T("ScrnL")];

	//
	bbox = [lbox addBox:BoxViewBox :0 :COCOA_DEFAULT_MARGIN :_T("OpenGLB")];
	box = [CocoaBox createI:CMsg::Drawing];
	[bbox addControl:box :260 :1];
	box_view = [box contentView];

	vbox = [bbox addBox:VerticalBox :0 :0 :_T("OpenGLV")];
	hbox = [vbox addBox:HorizontalBox :MiddlePos :0 :_T("UseGLH")];

	lbl = [CocoaLabel createI:CMsg::Method_ASTERISK];
	[hbox addControl:lbl];
	[box_view addSubview:lbl];

	popUseOpenGL = [CocoaPopUpButton createI:LABELS::opengl_use action:nil selidx:config.use_opengl];
	[hbox addControl:popUseOpenGL];
	[box_view addSubview:popUseOpenGL];

	hbox = [vbox addBox:HorizontalBox :MiddlePos :0 :_T("GLFilH")];

	lbl = [CocoaLabel createI:CMsg::Filter_Type];
	[hbox addControl:lbl];
	[box_view addSubview:lbl];

	popGLFilter = [CocoaPopUpButton createI:LABELS::opengl_filter action:nil selidx:config.gl_filter_type];
	[hbox addControl:popGLFilter];
	[box_view addSubview:popGLFilter];

	[tab_view addSubview:box];

	//
	rbox = [sbox addBox:VerticalBox :0 :0 :_T("CRTCR")];

	bbox = [rbox addBox:BoxViewBox :0 :COCOA_DEFAULT_MARGIN :_T("CRTCB")];
	box = [CocoaBox createI:CMsg::CRTC];
	[bbox addControl:box :260 :1];
	box_view = [box contentView];

	vbox = [bbox addBox:VerticalBox :0 :0 :_T("CRTCV")];
	hbox = [vbox addBox:HorizontalBox :MiddlePos :0 :_T("DISPTMG")];

	lbl = [CocoaLabel createI:CMsg::Disptmg_Skew];
	[hbox addControl:lbl];
	[box_view addSubview:lbl];

#if defined(_MBS1)
	popDisptmgSkew = [CocoaPopUpButton createT:LABELS::disp_skew action:nil selidx:(config.disptmg_skew + 2)];
#else
	popDisptmgSkew = [CocoaPopUpButton createT:LABELS::disp_skew action:nil selidx:config.disptmg_skew];
#endif
	[hbox addControl:popDisptmgSkew];
	[box_view addSubview:popDisptmgSkew];

	hbox = [vbox addBox:HorizontalBox :MiddlePos :0 :_T("CURDISP")];

#if defined(_MBS1)
	lbl = [CocoaLabel createI:CMsg::Curdisp_Skew_L3];
#else
	lbl = [CocoaLabel createI:CMsg::Curdisp_Skew];
#endif
	[hbox addControl:lbl];
	[box_view addSubview:lbl];

#if defined(_MBS1)
	popCurdispSkew = [CocoaPopUpButton createT:LABELS::disp_skew action:nil selidx:(config.curdisp_skew + 2)];
#else
	popCurdispSkew = [CocoaPopUpButton createT:LABELS::disp_skew action:nil selidx:config.curdisp_skew];
#endif
	[hbox addControl:popCurdispSkew];
	[box_view addSubview:popCurdispSkew];

	[tab_view addSubview:box];

	//
	hbox = [box_one addBox:HorizontalBox :MiddlePos :0 :_T("LEDH")];

	lbl = [CocoaLabel createI:CMsg::LED];
	[hbox addControl:lbl];
	[tab_view addSubview:lbl];

	int led_show = gui->GetLedBoxPhase(-1);
	popLEDShow = [CocoaPopUpButton createI:LABELS::led_show action:nil selidx:led_show];
	[hbox addControl:popLEDShow];
	[tab_view addSubview:popLEDShow];

	hbox = [box_one addBox:HorizontalBox :MiddlePos :0 :_T("LEDPOSH")];

	lbl = [CocoaLabel createI:CMsg::Position];
	[hbox addControl:lbl];
	[tab_view addSubview:lbl];

	popLEDPosition = [CocoaPopUpButton createI:LABELS::led_pos action:nil selidx:config.led_pos];
	[hbox addControl:popLEDPosition];
	[tab_view addSubview:popLEDPosition];

	//
	hbox = [box_one addBox:HorizontalBox :MiddlePos :0 :_T("CaptureH")];

	lbl = [CocoaLabel createI:CMsg::Capture_Type];
	[hbox addControl:lbl];
	[tab_view addSubview:lbl];

	popCaptureType = [CocoaPopUpButton createT:LABELS::capture_fmt action:nil selidx:config.capture_type];
	[hbox addControl:popCaptureType];
	[tab_view addSubview:popCaptureType];

	//
	hbox = [box_one addBox:HorizontalBox :MiddlePos :0 :_T("SnapPath")];

	lbl = [CocoaLabel createI:CMsg::Snapshot_Path];
	[hbox addControl:lbl];
	[tab_view addSubview:lbl];

	txtSnapPath = [CocoaTextField createT:config.snapshot_path action:nil];
	[hbox addControl:txtSnapPath width:360];
	[tab_view addSubview:txtSnapPath];

	btn = [CocoaButton createI:CMsg::Folder_ action:@selector(showFolderPanel:)];
	[btn setRelatedObject:txtSnapPath];
	[hbox addControl:btn];
	[tab_view addSubview:btn];

	//
	hbox = [box_one addBox:HorizontalBox :MiddlePos :0 :_T("FontPath")];

	lbl = [CocoaLabel createI:CMsg::Font_Path];
	[hbox addControl:lbl];
	[tab_view addSubview:lbl];

	txtFontPath = [CocoaTextField createT:config.font_path action:nil];
	[hbox addControl:txtFontPath width:360];
	[tab_view addSubview:txtFontPath];

	btn = [CocoaButton createI:CMsg::Folder_ action:@selector(showFolderPanel:)];
	[btn setRelatedObject:txtFontPath];
	[hbox addControl:btn];
	[tab_view addSubview:btn];

	//
	hbox = [box_one addBox:HorizontalBox :MiddlePos :0 :_T("MsgFont")];

	lbl = [CocoaLabel createI:CMsg::Message_Font];
	[hbox addControl:lbl];
	[tab_view addSubview:lbl];

	txtMsgFontName = [CocoaTextField createT:config.msgboard_msg_fontname action:nil];
	[hbox addControl:txtMsgFontName width:200];
	[tab_view addSubview:txtMsgFontName];

	lbl = [CocoaLabel createI:CMsg::_Size];
	[hbox addControl:lbl];
	[tab_view addSubview:lbl];

	txtMsgFontSize = [CocoaTextField createN:config.msgboard_msg_fontsize action:nil];
	[hbox addControl:txtMsgFontSize width:100];
	[tab_view addSubview:txtMsgFontSize];

	btn = [CocoaButton createI:CMsg::File_ action:@selector(showFilePanel:)];
	[btn setRelatedObject:txtMsgFontName];
	[hbox addControl:btn];
	[tab_view addSubview:btn];

	//
	hbox = [box_one addBox:HorizontalBox :MiddlePos :0 :_T("InfoFont")];

	lbl = [CocoaLabel createI:CMsg::Info_Font];
	[hbox addControl:lbl];
	[tab_view addSubview:lbl];

	txtInfoFontName = [CocoaTextField createT:config.msgboard_info_fontname action:nil];
	[hbox addControl:txtInfoFontName width:200];
	[tab_view addSubview:txtInfoFontName];

	lbl = [CocoaLabel createI:CMsg::_Size];
	[hbox addControl:lbl];
	[tab_view addSubview:lbl];

	txtInfoFontSize = [CocoaTextField createN:config.msgboard_info_fontsize action:nil];
	[hbox addControl:txtInfoFontSize width:100];
	[tab_view addSubview:txtInfoFontSize];

	btn = [CocoaButton createI:CMsg::File_ action:@selector(showFilePanel:)];
	[btn setRelatedObject:txtInfoFontName];
	[hbox addControl:btn];
	[tab_view addSubview:btn];

	//
	lang_list.Clear();
	clocale->GetLocaleNamesWithDefault(lang_list);

	int lang_selidx = clocale->SelectLocaleNameIndex(lang_list, config.language);

	hbox = [box_one addBox:HorizontalBox :MiddlePos :0 :_T("Lang")];
	lbl = [CocoaLabel createI:CMsg::Language_ASTERISK];
	[hbox addControl:lbl];
	[tab_view addSubview:lbl];

	popLanguage = [CocoaPopUpButton createL:&lang_list action:nil selidx:lang_selidx];
	[hbox addControl:popLanguage];
	[tab_view addSubview:popLanguage];

	//
	hbox = [box_one addBox:HorizontalBox :MiddlePos :0 :_T("NeedRes")];

	lbl = [CocoaLabel createI:CMsg::Need_restart_program];
	[hbox addControl:lbl];
	[tab_view addSubview:lbl];

	// Tape tab
	tab = [tabView tabViewItemAtIndex:2];
	tab_view = (CocoaView *)[tab view];

	box_one = [box_tab addBox:VerticalBox :0 :0 :_T("Tape")];
	sbox = [box_one addBox:HorizontalBox :0 :0 :_T("TapeS")];
	lbox = [sbox addBox:VerticalBox :0 :0 :_T("TapeL")];

	//
	bbox = [lbox addBox:BoxViewBox :0 :COCOA_DEFAULT_MARGIN :_T("LoadWavB")];
	box = [CocoaBox createI:CMsg::Load_Wav_File_from_Tape];
	[bbox addControl:box :300 :1];
	box_view = [box contentView];

	vbox = [bbox addBox:VerticalBox :0 :0 :_T("LoadWavV")];
	hbox = [vbox addBox:HorizontalBox :MiddlePos :0 :_T("LoadWavH")];

	chkReverseWave = [CocoaCheckBox createI:CMsg::Reverse_Wave action:nil value:config.wav_reverse];
	[hbox addControl:chkReverseWave];
	[box_view addSubview:chkReverseWave];

	chkHalfWave = [CocoaCheckBox createI:CMsg::Half_Wave action:nil value:config.wav_half];
	[hbox addControl:chkHalfWave];
	[box_view addSubview:chkHalfWave];

	hbox = [vbox addBox:HorizontalBox :MiddlePos :0 :_T("CorrH")];

	lbl = [CocoaLabel createI:CMsg::Correct];
	[hbox addControl:lbl];
	[box_view addSubview:lbl];

	int corr_idx = config.wav_correct ? config.wav_correct_type + 1 : 0;

	radCorrect = [CocoaRadioGroup create:260 cols:3 titleids:LABELS::correct action:nil selidx:corr_idx];
	[hbox addControl:radCorrect];
	[box_view addSubview:radCorrect];

	hbox = [vbox addBox:HorizontalBox :MiddlePos :0 :_T("CorrAmpH")];

	for(i=0; i<2; i++) {
		lbl = [CocoaLabel createT:i == 0 ? "1200Hz" : "2400Hz"];
		[hbox addControl:lbl];
		[box_view addSubview:lbl];

		txtCorrectAmp[i] = [CocoaTextField createN:config.wav_correct_amp[i] action:nil align:NSRightTextAlignment];
		[hbox addControl:txtCorrectAmp[i] width:60];
		[box_view addSubview:txtCorrectAmp[i]];
	}

	[tab_view addSubview:box];

	//
	rbox = [sbox addBox:VerticalBox :0 :0 :_T("TapeR")];

	bbox = [rbox addBox:BoxViewBox :0 :COCOA_DEFAULT_MARGIN :_T("SaveWavB")];
	box = [CocoaBox createI:CMsg::Save_Wav_File_to_Tape];
	[bbox addControl:box :260 :1];
	box_view = [box contentView];

	vbox = [bbox addBox:VerticalBox :0 :0 :_T("SaveWavV")];
	hbox = [vbox addBox:HorizontalBox :MiddlePos :0 :_T("SaveRatH")];

	lbl = [CocoaLabel createI:CMsg::Sample_Rate];
	[hbox addControl:lbl];
	[box_view addSubview:lbl];

	popSampleRate = [CocoaPopUpButton createT:LABELS::sound_rate action:nil selidx:config.wav_sample_rate];
	[hbox addControl:popSampleRate];
	[box_view addSubview:popSampleRate];

	hbox = [vbox addBox:HorizontalBox :MiddlePos :0 :_T("SaveBitH")];

	lbl = [CocoaLabel createI:CMsg::Sample_Bits];
	[hbox addControl:lbl];
	[box_view addSubview:lbl];

	popSampleBits = [CocoaPopUpButton createT:LABELS::sound_bits action:nil selidx:config.wav_sample_bits];
	[hbox addControl:popSampleBits];
	[box_view addSubview:popSampleBits];

	[tab_view addSubview:box];

	// FDD

	bbox = [box_one addBox:BoxViewBox :0 :COCOA_DEFAULT_MARGIN :_T("Fdd2B")];
	box = [CocoaBox createI:CMsg::Floppy_Disk_Drive];
	[bbox addControl:box :320 :1];
	box_view = [box contentView];

	vbox = [bbox addBox:VerticalBox :0 :0 :_T("Fdd2V")];
	hbox = [vbox addBox:HorizontalBox :MiddlePos :0 :_T("MountH")];

	// mount fdd
	lbl = [CocoaLabel createI:CMsg::When_start_up_mount_disk_at_];
	[hbox addControl:lbl];
	[box_view addSubview:lbl];
	for(i=0; i<MAX_DRIVE; i++) {
		char name[4];
		sprintf(name, "%d ", i);
		chkFddMount[i] = [CocoaCheckBox createT:name action:nil value:(config.mount_fdd & (1 << i))];
		[hbox addControl:chkFddMount[i]];
		[box_view addSubview:chkFddMount[i]];
	}

	chkDelayFd1 = [CocoaCheckBox createI:CMsg::Ignore_delays_to_find_sector action:nil value:(FLG_DELAY_FDSEARCH != 0)];
	[vbox addControl:chkDelayFd1];
	[box_view addSubview:chkDelayFd1];

	chkDelayFd2 = [CocoaCheckBox createI:CMsg::Ignore_delays_to_seek_track action:nil value:(FLG_DELAY_FDSEEK != 0)];
	[vbox addControl:chkDelayFd2];
	[box_view addSubview:chkDelayFd2];

	chkFdDensity = [CocoaCheckBox createI:CMsg::Suppress_checking_for_density action:nil value:(FLG_CHECK_FDDENSITY == 0)];
	[vbox addControl:chkFdDensity];
	[box_view addSubview:chkFdDensity];

	chkFdMedia = [CocoaCheckBox createI:CMsg::Suppress_checking_for_media_type action:nil value:(FLG_CHECK_FDMEDIA == 0)];
	[vbox addControl:chkFdMedia];
	[box_view addSubview:chkFdMedia];

	[tab_view addSubview:box];

	// Network tab
	tab = [tabView tabViewItemAtIndex:3];
	tab_view = (CocoaView *)[tab view];

	box_one = [box_tab addBox:VerticalBox :0 :0 :_T("Net")];

	for(i=0; i<MAX_PRINTER; i++) {
		char name[128];
		sprintf(name, CMSG(LPTVDIGIT_Hostname), i);

		_stprintf(bname, _T("LPT%d"), i);
		hbox = [box_one addBox:HorizontalBox :MiddlePos :0 :bname];

		lbl = [CocoaLabel createT:name];
		[hbox addControl:lbl width:120];
		[tab_view addSubview:lbl];

		txtLPTHost[i] = [CocoaTextField createT:config.printer_server_host[i] action:nil];
		[hbox addControl:txtLPTHost[i] width:140];
		[tab_view addSubview:txtLPTHost[i]];

		lbl = [CocoaLabel createI:CMsg::_Port align:NSRightTextAlignment];
		[hbox addControl:lbl];
		[tab_view addSubview:lbl];

		txtLPTPort[i] = [CocoaTextField createN:config.printer_server_port[i] action:nil align:NSRightTextAlignment];
		[hbox addControl:txtLPTPort[i] width:80];
		[tab_view addSubview:txtLPTPort[i]];

		lbl = [CocoaLabel createI:CMsg::_Print_delay align:NSRightTextAlignment];
		[hbox addControl:lbl];
		[tab_view addSubview:lbl];

		char delay[128];
		sprintf(delay, "%.1f", config.printer_delay[i]);
		txtLPTDelay[i] = [CocoaTextField createT:delay action:nil align:NSRightTextAlignment];
		[hbox addControl:txtLPTDelay[i] width:80];
		[tab_view addSubview:txtLPTDelay[i]];

		lbl = [CocoaLabel createI:CMsg::msec align:NSRightTextAlignment];
		[hbox addControl:lbl];
		[tab_view addSubview:lbl];
	}
	for(i=0; i<MAX_COMM; i++) {
		char name[128];
		sprintf(name, CMSG(COMVDIGIT_Hostname), i);

		_stprintf(bname, _T("COM%d"), i);
		hbox = [box_one addBox:HorizontalBox :0 :0 :bname];

		lbl = [CocoaLabel createT:name];
		[hbox addControl:lbl width:120];
		[tab_view addSubview:lbl];

		txtCOMHost[i] = [CocoaTextField createT:config.comm_server_host[i] action:nil];
		[hbox addControl:txtCOMHost[i] width:140];
		[tab_view addSubview:txtCOMHost[i]];

		lbl = [CocoaLabel createI:CMsg::_Port align:NSRightTextAlignment];
		[hbox addControl:lbl];
		[tab_view addSubview:lbl];

		txtCOMPort[i] = [CocoaTextField createN:config.comm_server_port[i] action:nil align:NSRightTextAlignment];
		[hbox addControl:txtCOMPort[i] width:80];
		[tab_view addSubview:txtCOMPort[i]];

		popCOMDipswitch[i] = [CocoaPopUpButton createI:LABELS::comm_baud action:nil selidx:(config.comm_dipswitch[i]-1)];
		[hbox addControl:popCOMDipswitch[i]];
		[tab_view addSubview:popCOMDipswitch[i]];
	}
#ifdef USE_DEBUGGER
	hbox = [box_one addBox:HorizontalBox :0 :0 :_T("Debugger")];

	lbl = [CocoaLabel createI:CMsg::Connectable_host_to_Debugger];
	[hbox addControl:lbl];
	[tab_view addSubview:lbl];

	txtDbgrHost = [CocoaTextField createT:config.debugger_server_host action:nil];
	[hbox addControl:txtDbgrHost width:140];
	[tab_view addSubview:txtDbgrHost];

	lbl = [CocoaLabel createI:CMsg::_Port align:NSRightTextAlignment];
	[hbox addControl:lbl];
	[tab_view addSubview:lbl];

	txtDbgrPort = [CocoaTextField createN:config.debugger_server_port action:nil align:NSRightTextAlignment];
	[hbox addControl:txtDbgrPort width:80];
	[tab_view addSubview:txtDbgrPort];
#endif
	// uart
	int uart_baud_index = 0;
	for(int i=0; LABELS::comm_uart_baudrate[i] != NULL; i++) {
		if (config.comm_uart_baudrate == (int)strtol(LABELS::comm_uart_baudrate[i], NULL, 10)) {
			uart_baud_index = i;
			break;
		}
	}
	bbox = [box_one addBox:BoxViewBox :0 :COCOA_DEFAULT_MARGIN :_T("UART")];
	box = [CocoaBox createI:CMsg::Settings_of_serial_ports_on_host];
	[bbox addControl:box :260 :1];
	box_view = [box contentView];

	vbox = [bbox addBox:VerticalBox :0 :0 :_T("UARTV")];
	hbox = [vbox addBox:HorizontalBox :0 :0 :_T("BaudRate")];
	lbl = [CocoaLabel createI:CMsg::Baud_Rate];
	[hbox addControl:lbl width:120];
	[box_view addSubview:lbl];
	popCOMUartBaud = [CocoaPopUpButton createT:LABELS::comm_uart_baudrate action:nil selidx:uart_baud_index];
	[hbox addControl:popCOMUartBaud];
	[box_view addSubview:popCOMUartBaud];

	hbox = [vbox addBox:HorizontalBox :0 :0 :_T("DataBit")];
	lbl = [CocoaLabel createI:CMsg::Data_Bit];
	[hbox addControl:lbl width:120];
	[box_view addSubview:lbl];
	popCOMUartDataBit = [CocoaPopUpButton createT:LABELS::comm_uart_databit action:nil selidx:(config.comm_uart_databit - 7)];
	[hbox addControl:popCOMUartDataBit];
	[box_view addSubview:popCOMUartDataBit];

	hbox = [vbox addBox:HorizontalBox :0 :0 :_T("Parity")];
	lbl = [CocoaLabel createI:CMsg::Parity];
	[hbox addControl:lbl width:120];
	[box_view addSubview:lbl];
	popCOMUartParity = [CocoaPopUpButton createI:LABELS::comm_uart_parity action:nil selidx:config.comm_uart_parity];
	[hbox addControl:popCOMUartParity];
	[box_view addSubview:popCOMUartParity];

	hbox = [vbox addBox:HorizontalBox :0 :0 :_T("StopBit")];
	lbl = [CocoaLabel createI:CMsg::Stop_Bit];
	[hbox addControl:lbl width:120];
	[box_view addSubview:lbl];
	popCOMUartStopBit = [CocoaPopUpButton createT:LABELS::comm_uart_stopbit action:nil selidx:(config.comm_uart_stopbit - 1)];
	[hbox addControl:popCOMUartStopBit];
	[box_view addSubview:popCOMUartStopBit];

	hbox = [vbox addBox:HorizontalBox :0 :0 :_T("FlowCtrl")];
	lbl = [CocoaLabel createI:CMsg::Flow_Control];
	[hbox addControl:lbl width:120];
	[box_view addSubview:lbl];
	popCOMUartFlowCtrl = [CocoaPopUpButton createI:LABELS::comm_uart_flowctrl action:nil selidx:config.comm_uart_flowctrl];
	[hbox addControl:popCOMUartFlowCtrl];
	[box_view addSubview:popCOMUartFlowCtrl];

	lbl = [CocoaLabel createI:CMsg::Need_re_connect_to_serial_port_when_modified_this];
	[vbox addControl:lbl];
	[box_view addSubview:lbl];

	[tab_view addSubview:box];


	// CPU, Memory tab
	tab = [tabView tabViewItemAtIndex:4];
	tab_view = (CocoaView *)[tab view];

	box_one = [box_tab addBox:VerticalBox :0 :0 :_T("CPU")];

	hbox = [box_one addBox:HorizontalBox :MiddlePos :0 :_T("ROMPath")];

	lbl = [CocoaLabel createI:CMsg::ROM_Path_ASTERISK];
	[hbox addControl:lbl];
	[tab_view addSubview:lbl];

	txtROMPath = [CocoaTextField createT:config.rom_path action:nil];
	[hbox addControl:txtROMPath width:380];
	[tab_view addSubview:txtROMPath];

	btn = [CocoaButton createI:CMsg::Folder_ action:@selector(showFolderPanel:)];
	[btn setRelatedObject:txtROMPath];
	[hbox addControl:btn];
	[tab_view addSubview:btn];

	hbox = [box_one addBox:HorizontalBox :MiddlePos :0 :_T("MEMSize")];

#if defined(_MBS1)
	lbl = [CocoaLabel createI:CMsg::Extended_RAM_ASTERISK];
	[hbox addControl:lbl];
	[tab_view addSubview:lbl];

	popExtRam = [CocoaPopUpButton createI:LABELS::exram_size action:nil selidx:emu->get_parami(VM::ParamExMemNum)];
	[hbox addControl:popExtRam width:120];
	[tab_view addSubview:popExtRam];

	if (0 <= config.exram_size_num && config.exram_size_num <= 4) {
		char str[128];
		strcpy(str, CMSG(LB_Now_SP));
		strcat(str, gMessages.Get(LABELS::exram_size[config.exram_size_num]));
		strcat(str, ")");
		lbl = [CocoaLabel createT:str];
		[hbox addControl:lbl];
		[tab_view addSubview:lbl];
	}

	chkMemNoWait = [CocoaCheckBox createI:CMsg::No_wait_to_access_the_main_memory action:nil value:config.mem_nowait];
	[box_one addControl:chkMemNoWait];
	[tab_view addSubview:chkMemNoWait];
#else
	chkExMem = [CocoaCheckBox createI:CMsg::Use_Extended_Memory_64KB action:nil value:(config.exram_size_num == 1)];
	[hbox addControl:chkExMem];
	[tab_view addSubview:chkExMem];
#endif

	hbox = [box_one addBox:HorizontalBox :MiddlePos :0 :_T("UndefCPU")];

	chkUndefOp = [CocoaCheckBox createI:CMsg::Show_message_when_the_CPU_fetches_undefined_opcode action:nil value:FLG_SHOWMSG_UNDEFOP];
	[hbox addControl:chkUndefOp];
	[tab_view addSubview:chkUndefOp];

#if defined(_MBS1)
# if defined(USE_Z80B_CARD)
	// z80b card
	hbox = [box_one addBox:HorizontalBox :MiddlePos :0 :_T("Z80BCard")];
	lbl = [CocoaLabel createI:CMsg::Connect_interrupt_signal_of_Z80B_Card_to_ASTERISK];
	[hbox addControl:lbl];
	[tab_view addSubview:lbl];
	popZ80BCardIrq = [CocoaPopUpButton createI:LABELS::z80bcard_irq action:nil selidx:config.z80b_card_out_irq];
	[hbox addControl:popZ80BCardIrq];
	[tab_view addSubview:popZ80BCardIrq];
# elif defined(USE_MPC_68008)
	hbox = [box_one addBox:HorizontalBox :MiddlePos :0 :_T("AddrErr")];

	chkAddrErr = [CocoaCheckBox createI:CMsg::Show_message_when_the_address_error_occured_in_MC68008 action:nil value:FLG_SHOWMSG_ADDRERR];
	[hbox addControl:chkAddrErr];
	[tab_view addSubview:chkAddrErr];
# endif
#endif

	lbl = [CocoaLabel createI:CMsg::Need_restart_program_or_PowerOn];
	[box_one addControl:lbl];
	[tab_view addSubview:lbl];

#if defined(_MBS1)
	// Sound tab
	tab = [tabView tabViewItemAtIndex:5];
	tab_view = (CocoaView *)[tab view];

	box_one = [box_tab addBox:VerticalBox :0 :0 :_T("Sound")];

	//
	bbox = [box_one addBox:BoxViewBox :0 :COCOA_DEFAULT_MARGIN :_T("FmOPN")];
	box = [CocoaBox createI:CMsg::FM_Synthesis_Card_ASTERISK];
	[bbox addControl:box :260 :1];
	box_view = [box contentView];
	vbox = [bbox addBox:VerticalBox :0 :0 :_T("FmOPNV")];

	chkFmOpnEn = [CocoaCheckBox createI:CMsg::Enable action:@selector(selectFmOpn:) value:IOPORT_USE_FMOPN];
	[vbox addControl:chkFmOpnEn];
	[box_view addSubview:chkFmOpnEn];

	lbl = [CocoaLabel createI:CMsg::IO_ports_are_FF1E_FF1F_FF16_FF17];
	[vbox addControl:lbl];
	[box_view addSubview:lbl];

//	hbox = [vbox addBox:HorizontalBox :MiddlePos :0 :_T("FmOPNClk")];

//	lbl = [CocoaLabel createI:CMsg::Clock];
//	[hbox addControl:lbl];
//	[box_view addSubview:lbl];

//	popFmOpnClk = [CocoaPopUpButton createT:LABELS::fmopn_clock action:nil selidx:config.opn_clock];
//	[hbox addControl:popFmOpnClk];
//	[box_view addSubview:popFmOpnClk];

//	lbl = [CocoaLabel createT:"Hz"];
//	[hbox addControl:lbl];
//	[box_view addSubview:lbl];

	hbox = [vbox addBox:HorizontalBox :MiddlePos :0 :_T("FmOPNChip")];

	lbl = [CocoaLabel createI:CMsg::Sound_chip];
	[hbox addControl:lbl];
	[box_view addSubview:lbl];

	popFmOpnChip = [CocoaPopUpButton createI:LABELS::type_of_soundcard action:nil selidx:emu->get_parami(VM::ParamChipTypeOnFmOpn) appendnum:config.type_of_fmopn appendstr:CMsg::LB_Now_RB];
	[hbox addControl:popFmOpnChip];
	[box_view addSubview:popFmOpnChip];

	[tab_view addSubview:box];

	//
	bbox = [box_one addBox:BoxViewBox :0 :COCOA_DEFAULT_MARGIN :_T("ExPSG")];
	box = [CocoaBox createI:CMsg::Extended_PSG_Port_ASTERISK];
	[bbox addControl:box :260 :1];
	box_view = [box contentView];
	vbox = [bbox addBox:VerticalBox :0 :0 :_T("ExPSGV")];

	chkExPsgEn = [CocoaCheckBox createI:CMsg::Enable action:@selector(selectExPsg:) value:IOPORT_USE_EXPSG];
	[vbox addControl:chkExPsgEn];
	[box_view addSubview:chkExPsgEn];

	lbl = [CocoaLabel createI:CMsg::IO_ports_are_FFE6_FFE7_FFEE_FFEF];
	[vbox addControl:lbl];
	[box_view addSubview:lbl];

	hbox = [vbox addBox:HorizontalBox :MiddlePos :0 :_T("ExPSGChip")];

	lbl = [CocoaLabel createI:CMsg::Sound_chip];
	[hbox addControl:lbl];
	[box_view addSubview:lbl];

	popExPsgChip = [CocoaPopUpButton createI:LABELS::type_of_soundcard action:nil selidx:emu->get_parami(VM::ParamChipTypeOnExPsg) appendnum:config.type_of_expsg appendstr:CMsg::LB_Now_RB];
	[hbox addControl:popExPsgChip];
	[box_view addSubview:popExPsgChip];

	[tab_view addSubview:box];

	//
	hbox = [box_one addBox:HorizontalBox :MiddlePos :0 :_T("IntrSnd")];

	lbl = [CocoaLabel createI:CMsg::Connect_interrupt_signal_of_FM_synthesis_to_ASTERISK_ASTERISK];
	[hbox addControl:lbl];
	[tab_view addSubview:lbl];

	popFmOpnIrq = [CocoaPopUpButton createI:LABELS::fmopn_irq action:nil selidx:config.opn_irq];
	[hbox addControl:popFmOpnIrq];
	[tab_view addSubview:popFmOpnIrq];

	lbl = [CocoaLabel createI:CMsg::Need_restart_program_or_PowerOn];
	[box_one addControl:lbl];
	[tab_view addSubview:lbl];

	lbl = [CocoaLabel createI:CMsg::This_is_the_common_setting_both_FM_synthesis_card_and_extended_PSG_port];
	[box_one addControl:lbl];
	[tab_view addSubview:lbl];
#endif

	// button

	hbox = [box_all addBox:HorizontalBox :RightPos | TopPos :0 :_T("BTN")];

	CocoaButton *btnCancel = [CocoaButton createI:CMsg::Cancel action:@selector(dialogCancel:)];
	[hbox addControl:btnCancel width:120];
	[view addSubview:btnCancel];

	CocoaButton *btnOK = [CocoaButton createI:CMsg::OK action:@selector(dialogOk:)];
	[hbox addControl:btnOK width:120];
	[view addSubview:btnOK];


	[box_all realize:self];

//	[self selectFddType:radFddType];

	return self;
}

- (NSInteger)runModal
{
	return [NSApp runModalForWindow:self];
}

- (void)close
{
	[NSApp stopModalWithCode:NSCancelButton];
	[super close];
}

- (void)dialogOk:(id)sender
{
	int i;
	int valuel;
	int val;

	// set data
	config.use_power_off = ([chkPowerOff state] == NSOnState);
#if defined(_MBS1)
	emu->set_parami(VM::ParamSysMode,([radSysMode[0] state] == NSOnState ? 1 : 0) | (config.sys_mode & ~1));
#endif
	if ([chkDipswitch state] == NSOnState) {
		config.dipswitch |= 4;
	} else {
		config.dipswitch &= ~4;
	}
	for(i=0; i<4; i++) {
		if ([radFddType[i] state] == NSOnState) emu->set_parami(VM::ParamFddType,i);
	}
	for(i=0;i<MAX_DRIVE;i++) {
		config.mount_fdd = (([chkFddMount[i] state] == NSOnState) ? config.mount_fdd | (1 << i) : config.mount_fdd & ~(1 << i));
	}
	config.delay_fdd = ([chkDelayFd1 state] == NSOnState ? MSK_DELAY_FDSEARCH : 0)
					 | ([chkDelayFd2 state] == NSOnState ? MSK_DELAY_FDSEEK : 0);
	config.check_fdmedia = ([chkFdDensity state] == NSOnState ? 0 : MSK_CHECK_FDDENSITY)
					 | ([chkFdMedia state] == NSOnState ? 0 : MSK_CHECK_FDMEDIA);

	for(i=IOPORT_STARTNUM;i<IOPORT_NUMS;i++) {
		if ((1 << i) & IOPORT_MSK_ALL) {
			val = emu->get_parami(VM::ParamIOPort);
			if ([chkIOPort[i] state] == NSOnState) {
				val |= (1 << i);
			} else {
				val &= ~(1 << i);
			}
			emu->set_parami(VM::ParamIOPort, val);
		}
	}
#ifndef USE_IOPORT_FDD
	int fdd_type = emu->get_parami(VM::ParamFddType);
	val = emu->get_parami(VM::ParamIOPort);
	val &= ~IOPORT_MSK_FDDALL;
	val |= (fdd_type == FDD_TYPE_3FDD ? IOPORT_MSK_3FDD
		 : (fdd_type == FDD_TYPE_5FDD
		 || fdd_type == FDD_TYPE_58FDD ? IOPORT_MSK_5FDD : 0));
	emu->set_parami(VM::ParamIOPort, val);
#endif

//	emu->set_parami(VM::ParamUseOpenGL, (int)[popUseOpenGL indexOfSelectedItem]);
	config.use_opengl = (int)[popUseOpenGL indexOfSelectedItem];
	config.gl_filter_type = [popGLFilter indexOfSelectedItem];

	config.led_pos = [popLEDPosition indexOfSelectedItem];
	config.capture_type = [popCaptureType indexOfSelectedItem];

#if defined(_MBS1)
	config.curdisp_skew = [popCurdispSkew indexOfSelectedItem] - 2;
	config.disptmg_skew = [popDisptmgSkew indexOfSelectedItem] - 2;
#else
	config.curdisp_skew = [popCurdispSkew indexOfSelectedItem];
	config.disptmg_skew = [popDisptmgSkew indexOfSelectedItem];
#endif

	config.snapshot_path.Set([[txtSnapPath stringValue] UTF8String]);
	config.font_path.Set([[txtFontPath stringValue] UTF8String]);

	config.msgboard_msg_fontname.Set([[txtMsgFontName stringValue] UTF8String]);
	config.msgboard_info_fontname.Set([[txtInfoFontName stringValue] UTF8String]);
	valuel = [txtMsgFontSize intValue];
	if (1 <= valuel && valuel <= 60) {
		config.msgboard_msg_fontsize = (int)valuel;
	}
	valuel = [txtInfoFontSize intValue];
	if (1 <= valuel && valuel <= 60) {
		config.msgboard_info_fontsize = (int)valuel;
	}

	// language
	valuel = (int)[popLanguage indexOfSelectedItem];
	clocale->ChooseLocaleName(lang_list, valuel, config.language);

	config.wav_reverse = ([chkReverseWave state] == NSOnState);
	config.wav_half = ([chkHalfWave state] == NSOnState);
//	config.wav_correct = ([chkCorrectWave state] == NSOnState);
	config.wav_correct = ([radCorrect selectedColumn] == 0);
	config.wav_correct_type = [radCorrect selectedColumn];
	config.wav_correct_type = (config.wav_correct_type > 0 ? config.wav_correct_type - 1 : 0);
	valuel = [txtCorrectAmp[0] intValue];
	if (valuel >= 100 && valuel <= 5000) {
		config.wav_correct_amp[0] = (int)valuel;
	}
	valuel = [txtCorrectAmp[1] intValue];
	if (valuel >= 100 && valuel <= 5000) {
		config.wav_correct_amp[1] = (int)valuel;
	}

	config.wav_sample_rate = [popSampleRate indexOfSelectedItem];
	config.wav_sample_bits = [popSampleBits indexOfSelectedItem];

	for(i=0;i<MAX_PRINTER;i++) {
		config.printer_server_host[i].Set([[txtLPTHost[i] stringValue] UTF8String]);
		valuel = [txtLPTPort[i] intValue];
		if (0 <= valuel && valuel <= 65535) {
			config.printer_server_port[i] = (int)valuel;
		}
		double valued = 0.0;
		valued = strtod([[txtLPTDelay[i] stringValue] UTF8String], NULL);
		if (valued < 0.1) valued = 0.1;
		if (valued > 1000.0) valued = 1000.0;
		valued = floor(valued * 10.0 + 0.5) / 10.0;
		config.printer_delay[i] = valued;
	}
	for(i=0;i<MAX_COMM;i++) {
		config.comm_server_host[i].Set([[txtCOMHost[i] stringValue] UTF8String]);
		valuel = [txtCOMPort[i] intValue];
		if (0 <= valuel && valuel <= 65535) {
			config.comm_server_port[i] = (int)valuel;
		}
		config.comm_dipswitch[i] = (int)[popCOMDipswitch[i] indexOfSelectedItem] + 1;
	}
#ifdef USE_DEBUGGER
	config.debugger_server_host.Set([[txtDbgrHost stringValue] UTF8String]);
	valuel = [txtDbgrPort intValue];
	if (0 <= valuel && valuel <= 65535) {
		config.debugger_server_port = (int)valuel;
	}
#endif
	int uart_baud_index = (int)[popCOMUartBaud indexOfSelectedItem];
	config.comm_uart_baudrate = (int)strtol(LABELS::comm_uart_baudrate[uart_baud_index], NULL, 10);
	config.comm_uart_databit = (int)[popCOMUartDataBit indexOfSelectedItem] + 7;
	config.comm_uart_parity = (int)[popCOMUartParity indexOfSelectedItem];
	config.comm_uart_stopbit = (int)[popCOMUartStopBit indexOfSelectedItem] + 1;
	config.comm_uart_flowctrl = (int)[popCOMUartFlowCtrl indexOfSelectedItem];

	config.rom_path.Set([[txtROMPath stringValue] UTF8String]);

	BIT_ONOFF(config.misc_flags, MSK_SHOWMSG_UNDEFOP, [chkUndefOp state] == NSOnState);

#if defined(_MBS1)
# if defined(USE_Z80B_CARD)
	config.z80b_card_out_irq = (int)[popZ80BCardIrq indexOfSelectedItem];
# elif defined(USE_MPC_68008)
	BIT_ONOFF(config.misc_flags, MSK_SHOWMSG_ADDRERR, [chkAddrErr state] == NSOnState);
# endif
	emu->set_parami(VM::ParamExMemNum, (int)[popExtRam indexOfSelectedItem]);

	config.mem_nowait = ([chkMemNoWait state] == NSOnState);

//	config.opn_clock = (int)[popFmOpnClk indexOfSelectedItem];
	config.opn_irq = (int)[popFmOpnIrq indexOfSelectedItem];
	emu->set_parami(VM::ParamChipTypeOnFmOpn, (int)[popFmOpnChip indexOfSelectedItem]);
	emu->set_parami(VM::ParamChipTypeOnExPsg, (int)[popExPsgChip indexOfSelectedItem]);
#else
	config.exram_size_num = ([chkExMem state] == NSOnState) ? 1 : 0;
#endif

	// set message font
	MsgBoard *msgboard = emu->get_msgboard();
	if (msgboard) {
		msgboard->SetFont();
	}

	gui->ChangeLedBox((int)[popLEDShow indexOfSelectedItem]);
	gui->ChangeLedBoxPosition(config.led_pos);
	config.save();
	emu->change_opengl_attr();

	emu->update_config();

    // OK button is pushed
	[NSApp stopModalWithCode:NSOKButton];
	[super close];
}

- (void)dialogCancel:(id)sender
{
    // Cancel button is pushed
	[self close];
}

- (void)showFolderPanel:(id)sender
{
	NSInteger result;

	CocoaTextField *text = [sender relatedObject];
	if (text == nil) return;

	NSOpenPanel *panel = [NSOpenPanel openPanel];

	// cannot select file
	[panel setCanChooseFiles:NO];
	[panel setCanChooseDirectories:YES];

	// set current folder
	[panel setDirectoryURL:[NSURL fileURLWithPath:[text stringValue]]];

	// Display modal dialog
	result = [panel runModal];

	if(result == NSOKButton) {
		// get file path (use NSURL)
		NSURL *filePath = [panel URL];

		[text setStringValue:[filePath path]];
	}
}

- (void)showFilePanel:(id)sender
{
	NSInteger result;

	CocoaTextField *text = [sender relatedObject];
	if (text == nil) return;

	NSOpenPanel *panel = [NSOpenPanel openPanel];

	// can select file
	[panel setCanChooseFiles:YES];
	[panel setCanChooseDirectories:NO];
	// set font folder
	NSURL *url = nil;
	BOOL exist = false;
	url = [NSURL fileURLWithPath:@"/System/Library/Fonts/"];
	exist = [url checkResourceIsReachableAndReturnError:nil];
	if (!exist) {
		url = [NSURL fileURLWithPath:@"/Library/Fonts/"];
	}
	[panel setDirectoryURL:url];

	// Display modal dialog
	result = [panel runModal];

	if(result == NSOKButton) {
		// get file path (use NSURL)
		NSURL *filePath = [panel URL];
		NSArray *arr = [filePath pathComponents];
		[text setStringValue:[arr lastObject]];
	}
}

- (void)showFontPanel:(id)sender
{
	NSInteger result;

	CocoaObjectStructure *obj = [sender relatedObject];
	if (obj == nil) return;
	CocoaTextField *fname = [obj obj1];
	if (fname == nil) return;
	CocoaTextField *fsize = [obj obj2];
	if (fsize == nil) return;

	// set font
	NSFont *font = [NSFont fontWithName:[fname stringValue] size:[fsize integerValue]];

	CocoaFontPanel *panel = [[CocoaFontPanel alloc] init];

	[panel setPanelFont:font isMultiple:NO];

	// Display modal dialog
	result = [panel runModal];

	if(result == NSOKButton) {
	}
//	[[NSFontManager sharedFontManager]orderFrontFontPanel:self];
}

- (void)changeFont:(id)sender
{

}

#if defined(_MBS1)
- (void)selectSysMode:(CocoaRadioButton *)sender
{
	int idx = [sender index];
	for(int i=0; i<2; i++) {
		[radSysMode[i] setState:(i == idx ? NSOnState : NSOffState)];
	}
}
- (void)selectFmOpn:(CocoaCheckBox *)sender
{
	CocoaCheckBox *receiver;
	if (sender == chkFmOpnEn) {
		receiver = chkIOPort[IOPORT_POS_FMOPN];
	} else {
		receiver = chkFmOpnEn;
	}
	[receiver setState:[sender state]];
}
- (void)selectExPsg:(CocoaCheckBox *)sender
{
	CocoaCheckBox *receiver;
	if (sender == chkExPsgEn) {
		receiver = chkIOPort[IOPORT_POS_EXPSG];
	} else {
		receiver = chkExPsgEn;
	}
	[receiver setState:[sender state]];
}
#endif

- (void)selectFddType:(CocoaRadioButton *)sender
{
#ifdef USE_IOPORT_FDD
	int num = (int)[sender selectedRow];
	switch(num) {
		case 0:
			// No FDD
			[chkIOPort[0] setState:NSOffState];
			[chkIOPort[1] setState:NSOffState];
			[chkIOPort[0] setEnabled:YES];
			[chkIOPort[1] setEnabled:YES];
			break;
		case 1:
			// 3inch FDD
			[chkIOPort[0] setState:NSOffState];
			[chkIOPort[1] setState:NSOnState];
			[chkIOPort[0] setEnabled:NO];
			[chkIOPort[1] setEnabled:NO];
			break;
		case 2:
		case 3:
			// 5inch FDD
			[chkIOPort[0] setState:NSOnState];
			[chkIOPort[1] setState:NSOffState];
			[chkIOPort[0] setEnabled:NO];
			[chkIOPort[1] setEnabled:NO];
			break;
		default:
			break;
	}
#endif
}

- (void)selectIO:(CocoaCheckBox *)sender
{
	int num = [sender index];
	switch(num) {
#ifdef USE_IOPORT_FDD
		case IOPORT_POS_5FDD:
			// 5inch FDD
			if ([sender state] == NSOnState) {
				[chkIOPort[IOPORT_POS_3FDD] setState:NSOffState];
			}
			break;
		case IOPORT_POS_3FDD:
			// 3inch FDD
			if ([sender state] == NSOnState) {
				[chkIOPort[IOPORT_POS_5FDD] setState:NSOffState];
			}
			break;
#endif
		case IOPORT_POS_PSG9:
			// 9psg
			if ([sender state] == NSOnState) {
				[chkIOPort[IOPORT_POS_KANJI] setState:NSOffState];
#if defined(_MBS1)
				[chkIOPort[IOPORT_POS_CM01] setState:NSOffState];
#endif
			}
			break;
		case IOPORT_POS_KANJI:
			// Kanji
			if ([sender state] == NSOnState) {
				[chkIOPort[IOPORT_POS_PSG9] setState:NSOffState];
			}
			break;
#if defined(_MBS1)
		case IOPORT_POS_CM01:
			// Comm Card
			if ([sender state] == NSOnState) {
				[chkIOPort[IOPORT_POS_PSG9] setState:NSOffState];
				[chkIOPort[IOPORT_POS_KANJI] setState:NSOnState];
			}
			break;
		case IOPORT_POS_FMOPN:
			// Fm OPN
			[chkFmOpnEn setState:[chkIOPort[num] state]];
			break;
		case IOPORT_POS_EXPSG:
			// Ex PSG
			[chkExPsgEn setState:[chkIOPort[num] state]];
			break;
#endif
		default:
			break;
	}
}

- (void)selectCorrect:(CocoaCheckBox *)sender
{
//	if ([sender state] == NSOnState) {
//		[radCorrect setEnabled:YES];
//	} else {
//		[radCorrect setEnabled:NO];
//	}
}

@end
