/** @file wx_config_dlg.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2014.03.23

	@brief [ wx_config_dlg ]
*/

#include <wx/wx.h>
#include <wx/fontdlg.h>
#include "wx_config_dlg.h"
#include "../../emu.h"
#include "../../config.h"
#include "../../clocale.h"
#include "../../res/resource.h"
#ifdef USE_MESSAGE_BOARD
#include "../../msgboard.h"
#endif
#include "../../msgs.h"
#include "../../labels.h"
#include "../gui_base.h"

//#ifdef USE_IOPORT_FDD
//#define IOPORT_STARTNUM 0
//#else
//#define IOPORT_STARTNUM 2
//#endif

// Attach Event
BEGIN_EVENT_TABLE(MyConfigDlg, wxDialog)
	EVT_COMMAND_RANGE(IDC_RADIO_NOFDD, IDC_RADIO_5FDD, wxEVT_RADIOBUTTON, MyConfigDlg::OnChangeFddType)
	EVT_COMMAND_RANGE(IDC_CHK_IOPORT1, IDC_CHK_IOPORT17, wxEVT_CHECKBOX, MyConfigDlg::OnChangeIOPort)
	EVT_COMMAND(IDC_CHK_EN_FMOPN, wxEVT_CHECKBOX, MyConfigDlg::OnChangeSoundCard)
	EVT_COMMAND(IDC_CHK_EN_EXPSG, wxEVT_CHECKBOX, MyConfigDlg::OnChangeSoundCard)
	EVT_BUTTON(IDC_BTN_SNAP_PATH, MyConfigDlg::OnShowFolderDialog)
	EVT_BUTTON(IDC_BTN_FONT_FILE, MyConfigDlg::OnShowFolderDialog)
	EVT_BUTTON(IDC_BTN_ROM_PATH, MyConfigDlg::OnShowFolderDialog)
	EVT_BUTTON(IDC_BTN_FONT_NAME_S, MyConfigDlg::OnShowFontDialog)
	EVT_BUTTON(IDC_BTN_FONT_NAME_L, MyConfigDlg::OnShowFontDialog)
//	EVT_CHECKBOX(IDC_CHK_CORRECT, MyConfigDlg::OnChangeCorrect)
END_EVENT_TABLE()

MyConfigDlg::MyConfigDlg(wxWindow* parent, wxWindowID id, EMU *parent_emu, GUI_BASE *parent_gui)
	: MyDialog(parent, id, _("Configure"), parent_emu, parent_gui)
{
}

MyConfigDlg::~MyConfigDlg()
{
}

/**
 * create config dialog when ShowModal called
 */
