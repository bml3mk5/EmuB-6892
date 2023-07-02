/** @file win_configbox.cpp

	HITACHI BASIC MASTER LEVEL3 Mark5 Emulator 'EmuB-6892'
	HITACHI MB-S1 Emulator 'EmuB-S1'
	Skelton for retropc emulator

	@author Sasaji
	@date   2012.03.24 -

	@brief [ config box ]
*/
#include "win_configbox.h"
#include "../../emu.h"
#ifdef USE_MESSAGE_BOARD
#include "../../msgboard.h"
#endif
#include "win_folderbox.h"
#include "win_filebox.h"
#include "win_fontbox.h"
#include "../../res/resource.h"
#include "../../config.h"
#include "../../clocale.h"
#include "../../utility.h"
#include "../../rec_video_defs.h"
#include "win_gui.h"
#include "../../labels.h"
#include <math.h>

namespace GUI_WIN
{

#ifdef USE_IOPORT_FDD
#define IOPORT_STARTNUM 0
#else
#define IOPORT_STARTNUM 2
#endif

#if defined(_MBS1)
#define CONFIG_TABS 6
#else
#define CONFIG_TABS 5
#endif

ConfigBox::ConfigBox(HINSTANCE hInst, CFont *new_font, EMU *new_emu, GUI *new_gui)
	: CDialogBox(hInst, IDD_CONFIGURE, new_font, new_emu, new_gui)
{
	hInstance = hInst;

	fdd_type  = config.fdd_type;
	io_port = config.io_port;

	selected_tabctrl = 0;
}

ConfigBox::~ConfigBox()
{
}

INT_PTR ConfigBox::onInitDialog(UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND hCtrl;

	//
	CDialogBox::onInitDialog(message, wParam, lParam);

	fdd_type = emu->get_parami(VM::ParamFddType);
	io_port = emu->get_parami(VM::ParamIOPort);
#if defined(_MBS1)
	sys_mode = emu->get_parami(VM::ParamSysMode);
	exram_size_num = emu->get_parami(VM::ParamExMemNum);
#endif

	_TCHAR str[128];

	tab_items.Clear();

	// tab control
	CBox *box_all = new CBox(CBox::VerticalBox, 0, margin, _T("all"));
	CBox *box_tab = AdjustTabControl(box_all, IDC_TAB1, IDC_STATIC_0);
	box_tab->SetTabItems(&tab_items);

	HWND hTabCtrl = GetDlgItem(hDlg, IDC_TAB1);

	TCITEM tcitm;
	tcitm.mask = TCIF_TEXT;

	for(int i=0; LABELS::tabs[i] != CMsg::End; i++) {
		tcitm.pszText = (LPSTR)CMSGVM(LABELS::tabs[i]);
		TabCtrl_InsertItem(hTabCtrl , i , &tcitm);
		tab_items.Add(new CTabItemIds());
	}
	TabCtrl_SetCurSel(hTabCtrl, selected_tabctrl);

	// 0:Mode
	CBox *box_0all = box_tab->AddBox(CBox::VerticalBox, 0, 0, _T("0all"));
	tab_items.SetCurrentPosition(0);

	CBox *box_0h = box_0all->AddBox(CBox::HorizontalBox, 0, 0, _T("0h"));

	CBox *box_0lv = box_0h->AddBox(CBox::VerticalBox, 0, 0, _T("0lv"));

#if defined(_MBS1)
	// MODE switch
	CBox *box_sysmode_sw = CreateGroup(box_0lv, IDC_STATIC, CMsg::System_Mode_ASTERISK, CBox::VerticalBox);

	for(int id=ID_DIPSWITCH1, i=0; id<=ID_DIPSWITCH2; id++, i++) {
		CBox *box_mode_sw = box_sysmode_sw->AddBox(CBox::HorizontalBox);
		if (i == 0) CreateStatic(box_mode_sw, IDC_STATIC_MODE_SW, _T(">"));
		else AdjustStatic(box_mode_sw, IDC_STATIC_MODE_SW);
		CreateRadioButton(box_mode_sw, id, LABELS::sys_mode[i], (i == 0), 160, 0);
	}
	CBox *box_modec_sw = box_sysmode_sw->AddBox(CBox::HorizontalBox, 0, 0, _T("modec_sw"));
	box_modec_sw->AddSpace(20, 10);
	CreateCheckBox(box_modec_sw, ID_DIPSWITCH3, CMsg::NEWON7, (config.dipswitch & 4) != 0);
	CheckDlgButton(hDlg, ID_DIPSWITCH1, (sys_mode & 1) != 0);
	CheckDlgButton(hDlg, ID_DIPSWITCH2, (sys_mode & 1) == 0);
#else
	// DIP switch
	CBox *box_dip_sw = CreateGroup(box_0lv, IDC_STATIC, CMsg::DIP_Switch_ASTERISK, CBox::VerticalBox);
	CBox *box_mode_sw = box_dip_sw->AddBox(CBox::HorizontalBox, 0, 0, _T("mode_sw"));
	CreateStatic(box_mode_sw, IDC_STATIC_MODE_SW, _T(">"));
	CreateCheckBox(box_mode_sw, ID_DIPSWITCH3, CMsg::MODE_Switch, (config.dipswitch & 4) != 0);
#endif

	// FDD type select
	CBox *box_fdd_type = CreateGroup(box_0lv, IDC_STATIC, CMsg::FDD_Type_ASTERISK, CBox::VerticalBox);
	for(int id=IDC_RADIO_NOFDD, i=0; LABELS::fdd_type[i] != CMsg::End; id++, i++) {
		CBox *box_fdd_type0 = box_fdd_type->AddBox(CBox::HorizontalBox);
		if (i == 0) CreateStatic(box_fdd_type0, IDC_STATIC_ALLOW, _T(">"));
		else AdjustStatic(box_fdd_type0, IDC_STATIC_ALLOW);
		CreateRadioButton(box_fdd_type0, id, LABELS::fdd_type[i], (i == 0), 160, 0);
	}
	// FDD type select
	CheckDlgButton(hDlg, IDC_RADIO_NOFDD, fdd_type == FDD_TYPE_NOFDD);
	CheckDlgButton(hDlg, IDC_RADIO_3FDD, fdd_type == FDD_TYPE_3FDD);
	CheckDlgButton(hDlg, IDC_RADIO_5FDD, fdd_type == FDD_TYPE_5FDD);
	CheckDlgButton(hDlg, IDC_RADIO_5_8FDD, fdd_type == FDD_TYPE_58FDD);

	// power off
	CreateCheckBox(box_0lv, IDC_CHK_POWEROFF, CMsg::Enable_the_state_of_power_off, config.use_power_off);

	// I/O port address
	CBox *box_0rv = box_0h->AddBox(CBox::VerticalBox, 0, 0, _T("0rv"));
	CBox *box_ioport = CreateGroup(box_0rv, IDC_STATIC, CMsg::I_O_Port_Address_ASTERISK, CBox::VerticalBox);
	for(int i=0; LABELS::io_port[i] != CMsg::End; i++) {
		int pos = LABELS::io_port_pos[i];
		if ((1 << pos) & IOPORT_MSK_ALL) {
			CBox *box_ioport0 = box_ioport->AddBox(CBox::HorizontalBox);
			CreateStatic(box_ioport0, IDC_STATIC_IOPORT1 + pos, _T(">"));
			CreateCheckBox(box_ioport0, IDC_CHK_IOPORT1 + pos, LABELS::io_port[i], (io_port & (1 << pos)) != 0);
		}
	}

	// text
	CreateStatic(box_0all, IDC_STATIC, CMsg::Need_restart_program_or_PowerOn);


	// 1:Screen
	CBox *box_1all = box_tab->AddBox(CBox::VerticalBox, 0, 0, _T("1all"));
	tab_items.SetCurrentPosition(1);

	CBox *box_1h = box_1all->AddBox(CBox::HorizontalBox, 0, 0, _T("1h"));

	CBox *box_1lv = box_1h->AddBox(CBox::VerticalBox, 0, 0, _T("1lv"));

	// DirectX
	CBox *box_d3d = CreateGroup(box_1lv, IDC_STATIC, CMsg::Drawing, CBox::HorizontalBox);

	CBox *box_d3d_tit = box_d3d->AddBox(CBox::VerticalBox, 0, 0, _T("d3d_tit"));
#ifdef USE_DIRECT3D
	CreateStatic(box_d3d_tit, IDC_STATIC_D3D_USE, CMsg::Method);
#endif
#ifdef USE_OPENGL
	CreateStatic(box_d3d_tit, IDC_STATIC_OPENGL_USE, CMsg::Method_ASTERISK);
#endif
#ifdef USE_DIRECT3D
	CreateStatic(box_d3d_tit, IDC_STATIC_D3D_FILTER, CMsg::Filter_Type);
#endif
#ifdef USE_OPENGL
	CreateStatic(box_d3d_tit, IDC_STATIC_OPENGL_FILTER, CMsg::Filter_Type);
#endif

	CBox *box_d3d_com = box_d3d->AddBox(CBox::VerticalBox, 0, 0, _T("d3d_com"));
#ifdef USE_DIRECT3D
	// d3d use
	CreateComboBox(box_d3d_com, IDC_COMBO_D3D_USE, LABELS::d3d_use, config.use_direct3d, 6);
#endif
#ifdef USE_OPENGL
	// opengl use
	CreateComboBox(box_d3d_com, IDC_COMBO_OPENGL_USE, LABELS::opengl_use, config.use_opengl, 6);
#endif
#ifdef USE_DIRECT3D
	// d3d filter type
	CreateComboBox(box_d3d_com, IDC_COMBO_D3D_FILTER, LABELS::d3d_filter, config.d3d_filter_type, 6);
#endif
#ifdef USE_OPENGL
	// opengl filter type
	CreateComboBox(box_d3d_com, IDC_COMBO_OPENGL_FILTER, LABELS::opengl_filter, config.gl_filter_type, 6);
#endif

	// crtc
	CBox *box_1rv = box_1h->AddBox(CBox::VerticalBox, 0, 0, _T("1rv"));
//	(box_1rv);

	CBox *box_crtc = CreateGroup(box_1rv, IDC_STATIC, CMsg::CRTC, CBox::HorizontalBox);

	CBox *box_crtc_tit = box_crtc->AddBox(CBox::VerticalBox, 0, 0, _T("crtc_tit"));
	CreateStatic(box_crtc_tit, IDC_STATIC_DISPTMG, CMsg::Disptmg_Skew);
#if defined(_MBS1)
	CreateStatic(box_crtc_tit, IDC_STATIC_CURDISP, CMsg::Curdisp_Skew_L3);
#else
	CreateStatic(box_crtc_tit, IDC_STATIC_CURDISP, CMsg::Curdisp_Skew);
#endif

	CBox *box_crtc_com = box_crtc->AddBox(CBox::VerticalBox, 0, 0, _T("crtc_com"));
#if defined(_MBS1)
	CreateComboBox(box_crtc_com, IDC_COMBO_DISPTMG, LABELS::disp_skew, config.disptmg_skew + 2, 2);
	CreateComboBox(box_crtc_com, IDC_COMBO_CURDISP, LABELS::disp_skew, config.curdisp_skew + 2, 2);
#else
	CreateComboBox(box_crtc_com, IDC_COMBO_DISPTMG, LABELS::disp_skew, config.disptmg_skew, 2);
	CreateComboBox(box_crtc_com, IDC_COMBO_CURDISP, LABELS::disp_skew, config.curdisp_skew, 2);
#endif

	// LED
	int led_show = gui->GetLedBoxPhase(-1);
	CBox *box_led = box_1all->AddBox(CBox::HorizontalBox, 0, 0, _T("led"));
	// led show
	CreateStatic(box_led, IDC_STATIC_LED_SHOW, CMsg::LED);
	CreateComboBox(box_led, IDC_COMBO_LED_SHOW, LABELS::led_show, led_show, 8);
	// led pos
	CreateStatic(box_led, IDC_STATIC_LED_POS, CMsg::Position);
	CreateComboBox(box_led, IDC_COMBO_LED_POS, LABELS::led_pos, config.led_pos, 8);

	// Capture Type
	CBox *box_capt = box_1all->AddBox(CBox::HorizontalBox, 0, 0, _T("capture"));
	CreateStatic(box_capt, IDC_STATIC_CAPTURE_TYPE, CMsg::Capture_Type);
	CreateComboBox(box_capt, IDC_COMBO_CAPTURE_TYPE, LABELS::capture_fmt, config.capture_type, 8);

	//
	// snapshot path
	CBox *box_snapshot = box_1all->AddBox(CBox::HorizontalBox, 0, 0, _T("snapshot"));
	CreateStatic(box_snapshot, IDC_STATIC_SNAP_PATH, CMsg::Snapshot_Path, 100);
	CreateEditBox(box_snapshot, IDC_SNAP_PATH, config.snapshot_path.GetM(), 30);
	CreateButton(box_snapshot, IDC_BTN_SNAP_PATH, CMsg::Folder_, 5);

	CBox *box_fontfile = box_1all->AddBox(CBox::HorizontalBox, 0, 0, _T("fontfile"));
#if defined(USE_WIN)
	// font file
	CreateStatic(box_fontfile, IDC_STATIC_FONT_FILE, CMsg::Font_File_ASTERISK, 100);
	CreateEditBox(box_fontfile, IDC_FONT_FILE, config.font_path.GetM(), 30);
	CreateButton(box_fontfile, IDC_BTN_FONT_FILE, CMsg::File_, 5);
#endif
#if defined(USE_SDL) || defined(USE_SDL2)
	// font path
	CreateStatic(box_fontfile, IDC_STATIC_FONT_FILE, CMsg::Font_Path, 100);
	CreateEditBox(box_fontfile, IDC_FONT_FILE, config.font_path.GetM(), 30);
	CreateButton(box_fontfile, IDC_BTN_FONT_FILE, CMsg::Folder_, 5);
#endif

	// message font
	CBox *box_fontns = box_1all->AddBox(CBox::HorizontalBox, 0, 0, _T("fontns"));
	CreateStatic(box_fontns, IDC_STATIC_MSG_FONT_NAME_S, CMsg::Message_Font, 100);
	CreateEditBox(box_fontns, IDC_MSG_FONT_NAME_S, config.msgboard_msg_fontname.GetM(), 10);
	CreateStatic(box_fontns, IDC_STATIC_MSG_FONT_SIZE_S, CMsg::_Size);
	CreateEditBox(box_fontns, IDC_MSG_FONT_SIZE_S, config.msgboard_msg_fontsize, 3);
	CreateButton(box_fontns, IDC_BTN_FONT_NAME_S, CMsg::Font_, 5);

	// info font
	CBox *box_fontnl = box_1all->AddBox(CBox::HorizontalBox, 0, 0, _T("fontnl"));
	CreateStatic(box_fontnl, IDC_STATIC_MSG_FONT_NAME_L, CMsg::Info_Font, 100);
	CreateEditBox(box_fontnl, IDC_MSG_FONT_NAME_L, config.msgboard_info_fontname.GetM(), 10);
	CreateStatic(box_fontnl, IDC_STATIC_MSG_FONT_SIZE_L, CMsg::_Size);
	CreateEditBox(box_fontnl, IDC_MSG_FONT_SIZE_L, config.msgboard_info_fontsize, 3);
	CreateButton(box_fontnl, IDC_BTN_FONT_NAME_L, CMsg::Font_, 5);

	// language
	CBox *box_lang = box_1all->AddBox(CBox::HorizontalBox, 0, 0, _T("lang"));
	lang_list.Clear();
	clocale->GetLocaleNamesWithDefault(lang_list);
	int lang_selidx = clocale->SelectLocaleNameIndex(lang_list, config.language);
	CreateStatic(box_lang, IDC_STATIC_LANGUAGE, CMsg::Language_ASTERISK);
	CreateComboBox(box_lang, IDC_COMBO_LANGUAGE, lang_list, lang_selidx, 8);

	// text
	CreateStatic(box_1all, IDC_STATIC, CMsg::Need_restart_program);

	// 2:Tape, FDD
	CBox *box_2all = box_tab->AddBox(CBox::VerticalBox, 0, 0, _T("2all"));
	tab_items.SetCurrentPosition(2);

	CBox *box_2h = box_2all->AddBox(CBox::HorizontalBox, 0, 0, _T("2h"));

	CBox *box_2lv = box_2h->AddBox(CBox::VerticalBox, 0, 0, _T("2lv"));

	// Load wav file
	CBox *box_ldwav = CreateGroup(box_2lv, IDC_STATIC, CMsg::Load_Wav_File_from_Tape, CBox::VerticalBox);

	CBox *box_ldwav0 = box_ldwav->AddBox(CBox::HorizontalBox, 0, 0, _T("ldwav0"));
	CreateCheckBox(box_ldwav0, IDC_CHK_REVERSE, CMsg::Reverse_Wave, config.wav_reverse);
	CreateCheckBox(box_ldwav0, IDC_CHK_HALFWAVE, CMsg::Half_Wave, config.wav_half);
	CBox *box_ldwav1 = box_ldwav->AddBox(CBox::HorizontalBox, 0, 0, _T("ldwav1"));
	CreateStatic(box_ldwav1, IDC_STATIC_CORRECT, CMsg::Correct);
	for(int i=0; LABELS::correct[i] != CMsg::End; i++) {
		CreateRadioButton(box_ldwav1, IDC_RADIO_NOCORR + i, LABELS::correct[i], (i == 0));
	}
	CBox *box_ldwav2 = box_ldwav->AddBox(CBox::HorizontalBox, 0, 0, _T("ldwav2"));
	for(int i=0; i<2; i++) {
		CreateStatic(box_ldwav2, IDC_STATIC_CORRAMP0 + i, LABELS::correct_amp[i]);
		CreateEditBox(box_ldwav2, IDC_TXT_CORRAMP0 + i, config.wav_correct_amp[i], 4, WS_EX_RIGHT);
	}
	CheckDlgButton(hDlg, IDC_RADIO_NOCORR, !config.wav_correct);
	CheckDlgButton(hDlg, IDC_RADIO_COSW, config.wav_correct && config.wav_correct_type == 0);
	CheckDlgButton(hDlg, IDC_RADIO_SINW, config.wav_correct && config.wav_correct_type == 1);

	// Save wav file
	CBox *box_2rv = box_2h->AddBox(CBox::VerticalBox, 0, 0, _T("2rv"));

	CBox *box_svwav = CreateGroup(box_2rv, IDC_STATIC, CMsg::Save_Wav_File_to_Tape, CBox::HorizontalBox);

	CBox *box_svwav_t = box_svwav->AddBox(CBox::VerticalBox, 0, 0, _T("svwav_t"));
	CreateStatic(box_svwav_t, IDC_STATIC_SRATE, CMsg::Sample_Rate);
	CreateStatic(box_svwav_t, IDC_STATIC_SBITS, CMsg::Sample_Bits);

	CBox *box_svwav_c = box_svwav->AddBox(CBox::VerticalBox, 0, 0, _T("svwav_c"));
	CreateComboBox(box_svwav_c, IDC_COMBO_SRATE, LABELS::sound_rate, config.wav_sample_rate, 4);
	CreateComboBox(box_svwav_c, IDC_COMBO_SBITS, LABELS::sound_bits, config.wav_sample_bits, 4);

	// FDD
	CBox *box_fdd = CreateGroup(box_2all, IDC_STATIC, CMsg::Floppy_Disk_Drive, CBox::VerticalBox);

	// mount fdd
	CBox *box_fdd_mount = box_fdd->AddBox(CBox::HorizontalBox, 0, 0, _T("fdd_mount"));
	CreateStatic(box_fdd_mount, IDC_STATIC_FDD_MOUNT, CMsg::When_start_up_mount_disk_at_);
	for(int i=0; i<MAX_DRIVE; i++) {
		UTILITY::stprintf(str, sizeof(str), _T("%d"), i);
		CreateCheckBox(box_fdd_mount, IDC_CHK_FDD_MOUNT0 + i, str, (config.mount_fdd & (1 << i)) != 0);
	}

	CreateCheckBox(box_fdd, IDC_CHK_DELAYFD1, CMsg::Ignore_delays_to_find_sector, FLG_DELAY_FDSEARCH != 0);
	CreateCheckBox(box_fdd, IDC_CHK_DELAYFD2, CMsg::Ignore_delays_to_seek_track, FLG_DELAY_FDSEEK != 0);
	CreateCheckBox(box_fdd, IDC_CHK_FDDENSITY, CMsg::Suppress_checking_for_density, !FLG_CHECK_FDDENSITY);
	CreateCheckBox(box_fdd, IDC_CHK_FDMEDIA, CMsg::Suppress_checking_for_media_type, !FLG_CHECK_FDMEDIA);

	// 3:Network
	CBox *box_3all = box_tab->AddBox(CBox::VerticalBox, 0, 0, _T("3all"));
	tab_items.SetCurrentPosition(3);

	// LPT0 .. 2
	for(int i=0; i<MAX_PRINTER; i++) {
		CBox *box_3h = box_3all->AddBox(CBox::HorizontalBox, 0, 0, _T("3h"));
		UTILITY::stprintf(str, sizeof(str), CMSGM(LPTVDIGIT_Hostname), i); 
		CreateStatic(box_3h, IDC_STATIC_LPT0 + i, str, 100);
		CreateEditBox(box_3h, IDC_HOSTNAME_LPT0 + i, config.printer_server_host[i], 12, WS_EX_LEFT);
		CreateStatic(box_3h, IDC_STATIC_P_LPT0 + i, CMsg::_Port);
		CreateEditBox(box_3h, IDC_PORT_LPT0 + i, config.printer_server_port[i], 6, WS_EX_RIGHT);
		CreateStatic(box_3h, IDC_STATIC_LPT0_1 + i, CMsg::_Print_delay);
		UTILITY::stprintf(str, sizeof(str), _T("%.1f"), config.printer_delay[i]);
		CreateEditBox(box_3h, IDC_DELAY_LPT0 + i, str, 6, WS_EX_RIGHT);
		CreateStatic(box_3h, IDC_STATIC_LPT0_2 + i, CMsg::msec);
	}
	// COM0 .. 1
	for(int i=0; i<MAX_COMM; i++) {
		CBox *box_3h = box_3all->AddBox(CBox::HorizontalBox, 0, 0, _T("3h"));
		UTILITY::stprintf(str, sizeof(str), CMSGM(COMVDIGIT_Hostname), i); 
		CreateStatic(box_3h, IDC_STATIC_COM0 + i, str, 100);
		CreateEditBox(box_3h, IDC_HOSTNAME_COM0 + i, config.comm_server_host[i], 12, WS_EX_LEFT);
		CreateStatic(box_3h, IDC_STATIC_P_COM0 + i, CMsg::_Port);
		CreateEditBox(box_3h, IDC_PORT_COM0 + i, config.comm_server_port[i], 6, WS_EX_RIGHT);
		CreateComboBox(box_3h, IDC_COMBO_COM0 + i, LABELS::comm_baud, config.comm_dipswitch[i] - 1, 8);
	}
#ifdef USE_DEBUGGER
	// Debugger
	CBox *box_3dbg = box_3all->AddBox(CBox::HorizontalBox, 0, 0, _T("3dbg"));
	CreateStatic(box_3dbg, IDC_STATIC_DBGR, CMsg::Connectable_host_to_Debugger);
//	box_3dbg->AddSpace(32, 1);
	CreateEditBox(box_3dbg, IDC_HOSTNAME_DBGR, config.debugger_server_host, 12, WS_EX_LEFT);
	CreateStatic(box_3dbg, IDC_STATIC_P_DBGR, CMsg::_Port);
	CreateEditBox(box_3dbg, IDC_PORT_DBGR, config.debugger_server_port, 6, WS_EX_RIGHT);
#endif
	// uart
//	int uart_w = font->GetTextWidth(hDlg, _T("mmmmmmmmmmmm"));
	int uart_w = 100;
	CBox *box_3uart = CreateGroup(box_3all, IDC_STATIC_UART, CMsg::Settings_of_serial_ports_on_host, CBox::VerticalBox);
	CBox *box_3uah = box_3uart->AddBox(CBox::HorizontalBox, 0, 0, _T("3uart0"));
	CBox *box_3uh = box_3uah->AddBox(CBox::VerticalBox, 0, 0, _T("3uart1"));
	CreateStatic(box_3uh, IDC_STATIC_UART_BAUDRATE, CMsg::Baud_Rate, uart_w);
	CreateStatic(box_3uh, IDC_STATIC_UART_DATABIT, CMsg::Data_Bit, uart_w);
	CreateStatic(box_3uh, IDC_STATIC_UART_PARITY, CMsg::Parity, uart_w);
	CreateStatic(box_3uh, IDC_STATIC_UART_STOPBIT, CMsg::Stop_Bit, uart_w);
	CreateStatic(box_3uh, IDC_STATIC_UART_FLOWCTRL, CMsg::Flow_Control, uart_w);
	box_3uh = box_3uah->AddBox(CBox::VerticalBox, 0, 0, _T("3uart2"));
	// uart
	int vali = 0;
	for(int i=0; LABELS::comm_uart_baudrate[i] != NULL; i++) {
		if (config.comm_uart_baudrate == _tcstol(LABELS::comm_uart_baudrate[i], NULL, 10)) {
			vali = i;
			break;
		}
	}
	CreateComboBox(box_3uh, IDC_COMBO_UART_BAUDRATE, LABELS::comm_uart_baudrate, vali, 6);
	vali = config.comm_uart_databit - 7; // 7bit: 0 8bit: 1
	if (vali < 0 || 1 < vali) vali = 1;
	CreateComboBox(box_3uh, IDC_COMBO_UART_DATABIT, LABELS::comm_uart_databit, vali, 6);
	CreateComboBox(box_3uh, IDC_COMBO_UART_PARITY, LABELS::comm_uart_parity, config.comm_uart_parity, 6);
	CreateComboBox(box_3uh, IDC_COMBO_UART_STOPBIT, LABELS::comm_uart_stopbit, config.comm_uart_stopbit - 1, 6);
	CreateComboBox(box_3uh, IDC_COMBO_UART_FLOWCTRL, LABELS::comm_uart_flowctrl, config.comm_uart_flowctrl, 6);
	CreateStatic(box_3uart, IDC_STATIC_UART_MSG, CMsg::Need_re_connect_to_serial_port_when_modified_this);

	// 4:CPU, Memory
	CBox *box_4all = box_tab->AddBox(CBox::VerticalBox, 0, 0, _T("4all"));
	tab_items.SetCurrentPosition(4);

	// rom path
	CBox *box_rompath = box_4all->AddBox(CBox::HorizontalBox, 0, 0, _T("rompath"));
	CreateStatic(box_rompath, IDC_STATIC_ROM_PATH, CMsg::ROM_Path_ASTERISK);
	CreateEditBox(box_rompath, IDC_ROM_PATH, config.rom_path.GetM(), 30);
	CreateButton(box_rompath, IDC_BTN_ROM_PATH, CMsg::Folder_, 5);

#if defined(_MBS1)
	// exram size
	CBox *box_exram = box_4all->AddBox(CBox::HorizontalBox, 0, 0, _T("exram"));
	CreateStatic(box_exram, IDC_STATIC_EXMEM, CMsg::Extended_RAM_ASTERISK);
	CreateComboBox(box_exram, IDC_COMBO_EXMEM,LABELS::exram_size, exram_size_num, 5);
	if (0 <= config.exram_size_num && config.exram_size_num <= 4) {
		UTILITY::tcscpy(str, sizeof(str), CMSGM(LB_Now_SP));
		UTILITY::tcscat(str, sizeof(str), CMSGVM(LABELS::exram_size[config.exram_size_num]));
		UTILITY::tcscat(str, sizeof(str), _T(")"));
	} else {
		str[0] = _T('\0');
	}
	CreateStatic(box_exram, IDC_STATIC_EXMEM_NOW, str);

	// no wait
	CreateCheckBox(box_4all, IDC_CHK_MEMNOWAIT, CMsg::No_wait_to_access_the_main_memory, config.mem_nowait);
#else
	// ex memory
	CreateCheckBox(box_4all, IDC_CHK_EXMEM, CMsg::Use_Extended_Memory_64KB, config.exram_size_num == 1);
#endif

	// undef opcode
	CreateCheckBox(box_4all, IDC_CHK_UNDEFOP, CMsg::Show_message_when_the_CPU_fetches_undefined_opcode, FLG_SHOWMSG_UNDEFOP != 0);

#if defined(_MBS1)
# if defined(USE_Z80B_CARD)
	// z80b card
	CBox *box_z80bint = box_4all->AddBox(CBox::HorizontalBox, 0, 0, _T("z80int"));
	CreateStatic(box_z80bint, IDC_STATIC_Z80BCARD_IRQ, CMsg::Connect_interrupt_signal_of_Z80B_Card_to_ASTERISK);
	CreateComboBox(box_z80bint, IDC_COMBO_Z80BCARD_IRQ, LABELS::z80bcard_irq, config.z80b_card_out_irq, 5);
# elif defined(USE_MPC_68008)
	// MPC-68008 card
	CreateCheckBox(box_4all, IDC_CHK_ADDRERR, CMsg::Show_message_when_the_address_error_occured_in_MC68008, FLG_SHOWMSG_ADDRERR != 0);
# endif
#endif

	// clear CPU registers
	CreateCheckBox(box_4all, IDC_CHK_CLEAR_CPUREG, CMsg::Clear_CPU_registers_at_power_on, FLG_CLEAR_CPUREG != 0);

	// text
	CreateStatic(box_4all, IDC_STATIC, CMsg::Need_restart_program_or_PowerOn);

#if defined(_MBS1)
	// 5:Sound
	CBox *box_5all = box_tab->AddBox(CBox::VerticalBox, 0, 0, _T("5all"));
	tab_items.SetCurrentPosition(5);

	// FM synth card
	CBox *box_fmopngrp = CreateGroup(box_5all, IDC_STATIC, CMsg::FM_Synthesis_Card_ASTERISK, CBox::VerticalBox);

	CreateCheckBox(box_fmopngrp, IDC_CHK_EN_FMOPN, CMsg::Enable, IOPORT_USE_FMOPN != 0);
	CreateStatic(box_fmopngrp, IDC_STATIC, CMsg::IO_ports_are_FF1E_FF1F_FF16_FF17);

//	CBox *box_fmopnclk = box_fmopngrp->AddBox(CBox::HorizontalBox, 0, 0, _T("opnclk"));
//	CreateStatic(box_fmopnclk, IDC_STATIC, CMsg::Clock);
//	CreateComboBox(box_fmopnclk, IDC_COMBO_FMOPN, LABELS::fmopn_clock, config.opn_clock, 5);
//	CreateStatic(box_fmopnclk, IDC_STATIC, _T("Hz"));

	// chip type
	int type_of_fmopn = emu->get_parami(VM::ParamChipTypeOnFmOpn);
	CBox *box_fmopn_chip = box_fmopngrp->AddBox(CBox::HorizontalBox, 0, 0, _T("opnchip"));
	CreateStatic(box_fmopn_chip, IDC_STATIC, CMsg::Sound_chip);
	CreateComboBox(box_fmopn_chip, IDC_COMBO_CHIP_FMOPN, LABELS::type_of_soundcard, type_of_fmopn, 16, config.type_of_fmopn, CMsg::LB_Now_RB);

	// Ex PSG port
	CBox *box_expsggrp = CreateGroup(box_5all, IDC_STATIC, CMsg::Extended_PSG_Port_ASTERISK, CBox::VerticalBox);

	CreateCheckBox(box_expsggrp, IDC_CHK_EN_EXPSG, CMsg::Enable, IOPORT_USE_EXPSG != 0);
	CreateStatic(box_expsggrp, IDC_STATIC, CMsg::IO_ports_are_FFE6_FFE7_FFEE_FFEF);

	// chip type
	int type_of_expsg = emu->get_parami(VM::ParamChipTypeOnExPsg);
	CBox *box_expsg_chip = box_expsggrp->AddBox(CBox::HorizontalBox, 0, 0, _T("psgchip"));
	CreateStatic(box_expsg_chip, IDC_STATIC, CMsg::Sound_chip);
	CreateComboBox(box_expsg_chip, IDC_COMBO_USE_EXPSG, LABELS::type_of_soundcard, type_of_expsg, 16, config.type_of_expsg, CMsg::LB_Now_RB);

	// interrupt
	CBox *box_fmint = box_5all->AddBox(CBox::HorizontalBox, 0, 0, _T("fmint"));
	CreateStatic(box_fmint, IDC_STATIC, CMsg::Connect_interrupt_signal_of_FM_synthesis_to_ASTERISK_ASTERISK);
	CreateComboBox(box_fmint, IDC_COMBO_FMOPN_IRQ, LABELS::fmopn_irq, config.opn_irq, 5);

	// text
	CreateStatic(box_5all, IDC_STATIC, CMsg::Need_restart_program_or_PowerOn);
	CreateStatic(box_5all, IDC_STATIC, CMsg::This_is_the_common_setting_both_FM_synthesis_card_and_extended_PSG_port);
#endif

	// button
	CBox *box_btn = box_all->AddBox(CBox::HorizontalBox, CBox::RightPos);
	CreateButton(box_btn, IDOK, CMsg::OK, 8, true);
	CreateButton(box_btn, IDCANCEL, CMsg::Cancel, 8);

	RECT prc;
	GetClientRect(hTabCtrl, &prc);
	TabCtrl_AdjustRect(hTabCtrl, FALSE, &prc);
	box_tab->SetTopMargin(prc.top + 8);

	box_all->Realize(*this);

	int ax, ay;
#if defined(_MBS1)
	hCtrl = GetDlgItem(hDlg, IDC_STATIC_MODE_SW);
	box_sysmode_sw->GetPositionByItem((1 - (config.sys_mode & 1)), ax, ay);
	SetWindowPos(hCtrl, 0, ax, ay + padding, 1, 1, SWP_NOSIZE | SWP_NOZORDER);
#endif
	hCtrl = GetDlgItem(hDlg, IDC_STATIC_ALLOW);
	box_fdd_type->GetPositionByItem(config.fdd_type, ax, ay);
	SetWindowPos(hCtrl, 0, ax, ay + padding, 1, 1, SWP_NOSIZE | SWP_NOZORDER);

	select_tabctrl(selected_tabctrl);

	delete box_all;

	return (INT_PTR)TRUE;
}

INT_PTR ConfigBox::onCommand(UINT message, WPARAM wParam, LPARAM lParam)
{
#ifdef USE_IOPORT_FDD
	HWND hCtrl;
#endif
	WORD wId = LOWORD(wParam);

	if (wId == IDOK) {
		onOK(message, wParam, lParam);
	}
	else if (wId == IDC_RADIO_NOFDD) {
		CheckDlgButton(hDlg, IDC_RADIO_3FDD, 0);
		CheckDlgButton(hDlg, IDC_RADIO_5FDD, 0);
		CheckDlgButton(hDlg, IDC_RADIO_5_8FDD, 0);
#ifdef USE_IOPORT_FDD
		// I/O port
		CheckDlgButton(hDlg, IDC_CHK_IOPORT1, 0);
		hCtrl = GetDlgItem(hDlg, IDC_CHK_IOPORT1);
		EnableWindow(hCtrl, true);
		CheckDlgButton(hDlg, IDC_CHK_IOPORT2, 0);
		hCtrl = GetDlgItem(hDlg, IDC_CHK_IOPORT2);
		EnableWindow(hCtrl, true);
#endif
		return (INT_PTR)FALSE;
	}
	else if (wId == IDC_RADIO_3FDD) {
		CheckDlgButton(hDlg, IDC_RADIO_NOFDD, 0);
		CheckDlgButton(hDlg, IDC_RADIO_5FDD, 0);
		CheckDlgButton(hDlg, IDC_RADIO_5_8FDD, 0);
#ifdef USE_IOPORT_FDD
		// I/O port
		CheckDlgButton(hDlg, IDC_CHK_IOPORT1, 0);
		hCtrl = GetDlgItem(hDlg, IDC_CHK_IOPORT1);
		EnableWindow(hCtrl, false);
		CheckDlgButton(hDlg, IDC_CHK_IOPORT2, 1);
		hCtrl = GetDlgItem(hDlg, IDC_CHK_IOPORT2);
		EnableWindow(hCtrl, false);
#endif
		return (INT_PTR)FALSE;
	}
	else if (wId == IDC_RADIO_5FDD) {
		CheckDlgButton(hDlg, IDC_RADIO_NOFDD, 0);
		CheckDlgButton(hDlg, IDC_RADIO_3FDD, 0);
		CheckDlgButton(hDlg, IDC_RADIO_5_8FDD, 0);
#ifdef USE_IOPORT_FDD
		// I/O port
		CheckDlgButton(hDlg, IDC_CHK_IOPORT1, 1);
		hCtrl = GetDlgItem(hDlg, IDC_CHK_IOPORT1);
		EnableWindow(hCtrl, false);
		CheckDlgButton(hDlg, IDC_CHK_IOPORT2, 0);
		hCtrl = GetDlgItem(hDlg, IDC_CHK_IOPORT2);
		EnableWindow(hCtrl, false);
#endif
		return (INT_PTR)FALSE;
	}
	else if (wId == IDC_RADIO_5_8FDD) {
		CheckDlgButton(hDlg, IDC_RADIO_NOFDD, 0);
		CheckDlgButton(hDlg, IDC_RADIO_3FDD, 0);
		CheckDlgButton(hDlg, IDC_RADIO_5FDD, 0);
#ifdef USE_IOPORT_FDD
		// I/O port
		CheckDlgButton(hDlg, IDC_CHK_IOPORT1, 1);
		hCtrl = GetDlgItem(hDlg, IDC_CHK_IOPORT1);
		EnableWindow(hCtrl, false);
		CheckDlgButton(hDlg, IDC_CHK_IOPORT2, 0);
		hCtrl = GetDlgItem(hDlg, IDC_CHK_IOPORT2);
		EnableWindow(hCtrl, false);
#endif
		return (INT_PTR)FALSE;
	}
	else if (wId == IDC_RADIO_NOCORR)
	{
		CheckDlgButton(hDlg, IDC_RADIO_COSW, 0);
		CheckDlgButton(hDlg, IDC_RADIO_SINW, 0);
		return (INT_PTR)FALSE;
	}
	else if (wId == IDC_RADIO_COSW)
	{
		CheckDlgButton(hDlg, IDC_RADIO_NOCORR, 0);
		CheckDlgButton(hDlg, IDC_RADIO_SINW, 0);
		return (INT_PTR)FALSE;
	}
	else if (wId == IDC_RADIO_SINW)
	{
		CheckDlgButton(hDlg, IDC_RADIO_NOCORR, 0);
		CheckDlgButton(hDlg, IDC_RADIO_COSW, 0);
		return (INT_PTR)FALSE;
	}
//	else if (wId == IDC_CHK_CORRECT)
//	{
//		if (IsDlgButtonChecked(hDlg, IDC_CHK_CORRECT)) {
//			hCtrl = GetDlgItem(hDlg, IDC_RADIO_COSW);
//			EnableWindow(hCtrl, true);
//			hCtrl = GetDlgItem(hDlg, IDC_RADIO_SINW);
//			EnableWindow(hCtrl, true);
//		} else {
//			hCtrl = GetDlgItem(hDlg, IDC_RADIO_COSW);
//			EnableWindow(hCtrl, false);
//			hCtrl = GetDlgItem(hDlg, IDC_RADIO_SINW);
//			EnableWindow(hCtrl, false);
//		}
//		return (INT_PTR)FALSE;
//	}
#ifdef USE_IOPORT_FDD
	else if (wId == IDC_CHK_IOPORT1)
	{
		if (IsDlgButtonChecked(hDlg, IDC_CHK_IOPORT1)) {
			CheckDlgButton(hDlg, IDC_CHK_IOPORT2, 0);
		}
		return (INT_PTR)FALSE;
	}
	else if (wId == IDC_CHK_IOPORT2)
	{
		if (IsDlgButtonChecked(hDlg, IDC_CHK_IOPORT2)) {
			CheckDlgButton(hDlg, IDC_CHK_IOPORT1, 0);
		}
		return (INT_PTR)FALSE;
	}
#endif
	else if (wId == IDC_CHK_IOPORT6)
	{
		if (IsDlgButtonChecked(hDlg, IDC_CHK_IOPORT6)) {
			CheckDlgButton(hDlg, IDC_CHK_IOPORT7, 0);
#if defined(_MBS1)
			CheckDlgButton(hDlg, IDC_CHK_IOPORT10, 0);
#endif
		}
		return (INT_PTR)FALSE;
	}
	else if (wId == IDC_CHK_IOPORT7)
	{
		if (IsDlgButtonChecked(hDlg, IDC_CHK_IOPORT7)) {
			CheckDlgButton(hDlg, IDC_CHK_IOPORT6, 0);
		}
		return (INT_PTR)FALSE;
	}
#if defined(_MBS1)
	else if (wId == IDC_CHK_IOPORT8)
	{
		// ex psg
		CheckDlgButton(hDlg, IDC_CHK_EN_EXPSG, IsDlgButtonChecked(hDlg, IDC_CHK_IOPORT8) ? 1 : 0);
		return (INT_PTR)FALSE;
	}
	else if (wId == IDC_CHK_IOPORT10)
	{
		// cm01
		if (IsDlgButtonChecked(hDlg, IDC_CHK_IOPORT10)) {
			CheckDlgButton(hDlg, IDC_CHK_IOPORT6, 0);
			CheckDlgButton(hDlg, IDC_CHK_IOPORT7, 1);
		}
		return (INT_PTR)FALSE;
	}
	else if (wId == IDC_CHK_IOPORT13)
	{
		// fm opn
		CheckDlgButton(hDlg, IDC_CHK_EN_FMOPN, IsDlgButtonChecked(hDlg, IDC_CHK_IOPORT13) ? 1 : 0);
		return (INT_PTR)FALSE;
	}
	else if (wId == IDC_CHK_EN_EXPSG)
	{
		// ex psg
		CheckDlgButton(hDlg, IDC_CHK_IOPORT8, IsDlgButtonChecked(hDlg, IDC_CHK_EN_EXPSG) ? 1 : 0);
		return (INT_PTR)FALSE;
	}
	else if (wId == IDC_CHK_EN_FMOPN)
	{
		// fm opn
		CheckDlgButton(hDlg, IDC_CHK_IOPORT13, IsDlgButtonChecked(hDlg, IDC_CHK_EN_FMOPN) ? 1 : 0);
		return (INT_PTR)FALSE;
	}
#endif
	else if (wId == IDC_BTN_SNAP_PATH)
	{
		_TCHAR buf[_MAX_PATH];
		FolderBox fbox(hDlg);
		GetDlgItemText(hDlg, IDC_SNAP_PATH, buf, _MAX_PATH);
		if (buf[0] == _T('\0')) {
			const _TCHAR *tbuf = emu->application_path();
#if defined(USE_UTF8_ON_MBCS)
			UTILITY::conv_to_native_path(tbuf, buf, _MAX_PATH);
#else
			UTILITY::tcscpy(buf, _MAX_PATH, tbuf);
#endif
		}
		if (fbox.Show(CMSGM(Select_a_folder_to_save_snapshot_images), buf, _MAX_PATH)) {
			SetDlgItemText(hDlg, IDC_SNAP_PATH, buf);
		}
		return (INT_PTR)FALSE;
	}
	else if (wId == IDC_BTN_FONT_FILE)
	{
#ifdef USE_WIN
		const CMsg::Id filter[] =
			{ CMsg::Supported_Files_ttf_otf, CMsg::All_Files_, CMsg::End };
		_TCHAR buf[_MAX_PATH];
		FileBox fbox(hDlg);
		GetDlgItemText(hDlg, IDC_FONT_FILE, buf, _MAX_PATH);
		if (fbox.Show(
			filter,
			CMSGM(Select_a_font_file_for_showing_messages),
			NULL,
			_T("ttf"),
			false,
			buf)) {
			SetDlgItemText(hDlg, IDC_FONT_FILE, buf);
		}
#endif
#if defined(USE_SDL) || defined(USE_SDL2)
		_TCHAR buf[_MAX_PATH];
		FolderBox fbox(hDlg);
		GetDlgItemText(hDlg, IDC_FONT_FILE, buf, _MAX_PATH);
		if (buf[0] == _T('\0')) {
			const _TCHAR *tbuf = emu->application_path();
#if defined(USE_UTF8_ON_MBCS)
			UTILITY::conv_to_native_path(tbuf, buf, _MAX_PATH);
#else
			UTILITY::tcscpy(buf, _MAX_PATH, tbuf);
#endif
		}
		if (fbox.Show(CMSGM(Select_a_font_folder_for_showing_messages), buf, _MAX_PATH)) {
			SetDlgItemText(hDlg, IDC_FONT_FILE, buf);
		}
#endif
		return (INT_PTR)FALSE;
	}
	else if (wId == IDC_BTN_FONT_NAME_L)
	{
		_TCHAR buf[_MAX_PATH];
		double font_size;
		BOOL rc;
		FontBox fbox(hDlg);
		GetDlgItemText(hDlg, IDC_MSG_FONT_NAME_L, buf, _MAX_PATH);
		font_size = (double)GetDlgItemInt(hDlg, IDC_MSG_FONT_SIZE_L, &rc, TRUE);
		if (fbox.Show(
			CMSGM(Select_a_font),
			NULL, buf, _MAX_PATH, &font_size)) {
			SetDlgItemText(hDlg, IDC_MSG_FONT_NAME_L, buf);
			SetDlgItemInt(hDlg, IDC_MSG_FONT_SIZE_L, (UINT)font_size, TRUE);
		}
		return (INT_PTR)FALSE;
	}
	else if (wId == IDC_BTN_FONT_NAME_S)
	{
		_TCHAR buf[_MAX_PATH];
		double font_size;
		BOOL rc;
		FontBox fbox(hDlg);
		GetDlgItemText(hDlg, IDC_MSG_FONT_NAME_S, buf, _MAX_PATH);
		font_size = (double)GetDlgItemInt(hDlg, IDC_MSG_FONT_SIZE_S, &rc, TRUE);
		if (fbox.Show(
			CMSGM(Select_a_font),
			NULL, buf, _MAX_PATH, &font_size)) {
			SetDlgItemText(hDlg, IDC_MSG_FONT_NAME_S, buf);
			SetDlgItemInt(hDlg, IDC_MSG_FONT_SIZE_S, (UINT)font_size, TRUE);
		}
		return (INT_PTR)FALSE;
	}
	else if (wId == IDC_BTN_ROM_PATH)
	{
		_TCHAR buf[_MAX_PATH];
		FolderBox fbox(hDlg);
		GetDlgItemText(hDlg, IDC_ROM_PATH, buf, _MAX_PATH);
		if (buf[0] == _T('\0')) {
			const _TCHAR *tbuf = emu->application_path();
#if defined(USE_UTF8_ON_MBCS)
			UTILITY::conv_to_native_path(tbuf, buf, _MAX_PATH);
#else
			UTILITY::tcscpy(buf, _MAX_PATH, tbuf);
#endif
		}
		if (fbox.Show(CMSGM(Select_a_folder_containing_the_rom_images), buf, _MAX_PATH)) {
			SetDlgItemText(hDlg, IDC_ROM_PATH, buf);
		}
		return (INT_PTR)FALSE;
	}

	if (wId == IDOK || wId == IDCANCEL) {
		::EndDialog(hDlg, wId);
		return (INT_PTR)TRUE;
	}
	return (INT_PTR)FALSE;
}

INT_PTR ConfigBox::onNotify(UINT message, WPARAM wParam, LPARAM lParam)
{
	// change tab
	LPNMHDR lpNmHdr = (NMHDR *)lParam;
	int i;
	if (lpNmHdr->idFrom == IDC_TAB1) {
		switch (lpNmHdr->code) {
		case TCN_SELCHANGE:
			i = TabCtrl_GetCurSel(lpNmHdr->hwndFrom);
			select_tabctrl(i);
			break;
		}
	}
	return (INT_PTR)TRUE;
}

INT_PTR ConfigBox::onOK(UINT message, WPARAM wParam, LPARAM lParam)
{
	_TCHAR buf[_MAX_PATH];
	int valuel;
	BOOL rc;

	// power off
	config.use_power_off = (IsDlgButtonChecked(hDlg, IDC_CHK_POWEROFF) == BST_CHECKED);

	// MODE switch
#if defined(_MBS1)
	if (IsDlgButtonChecked(hDlg, ID_DIPSWITCH1)) {
		sys_mode |= 1;
	} else {
		sys_mode &= ~1;
	}
#endif
	if (IsDlgButtonChecked(hDlg, ID_DIPSWITCH3)) {
		config.dipswitch |= 4;
	} else {
		config.dipswitch &= ~4;
	}

	// fdd type
	fdd_type = (IsDlgButtonChecked(hDlg, IDC_RADIO_3FDD) ? FDD_TYPE_3FDD : 0)
			 + (IsDlgButtonChecked(hDlg, IDC_RADIO_5FDD) ? FDD_TYPE_5FDD : 0)
			 + (IsDlgButtonChecked(hDlg, IDC_RADIO_5_8FDD) ? FDD_TYPE_58FDD : 0);
#ifndef USE_IOPORT_FDD
	io_port &= ~IOPORT_MSK_FDDALL;
	io_port |= (fdd_type == FDD_TYPE_3FDD ? IOPORT_MSK_3FDD
			 : (fdd_type == FDD_TYPE_5FDD
			 || fdd_type == FDD_TYPE_58FDD ? IOPORT_MSK_5FDD : 0));
#endif

	config.mount_fdd = 0;
	for (int i=0; i<MAX_DRIVE; i++) {
		config.mount_fdd |= (IsDlgButtonChecked(hDlg, IDC_CHK_FDD_MOUNT0 + i) == BST_CHECKED ? 1 << i : 0);
	}
	config.delay_fdd = (IsDlgButtonChecked(hDlg, IDC_CHK_DELAYFD1) == BST_CHECKED ? MSK_DELAY_FDSEARCH : 0)
					| (IsDlgButtonChecked(hDlg, IDC_CHK_DELAYFD2) == BST_CHECKED ? MSK_DELAY_FDSEEK : 0);
	config.check_fdmedia = (IsDlgButtonChecked(hDlg, IDC_CHK_FDDENSITY) == BST_CHECKED ? 0 : MSK_CHECK_FDDENSITY)
					| (IsDlgButtonChecked(hDlg, IDC_CHK_FDMEDIA) == BST_CHECKED ? 0 : MSK_CHECK_FDMEDIA);

	config.wav_reverse = (IsDlgButtonChecked(hDlg, IDC_CHK_REVERSE) == BST_CHECKED);
	config.wav_half = (IsDlgButtonChecked(hDlg, IDC_CHK_HALFWAVE) == BST_CHECKED);
	config.wav_correct = !(IsDlgButtonChecked(hDlg, IDC_RADIO_NOCORR) == BST_CHECKED);
	config.wav_correct_type = (IsDlgButtonChecked(hDlg, IDC_RADIO_SINW) == BST_CHECKED ? 1 : 0);
	valuel = GetDlgItemInt(hDlg, IDC_TXT_CORRAMP0, &rc, false);
	if (rc == TRUE && 100 <= valuel && valuel <= 5000) {
		config.wav_correct_amp[0] = valuel;
	}
	valuel = GetDlgItemInt(hDlg, IDC_TXT_CORRAMP1, &rc, false);
	if (rc == TRUE && 100 <= valuel && valuel <= 5000) {
		config.wav_correct_amp[1] = valuel;
	}

	config.wav_sample_rate = (uint8_t)SendDlgItemMessage(hDlg, IDC_COMBO_SRATE, CB_GETCURSEL, 0, 0);
	config.wav_sample_bits = (uint8_t)SendDlgItemMessage(hDlg, IDC_COMBO_SBITS, CB_GETCURSEL, 0, 0);

	// I/O port address
	for(int i=0; LABELS::io_port[i] != CMsg::End; i++) {
		int pos = LABELS::io_port_pos[i];
		if ((1 << pos) & IOPORT_MSK_ALL) {
			if (IsDlgButtonChecked(hDlg, IDC_CHK_IOPORT1 + pos) != BST_UNCHECKED)  {
				io_port |= (1 << pos);
			} else {
				io_port &= ~(1 << pos);
			}
		}
	}
	// crtc
#if defined(_MBS1)
	config.disptmg_skew = (int8_t)(SendDlgItemMessage(hDlg, IDC_COMBO_DISPTMG, CB_GETCURSEL, 0, 0) - 2);
	config.curdisp_skew = (int8_t)(SendDlgItemMessage(hDlg, IDC_COMBO_CURDISP, CB_GETCURSEL, 0, 0) - 2);
#else
	config.disptmg_skew = (uint8_t)SendDlgItemMessage(hDlg, IDC_COMBO_DISPTMG, CB_GETCURSEL, 0, 0);
	config.curdisp_skew = (uint8_t)SendDlgItemMessage(hDlg, IDC_COMBO_CURDISP, CB_GETCURSEL, 0, 0);
#endif

#ifdef USE_DIRECT3D
	// d3d use
	config.use_direct3d = (uint8_t)SendDlgItemMessage(hDlg, IDC_COMBO_D3D_USE, CB_GETCURSEL, 0, 0);

	// d3d filter type
	config.d3d_filter_type = (uint8_t)SendDlgItemMessage(hDlg, IDC_COMBO_D3D_FILTER, CB_GETCURSEL, 0, 0);
#endif
#ifdef USE_OPENGL
	// opengl use
	config.use_opengl = (uint8_t)SendDlgItemMessage(hDlg, IDC_COMBO_OPENGL_USE, CB_GETCURSEL, 0, 0);

	// opengl filter type
	config.gl_filter_type = (uint8_t)SendDlgItemMessage(hDlg, IDC_COMBO_OPENGL_FILTER, CB_GETCURSEL, 0, 0);
#endif

	// led show
	int led_show = (int)SendDlgItemMessage(hDlg, IDC_COMBO_LED_SHOW, CB_GETCURSEL, 0, 0);
	// led pos
	config.led_pos = (uint8_t)SendDlgItemMessage(hDlg, IDC_COMBO_LED_POS, CB_GETCURSEL, 0, 0);

	// capture type
	config.capture_type = (uint8_t)SendDlgItemMessage(hDlg, IDC_COMBO_CAPTURE_TYPE, CB_GETCURSEL, 0, 0);

	// snapshot path
	GetDlgItemText(hDlg, IDC_SNAP_PATH, buf, _MAX_PATH);
	config.snapshot_path.SetM(buf);
	// font file / path
	GetDlgItemText(hDlg, IDC_FONT_FILE, buf, _MAX_PATH);
	config.font_path.SetM(buf);
	// message font name
	GetDlgItemText(hDlg, IDC_MSG_FONT_NAME_S, buf, _MAX_PATH);
	config.msgboard_msg_fontname.SetM(buf);
	GetDlgItemText(hDlg, IDC_MSG_FONT_NAME_L, buf, _MAX_PATH);
	config.msgboard_info_fontname.SetM(buf);
	// message font size
	valuel = GetDlgItemInt(hDlg, IDC_MSG_FONT_SIZE_L, &rc, TRUE);
	if (rc == TRUE && 1 <= valuel && valuel <= 60) {
		config.msgboard_info_fontsize = (uint8_t)valuel;
	}
	valuel = GetDlgItemInt(hDlg, IDC_MSG_FONT_SIZE_S, &rc, TRUE);
	if (rc == TRUE && 1 <= valuel && valuel <= 60) {
		config.msgboard_msg_fontsize = (uint8_t)valuel;
	}

	// language
	valuel = (int)SendDlgItemMessage(hDlg, IDC_COMBO_LANGUAGE, CB_GETCURSEL, 0, 0);
	clocale->ChooseLocaleName(lang_list, valuel, config.language);

	// hostname, port
	for(int i=0; i<MAX_PRINTER; i++) {
		GetDlgItemText(hDlg, IDC_HOSTNAME_LPT0 + i, buf, _MAX_PATH);
		config.printer_server_host[i].Set(buf);
		valuel = GetDlgItemInt(hDlg, IDC_PORT_LPT0 + i, &rc, TRUE);
		if (rc == TRUE && 0 <= valuel && valuel <= 65535) {
			config.printer_server_port[i] = valuel;
		}
		GetDlgItemText(hDlg, IDC_DELAY_LPT0 + i, buf, _MAX_PATH);
		double valued = _tcstod(buf, NULL);
		if (valued < 0.1) valued = 0.1;
		if (valued > 1000.0) valued = 1000.0;
		valued = floor(valued * 10.0 + 0.5) / 10.0;
		config.printer_delay[i] = valued;
	}
	for(int i=0; i<MAX_COMM; i++) {
		GetDlgItemText(hDlg, IDC_HOSTNAME_COM0 + i, buf, _MAX_PATH);
		config.comm_server_host[i].Set(buf);
		valuel = GetDlgItemInt(hDlg, IDC_PORT_COM0 + i, &rc, TRUE);
		if (rc == TRUE && 0 <= valuel && valuel <= 65535) {
			config.comm_server_port[i] = valuel;
		}
		valuel = (int)SendDlgItemMessage(hDlg, IDC_COMBO_COM0 + i, CB_GETCURSEL, 0, 0);
		if (valuel >= 0 && valuel <= 3) {
			config.comm_dipswitch[i] = valuel + 1;
		}
	}
#ifdef USE_DEBUGGER
	GetDlgItemText(hDlg, IDC_HOSTNAME_DBGR, buf, _MAX_PATH);
	config.debugger_server_host.Set(buf);
	valuel = GetDlgItemInt(hDlg, IDC_PORT_DBGR, &rc, TRUE);
	if (rc == TRUE && 0 <= valuel && valuel <= 65535) {
		config.debugger_server_port = valuel;
	}
#endif
	// uart
	valuel = GetDlgItemInt(hDlg, IDC_COMBO_UART_BAUDRATE, &rc, FALSE);
	if (rc == TRUE && valuel > 0) config.comm_uart_baudrate = valuel;
	valuel = (int)SendDlgItemMessage(hDlg, IDC_COMBO_UART_DATABIT, CB_GETCURSEL, 0, 0);
	if (valuel >= 0 && valuel <= 1) config.comm_uart_databit = valuel + 7; // 7bit: 0 8bit: 1
	valuel = (int)SendDlgItemMessage(hDlg, IDC_COMBO_UART_PARITY, CB_GETCURSEL, 0, 0);
	if (valuel >= 0 && valuel <= 4) config.comm_uart_parity = valuel;
	valuel = (int)SendDlgItemMessage(hDlg, IDC_COMBO_UART_STOPBIT, CB_GETCURSEL, 0, 0);
	if (valuel >= 0 && valuel <= 1) config.comm_uart_stopbit = valuel + 1;
	valuel = (int)SendDlgItemMessage(hDlg, IDC_COMBO_UART_FLOWCTRL, CB_GETCURSEL, 0, 0);
	if (valuel >= 0 && valuel <= 2) config.comm_uart_flowctrl = valuel;

	// rom path
	GetDlgItemText(hDlg, IDC_ROM_PATH, buf, _MAX_PATH);
	config.rom_path.SetM(buf);
#if defined(_MBS1)
	// exram size
	exram_size_num = (int)SendDlgItemMessage(hDlg, IDC_COMBO_EXMEM, CB_GETCURSEL, 0, 0);
	// no wait
	config.mem_nowait = (IsDlgButtonChecked(hDlg, IDC_CHK_MEMNOWAIT) == BST_CHECKED);
#else
	// extended memory
	config.exram_size_num = IsDlgButtonChecked(hDlg, IDC_CHK_EXMEM) ? 1 : 0;
#endif

	// undef opcode
	BIT_ONOFF(config.misc_flags, MSK_SHOWMSG_UNDEFOP, IsDlgButtonChecked(hDlg, IDC_CHK_UNDEFOP) == BST_CHECKED);

#if defined(_MBS1)
# if defined(USE_Z80B_CARD)
	// z80bcard irq
	config.z80b_card_out_irq = (int)SendDlgItemMessage(hDlg, IDC_COMBO_Z80BCARD_IRQ, CB_GETCURSEL, 0, 0);
# elif defined(USE_MPC_68008)
	// MPC-68008 card

	// address error
	BIT_ONOFF(config.misc_flags, MSK_SHOWMSG_ADDRERR, IsDlgButtonChecked(hDlg, IDC_CHK_ADDRERR) == BST_CHECKED);
# endif
#endif

	// clear CPU registers
	BIT_ONOFF(config.misc_flags, MSK_CLEAR_CPUREG, IsDlgButtonChecked(hDlg, IDC_CHK_CLEAR_CPUREG) == BST_CHECKED);

#if defined(_MBS1)
	// fmopn clock
//	config.opn_clock = (int)SendDlgItemMessage(hDlg, IDC_COMBO_FMOPN, CB_GETCURSEL, 0, 0);
	// fmopn irq
	config.opn_irq = (int)SendDlgItemMessage(hDlg, IDC_COMBO_FMOPN_IRQ, CB_GETCURSEL, 0, 0);

	// chip type of FM synth
	int type_of_fmopn = (int)SendDlgItemMessage(hDlg, IDC_COMBO_CHIP_FMOPN, CB_GETCURSEL, 0, 0);

	// use opn instead of psg
//	config.use_opn_expsg = (IsDlgButtonChecked(hDlg, IDC_CHK_USE_OPN_EXPSG) == BST_CHECKED);
	int type_of_expsg = (int)SendDlgItemMessage(hDlg, IDC_COMBO_USE_EXPSG, CB_GETCURSEL, 0, 0);

	emu->set_parami(VM::ParamSysMode, sys_mode);
	emu->set_parami(VM::ParamExMemNum, exram_size_num);

	emu->set_parami(VM::ParamChipTypeOnFmOpn, type_of_fmopn);
	emu->set_parami(VM::ParamChipTypeOnExPsg, type_of_expsg);
#endif

	emu->set_parami(VM::ParamFddType, fdd_type);
	emu->set_parami(VM::ParamIOPort, io_port);

	// message font
#ifdef USE_MESSAGE_BOARD
	MsgBoard *msgboard = emu->get_msgboard();
	if (msgboard) {
		msgboard->SetFont();
	}
#endif

	gui->ChangeLedBox(led_show);
	gui->ChangeLedBoxPosition(config.led_pos);

	config.save();
#ifdef USE_OPENGL
	emu->change_opengl_attr();
#endif
	emu->update_config();

	return (INT_PTR)TRUE;
}

#if 0
INT_PTR ConfigBox::onControlColorStatic(UINT message, WPARAM wParam, LPARAM lParam)
{
	HANDLE h = (HANDLE)GetStockObject(NULL_BRUSH);
	SetBkMode((HDC)wParam, TRANSPARENT);
	return (INT_PTR)h;
}
#endif

void ConfigBox::select_tabctrl(int tab_num)
{
	HWND hCtrl;
	int cmdShow;

	selected_tabctrl = tab_num;
	for(int i=0; i<CONFIG_TABS; i++) {
		cmdShow = (selected_tabctrl == i ? SW_SHOW : SW_HIDE);

		CTabItemIds *ids = tab_items.Item(i);
		if (ids) {
			for(int j=0; j<(int)ids->size(); j++) {
				hCtrl = GetDlgItem(hDlg, ids->at(j));
				ShowWindow(hCtrl, cmdShow);
			}
		}
	}

	switch(selected_tabctrl) {
	case 0:
		// Mode
#if defined(_BML3MK5)
		// Mode switch
		hCtrl = GetDlgItem(hDlg, IDC_STATIC_MODE_SW);
		ShowWindow(hCtrl, (config.dipswitch & 4) != 0 ? SW_SHOWNORMAL : SW_HIDE);
#endif
		// I/O port address
		for(int i=0; LABELS::io_port[i] != CMsg::End; i++) {
			int pos = LABELS::io_port_pos[i];
//			CheckDlgButton(hDlg, IDC_CHK_IOPORT1 + pos, (io_port & (1 << pos)) != 0);
			hCtrl = GetDlgItem(hDlg, IDC_STATIC_IOPORT1 + pos);
			ShowWindow(hCtrl, (config.io_port & (1 << pos)) != 0 ? SW_SHOWNORMAL : SW_HIDE);
		}
#ifdef USE_IOPORT_FDD
		if (fdd_type == FDD_TYPE_3FDD) {
			// I/O port (3inch)
			CheckDlgButton(hDlg, IDC_CHK_IOPORT1, 0);
			hCtrl = GetDlgItem(hDlg, IDC_CHK_IOPORT1);
			EnableWindow(hCtrl, false);
			CheckDlgButton(hDlg, IDC_CHK_IOPORT2, 1);
			hCtrl = GetDlgItem(hDlg, IDC_CHK_IOPORT2);
			EnableWindow(hCtrl, false);
		}
		else if (fdd_type == FDD_TYPE_5FDD
#if defined(_MBS1)
				|| fdd_type == FDD_TYPE_5HFDD)
#else
				|| fdd_type == FDD_TYPE_8FDD)
#endif
		{
			// I/O port (5inch)
			CheckDlgButton(hDlg, IDC_CHK_IOPORT1, 1);
			hCtrl = GetDlgItem(hDlg, IDC_CHK_IOPORT1);
			EnableWindow(hCtrl, false);
			CheckDlgButton(hDlg, IDC_CHK_IOPORT2, 0);
			hCtrl = GetDlgItem(hDlg, IDC_CHK_IOPORT2);
			EnableWindow(hCtrl, false);
		}
#endif
		break;
	default:
		break;
	}
}

}; /* namespace GUI_WIN */
