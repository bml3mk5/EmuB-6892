/** @file display.h

	HITACHI BASIC MASTER LEVEL3 Mark5 Emulator 'EmuB-6892'
	Skelton for retropc emulator

	@author Sasaji
	@date   2011.06.08 -

	@brief [ display ]
*/

#ifndef DISPLAY_H
#define DISPLAY_H

#include "../vm_defs.h"
//#include "../../emu.h"
#include "../device.h"

#define L3_ADDR_VRAM_START		0x0400
#define L3_ADDR_VRAM_END		0x4400
#define L3_VRAM_SIZE			0x4000
#define L3_VRAM_SIZE_1			0x3fff

#define ADDR_IGRAM_START	0xa000
#define ADDR_IGRAM_END		0xa800

#define IGRAM_SIZE			0x800

//#define CRT_MON_HEIGHT	524
//#define CRT_MON_HEIGHT	320
#define CRT_MON_HEIGHT 304

#define VRAM_BUF_HEIGHT	(SCREEN_HEIGHT >> 1)
#define VRAM_BUF_WIDTH	128

class EMU;
class HD46505;

/**
	@brief Display Monitor
*/
class DISPLAY : public DEVICE
{
public:
	/// @brief signals on DISPLAY
	enum SIG_DISPLAY_IDS {
		SIG_DISPLAY_LPSTB	= 1,
		SIG_DISPLAY_VSYNC	= 2,
		SIG_DISPLAY_WRITE_REGS	= 6
	};

private:
	HD46505* d_crtc;

	uint8_t* crtc_regs;
	uint16_t cursor_addr;
	int *crtc_curdisp;

	int   mode_sel;					///< ffd0
	bool  ig_mask;					///< compatible for MB-6890/6891

	uint8_t crtc_chr_clocks;		///< crtc clocks 0:1MHz 1:2MHz
	int   crtc_ppch;				///< crtc dot per char w.40:16 w.80:8

	// crtc reg8
	int  *videomode;				///< 1:videomode(interlace only)
	int  *disptmg_skew;				///< display timing skew
	int  *curdisp_skew;				///< cursor disp skew

	int   bg_color;					///< bg color (mode-sel bit0-bit2)
	int   hireso;					///< 0:normal 1:hireso (mode-sel bit6)
	int   width_sel;				///< 0:w.40 1:w.80 (mode-sel bit7)

	// synchronizable vertical range
	int v_total_min[3];
	int v_total_max[3];

	int dws_left_diff;
	// synchronizable horizontal range
	int h_total_min[4];
	int h_total_max[4];

	int sw;

	uint32_t Rmask;
	uint32_t Gmask;
	uint32_t Bmask;
	uint8_t  Rshift;
	uint8_t  Gshift;
	uint8_t  Bshift;
	uint32_t Rloss[2];
	uint32_t Gloss[2];
	uint32_t Bloss[2];
	uint32_t Rboad;
	uint32_t Gboad;
	uint32_t Bboad;

	uint8_t font[0x1000];
	uint8_t chrfont[2][256][16];
	uint8_t* l3vram;
	uint8_t* color_ram;
	uint8_t* ig_ram;

	// CRT monitor
	typedef struct st_crt_mon {
		uint8_t disptmg;	///< bit4: first raster  bit1: hsync start  bit0: disptmg area
		uint8_t text;		///< vram
		uint8_t color;		///< color ram
		uint8_t attr;		///< bg color
	} t_crt_mon;
	t_crt_mon crt_mon[VRAM_BUF_HEIGHT][VRAM_BUF_WIDTH];
	int crt_mon_v_count;
	int crt_mon_v_start;
	int crt_mon_col;
	int crt_mon_row;
	int crt_mon_l3vaddr;
#ifdef USE_KEEPIMAGE
	int crt_mon_page;
#endif
	int crt_mon_stepcols;
//	int crt_mon_top;

	bool crt_mon_crtc_hsync;
	int crt_mon_bg_color;

	typedef struct st_crt_mon_vline {
		// horizontal
		int16_t left_col;				///< display timing area left side
#ifdef USE_LIGHTPEN
		int16_t right_col;				///< display timing area right side
#endif
		uint8_t flags;					///< bit0:odd line  bit7:reserved
	} t_crt_mon_vline;
	t_crt_mon_vline crt_mon_vline[VRAM_BUF_HEIGHT];

	typedef struct st_crt_mon_vinfo {
		int16_t row;
		uint8_t raster;					///< bit0-5:rastar line
		uint8_t cursor_st_col;			///< cursor display position
	} t_crt_mon_vinfo;
	t_crt_mon_vinfo crt_mon_vinfo[SCREEN_HEIGHT];

#ifdef USE_LIGHTPEN
	int crt_mon_disptmg_top;
	int crt_mon_disptmg_bottom;
#endif

	int screen_left;
	int screen_right;
	int screen_top;
	int screen_bottom;
	int scrnline1_offset;
	scrntype scrnline1[SCREEN_HEIGHT][SCREEN_WIDTH];
	scrntype scrnline2[SCREEN_WIDTH];
	scrntype scrntop[SCREEN_HEIGHT][SCREEN_WIDTH];
	scrntype palette_pc[8];
	int  ppd[8];	// pixel per 1dot
	uint8_t dof[8];	// dot mask

	uint8_t scanline;
	int  afterimage;
#ifdef USE_KEEPIMAGE
	int  keepimage;
#endif
	int  buf_stepcols;