void MyConfigDlg::InitDialog()
{
	wxSizerFlags flags = wxSizerFlags().Expand().Border(wxALL, 2);

	wxBoxSizer *szrLeft;
	wxBoxSizer *szrRight;
	wxBoxSizer *szrMain;
	wxBoxSizer *szrSub;
	wxBoxSizer *szrAll = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *bszr, *bszr2, *bszr3;
	wxGridSizer *gszr;

	MyNotebook *book = new MyNotebook(this, IDC_NOTEBOOK, wxDefaultPosition, wxDefaultSize);
	wxPanel *page;

	// 0: Mode
	page = new wxPanel(book);
	book->AddPageById(page, CMsg::Mode);

	szrMain = new wxBoxSizer(wxVERTICAL);
	szrSub = new wxBoxSizer(wxHORIZONTAL);

	szrLeft = new wxBoxSizer(wxVERTICAL);

	// DIP switch
	wxBoxSizer *szrDIPSwitch;
#if defined(_MBS1)
	wxStaticText *sta;
	szrDIPSwitch = new wxStaticBoxSizer(new MyStaticBox(page, wxID_ANY, CMsg::System_Mode_ASTERISK), wxVERTICAL);
	gszr = new wxFlexGridSizer(3, 2, 0, 0);
	for(int i=0; i<2; i++) {
		staSysMode[i] = new wxStaticText(page, wxID_ANY, _T(">"));
		gszr->Add(staSysMode[i], flags);
		radSysMode[i] = new MyRadioButton(page, wxID_ANY, LABELS::sys_mode[i], wxDefaultPosition, wxDefaultSize, i == 0 ? wxRB_GROUP : 0);
		gszr->Add(radSysMode[i], flags);
	}
	sta = new wxStaticText(page, wxID_ANY, _T(" "));
	gszr->Add(sta, flags);
	chkDIPSwitch = new MyCheckBox(page, ID_DIPSWITCH3, CMsg::NEWON7);
	gszr->Add(chkDIPSwitch, wxSizerFlags().Expand().Border(wxALL, 2).Border(wxLEFT, 16));
	szrDIPSwitch->Add(gszr, flags);
#else
	szrDIPSwitch = new wxStaticBoxSizer(new MyStaticBox(page, wxID_ANY, CMsg::DIP_Switch_ASTERISK), wxVERTICAL);
	gszr = new wxFlexGridSizer(1, 2, 0, 0);
	staDIPSwitch[0] = new wxStaticText(page, IDC_STATIC_MODE_SW, _T(">"));
	gszr->Add(staDIPSwitch[0], flags);
	chkDIPSwitch[0] = new MyCheckBox(page, ID_DIPSWITCH3, CMsg::MODE_Switch);
	gszr->Add(chkDIPSwitch[0], flags);
	szrDIPSwitch->Add(gszr, flags);
#endif
	szrLeft->Add(szrDIPSwitch);

	// FDD type
	wxBoxSizer *szrFDDType;
	szrFDDType = new wxStaticBoxSizer(new MyStaticBox(page, wxID_ANY, CMsg::FDD_Type_ASTERISK), wxVERTICAL);
	gszr = new wxFlexGridSizer(4, 2, 0, 0);
	for(int i=0; LABELS::fdd_type[i] != CMsg::End; i++) {
		staFDDType[i] = new wxStaticText(page, IDC_STATIC_NOFDD + i, _T(">"));
		gszr->Add(staFDDType[i], flags);
		radFDDType[i] = new MyRadioButton(page, IDC_RADIO_NOFDD + i, LABELS::fdd_type[i], wxDefaultPosition,wxDefaultSize, i == 0 ? wxRB_GROUP : 0);
		gszr->Add(radFDDType[i], flags);
	}
	szrFDDType->Add(gszr, flags);
	szrLeft->Add(szrFDDType);

	chkPowerOff = new MyCheckBox(page, IDC_CHK_POWEROFF, CMsg::Enable_the_state_of_power_off);
	szrLeft->Add(chkPowerOff, flags);

	szrRight = new wxBoxSizer(wxVERTICAL);

	// I/O Port address
	wxBoxSizer *szrIOPort;
	szrIOPort = new wxStaticBoxSizer(new MyStaticBox(page, wxID_ANY, CMsg::I_O_Port_Address_ASTERISK), wxVERTICAL);
	gszr = new wxFlexGridSizer(2, 0, 0);
	for(int i=0; i<IOPORT_NUMS; i++) {
		staIOPort[i] = NULL;
		chkIOPort[i] = NULL;
	}
	for(int i=0; LABELS::io_port[i] != CMsg::End; i++) {
		int pos = LABELS::io_port_pos[i];
		if ((1 << pos) & IOPORT_MSK_ALL) {
			staIOPort[pos] = new wxStaticText(page, IDC_STATIC_IOPORT1 + pos, _T(">"));
			gszr->Add(staIOPort[pos], flags);
			chkIOPort[pos] = new MyCheckBox(page, IDC_CHK_IOPORT1 + pos, LABELS::io_port[i]);
			gszr->Add(chkIOPort[pos], flags);
		}
	}
	szrIOPort->Add(gszr, flags);
	szrRight->Add(szrIOPort);

	szrSub->Add(szrLeft, flags);
	szrSub->Add(szrRight, flags);
	szrMain->Add(szrSub, flags);


	bszr = new wxBoxSizer(wxVERTICAL);
	bszr->Add(new MyStaticText(page, wxID_ANY, CMsg::Need_restart_program_or_PowerOn), flags);
	szrMain->Add(bszr, flags);

	page->SetSizerAndFit(szrMain);

	// 1: Screen
	page = new wxPanel(book);
	book->AddPageById(page, CMsg::Screen);

	szrMain = new wxBoxSizer(wxVERTICAL);
	szrSub = new wxBoxSizer(wxHORIZONTAL);

	// GL Filter Type
	wxBoxSizer *szrGLFilter;
	szrGLFilter = new wxStaticBoxSizer(new MyStaticBox(page, wxID_ANY, CMsg::Drawing), wxVERTICAL);
	bszr = new wxBoxSizer(wxHORIZONTAL);
	bszr->Add(new MyStaticText(page, wxID_ANY, CMsg::Method_ASTERISK), flags);
	comGLUse = new MyChoice(page, IDC_COMBO_OPENGL_USE, wxDefaultPosition, wxDefaultSize, LABELS::opengl_use);
	bszr->Add(comGLUse, flags);
	szrGLFilter->Add(bszr, flags);
	bszr = new wxBoxSizer(wxHORIZONTAL);
	bszr->Add(new MyStaticText(page, wxID_ANY, CMsg::Filter_Type), flags);
	comGLFilter = new MyChoice(page, IDC_COMBO_OPENGL_FILTER, wxDefaultPosition, wxDefaultSize, LABELS::opengl_filter);
	bszr->Add(comGLFilter, flags);
	szrGLFilter->Add(bszr, flags);
	szrSub->Add(szrGLFilter);

	// CRTC
	wxBoxSizer *szrCrtc;
	szrCrtc = new wxStaticBoxSizer(new MyStaticBox(page, wxID_ANY, CMsg::CRTC), wxVERTICAL);
	bszr = new wxBoxSizer(wxVERTICAL);
	bszr2 = new wxBoxSizer(wxHORIZONTAL);
	bszr2->Add(new MyStaticText(page, wxID_ANY, CMsg::Disptmg_Skew), flags);
	comDisptmg = new MyChoice(page, IDC_COMBO_DISPTMG, wxDefaultPosition, wxDefaultSize, LABELS::disp_skew, 0);
	bszr2->Add(comDisptmg, flags);
	bszr->Add(bszr2, flags);
	bszr3 = new wxBoxSizer(wxHORIZONTAL);
#if defined(_MBS1)
	bszr3->Add(new MyStaticText(page, wxID_ANY, CMsg::Curdisp_Skew_L3), flags);
#else
	bszr3->Add(new MyStaticText(page, wxID_ANY, CMsg::Curdisp_Skew), flags);
#endif
	comCurdisp = new MyChoice(page, IDC_COMBO_CURDISP, wxDefaultPosition, wxDefaultSize, LABELS::disp_skew, 0);
	bszr3->Add(comCurdisp, flags);
	bszr->Add(bszr3, flags);
	szrCrtc->Add(bszr, flags);
	szrSub->Add(szrCrtc);

	szrMain->Add(szrSub, flags);

	// LED
	bszr = new wxBoxSizer(wxHORIZONTAL);
	bszr->Add(new MyStaticText(page, wxID_ANY, CMsg::LED), flags);
	comLedShow = new MyChoice(page, IDC_COMBO_LED_SHOW, wxDefaultPosition, wxDefaultSize, LABELS::led_show);
	bszr->Add(comLedShow, flags);
	bszr->Add(new MyStaticText(page, wxID_ANY, CMsg::Position), flags);
	comLedPos = new MyChoice(page, IDC_COMBO_LED_POS, wxDefaultPosition, wxDefaultSize, LABELS::led_pos);
	bszr->Add(comLedPos, flags);

	szrMain->Add(bszr, flags);

	// Capture type
	bszr = new wxBoxSizer(wxHORIZONTAL);
	bszr->Add(new MyStaticText(page, wxID_ANY, CMsg::Capture_Type), flags);
	comCapType = new MyChoice(page, IDC_COMBO_CAPTURE_TYPE, wxDefaultPosition, wxDefaultSize, LABELS::capture_fmt);
	bszr->Add(comCapType, flags);

	szrMain->Add(bszr, flags);

	// Snapshot path
	bszr = new wxBoxSizer(wxHORIZONTAL);
	bszr->Add(new MyStaticText(page, wxID_ANY, CMsg::Snapshot_Path), flags);
	txtSnapPath = new wxTextCtrl(page, IDC_SNAP_PATH, wxT(""), wxDefaultPosition, wxSize(300,-1));
	bszr->Add(txtSnapPath, flags);
	btnSnapPath = new MyButton(page, IDC_BTN_SNAP_PATH, CMsg::Folder_);
	bszr->Add(btnSnapPath, flags);

	szrMain->Add(bszr, flags);

#if 0
	// Font path
	bszr = new wxBoxSizer(wxHORIZONTAL);
	bszr->Add(new MyStaticText(page, wxID_ANY, CMsg::Font_Path), flags);
	txtFontPath = new wxTextCtrl(page, IDC_FONT_FILE, wxT(""), wxDefaultPosition, wxSize(300,-1));
	bszr->Add(txtFontPath, flags);
	btnFontPath = new MyButton(page, IDC_BTN_FONT_FILE, CMsg::Folder_);
	bszr->Add(btnFontPath, flags);

	szrMain->Add(bszr, flags);
#else
	txtFontPath = NULL;
	btnFontPath = NULL;
#endif

	// Message Font
	bszr = new wxBoxSizer(wxHORIZONTAL);
	bszr->Add(new MyStaticText(page, wxID_ANY, CMsg::Message_Font), flags);
	txtMsgFont = new wxTextCtrl(page, IDC_MSG_FONT_NAME_S);
	bszr->Add(txtMsgFont, flags);
	bszr->Add(new MyStaticText(page, wxID_ANY, CMsg::_Size), flags);
	txtMsgSize = new wxTextCtrl(page, IDC_MSG_FONT_SIZE_S, wxT(""), wxDefaultPosition, wxSize(36,-1), wxTE_RIGHT);
	bszr->Add(txtMsgSize, flags);
	btnMsgFont = new MyButton(page, IDC_BTN_FONT_NAME_S, CMsg::Font_);
	bszr->Add(btnMsgFont, flags);

	szrMain->Add(bszr, flags);

	// Info Font
	bszr = new wxBoxSizer(wxHORIZONTAL);
	bszr->Add(new MyStaticText(page, wxID_ANY, CMsg::Info_Font), flags);
	txtInfoFont = new wxTextCtrl(page, IDC_MSG_FONT_NAME_L);
	bszr->Add(txtInfoFont, flags);
	bszr->Add(new MyStaticText(page, wxID_ANY, CMsg::_Size), flags);
	txtInfoSize = new wxTextCtrl(page, IDC_MSG_FONT_SIZE_L, wxT(""), wxDefaultPosition, wxSize(36,-1), wxTE_RIGHT);
	bszr->Add(txtInfoSize, flags);
	btnInfoFont = new MyButton(page, IDC_BTN_FONT_NAME_L, CMsg::Font_);
	bszr->Add(btnInfoFont, flags);

	szrMain->Add(bszr, flags);

	//
	lang_list.Clear();
	clocale->GetLocaleNamesWithDefault(lang_list);

	bszr = new wxBoxSizer(wxHORIZONTAL);
	bszr->Add(new MyStaticText(page, wxID_ANY, CMsg::Language_ASTERISK), flags);
	comLanguage = new MyChoice(page, IDC_COMBO_LANGUAGE, wxDefaultPosition, wxDefaultSize, lang_list);
	bszr->Add(comLanguage, flags);

	szrMain->Add(bszr, flags);

	//
	bszr = new wxBoxSizer(wxVERTICAL);
	bszr->Add(new MyStaticText(page, wxID_ANY, CMsg::Need_restart_program), flags);

	szrMain->Add(bszr, flags);
	page->SetSizerAndFit(szrMain);

	// 2: Tape, FDD
	page = new wxPanel(book);
	book->AddPageById(page, CMsg::Tape_FDD);

	szrMain = new wxBoxSizer(wxVERTICAL);
	szrSub = new wxBoxSizer(wxHORIZONTAL);

	szrLeft = new wxBoxSizer(wxVERTICAL);

	// Load wav file
	wxBoxSizer *szrLoadWav;

	szrLoadWav = new wxStaticBoxSizer(new MyStaticBox(page, wxID_ANY, CMsg::Load_Wav_File_from_Tape), wxVERTICAL);
	bszr = new wxBoxSizer(wxVERTICAL);
	chkWavReverse = new MyCheckBox(page, IDC_CHK_REVERSE, CMsg::Reverse_Wave);
	bszr->Add(chkWavReverse, flags);
	chkWavHalf = new MyCheckBox(page, IDC_CHK_HALFWAVE, CMsg::Half_Wave);
	bszr->Add(chkWavHalf, flags);
	bszr2 = new wxBoxSizer(wxHORIZONTAL);
	bszr2->Add(new MyStaticText(page, wxID_ANY, CMsg::Correct), flags);
	radWavNoCorrect = new MyRadioButton(page, IDC_RADIO_NOCORR, CMsg::None_, wxDefaultPosition,wxDefaultSize,wxRB_GROUP);
	bszr2->Add(radWavNoCorrect, flags);
	radWavCorrectCos = new MyRadioButton(page, IDC_RADIO_COSW, CMsg::COS_Wave, wxDefaultPosition,wxDefaultSize);
	bszr2->Add(radWavCorrectCos, flags);
	radWavCorrectSin = new MyRadioButton(page, IDC_RADIO_SINW, CMsg::SIN_Wave, wxDefaultPosition,wxDefaultSize);
	bszr2->Add(radWavCorrectSin, flags);
	bszr->Add(bszr2, flags);
	bszr2 = new wxBoxSizer(wxHORIZONTAL);
	for(int i=0; i<2; i++) {
		bszr2->Add(new wxStaticText(page, wxID_ANY, i == 0 ? wxT("1200Hz") : wxT("2400Hz")), flags);
		txtCorrectAmp[i] = new wxTextCtrl(page, IDC_TXT_CORRAMP0 + i, wxT(""), wxDefaultPosition, wxSize(48,-1), wxTE_RIGHT);
		bszr2->Add(txtCorrectAmp[i], flags);
	}
	bszr->Add(bszr2, flags);
	szrLoadWav->Add(bszr, flags);
	szrLeft->Add(szrLoadWav);

	szrRight = new wxBoxSizer(wxVERTICAL);

	// Save wav file
	wxBoxSizer *szrSaveWav;

	szrSaveWav = new wxStaticBoxSizer(new MyStaticBox(page, wxID_ANY, CMsg::Save_Wav_File_to_Tape), wxVERTICAL);
	bszr = new wxBoxSizer(wxVERTICAL);
	bszr2 = new wxBoxSizer(wxHORIZONTAL);
	bszr2->Add(new MyStaticText(page, wxID_ANY, CMsg::Sample_Rate), flags);
	comWavSampleRate = new MyChoice(page, IDC_COMBO_SRATE, wxDefaultPosition, wxDefaultSize, LABELS::wav_sampling_rate, 0);
	bszr2->Add(comWavSampleRate, flags);
	bszr->Add(bszr2, flags);
	bszr3 = new wxBoxSizer(wxHORIZONTAL);
	bszr3->Add(new MyStaticText(page, wxID_ANY, CMsg::Sample_Bits), flags);
	comWavSampleBits = new MyChoice(page, IDC_COMBO_SBITS, wxDefaultPosition, wxDefaultSize, LABELS::wav_sampling_bits, 0);
	bszr3->Add(comWavSampleBits, flags);
	bszr->Add(bszr3, flags);
	szrSaveWav->Add(bszr, flags);
	szrRight->Add(szrSaveWav);

	szrSub->Add(szrLeft, flags);
	szrSub->Add(szrRight, flags);
	szrMain->Add(szrSub, flags);

	// FDD
	wxBoxSizer *szrFdd;

	szrFdd = new wxStaticBoxSizer(new MyStaticBox(page, wxID_ANY, CMsg::Floppy_Disk_Drive), wxVERTICAL);
	bszr = new wxBoxSizer(wxHORIZONTAL);
	bszr->Add(new MyStaticText(page, wxID_ANY, CMsg::When_start_up_mount_disk_at_), flags);
	for(int drv=0; drv<USE_FLOPPY_DISKS; drv++) {
		wxString str = wxString::Format("%d", drv);
		chkFDMount[drv] = new wxCheckBox(page, IDC_CHK_FDD_MOUNT0 + drv, str);
		bszr->Add(chkFDMount[drv], flags);
	}
	szrFdd->Add(bszr, flags);
	chkDelayFd1 = new MyCheckBox(page, IDC_CHK_DELAYFD1, CMsg::Ignore_delays_to_find_sector);
	szrFdd->Add(chkDelayFd1, flags);
	chkDelayFd2 = new MyCheckBox(page, IDC_CHK_DELAYFD2, CMsg::Ignore_delays_to_seek_track);
	szrFdd->Add(chkDelayFd2, flags);
	chkFdDensity = new MyCheckBox(page, IDC_CHK_FDDENSITY, CMsg::Suppress_checking_for_density);
	szrFdd->Add(chkFdDensity, flags);
	chkFdMedia = new MyCheckBox(page, IDC_CHK_FDMEDIA, CMsg::Suppress_checking_for_media_type);
	szrFdd->Add(chkFdMedia, flags);
	chkFdSavePlain = new MyCheckBox(page, IDC_CHK_SAVE_FDPLAIN, CMsg::Save_a_plain_disk_image_as_it_is);
	szrFdd->Add(chkFdSavePlain, flags);

	szrMain->Add(szrFdd, flags);

	page->SetSizerAndFit(szrMain);

	// 3: Network
	page = new wxPanel(book);
	book->AddPageById(page, CMsg::Network);

	szrMain = new wxBoxSizer(wxVERTICAL);

	for(int i=0; i<MAX_PRINTER; i++) {
		bszr = new wxBoxSizer(wxHORIZONTAL);
		wxString name = wxString::Format(CMSG(LPTVDIGIT_Hostname), i);
		bszr->Add(new wxStaticText(page, wxID_ANY, name), flags);
		txtHostLPT[i] = new wxTextCtrl(page, IDC_HOSTNAME_LPT0 + i);
		bszr->Add(txtHostLPT[i], flags);
		bszr->Add(new MyStaticText(page, wxID_ANY, CMsg::_Port), flags);
		txtPortLPT[i] = new wxTextCtrl(page, IDC_PORT_LPT0 + i, wxT(""), wxDefaultPosition, wxSize(64,-1), wxTE_RIGHT);
		bszr->Add(txtPortLPT[i], flags);
		bszr->Add(new MyStaticText(page, wxID_ANY, CMsg::_Print_delay), flags);
		txtDelayLPT[i] = new wxTextCtrl(page, IDC_DELAY_LPT0 + i, wxT(""), wxDefaultPosition, wxSize(48,-1), wxTE_RIGHT);
		bszr->Add(txtDelayLPT[i], flags);
		bszr->Add(new MyStaticText(page, wxID_ANY, CMsg::msec), flags);
		szrMain->Add(bszr, flags);
	}
	for(int i=0; i<MAX_COMM; i++) {
		bszr = new wxBoxSizer(wxHORIZONTAL);
		wxString name = wxString::Format(CMSG(COMVDIGIT_Hostname), i);
		bszr->Add(new wxStaticText(page, wxID_ANY, name), flags);
		txtHostCOM[i] = new wxTextCtrl(page, IDC_HOSTNAME_COM0 + i);
		bszr->Add(txtHostCOM[i], flags);
		bszr->Add(new MyStaticText(page, wxID_ANY, CMsg::_Port), flags);
		txtPortCOM[i] = new wxTextCtrl(page, IDC_PORT_COM0 + i, wxT(""), wxDefaultPosition, wxSize(64,-1), wxTE_RIGHT);
		bszr->Add(txtPortCOM[i], flags);
		comCOMBaud[i] = new MyChoice(page, IDC_COMBO_COM0 + i, wxDefaultPosition, wxDefaultSize, LABELS::comm_baud);
		bszr->Add(comCOMBaud[i], flags);

		szrMain->Add(bszr, flags);
	}
	// uart
	wxSize sz(120, -1);
	wxStaticBoxSizer *szrUart = new wxStaticBoxSizer(new MyStaticBox(page, wxID_ANY, CMsg::Settings_of_serial_ports_on_host), wxVERTICAL);
	bszr = new wxBoxSizer(wxHORIZONTAL);
	bszr->Add(new MyStaticText(page, wxID_ANY, CMsg::Baud_Rate, wxDefaultPosition, sz));
	comUartBaud = new MyChoice(page, IDC_COMBO_UART_BAUDRATE, wxDefaultPosition, wxDefaultSize, LABELS::comm_uart_baudrate);
	bszr->Add(comUartBaud);
	szrUart->Add(bszr, flags);
	bszr = new wxBoxSizer(wxHORIZONTAL);
	bszr->Add(new MyStaticText(page, wxID_ANY, CMsg::Data_Bit, wxDefaultPosition, sz));
	comUartDataBit = new MyChoice(page, IDC_COMBO_UART_DATABIT, wxDefaultPosition, wxDefaultSize, LABELS::comm_uart_databit);
	bszr->Add(comUartDataBit);
	szrUart->Add(bszr, flags);
	bszr = new wxBoxSizer(wxHORIZONTAL);
	bszr->Add(new MyStaticText(page, wxID_ANY, CMsg::Parity, wxDefaultPosition, sz));
	comUartParity = new MyChoice(page, IDC_COMBO_UART_PARITY, wxDefaultPosition, wxDefaultSize, LABELS::comm_uart_parity);
	bszr->Add(comUartParity);
	szrUart->Add(bszr, flags);
	bszr = new wxBoxSizer(wxHORIZONTAL);
	bszr->Add(new MyStaticText(page, wxID_ANY, CMsg::Stop_Bit, wxDefaultPosition, sz));
	comUartStopBit = new MyChoice(page, IDC_COMBO_UART_STOPBIT, wxDefaultPosition, wxDefaultSize, LABELS::comm_uart_stopbit);
	bszr->Add(comUartStopBit);
	szrUart->Add(bszr, flags);
	bszr = new wxBoxSizer(wxHORIZONTAL);
	bszr->Add(new MyStaticText(page, wxID_ANY, CMsg::Flow_Control, wxDefaultPosition, sz));
	comUartFlowCtrl = new MyChoice(page, IDC_COMBO_UART_FLOWCTRL, wxDefaultPosition, wxDefaultSize, LABELS::comm_uart_flowctrl);
	bszr->Add(comUartFlowCtrl);
	szrUart->Add(bszr, flags);
	szrUart->Add(new MyStaticText(page, wxID_ANY, CMsg::Need_re_connect_to_serial_port_when_modified_this));
	szrMain->Add(szrUart, flags);

	page->SetSizerAndFit(szrMain);

	// 4: CPU, Memory
	page = new wxPanel(book);
	book->AddPageById(page, CMsg::CPU_Memory);

	szrMain = new wxBoxSizer(wxVERTICAL);

	// ROM path
	bszr = new wxBoxSizer(wxHORIZONTAL);
	bszr->Add(new MyStaticText(page, wxID_ANY, CMsg::ROM_Path_ASTERISK), flags);
	txtROMPath = new wxTextCtrl(page, IDC_ROM_PATH, wxT(""), wxDefaultPosition, wxSize(300,-1));
	bszr->Add(txtROMPath, flags);
	btnROMPath = new MyButton(page, IDC_BTN_ROM_PATH, CMsg::Folder_);
	bszr->Add(btnROMPath, flags);
	szrMain->Add(bszr, flags);

#if defined(_MBS1)
	bszr = new wxBoxSizer(wxHORIZONTAL);
	bszr->Add(new MyStaticText(page, wxID_ANY, CMsg::Extended_RAM_ASTERISK), flags);
	comExMem = new MyChoice(page, IDC_COMBO_EXMEM, wxDefaultPosition, wxDefaultSize, LABELS::exram_size);
	bszr->Add(comExMem, flags);
	bszr->Add(new MyStaticText(page, wxID_ANY, CMsg::LB_Now_SP), flags);
	bszr->Add(new MyStaticText(page, wxID_ANY, LABELS::exram_size[pConfig->exram_size_num]), flags);
	bszr->Add(new wxStaticText(page, wxID_ANY, wxT(")")), flags);
	szrMain->Add(bszr, flags);

	chkMemNoWait = new MyCheckBox(page, IDC_CHK_MEMNOWAIT, CMsg::No_wait_to_access_the_main_memory);
	szrMain->Add(chkMemNoWait, flags);
#else
	chkUseExMem = new MyCheckBox(page, IDC_CHK_EXMEM, CMsg::Use_Extended_Memory_64KB);
	szrMain->Add(chkUseExMem, flags);
#endif

	chkUndefOp = new MyCheckBox(page, IDC_CHK_UNDEFOP, CMsg::Show_message_when_the_CPU_fetches_undefined_opcode);
	szrMain->Add(chkUndefOp, flags);

#if defined(_MBS1)
# if defined(USE_Z80B_CARD)
	// Interrupt of Z80B Card
	bszr = new wxBoxSizer(wxHORIZONTAL);
	bszr->Add(new MyStaticText(page, wxID_ANY, CMsg::Connect_interrupt_signal_of_Z80B_Card_to_ASTERISK), flags);
	comZ80BIntr = new MyChoice(page, IDC_COMBO_Z80BCARD_IRQ, wxDefaultPosition, wxDefaultSize, LABELS::z80bcard_irq);
	bszr->Add(comZ80BIntr, flags);
	szrMain->Add(bszr, flags);
# elif defined(USE_MPC_68008)
	chkAddrErr = new MyCheckBox(page, IDC_CHK_ADDRERR, CMsg::Show_message_when_the_address_error_occured_in_MC68008);
	szrMain->Add(chkAddrErr, flags);
# endif
#endif

	chkClrCPUReg = new MyCheckBox(page, IDC_CHK_CLEAR_CPUREG, CMsg::Clear_CPU_registers_at_power_on);
	szrMain->Add(chkClrCPUReg, flags);

	//
	bszr = new wxBoxSizer(wxVERTICAL);
	bszr->Add(new MyStaticText(page, wxID_ANY, CMsg::Need_restart_program_or_PowerOn), flags);
	szrMain->Add(bszr, flags);

	page->SetSizerAndFit(szrMain);

#if defined(_MBS1)
	// 5: Sound
	page = new wxPanel(book);
	book->AddPageById(page, CMsg::Sound);

	szrMain = new wxBoxSizer(wxVERTICAL);

	// FM OPN
	bszr = new wxStaticBoxSizer(new MyStaticBox(page, wxID_ANY, CMsg::FM_Synthesis_Card_ASTERISK), wxVERTICAL);
	chkFMOPN = new MyCheckBox(page, IDC_CHK_EN_FMOPN, CMsg::Enable);
	bszr->Add(chkFMOPN, flags);
	bszr->Add(new MyStaticText(page, wxID_ANY, CMsg::IO_ports_are_FF1E_FF1F_FF16_FF17), flags);
	bszr2 = new wxBoxSizer(wxHORIZONTAL);
	bszr2->Add(new MyStaticText(page, wxID_ANY, CMsg::Sound_chip), flags);
	comFMOPN = new MyChoice(page, IDC_COMBO_FMOPN, wxDefaultPosition, wxDefaultSize, LABELS::type_of_soundcard, 0, emu->get_parami(VM::ParamChipTypeOnFmOpn), pConfig->type_of_fmopn, CMsg::LB_Now_RB);
	bszr2->Add(comFMOPN, flags);
	bszr->Add(bszr2, flags);
	szrMain->Add(bszr, flags);

	// EX PSG
	bszr = new wxStaticBoxSizer(new MyStaticBox(page, wxID_ANY, CMsg::Extended_PSG_Port_ASTERISK), wxVERTICAL);
	chkEXPSG = new MyCheckBox(page, IDC_CHK_EN_EXPSG, CMsg::Enable);
	bszr->Add(chkEXPSG, flags);
	bszr->Add(new MyStaticText(page, wxID_ANY, CMsg::IO_ports_are_FFE6_FFE7_FFEE_FFEF), flags);
	bszr2 = new wxBoxSizer(wxHORIZONTAL);
	bszr2->Add(new MyStaticText(page, wxID_ANY, CMsg::Sound_chip), flags);
	comEXPSG = new MyChoice(page, IDC_COMBO_USE_EXPSG, wxDefaultPosition, wxDefaultSize, LABELS::type_of_soundcard, 0, emu->get_parami(VM::ParamChipTypeOnExPsg), pConfig->type_of_expsg, CMsg::LB_Now_RB);
	bszr2->Add(comEXPSG, flags);
	bszr->Add(bszr2, flags);
	szrMain->Add(bszr, flags);

	// Interrupt of sound card
	bszr = new wxBoxSizer(wxHORIZONTAL);
	bszr->Add(new MyStaticText(page, wxID_ANY, CMsg::Connect_interrupt_signal_of_FM_synthesis_to_ASTERISK_ASTERISK), flags);
	comFMIntr = new MyChoice(page, IDC_COMBO_FMOPN_IRQ, wxDefaultPosition, wxDefaultSize, LABELS::fmopn_irq);
	bszr->Add(comFMIntr, flags);
	szrMain->Add(bszr, flags);

	szrMain->Add(new MyStaticText(page, wxID_ANY, CMsg::Need_restart_program), flags);
	szrMain->Add(new MyStaticText(page, wxID_ANY, CMsg::This_is_the_common_setting_both_FM_synthesis_card_and_extended_PSG_port), flags);

	page->SetSizerAndFit(szrMain);
#endif


	szrAll->Add(book, flags);

	// button
	wxSizer *szrButtons = CreateButtonSizer(wxOK|wxCANCEL);
	szrAll->Add(szrButtons, flags);


	UpdateDialog();

	SetSizerAndFit(szrAll);
//	SetSizer(szrAll);
}

