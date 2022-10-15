/** @file msm58321.h

	Skelton for retropc emulator

	@author Takeda.Toshiya
	@date   2008.05.02-

	@brief [ MSM58321/MSM5832 ]
*/

#ifndef _MSM58321_H_
#define _MSM58321_H_

#include "vm_defs.h"
#include "device.h"
#include "../curtime.h"

#undef USE_MSM58321_OUTPUTS_DATA
#undef USE_MSM58321_BUSY

class EMU;

/**
	@brief MSM58321 - Realtime Clock
*/
class MSM58321 : public DEVICE
{
public:
	/// @brief signals of MSM58321
	enum SIG_MSM58321_IDS {
		SIG_MSM58321_DATA		= 0,
		//SIG_MSM58321_CS		= 1,
		//SIG_MSM58321_READ		= 2,
		//SIG_MSM58321_WRITE	= 3,
		SIG_MSM58321_READWRITE	= 1,
		// for MSM58321 only
		//SIG_MSM58321_ADDR_WRITE	= 4,
		// for MSM5832 only
		SIG_MSM5832_ADDR		= 5,
		SIG_MSM5832_HOLD		= 6,
	};

private:
	/// @brief event ids
	enum EVENT_IDS {
		EVENT_BUSY	= 0,
		EVENT_INC	= 1,
		EVENT_PULSE	= 2,
	};

private:
	// output signals
#ifdef USE_MSM58321_OUTPUTS_DATA
	outputs_t outputs_data;
#endif
#if !defined(HAS_MSM5832) && defined(USE_MSM58321_BUSY)
	outputs_t outputs_busy;
#endif

	CurTime cur_time;
	int register_id, register_id_pulse;

	uint8_t regs[16];
	uint8_t wreg, regnum;
	bool cs, rd, wr, addr_wr, busy;
	uint8_t hold;
	int count_1024hz, count_1s, count_1m, count_1h;

	//for resume
#pragma pack(1)
	struct vm_state_st {
		int		register_id;
		uint8_t regs[16];
		uint8_t wreg;
		uint8_t regnum;
		uint8_t sigs;
		uint8_t hold;
		// 24 bytes
		int count_1024hz;
		int count_1s;
		// 32 bytes
		int count_1m;
		int count_1h;
		char reserved[8];
	};
#pragma pack()

	void read_from_cur_time();
	void write_to_cur_time();
#ifdef USE_MSM58321_OUTPUTS_DATA
	void output_data();
#endif
	void set_busy(bool val);

public:
	MSM58321(VM* parent_vm, EMU* parent_emu, const char* identifier) : DEVICE(parent_vm, parent_emu, identifier)
	{
#ifdef USE_MSM58321_OUTPUTS_DATA
		init_output_signals(&outputs_data);
#endif
#ifndef HAS_MSM5832
#ifdef USE_MSM58321_BUSY
		init_output_signals(&outputs_busy);
#endif
		set_class_name("MSM58321");
#else
		set_class_name("MSM5832");
#endif
	}
	~MSM58321() {}

	// common functions
	void initialize();
	void reset();
	void enable(bool value);
	void event_callback(int event_id, int err);
	void write_signal(int id, uint32_t data, uint32_t mask);
	uint32_t read_signal(int ch);
	void save_state(FILEIO* fio);
	bool load_state(FILEIO* fio);

	// unique functions
	void pause(int value);

#ifdef USE_MSM58321_OUTPUTS_DATA
	void set_context_data(DEVICE* device, int id, uint32_t mask, int shift)
	{
		register_output_signal(&outputs_data, device, id, mask, shift);
	}
#endif
#if !defined(HAS_MSM5832) && defined(USE_MSM58321_BUSY)
	void set_context_busy(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_busy, device, id, mask);
	}
#endif
#ifdef USE_DEBUGGER
	bool debug_write_reg(uint32_t reg_num, uint32_t data);
	bool debug_write_reg(const _TCHAR *reg, uint32_t data);
	void debug_regs_info(_TCHAR *buffer, size_t buffer_len);
#endif
};

#endif

