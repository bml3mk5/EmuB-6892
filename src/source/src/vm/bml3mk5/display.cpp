/** @file display.cpp

	HITACHI BASIC MASTER LEVEL3 Mark5 Emulator 'EmuB-6892'
	Skelton for retropc emulator

	@author Sasaji
	@date   2011.06.08 -

	@brief [ display ]
*/

#include "display.h"
#include <stdlib.h>
#include "../../emu.h"
#include "../hd46505.h"
#include "../../fileio.h"
#include "../../config.h"
#include "../../utility.h"

#define CURSOR_MAXPOS 0xffffff

#ifdef _DEBUG
//#define DEBUG_CRTMON
//#define DEBUG_CRTMON2
#endif

void DISPLAY::initialize()
{
	memset(font, 0, sizeof(font));
	font_rom_loaded = 0;
	font_rom_loaded_at_first = false;

	vm_pause = emu->get_pause_ptr();

	// load rom images
	load_font_rom_file();

	emu->get_rgbformat(&Rmask, &Gmask, &Bmask, &Rshift, &Gshift, &Bshift);

	// create pc palette
#if 1
	for(int i = 0; i < 8; i++) {
		palette_pc[i] = emu->map_rgbcolor((i & 2) ? 255 : 0, (i & 4) ? 255 : 0, (i & 1) ? 255 : 0);
		logging->out_debugf(_T("palette_pc[%d]:%x"),i,palette_pc[i]);
	}
#else
	palette_pc[0] = emu->map_rgbcolor(0x1f,0x1f,0x1f);
	palette_pc[1] = emu->map_rgbcolor(0x3f,0x3f,0x3f);
	palette_pc[2] = emu->map_rgbcolor(0x5f,0x5f,0x5f);
	palette_pc[3] = emu->map_rgbcolor(0x7f,0x7f,0x7f);
	palette_pc[4] = emu->map_rgbcolor(0x9f,0x9f,0x9f);
	palette_pc[5] = emu->map_rgbcolor(0xbf,0xbf,0xbf);
	palette_pc[6] = emu->map_rgbcolor(0xdf,0xdf,0xdf);
	palette_pc[7] = emu->map_rgbcolor(0xff,0xff,0xff);
#endif

	memset(scrntop, 0, sizeof(scrntop));

	screen_left = 0;
	screen_top = 0;
	screen_right = SCREEN_WIDTH + screen_left;
	screen_bottom = SCREEN_HEIGHT + screen_top;
	scrnline1_offset = 0;

	mode_sel = 0;
	ig_mask = true;

	cursor_addr = 0;

	update_chr_clocks(0);

	bg_color = 0;
	crt_mon_bg_color = 0;
	hireso = 0;
	width_sel = 0;

	// synchronizable vertical range
	v_total_min[0] = (262 - 12);
	v_total_max[0] = (262 + 18);
	v_total_min[1] = (486 - 12);
	v_total_max[1] = (486 + 18);
	v_total_min[2] = (518 - 12);
	v_total_max[2] = (518 + 18);

	for(int i=0; i<VRAM_BUF_HEIGHT; i++) {
		crt_mon_vline[i].left_col = 0;
#ifdef USE_LIGHTPEN
		crt_mon_vline[i].right_col = 0;
#endif
		crt_mon_vline[i].flags = 0;
	}
	for(int i=0; i<SCREEN_HEIGHT; i++) {
		crt_mon_vinfo[i].row = 0;
		crt_mon_vinfo[i].raster = 0;
		crt_mon_vinfo[i].cursor_st_col = 254;
	}
	dws_left_diff = 0;
	// synchronizable horizontal range
	for(int i=0; i<4; i++) {
		h_total_min[i] = CHARS_PER_LINE * (i+1) - 3;
		h_total_max[i] = CHARS_PER_LINE * (i+1) + 2;
	}

	sw = 0;

	//
	memset(crt_mon, 0, sizeof(crt_mon));
	crt_mon_col = 0;
	crt_mon_row = 0;
	crt_mon_v_count = 0;
	crt_mon_v_start = 0;
	crt_mon_l3vaddr = L3_ADDR_VRAM_START;
#ifdef USE_KEEPIMAGE
	crt_mon_page = 0;
#endif
#ifdef USE_LIGHTPEN
	crt_mon_disptmg_top = 0;
	crt_mon_disptmg_bottom = 0;
#endif

	skip_frame_count = 0;

	for(int i=0; i<8; i++) {
		ppd[i]=(mode_sel & 0x80) ? 1 : 2;	// pixel per 1dot
		dof[i]=(0x80 >> i);
	}
	// pixel per 1char
	crtc_ppch = (mode_sel & 0x80) ? 8 : 16;

	update_config();
}

void DISPLAY::reset()
{
	load_font_rom_file();

	REG_IGENABLE = 0;
	ig_mask = true;
}

void DISPLAY::update_config()
{
	scanline = pConfig->scan_line;
	afterimage = pConfig->afterimage;
#ifdef USE_KEEPIMAGE
	keepimage = pConfig->keepimage;
#endif
	buf_stepcols = (scanline <= 2 ? 1 : 2);
	crt_mon_stepcols = (scanline <= 2 ? 1 : 2);

	// scanline
//	dws_ys = (scanline ? 2 : 1);	// skip step
	dws_ys = (scanline > 1 ? 2 : 1);	// skip step

	// color filter for afterimage
	if (afterimage == 0) {
		memset(scrnline1, 0x00, sizeof(scrntype) * SCREEN_WIDTH);
		memset(scrnline2, 0xff, sizeof(scrnline2));
		scrnline1_offset = 0;
	} else if (afterimage == 1) {
		memset(scrnline2, 0xff, sizeof(scrnline2));
		scrnline1_offset = SCREEN_WIDTH;

		Rloss[0] = (uint32_t)pConfig->rloss[0] * (scanline <= 1 ? 1 : scanline);
		Rloss[1] = (uint32_t)pConfig->rloss[1] * (scanline <= 1 ? 1 : scanline);
		Gloss[0] = (uint32_t)pConfig->gloss[0] * (scanline <= 1 ? 1 : scanline);
		Gloss[1] = (uint32_t)pConfig->gloss[1] * (scanline <= 1 ? 1 : scanline);
		Bloss[0] = (uint32_t)pConfig->bloss[0] * (scanline <= 1 ? 1 : scanline);
		Bloss[1] = (uint32_t)pConfig->bloss[1] * (scanline <= 1 ? 1 : scanline);
		Rboad = (128 << Rshift);
		Gboad = (128 << Gshift);
		Bboad = (128 << Bshift);
		for(int i=0; i<2; i++) {
			if (Rloss[i] > 255) Rloss[i] = 255;
			if (Gloss[i] > 255) Gloss[i] = 255;
			if (Bloss[i] > 255) Bloss[i] = 255;
			Rloss[i] <<= Rshift;
			Gloss[i] <<= Gshift;
			Bloss[i] <<= Bshift;
		}
	} else if (afterimage == 2) {
		memset(scrnline2, 0xbf, sizeof(scrnline2));
		scrnline1_offset = SCREEN_WIDTH;
	}

#ifdef USE_KEEPIMAGE
	if (keepimage > 0) {
		memset(scrntop, 0, sizeof(scrntop));
	}
#endif

	update_dws_params();
}