void MyConfigDlg::UpdateDialog()
{
	int i;
	int fdd_type = emu->get_parami(VM::ParamFddType);
	int io_port = emu->get_parami(VM::ParamIOPort);

	chkPowerOff->SetValue(pConfig->use_power_off);
#if defined(_MBS1)
	for(i=0; i<2; i++) {
		staSysMode[i]->Show((pConfig->sys_mode & 1) == (1 - i));
		radSysMode[i]->SetValue((emu->get_parami(VM::ParamSysMode) & 1) == (1 - i));
	}
	chkDIPSwitch->SetValue((pConfig->dipswitch & 4) != 0);
#else
	staDIPSwitch[0]->Show((pConfig->dipswitch & 4) != 0);
	chkDIPSwitch[0]->SetValue((pConfig->dipswitch & 4) != 0);
#endif

	for(i=0; i<4; i++) {
		staFDDType[i]->Show(pConfig->fdd_type == i);
		radFDDType[i]->SetValue(fdd_type == i);
	}
	change_fdd_type(fdd_type);

	for(i=0; LABELS::io_port[i] != CMsg::End; i++) {
		int pos = LABELS::io_port_pos[i];
		if ((1 << pos) & IOPORT_MSK_ALL) {
			staIOPort[pos]->Show((pConfig->io_port & (1 << pos)) != 0);
			chkIOPort[pos]->SetValue((io_port & (1 << pos)) != 0);
		}
	}
#ifdef USE_OPENGL
	comGLUse->Select(pConfig->use_opengl);
	comGLFilter->Select(pConfig->gl_filter_type);
#else
	comGLUse->Select(0);
	comGLUse->Enable(false);
	comGLFilter->Select(0);
	comGLFilter->Enable(false);
#endif

	int led_show = gui->GetLedBoxPhase(-1);
	comLedShow->Select(led_show);
	comLedPos->Select(pConfig->led_pos);

#if defined(_MBS1)
	comDisptmg->Select(pConfig->disptmg_skew + 2);
	comCurdisp->Select(pConfig->curdisp_skew + 2);
#else
	comDisptmg->Select(pConfig->disptmg_skew);
	comCurdisp->Select(pConfig->curdisp_skew);
#endif

	comCapType->Select(pConfig->capture_type);

	txtSnapPath->SetValue(pConfig->snapshot_path.Get());
#if 0
	txtFontPath->SetValue(pConfig->font_path.Get());
#endif
	txtMsgFont->SetValue(pConfig->msgboard_msg_fontname.Get());
	txtMsgSize->SetValue(wxString::Format(wxT("%d"), pConfig->msgboard_msg_fontsize));
	txtInfoFont->SetValue(pConfig->msgboard_info_fontname.Get());
	txtInfoSize->SetValue(wxString::Format(wxT("%d"), pConfig->msgboard_info_fontsize));

	int lang_selidx = clocale->SelectLocaleNameIndex(lang_list, pConfig->language);
	comLanguage->Select(lang_selidx);

	chkWavReverse->SetValue(pConfig->wav_reverse != 0);
	chkWavHalf->SetValue(pConfig->wav_half != 0);
	radWavNoCorrect->SetValue(!pConfig->wav_correct);
	radWavCorrectCos->SetValue(pConfig->wav_correct && pConfig->wav_correct_type == 0);
	radWavCorrectSin->SetValue(pConfig->wav_correct && pConfig->wav_correct_type == 1);
//	if (pConfig->wav_correct == 1) {
//		chkWavCorrect->SetValue(true);
//		radWavCorrectCos->Enable(true);
//		radWavCorrectSin->Enable(true);
//	} else {
//		chkWavCorrect->SetValue(false);
//		radWavCorrectCos->Enable(false);
//		radWavCorrectSin->Enable(false);
//	}
	for(i=0; i<2; i++) {
		txtCorrectAmp[i]->SetValue(wxString::Format(wxT("%d"), pConfig->wav_correct_amp[i]));
	}

	comWavSampleRate->Select(pConfig->wav_sample_rate);
	comWavSampleBits->Select(pConfig->wav_sample_bits);

	for(i=0; i<USE_FLOPPY_DISKS; i++) {
		chkFDMount[i]->SetValue((pConfig->mount_fdd & (1 << i)) != 0);
	}
	chkDelayFd1->SetValue(FLG_DELAY_FDSEARCH != 0);
	chkDelayFd2->SetValue(FLG_DELAY_FDSEEK != 0);
	chkFdDensity->SetValue(FLG_CHECK_FDDENSITY == 0);
	chkFdMedia->SetValue(FLG_CHECK_FDMEDIA == 0);
	chkFdSavePlain->SetValue(FLG_SAVE_FDPLAIN != 0);

	for(i=0; i<MAX_PRINTER; i++) {
		txtHostLPT[i]->SetValue(pConfig->printer_server_host[i].Get());
		txtPortLPT[i]->SetValue(wxString::Format(wxT("%d"), pConfig->printer_server_port[i]));
		txtDelayLPT[i]->SetValue(wxString::Format(wxT("%.1f"), pConfig->printer_delay[i]));
	}
	for(i=0; i<MAX_COMM; i++) {
		txtHostCOM[i]->SetValue(pConfig->comm_server_host[i].Get());
		txtPortCOM[i]->SetValue(wxString::Format(wxT("%d"), pConfig->comm_server_port[i]));
		comCOMBaud[i]->Select(pConfig->comm_dipswitch[i] - 1);
	}

	int match = 0;
	for(i=0; LABELS::comm_uart_baudrate[i] != NULL; i++) {
		if (pConfig->comm_uart_baudrate == (int)_tcstol(LABELS::comm_uart_baudrate[i], NULL, 10)) {
			match = i;
			break;
		}
	}
	comUartBaud->Select(match);
	comUartDataBit->Select(pConfig->comm_uart_databit - 7);
	comUartParity->Select(pConfig->comm_uart_parity);
	comUartStopBit->Select(pConfig->comm_uart_stopbit);
	comUartFlowCtrl->Select(pConfig->comm_uart_flowctrl);

	txtROMPath->SetValue(pConfig->rom_path.Get());

#if defined(_MBS1)
	comExMem->Select(emu->get_parami(VM::ParamExMemNum));
	chkMemNoWait->SetValue(pConfig->mem_nowait);
#else
	chkUseExMem->SetValue(pConfig->exram_size_num == 1);
#endif
	chkUndefOp->SetValue(FLG_SHOWMSG_UNDEFOP != 0);
	chkClrCPUReg->SetValue(FLG_CLEAR_CPUREG != 0);

#if defined(_MBS1)
# if defined(USE_Z80B_CARD)
	comZ80BIntr->Select(pConfig->z80b_card_out_irq);
# elif defined(USE_MPC_68008)
	chkAddrErr->SetValue(FLG_SHOWMSG_ADDRERR != 0);
# endif
	chkFMOPN->SetValue(IOPORT_USE_FMOPN != 0);
	comFMOPN->Select(emu->get_parami(VM::ParamChipTypeOnFmOpn));
	chkEXPSG->SetValue(IOPORT_USE_EXPSG != 0);
	comEXPSG->Select(emu->get_parami(VM::ParamChipTypeOnExPsg));
	comFMIntr->Select(pConfig->opn_irq);
#endif
}

