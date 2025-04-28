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
#import "../../utility.h"

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

	CocoaLayout *box_all = [CocoaLayout create:view :VerticalBox :0 :COCOA_DEFAULT_MARGIN :_T("box_all")];
	CocoaLayout *box_tab = [box_all addBox:TabViewBox :0 :COCOA_DEFAULT_MARGIN :_T("box_tab")];

	CocoaTabView *tabView = [CocoaTabView createI:box_tab tabs:LABELS::tabs width:300 height:200];

	NSTabViewItem *tab;
	CocoaView *tab_view;
//	CocoaBox *grp;
//	CocoaLabel *lbl;
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

	// ----------------------------------------
	// Mode tab
	// ----------------------------------------
	tab = [tabView tabViewItemAtIndex:0];
	tab_view = (CocoaView *)[tab view];
	[box_tab setContentView:tab_view];

	box_one = [box_tab addBox:VerticalBox :0 :0 :_T("mode")];
	sbox = [box_one addBox:HorizontalBox :0 :0 :_T("modeS")];
	lbox = [sbox addBox:VerticalBox :0 :0 :_T("modeL")];

#if defined(_MBS1)
	//
	bbox = [lbox addBox:BoxViewBox :0 :COCOA_DEFAULT_MARGIN :_T("SysModeB")];

	[CocoaBox createI:bbox :CMsg::System_Mode_ASTERISK :260 :1];

	vbox = [bbox addBox:VerticalBox :0 :0 :_T("SysModA")];

	for(i = 0; i < 2; i++) {
		hbox = [vbox addBox:HorizontalBox :0 :0 :_T("SysModH")];

		[CocoaLabel createT:hbox title:(pConfig->sys_mode & 1) == (1 - i) ? ">" : " " width:16];

		radSysMode[i] = [CocoaRadioButton createI:hbox title:LABELS::sys_mode[i] index:i action:@selector(selectSysMode:) value:(emu->get_parami(VM::ParamSysMode) & 1) == (1 - i)];
	}
	hbox = [vbox addBox:HorizontalBox :0 :0 :_T("Newon7H")];
	[hbox addSpace:32 :20];
	chkDipswitch = [CocoaCheckBox createI:hbox title:CMsg::NEWON7 action:nil value:(pConfig->dipswitch & 4)];
#else
	//
	bbox = [lbox addBox:BoxViewBox :0 :COCOA_DEFAULT_MARGIN :_T("DipSwB")];

	[CocoaBox createI:bbox :CMsg::DIP_Switch_ASTERISK :260 :1];

	vbox = [bbox addBox:VerticalBox :0 :0 :_T("DipSwA")];
	hbox = [vbox addBox:HorizontalBox :0 :0 :_T("DipSwH")];

	[CocoaLabel createT:hbox title:(pConfig->dipswitch & 4) ? ">" : " " width:16];
	chkDipswitch = [CocoaCheckBox createI:hbox title:CMsg::MODE_Switch action:nil value:(pConfig->dipswitch & 4)];
#endif

	//
	bbox = [lbox addBox:BoxViewBox :0 :COCOA_DEFAULT_MARGIN :_T("FddTypeB")];

	[CocoaBox createI:bbox :CMsg::FDD_Type_ASTERISK :260 :1];

	vbox = [bbox addBox:VerticalBox :0 :0 :_T("FddTypeA")];

	for(i = 0; i < 4; i++) {
		hbox = [vbox addBox:HorizontalBox :0 :0 :_T("FddTypeH")];

		[CocoaLabel createT:hbox title:(pConfig->fdd_type == i) ? ">" : " " width:16];
		radFddType[i] = [CocoaRadioButton createI:hbox title:LABELS::fdd_type[i] index:i action:@selector(selectFddType:) value:emu->get_parami(VM::ParamFddType) == i];
	}

	chkPowerOff = [CocoaCheckBox createI:lbox title:CMsg::Enable_the_state_of_power_off action:nil value:pConfig->use_power_off];

	//
	rbox = [sbox addBox:VerticalBox :0 :0 :_T("modeR")];

	bbox = [rbox addBox:BoxViewBox :0 :COCOA_DEFAULT_MARGIN :_T("IOBOX")];

	[CocoaBox createI:bbox :CMsg::I_O_Port_Address_ASTERISK :320 :1];

	vbox = [bbox addBox:VerticalBox :0 :0 :_T("IOVER")];
	for(i=0; LABELS::io_port[i] != CMsg::End; i++) {
		int pos = LABELS::io_port_pos[i];
		if ((1 << pos) & IOPORT_MSK_ALL) {
			UTILITY::stprintf(bname, sizeof(bname)/sizeof(bname[0]), _T("IO%dH"), pos);
			hbox = [vbox addBox:HorizontalBox :0 :0 :bname];
			[CocoaLabel createT:hbox title:(pConfig->io_port & (1 << pos)) ? ">" : " " width:16];
			chkIOPort[pos] = [CocoaCheckBox createI:hbox title:LABELS::io_port[i] index:pos action:@selector(selectIO:) value:(emu->get_parami(VM::ParamIOPort) & (1 << pos))];
			[self selectIO:chkIOPort[pos]];
		}
	}

	//
	[CocoaLabel createI:box_one title:CMsg::Need_restart_program_or_PowerOn];

	// ----------------------------------------
	// Screen tab
	// ----------------------------------------
	tab = [tabView tabViewItemAtIndex:1];
	tab_view = (CocoaView *)[tab view];
	[box_tab setContentView:tab_view];

	box_one = [box_tab addBox:VerticalBox :0 :0 :_T("Scrn")];
	sbox = [box_one addBox:HorizontalBox :0 :0 :_T("ScrnS")];
	lbox = [sbox addBox:VerticalBox :0 :0 :_T("ScrnL")];

	// OpenGL
	bbox = [lbox addBox:BoxViewBox :0 :COCOA_DEFAULT_MARGIN :_T("OpenGLB")];
	[CocoaBox createI:bbox :CMsg::Drawing :260 :1];

	vbox = [bbox addBox:VerticalBox :0 :0 :_T("OpenGLV")];

	hbox = [vbox addBox:HorizontalBox :MiddlePos :0 :_T("UseGLH")];
	[CocoaLabel createI:hbox title:CMsg::Method_ASTERISK];
	popUseOpenGL = [CocoaPopUpButton createI:hbox items:LABELS::opengl_use action:nil selidx:pConfig->use_opengl];

	hbox = [vbox addBox:HorizontalBox :MiddlePos :0 :_T("GLFilH")];
	[CocoaLabel createI:hbox title:CMsg::Filter_Type];
	popGLFilter = [CocoaPopUpButton createI:hbox items:LABELS::opengl_filter action:nil selidx:pConfig->gl_filter_type];

	// CRTC skew
	rbox = [sbox addBox:VerticalBox :0 :0 :_T("CRTCR")];

	bbox = [rbox addBox:BoxViewBox :0 :COCOA_DEFAULT_MARGIN :_T("CRTCB")];
	[CocoaBox createI:bbox :CMsg::CRTC :260 :1];

	vbox = [bbox addBox:VerticalBox :0 :0 :_T("CRTCV")];
	hbox = [vbox addBox:HorizontalBox :MiddlePos :0 :_T("DISPTMG")];

	[CocoaLabel createI:hbox title:CMsg::Disptmg_Skew];

