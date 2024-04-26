/** @file hd46505.cpp

	Skelton for retropc emulator

	@author Takeda.Toshiya
	@date   2006.09.20 -

	@note
	Modified by Sasaji at 2011.12.07

	@brief [ HD46505 ]
*/

#include "hd46505.h"
//#include "../emu.h"
#include "../config.h"
#include "../fileio.h"
#include "../utility.h"

void HD46505::initialize()
{
	// initialize
	chr_clocks = CPU_CLOCKS;

	display = false;
	vblank = vsync = hsync = true;

	memset(regs, 0, sizeof(regs));
	ch = 0;

	// initial settings for 1st frame
	hz_total = 1;
	hz_disp = 0;
	hs_start = 0;
	hs_end = 0;

	h_count = 0;
	hz_total_per_vline = 1;

	vt_total = 1;
	vt_total_per_frame = 1;
	vt_disp = 0;
	vs_start = 0;
	vs_end = 0;

	v_count = -1;

	timing_changed = false;
	disp_end_clock = 0;

	memory_address = 0;
	raster_address = 0;
	interlacemode = 0;
	videomode = 0;
	disptmg_skew = 0;
	max_raster_address = 1;
	video_odd_line = 0;

	cursor_blink = 0;
	cursor_disp = 0;

#ifdef HD46505_HORIZ_FREQ
	horiz_freq = 0;
	next_horiz_freq = HD46505_HORIZ_FREQ;
#endif
	for(int i=0; i<3; i++) {
		register_id[i] = -1;
	}

	// register events
	register_frame_event(this);
	register_vline_event(this);
}

void HD46505::reset()
{
	// set random parameter (means async with display)
	regs[0] = 95;
	regs[1] = 0;
	regs[2] = 88;
	regs[3] = 0x44;
	regs[4] = 31;
	regs[5] = 5;
	regs[6] = 25;
	regs[7] = 29;
	regs[8] = 0;
	regs[9] = 7;
	regs[10] = 0x20;
	timing_changed = true;

	for(int i=0; i<3; i++) {
		register_id[i] = -1;
	}
}

void HD46505::write_io8(uint32_t addr, uint32_t data)
{
	if(addr & 1) {
		if(ch < 18) {
			switch(ch) {
				case 9:
					data &= 0x1f;
					set_max_raster_address((uint8_t)data);
					if (regs[ch] != data) {
						timing_changed = true;
					}
					break;
				case 11:
					data &= 0x1f;
					break;
				case 12:
				case 14:
				case 16:
					data &= 0x3f;
					break;
				case 4:
				case 6:
				case 7:
					data &= 0x7f;
					if (regs[ch] != data) {
						timing_changed = true;
					}
					break;
				case 10:
					data &= 0x7f;
					break;
				case 8:
					data &= 0xf3;
					set_interlace_and_skew((uint8_t)data);
					if ((regs[ch] & 3) != (data & 3)) {
						timing_changed = true;
					}
					break;
			}
			regs[ch] = data;
		}
	}
	else {
		ch = data;
	}
}

void HD46505::set_interlace_and_skew(uint8_t data)
{
	interlacemode = (data & 1) ? 1 : 0;				// 0:noninterlace 1:interlace
	videomode = ((data & 3) == 3) ? 1 : 0;			// 1:videomode(interlace only)
	if ((data & 0x30) == 0x30) {
		disptmg_skew |= 4;							// disp off
	} else {
		disptmg_skew = ((data & 0x30) >> 4);		// disptmg skew
	}
	curdisp_skew = ((data & 0xc0) >> 6);			// curdisp skew

	set_max_raster_address(regs[9]);
}

void HD46505::set_max_raster_address(uint8_t data)
{
	max_raster_address = data + videomode + 1;
//	if (videomode) max_raster_address >>= 1;
	if (max_raster_address <= 0) max_raster_address = 1;
}

void HD46505::set_display_cursor()
{
	int bp = regs[10] & 0x60;	// cursor blink speed

	cursor_blink = (cursor_blink + 1) & 0x1f;
	cursor_disp = (curdisp_skew != 3
		&& (bp == 0 || (bp == 0x40 && (cursor_blink & 8) == 0) || (bp == 0x60 && (cursor_blink & 0x10) == 0))
	) ? 1 : 0;
}

uint32_t HD46505::read_io8(uint32_t addr)
{
	if(addr & 1) {
		return (12 <= ch && ch < 18) ? regs[ch] : 0xff;
	}
	else {
		return ch;
	}
}