int MyConfigDlg::ShowModal()
{
//	InitDialog();
	int rc = MyDialog::ShowModal();
	if (rc == wxID_OK) {
		ModifyParam();
	}
	return rc;
}

void MyConfigDlg::ModifyParam()
{
	int i;
	int fdd_type = 0;
	int io_port = 0;
	unsigned long value;

	for(i=0; i<3; i++) {
		if (radFDDType[i]->GetValue()) {
			fdd_type = i;
			break;
		}
	}
	for(i=0; LABELS::io_port[i] != CMsg::End; i++) {
		int pos = LABELS::io_port_pos[i];
		if ((1 << pos) & IOPORT_MSK_ALL) {
			if (chkIOPort[pos]->GetValue()) {
				io_port |= (1 << pos);
			}
		}
	}

	pConfig->mount_fdd = 0;
	for(i=0; i<USE_FLOPPY_DISKS; i++) {
		if (chkFDMount[i]->GetValue()) {
			pConfig->mount_fdd |= (1 << i);
		}
	}
	pConfig->option_fdd = (chkDelayFd1->GetValue() ? MSK_DELAY_FDSEARCH : 0)
		| (chkDelayFd2->GetValue() ? MSK_DELAY_FDSEEK : 0)
		| (chkFdDensity->GetValue() ? 0 : MSK_CHECK_FDDENSITY)
		| (chkFdMedia->GetValue() ? 0 : MSK_CHECK_FDMEDIA)
		| (chkFdDensity->GetValue() ? MSK_SAVE_FDPLAIN : 0);

	// immediate modify
	pConfig->use_power_off = chkPowerOff->GetValue();
#if defined(_MBS1)
	emu->set_parami(VM::ParamSysMode, (radSysMode[0]->GetValue() ? 1 : 0) | (pConfig->sys_mode & ~1));
	pConfig->dipswitch = chkDIPSwitch->GetValue() ? (pConfig->dipswitch | 4) : (pConfig->dipswitch & ~4);
#else
	pConfig->dipswitch = chkDIPSwitch[0]->GetValue() ? (pConfig->dipswitch | 4) : (pConfig->dipswitch & ~4);
#endif
	emu->set_parami(VM::ParamFddType, fdd_type);
	emu->set_parami(VM::ParamIOPort, io_port);

#ifdef USE_OPENGL
	pConfig->use_opengl = comGLUse->GetCurrentSelection();
	pConfig->gl_filter_type = comGLFilter->GetCurrentSelection();
#endif
#if defined(_MBS1)
	pConfig->disptmg_skew = comDisptmg->GetCurrentSelection() - 2;
	pConfig->curdisp_skew = comCurdisp->GetCurrentSelection() - 2;
#else
	pConfig->disptmg_skew = comDisptmg->GetCurrentSelection();
	pConfig->curdisp_skew = comCurdisp->GetCurrentSelection();
#endif
	pConfig->led_pos = comLedPos->GetCurrentSelection();

	pConfig->capture_type = comCapType->GetCurrentSelection();

	pConfig->snapshot_path.Set(txtSnapPath->GetValue());
#if 0
	pConfig->font_path.Set(txtFontPath->GetValue());
#endif
	pConfig->msgboard_msg_fontname.Set(txtMsgFont->GetValue());
	if (txtMsgSize->GetValue().ToULong(&value)) {
		if (1 <= value && value <= 60) {
			pConfig->msgboard_msg_fontsize = (uint8_t)value;
		}
	}
	pConfig->msgboard_info_fontname.Set(txtInfoFont->GetValue());
	if (txtInfoSize->GetValue().ToULong(&value)) {
		if (1 <= value && value <= 60) {
			pConfig->msgboard_info_fontsize = (uint8_t)value;
		}
	}

	clocale->ChooseLocaleName(lang_list, comLanguage->GetCurrentSelection(), pConfig->language);

	pConfig->wav_reverse = chkWavReverse->GetValue() ? 1 : 0;
	pConfig->wav_half = chkWavHalf->GetValue() ? 1 : 0;
	pConfig->wav_correct = !radWavNoCorrect->GetValue();
	pConfig->wav_correct_type = radWavCorrectSin->GetValue() ? 1 : 0;
	for(i=0; i<2; i++) {
		if (txtCorrectAmp[i]->GetValue().ToULong(&value)) {
			if (100 <= value && value <= 5000) {
				pConfig->wav_correct_amp[i] = (int)value;
			}
		}
	}
	pConfig->wav_sample_rate = comWavSampleRate->GetCurrentSelection();
	pConfig->wav_sample_bits = comWavSampleBits->GetCurrentSelection();

	for(i=0; i<MAX_PRINTER; i++) {
		pConfig->printer_server_host[i].Set(txtHostLPT[i]->GetValue());
		if (txtPortLPT[i]->GetValue().ToULong(&value)) {
			if (1 <= value && value <= 65535) {
				pConfig->printer_server_port[i] = (int)value;
			}
		}
		double valued = 0.0;
		if (txtDelayLPT[i]->GetValue().ToDouble(&valued)) {
			if (valued < 0.1) valued = 0.1;
			if (valued > 1000.0) valued = 1000.0;
			valued = floor(valued * 10.0 + 0.5) / 10.0;
			pConfig->printer_delay[i] = valued;
		}
	}
	for(i=0; i<MAX_COMM; i++) {
		pConfig->comm_server_host[i].Set(txtHostCOM[i]->GetValue());
		if (txtPortCOM[i]->GetValue().ToULong(&value)) {
			if (1 <= value && value <= 65535) {
				pConfig->comm_server_port[i] = (int)value;
			}
		}
		pConfig->comm_dipswitch[i] = comCOMBaud[i]->GetCurrentSelection() + 1;
	}

	i = comUartBaud->GetCurrentSelection();
	pConfig->comm_uart_baudrate = (int)_tcstol(LABELS::comm_uart_baudrate[i], NULL, 10);
	pConfig->comm_uart_databit = comUartDataBit->GetCurrentSelection() + 7;
	pConfig->comm_uart_parity = comUartParity->GetCurrentSelection();
	pConfig->comm_uart_stopbit = comUartStopBit->GetCurrentSelection();
	pConfig->comm_uart_flowctrl = comUartFlowCtrl->GetCurrentSelection();

	pConfig->rom_path.Set(txtROMPath->GetValue());

#if defined(_MBS1)
	emu->set_parami(VM::ParamExMemNum, comExMem->GetCurrentSelection());
	pConfig->mem_nowait = chkMemNoWait->GetValue();
#else
	pConfig->exram_size_num = chkUseExMem->GetValue() ? 1 : 0;
#endif
	BIT_ONOFF(pConfig->misc_flags, MSK_SHOWMSG_UNDEFOP, chkUndefOp->GetValue());
	BIT_ONOFF(pConfig->misc_flags, MSK_CLEAR_CPUREG, chkClrCPUReg->GetValue());

#if defined(_MBS1)
# if defined(USE_Z80B_CARD)
	pConfig->z80b_card_out_irq = comZ80BIntr->GetCurrentSelection();
# elif defined(USE_MPC_68008)
	BIT_ONOFF(pConfig->misc_flags, MSK_SHOWMSG_ADDRERR, chkAddrErr->GetValue());
# endif
	emu->set_parami(VM::ParamChipTypeOnFmOpn, comFMOPN->GetCurrentSelection());
	emu->set_parami(VM::ParamChipTypeOnExPsg, comEXPSG->GetCurrentSelection());
	pConfig->opn_irq = comFMIntr->GetCurrentSelection();
#endif

	// set message font
#ifdef USE_MESSAGE_BOARD
	MsgBoard *msgboard = emu->get_msgboard();
	if (msgboard) {
		msgboard->SetFont();
	}
#endif

	gui->ChangeLedBox(comLedShow->GetCurrentSelection());
	gui->ChangeLedBoxPosition(pConfig->led_pos);
}

