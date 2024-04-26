/** @file ag_config_dlg.cpp

	Skelton for retropc emulator
	SDL + Ager edition

	@author Sasaji
	@date   2012.04.08

	@brief [ ag_config_dlg ]
*/

#include "ag_config_dlg.h"
#include <agar/core/types.h>
#include "../gui.h"
//#include "../../config.h"
#include "ag_gui_base.h"
//#include "../../clocale.h"
#include "../../msgboard.h"
#include "../../clocale.h"
#include "../../utility.h"
#include "../../labels.h"
#include <math.h>

namespace GUI_AGAR
{

#ifdef USE_IOPORT_FDD
#define IOPORT_STARTNUM 0
#else
#define IOPORT_STARTNUM 2
#endif

#if defined(_MBS1)
#define CONFIG_TABS	6
#else
#define CONFIG_TABS	5
#endif

AG_CONFIG_DLG::AG_CONFIG_DLG(EMU *parent_emu, AG_GUI_BASE *parent_gui) : AG_DLG(parent_emu, parent_gui)
{
	memset(&param, 0, sizeof(param));
	param.fdd_type  = pConfig->fdd_type;
	param.io_port = pConfig->io_port;
#if defined(_MBS1)
	param.sys_mode_rev = (1 - (pConfig->sys_mode & 1));
#endif
	param.wav_reverse = 0;
	param.wav_half = 1;
	param.wav_correct = 0;

	param.wav_sample_rate = 3;
	param.wav_sample_bits = 0;
#if defined(_MBS1)
	param.disptmg_skew = pConfig->disptmg_skew + 2;
	param.curdisp_skew = pConfig->curdisp_skew + 2;
#else
	param.disptmg_skew = pConfig->disptmg_skew;
	param.curdisp_skew = pConfig->curdisp_skew;
#endif
	param.gl_filter_type = pConfig->gl_filter_type;
	param.led_show = 0;
	param.led_pos = pConfig->led_pos;
	param.capture_type = pConfig->capture_type;
#if defined(_MBS1)
	param.exram_num = pConfig->exram_size_num;
#endif

	paramtmp = NULL;

	dirdlg = new AG_DIR_DLG(parent_emu, parent_gui);
}

AG_CONFIG_DLG::~AG_CONFIG_DLG()
{
	delete dirdlg;
	delete paramtmp;
}

/**
 * create config dialog
 */
void AG_CONFIG_DLG::Create()
{
#if defined(_MBS1)
	param.disptmg_skew = pConfig->disptmg_skew + 2;
	param.curdisp_skew = pConfig->curdisp_skew + 2;
#else
	param.disptmg_skew = pConfig->disptmg_skew;
	param.curdisp_skew = pConfig->curdisp_skew;
#endif
	param.use_opengl = pConfig->use_opengl;
	param.gl_filter_type = pConfig->gl_filter_type;
	param.led_show = gui->GetLedBoxPhase(-1);
	param.led_pos = pConfig->led_pos;
	param.capture_type = pConfig->capture_type;
#if defined(_MBS1)
	param.sys_mode_rev = (1 - (emu->get_parami(VM::ParamSysMode) & 1));
#else
	param.exram_size_num = pConfig->exram_size_num;
#endif

	param.use_power_off = (pConfig->use_power_off ? 1 : 0);
	param.dipswitch = pConfig->dipswitch;
	param.fdd_type = emu->get_parami(VM::ParamFddType);
	param.mount_fdd = pConfig->mount_fdd;
	param.delayfd1 = FLG_DELAY_FDSEARCH ? 1 : 0;
	param.delayfd2 = FLG_DELAY_FDSEEK ? 1 : 0;
	param.chk_fddensity = FLG_CHECK_FDDENSITY ? 0 : 1;
	param.chk_fdmedia = FLG_CHECK_FDMEDIA ? 0 : 1;
	param.save_fdplain = FLG_SAVE_FDPLAIN ? 1 : 0;
	param.io_port = emu->get_parami(VM::ParamIOPort);
	param.wav_reverse = pConfig->wav_reverse ? 1 : 0;
	param.wav_half = pConfig->wav_half ? 1 : 0;
	param.wav_correct = pConfig->wav_correct ? pConfig->wav_correct_type + 1 : 0;
	sprintf(param.wav_correct_amp[0], "%d", pConfig->wav_correct_amp[0]);
	sprintf(param.wav_correct_amp[1], "%d", pConfig->wav_correct_amp[1]);
	param.wav_sample_rate = pConfig->wav_sample_rate;
	param.wav_sample_bits = pConfig->wav_sample_bits;

	memcpy(param.snap_path, pConfig->snapshot_path.Get(), _MAX_PATH);
	memcpy(param.rom_path, pConfig->rom_path.Get(), _MAX_PATH);
	memcpy(param.font_path, pConfig->font_path.Get(), _MAX_PATH);
	memcpy(param.msg_font_name, pConfig->msgboard_msg_fontname.Get(), _MAX_PATH);
	sprintf(param.msg_font_size, "%d", pConfig->msgboard_msg_fontsize);
	memcpy(param.info_font_name, pConfig->msgboard_info_fontname.Get(), _MAX_PATH);
	sprintf(param.info_font_size, "%d", pConfig->msgboard_info_fontsize);
	memcpy(param.menu_font_name, pConfig->menu_fontname.Get(), _MAX_PATH);
	sprintf(param.menu_font_size, "%d", pConfig->menu_fontsize);

	for(int i=0; i<MAX_PRINTER; i++) {
		memcpy(param.prn_host[i], pConfig->printer_server_host[i].Get(), _MAX_PATH);
		sprintf(param.prn_port[i], "%d", pConfig->printer_server_port[i]);
		sprintf(param.prn_delay[i], "%.1f", pConfig->printer_delay[i]);
	}
	for(int i=0; i<MAX_COMM; i++) {
		memcpy(param.com_host[i], pConfig->comm_server_host[i].Get(), _MAX_PATH);
		sprintf(param.com_port[i], "%d", pConfig->comm_server_port[i]);
		param.com_dipsw[i] = pConfig->comm_dipswitch[i];
	}
#ifdef USE_DEBUGGER
	memcpy(param.dbgr_host, pConfig->debugger_server_host.Get(), _MAX_PATH);
	sprintf(param.dbgr_port, "%d", pConfig->debugger_server_port);
#endif
	param.uart_baud_index = 0;
	for(int i=0; LABELS::comm_uart_baudrate[i] != NULL; i++) {
		int val = (int)strtol(LABELS::comm_uart_baudrate[i], NULL, 10);
		if (pConfig->comm_uart_baudrate == val) {
			param.uart_baud_index = i;
			break;
		}
	}
	param.uart_databit = pConfig->comm_uart_databit - 7;
	if (param.uart_databit < 0) param.uart_databit = 0;
	param.uart_parity = pConfig->comm_uart_parity;
	param.uart_stopbit = pConfig->comm_uart_stopbit - 1;
	param.uart_flowctrl = pConfig->comm_uart_flowctrl;

#if defined(_MBS1)
	param.exram_num = emu->get_parami(VM::ParamExMemNum);
	param.mem_nowait = pConfig->mem_nowait ? 1 : 0;

//	param.fmopn_clock_num = pConfig->opn_clock;
	param.fmopn_irq = pConfig->opn_irq;
	param.type_of_fmopn = emu->get_parami(VM::ParamChipTypeOnFmOpn);
	param.type_of_expsg = emu->get_parami(VM::ParamChipTypeOnExPsg);
# if defined(USE_Z80B_CARD)
	param.z80b_card_out_irq = pConfig->z80b_card_out_irq;
# elif defined(USE_MPC_68008)
	param.showmsg_addrerr = FLG_SHOWMSG_ADDRERR ? 1 : 0;
# endif
#endif
	param.showmsg_undefop = FLG_SHOWMSG_UNDEFOP ? 1 : 0;
	param.clear_cpureg = FLG_CLEAR_CPUREG ? 1 : 0;

	if (paramtmp == NULL) {
		paramtmp = new ConfigDlgParam;
	}
	*paramtmp = param;

	AG_Window *win = AG_WindowNew(AG_WINDOW_MODAL | AG_WINDOW_DIALOG | AG_WINDOW_NORESIZE);
	AG_WindowSetCaptionS(win, CMSG(Configure));

	AG_Box *vbox_base;
	AG_Box *hbox_base;
	AG_Box *vbox_left;
	AG_Box *vbox_right;
	AG_Box *vbox;
	AG_Box *hbox;
	AG_Radio *rad;
	AG_Checkbox *cbox;
	AG_Notebook *nb;
	AG_NotebookTab *tabs[CONFIG_TABS];

	int i;
	char name[128];

	nb = AG_NotebookNew(win, AG_NOTEBOOK_HFILL);

	//
	// 0 Mode
	tabs[0] = AG_NotebookAddTab(nb, CMSGV(LABELS::tabs[0]), AG_BOX_HORIZ);

	vbox_base = AG_BoxNewVert(tabs[0], AG_BOX_HFILL | AG_BOX_VFILL);
	hbox_base = AG_BoxNewHoriz(vbox_base, AG_BOX_HFILL | AG_BOX_HOMOGENOUS);

	// left
	vbox_left = AG_BoxNewVert(hbox_base, AG_BOX_VFILL);

#if defined(_MBS1)
	// System Mode
	vbox = AG_BoxNewVert(vbox_left, AG_BOX_FRAME | AG_BOX_HFILL);
	AG_LabelNewS(vbox, AG_LABEL_HFILL, CMSG(System_Mode_ASTERISK));

	set_label(radSysModeBase[0],CMSG(A_Mode_S1));
	set_label(radSysModeBase[1],CMSG(B_Mode_L3));
	for(i=0; i<2; i++) {
		radSysMode[i] = radSysModeBase[i];
		if (i == (1 - (pConfig->sys_mode & 1))) {
			radSysModeBase[i][0] = '>';
		}
	}
	radSysMode[2]=NULL;

	rad = AG_RadioNewInt(vbox, AG_RADIO_EXPAND, (const char **)radSysMode, &paramtmp->sys_mode_rev);

	strcpy(chkDipSwitchBase[0],CMSG(NEWON7));
	hbox = AG_BoxNewHoriz(vbox, AG_BOX_HFILL);
	cbox = AG_CheckboxNewFlag(hbox, 0, chkDipSwitchBase[0], (Uint *)&paramtmp->dipswitch, 0x04);
#else
	// Mode switch
	vbox = AG_BoxNewVert(vbox_left, AG_BOX_FRAME | AG_BOX_HFILL);
	AG_LabelNewS(vbox, AG_LABEL_HFILL, CMSG(DIP_Switch_ASTERISK));

	set_label(chkDipSwitchBase[0],CMSG(MODE_Switch));
	if (pConfig->dipswitch & 0x04) {
		chkDipSwitchBase[0][0] = '>';
	}
	cbox = AG_CheckboxNewFlag(vbox, 0, chkDipSwitchBase[0], (Uint *)&paramtmp->dipswitch, 0x04);
#endif

	// FDD type
	vbox = AG_BoxNewVert(vbox_left, AG_BOX_FRAME | AG_BOX_HFILL);
	AG_LabelNewS(vbox, AG_LABEL_HFILL, CMSG(FDD_Type_ASTERISK));

	for(i=0; i<4; i++) {
		set_label(radFddTypeBase[i], LABELS::fdd_type[i]);
	}
	for(i=0; i<4; i++) {
		radFddType[i] = radFddTypeBase[i];
		if (i == pConfig->fdd_type) {
			radFddTypeBase[i][0] = '>';
		}
	}
	radFddType[4]=NULL;

	rad = AG_RadioNewInt(vbox, AG_RADIO_EXPAND, (const char **)radFddType, &paramtmp->fdd_type);
	AG_AddEvent(rad, "radio-changed", OnChangeFddType, "%Cp", this);

	AG_CheckboxNewInt(vbox_left, 0, CMSG(Enable_the_state_of_power_off), &paramtmp->use_power_off);

	// right
	vbox_right = AG_BoxNewVert(hbox_base, AG_BOX_VFILL);

	// I/O Port address
	vbox = AG_BoxNewVert(vbox_right, AG_BOX_FRAME | AG_BOX_HFILL);
	AG_LabelNewS(vbox, AG_LABEL_HFILL, CMSG(I_O_Port_Address_ASTERISK));

	for(i=0; i<IOPORT_NUMS; i++) {
		chkIOPort[i] = NULL;
		chkIOPortBase[i][0]=0;
	}
	for(i=0; LABELS::io_port[i] != CMsg::End; i++) {
		int pos = LABELS::io_port_pos[i];
		if ((1 << pos) & IOPORT_MSK_ALL) {
			set_label(chkIOPortBase[pos], LABELS::io_port[i]);
		}
	}
	for(i=0; LABELS::io_port[i] != CMsg::End; i++) {
		int pos = LABELS::io_port_pos[i];
		if ((1 << pos) & IOPORT_MSK_ALL) {
			if (pConfig->io_port & (1 << pos)) {
				chkIOPortBase[pos][0] = '>';
			}
			cbox = AG_CheckboxNewFlag(vbox, 0, chkIOPortBase[pos], (Uint *)&paramtmp->io_port, (1 << pos));
			AG_AddEvent(cbox, "checkbox-changed", OnChangeIOPort, "%Cp %i", this, pos);
			chkIOPort[pos] = cbox;
		} else {
			chkIOPort[pos] = NULL;
		}
	}

	//
	AG_LabelNewS(vbox_base, AG_LABEL_HFILL, CMSG(Need_restart_program_or_PowerOn));


	//
	// 1 screen
	tabs[1] = AG_NotebookAddTab(nb, CMSGV(LABELS::tabs[1]), AG_BOX_HORIZ);

	vbox_base = AG_BoxNewVert(tabs[1], AG_BOX_EXPAND);
	hbox_base = AG_BoxNewHoriz(vbox_base, AG_BOX_HFILL | AG_BOX_HOMOGENOUS);

#ifdef USE_OPENGL
	// GL Filter Type
	vbox = AG_BoxNewVert(hbox_base, AG_BOX_FRAME);
	AG_LabelNewS(vbox, AG_LABEL_HFILL, CMSG(Drawing));

	hbox = AG_BoxNewHoriz(vbox, AG_BOX_HFILL);
	AG_LabelNewS(hbox, 0, CMSG(Method_ASTERISK));
	UComboNew(hbox, LABELS::opengl_use, pConfig->use_opengl, OnChangeUseOpenGL);

	hbox = AG_BoxNewHoriz(vbox, AG_BOX_HFILL);
	AG_LabelNewS(hbox, 0, CMSG(Filter_Type));
	UComboNew(hbox, LABELS::opengl_filter, pConfig->gl_filter_type, OnChangeGLFilterType);
#endif

	// CRTC
	vbox = AG_BoxNewVert(hbox_base, AG_BOX_FRAME);
	AG_LabelNewS(vbox, AG_LABEL_HFILL, CMSG(CRTC));

	hbox = AG_BoxNewHoriz(vbox, AG_BOX_HFILL);
	AG_LabelNewS(hbox, 0, CMSG(Disptmg_Skew));
#if defined(_MBS1)
	UComboNew(hbox, LABELS::disp_skew, pConfig->disptmg_skew + 2, OnChangeDisptmgSkew);
#else
	UComboNew(hbox, LABELS::disp_skew, pConfig->disptmg_skew, OnChangeDisptmgSkew);
#endif

	hbox = AG_BoxNewHoriz(vbox, AG_BOX_HFILL);
#if defined(_MBS1)
	AG_LabelNewS(hbox, 0, CMSG(Curdisp_Skew_L3));
	UComboNew(hbox, LABELS::disp_skew, pConfig->curdisp_skew + 2, OnChangeCurdispSkew);
#else
	AG_LabelNewS(hbox, 0, CMSG(Curdisp_Skew));
	UComboNew(hbox, LABELS::disp_skew, pConfig->curdisp_skew, OnChangeCurdispSkew);
#endif

	// LED
	hbox_base = AG_BoxNewHoriz(vbox_base, AG_BOX_HFILL | AG_BOX_HOMOGENOUS);

	vbox = AG_BoxNewVert(hbox_base, 0);
	hbox = AG_BoxNewHoriz(vbox, AG_BOX_HFILL);
	AG_LabelNewS(hbox, 0, CMSG(LED));
	UComboNew(hbox, LABELS::led_show, paramtmp->led_show, OnChangeLedShow);

	vbox = AG_BoxNewVert(hbox_base, 0);
	hbox = AG_BoxNewHoriz(vbox, AG_BOX_HFILL);
	AG_LabelNewS(hbox, 0, CMSG(Position));
	UComboNew(hbox, LABELS::led_pos, pConfig->led_pos, OnChangeLedPos);

	// capture type
	hbox = AG_BoxNewHoriz(vbox_base, AG_BOX_HFILL);
	AG_LabelNewS(hbox, 0, CMSG(Capture_Type));
	UComboNew(hbox, LABELS::capture_fmt, pConfig->capture_type, OnChangeCaptureType);

	// snapshot path
	hbox = AG_BoxNewHoriz(vbox_base, AG_BOX_HFILL);
	TextboxNew(hbox, CMSG(Snapshot_Path), 40, paramtmp->snap_path, sizeof(paramtmp->snap_path)-1);
	AG_ButtonNewFn(hbox, 0, CMSG(Folder_), OnShowDirDlg, "%Cp %Cp %i", this, win, SnapShotPath);

	// font path
	hbox = AG_BoxNewHoriz(vbox_base, AG_BOX_HFILL);
	TextboxNew(hbox, CMSG(Font_Path), 40, paramtmp->font_path, sizeof(paramtmp->font_path)-1);
	AG_ButtonNewFn(hbox, 0, CMSG(Folder_), OnShowDirDlg, "%Cp %Cp %i", this, win, FontPath);

	// msgboard font
	hbox = AG_BoxNewHoriz(vbox_base, AG_BOX_HFILL);
	TextboxNew(hbox, CMSG(Message_Font), 16, paramtmp->msg_font_name, sizeof(paramtmp->msg_font_name)-1);
	TextboxNew(hbox, CMSG(_Size), 4, paramtmp->msg_font_size, sizeof(paramtmp->msg_font_size)-1);

	hbox = AG_BoxNewHoriz(vbox_base, AG_BOX_HFILL);
	TextboxNew(hbox, CMSG(Info_Font), 16, paramtmp->info_font_name, sizeof(paramtmp->info_font_name)-1);
	TextboxNew(hbox, CMSG(_Size), 4, paramtmp->info_font_size, sizeof(paramtmp->info_font_size)-1);

	// menu font
	hbox = AG_BoxNewHoriz(vbox_base, AG_BOX_HFILL);
	TextboxNew(hbox, CMSG(Menu_Font_ASTERISK), 16, paramtmp->menu_font_name, sizeof(paramtmp->menu_font_name)-1);
	TextboxNew(hbox, CMSG(_Size), 4, paramtmp->menu_font_size, sizeof(paramtmp->menu_font_size)-1);

	// language
	lang_list.Clear();
	clocale->GetLocaleNamesWithDefault(lang_list);

	paramtmp->lang_selidx = clocale->SelectLocaleNameIndex(lang_list, pConfig->language);

	hbox = AG_BoxNewHoriz(vbox_base, AG_BOX_HFILL);
	AG_LabelNewS(hbox, 0, CMSG(Language_ASTERISK));
	UComboNew(hbox, lang_list, paramtmp->lang_selidx, OnChangeLanguage);

	//
	AG_LabelNewS(vbox_base, AG_LABEL_HFILL, CMSG(Need_restart_program));

	//
	// 2 tape
	tabs[2] = AG_NotebookAddTab(nb, CMSGV(LABELS::tabs[2]), AG_BOX_HORIZ);

	vbox_base = AG_BoxNewVert(tabs[2], AG_BOX_EXPAND);
	hbox_base = AG_BoxNewHoriz(vbox_base, AG_BOX_HFILL | AG_BOX_HOMOGENOUS);

	// Load wav file
	vbox = AG_BoxNewVert(hbox_base, AG_BOX_FRAME | AG_BOX_HFILL);
	AG_LabelNewS(vbox, AG_LABEL_HFILL, CMSG(Load_Wav_File_from_Tape));

	AG_CheckboxNewInt(vbox, 0, CMSG(Reverse_Wave), &paramtmp->wav_reverse);
	AG_CheckboxNewInt(vbox, 0, CMSG(Half_Wave), &paramtmp->wav_half);
	hbox = AG_BoxNewHoriz(vbox, AG_BOX_HFILL);
	AG_LabelNewS(hbox, 0, CMSG(Correct));

	rad = RadioNewInt(hbox, AG_RADIO_EXPAND, LABELS::correct, &paramtmp->wav_correct);

	hbox = AG_BoxNewHoriz(vbox, AG_BOX_HFILL);
	TextboxNew(hbox, "1200Hz", 6, paramtmp->wav_correct_amp[0], sizeof(paramtmp->wav_correct_amp[0])-1);
	TextboxNew(hbox, "2400Hz", 6, paramtmp->wav_correct_amp[1], sizeof(paramtmp->wav_correct_amp[1])-1);

//	AG_AddEvent(cbox, "checkbox-changed", OnChangeCorrect, "%Cp %Cp", this, rad);
//	if (paramtmp->wav_correct == 0) {
//		AG_WidgetDisable(rad);
//	} else {
//		AG_WidgetEnable(rad);
//	}

	// Save wav file
	vbox = AG_BoxNewVert(hbox_base, AG_BOX_FRAME | AG_BOX_HFILL);
	AG_LabelNewS(vbox, AG_LABEL_HFILL, CMSG(Save_Wav_File_to_Tape));

	hbox = AG_BoxNewHoriz(vbox, AG_BOX_HFILL);
	AG_LabelNewS(hbox, 0, CMSG(Sample_Rate));
	UComboNew(hbox, LABELS::wav_sampling_rate, paramtmp->wav_sample_rate, OnChangeSampleRate);

	hbox = AG_BoxNewHoriz(vbox, AG_BOX_HFILL);
	AG_LabelNewS(hbox, 0, CMSG(Sample_Bits));
	UComboNew(hbox, LABELS::wav_sampling_bits, paramtmp->wav_sample_bits, OnChangeSampleBits);

	// FDD
	vbox = AG_BoxNewVert(vbox_base, AG_BOX_FRAME | AG_BOX_HFILL);
	AG_LabelNewS(vbox, AG_LABEL_HFILL, CMSG(Floppy_Disk_Drive));

	// fdd mount
	hbox = AG_BoxNewHoriz(vbox, AG_BOX_HFILL);
	AG_LabelNewS(hbox, 0, CMSG(When_start_up_mount_disk_at_));
	for(i=0; i<USE_FLOPPY_DISKS; i++) {
		sprintf(name, "%d  ", i);
		AG_CheckboxNewFlag(hbox, 0, name, (Uint *)&paramtmp->mount_fdd, (1 << i));
	}
	// delay
	AG_CheckboxNewInt(vbox, 0, CMSG(Ignore_delays_to_find_sector), &paramtmp->delayfd1);
	AG_CheckboxNewInt(vbox, 0, CMSG(Ignore_delays_to_seek_track), &paramtmp->delayfd2);
	// density media
	AG_CheckboxNewInt(vbox, 0, CMSG(Suppress_checking_for_density), &paramtmp->chk_fddensity);
	AG_CheckboxNewInt(vbox, 0, CMSG(Suppress_checking_for_media_type), &paramtmp->chk_fdmedia);

	AG_CheckboxNewInt(vbox, 0, CMSG(Save_a_plain_disk_image_as_it_is), &paramtmp->save_fdplain);

	//
	// 3 network
	tabs[3] = AG_NotebookAddTab(nb, CMSGV(LABELS::tabs[3]), AG_BOX_HORIZ);

	vbox_base = AG_BoxNewVert(tabs[3], AG_BOX_EXPAND);

	// LPT
	for(i=0; i<MAX_PRINTER; i++) {
		hbox = AG_BoxNewHoriz(vbox_base, AG_BOX_HFILL);
		sprintf(name, CMSG(LPTVDIGIT_Hostname), i);
		TextboxNew(hbox, name, 15, paramtmp->prn_host[i], sizeof(paramtmp->prn_host[i])-1);
		TextboxNew(hbox, CMSG(_Port), 6, paramtmp->prn_port[i], sizeof(paramtmp->prn_port[i])-1);
		TextboxNew(hbox, CMSG(_Print_delay), 6, paramtmp->prn_delay[i], sizeof(paramtmp->prn_delay[i])-1);
		LabelNew(hbox, CMSG(msec));
	}
	// COM
	for(i=0; i<MAX_COMM; i++) {
		hbox = AG_BoxNewHoriz(vbox_base, AG_BOX_HFILL);
		sprintf(name, CMSG(COMVDIGIT_Hostname), i);
		TextboxNew(hbox, name, 15, paramtmp->com_host[i], sizeof(paramtmp->com_host[i])-1);
		TextboxNew(hbox, CMSG(_Port), 6, paramtmp->com_port[i], sizeof(paramtmp->com_port[i])-1);

		UComboNew(hbox, LABELS::comm_baud, paramtmp->com_dipsw[i]-1, OnChangeCommBaud, i);
	}
#ifdef USE_DEBUGGER
	hbox = AG_BoxNewHoriz(vbox_base, AG_BOX_HFILL);
	AG_LabelNewS(hbox, 0, CMSG(Connectable_host_to_Debugger));
	hbox = AG_BoxNewHoriz(vbox_base, AG_BOX_HFILL);
	TextboxNew(hbox, _T(" "), 15, paramtmp->dbgr_host, sizeof(paramtmp->dbgr_host)-1);
	TextboxNew(hbox, CMSG(_Port), 6, paramtmp->dbgr_port, sizeof(paramtmp->dbgr_port)-1);
#endif
	// uart
	vbox = AG_BoxNewVert(vbox_base, AG_BOX_FRAME | AG_BOX_HFILL);
	AG_LabelNewS(vbox, AG_LABEL_HFILL, CMSG(Settings_of_serial_ports_on_host));
	hbox = AG_BoxNewHoriz(vbox, AG_BOX_HFILL);
	AG_LabelNewS(hbox, 0, CMSG(Baud_Rate));
	UComboNew(hbox, LABELS::comm_uart_baudrate, paramtmp->uart_baud_index, OnChangeCommUartBaud);
	hbox = AG_BoxNewHoriz(vbox, AG_BOX_HFILL);
	AG_LabelNewS(hbox, 0, CMSG(Data_Bit));
	UComboNew(hbox, LABELS::comm_uart_databit, paramtmp->uart_databit, OnChangeCommUartDataBit);
	hbox = AG_BoxNewHoriz(vbox, AG_BOX_HFILL);
	AG_LabelNewS(hbox, 0, CMSG(Parity));
	UComboNew(hbox, LABELS::comm_uart_parity, paramtmp->uart_parity, OnChangeCommUartParity);
	hbox = AG_BoxNewHoriz(vbox, AG_BOX_HFILL);
	AG_LabelNewS(hbox, 0, CMSG(Stop_Bit));
	UComboNew(hbox, LABELS::comm_uart_stopbit, paramtmp->uart_stopbit, OnChangeCommUartStopBit);
	hbox = AG_BoxNewHoriz(vbox, AG_BOX_HFILL);
	AG_LabelNewS(hbox, 0, CMSG(Flow_Control));
	UComboNew(hbox, LABELS::comm_uart_flowctrl, paramtmp->uart_flowctrl, OnChangeCommUartFlowCtrl);
	AG_LabelNewS(vbox, 0, CMSG(Need_re_connect_to_serial_port_when_modified_this));

	//
	// 4 CPU, Memory
	tabs[4] = AG_NotebookAddTab(nb, CMSGV(LABELS::tabs[4]), AG_BOX_HORIZ);

	vbox_base = AG_BoxNewVert(tabs[4], AG_BOX_EXPAND);

	hbox = AG_BoxNewHoriz(vbox_base, AG_BOX_HFILL);
	TextboxNew(hbox, CMSG(ROM_Path_ASTERISK), 40, paramtmp->rom_path, sizeof(paramtmp->rom_path)-1);
	AG_ButtonNewFn(hbox, 0, CMSG(Folder_), OnShowDirDlg, "%Cp %Cp %i", this, win, RomPath);
#if defined(_MBS1)
	// extended memory
	hbox = AG_BoxNewHoriz(vbox_base, AG_BOX_HFILL);
	AG_LabelNewS(hbox, 0, CMSG(Extended_RAM_ASTERISK));
	UComboNew(hbox, LABELS::exram_size, paramtmp->exram_num, OnChangeExtRam);
	if (0 <= pConfig->exram_size_num && pConfig->exram_size_num <= 4) {
		_TCHAR str[100];
		UTILITY::tcscpy(str, sizeof(str), CMSG(LB_Now_SP));
		UTILITY::tcscat(str, sizeof(str), CMSGV(LABELS::exram_size[pConfig->exram_size_num]));
		UTILITY::tcscat(str, sizeof(str), _T(")"));
		AG_LabelNewS(hbox, 0, str);
	}
	// memory wait
	AG_CheckboxNewInt(vbox_base, 0, CMSG(No_wait_to_access_the_main_memory), &paramtmp->mem_nowait);
#else
	// extended memory
	AG_CheckboxNewFlag(vbox_base, 0, CMSG(Use_Extended_Memory_64KB), (Uint *)&paramtmp->exram_size_num, 1);
#endif
	//
	AG_CheckboxNewInt(vbox_base, 0, CMSG(Show_message_when_the_CPU_fetches_undefined_opcode), &paramtmp->showmsg_undefop);

#if defined(_MBS1)
# if defined(USE_Z80B_CARD)
	// Z80B Card
	hbox = AG_BoxNewHoriz(vbox_base, AG_BOX_HFILL);
	AG_LabelNewS(hbox, 0, CMSG(Connect_interrupt_signal_of_Z80B_Card_to_ASTERISK));
	UComboNew(hbox, LABELS::z80bcard_irq, paramtmp->z80b_card_out_irq, OnChangeZ80BCardIrq);
# elif defined(USE_MPC_68008)
	// MPC-68008 card
	AG_CheckboxNewInt(vbox_base, 0, CMSG(Show_message_when_the_address_error_occured_in_MC68008), &paramtmp->showmsg_addrerr);
# endif
#endif

	//
	AG_CheckboxNewInt(vbox_base, 0, CMSG(Clear_CPU_registers_at_power_on), &paramtmp->clear_cpureg);

	//
	AG_LabelNewS(vbox_base, AG_LABEL_HFILL, CMSG(Need_restart_program_or_PowerOn));

#if defined(_MBS1)
	//
	// 5 Sound
	tabs[5] = AG_NotebookAddTab(nb, CMSGV(LABELS::tabs[5]), AG_BOX_HORIZ);

	vbox_base = AG_BoxNewVert(tabs[5], AG_BOX_EXPAND);

	//
	vbox = AG_BoxNewVert(vbox_base, AG_BOX_FRAME);
	AG_LabelNewS(vbox, AG_LABEL_HFILL, CMSG(FM_Synthesis_Card_ASTERISK));
	AG_CheckboxNewFlag(vbox, 0, CMSG(Enable), (Uint *)&paramtmp->io_port, IOPORT_MSK_FMOPN);
	AG_LabelNewS(vbox, AG_LABEL_HFILL, CMSG(IO_ports_are_FF1E_FF1F_FF16_FF17));

//	hbox = AG_BoxNewHoriz(vbox, AG_BOX_HFILL);
//	AG_LabelNewS(hbox, 0, CMSG(Clock));
//	UComboNew(hbox, LABELS::fmopn_clock, paramtmp->fmopn_clock_num, OnChangeFmOpnClock);
//	AG_LabelNewS(hbox, 0, _T("Hz"));

	hbox = AG_BoxNewHoriz(vbox, AG_BOX_HFILL);
	AG_LabelNewS(hbox, 0, CMSG(Sound_chip));
	UComboNew(hbox, LABELS::type_of_soundcard, paramtmp->type_of_fmopn, OnChangeFmOpnChip, -1, pConfig->type_of_fmopn, CMsg::LB_Now_RB);

	//
	vbox = AG_BoxNewVert(vbox_base, AG_BOX_FRAME);
	AG_LabelNewS(vbox, AG_LABEL_HFILL, CMSG(Extended_PSG_Port_ASTERISK));
	AG_CheckboxNewFlag(vbox, 0, CMSG(Enable), (Uint *)&paramtmp->io_port, IOPORT_MSK_EXPSG);
	AG_LabelNewS(vbox, AG_LABEL_HFILL, CMSG(IO_ports_are_FFE6_FFE7_FFEE_FFEF));

	hbox = AG_BoxNewHoriz(vbox, AG_BOX_HFILL);
	AG_LabelNewS(hbox, 0, CMSG(Sound_chip));
	UComboNew(hbox, LABELS::type_of_soundcard, paramtmp->type_of_expsg, OnChangeExpsgChip, -1, pConfig->type_of_expsg, CMsg::LB_Now_RB);

	//
	hbox = AG_BoxNewHoriz(vbox_base, AG_BOX_HFILL);
	AG_LabelNewS(hbox, 0, CMSG(Connect_interrupt_signal_of_FM_synthesis_to_ASTERISK_ASTERISK));
	UComboNew(hbox, LABELS::fmopn_irq, paramtmp->fmopn_irq, OnChangeFmOpnIrq);

	//
	AG_LabelNewS(vbox_base, AG_LABEL_HFILL, CMSG(Need_restart_program_or_PowerOn));
	AG_LabelNewS(vbox_base, AG_LABEL_HFILL, CMSG(This_is_the_common_setting_both_FM_synthesis_card_and_extended_PSG_port));
#endif

	//
	// button
	hbox_base = AG_BoxNewHoriz(win, AG_BOX_EXPAND);
	AG_ButtonNewFn(hbox_base, 0, CMSG(OK), OnOk, "%Cp %Cp", this, win);
	AG_ButtonNewFn(hbox_base, 0, CMSG(Cancel), OnClose, "%Cp %Cp", this, win);

	AG_SetEvent(win, "window-close", OnClose, "%Cp %Cp", this, win);

	gui->PostEtSystemPause(true);
	AG_WindowShow(win);

	//
	change_fdd_type(paramtmp->fdd_type);
}

void AG_CONFIG_DLG::change_fdd_type(int index)
{
	if (index == 0) {
		paramtmp->io_port = paramtmp->io_port & ~IOPORT_MSK_FDDALL;
#ifdef USE_IOPORT_FDD
		AG_WidgetEnable(chkIOPort[0]);
		AG_WidgetEnable(chkIOPort[1]);
#endif
	} else if (index == 1) {
		paramtmp->io_port = (paramtmp->io_port & ~IOPORT_MSK_FDDALL) | IOPORT_MSK_3FDD;
#ifdef USE_IOPORT_FDD
		AG_WidgetDisable(chkIOPort[0]);
		AG_WidgetDisable(chkIOPort[1]);
#endif
	} else if (index == 2 || index == 3) {
		paramtmp->io_port = (paramtmp->io_port & ~IOPORT_MSK_FDDALL) | IOPORT_MSK_5FDD;
#ifdef USE_IOPORT_FDD
		AG_WidgetDisable(chkIOPort[0]);
		AG_WidgetDisable(chkIOPort[1]);
#endif
	}
}

void AG_CONFIG_DLG::change_io_port(int index, int status)
{
#ifdef USE_IOPORT_FDD
	if (index == 0 && status & (1 << 0)) {
		paramtmp->io_port &= ~(1 << 1);
	} else if (index == 1 && status & (1 << 1)) {
		paramtmp->io_port &= ~(1 << 0);
	}
#endif
	if (index == 5 && status & (1 << 5)) {
		paramtmp->io_port &= ~(1 << 6);
#if defined(_MBS1)
		paramtmp->io_port &= ~(1 << 9);
#endif
	} else if (index == 6 && status & (1 << 6)) {
		paramtmp->io_port &= ~(1 << 5);
#if defined(_MBS1)
	} else if (index == 9 && status & (1 << 9)) {
		paramtmp->io_port &= ~(1 << 5);
		paramtmp->io_port |= (1 << 6);
#endif
	}
}

void AG_CONFIG_DLG::set_label(char *label, const char *str)
{
	strcpy(label,"  ");
	strcat(label, str);
}

void AG_CONFIG_DLG::set_label(char *label, CMsg::Id msg_id)
{
	strcpy(label,"  ");
	strcat(label, gMessages.Get(msg_id));
}

void AG_CONFIG_DLG::Close(AG_Window *win)
{
	AG_ObjectDetach(win);
	emu->update_config();
	gui->PostEtSystemPause(false);
}

int AG_CONFIG_DLG::SetData(AG_Window *win)
{
	param = *paramtmp;

#if defined(_MBS1)
	pConfig->disptmg_skew = param.disptmg_skew - 2;
	pConfig->curdisp_skew = param.curdisp_skew - 2;
#else
	pConfig->disptmg_skew = param.disptmg_skew;
	pConfig->curdisp_skew = param.curdisp_skew;
	pConfig->exram_size_num = param.exram_size_num;
#endif
	pConfig->use_opengl = (uint8_t)param.use_opengl;
	pConfig->gl_filter_type = (uint8_t)param.gl_filter_type;
	pConfig->led_pos = param.led_pos;
	pConfig->capture_type = param.capture_type;

	pConfig->use_power_off = (param.use_power_off != 0);
	pConfig->dipswitch = (param.dipswitch & 0xff);

	emu->set_parami(VM::ParamFddType, param.fdd_type);
#ifndef USE_IOPORT_FDD
	param.io_port &= ~IOPORT_MSK_FDDALL;
	param.io_port |= (param.fdd_type == FDD_TYPE_3FDD ? IOPORT_MSK_3FDD
		: (param.fdd_type == FDD_TYPE_5FDD
		|| param.fdd_type == FDD_TYPE_58FDD ? IOPORT_MSK_5FDD : 0));
#endif
	pConfig->mount_fdd = param.mount_fdd;
	pConfig->option_fdd = (param.delayfd1 ? MSK_DELAY_FDSEARCH : 0)
		| (param.delayfd2 ? MSK_DELAY_FDSEEK : 0)
		| (param.chk_fddensity ? 0 : MSK_CHECK_FDDENSITY)
		| (param.chk_fdmedia ? 0 : MSK_CHECK_FDMEDIA)
		| (param.save_fdplain ? MSK_SAVE_FDPLAIN : 0);

	emu->set_parami(VM::ParamIOPort, param.io_port);
#if defined(_MBS1)
	emu->set_parami(VM::ParamSysMode, (1 - param.sys_mode_rev) | (pConfig->sys_mode & ~1));
#endif
	int val = 0;

	pConfig->wav_reverse = (param.wav_reverse == 1);
	pConfig->wav_half = (param.wav_half == 1);
	pConfig->wav_correct = (param.wav_correct != 0);
	pConfig->wav_correct_type = (param.wav_correct > 0 ? (uint8_t)(param.wav_correct - 1) : 0);
	if (sscanf(param.wav_correct_amp[0], "%d", &val) && val >= 100 && val <= 5000) {
		pConfig->wav_correct_amp[0] = val;
	}
	if (sscanf(param.wav_correct_amp[1], "%d", &val) && val >= 100 && val <= 5000) {
		pConfig->wav_correct_amp[1] = val;
	}

	pConfig->wav_sample_rate = (uint8_t)param.wav_sample_rate;
	pConfig->wav_sample_bits = (uint8_t)param.wav_sample_bits;

	for(int i=0; i<MAX_PRINTER; i++) {
		pConfig->printer_server_host[i].Set(param.prn_host[i]);
		val = 0;
		if (sscanf(param.prn_port[i], "%d", &val) == 1 && val > 0 && val < 65535) {
			pConfig->printer_server_port[i] = val;
		}
		double valued = 0.0;
		valued = strtod(param.prn_delay[i], NULL);
		if (valued < 0.1) valued = 0.1;
		if (valued > 1000.0) valued = 1000.0;
		valued = floor(valued * 10.0 + 0.5) / 10.0;
		pConfig->printer_delay[i] = valued;
	}
	for(int i=0; i<MAX_COMM; i++) {
		pConfig->comm_server_host[i].Set(param.com_host[i]);
		val = 0;
		if (sscanf(param.com_port[i], "%d", &val) == 1 && val > 0 && val < 65535) {
			pConfig->comm_server_port[i] = val;
		}
		pConfig->comm_dipswitch[i] = param.com_dipsw[i];
	}
#ifdef USE_DEBUGGER
	pConfig->debugger_server_host.Set(param.dbgr_host);
	val = 0;
	if (sscanf(param.dbgr_port, "%d", &val) == 1 && val > 0 && val < 65535) {
		pConfig->debugger_server_port = val;
	}
#endif
	pConfig->comm_uart_baudrate = strtol(LABELS::comm_uart_baudrate[param.uart_baud_index], NULL, 10);
	pConfig->comm_uart_databit = param.uart_databit + 7;
	pConfig->comm_uart_parity = param.uart_parity;
	pConfig->comm_uart_stopbit = param.uart_stopbit + 1;
	pConfig->comm_uart_flowctrl = param.uart_flowctrl;

	pConfig->snapshot_path.Set(param.snap_path);
	pConfig->font_path.Set(param.font_path);
	pConfig->msgboard_msg_fontname.Set(param.msg_font_name);
	pConfig->msgboard_info_fontname.Set(param.info_font_name);
	val = 0;
	if (sscanf(param.msg_font_size, "%d", &val) == 1 && val >= 6 && val <= 60) {
		pConfig->msgboard_msg_fontsize = val;
	}
	val = 0;
	if (sscanf(param.info_font_size, "%d", &val) == 1 && val >= 6 && val <= 60) {
		pConfig->msgboard_info_fontsize = val;
	}
	pConfig->menu_fontname.Set(param.menu_font_name);
	val = 0;
	if (sscanf(param.menu_font_size, "%d", &val) == 1 && val >= 6 && val <= 60) {
		pConfig->menu_fontsize = val;
	}

	// language
	clocale->ChooseLocaleName(lang_list, param.lang_selidx, pConfig->language);

	pConfig->rom_path.Set(param.rom_path);

	BIT_ONOFF(pConfig->misc_flags, MSK_SHOWMSG_UNDEFOP, param.showmsg_undefop);

	BIT_ONOFF(pConfig->misc_flags, MSK_CLEAR_CPUREG, param.clear_cpureg);

#if defined(_MBS1)
	emu->set_parami(VM::ParamExMemNum, param.exram_num);
	pConfig->mem_nowait = (param.mem_nowait == 1);
//	pConfig->opn_clock = param.fmopn_clock_num;
	pConfig->opn_irq = param.fmopn_irq;
	emu->set_parami(VM::ParamChipTypeOnFmOpn, param.type_of_fmopn);
	emu->set_parami(VM::ParamChipTypeOnExPsg, param.type_of_expsg);
# if defined(USE_Z80B_CARD)
	pConfig->z80b_card_out_irq = param.z80b_card_out_irq;
# elif defined(USE_MPC_68008)
	BIT_ONOFF(pConfig->misc_flags, MSK_SHOWMSG_ADDRERR, param.showmsg_addrerr);
# endif
#endif

	// set message font
#ifdef USE_MESSAGE_BOARD
	MsgBoard *msgboard = emu->get_msgboard();
	if (msgboard) {
		msgboard->SetFont();
	}
#endif
	// set menu font
//	gui->ConfigLoad();
	gui->ChangeLedBox(param.led_show);
	gui->ChangeLedBoxPosition(pConfig->led_pos);
	pConfig->save();
#ifdef USE_OPENGL
	emu->change_opengl_attr();
#endif

	delete paramtmp;
	paramtmp = NULL;

	return 1;
}

void AG_CONFIG_DLG::show_dir_dlg(AG_Window *win, DirDlgType type)
{
	switch(type) {
	case RomPath:
		dirdlg->CreateLoad(CMSG(ROM_Path), paramtmp->rom_path, sizeof(paramtmp->rom_path), win);
		break;
	case SnapShotPath:
		dirdlg->CreateLoad(CMSG(Snapshot_Path), paramtmp->snap_path, sizeof(paramtmp->snap_path), win);
		break;
	case FontPath:
		dirdlg->CreateLoad(CMSG(Font_Path), paramtmp->font_path, sizeof(paramtmp->font_path), win);
		break;
	}
}

/*
 * Event Handler (static)
 */
void AG_CONFIG_DLG::OnOk(AG_Event *event)
{
	AG_CONFIG_DLG *dlg = (AG_CONFIG_DLG *)AG_PTR(1);
	AG_Window *win = (AG_Window *)AG_PTR(2);

	if (dlg->SetData(win)) {
		dlg->Close(win);
	}
}

void AG_CONFIG_DLG::OnClose(AG_Event *event)
{
	AG_CONFIG_DLG *dlg = (AG_CONFIG_DLG *)AG_PTR(1);
	AG_Window *win = (AG_Window *)AG_PTR(2);

	dlg->Close(win);
}

void AG_CONFIG_DLG::OnChangeCorrect(AG_Event *event)
{
//	AG_CONFIG_DLG *dlg = (AG_CONFIG_DLG *)AG_PTR(1);
//	AG_Radio *rad = (AG_Radio *)AG_PTR(2);
//
//	if (dlg->paramtmp->wav_correct == 0) {
//		AG_WidgetDisable(rad);
//	} else {
//		AG_WidgetEnable(rad);
//	}
}

void AG_CONFIG_DLG::OnChangeSampleRate(AG_Event *event)
{
	AG_CONFIG_DLG *dlg = (AG_CONFIG_DLG *)AG_PTR(1);
	AG_TlistItem *item = (AG_TlistItem *)AG_PTR(2);
	int state = AG_INT(3);

	if (state) {
		dlg->paramtmp->wav_sample_rate = item->label;
	}
}

void AG_CONFIG_DLG::OnChangeSampleBits(AG_Event *event)
{
	AG_CONFIG_DLG *dlg = (AG_CONFIG_DLG *)AG_PTR(1);
	AG_TlistItem *item = (AG_TlistItem *)AG_PTR(2);
	int state = AG_INT(3);

	if (state) {
		dlg->paramtmp->wav_sample_bits = item->label;
	}
}

void AG_CONFIG_DLG::OnChangeFddType(AG_Event *event)
{
	AG_CONFIG_DLG *dlg = (AG_CONFIG_DLG *)AG_PTR(1);
	int index = (int)AG_INT(2);

	dlg->change_fdd_type(index);
}

void AG_CONFIG_DLG::OnChangeIOPort(AG_Event *event)
{
	AG_CONFIG_DLG *dlg = (AG_CONFIG_DLG *)AG_PTR(1);
	int index = AG_INT(2);
	int status = AG_INT(3);

	dlg->change_io_port(index, status);
}

void AG_CONFIG_DLG::OnChangeDisptmgSkew(AG_Event *event)
{
	AG_CONFIG_DLG *dlg = (AG_CONFIG_DLG *)AG_PTR(1);
	AG_TlistItem *item = (AG_TlistItem *)AG_PTR(2);
	int state = AG_INT(3);

	if (state) {
		dlg->paramtmp->disptmg_skew = item->label;
	}
}

void AG_CONFIG_DLG::OnChangeCurdispSkew(AG_Event *event)
{
	AG_CONFIG_DLG *dlg = (AG_CONFIG_DLG *)AG_PTR(1);
	AG_TlistItem *item = (AG_TlistItem *)AG_PTR(2);
	int state = AG_INT(3);

	if (state) {
		dlg->paramtmp->curdisp_skew = item->label;
	}
}

void AG_CONFIG_DLG::OnChangeUseOpenGL(AG_Event *event)
{
	AG_CONFIG_DLG *dlg = (AG_CONFIG_DLG *)AG_PTR(1);
	AG_TlistItem *item = (AG_TlistItem *)AG_PTR(2);
	int state = AG_INT(3);

	if (state) {
		dlg->paramtmp->use_opengl = item->label;
	}
}

void AG_CONFIG_DLG::OnChangeGLFilterType(AG_Event *event)
{
	AG_CONFIG_DLG *dlg = (AG_CONFIG_DLG *)AG_PTR(1);
	AG_TlistItem *item = (AG_TlistItem *)AG_PTR(2);
	int state = AG_INT(3);

	if (state) {
		dlg->paramtmp->gl_filter_type = item->label;
	}
}

void AG_CONFIG_DLG::OnChangeLedShow(AG_Event *event)
{
	AG_CONFIG_DLG *dlg = (AG_CONFIG_DLG *)AG_PTR(1);
	AG_TlistItem *item = (AG_TlistItem *)AG_PTR(2);
	int state = AG_INT(3);

	if (state) {
		dlg->paramtmp->led_show = item->label;
	}
}

void AG_CONFIG_DLG::OnChangeLedPos(AG_Event *event)
{
	AG_CONFIG_DLG *dlg = (AG_CONFIG_DLG *)AG_PTR(1);
	AG_TlistItem *item = (AG_TlistItem *)AG_PTR(2);
	int state = AG_INT(3);

	if (state) {
		dlg->paramtmp->led_pos = item->label;
	}
}

void AG_CONFIG_DLG::OnChangeCaptureType(AG_Event *event)
{
	AG_CONFIG_DLG *dlg = (AG_CONFIG_DLG *)AG_PTR(1);
	AG_TlistItem *item = (AG_TlistItem *)AG_PTR(2);
	int state = AG_INT(3);

	if (state) {
		dlg->paramtmp->capture_type = item->label;
	}
}

void AG_CONFIG_DLG::OnChangeCommBaud(AG_Event *event)
{
	AG_CONFIG_DLG *dlg = (AG_CONFIG_DLG *)AG_PTR(1);
	int num   = AG_INT(2);
	AG_TlistItem *item = (AG_TlistItem *)AG_PTR(3);
	int state = AG_INT(4);

	if (state) {
		dlg->paramtmp->com_dipsw[num] = item->label + 1;
	}
}

void AG_CONFIG_DLG::OnChangeCommUartBaud(AG_Event *event)
{
	AG_CONFIG_DLG *dlg = (AG_CONFIG_DLG *)AG_PTR(1);
	AG_TlistItem *item = (AG_TlistItem *)AG_PTR(2);
	int state = AG_INT(3);

	if (state) {
		dlg->paramtmp->uart_baud_index = item->label;
	}
}

void AG_CONFIG_DLG::OnChangeCommUartDataBit(AG_Event *event)
{
	AG_CONFIG_DLG *dlg = (AG_CONFIG_DLG *)AG_PTR(1);
	AG_TlistItem *item = (AG_TlistItem *)AG_PTR(2);
	int state = AG_INT(3);

	if (state) {
		dlg->paramtmp->uart_databit = item->label;
	}
}

void AG_CONFIG_DLG::OnChangeCommUartParity(AG_Event *event)
{
	AG_CONFIG_DLG *dlg = (AG_CONFIG_DLG *)AG_PTR(1);
	AG_TlistItem *item = (AG_TlistItem *)AG_PTR(2);
	int state = AG_INT(3);

	if (state) {
		dlg->paramtmp->uart_parity = item->label;
	}
}

void AG_CONFIG_DLG::OnChangeCommUartStopBit(AG_Event *event)
{
	AG_CONFIG_DLG *dlg = (AG_CONFIG_DLG *)AG_PTR(1);
	AG_TlistItem *item = (AG_TlistItem *)AG_PTR(2);
	int state = AG_INT(3);

	if (state) {
		dlg->paramtmp->uart_stopbit = item->label;
	}
}

void AG_CONFIG_DLG::OnChangeCommUartFlowCtrl(AG_Event *event)
{
	AG_CONFIG_DLG *dlg = (AG_CONFIG_DLG *)AG_PTR(1);
	AG_TlistItem *item = (AG_TlistItem *)AG_PTR(2);
	int state = AG_INT(3);

	if (state) {
		dlg->paramtmp->uart_flowctrl = item->label;
	}
}

#if defined(_MBS1)
void AG_CONFIG_DLG::OnChangeExtRam(AG_Event *event)
{
	AG_CONFIG_DLG *dlg = (AG_CONFIG_DLG *)AG_PTR(1);
	AG_TlistItem *item = (AG_TlistItem *)AG_PTR(2);
	int state = AG_INT(3);

	if (state) {
		dlg->paramtmp->exram_num = item->label;
	}
}

#if 0
void AG_CONFIG_DLG::OnChangeFmOpnClock(AG_Event *event)
{
	AG_CONFIG_DLG *dlg = (AG_CONFIG_DLG *)AG_PTR(1);
	AG_TlistItem *item = (AG_TlistItem *)AG_PTR(2);
	int state = AG_INT(3);

	if (state) {
		dlg->paramtmp->fmopn_clock_num = item->label;
	}
}
#endif

void AG_CONFIG_DLG::OnChangeFmOpnIrq(AG_Event *event)
{
	AG_CONFIG_DLG *dlg = (AG_CONFIG_DLG *)AG_PTR(1);
	AG_TlistItem *item = (AG_TlistItem *)AG_PTR(2);
	int state = AG_INT(3);

	if (state) {
		dlg->paramtmp->fmopn_irq = item->label;
	}
}

void AG_CONFIG_DLG::OnChangeFmOpnChip(AG_Event *event)
{
	AG_CONFIG_DLG *dlg = (AG_CONFIG_DLG *)AG_PTR(1);
	AG_TlistItem *item = (AG_TlistItem *)AG_PTR(2);
	int state = AG_INT(3);

	if (state) {
		dlg->paramtmp->type_of_fmopn = item->label;
	}
}

void AG_CONFIG_DLG::OnChangeExpsgChip(AG_Event *event)
{
	AG_CONFIG_DLG *dlg = (AG_CONFIG_DLG *)AG_PTR(1);
	AG_TlistItem *item = (AG_TlistItem *)AG_PTR(2);
	int state = AG_INT(3);

	if (state) {
		dlg->paramtmp->type_of_expsg = item->label;
	}
}
#if defined(USE_Z80B_CARD)
void AG_CONFIG_DLG::OnChangeZ80BCardIrq(AG_Event *event)
{
	AG_CONFIG_DLG *dlg = (AG_CONFIG_DLG *)AG_PTR(1);
	AG_TlistItem *item = (AG_TlistItem *)AG_PTR(2);
	int state = AG_INT(3);

	if (state) {
		dlg->paramtmp->z80b_card_out_irq = item->label;
	}
}
#endif
#endif

void AG_CONFIG_DLG::OnChangeLanguage(AG_Event *event)
{
	AG_CONFIG_DLG *dlg = (AG_CONFIG_DLG *)AG_PTR(1);
	AG_TlistItem *item = (AG_TlistItem *)AG_PTR(2);
	int state = AG_INT(3);

	if (state) {
		dlg->paramtmp->lang_selidx = item->label;
	}
}

void AG_CONFIG_DLG::OnShowDirDlg(AG_Event *event)
{
	AG_CONFIG_DLG *dlg = (AG_CONFIG_DLG *)AG_PTR(1);
	AG_Window *win = (AG_Window *)AG_PTR(2);
	DirDlgType type = (DirDlgType)AG_INT(3);

	dlg->show_dir_dlg(win, type);
}

}; /* namespace GUI_AGAR */
