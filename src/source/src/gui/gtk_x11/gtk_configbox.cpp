/** @file gtk_configbox.cpp

	HITACHI BASIC MASTER LEVEL3 Mark5 Emulator 'EmuB-6892'
	HITACHI MB-S1 Emulator 'EmuB-S1'
	Skelton for retropc emulator

	@author Sasaji
	@date   2015.05.10 -

	@brief [ config box ]
*/

#include "gtk_configbox.h"
#include "gtk_folderbox.h"
#include "gtk_filebox.h"
#include "../../emu.h"
#include "../../config.h"
#include "../../clocale.h"
#include "../../msgboard.h"
#include "../../utility.h"
#include "../../msgs.h"
#include "gtk_x11_gui.h"
#include "../../labels.h"
#include <math.h>

extern EMU *emu;

namespace GUI_GTK_X11
{

#ifdef USE_IOPORT_FDD
#define IOPORT_STARTNUM 0
#else
#define IOPORT_STARTNUM 2
#endif

ConfigBox::ConfigBox(GUI *new_gui) : DialogBox(new_gui)
{
	memset(chkIO, 0, sizeof(chkIO));
}

ConfigBox::~ConfigBox()
{
}

bool ConfigBox::Show(GtkWidget *parent_window)
{
	DialogBox::Show(parent_window);

	if (dialog) return true;

	create_dialog(window, CMsg::Configure);
	add_accept_button(CMsg::OK);
	add_reject_button(CMsg::Cancel);

	GtkWidget *cont = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	GtkWidget *nb;
	GtkWidget *vboxall;
	GtkWidget *hboxall;
	GtkWidget *vboxl;
	GtkWidget *vboxr;
	GtkWidget *hbox;
	GtkWidget *vbox;
//	GtkWidget *frm;
//	GtkWidget *lbl;
//	GtkWidget *btn;

	char buf[128];

	// create notebook tab
	nb = create_notebook(cont);

	// ----------------------------------------
	// 0 Mode
	// ----------------------------------------

	vboxall = create_vbox(NULL);
	add_note(nb, vboxall, LABELS::tabs[0]);

	hboxall = create_hbox(vboxall);
	vboxl = create_vbox(hboxall);

#if defined(_MBS1)
	int sys_mode = 1 - (emu->get_parami(VM::ParamSysMode) & 1);
	create_frame(vboxl, CMsg::System_Mode_ASTERISK, &vbox);
	for(int i=0; i<2; i++) {
		hbox = create_hbox(vbox);
		create_label(hbox, (pConfig->sys_mode & 1) == (1 - i) ? ">" : " ");
		radSYS[i] = create_radio_box(hbox,radSYS,LABELS::sys_mode[i],i,sys_mode);
	}
	hbox = create_hbox(vbox);
	create_label(hbox, "   ");
	chkMODE = create_check_box(hbox, CMsg::NEWON7, pConfig->dipswitch & 4);
#else
	create_frame(vboxl, CMsg::DIP_Switch_ASTERISK, &vbox, &hbox);
	create_label(hbox, (pConfig->dipswitch & 4) ? ">" : " ");
	chkMODE = create_check_box(hbox, CMsg::MODE_Switch, pConfig->dipswitch & 4);
#endif

	int fdd_type = emu->get_parami(VM::ParamFddType);
	create_frame(vboxl, CMsg::FDD_Type_ASTERISK, &vbox);
	for(int i=0; i<4; i++) {
		hbox = create_hbox(vbox);
		create_label(hbox, pConfig->fdd_type == i ? ">" : " ");
		radFDD[i] = create_radio_box(hbox,radFDD,LABELS::fdd_type[i],i,fdd_type, G_CALLBACK(OnChangedFDD));
	}

	chkPowerOff = create_check_box(vboxl, CMsg::Enable_the_state_of_power_off, pConfig->use_power_off);

	vboxr = create_vbox(hboxall);

	int io_port = emu->get_parami(VM::ParamIOPort);
	create_frame(vboxr, CMsg::I_O_Port_Address_ASTERISK, &vbox);
	for(int i=0; LABELS::io_port[i] != CMsg::End; i++) {
		int pos = LABELS::io_port_pos[i];
		if ((1 << pos) & IOPORT_MSK_ALL) {
			hbox = create_hbox(vbox);
			create_label(hbox, pConfig->io_port & (1 << pos) ? ">" : " ");
			chkIO[pos] = create_check_box(hbox,LABELS::io_port[i],io_port & (1 << pos), pos, G_CALLBACK(OnChangedIO));
		} else {
			chkIO[pos] = NULL;
		}
	}
	ChangeFDDType(get_radio_state_idx(radFDD,4));

	hbox = create_hbox(vboxall);
	create_label(hbox, CMsg::Need_restart_program_or_PowerOn);

	// ----------------------------------------
	// 1 screen
	// ----------------------------------------

	vboxall = create_vbox(NULL);
	add_note(nb, vboxall, LABELS::tabs[1]);

	hboxall = create_hbox(vboxall);

	int use_opengl = 0;
	int gl_filter_type = 0;
#ifdef USE_OPENGL
	use_opengl = pConfig->use_opengl;
	gl_filter_type = pConfig->gl_filter_type;
#endif
	create_frame(hboxall, CMsg::Drawing, &vbox, &hbox);
	comUseOpenGL = create_combo_box(hbox,CMsg::Method_ASTERISK,LABELS::opengl_use,use_opengl);
	hbox = create_hbox(vbox);
	comGLFilter = create_combo_box(hbox,CMsg::Filter_Type,LABELS::opengl_filter,gl_filter_type);
#ifndef USE_OPENGL
	gtk_widget_set_sensitive(comUseOpenGL, FALSE);
	gtk_widget_set_sensitive(comGLFilter, FALSE);
#endif

	create_frame(hboxall, CMsg::CRTC, &vbox, &hbox);
#if defined(_MBS1)
	comDispSkew = create_combo_box(hbox,CMsg::Disptmg_Skew,LABELS::disp_skew,pConfig->disptmg_skew + 2);
#else
	comDispSkew = create_combo_box(hbox,CMsg::Disptmg_Skew,LABELS::disp_skew,pConfig->disptmg_skew);
#endif
	hbox = create_hbox(vbox);
#if defined(_MBS1)
	comCurdSkew = create_combo_box(hbox,CMsg::Curdisp_Skew_L3,LABELS::disp_skew,pConfig->curdisp_skew + 2);
#else
	comCurdSkew = create_combo_box(hbox,CMsg::Curdisp_Skew,LABELS::disp_skew,pConfig->curdisp_skew);
#endif

	int led_show = gui->GetLedBoxPhase(-1);
	hbox = create_hbox(vboxall);
	comLEDShow = create_combo_box(hbox,CMsg::LED,LABELS::led_show,led_show);
	comLEDPos = create_combo_box(hbox,CMsg::Position,LABELS::led_pos,pConfig->led_pos);

	hbox = create_hbox(vboxall);
	comCapType = create_combo_box(hbox,CMsg::Capture_Type,LABELS::capture_fmt,pConfig->capture_type);

	hbox = create_hbox(vboxall);
	txtSnapPath = create_text_with_label(hbox, CMsg::Snapshot_Path, pConfig->snapshot_path.GetN(), 40);
	create_button(hbox,CMsg::Folder_, G_CALLBACK(OnSelectSnapPath));

	hbox = create_hbox(vboxall);
	txtFontPath = create_text_with_label(hbox, CMsg::Font_Path, pConfig->font_path.GetN(), 40);
	create_button(hbox, CMsg::Folder_, G_CALLBACK(OnSelectFontPath));

	hbox = create_hbox(vboxall);
	txtMsgFontName = create_text_with_label(hbox, CMsg::Message_Font, pConfig->msgboard_msg_fontname.GetN(), 24);
	sprintf(buf, "%d", pConfig->msgboard_msg_fontsize);
	txtMsgFontSize = create_text_with_label(hbox, CMsg::_Size, buf, 3);
	create_button(hbox, CMsg::File_, G_CALLBACK(OnSelectMessageFont));

	hbox = create_hbox(vboxall);
	txtInfoFontName = create_text_with_label(hbox, CMsg::Info_Font, pConfig->msgboard_info_fontname.GetN(), 24);
	sprintf(buf, "%d", pConfig->msgboard_info_fontsize);
	txtInfoFontSize = create_text_with_label(hbox, CMsg::_Size, buf, 3);
	create_button(hbox, CMsg::File_, G_CALLBACK(OnSelectInfoFont));

	lang_list.Clear();
	clocale->GetLocaleNamesWithDefault(lang_list);
	int lang_selidx = clocale->SelectLocaleNameIndex(lang_list, pConfig->language);

	hbox = create_hbox(vboxall);
	comLanguage = create_combo_box(hbox, CMsg::Language_ASTERISK, lang_list, lang_selidx);

	hbox = create_hbox(vboxall);
	create_label(hbox, CMsg::Need_restart_program);

	// ----------------------------------------
	// 2 tape, FDD
	// ----------------------------------------

	vboxall = create_vbox(NULL);
	add_note(nb, vboxall, LABELS::tabs[2]);

#ifdef USE_DATAREC
	hboxall = create_hbox(vboxall);

	create_frame(hboxall, CMsg::Load_Wav_File_from_Tape, &vbox, &hbox);
	chkReverseWave = create_check_box(hbox, CMsg::Reverse_Wave, pConfig->wav_reverse);
	hbox = create_hbox(vbox);
	chkHalfWave = create_check_box(hbox, CMsg::Half_Wave, pConfig->wav_half);
	hbox = create_hbox(vbox);
	create_label(hbox, CMsg::Correct);
	int correct_type = pConfig->wav_correct ? pConfig->wav_correct_type + 1 : 0;
	create_radio_box(hbox, LABELS::correct, 3, radCorrectType, correct_type);

	hbox = create_hbox(vbox);
	for(int i=0; i<2; i++) {
		sprintf(buf, "%d", pConfig->wav_correct_amp[i]);
		txtCorrectAmp[i] = create_text_with_label(hbox, LABELS::correct_amp[i], buf, 4);
	}

	create_frame(hboxall, CMsg::Save_Wav_File_to_Tape, &vbox, &hbox);
	comSampleRate = create_combo_box(hbox,CMsg::Sample_Rate,LABELS::wav_sampling_rate,pConfig->wav_sample_rate);
	hbox = create_hbox(vbox);
	comSampleBits = create_combo_box(hbox,CMsg::Sample_Bits,LABELS::wav_sampling_bits,pConfig->wav_sample_bits);
#endif

	// FDD
#ifdef USE_FD1
	hboxall = create_hbox(vboxall);
	// fdd mount
	create_frame(hboxall, CMsg::Floppy_Disk_Drive, &vbox, &hbox);
	create_label(hbox, CMsg::When_start_up_mount_disk_at_);
	for(int i=0; i<USE_FLOPPY_DISKS; i++) {
		UTILITY::sprintf(buf, sizeof(buf), "%d  ", i);
		chkFDMount[i] = create_check_box(hbox, buf, pConfig->mount_fdd & (1 << i));
	}
	hbox = create_hbox(vbox);
	chkDelayFd1 = create_check_box(hbox, CMsg::Ignore_delays_to_find_sector, FLG_DELAY_FDSEARCH);
	hbox = create_hbox(vbox);
	chkDelayFd2 = create_check_box(hbox, CMsg::Ignore_delays_to_seek_track, FLG_DELAY_FDSEEK);
	hbox = create_hbox(vbox);
	chkFdDensity = create_check_box(hbox, CMsg::Suppress_checking_for_density, (FLG_CHECK_FDDENSITY == 0));
	hbox = create_hbox(vbox);
	chkFdMedia = create_check_box(hbox, CMsg::Suppress_checking_for_media_type, (FLG_CHECK_FDMEDIA == 0));
	hbox = create_hbox(vbox);
	chkFdSavePlain = create_check_box(hbox, CMsg::Save_a_plain_disk_image_as_it_is, (FLG_SAVE_FDPLAIN != 0));
#endif

	// HDD
#ifdef USE_HD1
	hboxall = create_hbox(vboxall);
	// hdd mount
	create_frame(hboxall, CMsg::Hard_Disk_Drive, &vbox, &hbox);
	create_label(hbox, CMsg::When_start_up_mount_disk_at_);
	for(int i=0; i<USE_HARD_DISKS; i++) {
		UTILITY::sprintf(buf, sizeof(buf), "%d  ", i);
		chkHDMount[i] = create_check_box(hbox, buf, pConfig->mount_hdd & (1 << i));
	}

	hbox = create_hbox(vbox);
	chkDelayHd2 = create_check_box(hbox, CMsg::Ignore_delays_to_seek_track, (FLG_DELAY_HDSEEK != 0));
#endif

	// ----------------------------------------
	// 3 network
	// ----------------------------------------

	vboxall = create_vbox(NULL);
	add_note(nb, vboxall, LABELS::tabs[3]);

	// LPT
#ifdef MAX_PRINTER
	for(int drv=0; drv<MAX_PRINTER; drv++) {
		hbox = create_hbox(vboxall);
		sprintf(buf, CMSG(LPTVDIGIT_Hostname), drv);
		txtLPTHost[drv] = create_text_with_label(hbox, buf, pConfig->printer_server_host[drv].GetN(), 20);
		sprintf(buf, "%d", pConfig->printer_server_port[drv]);
		txtLPTPort[drv] = create_text_with_label(hbox, CMsg::_Port, buf, 5);
		sprintf(buf, "%.1f", pConfig->printer_delay[drv]);
		txtLPTDelay[drv] = create_text_with_label(hbox, CMsg::_Print_delay, buf, 5);
		create_label(hbox, CMsg::msec);
	}
#endif
#ifdef MAX_COMM
	// COM
	for(int drv=0; drv<MAX_COMM; drv++) {
		hbox = create_hbox(vboxall);
		sprintf(buf, CMSG(COMVDIGIT_Hostname), drv);
		txtCOMHost[drv] = create_text_with_label(hbox, buf, pConfig->comm_server_host[drv].GetN(), 20);
		sprintf(buf, "%d", pConfig->comm_server_port[drv]);
		txtCOMPort[drv] = create_text_with_label(hbox, CMsg::_Port, buf, 5);
		comCOMBaud[drv] = create_combo_box(hbox, " ", LABELS::comm_baud, pConfig->comm_dipswitch[drv]);
	}
#endif
#ifdef USE_DEBUGGER
	hbox = create_hbox(vboxall);
	create_label(hbox, CMsg::Connectable_host_to_Debugger);
	hbox = create_hbox(vboxall);
	txtDbgrHost = create_text_with_label(hbox, " ", pConfig->debugger_server_host.GetN(), 20);
	sprintf(buf, "%d", pConfig->debugger_server_port);
	txtDbgrPort = create_text_with_label(hbox, CMsg::_Port, buf, 5);
#endif
	// uart
	int uart_baud_index = 0;
	for(int i=0; LABELS::comm_uart_baudrate[i] != NULL; i++) {
		if (pConfig->comm_uart_baudrate == (int)strtol(LABELS::comm_uart_baudrate[i], NULL, 10)) {
			uart_baud_index = i;
			break;
		}
	}
	create_frame(vboxall, CMsg::Settings_of_serial_ports_on_host, &vbox, &hbox);
	create_label(hbox, CMsg::Baud_Rate);
	comCOMUartBaud = create_combo_box(hbox, " ", LABELS::comm_uart_baudrate, uart_baud_index);
	hbox = create_hbox(vbox);
	create_label(hbox, CMsg::Data_Bit);
	comCOMUartDataBit = create_combo_box(hbox, " ", LABELS::comm_uart_databit, pConfig->comm_uart_databit - 7);
	hbox = create_hbox(vbox);
	create_label(hbox, CMsg::Parity);
	comCOMUartParity = create_combo_box(hbox, " ", LABELS::comm_uart_parity, pConfig->comm_uart_parity);
	hbox = create_hbox(vbox);
	create_label(hbox, CMsg::Stop_Bit);
	comCOMUartStopBit = create_combo_box(hbox, " ", LABELS::comm_uart_stopbit, pConfig->comm_uart_stopbit - 1);
	hbox = create_hbox(vbox);
	create_label(hbox, CMsg::Flow_Control);
	comCOMUartFlowCtrl = create_combo_box(hbox, " ", LABELS::comm_uart_flowctrl, pConfig->comm_uart_flowctrl);
	create_label(vbox, CMsg::Need_re_connect_to_serial_port_when_modified_this);

	// ----------------------------------------
	// 4 CPU, Memory
	// ----------------------------------------

	vboxall = create_vbox(NULL);
	add_note(nb, vboxall, LABELS::tabs[4]);

	hbox = create_hbox(vboxall);
	txtROMPath = create_text_with_label(hbox, CMsg::ROM_Path_ASTERISK, pConfig->rom_path.GetN(), 40);
	create_button(hbox,CMsg::Folder_, G_CALLBACK(OnSelectROMPath));

#if defined(_MBS1)
	int exram_num = emu->get_parami(VM::ParamExMemNum);
	hbox = create_hbox(vboxall);
	comExRam = create_combo_box(hbox, CMsg::Extended_RAM_ASTERISK, LABELS::exram_size, exram_num);
	if (0 <= pConfig->exram_size_num && pConfig->exram_size_num <= 4) {
		strcpy(buf, CMSG(LB_Now_SP));
		strcat(buf, gMessages.Get(LABELS::exram_size[pConfig->exram_size_num]));
		strcat(buf, ")");
		create_label(hbox, buf);
	}

	hbox = create_hbox(vboxall);
	chkMemNoWait = create_check_box(hbox, CMsg::No_wait_to_access_the_main_memory, pConfig->mem_nowait);
#else
	hbox = create_hbox(vboxall);
	chkExMem = create_check_box(hbox, CMsg::Use_Extended_Memory_64KB, pConfig->exram_size_num);
#endif

	hbox = create_hbox(vboxall);
	chkUndefOp = create_check_box(hbox, CMsg::Show_message_when_the_CPU_fetches_undefined_opcode, FLG_SHOWMSG_UNDEFOP != 0);

#if defined(_MBS1)
# if defined(USE_Z80B_CARD)
	hbox = create_hbox(vboxall);
	comZ80CardIrq = create_combo_box(hbox, CMsg::Connect_interrupt_signal_of_Z80B_Card_to_ASTERISK, LABELS::z80bcard_irq, pConfig->z80b_card_out_irq);
# elif defined(USE_MPC_68008)
	hbox = create_hbox(vboxall);
	chkAddrErr = create_check_box(hbox, CMsg::Show_message_when_the_address_error_occured_in_MC68008, FLG_SHOWMSG_ADDRERR != 0);
# endif
#endif

	hbox = create_hbox(vboxall);
	chkClrCPUReg = create_check_box(hbox, CMsg::Clear_CPU_registers_at_power_on, FLG_CLEAR_CPUREG != 0);

	hbox = create_hbox(vboxall);
	create_label(hbox, CMsg::Need_restart_program_or_PowerOn);

#if defined(_MBS1)
	// ----------------------------------------
	// 5 Sound
	// ----------------------------------------

	vboxall = create_vbox(NULL);
	add_note(nb, vboxall, LABELS::tabs[5]);

	create_frame(vboxall, CMsg::FM_Synthesis_Card_ASTERISK, &vbox, &hbox);
	chkFmOpnEn = create_check_box(hbox, CMsg::Enable, IOPORT_USE_FMOPN, 0, G_CALLBACK(OnChangedFmOpn));
	hbox = create_hbox(vbox);
	create_label(hbox, CMsg::IO_ports_are_FF1E_FF1F_FF16_FF17);
//	hbox = create_hbox(vbox);
//	comFmOpnClk = create_combo_box(hbox, CMsg::Clock, LABELS::fmopn_clock, pConfig->opn_clock);
//	create_label(hbox, "Hz");
	hbox = create_hbox(vbox);
	comFmOpnChip = create_combo_box(hbox, CMsg::Sound_chip, LABELS::type_of_soundcard, emu->get_parami(VM::ParamChipTypeOnFmOpn), pConfig->type_of_fmopn, CMsg::LB_Now_RB);

	create_frame(vboxall, CMsg::Extended_PSG_Port_ASTERISK, &vbox, &hbox);
	chkExPsgEn = create_check_box(hbox, CMsg::Enable, IOPORT_USE_EXPSG, 0, G_CALLBACK(OnChangedExPsg));
	hbox = create_hbox(vbox);
	create_label(hbox, CMsg::IO_ports_are_FFE6_FFE7_FFEE_FFEF);
	hbox = create_hbox(vbox);
	comExPsgChip = create_combo_box(hbox, CMsg::Sound_chip, LABELS::type_of_soundcard, emu->get_parami(VM::ParamChipTypeOnExPsg), pConfig->type_of_expsg, CMsg::LB_Now_RB);

	hbox = create_hbox(vboxall);
	comFmOpnIrq = create_combo_box(hbox, CMsg::Connect_interrupt_signal_of_FM_synthesis_to_ASTERISK_ASTERISK, LABELS::fmopn_irq, pConfig->opn_irq);

	hbox = create_hbox(vboxall);
	create_label(hbox, CMsg::Need_restart_program_or_PowerOn);
	hbox = create_hbox(vboxall);
	create_label(hbox, CMsg::This_is_the_common_setting_both_FM_synthesis_card_and_extended_PSG_port);
#endif

	//

	gtk_widget_show_all(dialog);
	g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(OnResponse), (gpointer)this);
//	gint rc = gtk_dialog_run(GTK_DIALOG(dialog));