void MyConfigDlg::change_fdd_type(int index)
{
#ifdef USE_IOPORT_FDD
	switch(index) {
	case FDD_TYPE_NOFDD:
		// no fdd
		chkIOPort[0]->SetValue(false);
		chkIOPort[1]->SetValue(false);
		chkIOPort[0]->Enable(true);
		chkIOPort[1]->Enable(true);
		break;
	case FDD_TYPE_3FDD:
		// 3 inch
		chkIOPort[0]->SetValue(false);
		chkIOPort[1]->SetValue(true);
		chkIOPort[0]->Enable(false);
		chkIOPort[1]->Enable(false);
		break;
	case FDD_TYPE_5FDD:
	case FDD_TYPE_8FDD:
		// 5 inch
		chkIOPort[0]->SetValue(true);
		chkIOPort[1]->SetValue(false);
		chkIOPort[0]->Enable(false);
		chkIOPort[1]->Enable(false);
		break;
	}
#endif
}

void MyConfigDlg::change_io_port(int index)
{
#ifdef USE_IOPORT_FDD
	if (index == 0) {
		chkIOPort[0]->SetValue(true);
		chkIOPort[1]->SetValue(false);
	} else if (index == 1) {
		chkIOPort[0]->SetValue(false);
		chkIOPort[1]->SetValue(true);
	}
#endif
	if (index == IOPORT_POS_PSG9) {
//		chkIOPort[IOPORT_POS_PSG9]->SetValue(true);
		chkIOPort[IOPORT_POS_KANJI]->SetValue(false);
#if defined(_MBS1)
		chkIOPort[IOPORT_POS_CM01]->SetValue(false);
		chkIOPort[IOPORT_POS_KANJI2]->SetValue(false);
#endif
	} else if (index == IOPORT_POS_KANJI) {
		chkIOPort[IOPORT_POS_PSG9]->SetValue(false);
//		chkIOPort[IOPORT_POS_KANJI]->SetValue(true);
#if defined(_MBS1)
	} else if (index == IOPORT_POS_CM01) {
		chkIOPort[IOPORT_POS_PSG9]->SetValue(false);
//		chkIOPort[IOPORT_POS_CM01]->SetValue(true);
		chkIOPort[IOPORT_POS_KANJI]->SetValue(true);
	} else if (index == IOPORT_POS_KANJI2) {
		chkIOPort[IOPORT_POS_PSG9]->SetValue(false);
//		chkIOPort[IOPORT_POS_KANJI2]->SetValue(true);
#endif
	}
}

