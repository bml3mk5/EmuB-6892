/** @file rtc.h

	HITACHI BASIC MASTER LEVEL3 Mark5 / MB-S1 Emulator 'EmuB-6892/EmuB-S1'
	Skelton for retropc emulator

	@author Sasaji
	@date   2018.01.01 -

	@brief [ real time clock control ]
*/

#ifndef RTC_H
#define RTC_H

#include "../vm_defs.h"
#include "../device.h"

class EMU;

/**
	@brief real time clock control
*/
class RTC : public DEVICE
{
private:
	DEVICE *d_rtc;

	uint8_t rtc_reg_sel;
	uint8_t rtc_data;

	//for resume
#pragma pack(1)
	struct vm_state_st {
		uint8_t rtc_reg_sel;
		uint8_t rtc_data;
		char  reserved[14];
	};
#pragma pack()

public:
	RTC(VM* parent_vm, EMU* parent_emu, const char* identifier) : DEVICE(parent_vm, parent_emu, identifier) {
		set_class_name("RTC");
		d_rtc = NULL;
	}
	~RTC() {}

	// common functions
	void initialize();
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);

	// unique functions
	void set_context_rtc(DEVICE* device) {
		d_rtc = device;
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

#endif /* RTC_H */