	emu->set_pause(1, true);

	return true;
}

void ConfigBox::Hide()
{
	DialogBox::Hide();
	emu->set_pause(1, false);
}

bool ConfigBox::SetData()
{
	int val = 0;

	pConfig->use_power_off = (get_check_state(chkPowerOff));
#if defined(_MBS1)
	val = emu->get_parami(VM::ParamSysMode); 
	val = (get_radio_state_idx(radSYS,2) ? val & ~1 : val | 1);
	emu->set_parami(VM::ParamSysMode, val);
#endif
	if (get_check_state(chkMODE)) {
		pConfig->dipswitch |= 4;
	} else {
		pConfig->dipswitch &= ~4;
	}
	emu->set_parami(VM::ParamFddType, get_radio_state_idx(radFDD, 4));
	emu->set_parami(VM::ParamIOPort, get_check_state_num(chkIO, IOPORT_NUMS));
#ifndef USE_IOPORT_FDD
	int fdd_type = emu->get_parami(VM::ParamFddType);
	val = emu->get_parami(VM::ParamIOPort);
	val &= ~IOPORT_MSK_FDDALL;
	val |= (fdd_type == FDD_TYPE_3FDD ? IOPORT_MSK_3FDD
		 : (fdd_type == FDD_TYPE_5FDD
		 || fdd_type == FDD_TYPE_58FDD ? IOPORT_MSK_5FDD : 0));
	emu->set_parami(VM::ParamIOPort, val);
#endif

#ifdef USE_FD1
	pConfig->mount_fdd = get_check_state_num(chkFDMount, USE_FLOPPY_DISKS);
	pConfig->option_fdd = (get_check_state(chkDelayFd1) ? MSK_DELAY_FDSEARCH : 0)
		| (get_check_state(chkDelayFd2) ? MSK_DELAY_FDSEEK : 0)
		| (get_check_state(chkFdDensity) ? 0 : MSK_CHECK_FDDENSITY)
		| (get_check_state(chkFdMedia) ? 0 : MSK_CHECK_FDMEDIA)
		| (get_check_state(chkFdSavePlain) ? MSK_SAVE_FDPLAIN : 0);
#endif

#ifdef USE_HD1
	pConfig->mount_hdd = get_check_state_num(chkHDMount, USE_HARD_DISKS);

	pConfig->option_hdd = (get_check_state(chkDelayHd2) ? MSK_DELAY_HDSEEK : 0);
#endif

#ifdef USE_OPENGL
	pConfig->use_opengl = (uint8_t)get_combo_sel_num(comUseOpenGL);
	pConfig->gl_filter_type = (uint8_t)get_combo_sel_num(comGLFilter);
#endif

#if defined(_MBS1)
	pConfig->disptmg_skew = get_combo_sel_num(comDispSkew) - 2;
	pConfig->curdisp_skew = get_combo_sel_num(comCurdSkew) - 2;
#else
	pConfig->disptmg_skew = get_combo_sel_num(comDispSkew);
	pConfig->curdisp_skew = get_combo_sel_num(comCurdSkew);
#endif

	int led_show = get_combo_sel_num(comLEDShow);
	pConfig->led_pos = get_combo_sel_num(comLEDPos);

	pConfig->capture_type = get_combo_sel_num(comCapType);

	pConfig->snapshot_path.Set(get_text(txtSnapPath));
	pConfig->font_path.Set(get_text(txtFontPath));
	pConfig->msgboard_msg_fontname.Set(get_text(txtMsgFontName));
	pConfig->msgboard_info_fontname.Set(get_text(txtInfoFontName));
	val = 0;
	if (sscanf(get_text(txtMsgFontSize), "%d", &val) == 1 && val >= 6 && val <= 60) {
		pConfig->msgboard_msg_fontsize = val;
	}
	val = 0;
	if (sscanf(get_text(txtInfoFontSize), "%d", &val) == 1 && val >= 6 && val <= 60) {
		pConfig->msgboard_info_fontsize = val;
	}

	val = get_combo_sel_num(comLanguage);
	clocale->ChooseLocaleName(lang_list, val, pConfig->language);

#ifdef USE_DATAREC
	pConfig->wav_reverse = get_check_state(chkReverseWave);
	pConfig->wav_half = get_check_state(chkHalfWave);
	pConfig->wav_correct_type = (uint8_t)get_radio_state_idx(radCorrectType, 3);
	pConfig->wav_correct = (pConfig->wav_correct_type > 0);
	if (pConfig->wav_correct_type > 0) pConfig->wav_correct_type--;

	for(int i=0; i<2; i++) {
		val = 0;
		if (sscanf(get_text(txtCorrectAmp[i]), "%d", &val) == 1 && val >= 100 && val <= 5000) {
			pConfig->wav_correct_amp[i] = val;
		}
	}

	pConfig->wav_sample_rate = (uint8_t)get_combo_sel_num(comSampleRate);
	pConfig->wav_sample_bits = (uint8_t)get_combo_sel_num(comSampleBits);
#endif
#ifdef MAX_PRINTER
	for(int i=0; i<MAX_PRINTER; i++) {
		pConfig->printer_server_host[i].Set(get_text(txtLPTHost[i]));
		val = 0;
		if (sscanf(get_text(txtLPTPort[i]), "%d", &val) == 1 && val > 0 && val < 65535) {
			pConfig->printer_server_port[i] = val;
		}
		double valued = 0.0;
		valued = strtod(get_text(txtLPTDelay[i]), NULL);
		if (valued < 0.1) valued = 0.1;
		if (valued > 1000.0) valued = 1000.0;
		valued = floor(valued * 10.0 + 0.5) / 10.0;
		pConfig->printer_delay[i] = valued;
	}
#endif
#ifdef MAX_COMM
	for(int i=0; i<MAX_COMM; i++) {
		pConfig->comm_server_host[i].Set(get_text(txtCOMHost[i]));
		val = 0;
		if (sscanf(get_text(txtCOMPort[i]), "%d", &val) == 1 && val > 0 && val < 65535) {
			pConfig->comm_server_port[i] = val;
		}
		pConfig->comm_dipswitch[i] = get_combo_sel_num(comCOMBaud[i]);
	}
#endif
#ifdef USE_DEBUGGER
	pConfig->debugger_server_host.Set(get_text(txtDbgrHost));
	val = 0;
	if (sscanf(get_text(txtDbgrPort), "%d", &val) == 1 && val > 0 && val < 65535) {
		pConfig->debugger_server_port = val;
	}
#endif
	int uart_baud_index = get_combo_sel_num(comCOMUartBaud);
	pConfig->comm_uart_baudrate = (int)strtol(LABELS::comm_uart_baudrate[uart_baud_index], NULL, 10);
	pConfig->comm_uart_databit = get_combo_sel_num(comCOMUartDataBit) + 7;
	pConfig->comm_uart_parity = get_combo_sel_num(comCOMUartParity);
	pConfig->comm_uart_stopbit = get_combo_sel_num(comCOMUartStopBit) + 1;
	pConfig->comm_uart_flowctrl = get_combo_sel_num(comCOMUartFlowCtrl);

	pConfig->rom_path.Set(get_text(txtROMPath));
	BIT_ONOFF(pConfig->misc_flags, MSK_SHOWMSG_UNDEFOP, get_check_state(chkUndefOp));
	BIT_ONOFF(pConfig->misc_flags, MSK_CLEAR_CPUREG, get_check_state(chkClrCPUReg));

#if defined(_MBS1)
	int exram_num = get_combo_sel_num(comExRam);
	emu->set_parami(VM::ParamExMemNum, exram_num);
	pConfig->mem_nowait = get_check_state(chkMemNoWait);

//	pConfig->opn_clock = get_combo_sel_num(comFmOpnClk);
	pConfig->opn_irq = get_combo_sel_num(comFmOpnIrq);
	emu->set_parami(VM::ParamChipTypeOnFmOpn, get_combo_sel_num(comFmOpnChip));
	emu->set_parami(VM::ParamChipTypeOnExPsg, get_combo_sel_num(comExPsgChip));
#if defined(USE_Z80B_CARD)
	pConfig->z80b_card_out_irq = get_combo_sel_num(comZ80CardIrq);
#elif defined(USE_MPC_68008)
	BIT_ONOFF(pConfig->misc_flags, MSK_SHOWMSG_ADDRERR, get_check_state(chkAddrErr));
#endif
#else
	pConfig->exram_size_num = get_check_state(chkExMem);
#endif

	// set message font
	MsgBoard *msgboard = emu->get_msgboard();
	if (msgboard) {
		msgboard->SetFont();
	}

	gui->ChangeLedBox(led_show);
	gui->ChangeLedBoxPosition(pConfig->led_pos);
	pConfig->save();
#ifdef USE_OPENGL
	emu->change_opengl_attr();
#endif
	emu->update_config();

	return true;
}