#if defined(_MBS1)
	popDisptmgSkew = [CocoaPopUpButton createT:hbox items:LABELS::disp_skew action:nil selidx:(pConfig->disptmg_skew + 2)];
#else
	popDisptmgSkew = [CocoaPopUpButton createT:hbox items:LABELS::disp_skew action:nil selidx:pConfig->disptmg_skew];
#endif

	hbox = [vbox addBox:HorizontalBox :MiddlePos :0 :_T("CURDISP")];
#if defined(_MBS1)
	[CocoaLabel createI:hbox title:CMsg::Curdisp_Skew_L3];
#else
	[CocoaLabel createI:hbox title:CMsg::Curdisp_Skew];
#endif

#if defined(_MBS1)
	popCurdispSkew = [CocoaPopUpButton createT:hbox items:LABELS::disp_skew action:nil selidx:(pConfig->curdisp_skew + 2)];
#else
	popCurdispSkew = [CocoaPopUpButton createT:hbox items:LABELS::disp_skew action:nil selidx:pConfig->curdisp_skew];
#endif

	// LED
	hbox = [box_one addBox:HorizontalBox :MiddlePos :0 :_T("LEDH")];
	[CocoaLabel createI:hbox title:CMsg::LED];
	int led_show = gui->GetLedBoxPhase(-1);
	popLEDShow = [CocoaPopUpButton createI:hbox items:LABELS::led_show action:nil selidx:led_show];

	hbox = [box_one addBox:HorizontalBox :MiddlePos :0 :_T("LEDPOSH")];
	[CocoaLabel createI:hbox title:CMsg::Position];
	popLEDPosition = [CocoaPopUpButton createI:hbox items:LABELS::led_pos action:nil selidx:pConfig->led_pos];

	// Capture
	hbox = [box_one addBox:HorizontalBox :MiddlePos :0 :_T("CaptureH")];
	[CocoaLabel createI:hbox title:CMsg::Capture_Type];
	popCaptureType = [CocoaPopUpButton createT:hbox items:LABELS::capture_fmt action:nil selidx:pConfig->capture_type];

	// Snapshot path
	hbox = [box_one addBox:HorizontalBox :MiddlePos :0 :_T("SnapPath")];
	[CocoaLabel createI:hbox title:CMsg::Snapshot_Path];
	txtSnapPath = [CocoaTextField createT:hbox text:pConfig->snapshot_path.Get() action:nil width:360];
	btn = [CocoaButton createI:hbox title:CMsg::Folder_ action:@selector(showFolderPanel:)];
	[btn setRelatedObject:txtSnapPath];

	// Font path
	hbox = [box_one addBox:HorizontalBox :MiddlePos :0 :_T("FontPath")];
	[CocoaLabel createI:hbox title:CMsg::Font_Path];
	txtFontPath = [CocoaTextField createT:hbox text:pConfig->font_path.Get() action:nil width:360];
	btn = [CocoaButton createI:hbox title:CMsg::Folder_ action:@selector(showFolderPanel:)];
	[btn setRelatedObject:txtFontPath];

	// Message font
	hbox = [box_one addBox:HorizontalBox :MiddlePos :0 :_T("MsgFont")];
	[CocoaLabel createI:hbox title:CMsg::Message_Font];
	txtMsgFontName = [CocoaTextField createT:hbox text:pConfig->msgboard_msg_fontname.Get() action:nil width:200];

	[CocoaLabel createI:hbox title:CMsg::_Size];
	txtMsgFontSize = [CocoaTextField createN:hbox num:pConfig->msgboard_msg_fontsize action:nil width:100];
	btn = [CocoaButton createI:hbox title:CMsg::File_ action:@selector(showFilePanel:)];
	[btn setRelatedObject:txtMsgFontName];

	// Information font
	hbox = [box_one addBox:HorizontalBox :MiddlePos :0 :_T("InfoFont")];
	[CocoaLabel createI:hbox title:CMsg::Info_Font];
	txtInfoFontName = [CocoaTextField createT:hbox text:pConfig->msgboard_info_fontname.Get() action:nil width:200];

	[CocoaLabel createI:hbox title:CMsg::_Size];
	txtInfoFontSize = [CocoaTextField createN:hbox num:pConfig->msgboard_info_fontsize action:nil width:100];
	btn = [CocoaButton createI:hbox title:CMsg::File_ action:@selector(showFilePanel:)];
	[btn setRelatedObject:txtInfoFontName];

	// Language
	lang_list.Clear();
	clocale->GetLocaleNamesWithDefault(lang_list);

	int lang_selidx = clocale->SelectLocaleNameIndex(lang_list, pConfig->language);

	hbox = [box_one addBox:HorizontalBox :MiddlePos :0 :_T("Lang")];
	[CocoaLabel createI:hbox title:CMsg::Language_ASTERISK];
	popLanguage = [CocoaPopUpButton createL:hbox items:&lang_list action:nil selidx:lang_selidx];

	//
	hbox = [box_one addBox:HorizontalBox :MiddlePos :0 :_T("NeedRes")];
	[CocoaLabel createI:hbox title:CMsg::Need_restart_program];

	// ----------------------------------------
	// Tape, FDD tab
	// ----------------------------------------
	tab = [tabView tabViewItemAtIndex:2];
	tab_view = (CocoaView *)[tab view];
	[box_tab setContentView:tab_view];

	box_one = [box_tab addBox:VerticalBox :0 :0 :_T("Tape")];