/*
 * Event Handler
 */

void MyConfigDlg::OnChangeCorrect(wxCommandEvent &event)
{
//	if (event.IsChecked()) {
//		radWavCorrectCos->Enable(true);
//		radWavCorrectSin->Enable(true);
//	} else {
//		radWavCorrectCos->Enable(false);
//		radWavCorrectSin->Enable(false);
//	}
}

void MyConfigDlg::OnChangeFddType(wxCommandEvent &event)
{
	int index = event.GetId() - IDC_RADIO_NOFDD;
	change_fdd_type(index);
}

void MyConfigDlg::OnChangeIOPort(wxCommandEvent &event)
{
	int index = event.GetId() - IDC_CHK_IOPORT1;
//	logging->out_debug(_T("MyConfigDlg::OnChangeIOPort: %d -> %d"),index,event.IsChecked() ? 1 : 0);
	if (event.IsChecked()) {
		change_io_port(index);
	}
#if defined(_MBS1)
	switch(index) {
	case IOPORT_POS_FMOPN:
		chkFMOPN->SetValue(chkIOPort[IOPORT_POS_FMOPN]->GetValue());
		break;
	case IOPORT_POS_EXPSG:
		chkEXPSG->SetValue(chkIOPort[IOPORT_POS_EXPSG]->GetValue());
		break;
	default:
		break;
	}
#endif
}