void DISPLAY::update_dws_params()
{
//	crt_mon_disptmg_left = (*disptmg_skew) >= 4 ? VRAM_BUF_WIDTH
//		: crtc_regs[0]-crtc_regs[2]-(crtc_regs[3] & 0x0f)-(crtc_chr_clocks ? 14 : 5) + (*disptmg_skew & 3) - pConfig->disptmg_skew;

//	crt_mon_top = crtc_regs[4] + crtc_regs[5] - ((crtc_regs[3] & 0xf0) >> 4) - crtc_regs[7];

	crt_mon_crtc_hsync = ((h_total_min[0] <= crtc_regs[0] && crtc_regs[0] <= h_total_max[0])
	 || (h_total_min[1] <= crtc_regs[0] && crtc_regs[0] <= h_total_max[1])
	 || (h_total_min[2] <= crtc_regs[0] && crtc_regs[0] <= h_total_max[2])
	 || (h_total_min[3] <= crtc_regs[0] && crtc_regs[0] <= h_total_max[3]));

	crtc_vs_mid = (*crtc_vs_start) + 2;

#ifdef DEBUG_CRTMON
	logging->out_debugf(_T("DISPLAY:update_dws_params: clk:%d r0:%d r1:%d r2:%d r3:%d left_col:% 4d")
		,crtc_chr_clocks
		,crtc_regs[0],crtc_regs[1],crtc_regs[2],(crtc_regs[3] & 0xf)
		,crt_mon_disptmg_left
	);
#endif
}

void DISPLAY::update_chr_clocks(int clk)
{
	crtc_chr_clocks = (uint8_t)clk;
	crtc_ppch = (crtc_chr_clocks ? 8 : 16);
}

void DISPLAY::load_font_rom_file()
{
	const _TCHAR *app_path, *rom_path[2];

	rom_path[0] = pConfig->rom_path.Get();
	rom_path[1] = emu->application_path();

	for(int i=0; i<2; i++) {
		app_path = rom_path[i];

		if (!font_rom_loaded) {
			font_rom_loaded = emu->load_data_from_file(app_path, _T("FONT.ROM"), font, sizeof(font), (const uint8_t *)"\x00\x00\x00\x00", 4);
		}
	}

	if (!font_rom_loaded) {
		if (!font_rom_loaded_at_first) {
			logging->out_logf_x(LOG_WARN, CMsg::VSTR_couldn_t_be_loaded, _T("FONT.ROM"));
		}
		memset(chrfont, 0x10, sizeof(chrfont));
	} else {
		// convert font data to useful array data
		for(int i = 0; i < 2; i++) {
			for(int c = 0; c < 256; c++) {
				for(int l = 0; l < 16; l++) {
					chrfont[i][c][l] = get_font_data(i, c, l);
				}
			}
		}
	}
	font_rom_loaded_at_first = true;
}

void DISPLAY::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xffff) {
	case 0xffd0:
		// mode sel
		mode_sel = REG_MODE_SEL;

		bg_color = mode_sel & 0x07;				// bg color
		hireso = (mode_sel & 0x40) ? 0 : 1;		// 0:normal 1:hireso
		width_sel = (mode_sel & 0x80) ? 1 : 0;	// 0:w.40  1:w.80

		for(int i=0; i<8; i++) {
			ppd[i]=width_sel ? 1 : 2;	// pixel per 1dot
			dof[i]=(0x80 >> i);
		}
		// pixel per 1char
		update_chr_clocks(width_sel);

		d_crtc->write_signal(HD46505::SIG_HD46505_CHR_CLOCKS, CPU_CLOCKS * ((data & 0x80) ? 2 : 1), 0xffffffff);
		update_dws_params();

		break;
	case 0xffd6:
		// interlace sel
		REG_INTERLACE_SEL = ((data & 0x08) >> 3);	// 0:noninterlace char  1:interlace char
		break;
	case 0xffe9:
		// ig mode reg
		REG_IGENABLE = (data & 0x01);
		ig_mask &= (((data & 0x100) == 0) | (IOPORT_USE_DISIG != 0));
		break;
	}
}