#ifdef USE_DATAREC
	sbox = [box_one addBox:HorizontalBox :0 :0 :_T("TapeS")];
	lbox = [sbox addBox:VerticalBox :0 :0 :_T("TapeL")];

	// Load wave
	bbox = [lbox addBox:BoxViewBox :0 :COCOA_DEFAULT_MARGIN :_T("LoadWavB")];
	[CocoaBox createI:bbox :CMsg::Load_Wav_File_from_Tape :300 :1];

	vbox = [bbox addBox:VerticalBox :0 :0 :_T("LoadWavV")];

	hbox = [vbox addBox:HorizontalBox :MiddlePos :0 :_T("LoadWavH")];
	chkReverseWave = [CocoaCheckBox createI:hbox title:CMsg::Reverse_Wave action:nil value:pConfig->wav_reverse];
	chkHalfWave = [CocoaCheckBox createI:hbox title:CMsg::Half_Wave action:nil value:pConfig->wav_half];

	hbox = [vbox addBox:HorizontalBox :MiddlePos :0 :_T("CorrH")];
	[CocoaLabel createI:hbox title:CMsg::Correct];
	int corr_idx = pConfig->wav_correct ? pConfig->wav_correct_type + 1 : 0;
	radCorrect = [CocoaRadioGroup create:hbox width:260 cols:3 titleids:LABELS::correct action:nil selidx:corr_idx];

	hbox = [vbox addBox:HorizontalBox :MiddlePos :0 :_T("CorrAmpH")];
	for(i=0; i<2; i++) {
		[CocoaLabel createT:hbox title:i == 0 ? "1200Hz" : "2400Hz"];
		txtCorrectAmp[i] = [CocoaTextField createN:hbox num:pConfig->wav_correct_amp[i] action:nil align:NSTextAlignmentRight width:60];
	}

	// Save wave
	rbox = [sbox addBox:VerticalBox :0 :0 :_T("TapeR")];

	bbox = [rbox addBox:BoxViewBox :0 :COCOA_DEFAULT_MARGIN :_T("SaveWavB")];
	[CocoaBox createI:bbox :CMsg::Save_Wav_File_to_Tape :260 :1];

	vbox = [bbox addBox:VerticalBox :0 :0 :_T("SaveWavV")];

	hbox = [vbox addBox:HorizontalBox :MiddlePos :0 :_T("SaveRatH")];
	[CocoaLabel createI:hbox title:CMsg::Sample_Rate];
	popSampleRate = [CocoaPopUpButton createT:hbox items:LABELS::wav_sampling_rate action:nil selidx:pConfig->wav_sample_rate];

	hbox = [vbox addBox:HorizontalBox :MiddlePos :0 :_T("SaveBitH")];
	[CocoaLabel createI:hbox title:CMsg::Sample_Bits];
	popSampleBits = [CocoaPopUpButton createT:hbox items:LABELS::wav_sampling_bits action:nil selidx:pConfig->wav_sample_bits];
#endif

	// FDD