void MyConfigDlg::OnChangeSoundCard(wxCommandEvent &event)
{
#if defined(_MBS1)
	switch(event.GetId()) {
	case IDC_CHK_EN_FMOPN:
		chkIOPort[IOPORT_POS_FMOPN]->SetValue(chkFMOPN->GetValue());
		break;
	case IDC_CHK_EN_EXPSG:
		chkIOPort[IOPORT_POS_EXPSG]->SetValue(chkEXPSG->GetValue());
		break;
	default:
		break;
	}
#endif
}

void MyConfigDlg::OnShowFolderDialog(wxCommandEvent &event)
{
	int id = event.GetId();
	CMsg::Id msg_id = CMsg::Null;
	wxString path;

	switch(id) {
	case IDC_BTN_SNAP_PATH:
		path = txtSnapPath->GetValue();
		msg_id = CMsg::Select_a_folder_to_save_snapshot_images;
		break;
	case IDC_BTN_FONT_FILE:
		path = txtFontPath->GetValue();
		msg_id = CMsg::Select_a_font;
		break;
	case IDC_BTN_ROM_PATH:
		path = txtROMPath->GetValue();
		msg_id = CMsg::Select_a_folder_containing_the_rom_images;
		break;
	default:
		return;
	}

	wxDirDialog *dlg = new wxDirDialog(this, gMessages.Get(msg_id), path);
	if (dlg->ShowModal() == wxID_OK) {
		path = dlg->GetPath();
		switch(id) {
		case IDC_BTN_SNAP_PATH:
			txtSnapPath->SetValue(path);
			break;
		case IDC_BTN_FONT_FILE:
			txtFontPath->SetValue(path);
			break;
		case IDC_BTN_ROM_PATH:
			txtROMPath->SetValue(path);
			break;
		}
	}
	delete dlg;
}

