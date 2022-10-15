/** @file timer.h

	HITACHI BASIC MASTER LEVEL3 Mark5 Emulator 'EmuB-6892'
	Skelton for retropc emulator

	@author Sasaji
	@date   2011.06.08 -

	@brief [ timer ]
*/

#ifndef TIMER_H
#define TIMER_H

#include "../vm_defs.h"
#include "../device.h"
//#include "../../config.h"

#define ADDR_TIMER_IRQ	0xffca
#define ADDR_TIME_MASK	0xffd4

class EMU;

/**
	@brief timer - process the timer irq signal
*/
class TIMER : public DEVICE
{
public:
	/// @brief signals on TIMER
	enum SIG_TIMER_IDS {
		SIG_TIMER_VSYNC		= 1
	};

private:
	DEVICE *d_board;

	uint8_t	timer_irq;
//	uint8_t	time_mask;

	//for resume
#pragma pack(1)
	struct vm_state_st {
		uint8_t	timer_irq;
		uint8_t	time_mask;

		char  reserved[14];
	};
#pragma pack()

	void update_timer_clock();

public:
	TIMER(VM* parent_vm, EMU* parent_emu, const char* identifier) : DEVICE(parent_vm, parent_emu, identifier) {
		set_class_name("TIMER");
	}
	~TIMER() {}

	// common functions
	void initialize();
	void reset();
	void write_signal(int id, uint32_t data, uint32_t mask);
	void event_frame();

	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);

	// unique functions
	void set_context_board(DEVICE* device) {
		d_board = device;
	}

	void event_callback(int event_id, int err);

	void save_state(FILEIO *fio);
	bool load_state(FILEIO *fio);

#ifdef USE_DEBUGGER
	uint32_t debug_read_io8(uint32_t addr);
	bool debug_write_reg(uint32_t reg_num, uint32_t data);
	bool debug_write_reg(const _TCHAR *reg, uint32_t data);
	void debug_regs_info(_TCHAR *buffer, size_t buffer_len);
#endif
};

#endif /* TIMER_H */

