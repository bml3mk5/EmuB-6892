/** @file ym2203.h

	Skelton for retropc emulator

	@author Takeda.Toshiya
	@date   2006.09.15-

	@note
	Modified by Sasaji at 2012.06.20
	@attention
	AY-3-8910 functions has gone to AY38910 class

	@brief [ YM2203 / YM2608 ]
*/

#ifndef _YM2203_H_
#define _YM2203_H_

#include "vm_defs.h"
#include "device.h"
#include "fmgen/opna.h"

#define HAS_YM_SERIES

#define SUPPORT_YM2203_PORT_A
#define SUPPORT_YM2203_PORT_B
#define SUPPORT_YM2203_PORT

class EMU;

/**
	@brief YM2203 / YM2608 - FM Sound Operator
*/
class YM2203 : public DEVICE
{
public:
	/// @brief signals of YM2203
	enum SIG_YM2203_IDS {
		SIG_YM2203_PORT_A	= 0,
		SIG_YM2203_PORT_B	= 1,
		SIG_YM2203_MUTE		= 2,
	};

private:
	/// @brief event ids
	enum EVENT_IDS {
		EVENT_FM_TIMER	= 0
	};

private:
#ifdef HAS_YM2608
	FM::OPNA* opna;
#endif
	FM::OPN* opn;
	int base_decibel_fm, base_decibel_psg;

	uint8_t ch;
	uint8_t fnum2;
#ifdef HAS_YM2608
	uint8_t ch1, data1;
	uint8_t fnum21;
#endif

#ifdef SUPPORT_YM2203_PORT
	struct {
		uint8_t wreg;
		uint8_t rreg;
		bool first;
		// output signals
		outputs_t outputs;
	} port[2];
	uint8_t mode;
#endif

	int chip_clock;
	bool irq_prev, mute;

	uint64_t clock_prev;
	uint64_t clock_accum;
	uint64_t clock_const;
	int timer_event_id;

	uint64_t clock_busy;
	bool busy;

	void update_count();
	void update_event();

//	int usec_per_vline;
#ifdef USE_EMU_INHERENT_SPEC
//	int sound_volume;
//	int32_t buffer_tmp[40];
#endif

	// output signals
	outputs_t outputs_irq;
	void update_interrupt();

#ifdef HAS_YM2608
	bool is_ym2608;
#endif

#ifdef USE_EMU_INHERENT_SPEC
	//for resume
#pragma pack(1)
	struct vm_state_st {
		struct {
			uint8_t ch;
			uint8_t fnum2;
			uint8_t ch1;
			uint8_t data1;
			uint8_t fnum21;
			uint8_t mode;
			struct {
				uint8_t wreg;
				uint8_t rreg;
				uint8_t first;
			} port[2];
			uint8_t irq_prev;
			uint8_t mute;
			uint8_t busy;
			char reserved[1];

			uint32_t timer_event_id;
			int32_t chip_clock;
			uint64_t clock_prev;

			uint64_t clock_accum;
			uint64_t clock_const;

			uint64_t clock_busy;

			char reserved2[8];
		} v3;
	};

	//for resume
	struct vm_state_psg_st {
		uint8_t ch, mode;
		// PSG register only
		union {
			struct {
				uint8_t reg[13];
				struct {
					uint8_t wreg;
					uint8_t rreg;
					uint8_t first;
				} port[2];
				char reserved;
			} v1;
			struct {
				uint8_t reg[14];
				struct {
					uint8_t wreg;
					uint8_t rreg;
					uint8_t first;
				} port[2];
			} v2;
		};
		char reserved[10];
	};
#pragma pack()
#endif

#ifdef USE_DEBUGGER
#ifdef HAS_YM2608
	uint8_t debug_regs[512];
#else
	uint8_t debug_regs[256];
#endif
#endif

public:
	YM2203(VM* parent_vm, EMU* parent_emu, const char* identifier);
	~YM2203() {}

	// common functions
	void initialize();
	void set_chiptype(bool ym2608);
	void release();
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
	void event_vline(int v, int clock);
	void event_callback(int event_id, int error);
	void mix(int32_t* buffer, int cnt);
	void set_volume(int ch, int decibel_l, int decibel_r, bool vol_mute, int pattern);
	void update_timing(int new_clocks, double new_frames_per_sec, int new_lines_per_frame);
	void save_state(FILEIO *fp);
	bool load_state(FILEIO *fp);

	// unique functions
	void init_context_irq() {
		init_output_signals(&outputs_irq);
	}
	void set_context_irq(DEVICE* device, int id, uint32_t mask) {
		register_output_signal(&outputs_irq, device, id, mask);
	}
	void set_context_port_a(DEVICE* device, int id, uint32_t mask, int shift) {
		register_output_signal(&port[0].outputs, device, id, mask, shift);
	}
	void set_context_port_b(DEVICE* device, int id, uint32_t mask, int shift) {
		register_output_signal(&port[1].outputs, device, id, mask, shift);
	}
	void initialize_sound(int rate, int clock, int samples, int decibel_fm, int decibel_psg, int ptn);
	void set_reg(uint32_t addr, uint32_t data); // for patch
	uint32_t read_status();
#ifdef HAS_YM2608
	uint32_t read_status_ex();
#endif

#ifdef USE_DEBUGGER
	uint32_t debug_read_io8(uint32_t addr);
	void debug_write_data8(int type, uint32_t addr, uint32_t data);
	uint32_t debug_read_data8(int type, uint32_t addr);
	uint32_t debug_physical_addr_mask(int type);
	bool debug_write_reg(uint32_t reg_num, uint32_t data);
	bool debug_write_reg(const _TCHAR *reg, uint32_t data);
	void debug_regs_info(_TCHAR *buffer, size_t buffer_len);
#endif
};

#endif