void ConfigBox::ShowFolderBox(const char *title, GtkWidget *entry)
{
	FolderBox fbox(dialog);
	const char *path = gtk_entry_get_text(GTK_ENTRY(entry));
	if (fbox.Show(title, path)) {
		gtk_entry_set_text(GTK_ENTRY(entry),fbox.GetPath());
	}
}

void ConfigBox::ShowFontFileBox(const char *title, GtkWidget *entry)
{
	FileBox fbox;
	const CMsg::Id filter[] = {
			CMsg::Supported_Files_ttf_otf,
			CMsg::All_Files_,
			CMsg::End
	};
	const char *path = gtk_entry_get_text(GTK_ENTRY(entry));
	const char *dir = "";
//	const char *ext = ".ttf";
	if (fbox.Show(dialog, filter, title, dir, false, path)) {
		gtk_entry_set_text(GTK_ENTRY(entry),fbox.GetPath());
	}
}

void ConfigBox::ChangeFDDType(int index)
{
#ifdef USE_IOPORT_FDD
//	int index = get_radio_state_idx(radFDD,3);
//	g_print("ChangeFDDType: %d\n", index);
	if (chkIO[0] == NULL) return;
	if (index == 0) {
		set_check_state(chkIO[0],false);
		set_check_state(chkIO[1],false);
		gtk_widget_set_sensitive(chkIO[0], true);
		gtk_widget_set_sensitive(chkIO[1], true);
	} else if (index == 1) {
		set_check_state(chkIO[0],false);
		set_check_state(chkIO[1],true);
		gtk_widget_set_sensitive(chkIO[0], false);
		gtk_widget_set_sensitive(chkIO[1], false);
	} else if (index == 2) {
		set_check_state(chkIO[0],true);
		set_check_state(chkIO[1],false);
		gtk_widget_set_sensitive(chkIO[0], false);
		gtk_widget_set_sensitive(chkIO[1], false);
	}
#endif
}