void DISPLAY::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch(id) {
#ifdef USE_LIGHTPEN
		case SIG_DISPLAY_LPSTB:
			{
			int width = crtc_regs[1];
			int dotx = (data >> 16) & 0xffff;
			int doty = (data & 0xffff);
			int x = dotx;
			int y = doty;
			int offset = (width_sel ? 5 : 4);
			uint32_t newaddr;

			x /= crtc_ppch;
			y /= 2;

			if (y < crt_mon_disptmg_top) {
				y = crt_mon_disptmg_top;
			}
			if (y > crt_mon_disptmg_bottom) {
				y = crt_mon_disptmg_bottom;
			}
			t_crt_mon_vline *vline = &crt_mon_vline[y];
			if (x < vline->left_col) {
				x = vline->right_col;
				y--;
			}
			if (x > vline->right_col) {
				x = vline->left_col;
				y++;
			}

			x = (x - vline->left_col);
			y = (y - crt_mon_disptmg_top) / 8;

			newaddr = ((crtc_regs[12] << 8)+crtc_regs[13]) + y * width + x + offset;
			crtc_regs[16] = (newaddr >> 8) & 0x3f;
			crtc_regs[17] = newaddr & 0xff;

#ifdef DEBUG_CRTMON
			logging->out_debugf("lp dot(%03d,%03d) t:%03d b:%03d ch(%02d,%02d) start:%02x%02x ma:%04x pos:%02x%02x"
				,dotx,doty
				,crt_mon_disptmg_top,crt_mon_disptmg_bottom
				,x,y
				,crtc_regs[12],crtc_regs[13]
				,*crtc_ma
				,crtc_regs[16],crtc_regs[17]);
#endif
			}
			break;
#endif
		case SIG_DISPLAY_VSYNC:
			if ((data & mask) == 0) {
#ifdef DEBUG_CRTMON
				logging->out_debugf(_T("DISPLAY:SIG_DISPLAY_VSYNC_OFF: crtc:%d nras:%d cmr:%d cmc:%d cmras:%d top:%d btm:%d vtop:%d vbtm:%d")
					,(*crtc_vt_count), crt_mon_now_raster
					,crt_mon_row, crt_mon_col, (*crtc_ra)
					,top_raster, bottom_raster, vsync_top_raster, vsync_bottom_raster
				);
#endif
			} else {
				// synchronize display vertical raster
				for(int i=0; i<3; i++) {
					if (v_total_min[i] <= crt_mon_v_count && crt_mon_v_count <= v_total_max[i]) {
						crt_mon_v_start = 0;
						crt_mon_v_count = 0;
						crt_mon_row = CRT_MON_HEIGHT - 2;
						break;
					}
				}
			}
			break;
		case SIG_DISPLAY_WRITE_REGS:
			update_dws_params();
			break;
	}
}

void DISPLAY::set_display_size(int left, int top, int right, int bottom)
{
	screen_left = left;
	screen_right = right;
	screen_top = top;
	screen_bottom = bottom;
}

void DISPLAY::draw_screen()
{
	if ((afterimage == 0 && skip_frame_count >= 1)
	||  (afterimage > 0 && skip_frame_count >= 2)) {
		// clear cursor
#ifdef DEBUG_CRTMON2
		logging->out_debugf(_T("draw_screen:skip sw:%d skip_frame_count:%d afterimage:%d pause:%d")
			,sw , skip_frame_count, afterimage, (*vm_pause)
		);
#endif
		return;
	}
#ifdef DEBUG_CRTMON2
		logging->out_debugf(_T("draw_screen:draw sw:%d skip_frame_count:%d afterimage:%d pause:%d")
			,sw , skip_frame_count, afterimage, (*vm_pause)
		);
#endif

//	if (*vm_pause) return;

	crt_mon_bg_color = bg_color;

	// render screen
	if (afterimage == 0) draw_screen_sub();
	else if (afterimage == 1) draw_screen_sub_afterimage1();
	else draw_screen_sub_afterimage2();

	// sw bit0 : even/odd line (y axis)  bit1: even/odd char (x axis)
	sw = ((sw + 1) & 3);

	if (scanline <= 1 && *crtc_odd_line && skip_frame_count == 0) {
		sw = *crtc_odd_line;
	}
}

uint8_t DISPLAY::get_font_data(int interlace, int chr_code, int y)
{
	int code = 0;
	int offset = 0;
	if (interlace == 1) {
		// interlace mode char
		if (chr_code < 0x80) {
			code = chr_code << 4;	// 16byte per 1char
			offset = (y & 0x0e);	// 0 0 2 2 4 4 6 6 8 8 a a c c e e
		} else {
			code = chr_code << 4;	// 16byte per 1char
			offset = (y & 0x0f);	// 0 1 2 3 4 5 6 7 8 9 a b c d e f
		}
	} else {
		// non interlace mode char
		if (chr_code < 0x80) {
			// ascii code
			code = chr_code << 4;	// 16byte per 1char
		} else {
			code = ((chr_code - 0x80) << 4) + 1;
		}
//		offset = (y & 0x0e);		// 0 0 2 2 4 4 6 6 8 8 a a c c e e
		offset = ((y << 1) & 0x0e);	// 0 2 4 6 8 a c e 0 2 4 6 8 a c e
	}
	return font[code + offset];
}

#define COLOR_REVERSE(fg,bg) { \
	uint8_t c; \
	c = (bg); \
	(bg) = (fg); \
	(fg) = c; \
}

#define SET_SCRN_DOT(xx,dd) ((dd & scrnline2[xx]) | scrnline1p[xx])

#define DISP_TEXT_CHAR_ONE_DOT(pos, code, c_f, c_b, scrn1, palette) { \
	dws_fg = (code & dof[pos]) ? c_f : c_b; \
	dws_dtm = dws_x+dws_bi+ppd[pos]; \
	if (dws_dtm > SCREEN_WIDTH) dws_dtm = SCREEN_WIDTH; \
	for(dws_dt = dws_x+dws_bi; dws_dt < dws_dtm; dws_dt++) { \
		scrn1 = (palette); \
	} \
}

#define DISP_TEXT_CHAR_MAIN(code, c_f, c_b, scrn1, palette) { \
	dws_bi = 0; \
	for(dws_bt = 0; dws_bt < 8; dws_bt++) { \
		DISP_TEXT_CHAR_ONE_DOT(dws_bt, code, c_f, c_b, scrn1, palette); \
		dws_bi += ppd[dws_bt]; \
	} \
}

#ifdef USE_KEEPIMAGE
#define DISP_TEXT_CHAR_KI \
	if (dws_ki) { \
		DISP_TEXT_CHAR_MAIN(dws_p, dws_ck_f, dws_ck_b, scrntop[dws_y][dws_dt], palette_pc[dws_fg]) \
	}
#else
#define DISP_TEXT_CHAR_KI
#endif

#define DISP_TEXT_CHAR { \
	dws_p = chrfont[REG_INTERLACE_SEL][dws_code][(dws_raster >> dws_videomode_n) & 0xf]; \
