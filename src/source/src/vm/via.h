/** @file via.h

	HITACHI BASIC MASTER LEVEL3 Mark5 / MB-S1 Emulator 'EmuB-6892/EmuB-S1'
	Skelton for retropc emulator

	@author Sasaji
	@date   2012.06.08 -

	@brief [ via modoki (mcs6522) ]
*/

#ifndef VIA_MODOKI_H
#define VIA_MODOKI_H

#include "vm_defs.h"
#include "device.h"

class EMU;

/**
	@brief via modoki (mcs6522) - Versatile Interface Adapter
*/
class VIA : public DEVICE
{
public:
	/// @brief signals on VIA I/O port
	enum SIG_VIA_IDS {
		SIG_VIA_PA	= 1,
		SIG_VIA_CA1	= 2,
		SIG_VIA_CA2	= 3,
		SIG_VIA_PB	= 4,
		SIG_VIA_CB1	= 5,
		SIG_VIA_CB2	= 6,
		SIG_VIA_CLOCK_UNIT = 7
	};

private:
	/// @brief event ids
	enum EVENT_IDS {
		EVENT_VIA_CA2			= 1,
		EVENT_VIA_CB2			= 2,
		EVENT_VIA_TIMER1		= 3,
		EVENT_VIA_TIMER2		= 4,
		EVENT_VIA_SHIFT2_LOW	= 5,
		EVENT_VIA_SHIFT2_HIGH	= 6,
		EVENT_VIA_SHIFT2_INTR	= 7,
	};

private:
	uint8_t	drb;	///< Data Register B (ORB/IRB)
	uint8_t	dra;	///< Data Register A (ORA/IRA)
	uint8_t	ddrb;	///< Data Direction Register B
	uint8_t	ddra;	///< Data Direction Register A

	int		t1l;	///< Timer 1 latches
	int		t1c;	///< Timer 1 counter
	int		t2l;	///< Timer 2 latches
	int		t2c;	///< Timer 2 counter

	uint8_t	sr;		///< Shift Register
	uint8_t	acr;	///< Auxiliary Control Register
	uint8_t	pcr;	///< Peripheral Control Register
	uint8_t	ifr;	///< Interrupt Flag Register
	uint8_t	ier;	///< Interrupt Enable Register

	uint8_t	cra;	///< Control Register A
	uint8_t	crb;	///< Control Register B

	uint8_t	ca1;	///< CA1 signal
	uint8_t	ca2;	///< CA2 signal
	uint8_t	cb1;	///< CB1 signal
	uint8_t	cb2;	///< CB2 signal

	uint8_t	drb_res;	///< IRB reserve data
	uint8_t	dra_res;	///< IRA reserve data

	uint8_t	t1_pb7;	///< PB7 with timer 1
	uint8_t	t2_pb6;	///< PB6 with timer 2

	uint64_t	timer1_clock;	///< timer1 start clock
	uint64_t	timer2_clock;	///< timer2 start clock
	int		shift_timer;	///< shift timer
	int		shift_count;	///< shift count

	int ca2_register_id;
	int cb2_register_id;
	int t1_register_id;
	int t2_register_id;
	int s2_register_id;

	outputs_t outputs_pa;
	outputs_t outputs_ca2;
	outputs_t outputs_pb;
	outputs_t outputs_cb1;
	outputs_t outputs_cb2;
	outputs_t outputs_irq;

	uint8_t now_irq;

	uint8_t clk_unit;

	//for resume
#pragma pack(1)
	struct vm_state_st {
		int ca2_register_id;
		int cb2_register_id;
		int t1_register_id;
		int t2_register_id;

		int s2_register_id;
		uint8_t	drb;
		uint8_t	dra;
		uint8_t	ddrb;
		uint8_t	ddra;
		int		t1l;
		int		t1c;

		int		t2l;
		int		t2c;
		uint8_t	sr;
		uint8_t	acr;
		uint8_t	pcr;
		uint8_t	ifr;
		uint8_t	ier;
		uint8_t	cra;
		uint8_t	crb;
		uint8_t	ca1;

		uint8_t	ca2;
		uint8_t	cb1;
		uint8_t	cb2;
		uint8_t	drb_res;
		uint8_t	dra_res;
		uint8_t	t1_pb7;
		uint8_t	t2_pb6;

		uint8_t   now_irq;

//		uint32_t	timer1_clock;
//		uint32_t	timer2_clock;
//		int		shift_count;

		union {
			struct {
				uint32_t	timer1_clock;
				uint32_t	timer2_clock;
				int		shift_count;
				char reserved[8];
			} v1;
			struct {
				uint64_t	timer1_clock;
				uint64_t	timer2_clock;
				int		shift_count;
			} v2;
		} d;

		char reserved[4];
	};
#pragma pack()

	void set_ca2(uint8_t val);
	void set_cb1(uint8_t val);
	void set_cb2(uint8_t val);
	void set_irq(uint8_t data, uint8_t mask);
	void calc_clock(int &tc, uint64_t &clock);

	void shift_low();
	void shift();
	void set_shift_intr();
	void cancel_my_events();

public:
	VIA(VM* parent_vm, EMU* parent_emu, const char* identifier) : DEVICE(parent_vm, parent_emu, identifier) {
		set_class_name("VIA");
		init_output_signals(&outputs_pa);
		init_output_signals(&outputs_ca2);
		init_output_signals(&outputs_pb);
		init_output_signals(&outputs_cb1);
		init_output_signals(&outputs_cb2);
		init_output_signals(&outputs_irq);

		clk_unit = 1;
	}
	~VIA() {}

	// common functions
	void initialize();
	void reset();

	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);

	void write_signal(int id, uint32_t data, uint32_t mask);

	// unique function
	void set_context_pa(DEVICE* device, int id, uint32_t mask) {
		register_output_signal(&outputs_pa, device, id, mask);
	}
	void set_context_ca2(DEVICE* device, int id, uint32_t mask) {
		register_output_signal(&outputs_ca2, device, id, mask);
	}
	void set_context_pb(DEVICE* device, int id, uint32_t mask) {
		register_output_signal(&outputs_pb, device, id, mask);
	}
	void set_context_cb1(DEVICE* device, int id, uint32_t mask) {
		register_output_signal(&outputs_cb1, device, id, mask);
	}
	void set_context_cb2(DEVICE* device, int id, uint32_t mask) {
		register_output_signal(&outputs_cb2, device, id, mask);
	}
	void set_context_irq(DEVICE* device, int id, uint32_t mask) {
		register_output_signal(&outputs_irq, device, id, mask);
	}

	// event callback
	void event_frame();
	void event_callback(int event_id, int err);

	void save_state(FILEIO *fp);
	bool load_state(FILEIO *fp);

#ifdef USE_DEBUGGER
	uint32_t debug_read_io8(uint32_t addr);
	bool debug_write_reg(uint32_t reg_num, uint32_t data);
	bool debug_write_reg(const _TCHAR *reg, uint32_t data);
	void debug_regs_info(_TCHAR *buffer, size_t buffer_len);
#endif
};

#endif /* VIA_MODOKI_H */