#ifdef USE_FD1
	bbox = [box_one addBox:BoxViewBox :0 :COCOA_DEFAULT_MARGIN :_T("Fdd2B")];
	[CocoaBox createI:bbox :CMsg::Floppy_Disk_Drive :320 :1];

	vbox = [bbox addBox:VerticalBox :0 :0 :_T("Fdd2V")];

	// mount fdd
	hbox = [vbox addBox:HorizontalBox :MiddlePos :0 :_T("MountH")];
	[CocoaLabel createI:hbox title:CMsg::When_start_up_mount_disk_at_];
	for(i=0; i<USE_FLOPPY_DISKS; i++) {
		char name[4];
		UTILITY::sprintf(name, sizeof(name), "%d ", i);
		chkFddMount[i] = [CocoaCheckBox createT:hbox title:name action:nil value:(pConfig->mount_fdd & (1 << i))];
	}

	chkDelayFd1 = [CocoaCheckBox createI:vbox title:CMsg::Ignore_delays_to_find_sector action:nil value:(FLG_DELAY_FDSEARCH != 0)];
	chkDelayFd2 = [CocoaCheckBox createI:vbox title:CMsg::Ignore_delays_to_seek_track action:nil value:(FLG_DELAY_FDSEEK != 0)];
	chkFdDensity = [CocoaCheckBox createI:vbox title:CMsg::Suppress_checking_for_density action:nil value:(FLG_CHECK_FDDENSITY == 0)];
	chkFdMedia = [CocoaCheckBox createI:vbox title:CMsg::Suppress_checking_for_media_type action:nil value:(FLG_CHECK_FDMEDIA == 0)];
	chkFdSavePlain = [CocoaCheckBox createI:vbox title:CMsg::Save_a_plain_disk_image_as_it_is action:nil value:(FLG_SAVE_FDPLAIN != 0)];
#endif

	// HDD
#ifdef USE_HD1
	bbox = [box_one addBox:BoxViewBox :0 :COCOA_DEFAULT_MARGIN :_T("Hdd2B")];
	[CocoaBox createI:bbox :CMsg::Hard_Disk_Drive :320 :1];

	vbox = [bbox addBox:VerticalBox :0 :0 :_T("Hdd2V")];

	// mount hdd
	hbox = [vbox addBox:HorizontalBox :MiddlePos :0 :_T("MountH")];
	[CocoaLabel createI:hbox title:CMsg::When_start_up_mount_disk_at_];
	for(i=0; i<USE_HARD_DISKS; i++) {
		char name[8];
		UTILITY::sprintf(name, sizeof(name), "%d ", i);
		chkHddMount[i] = [CocoaCheckBox createT:hbox title:name action:nil value:(pConfig->mount_hdd & (1 << i))];
	}

	chkDelayHd2 = [CocoaCheckBox createI:vbox title:CMsg::Ignore_delays_to_seek_track action:nil value:(FLG_DELAY_HDSEEK != 0)];
#endif

	// ----------------------------------------
	// Network tab
	// ----------------------------------------
	tab = [tabView tabViewItemAtIndex:3];
	tab_view = (CocoaView *)[tab view];
	[box_tab setContentView:tab_view];

	box_one = [box_tab addBox:VerticalBox :0 :0 :_T("Net")];

#ifdef MAX_PRINTER
	for(i=0; i<MAX_PRINTER; i++) {
		char name[128];
		UTILITY::sprintf(name, sizeof(name), CMSG(LPTVDIGIT_Hostname), i);

		UTILITY::stprintf(bname, sizeof(bname)/sizeof(bname[0]), _T("LPT%d"), i);
		hbox = [box_one addBox:HorizontalBox :MiddlePos :0 :bname];
		[CocoaLabel createT:hbox title:name width:120];
		txtLPTHost[i] = [CocoaTextField createT:hbox text:pConfig->printer_server_host[i].Get() action:nil width:140];

		[CocoaLabel createI:hbox title:CMsg::_Port align:NSTextAlignmentRight];
		txtLPTPort[i] = [CocoaTextField createN:hbox num:pConfig->printer_server_port[i] action:nil align:NSTextAlignmentRight width:80];

		[CocoaLabel createI:hbox title:CMsg::_Print_delay align:NSTextAlignmentRight];
		char delay[128];
		UTILITY::sprintf(delay, sizeof(delay), "%.1f", pConfig->printer_delay[i]);
		txtLPTDelay[i] = [CocoaTextField createT:hbox text:delay action:nil align:NSTextAlignmentRight width:80];

		[CocoaLabel createI:hbox title:CMsg::msec align:NSTextAlignmentRight];
	}
#endif
#ifdef MAX_COMM
	for(i=0; i<MAX_COMM; i++) {
		char name[128];
		UTILITY::sprintf(name, sizeof(name), CMSG(COMVDIGIT_Hostname), i);

		UTILITY::stprintf(bname, sizeof(bname)/sizeof(bname[0]), _T("COM%d"), i);
		hbox = [box_one addBox:HorizontalBox :0 :0 :bname];
		[CocoaLabel createT:hbox title:name width:120];
		txtCOMHost[i] = [CocoaTextField createT:hbox text:pConfig->comm_server_host[i].Get() action:nil width:140];

		[CocoaLabel createI:hbox title:CMsg::_Port align:NSTextAlignmentRight];
		txtCOMPort[i] = [CocoaTextField createN:hbox num:pConfig->comm_server_port[i] action:nil align:NSTextAlignmentRight width:80];

		popCOMDipswitch[i] = [CocoaPopUpButton createI:hbox items:LABELS::comm_baud action:nil selidx:(pConfig->comm_dipswitch[i]-1)];
	}
#endif
#ifdef USE_DEBUGGER
	hbox = [box_one addBox:HorizontalBox :0 :0 :_T("Debugger")];

	[CocoaLabel createI:hbox title:CMsg::Connectable_host_to_Debugger];
	txtDbgrHost = [CocoaTextField createT:hbox text:pConfig->debugger_server_host.Get() action:nil width:140];

	[CocoaLabel createI:hbox title:CMsg::_Port align:NSTextAlignmentRight];
	txtDbgrPort = [CocoaTextField createN:hbox num:pConfig->debugger_server_port action:nil align:NSTextAlignmentRight width:80];
