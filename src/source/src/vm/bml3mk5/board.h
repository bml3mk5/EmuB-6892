/** @file board.h

	HITACHI BASIC MASTER LEVEL3 Mark5 Emulator 'EmuB-6892'
	Skelton for retropc emulator

	@author Sasaji
	@date   2012.06.21 -

	@brief [ main board ]
*/

#ifndef BOARD_H
#define BOARD_H

#include "../vm_defs.h"
//#include "../../emu.h"
#include "../device.h"
//#include "../../config.h"

class EMU;

/**
	@brief main board -	process the RESET signal
*/
class BOARD : public DEVICE
{
public:
	/// @brief signals on BOARD
	enum SIG_BOARD_IDS {
		SIG_BOARD_WRESET_RELEASE = 1,
		SIG_BOARD_PRESET_RELEASE = 2
	};

private:
	outputs_t outputs_reset;
	outputs_t outputs_nmi;
	outputs_t outputs_irq;
	outputs_t outputs_firq;
	outputs_t outputs_halt;

	uint16_t now_halt;
	uint16_t now_nmi;
	uint16_t now_irq;
	uint16_t now_firq;
	uint16_t now_wreset;

	int wreset_register_id;	// normal reset
	int preset_register_id;	// power on reset

	DEVICE *d_cpu;

// for save config
#pragma pack(1)
	struct vm_state_st {
		int		fdd_type;			///< after process power on
		int		io_port;			///< after process power on
		uint8_t	flags;

		// version 2
		char    reserved1[3];

		uint16_t	now_halt;
		uint16_t	now_nmi;

		uint16_t	now_irq;
		uint16_t	now_firq;

		uint16_t	now_wreset;

		// add on version 3
		int		wreset_register_id;
		int		preset_register_id;

		char  reserved[2];
	};
#pragma pack()

public:
	BOARD(VM* parent_vm, EMU* parent_emu, const char* identifier) : DEVICE(parent_vm, parent_emu, identifier) {
		set_class_name("BOARD");
		init_output_signals(&outputs_reset);
		init_output_signals(&outputs_nmi);
		init_output_signals(&outputs_irq);
		init_output_signals(&outputs_firq);
		init_output_signals(&outputs_halt);
	}
	~BOARD() {}

	// common functions
	void initialize();
	void reset();
	void cancel_my_event(int &id);
	void write_signal(int id, uint32_t data, uint32_t mask);
	uint32_t read_signal(int id);
	void event_callback(int event_id, int err);

	// unique functions
	void set_context_reset(DEVICE* device, int id, uint32_t mask) {
		register_output_signal(&outputs_reset, device, id, mask);
	}
	void set_context_nmi(DEVICE* device, int id, uint32_t mask) {
		register_output_signal(&outputs_nmi, device, id, mask);
	}
	void set_context_irq(DEVICE* device, int id, uint32_t mask) {
		register_output_signal(&outputs_irq, device, id, mask);
	}
	void set_context_firq(DEVICE* device, int id, uint32_t mask) {
		register_output_signal(&outputs_firq, device, id, mask);
	}
	void set_context_halt(DEVICE* device, int id, uint32_t mask) {
		register_output_signal(&outputs_halt, device, id, mask);
	}
	void set_context_cpu(DEVICE *device) {
		d_cpu = device;
	}
	uint32_t update_led();

	void save_state(FILEIO *fio);
	bool load_state(FILEIO *fio);

#ifdef USE_DEBUGGER
	bool debug_write_reg(uint32_t reg_num, uint32_t data);
	bool debug_write_reg(const _TCHAR *reg, uint32_t data);
	void debug_regs_info(_TCHAR *buffer, size_t buffer_len);
#endif
};

#endif /* BOARD_H */