void ConfigBox::ChangeIOPort(int index)
{
//	g_print("ChangeIOPort: %d\n", index);
	if (chkIO[2] == NULL) return;
	bool status = get_check_state(chkIO[index]);
#ifdef USE_IOPORT_FDD
	if (index == 0 && status) {
		set_check_state(chkIO[index+1],false);
	} else if (index == 1 && status) {
		set_check_state(chkIO[index-1],false);
	}
#endif
#if defined(_MBS1)
	if (index == IOPORT_POS_PSG9 && status) {
		set_check_state(chkIO[IOPORT_POS_KANJI],false);
		set_check_state(chkIO[IOPORT_POS_CM01],false);
		set_check_state(chkIO[IOPORT_POS_KANJI2],false);
	} else if (index == IOPORT_POS_KANJI && status) {
		set_check_state(chkIO[IOPORT_POS_PSG9],false);
	} else if (index == IOPORT_POS_CM01 && status) {
		set_check_state(chkIO[IOPORT_POS_PSG9],false);
		set_check_state(chkIO[IOPORT_POS_KANJI],true);
	} else if (index == IOPORT_POS_KANJI2 && status) {
		set_check_state(chkIO[IOPORT_POS_PSG9],false);
	} else if (index == IOPORT_POS_EXPSG) {
		set_check_state(chkExPsgEn, status);
	} else if (index == IOPORT_POS_FMOPN) {
		set_check_state(chkFmOpnEn, status);
	}
#else
	if (index == IOPORT_POS_PSG9 && status) {
		set_check_state(chkIO[IOPORT_POS_KANJI],false);
	} else if (index == IOPORT_POS_KANJI && status) {
		set_check_state(chkIO[IOPORT_POS_PSG9],false);
	}
#endif
}