void MyConfigDlg::OnShowFontDialog(wxCommandEvent &event)
{
	int id = event.GetId();
	wxFont font;
	wxFontData fontdata;
	unsigned long value = 0;

	switch(id) {
	case IDC_BTN_FONT_NAME_S:
		font.SetFaceName(txtMsgFont->GetValue());
		if (!(txtMsgSize->GetValue().ToULong(&value) && 1 <= value && value <= 60)) {
			value = 12;
		}
		font.SetPointSize((int)value);
		break;
	case IDC_BTN_FONT_NAME_L:
		font.SetFaceName(txtInfoFont->GetValue());
		if (!(txtInfoSize->GetValue().ToULong(&value) && 1 <= value && value <= 60)) {
			value = 12;
		}
		font.SetPointSize((int)value);
		break;
	default:
		return;
	}
	fontdata.SetChosenFont(font);

	wxFontDialog *dlg = new wxFontDialog(this, fontdata);
	if (dlg->ShowModal() == wxID_OK) {
		fontdata = dlg->GetFontData();
		font = fontdata.GetChosenFont();
		switch(id) {
		case IDC_BTN_FONT_NAME_S:
			txtMsgFont->SetValue(font.GetFaceName());
			txtMsgSize->SetValue(wxString::Format(wxT("%d"), font.GetPointSize()));
			break;
		case IDC_BTN_FONT_NAME_L:
			txtInfoFont->SetValue(font.GetFaceName());
			txtInfoSize->SetValue(wxString::Format(wxT("%d"), font.GetPointSize()));
			break;
		}
	}
	delete dlg;
}
