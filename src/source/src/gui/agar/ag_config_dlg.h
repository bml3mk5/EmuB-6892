/** @file ag_config_dlg.h

	Skelton for retropc emulator
	SDL + Agar edition

	@author Sasaji
	@date   2012.04.08

	@brief [ ag_config_dlg ]
*/

#ifndef AG_CONFIG_DLG_H
#define AG_CONFIG_DLG_H

#include "ag_dlg.h"
#include "../../vm/vm.h"
#include "ag_dir_dlg.h"
#include "../../config.h"
#include "../../cchar.h"
#include "../../cptrlist.h"

namespace GUI_AGAR
{

/**
	@brief config dialog
*/
class AG_CONFIG_DLG : public AG_DLG {
	friend class AG_GUI_BASE;

private:
	typedef struct ConfigDlgParam_st {
		int  use_power_off;
#if defined(_MBS1)
		int  sys_mode_rev;
#endif
		int  dipswitch;

		int  fdd_type;
		int  mount_fdd;
		int  delayfd1;
		int  delayfd2;
		int  chk_fddensity;
		int  chk_fdmedia;

		int  wav_reverse;
		int  wav_half;
		int  wav_correct;
		char wav_correct_amp[2][8];

		int  wav_sample_rate;
		int  wav_sample_bits;

		int  io_port;

		int  disptmg_skew;
		int  curdisp_skew;

		int  use_opengl;
		int  gl_filter_type;
		int  led_show;
		int  led_pos;
		int  capture_type;

		int  showmsg_undefop;
		int  clear_cpureg;

#if defined(_MBS1)
		int  mem_nowait;
//		int  fmopn_clock_num;
		int  fmopn_irq;
		int  type_of_fmopn;
		int  type_of_expsg;
#else
		int  exram_size_num;
#endif
#if defined(_MBS1)
# if defined(USE_Z80B_CARD)
		int  z80b_card_out_irq;
# elif defined(USE_MPC_68008)
		int  showmsg_addrerr;
# endif
#endif
		int  lang_selidx;

		char msg_font_name[_MAX_PATH];
		char msg_font_size[8];
		char info_font_name[_MAX_PATH];
		char info_font_size[8];
		char menu_font_name[_MAX_PATH];
		char menu_font_size[8];
		char font_path[_MAX_PATH];
		char snap_path[_MAX_PATH];

		char rom_path[_MAX_PATH];

		char prn_host[MAX_PRINTER][_MAX_PATH];
		char prn_port[MAX_PRINTER][8];
		char prn_delay[MAX_PRINTER][16];
		char com_host[MAX_COMM][_MAX_PATH];
		char com_port[MAX_COMM][8];
		int  com_dipsw[MAX_COMM];
#ifdef USE_DEBUGGER
		char dbgr_host[_MAX_PATH];
		char dbgr_port[8];
#endif
		int  uart_baud_index;
		int  uart_databit;
		int  uart_parity;
		int  uart_stopbit;
		int  uart_flowctrl;

#if defined(_MBS1)
		int  exram_num;
#endif
	} ConfigDlgParam;

	ConfigDlgParam param;
	ConfigDlgParam *paramtmp;

#if defined(_MBS1)
	char *radSysMode[3];
	char radSysModeBase[3][128];
#endif

	char *chkDipSwitch[1];
	char chkDipSwitchBase[1][128];

	char *radFddType[5];
	char radFddTypeBase[5][128];

	AG_Checkbox *chkIOPort[IOPORT_NUMS];
	char chkIOPortBase[IOPORT_NUMS][128];

	AG_DIR_DLG *dirdlg;

	CPtrList<CTchar> lang_list;

	enum DirDlgType {
		RomPath,
		SnapShotPath,
		FontPath
	};

	void change_fdd_type(int);
	void change_io_port(int, int);

	void show_dir_dlg(AG_Window *win, DirDlgType type);

	void set_label(char *label, const char *str);
	void set_label(char *label, CMsg::Id msg_id);

	static void OnOk(AG_Event *);
	static void OnClose(AG_Event *);
	static void OnChangeCorrect(AG_Event *);
	static void OnChangeSampleRate(AG_Event *);
	static void OnChangeSampleBits(AG_Event *);
	static void OnChangeFddType(AG_Event *);
	static void OnChangeIOPort(AG_Event *);
	static void OnChangeDisptmgSkew(AG_Event *);
	static void OnChangeCurdispSkew(AG_Event *);
	static void OnChangeUseOpenGL(AG_Event *);
	static void OnChangeGLFilterType(AG_Event *);
	static void OnChangeLedShow(AG_Event *);
	static void OnChangeLedPos(AG_Event *);
	static void OnChangeCaptureType(AG_Event *);
	static void OnChangeCommBaud(AG_Event *);
	static void OnChangeCommUartBaud(AG_Event *);
	static void OnChangeCommUartDataBit(AG_Event *);
	static void OnChangeCommUartParity(AG_Event *);
	static void OnChangeCommUartStopBit(AG_Event *);
	static void OnChangeCommUartFlowCtrl(AG_Event *);
#if defined(_MBS1)
	static void OnChangeExtRam(AG_Event *);
//	static void OnChangeFmOpnClock(AG_Event *);
	static void OnChangeFmOpnIrq(AG_Event *);
	static void OnChangeFmOpnChip(AG_Event *);
	static void OnChangeExpsgChip(AG_Event *);
#if defined(USE_Z80B_CARD)
	static void OnChangeZ80BCardIrq(AG_Event *);
#endif
#endif
	static void OnChangeLanguage(AG_Event *);
	static void OnShowDirDlg(AG_Event *);

public:
	AG_CONFIG_DLG(EMU *, AG_GUI_BASE *);
	~AG_CONFIG_DLG();

	void Create();
	void Close(AG_Window *);
	int  SetData(AG_Window *);
};

}; /* namespace GUI_AGAR */

#endif /* AG_CONFIG_DLG_H */