void HD46505::event_pre_frame()
{
	if(timing_changed) {
//		int videomode = (regs[8] & 3) == 3 ? 1 : 0;
		int ch_height = max_raster_address;
		if (videomode) ch_height >>= 1;
		video_odd_line = 0;

		hz_total = regs[0] + 1;
		hz_disp = regs[1];
		hs_start = regs[2];
		hs_end = hs_start + (regs[3] & 0x0f);

//		int new_vt_total = ((regs[4] & 0x7f) + 1) * ch_height + (regs[5] & 0x1f);
		vt_total = ((regs[4] & 0x7f) + 1) * ch_height + (regs[5] & 0x1f);
		vt_disp = (regs[6] & 0x7f) * ch_height;
		vs_start = ((regs[7] & 0x7f) + 1) * ch_height;
		vs_end = vs_start + ((regs[3] & 0xf0) ? (regs[3] >> 4) : 16);

//		int new_vt_total_per_frame = (int)((double)new_vt_total * (double)CHARS_PER_LINE / (double)hz_total * (double)chr_clocks / (double)cpu_clocks);
		int new_vt_total_per_frame = (int)((double)chr_clocks / (double)hz_total / (double)FRAMES_PER_SEC);
//		if(vt_total != new_vt_total) {
		if(vt_total_per_frame != new_vt_total_per_frame) {
//			vt_total = new_vt_total;
			vt_total_per_frame = new_vt_total_per_frame;
			set_lines_per_frame(vt_total_per_frame);
		}
		hz_total_per_vline = (int)((double)chr_clocks / (double)FRAMES_PER_SEC / (double)LINES_PER_FRAME);
		timing_changed = false;
		disp_end_clock = 0;
#ifdef HD46505_HORIZ_FREQ
		horiz_freq = 0;
#endif
		if (outputs_wregs.count) {
			write_signals(&outputs_wregs, 0);
		}
	}
#ifdef HD46505_HORIZ_FREQ
	if(horiz_freq != next_horiz_freq) {
		uint8_t r8=regs[8]&3;
		horiz_freq = next_horiz_freq;
		frames_per_sec = (double)horiz_freq / (double)vt_total;
		if(regs[8] & 1) {
			frames_per_sec *= 2; // interlace mode
		}
		set_frames_per_sec(frames_per_sec);
	}
#endif
}

void HD46505::update_timing(int new_clocks, double new_frames_per_sec, int new_lines_per_frame)
{
	cpu_clocks = new_clocks;
#ifndef HD46505_HORIZ_FREQ
	frames_per_sec = new_frames_per_sec;
#endif

	// update event clocks
	disp_end_clock = 0;
}

void HD46505::event_frame()
{
	// update envet clocks after update_timing() is called
	if(disp_end_clock == 0 && vt_total != 0) {
//		disp_end_clock = (int)((double)cpu_clocks * (double)hz_disp / frames_per_sec / (double)vt_total / (double)hz_total);
//		hs_start_clock = (int)((double)cpu_clocks * (double)hs_start / frames_per_sec / (double)vt_total / (double)hz_total);
//		hs_end_clock = (int)((double)cpu_clocks * (double)hs_end / frames_per_sec / (double)vt_total / (double)hz_total);

		disp_end_clock = (int)((double)cpu_clocks * (double)hz_disp / (double)chr_clocks);
		hs_start_clock = (int)((double)cpu_clocks * (double)hs_start / (double)chr_clocks);
		hs_end_clock = (int)((double)cpu_clocks * (double)hs_end / (double)chr_clocks);

	}
}

void HD46505::event_vline(int v, int clock)
{
	int next_h_count = h_count + hz_total_per_vline;
	int next_v_step = 0;
	int next_h_mod = 0;
	if (hz_total) {
		next_v_step = next_h_count / hz_total;
		next_h_mod = next_h_count % hz_total;
	}
	h_count = next_h_mod;
	v_count += next_v_step;

	if (v_count >= vt_total
	|| (v == 30 && ((vt_total % vt_total_per_frame) == 0))
	) {
		v_count = 0;

		memory_address = (((regs[12] << 8) | regs[13]) + (disptmg_skew & 3) - pConfig->disptmg_skew) & 0x3fff;
		raster_address = (videomode & video_odd_line);

		set_display_cursor();
	} else {
		raster_address += (1 + videomode);
		if (raster_address >= max_raster_address) {
			raster_address -= max_raster_address;
			memory_address = (memory_address + regs[1]) & 0x3fff;
		}
	}

	// if vt_disp == 0, raise vblank for one line
	bool new_vblank = ((v_count < vt_disp) || (v_count == 0 && vt_disp == 0));

	// display
	if(outputs_disp.count) {
		set_display(new_vblank);
		if(new_vblank && hz_disp < hz_total) {
			if (register_id[EVENT_DISPLAY] != -1) cancel_event(this, register_id[EVENT_DISPLAY]);
			register_event_by_clock(this, EVENT_DISPLAY, disp_end_clock, false, &register_id[EVENT_DISPLAY]);
		}
	}

	// vblank
	set_vblank(new_vblank);	// active low

	// vsync
	bool new_vsync = (vs_start <= v_count && v_count < vs_end);
	set_vsync(new_vsync);

	// hsync
	if(outputs_hsync.count && hs_start < hs_end && hs_end < hz_total) {
		set_hsync(false);
		if (register_id[EVENT_HSYNC_S] != -1) cancel_event(this, register_id[EVENT_HSYNC_S]);
		if (register_id[EVENT_HSYNC_E] != -1) cancel_event(this, register_id[EVENT_HSYNC_E]);
		register_event_by_clock(this, EVENT_HSYNC_S, hs_start_clock, false, &register_id[EVENT_HSYNC_S]);
		register_event_by_clock(this, EVENT_HSYNC_E, hs_end_clock, false, &register_id[EVENT_HSYNC_E]);
	}
}