\
	if (dws_disp_cursor_char) COLOR_REVERSE(dws_c_f, dws_c_b); \
	DISP_TEXT_CHAR_MAIN(dws_p, dws_c_f, dws_c_b, dws_scrn[dws_dt], SET_SCRN_DOT(dws_dt, palette_pc[dws_fg])) \
	DISP_TEXT_CHAR_KI \
}

#define DISP_IG_PATTERN_MAIN(c_f, c_b, scrn1, palette) { \
	dws_bi = 0; \
	for(dws_bt = 0; dws_bt < 8; dws_bt++) { \
		dws_p = ((dws_i_g & dof[dws_bt]) ? 4 : 0) | ((dws_i_r & dof[dws_bt]) ? 2 : 0) | ((dws_i_b & dof[dws_bt]) ? 1 : 0); \
		if (dws_p == 0) dws_p = c_b; \
		dws_dtm = dws_x+dws_bi+ppd[dws_bt]; \
		if (dws_dtm > SCREEN_WIDTH) dws_dtm = SCREEN_WIDTH; \
		for(dws_dt = dws_x+dws_bi; dws_dt < dws_dtm; dws_dt++) { \
			scrn1 = (palette); \
		} \
		dws_bi += ppd[dws_bt]; \
	} \
}

#ifdef USE_KEEPIMAGE
#define DISP_IG_PATTERN_KI \
	if (dws_ki) { \
		DISP_IG_PATTERN_MAIN(0, 0, scrntop[dws_y][dws_dt], palette_pc[dws_p]) \
	}
#else
#define DISP_IG_PATTERN_KI
#endif

#define DISP_IG_PATTERN { \
	dws_raster2 = dws_raster >> dws_int_vid; \
	dws_i_b = ig_ram[dws_code*8+dws_raster2]; \
	dws_i_r = ig_ram[dws_code*8+dws_raster2+IGRAM_SIZE]; \
	dws_i_g = ig_ram[dws_code*8+dws_raster2+IGRAM_SIZE+IGRAM_SIZE]; \
\
	if (FLG_ORIG_CURIG && dws_disp_cursor_char) { \
		dws_i_b = ~dws_i_b; \
		dws_i_r = ~dws_i_r; \
		dws_i_g = ~dws_i_g; \
	} \
	DISP_IG_PATTERN_MAIN(dws_c_f, dws_c_b, dws_scrn[dws_dt], SET_SCRN_DOT(dws_dt, palette_pc[dws_p])) \
	DISP_IG_PATTERN_KI \
}

#define DISP_WHITE_SPACE_MAIN(scrn1, palette) { \
	for(dws_dt = dws_x; dws_dt < dws_dtm; dws_dt++) { \
		scrn1 = (palette); \
	} \
}

#ifdef USE_KEEPIMAGE
#define DISP_WHITE_SPACE_KI \
	if (dws_ki) { \
		DISP_WHITE_SPACE_MAIN(scrntop[dws_y][dws_dt], palette_pc[dws_ck_f]) \
	}
#else
#define DISP_WHITE_SPACE_KI
#endif

#define DISP_WHITE_SPACE { \
	dws_p = 0xff; \
	dws_c_f = 7; \
	dws_c_b = bg_color; \
	dws_ck_f = 7; \
	dws_ck_b = 0; \
\
	if (dws_p_crt_mon_x->disptmg == 0x11) { \
		dws_bi = 0; \
		int btt = (1 << (rand() % 4)) | 0x20; \
		for(dws_bt = 0; dws_bt < 8; dws_bt++) { \
			dws_p = (btt & (1 << (7-dws_bt))) != 0 ? 3 : 7; \
			dws_dtm = dws_x+dws_bi+ppd[dws_bt]; \
			if (dws_dtm > SCREEN_WIDTH) dws_dtm = SCREEN_WIDTH; \
			for(dws_dt = dws_x+dws_bi; dws_dt < dws_dtm; dws_dt++) { \
				dws_scrn[dws_dt] = SET_SCRN_DOT(dws_dt, palette_pc[dws_p]); \
			} \
			dws_bi += ppd[dws_bt]; \
		} \
	} else { \
		dws_dtm = dws_x+crtc_ppch; \
		if (dws_dtm > SCREEN_WIDTH) dws_dtm = SCREEN_WIDTH; \
		DISP_WHITE_SPACE_MAIN(dws_scrn[dws_dt], SET_SCRN_DOT(dws_dt, palette_pc[dws_c_f])) \
	} \
	DISP_WHITE_SPACE_KI \
}

#define	DISP_GRAPHIC_HIRESO_MAIN(code, c_f, c_b, scrn1, palette) { \
	dws_bi = 0; \
	for(dws_bt = 0; dws_bt < 8; dws_bt++) { \
		dws_fg = (code & dof[dws_bt]) ? c_f : c_b; \
		dws_dtm = dws_x+dws_bi+ppd[dws_bt]; \
		if (dws_dtm > SCREEN_WIDTH) dws_dtm = SCREEN_WIDTH; \
		for(dws_dt = dws_x+dws_bi; dws_dt < dws_dtm; dws_dt++) { \
			scrn1 = (palette); \
		} \
		dws_bi += ppd[dws_bt]; \
	} \
}

#ifdef USE_KEEPIMAGE
#define DISP_GRAPHIC_HIRESO_KI \
	if (dws_ki) { \
		DISP_GRAPHIC_HIRESO_MAIN(dws_code, dws_ck_f, dws_ck_b, scrntop[dws_y][dws_dt], palette_pc[dws_fg]) \
	}
#else
#define DISP_GRAPHIC_HIRESO_KI
#endif

#define	DISP_GRAPHIC_HIRESO { \
	DISP_GRAPHIC_HIRESO_MAIN(dws_code, dws_c_f, dws_c_b, dws_scrn[dws_dt], SET_SCRN_DOT(dws_dt, palette_pc[dws_fg])) \
	DISP_GRAPHIC_HIRESO_KI \
}