void ConfigBox::ChangeFmOpn()
{
#if defined(_MBS1)
	set_check_state(chkIO[IOPORT_POS_FMOPN], get_check_state(chkFmOpnEn));
#endif
}

void ConfigBox::ChangeExPsg()
{
#if defined(_MBS1)
	set_check_state(chkIO[IOPORT_POS_EXPSG], get_check_state(chkExPsgEn));
#endif
}

void ConfigBox::OnChangedFDD(GtkWidget *widget, gpointer user_data)
{
	ConfigBox *obj = (ConfigBox *)user_data;
	int index = (int)(intptr_t)g_object_get_data(G_OBJECT(widget), "num");
	obj->ChangeFDDType(index);
}

void ConfigBox::OnChangedIO(GtkWidget *widget, gpointer user_data)
{
	ConfigBox *obj = (ConfigBox *)user_data;
	int index = (int)(intptr_t)g_object_get_data(G_OBJECT(widget), "num");
	obj->ChangeIOPort(index);
}

void ConfigBox::OnChangedFmOpn(GtkWidget *widget, gpointer user_data)
{
	ConfigBox *obj = (ConfigBox *)user_data;
	obj->ChangeFmOpn();
}

void ConfigBox::OnChangedExPsg(GtkWidget *widget, gpointer user_data)
{
	ConfigBox *obj = (ConfigBox *)user_data;
	obj->ChangeExPsg();
}

