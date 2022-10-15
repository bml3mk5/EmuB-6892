/** @file display.cpp

	HITACHI BASIC MASTER LEVEL3 Mark5 Emulator 'EmuB-6892'
	Skelton for retropc emulator

	@author Sasaji
	@date   2011.06.08 -

	@brief [ display sub ]
*/

#ifdef DISPLAY_H

{
	// videomode negative
	dws_videomode_n = 1 - (*videomode);
	// interace or videomode
	dws_int_vid = (REG_INTERLACE_SEL) + dws_videomode_n;

	// odd line ?
	dws_yd = (scanline >= 2 ? sw & 1 : 0);

	// set drawing area
	dws_y0=screen_top + dws_yd;
	dws_y1=screen_bottom;
	INIT_SCRN_DOT(dws_scrn, dws_y0);
	INIT_SCRNLINE1(dws_y0);

	for(dws_y = dws_y0; dws_y < dws_y1; ) {
		dws_p_vinfo = &crt_mon_vinfo[dws_y];
		dws_buf_y_h = dws_p_vinfo->row;
		dws_p_crt_mon = crt_mon[dws_buf_y_h];
		dws_p_vline = &crt_mon_vline[dws_buf_y_h];
		dws_raster = dws_p_vinfo->raster;

		if (scanline == 1) {
			if ((dws_p_vline->flags ^ dws_y) & 1) {
				// clear undrawing line
				for(dws_x = screen_left; dws_x < screen_right; dws_x++) {
					dws_scrn[dws_x] = SET_SCRN_DOT(dws_x, palette_pc[0]);
				}
				dws_y+=dws_ys;
				ADD_SCRN_DOT(dws_scrn);
				ADD_SCRNLINE1;
				continue;
			}
		}

		dws_buf_x = screen_left / crtc_ppch;
		dws_x0 = screen_left;
		dws_x1 = screen_right;
		dws_xodd = 0;
		if (buf_stepcols > 1) {
			dws_xodd = ((dws_buf_y_h + (sw >> 1)) & 1);
			dws_buf_x += dws_xodd;
			dws_x0 += (dws_xodd * crtc_ppch);
		}

#ifdef DEBUG_CRTMON
		logging->out_debugf(_T("DISPLAY:draw_sub: sw:%d yh:%3d ras:%2d flg:%c l:%2d yd:%2d")
			, sw, dws_buf_y_h, dws_p_vline->raster, dws_l_flg ? _T('t') : _T('f'), dws_l, dws_yd
		);
#endif
		{
			//
			//
			// L3
			//
			//
			for(dws_x = dws_x0; dws_x < dws_x1; ) {
				dws_p_crt_mon_x = &dws_p_crt_mon[dws_buf_x];
				dws_c_b = dws_p_crt_mon_x->attr;
				dws_disp_cursor_char = (dws_p_vinfo->cursor_st_col == dws_buf_x);
				if (dws_p_crt_mon_x->disptmg & 1) {

					dws_code = dws_p_crt_mon_x->text;
					dws_cram = dws_p_crt_mon_x->color;
#ifdef USE_KEEPIMAGE
					dws_ki = dws_cram & 0x80 ? true : false;
#endif
					dws_c_f = dws_cram & 0x07;
					dws_ck_f = dws_c_f;
					dws_ck_b = 0;
					// color reverse
					if ((dws_cram & 0x28) == 0x08) {
						COLOR_REVERSE(dws_c_f, dws_c_b);
#ifdef USE_KEEPIMAGE
						if (dws_ki) {
							COLOR_REVERSE(dws_ck_f, dws_ck_b);
						}
#endif
					}

					switch(dws_cram & 0x30) {
					case 0x30:
					case 0x20:
						// ig
						if (!REG_IGENABLE) {
							// disp ig pattern
							DISP_IG_PATTERN;
						} else {
							// disp white square
							DISP_WHITE_SPACE;
						}
						break;
					case 0x10:
						// graphic
						dws_p = dws_code;

						if (dws_disp_cursor_char) COLOR_REVERSE(dws_c_f, dws_c_b);
						if (dws_cram & 0x40) {
							// hireso graphic
							DISP_GRAPHIC_HIRESO;
						} else {
							// normal graphic
							DISP_GRAPHIC_NORMAL;
						}
						break;
					default:
						// character
						DISP_TEXT_CHAR;
						break;
					}
				} else {
					if (dws_p_crt_mon_x->disptmg & 2) {
						dws_raster += 2;
					}
					if (dws_disp_cursor_char) COLOR_REVERSE(dws_c_f, dws_c_b);
					for(dws_dt = dws_x; dws_dt < dws_x+crtc_ppch; dws_dt++) {
						dws_scrn[dws_dt] = SET_SCRN_DOT(dws_dt, palette_pc[dws_c_b]);
					}
				}
				dws_x += (crtc_ppch * buf_stepcols);
				dws_buf_x += buf_stepcols;
			}
		}

		dws_y+=dws_ys;
		ADD_SCRN_DOT(dws_scrn);
		ADD_SCRNLINE1;
	}

#ifdef USE_KEEPIMAGE
	// overlap the top most screen if need
	if (keepimage > 0) {
		dws_scrn = emu->screen_buffer(screen_top);
		for(dws_y = screen_top; dws_y < screen_bottom; dws_y++) {
			for(dws_x = screen_left; dws_x < screen_right; dws_x++) {
				if (scrntop[dws_y][dws_x] != palette_pc[0]) dws_scrn[dws_x] = scrntop[dws_y][dws_x];
			}
			dws_scrn += dws_scrn_offset;
		}
	}
#endif
}

#endif /* DISPLAY_H */