#define DISP_GRAPHIC_NORMAL_MAIN(code, c_f, c_b, scrn1, palette) { \
	dws_bi = 0; \
	for (dws_bt = 0; dws_bt < 8; dws_bt += 4) { \
		dws_fg = (code & ((0x10 >> dws_bt) << ((dws_raster >> (1 + dws_int_vid)) & 3))) ? c_f : c_b; \
		dws_dtm = dws_x+dws_bi+ppd[dws_bt]+ppd[dws_bt+1]+ppd[dws_bt+2]+ppd[dws_bt+3]; \
		if (dws_dtm > SCREEN_WIDTH) dws_dtm = SCREEN_WIDTH; \
		for(dws_dt = dws_x+dws_bi; dws_dt < dws_dtm; dws_dt++) { \
			scrn1 = (palette); \
		} \
		dws_bi += ppd[dws_bt] + ppd[dws_bt + 1] + ppd[dws_bt + 2] + ppd[dws_bt + 3]; \
	} \
}

#ifdef USE_KEEPIMAGE
#define DISP_GRAPHIC_NORMAL_KI \
	if (dws_ki) { \
		DISP_GRAPHIC_NORMAL_MAIN(dws_code, dws_ck_f, dws_ck_b, scrntop[dws_y][dws_dt], palette_pc[dws_fg]) \
	}
#else
#define DISP_GRAPHIC_NORMAL_KI
#endif

#define DISP_GRAPHIC_NORMAL { \
	DISP_GRAPHIC_NORMAL_MAIN(dws_code, dws_c_f, dws_c_b, dws_scrn[dws_dt], SET_SCRN_DOT(dws_dt, palette_pc[dws_fg])) \
	DISP_GRAPHIC_NORMAL_KI \
}

#undef SET_SCRN_DOT

#define INIT_SCRN_DOT(scrn,yy) scrn = dws_scrn0 + dws_scrn_offset * (yy)
#define INIT_SCRNLINE1(yy)
#define SET_SCRN_DOT(xx,dd) (dd)
#define ADD_SCRN_DOT(scrn)  scrn += dws_scrn_offset * dws_ys
#define ADD_SCRNLINE1

void DISPLAY::draw_screen_sub()
{
	dws_disp_cursor_char = false;
	dws_scrn_offset = emu->screen_buffer_offset();
	dws_scrn0 = emu->screen_buffer(0);
	dws_dot = 0;
	dws_buf_x = 0;
	dws_cram = 0;

#include "displaysub.h"
}

#undef INIT_SCRNLINE1
#undef SET_SCRN_DOT
#undef ADD_SCRNLINE1

#define INIT_SCRNLINE1(yy)  dws_scrnline1p = scrnline1[0] + scrnline1_offset * (yy)
#define SET_SCRN_DOT(xx,dd) ((dd & scrnline2[xx]) | dws_scrnline1p[xx])
#define ADD_SCRNLINE1       dws_scrnline1p += scrnline1_offset * dws_ys

void DISPLAY::draw_screen_sub_afterimage1()
{
	dws_disp_cursor_char = false;
	dws_scrn_offset = emu->screen_buffer_offset();
	dws_scrn0 = emu->screen_buffer(0);
	dws_dot = 0;
	dws_buf_x = 0;
	dws_cram = 0;

	// color filter for afterimage
	dws_scrn = dws_scrn0 + dws_scrn_offset * screen_top;
	for(dws_y = screen_top; dws_y < screen_bottom; dws_y++) {
		dws_scrnline1p = scrnline1[dws_y];
		for(dws_x = screen_left; dws_x < screen_right; dws_x++) {
			dws_dot = dws_scrn[dws_x];
			dws_r = (dws_dot & Rmask);
			dws_g = (dws_dot & Gmask);
			dws_b = (dws_dot & Bmask);

			if (dws_r > Rboad) dws_r = dws_r - Rloss[0];
			else if (dws_r > Rloss[1]) dws_r = dws_r - Rloss[1];
			else dws_r = 0;
			if (dws_g > Gboad) dws_g = dws_g - Gloss[0];
			else if (dws_g > Gloss[1]) dws_g = dws_g - Gloss[1];
			else dws_g = 0;
			if (dws_b > Bboad) dws_b = dws_b - Bloss[0];
			else if (dws_b > Bloss[1]) dws_b = dws_b - Bloss[1];
			else dws_b = 0;

			dws_dot = dws_r | dws_g | dws_b;

			dws_scrnline1p[dws_x] = dws_dot;
		}
		dws_scrn += dws_scrn_offset;
	}

#include "displaysub.h"
}

#undef INIT_SCRNLINE1
#undef SET_SCRN_DOT
#undef ADD_SCRNLINE1

#define INIT_SCRNLINE1(yy)  dws_scrnline1p = scrnline1[0] + scrnline1_offset * (yy)
#define SET_SCRN_DOT(xx,dd) ((dd & scrnline2[xx]) | dws_scrnline1p[xx])
#define ADD_SCRNLINE1       dws_scrnline1p += scrnline1_offset * dws_ys

void DISPLAY::draw_screen_sub_afterimage2()
{
	dws_disp_cursor_char = false;
	dws_scrn_offset = emu->screen_buffer_offset();
	dws_scrn0 = emu->screen_buffer(0);
	dws_dot = 0;
	dws_buf_x = 0;
	dws_cram = 0;

	// color filter for afterimage2
	dws_scrn = dws_scrn0 + dws_scrn_offset * screen_top;
	for(dws_y = screen_top; dws_y < screen_bottom; dws_y++) {
		dws_scrnline1p = scrnline1[dws_y];
		for(dws_x = screen_left; dws_x < screen_right; dws_x++) {
			// 0xff or 0xdf -> 0xe0 -> 0x00
			// 0xff or 0xbf -> 0xc0 -> 0x00
			dws_dot = dws_scrn[dws_x] & 0x20202020;
			dws_dot = (dws_dot << 2) | (dws_dot << 1);

			dws_scrnline1p[dws_x] = dws_dot;
		}
		dws_scrn += dws_scrn_offset;
	}

#include "displaysub.h"
}

#define SET_CRT_MON(row, name, value) \
	crt_mon[row][crt_mon_col].name = (value);