void ConfigBox::OnSelectSnapPath(GtkWidget *widget, gpointer user_data)
{
	ConfigBox *obj = (ConfigBox *)user_data;
	obj->ShowFolderBox(CMSG(Snapshot_Path), obj->txtSnapPath);
}

void ConfigBox::OnSelectFontPath(GtkWidget *widget, gpointer user_data)
{
	ConfigBox *obj = (ConfigBox *)user_data;
	obj->ShowFolderBox(CMSG(Font_Path), obj->txtFontPath);
}

void ConfigBox::OnSelectMessageFont(GtkWidget *widget, gpointer user_data)
{
	ConfigBox *obj = (ConfigBox *)user_data;
	obj->ShowFontFileBox(CMSG(Message_Font), obj->txtMsgFontName);
}

void ConfigBox::OnSelectInfoFont(GtkWidget *widget, gpointer user_data)
{
	ConfigBox *obj = (ConfigBox *)user_data;
	obj->ShowFontFileBox(CMSG(Info_Font), obj->txtInfoFontName);
}

void ConfigBox::OnSelectROMPath(GtkWidget *widget, gpointer user_data)
{
	ConfigBox *obj = (ConfigBox *)user_data;
	obj->ShowFolderBox(CMSG(ROM_Path), obj->txtROMPath);
}

void ConfigBox::OnResponse(GtkWidget *widget, gint response_id, gpointer user_data)
{
	ConfigBox *obj = (ConfigBox *)user_data;
	if (response_id == GTK_RESPONSE_ACCEPT) {
		obj->SetData();
	}
//	g_print("OnResponse: %d\n",response_id);
	obj->Hide();
}

}; /* namespace GUI_GTK_X11 */