void HD46505::update_raster()
{
	raster_address += (1 + videomode);
	if (raster_address >= max_raster_address) {
		raster_address -= max_raster_address;
		memory_address = (memory_address + regs[1]) & 0x3fff;
	}
}

void HD46505::event_callback(int event_id, int err)
{
	if(event_id == EVENT_DISPLAY) {
		set_display(false);
	}
	else if(event_id == EVENT_HSYNC_S) {
		set_hsync(true);
	}
	else if(event_id == EVENT_HSYNC_E) {
		set_hsync(false);
	}
	register_id[event_id] = -1;
}

void HD46505::set_display(bool val)
{
	if(display != val) {
		write_signals(&outputs_disp, val ? 0xffffffff : 0);
		display = val;
	}
}

void HD46505::set_vblank(bool val)
{
	if(vblank != val) {
		write_signals(&outputs_vblank, val ? 0xffffffff : 0);
		vblank = val;
	}
}

void HD46505::set_vsync(bool val)
{
	if(vsync != val) {
		if (val) {
			// reset start raster
			if (interlacemode) {
				video_odd_line = 1 - video_odd_line;
				raster_address = (videomode & video_odd_line);
			} else {
				video_odd_line = 0;
				raster_address = 0;
			}
		}
		write_signals(&outputs_vsync, val ? 0xffffffff : 0);
		vsync = val;
	}
}

void HD46505::set_hsync(bool val)
{
	if(hsync != val) {
		write_signals(&outputs_hsync, val ? 0xffffffff : 0);
		hsync = val;
	}
}

void HD46505::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch(id) {
		case SIG_HD46505_CHR_CLOCKS:
			if (chr_clocks != (int)data) {
				chr_clocks = (int)data;
				timing_changed = true;
			}
			break;
	}
}

// ----------------------------------------------------------------------------

void HD46505::save_state(FILEIO *fio)
{
	struct vm_state_st vm_state;

	//
	vm_state_ident.version = Uint16_LE(2);
	vm_state_ident.size = Uint32_LE(sizeof(vm_state_ident) + sizeof(vm_state));

	// copy values
	memset(&vm_state, 0, sizeof(vm_state));
	memcpy(vm_state.regs, regs, sizeof(regs));
	vm_state.ch = Int32_LE(ch);
	vm_state.cpu_clocks = Int32_LE(cpu_clocks);
	vm_state.chr_clocks = Int32_LE(chr_clocks);

	for(int i=0; i<3; i++) {
		vm_state.register_id[i] = Int32_LE(register_id[i]);
	}
	vm_state.v_count = Int32_LE(v_count);

	fio->Fwrite(&vm_state_ident, sizeof(vm_state_ident), 1);
	fio->Fwrite(&vm_state, sizeof(vm_state), 1);
}

bool HD46505::load_state(FILEIO *fio)
{
	vm_state_ident_t vm_state_i;
	struct vm_state_st vm_state;

	READ_STATE_CHUNK(fio, vm_state_i, vm_state);

	// copy values
	memcpy(regs, vm_state.regs, sizeof(regs));
	ch = Int32_LE(vm_state.ch);
	cpu_clocks = Int32_LE(vm_state.cpu_clocks);
	chr_clocks = Int32_LE(vm_state.chr_clocks);

	// calc values
	set_interlace_and_skew(regs[8]);

	if (Uint16_LE(vm_state_i.version) == 1) {
		// version 1 : reset vsync and hsync
		timing_changed = true;
		disp_end_clock = 0;
		v_count = -1;
		for(int i=0; i<3; i++) {
			if (register_id[i] != -1) cancel_event(this, register_id[i]);
			register_id[i] = -1;
		}
		set_display(false);
		set_vsync(false);
		set_vblank(false);
		set_hsync(false);
	} else {
		// version 2 : regist vtiming
		for(int i=0; i<3; i++) {
			register_id[i] = Int32_LE(vm_state.register_id[i]);
		}
		v_count = Int32_LE(vm_state.v_count);

		timing_changed = true;
		event_pre_frame();
	}

	return true;
}

// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
uint32_t HD46505::debug_read_io8(uint32_t addr)
{
	return read_io8(addr);
}

bool HD46505::debug_write_reg(uint32_t reg_num, uint32_t data)
{
	if (reg_num < 18) {
		int ch_tmp = ch;
		ch = reg_num;
		write_io8(1, data);
		ch = ch_tmp;
		return true;
	}
	return false;
}

bool HD46505::debug_write_reg(const _TCHAR *reg, uint32_t data)
{
	return false;
}

void HD46505::debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	buffer[0] = _T('\0');
	for(int i=0; i<18; i++) {
		UTILITY::sntprintf(buffer, buffer_len
			,_T(" %02X:%02X")
			, i, regs[i]);
		if ((i & 7) == 7) UTILITY::tcscat(buffer, buffer_len, _T("\n"));
	}
}
#endif