	int skip_frame_count;
	int *crtc_vt_total;		///< crtc total rows (dot unit)
	int *crtc_vt_count;		///< crtc now line
	int *crtc_vt_disp;		///< crtc can display rows (disptmg)
	int *crtc_ma;			///< crtc memory address
	int *crtc_ra;			///< crtc raster address
	int *crtc_max_ra;		///< crtc max raster address
	int *crtc_odd_line;		///< crtc odd line (videomode only)
	int *crtc_vs_start;		///< crtc vsync start line
	int *crtc_vs_end;		///< crtc vsync end line

	int crtc_vs_mid;

	int *vm_pause;

	//for resume
#pragma pack(1)
	struct vm_state_st {
		int   crtc_addr;
		uint8_t ig_enable;
		char    reserved0[3];
		uint8_t ig_mask;

		uint8_t chr_clocks;

		char  reserved[6];
	};
#pragma pack()

	int  font_rom_loaded;
	bool font_rom_loaded_at_first;

	// for draw_screen and update_vram
	uint8_t dws_p;
	uint8_t dws_code;
	uint8_t dws_c_f;
	uint8_t dws_c_b;
	uint8_t dws_ck_f;
	uint8_t dws_ck_b;
	uint8_t dws_cram;
	uint8_t dws_fg;
	uint8_t dws_i_r,dws_i_g,dws_i_b;
	int dws_bt;
	int dws_bi;
	int dws_dt, dws_dtm;
	int dws_x,dws_x0,dws_x1,dws_xodd;
	int dws_y,dws_y0,dws_y1,dws_ys,dws_yd;
	int dws_raster,dws_raster2;
	int dws_int_vid;
	int dws_videomode_n;
	bool dws_disp_cursor_char;
#ifdef USE_KEEPIMAGE
	bool dws_ki;
#endif
	int dws_buf_x;
	int dws_buf_y_h;
	t_crt_mon *dws_p_crt_mon;
	t_crt_mon *dws_p_crt_mon_x;
	t_crt_mon_vline *dws_p_vline;
	t_crt_mon_vinfo *dws_p_vinfo;

	int dws_scrn_offset;
	scrntype *dws_scrn0;
	scrntype *dws_scrn;
	scrntype dws_dot;
	scrntype *dws_scrnline1p;
	uint32_t dws_r,dws_g,dws_b;

	int crt_mon_disptmg_left;

	void load_font_rom_file();
	uint8_t get_font_data(int, int, int);

	void update_dws_params();
	void update_chr_clocks(int clk);

	void draw_screen_sub();
	void draw_screen_sub_afterimage1();
	void draw_screen_sub_afterimage2();
public:
	DISPLAY(VM* parent_vm, EMU* parent_emu, const char* identifier) : DEVICE(parent_vm, parent_emu, identifier)
	{
		set_class_name("DISPLAY");
	}
	~DISPLAY() {}

	// common functions
	void initialize();
	void reset();
	void update_config();
	void write_io8(uint32_t addr, uint32_t data);
	void write_signal(int id, uint32_t data, uint32_t mask);
	void event_frame();
	void update_display(int v, int clock);

	// unique function
	void set_context_crtc(HD46505* device) {
		d_crtc = device;
	}
	void set_vram_ptr(uint8_t* ptr) {
		// vram is main ram!
		l3vram = ptr;
	}
	void set_color_ram_ptr(uint8_t* ptr) {
		color_ram = ptr;
	}
	void set_ig_ram_ptr(uint8_t* ptr) {
		ig_ram = ptr;
	}
	void set_regs_ptr(uint8_t* ptr) {
		crtc_regs = ptr;
	}
	void set_crtc_vt_ptr(int* ptrt, int *ptrc, int *ptrd) {
		crtc_vt_total = ptrt;
		crtc_vt_count = ptrc;
		crtc_vt_disp  = ptrd;
	}
	void set_crtc_ma_ra_ptr(int *ptrma, int *ptrra) {
		crtc_ma = ptrma;
		crtc_ra = ptrra;
	}
	void set_crtc_max_ra_ptr(int *ptr) {
		crtc_max_ra = ptr;
	}
	void set_crtc_odd_line_ptr(int *ptr) {
		crtc_odd_line = ptr;
	}
	void set_crtc_reg8_ptr(int *ptrv, int *ptrds, int *ptrcs) {
		videomode = ptrv;
		disptmg_skew = ptrds;
		curdisp_skew = ptrcs;
	}
	void set_crtc_curdisp_ptr(int *ptr) {
		crtc_curdisp = ptr;
	}
	void set_crtc_vsync_ptr(int *ptrst, int *ptred) {
		crtc_vs_start = ptrst;
		crtc_vs_end = ptred;
	}
	void set_display_size(int left, int top, int right, int bottom);
	void draw_screen();

	void save_state(FILEIO *fio);
	bool load_state(FILEIO *fio);

#ifdef USE_DEBUGGER
	int  get_debug_graphic_memory_size(int num, int type, int *width, int *height);
	bool debug_graphic_type_name(int type, _TCHAR *buffer, size_t buffer_len);
	bool debug_draw_graphic(int type, int width, int height, scrntype *buffer);
	bool debug_dump_graphic(int type, int width, int height, uint16_t *buffer);
#endif
};

#endif /* DISPLAY_H */

