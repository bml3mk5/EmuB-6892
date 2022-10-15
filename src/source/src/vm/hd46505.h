/** @file hd46505.h

	Skelton for retropc emulator

	@author Takeda.Toshiya
	@date   2007.02.08 -

	@note
	Modified by Sasaji at 2011.12.07

	@brief [ HD46505 ]
*/

#ifndef HD46505_H
#define HD46505_H

#include "vm_defs.h"
#include "device.h"

class EMU;

/**
	@brief HD46505 - CRT Controller
*/
class HD46505 : public DEVICE
{
public:
	/// @brief signals on HD46505(CRTC)
	enum SIG_HD46505_IDS {
		SIG_HD46505_CPU_POWER	= 1,
		SIG_HD46505_CHR_CLOCKS	= 2
	};

private:
	/// @brief event ids
	enum EVENT_IDS {
		EVENT_DISPLAY	= 0,
		EVENT_HSYNC_S	= 1,
		EVENT_HSYNC_E	= 2,
	};

private:
	// output signals
	outputs_t outputs_disp;
	outputs_t outputs_vblank;
	outputs_t outputs_vsync;
	outputs_t outputs_hsync;
	outputs_t outputs_wregs;

	uint8_t regs[18];
	int ch;
	bool timing_changed;

	int cpu_clocks;
	int chr_clocks;
#ifdef HD46505_HORIZ_FREQ
	int horiz_freq, next_horiz_freq;
#endif
	double frames_per_sec;

	int hz_total, hz_disp;
	int hs_start, hs_end;
	int h_count;

	int hz_total_per_vline;

	int vt_total, vt_disp;
	int vt_total_per_frame;
	int vs_start, vs_end;
	int v_count;

	int disp_end_clock;
	int hs_start_clock, hs_end_clock;

	bool display, vblank, vsync, hsync;

	int memory_address;				///< refresh memory address(MA)
	int raster_address;				///< raster address (RA)
	int interlacemode;				///< 0:noninterlace 1:interlace (crtc reg8 bit0)
	int videomode;					///< 1:videomode(interlace only)
	int disptmg_skew;				///< display timing skew (0..2) / disp off if bit2 is 1
	int curdisp_skew;				///< cursor disp skew
	int max_raster_address;			///< max raster address
	int video_odd_line;				///< videomode odd line

	int cursor_blink;				///< cursor blink interval
	int cursor_disp;				///< display cursor

	int register_id[3];

#pragma pack(1)
	struct vm_state_st {
		uint8_t regs[18];
		int ch;
		int cpu_clocks;
		int chr_clocks;
		char reserved[2];
		// version 2
		int register_id[3];
		int v_count;
	};
#pragma pack()

	void set_display(bool val);
	void set_vblank(bool val);
	void set_vsync(bool val);
	void set_hsync(bool val);

	void set_interlace_and_skew(uint8_t data);
	void set_max_raster_address(uint8_t data);
	inline void set_display_cursor();

public:
	HD46505(VM* parent_vm, EMU* parent_emu, const char* identifier) : DEVICE(parent_vm, parent_emu, identifier) {
		set_class_name("HD46505");
		init_output_signals(&outputs_disp);
		init_output_signals(&outputs_vblank);
		init_output_signals(&outputs_vsync);
		init_output_signals(&outputs_hsync);
		init_output_signals(&outputs_wregs);
	}
	~HD46505() {}

	// common functions
	void initialize();
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void event_pre_frame();
	void event_frame();
	void event_vline(int v, int clock);
	void event_callback(int event_id, int err);
	void update_timing(int new_clocks, double new_frames_per_sec, int new_lines_per_frame);
	void write_signal(int id, uint32_t data, uint32_t mask);
	void update_raster();

	// unique function
	void set_context_disp(DEVICE* device, int id, uint32_t mask) {
		register_output_signal(&outputs_disp, device, id, mask);
	}
	void set_context_vblank(DEVICE* device, int id, uint32_t mask) {
		register_output_signal(&outputs_vblank, device, id, mask);
	}
	void set_context_vsync(DEVICE* device, int id, uint32_t mask) {
		register_output_signal(&outputs_vsync, device, id, mask);
	}
	void set_context_hsync(DEVICE* device, int id, uint32_t mask) {
		register_output_signal(&outputs_hsync, device, id, mask);
	}
	void set_context_write_regs(DEVICE* device, int id, uint32_t mask) {
		register_output_signal(&outputs_wregs, device, id, mask);
	}
#ifdef HD46505_HORIZ_FREQ
	void set_horiz_freq(int freq) {
		next_horiz_freq = freq;
	}
#endif
	uint8_t* get_regs() {
		return regs;
	}
	int* get_vt_total_ptr() {
		return &vt_total;
	}
	int* get_vt_count_ptr() {
		return &v_count;
	}
	int* get_vt_disp_ptr() {
		return &vt_disp;
	}
	int* get_ma_ptr() {
		return &memory_address;
	}
	int* get_ra_ptr() {
		return &raster_address;
	}
	int* get_max_ra_ptr() {
		return &max_raster_address;
	}
	int* get_video_odd_line_ptr() {
		return &video_odd_line;
	}
	int* get_interlacemode_ptr() {
		return &interlacemode;
	}
	int* get_videomode_ptr() {
		return &videomode;
	}
	int* get_disptmg_skew_ptr() {
		return &disptmg_skew;
	}
	int* get_curdisp_skew_ptr() {
		return &curdisp_skew;
	}
	int* get_curdisp_ptr() {
		return &cursor_disp;
	}
	int* get_vs_start_ptr() {
		return &vs_start;
	}
	int* get_vs_end_ptr() {
		return &vs_end;
	}

	void save_state(FILEIO *fio);
	bool load_state(FILEIO *fio);

#ifdef USE_DEBUGGER
	uint32_t debug_read_io8(uint32_t addr);
	bool debug_write_reg(uint32_t reg_num, uint32_t data);
	bool debug_write_reg(const _TCHAR *reg, uint32_t data);
	void debug_regs_info(_TCHAR *buffer, size_t buffer_len);
#endif
};

#endif /* HD46505_H */