#define ADD_CRT_MON(row, name, value) \
	crt_mon[row][crt_mon_col].name |= (value);

#define GET_CRT_MON(row, name) \
	crt_mon[row][crt_mon_col].name

#define P_SET_CRT_MON(ptr, name, value) \
	ptr->name = (value);

#define P_ADD_CRT_MON(ptr, name, value) \
	ptr->name |= (value);

#define P_GET_CRT_MON(ptr, name) \
	ptr->name

void DISPLAY::update_display(int v, int clock)
{
	static int crtc_vt_count_prev = -1;
	static int addr_offset;
	static bool now_skip_frame;
	static uint8_t disptmg;
	static int disptmg_left[2];
	static int disptmg_right[2];
	static int disptmg_st;
	static int hsync_left[2];
	static int hsync_right[2];
	static int addr_left;
	static bool now_hsync;
	static bool now_vsync;
	static int cursor_pos;
	static int videomode_n;
	static uint8_t raster;
	static int crt_mon_col_st;
	static t_crt_mon *crt_mon_bufy;
	static t_crt_mon *crt_mon_bufx;
	static t_crt_mon_vline *crt_mon_vline_y;
	static t_crt_mon_vinfo *crt_mon_vinfo_y;
	static uint8_t cursor_col;
	static int cursor_st_ras;
	static int cursor_ed_ras;
	static int cursor_skew;
	static int hireso_mask;
	static int ig_mask_mask;

	if ((*crtc_vt_count) == crtc_vt_count_prev) {
		return;
	}

	videomode_n = 1 - (*videomode);
	disptmg_st = (crt_mon_stepcols - 1) & (crt_mon_row + (sw >> 1));
	crt_mon_col = disptmg_st;

	if ((*crtc_vt_count) == 0) {
		// disptmg on
#ifdef USE_LIGHTPEN
		crt_mon_disptmg_top = crt_mon_row;
#endif
		// synchronize horizontal total
		if (crt_mon_crtc_hsync) {
			dws_left_diff = 0;
		}
#ifdef USE_KEEPIMAGE
		// top most page ?
		crt_mon_page = ((keepimage == 1 && (*crtc_ma) < 0x0800) || (keepimage == 2 && (*crtc_ma) >= 0x0800)) ? 0x80 : 0;
#endif
	}

	// pixel of width
	if (*crtc_vs_start <= *crtc_vt_count && *crtc_vt_count < crtc_vs_mid) {
		now_vsync = true;
	} else {
		now_vsync = false;
	}
#ifdef USE_LIGHTPEN
	if (*crtc_vt_count == *crtc_vt_disp) {
		crt_mon_disptmg_bottom = crt_mon_row;
	}
#endif
	crt_mon_disptmg_left = (*disptmg_skew) >= 4 ? VRAM_BUF_WIDTH
		: crtc_regs[0]-crtc_regs[2]-(crtc_regs[3] & 0x0f)-(crtc_chr_clocks ? 14 : 5) + (*disptmg_skew & 3) - pConfig->disptmg_skew;

	if (*crtc_vt_count >= *crtc_vt_disp) {
		disptmg_left[0] = crt_mon_disptmg_left + dws_left_diff;
		disptmg_right[0] = 0;
		disptmg_left[1] = VRAM_BUF_WIDTH;
		disptmg_right[1] = 0;
	} else {
		disptmg_left[0] = crt_mon_disptmg_left + dws_left_diff;
		disptmg_right[0] = disptmg_left[0] + crtc_regs[1];
		disptmg_left[1] = disptmg_left[0] + (crtc_chr_clocks ? crtc_regs[0] + 1 : 0);
		disptmg_right[1] = disptmg_right[0] + (crtc_chr_clocks ? crtc_regs[0] + 1 : 0);
	}

	if (disptmg_left[0] < 0) {
		addr_left = - disptmg_left[0];
	} else {
		addr_left = 0;
	}
	crt_mon_col_st = (crt_mon_stepcols - 1) & (disptmg_left[0] ^ disptmg_st);

	hsync_left[0] = disptmg_left[0] + crtc_regs[2];
	hsync_right[0] = hsync_left[0] + (crtc_regs[3] & 0x0f);
	hsync_left[1] = hsync_left[0] + (crtc_chr_clocks ? crtc_regs[0] + 1 : 0);
	hsync_right[1] = hsync_right[0] + (crtc_chr_clocks ? crtc_regs[0] + 1 : 0);

#ifdef DEBUG_CRTMON
	if (crt_mon_row < 2) {
		logging->out_debugf(_T("DISPLAY:update_vram: sw:%d crtc:cnt:%d ra:%d cmnr:%d cmr:%d cmsr:%d cmc:%d top:%d btm:%d vtop:%d vbtm:%d")
			,sw
			,(*crtc_vt_count), (*crtc_ra), crt_mon_now_raster
			,crt_mon_row, crt_mon_start_row,  crt_mon_col
			,top_raster, bottom_raster, vsync_top_raster, vsync_bottom_raster
		);
	}
#endif

	// diff horizontal total display and crtc
	if (!crt_mon_crtc_hsync) {
		dws_left_diff += (crtc_regs[0] + 1 - (1 << (crtc_chr_clocks + 6)));
		if (dws_left_diff >= VRAM_BUF_WIDTH) dws_left_diff = -VRAM_BUF_WIDTH + 1;
		else if (dws_left_diff <= -VRAM_BUF_WIDTH) dws_left_diff = VRAM_BUF_WIDTH - 1;
	}

	// now skip frame ?
	if (v == 0) {
		if (emu->now_skip_frame()) {
			skip_frame_count++;
		} else {
			skip_frame_count = 0;
		}
		if ((afterimage == 0 && skip_frame_count >= 1)
		||  (afterimage > 0 && skip_frame_count >= 2)) {
			// skip copy vram data
			now_skip_frame = true;
		} else {
			now_skip_frame = false;
		}
	}

	// copy vram data to buffer
	if (crt_mon_row < VRAM_BUF_HEIGHT && !now_skip_frame) { // && *crtc_vt_count < *crtc_vt_disp) {
		crt_mon_vline_y = &crt_mon_vline[crt_mon_row];
		crt_mon_vline_y->left_col = disptmg_left[0];
#ifdef USE_LIGHTPEN
		crt_mon_vline_y->right_col = disptmg_right[0];
#endif
		crt_mon_vline_y->flags = *crtc_odd_line;

		raster = (*crtc_ra) << videomode_n;

		crt_mon_vinfo_y = &crt_mon_vinfo[(crt_mon_row << 1) | (*videomode & *crtc_odd_line)];
		crt_mon_vinfo_y->raster = raster;
		crt_mon_vinfo_y->row = crt_mon_row;

		cursor_col = 254;
		cursor_st_ras = 255;
		cursor_ed_ras = 0;

		// calcrate address
		{
			// L3
			addr_offset = hireso * (((*crtc_ra) >> REG_INTERLACE_SEL) & 0x7) << (10 + width_sel);	// 0x400 or 0x800
			crt_mon_l3vaddr = *crtc_ma;
			crt_mon_l3vaddr += crt_mon_col_st;
			crt_mon_l3vaddr += addr_offset;
			crt_mon_l3vaddr += addr_left;
			crt_mon_l3vaddr &= L3_VRAM_SIZE_1;
			if (crt_mon_l3vaddr < L3_ADDR_VRAM_START) {
				crt_mon_l3vaddr += L3_VRAM_SIZE;
			}

			// cursor
			cursor_addr = (((crtc_regs[14] << 8) | crtc_regs[15]) + (*disptmg_skew & 3) - pConfig->disptmg_skew);
			cursor_addr += crt_mon_col_st;
			cursor_addr += addr_offset;
			cursor_addr += addr_left;
			cursor_addr &= L3_VRAM_SIZE_1;
			if (cursor_addr < L3_ADDR_VRAM_START) {
				cursor_addr += L3_VRAM_SIZE;
			}

			cursor_pos = (int)cursor_addr - crt_mon_l3vaddr;
			cursor_skew = (*curdisp_skew) + pConfig->curdisp_skew - 2;

			if ((*crtc_curdisp) != 0
			&& cursor_pos >= 0 && disptmg_left[0] + cursor_pos < disptmg_right[0]
			) {
				cursor_col = disptmg_left[0] + cursor_pos + cursor_skew;
				cursor_st_ras = (crtc_regs[10] & 0x1f) << videomode_n;
				cursor_ed_ras = (crtc_regs[11] + 1) << videomode_n;
//				logging->out_debugf(_T("DISPLAY: v:%d row:%d ra:%d pos:%d col:%d ras:%d-%d"), (*videomode), crt_mon_row, *crtc_ra, cursor_pos, crt_mon_vline_y->cursor_col, crt_mon_vline_y->cursor_st_ras, crt_mon_vline_y->cursor_ed_ras);
			}
		}

		if (cursor_st_ras <= raster && raster < cursor_ed_ras) {
			crt_mon_vinfo_y->cursor_st_col = cursor_col;
		} else {
			crt_mon_vinfo_y->cursor_st_col = 254;
		}

		if (raster & 1) {
			crt_mon_vinfo_y--;
			raster--;
		} else {
			crt_mon_vinfo_y++;
			raster++;
		}
		crt_mon_vinfo_y->raster = raster;
		crt_mon_vinfo_y->row = crt_mon_row;
		if (cursor_st_ras <= raster && raster < cursor_ed_ras) {
			crt_mon_vinfo_y->cursor_st_col = cursor_col;
		} else {
			crt_mon_vinfo_y->cursor_st_col = 254;
		}

		now_hsync = false;

		{
			// L3
			crt_mon_bufy = crt_mon[crt_mon_row];

			hireso_mask = (hireso << 6);
			ig_mask_mask = (ig_mask ? 0xdf : 0xff);

			for(; crt_mon_col < VRAM_BUF_WIDTH; crt_mon_col += crt_mon_stepcols) {
				crt_mon_bufx = &crt_mon_bufy[crt_mon_col];

				if ((disptmg_left[0] <= crt_mon_col && crt_mon_col < disptmg_right[0])
					|| (disptmg_left[1] <= crt_mon_col && crt_mon_col < disptmg_right[1])) {
					if (now_hsync) {
						d_crtc->update_raster();
						addr_offset += (hireso << (10 + width_sel));	// 0x400 or 0x800
						crt_mon_l3vaddr = *crtc_ma;
						crt_mon_l3vaddr += crt_mon_col_st;
						crt_mon_l3vaddr += addr_offset;
						crt_mon_l3vaddr &= L3_VRAM_SIZE_1;
						if (crt_mon_l3vaddr < L3_ADDR_VRAM_START) {
							crt_mon_l3vaddr += L3_VRAM_SIZE;
						}
						now_hsync = false;
					}
					P_SET_CRT_MON(crt_mon_bufx, disptmg, disptmg_left[0] == crt_mon_col && *crtc_vt_count == 0 ? 0x11 : 1);
					if (crt_mon_l3vaddr >= L3_ADDR_VRAM_END) {
						crt_mon_l3vaddr -= L3_VRAM_SIZE;
					}
					P_SET_CRT_MON(crt_mon_bufx, text, l3vram[crt_mon_l3vaddr]);
					// bit 0-5: color ram   bit 6: hireso  bit 7:top_most
#ifdef USE_KEEPIMAGE
					P_SET_CRT_MON(crt_mon_bufx, color, (color_ram[crt_mon_l3vaddr - L3_ADDR_VRAM_START] | hireso_mask | crt_mon_page) & ig_mask_mask);
#else
					P_SET_CRT_MON(crt_mon_bufx, color, (color_ram[crt_mon_l3vaddr - L3_ADDR_VRAM_START] | hireso_mask) & ig_mask_mask);
#endif
					P_SET_CRT_MON(crt_mon_bufx, attr, crt_mon_bg_color);
					crt_mon_l3vaddr += crt_mon_stepcols;
				} else {
					// out of disptmg area
					disptmg = 0;
					P_SET_CRT_MON(crt_mon_bufx, text, 0);
					if ((hsync_left[0] <= crt_mon_col && crt_mon_col < hsync_right[0])
					 || (hsync_left[1] <= crt_mon_col && crt_mon_col < hsync_right[1])) {
						P_SET_CRT_MON(crt_mon_bufx, attr, 0);
						if (!now_hsync) {
							disptmg |= 0x02;
							now_hsync = true;
						}
					} else if (now_vsync) {
						P_SET_CRT_MON(crt_mon_bufx, attr, 0);
					} else {
						P_SET_CRT_MON(crt_mon_bufx, attr, crt_mon_bg_color);
					}
					P_SET_CRT_MON(crt_mon_bufx, disptmg, disptmg);
				}
			}
		}
	}

	crt_mon_row++;
	if (crt_mon_row >= CRT_MON_HEIGHT) {
		crt_mon_row = 0;
	}
	crt_mon_v_count++;
	if (crt_mon_v_count >= (LINES_PER_FRAME * 2 + 16)) {
		crt_mon_v_count = crt_mon_v_start;
		crt_mon_v_start += 96;
		if (crt_mon_v_start >= LINES_PER_FRAME) crt_mon_v_start = LINES_PER_FRAME-1;
	}

	crtc_vt_count_prev = *crtc_vt_count;
}