#endif
	// uart
	int uart_baud_index = 0;
	for(int i=0; LABELS::comm_uart_baudrate[i] != NULL; i++) {
		if (pConfig->comm_uart_baudrate == (int)strtol(LABELS::comm_uart_baudrate[i], NULL, 10)) {
			uart_baud_index = i;
			break;
		}
	}
	bbox = [box_one addBox:BoxViewBox :0 :COCOA_DEFAULT_MARGIN :_T("UART")];
	[CocoaBox createI:bbox :CMsg::Settings_of_serial_ports_on_host :260 :1];

	vbox = [bbox addBox:VerticalBox :0 :0 :_T("UARTV")];

	hbox = [vbox addBox:HorizontalBox :0 :0 :_T("BaudRate")];
	[CocoaLabel createI:hbox title:CMsg::Baud_Rate width:120];
	popCOMUartBaud = [CocoaPopUpButton createT:hbox items:LABELS::comm_uart_baudrate action:nil selidx:uart_baud_index];

	hbox = [vbox addBox:HorizontalBox :0 :0 :_T("DataBit")];
	[CocoaLabel createI:hbox title:CMsg::Data_Bit width:120];
	popCOMUartDataBit = [CocoaPopUpButton createT:hbox items:LABELS::comm_uart_databit action:nil selidx:(pConfig->comm_uart_databit - 7)];

	hbox = [vbox addBox:HorizontalBox :0 :0 :_T("Parity")];
	[CocoaLabel createI:hbox title:CMsg::Parity width:120];
	popCOMUartParity = [CocoaPopUpButton createI:hbox items:LABELS::comm_uart_parity action:nil selidx:pConfig->comm_uart_parity];

	hbox = [vbox addBox:HorizontalBox :0 :0 :_T("StopBit")];
	[CocoaLabel createI:hbox title:CMsg::Stop_Bit width:120];
	popCOMUartStopBit = [CocoaPopUpButton createT:hbox items:LABELS::comm_uart_stopbit action:nil selidx:(pConfig->comm_uart_stopbit - 1)];

	hbox = [vbox addBox:HorizontalBox :0 :0 :_T("FlowCtrl")];
	[CocoaLabel createI:hbox title:CMsg::Flow_Control width:120];
	popCOMUartFlowCtrl = [CocoaPopUpButton createI:hbox items:LABELS::comm_uart_flowctrl action:nil selidx:pConfig->comm_uart_flowctrl];

	[CocoaLabel createI:vbox title:CMsg::Need_re_connect_to_serial_port_when_modified_this];


	// ----------------------------------------
	// CPU, Memory tab
	// ----------------------------------------
	tab = [tabView tabViewItemAtIndex:4];
	tab_view = (CocoaView *)[tab view];
	[box_tab setContentView:tab_view];

	box_one = [box_tab addBox:VerticalBox :0 :0 :_T("CPU")];

	hbox = [box_one addBox:HorizontalBox :MiddlePos :0 :_T("ROMPath")];
	[CocoaLabel createI:hbox title:CMsg::ROM_Path_ASTERISK];
	txtROMPath = [CocoaTextField createT:hbox text:pConfig->rom_path.Get() action:nil width:380];
	btn = [CocoaButton createI:hbox title:CMsg::Folder_ action:@selector(showFolderPanel:)];
	[btn setRelatedObject:txtROMPath];

	hbox = [box_one addBox:HorizontalBox :MiddlePos :0 :_T("MEMSize")];
#if defined(_MBS1)
	[CocoaLabel createI:hbox title:CMsg::Extended_RAM_ASTERISK];
	popExtRam = [CocoaPopUpButton createI:hbox items:LABELS::exram_size action:nil selidx:emu->get_parami(VM::ParamExMemNum) width:120];

	if (0 <= pConfig->exram_size_num && pConfig->exram_size_num <= 4) {
		char str[128];
		strcpy(str, CMSG(LB_Now_SP));
		strcat(str, gMessages.Get(LABELS::exram_size[pConfig->exram_size_num]));
		strcat(str, ")");
		[CocoaLabel createT:hbox title:str];
	}

	chkMemNoWait = [CocoaCheckBox createI:box_one title:CMsg::No_wait_to_access_the_main_memory action:nil value:pConfig->mem_nowait];
#else
	chkExMem = [CocoaCheckBox createI:hbox title:CMsg::Use_Extended_Memory_64KB action:nil value:(pConfig->exram_size_num == 1)];
#endif

	hbox = [box_one addBox:HorizontalBox :MiddlePos :0 :_T("UndefCPU")];
	chkUndefOp = [CocoaCheckBox createI:hbox title:CMsg::Show_message_when_the_CPU_fetches_undefined_opcode action:nil value:FLG_SHOWMSG_UNDEFOP];

#if defined(_MBS1)
# if defined(USE_Z80B_CARD)
	// z80b card
	hbox = [box_one addBox:HorizontalBox :MiddlePos :0 :_T("Z80BCard")];
	[CocoaLabel createI:hbox title:CMsg::Connect_interrupt_signal_of_Z80B_Card_to_ASTERISK];
	popZ80BCardIrq = [CocoaPopUpButton createI:hbox items:LABELS::z80bcard_irq action:nil selidx:pConfig->z80b_card_out_irq];
# elif defined(USE_MPC_68008)
	hbox = [box_one addBox:HorizontalBox :MiddlePos :0 :_T("AddrErr")];
	chkAddrErr = [CocoaCheckBox createI:hbox title:CMsg::Show_message_when_the_address_error_occured_in_MC68008 action:nil value:FLG_SHOWMSG_ADDRERR];