// ----------------------------------------------------------------------------
// event handler
// ----------------------------------------------------------------------------

void DISPLAY::event_frame()
{
}

// ----------------------------------------------------------------------------

void DISPLAY::save_state(FILEIO *fio)
{
	struct vm_state_st vm_state;

	//
	vm_state_ident.version = Uint16_LE(2);
	vm_state_ident.size = Uint32_LE(sizeof(vm_state_ident) + sizeof(vm_state));

	// copy values
	memset(&vm_state, 0, sizeof(vm_state));
	vm_state.ig_enable = REG_IGENABLE;
	vm_state.ig_mask = ig_mask ? 1 : 0;

	vm_state.chr_clocks = crtc_chr_clocks;
	fio->Fwrite(&vm_state_ident, sizeof(vm_state_ident), 1);
	fio->Fwrite(&vm_state, sizeof(vm_state), 1);
}

bool DISPLAY::load_state(FILEIO *fio)
{
	vm_state_ident_t vm_state_i;
	struct vm_state_st vm_state;

	READ_STATE_CHUNK(fio, vm_state_i, vm_state);

	REG_IGENABLE = vm_state.ig_enable;
	ig_mask = vm_state.ig_mask ? true : false;

	if (Uint16_LE(vm_state_i.version) >= 2) {
		crtc_chr_clocks = vm_state.chr_clocks;
		crtc_ppch = (crtc_chr_clocks ? 8 : 16);
	}

	// clear
	crt_mon_row = 9999;
	crt_mon_col = 0;

	return true;
}

// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER

enum en_gnames {
	GN_IG_A = 0,
	GN_IG_B,
	GN_IG_R,
	GN_IG_G
};

const struct st_gnames {
	const _TCHAR *name;
} c_gnames[] = {
	_T("IG RAM"),
	_T("IG RAM (blue)"),
	_T("IG RAM (red)"),
	_T("IG RAM (green)"),
	NULL
};

int DISPLAY::get_debug_graphic_memory_size(int num, int type, int *width, int *height)
{
	switch(type) {
	case GN_IG_A:
	case GN_IG_B:
	case GN_IG_R:
	case GN_IG_G:
		// IG
		*width = (16 * 8);
		*height = (16 * 8);
		break;
	default:
		return -1;
	}
	return 0;
}

bool DISPLAY::debug_graphic_type_name(int type, _TCHAR *buffer, size_t buffer_len)
{
	for(int i=0; c_gnames[i].name; i++) {
		if (type == i) {
			UTILITY::tcscpy(buffer, buffer_len, c_gnames[i].name);
			return true;
		}
	}
	return false;
}

bool DISPLAY::debug_draw_graphic(int type, int width, int height, scrntype *buffer)
{

	int size = width * height;
	switch (type) {
	case GN_IG_A:
	case GN_IG_B:
	case GN_IG_R:
	case GN_IG_G:
		// IG
		for(int x=0; x<16; x++) {
		for(int y=0; y<16; y++) {
			for(int li=0; li<8; li++) {
			for(int bt=0; bt<8; bt++) {
				int pos = bt + (y + (li + x * 8) * 16) * 8;
				if (pos < size) {
					int rampos = (y*16 + x) * 8 + li;
					uint8_t msk = (0x80 >> bt);
					buffer[pos]=emu->map_rgbcolor(
							((type & 3) == 0 || (type & 3) == 2) && (ig_ram[rampos+0x0800] & msk) ? 0xff : 0,	// r
							((type & 3) == 0 || (type & 3) == 3) && (ig_ram[rampos+0x1000] & msk) ? 0xff : 0,	// g
							((type & 3) == 0 || (type & 3) == 1) && (ig_ram[rampos] & msk) ? 0xff : 0	// b
						);
				}
			}
			}
		}
		}
		break;
	default:
		return false;
	}
	return true;
}

bool DISPLAY::debug_dump_graphic(int type, int width, int height, uint16_t *buffer)
{

	int size = width * height;
	switch (type) {
	case GN_IG_A:
	case GN_IG_B:
	case GN_IG_R:
	case GN_IG_G:
		// IG
		for(int x=0; x<16; x++) {
		for(int y=0; y<16; y++) {
			for(int li=0; li<8; li++) {
			for(int bt=0; bt<8; bt++) {
				int pos = bt + (y + (li + x * 8) * 16) * 8;
				if (pos < size) {
					int rampos = (y*16 + x) * 8 + li;
					uint8_t msk = (0x80 >> bt);
					buffer[pos]=(
						  (((type & 3) == 0 || (type & 3) == 2) && (ig_ram[rampos+0x0800] & msk) ? 2 : 0)
						| (((type & 3) == 0 || (type & 3) == 3) && (ig_ram[rampos+0x1000] & msk) ? 4 : 0)
						| (((type & 3) == 0 || (type & 3) == 1) && (ig_ram[rampos] & msk) ? 1 : 0)
						);
				}
			}
			}
		}
		}
		break;
	default:
		return false;
	}
	return true;
}

#endif