# endif
#endif

	hbox = [box_one addBox:HorizontalBox :MiddlePos :0 :_T("ClrCPUReg")];
	chkClrCPUReg = [CocoaCheckBox createI:hbox title:CMsg::Clear_CPU_registers_at_power_on action:nil value:FLG_CLEAR_CPUREG];

	[CocoaLabel createI:box_one title:CMsg::Need_restart_program_or_PowerOn];

#if defined(_MBS1)
	// ----------------------------------------
	// Sound tab
	// ----------------------------------------
	tab = [tabView tabViewItemAtIndex:5];
	tab_view = (CocoaView *)[tab view];
	[box_tab setContentView:tab_view];

	box_one = [box_tab addBox:VerticalBox :0 :0 :_T("Sound")];

	//
	bbox = [box_one addBox:BoxViewBox :0 :COCOA_DEFAULT_MARGIN :_T("FmOPN")];
	[CocoaBox createI:bbox :CMsg::FM_Synthesis_Card_ASTERISK :260 :1];

	vbox = [bbox addBox:VerticalBox :0 :0 :_T("FmOPNV")];

	chkFmOpnEn = [CocoaCheckBox createI:vbox title:CMsg::Enable action:@selector(selectFmOpn:) value:IOPORT_USE_FMOPN];

	[CocoaLabel createI:vbox title:CMsg::IO_ports_are_FF1E_FF1F_FF16_FF17];

	hbox = [vbox addBox:HorizontalBox :MiddlePos :0 :_T("FmOPNChip")];
	[CocoaLabel createI:hbox title:CMsg::Sound_chip];
	popFmOpnChip = [CocoaPopUpButton createI:hbox items:LABELS::type_of_soundcard action:nil selidx:emu->get_parami(VM::ParamChipTypeOnFmOpn) appendnum:pConfig->type_of_fmopn appendstr:CMsg::LB_Now_RB];

	//
	bbox = [box_one addBox:BoxViewBox :0 :COCOA_DEFAULT_MARGIN :_T("ExPSG")];
	[CocoaBox createI:bbox :CMsg::Extended_PSG_Port_ASTERISK :260 :1];

	vbox = [bbox addBox:VerticalBox :0 :0 :_T("ExPSGV")];

	chkExPsgEn = [CocoaCheckBox createI:vbox title:CMsg::Enable action:@selector(selectExPsg:) value:IOPORT_USE_EXPSG];

	[CocoaLabel createI:vbox title:CMsg::IO_ports_are_FFE6_FFE7_FFEE_FFEF];

	hbox = [vbox addBox:HorizontalBox :MiddlePos :0 :_T("ExPSGChip")];
	[CocoaLabel createI:hbox title:CMsg::Sound_chip];
	popExPsgChip = [CocoaPopUpButton createI:hbox items:LABELS::type_of_soundcard action:nil selidx:emu->get_parami(VM::ParamChipTypeOnExPsg) appendnum:pConfig->type_of_expsg appendstr:CMsg::LB_Now_RB];

	//
	hbox = [box_one addBox:HorizontalBox :MiddlePos :0 :_T("IntrSnd")];

	[CocoaLabel createI:hbox title:CMsg::Connect_interrupt_signal_of_FM_synthesis_to_ASTERISK_ASTERISK];

	popFmOpnIrq = [CocoaPopUpButton createI:hbox items:LABELS::fmopn_irq action:nil selidx:pConfig->opn_irq];

	[CocoaLabel createI:box_one title:CMsg::Need_restart_program_or_PowerOn];
	[CocoaLabel createI:box_one title:CMsg::This_is_the_common_setting_both_FM_synthesis_card_and_extended_PSG_port];
#endif

	// button

	hbox = [box_all addBox:HorizontalBox :RightPos | TopPos :0 :_T("BTN")];
	[CocoaButton createI:hbox title:CMsg::Cancel action:@selector(dialogCancel:) width:120];
	[CocoaButton createI:hbox title:CMsg::OK action:@selector(dialogOk:) width:120];


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
	[NSApp stopModalWithCode:NSModalResponseCancel];
	[super close];
}

- (void)dialogOk:(id)sender
{
	int i;
	int valuel;
	int val;

	// set data
	pConfig->use_power_off = ([chkPowerOff state] == NSControlStateValueOn);
#if defined(_MBS1)
	emu->set_parami(VM::ParamSysMode,([radSysMode[0] state] == NSControlStateValueOn ? 1 : 0) | (pConfig->sys_mode & ~1));
#endif
	if ([chkDipswitch state] == NSControlStateValueOn) {
		pConfig->dipswitch |= 4;
	} else {
		pConfig->dipswitch &= ~4;
	}
	for(i=0; i<4; i++) {
		if ([radFddType[i] state] == NSControlStateValueOn) emu->set_parami(VM::ParamFddType,i);
	}
#ifdef USE_FD1
	for(i=0;i<USE_FLOPPY_DISKS;i++) {
		pConfig->mount_fdd = (([chkFddMount[i] state] == NSControlStateValueOn) ? pConfig->mount_fdd | (1 << i) : pConfig->mount_fdd & ~(1 << i));
	}
	pConfig->option_fdd = ([chkDelayFd1 state] == NSControlStateValueOn ? MSK_DELAY_FDSEARCH : 0)
		| ([chkDelayFd2 state] == NSControlStateValueOn ? MSK_DELAY_FDSEEK : 0)
		| ([chkFdDensity state] == NSControlStateValueOn ? 0 : MSK_CHECK_FDDENSITY)
		| ([chkFdMedia state] == NSControlStateValueOn ? 0 : MSK_CHECK_FDMEDIA)
		| ([chkFdSavePlain state] == NSControlStateValueOn ? MSK_SAVE_FDPLAIN : 0);
#endif

#ifdef USE_HD1
	for(i=0;i<USE_HARD_DISKS;i++) {
		pConfig->mount_hdd = (([chkHddMount[i] state] == NSControlStateValueOn) ? pConfig->mount_hdd | (1 << i) : pConfig->mount_hdd & ~(1 << i));
	}
	pConfig->option_hdd = ([chkDelayHd2 state] == NSControlStateValueOn ? MSK_DELAY_HDSEEK : 0);
#endif

	for(i=IOPORT_STARTNUM;i<IOPORT_NUMS;i++) {
		if ((1 << i) & IOPORT_MSK_ALL) {
			val = emu->get_parami(VM::ParamIOPort);
			if ([chkIOPort[i] state] == NSControlStateValueOn) {
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
	pConfig->use_opengl = (int)[popUseOpenGL indexOfSelectedItem];
	pConfig->gl_filter_type = [popGLFilter indexOfSelectedItem];

	pConfig->led_pos = [popLEDPosition indexOfSelectedItem];
	pConfig->capture_type = [popCaptureType indexOfSelectedItem];

#if defined(_MBS1)
	pConfig->curdisp_skew = [popCurdispSkew indexOfSelectedItem] - 2;
	pConfig->disptmg_skew = [popDisptmgSkew indexOfSelectedItem] - 2;
#else
	pConfig->curdisp_skew = [popCurdispSkew indexOfSelectedItem];
	pConfig->disptmg_skew = [popDisptmgSkew indexOfSelectedItem];
#endif

	pConfig->snapshot_path.Set([[txtSnapPath stringValue] UTF8String]);
	pConfig->font_path.Set([[txtFontPath stringValue] UTF8String]);

	pConfig->msgboard_msg_fontname.Set([[txtMsgFontName stringValue] UTF8String]);
	pConfig->msgboard_info_fontname.Set([[txtInfoFontName stringValue] UTF8String]);
	valuel = [txtMsgFontSize intValue];
	if (1 <= valuel && valuel <= 60) {
		pConfig->msgboard_msg_fontsize = (int)valuel;
	}
	valuel = [txtInfoFontSize intValue];
	if (1 <= valuel && valuel <= 60) {
		pConfig->msgboard_info_fontsize = (int)valuel;
	}

	// language
	valuel = (int)[popLanguage indexOfSelectedItem];
	clocale->ChooseLocaleName(lang_list, valuel, pConfig->language);

#ifdef USE_DATAREC
	pConfig->wav_reverse = ([chkReverseWave state] == NSControlStateValueOn);
	pConfig->wav_half = ([chkHalfWave state] == NSControlStateValueOn);
//	pConfig->wav_correct = ([chkCorrectWave state] == NSControlStateValueOn);
	pConfig->wav_correct = ([radCorrect selectedColumn] == 0);
	pConfig->wav_correct_type = [radCorrect selectedColumn];
	pConfig->wav_correct_type = (pConfig->wav_correct_type > 0 ? pConfig->wav_correct_type - 1 : 0);
	valuel = [txtCorrectAmp[0] intValue];
	if (valuel >= 100 && valuel <= 5000) {
		pConfig->wav_correct_amp[0] = (int)valuel;
	}
	valuel = [txtCorrectAmp[1] intValue];
	if (valuel >= 100 && valuel <= 5000) {
		pConfig->wav_correct_amp[1] = (int)valuel;
	}

	pConfig->wav_sample_rate = [popSampleRate indexOfSelectedItem];
	pConfig->wav_sample_bits = [popSampleBits indexOfSelectedItem];
#endif

#ifdef MAX_PRINTER
	for(i=0;i<MAX_PRINTER;i++) {
		pConfig->printer_server_host[i].Set([[txtLPTHost[i] stringValue] UTF8String]);
		valuel = [txtLPTPort[i] intValue];
		if (0 <= valuel && valuel <= 65535) {
			pConfig->printer_server_port[i] = (int)valuel;
		}
		double valued = 0.0;
		valued = strtod([[txtLPTDelay[i] stringValue] UTF8String], NULL);
		if (valued < 0.1) valued = 0.1;
		if (valued > 1000.0) valued = 1000.0;
		valued = floor(valued * 10.0 + 0.5) / 10.0;
		pConfig->printer_delay[i] = valued;
	}
#endif
#ifdef MAX_COMM
	for(i=0;i<MAX_COMM;i++) {
		pConfig->comm_server_host[i].Set([[txtCOMHost[i] stringValue] UTF8String]);
		valuel = [txtCOMPort[i] intValue];
		if (0 <= valuel && valuel <= 65535) {
			pConfig->comm_server_port[i] = (int)valuel;
		}
		pConfig->comm_dipswitch[i] = (int)[popCOMDipswitch[i] indexOfSelectedItem] + 1;
	}
#endif
#ifdef USE_DEBUGGER
	pConfig->debugger_server_host.Set([[txtDbgrHost stringValue] UTF8String]);
	valuel = [txtDbgrPort intValue];
	if (0 <= valuel && valuel <= 65535) {
		pConfig->debugger_server_port = (int)valuel;
	}
#endif
	int uart_baud_index = (int)[popCOMUartBaud indexOfSelectedItem];
	pConfig->comm_uart_baudrate = (int)strtol(LABELS::comm_uart_baudrate[uart_baud_index], NULL, 10);
	pConfig->comm_uart_databit = (int)[popCOMUartDataBit indexOfSelectedItem] + 7;
	pConfig->comm_uart_parity = (int)[popCOMUartParity indexOfSelectedItem];
	pConfig->comm_uart_stopbit = (int)[popCOMUartStopBit indexOfSelectedItem] + 1;
	pConfig->comm_uart_flowctrl = (int)[popCOMUartFlowCtrl indexOfSelectedItem];

	pConfig->rom_path.Set([[txtROMPath stringValue] UTF8String]);

	BIT_ONOFF(pConfig->misc_flags, MSK_SHOWMSG_UNDEFOP, [chkUndefOp state] == NSControlStateValueOn);

	BIT_ONOFF(pConfig->misc_flags, MSK_CLEAR_CPUREG, [chkClrCPUReg state] == NSControlStateValueOn);

#if defined(_MBS1)
# if defined(USE_Z80B_CARD)
	pConfig->z80b_card_out_irq = (int)[popZ80BCardIrq indexOfSelectedItem];
# elif defined(USE_MPC_68008)
	BIT_ONOFF(pConfig->misc_flags, MSK_SHOWMSG_ADDRERR, [chkAddrErr state] == NSControlStateValueOn);
# endif
	emu->set_parami(VM::ParamExMemNum, (int)[popExtRam indexOfSelectedItem]);

	pConfig->mem_nowait = ([chkMemNoWait state] == NSControlStateValueOn);

//	pConfig->opn_clock = (int)[popFmOpnClk indexOfSelectedItem];
	pConfig->opn_irq = (int)[popFmOpnIrq indexOfSelectedItem];
	emu->set_parami(VM::ParamChipTypeOnFmOpn, (int)[popFmOpnChip indexOfSelectedItem]);
	emu->set_parami(VM::ParamChipTypeOnExPsg, (int)[popExPsgChip indexOfSelectedItem]);
#else
	pConfig->exram_size_num = ([chkExMem state] == NSControlStateValueOn) ? 1 : 0;
#endif

	// set message font
	MsgBoard *msgboard = emu->get_msgboard();
	if (msgboard) {
		msgboard->SetFont();
	}

	gui->ChangeLedBox((int)[popLEDShow indexOfSelectedItem]);
	gui->ChangeLedBoxPosition(pConfig->led_pos);
	pConfig->save();
	emu->change_opengl_attr();

	emu->update_config();

	// OK button is pushed
	[NSApp stopModalWithCode:NSModalResponseOK];
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

	if(result == NSModalResponseOK) {
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

	if(result == NSModalResponseOK) {
		// get file path (use NSURL)
		NSURL *filePath = [panel URL];
		NSArray *arr = [filePath pathComponents];
		[text setStringValue:[arr lastObject]];
	}
}

- (void)showFontPanel:(id)sender
{
//	NSInteger result;

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
	[panel runModal];

//	if(result == NSModalResponseOK) {
//	}
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
		[radSysMode[i] setState:(i == idx ? NSControlStateValueOn : NSControlStateValueOff)];
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
			[chkIOPort[0] setState:NSControlStateValueOff];
			[chkIOPort[1] setState:NSControlStateValueOff];
			[chkIOPort[0] setEnabled:YES];
			[chkIOPort[1] setEnabled:YES];
			break;
		case 1:
			// 3inch FDD
			[chkIOPort[0] setState:NSControlStateValueOff];
			[chkIOPort[1] setState:NSControlStateValueOn];
			[chkIOPort[0] setEnabled:NO];
			[chkIOPort[1] setEnabled:NO];
			break;
		case 2:
		case 3:
			// 5inch FDD
			[chkIOPort[0] setState:NSControlStateValueOn];
			[chkIOPort[1] setState:NSControlStateValueOff];
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
			if ([sender state] == NSControlStateValueOn) {
				[chkIOPort[IOPORT_POS_3FDD] setState:NSControlStateValueOff];
			}
			break;
		case IOPORT_POS_3FDD:
			// 3inch FDD
			if ([sender state] == NSControlStateValueOn) {
				[chkIOPort[IOPORT_POS_5FDD] setState:NSControlStateValueOff];
			}
			break;
#endif
		case IOPORT_POS_PSG9:
			// 9psg
			if ([sender state] == NSControlStateValueOn) {
				[chkIOPort[IOPORT_POS_KANJI] setState:NSControlStateValueOff];
#if defined(_MBS1)
				[chkIOPort[IOPORT_POS_CM01] setState:NSControlStateValueOff];
				[chkIOPort[IOPORT_POS_KANJI2] setState:NSControlStateValueOff];
#endif
			}
			break;
		case IOPORT_POS_KANJI:
			// Kanji
			if ([sender state] == NSControlStateValueOn) {
				[chkIOPort[IOPORT_POS_PSG9] setState:NSControlStateValueOff];
			}
			break;
#if defined(_MBS1)
		case IOPORT_POS_CM01:
			// Comm Card
			if ([sender state] == NSControlStateValueOn) {
				[chkIOPort[IOPORT_POS_PSG9] setState:NSControlStateValueOff];
				[chkIOPort[IOPORT_POS_KANJI] setState:NSControlStateValueOn];
			}
			break;
		case IOPORT_POS_KANJI2:
			// Kanji 2
			if ([sender state] == NSControlStateValueOn) {
				[chkIOPort[IOPORT_POS_PSG9] setState:NSControlStateValueOff];
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
//	if ([sender state] == NSControlStateValueOn) {
//		[radCorrect setEnabled:YES];
//	} else {
//		[radCorrect setEnabled:NO];
//	}
}

@end
