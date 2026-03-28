/** @file pia.h

	HITACHI BASIC MASTER LEVEL3 Mark5 / MB-S1 Emulator 'EmuB-6892/EmuB-S1'
	Skelton for retropc emulator

	@author Sasaji
	@date   2011.06.16 -

	@brief [ pia modoki (mc6821) ]
*/

#ifndef PIA_MODOKI_H
#define PIA_MODOKI_H

#include "vm_defs.h"
#include "device.h"

class EMU;

/**
	@brief pia modoki (mc6821) - Peripheral Interface Adapter
*/
class PIA : public DEVICE
{
public:
	/// @brief signals on PIA I/O port
	enum SIG_PIA_IDS {
		SIG_PIA_PA	= 1,
		SIG_PIA_CA1	= 2,
		SIG_PIA_CA2	= 3,
		SIG_PIA_PB	= 4,
		SIG_PIA_CB1	= 5,
		SIG_PIA_CB2	= 6
	};

private:
	/// @brief event ids
	enum EVENT_IDS {
		EVENT_PIA_CA2	= 1,
		EVENT_PIA_CB2	= 2,
	};
	/// @brief CR masks
	enum CR_MASKS {
		CRA_CA1_IRQEN = 0x01,
		CRA_CA1_RISE = 0x02,
		CRA_DDR_SEL = 0x04,
		CRA_CA2_IRQEN = 0x08,
		CRA_CA2_RISE = 0x10,
		CRA_CA2_OUT = 0x20,
		CRA_CA2_CTRL = 0x38,
		CRA_IRQ2 = 0x40,
		CRA_IRQ1 = 0x80,
		CRA_IRQ_CLR = 0x3f,
		CRB_CB1_IRQEN = 0x01,
		CRB_CB1_RISE = 0x02,
		CRB_DDR_SEL = 0x04,
		CRB_CB2_IRQEN = 0x08,
		CRB_CB2_RISE = 0x10,
		CRB_CB2_OUT = 0x20,
		CRB_CB2_CTRL = 0x38,
		CRB_IRQ2 = 0x40,
		CRB_IRQ1 = 0x80,
		CRB_IRQ_CLR = 0x3f,
	};

private:
	uint8_t	cra;	///< Control Register A
	uint8_t	crb;	///< Control Register B
	uint8_t	dra;	///< Data Register A
	uint8_t	drb;	///< Data Register B
	uint8_t	ddra;	///< Data Direction Register A
	uint8_t	ddrb;	///< Data Direction Register B
	uint8_t	ora;	///< Output Data Register A
	uint8_t	orb;	///< Output Data Register B

	uint8_t	ca1;	///< CA1 signal
	uint8_t	ca2;	///< CA2 signal
	uint8_t	cb1;	///< CB1 signal
	uint8_t	cb2;	///< CB2 signal

	int ca2_register_id;
	int cb2_register_id;

	outputs_t outputs_pa;
	outputs_t outputs_ca2;
	outputs_t outputs_irqa;
	outputs_t outputs_pb;
	outputs_t outputs_cb2;
	outputs_t outputs_irqb;

	bool now_irqa;
	bool now_irqb;

	//for resume
#pragma pack(1)
	struct vm_state_st {
		int ca2_register_id;
		int cb2_register_id;

		uint8_t	cra;
		uint8_t	crb;
		uint8_t	dra;
		uint8_t	drb;
		uint8_t	ddra;
		uint8_t	ddrb;
		uint8_t	ca;
		uint8_t	cb;

		uint8_t   now_irq;
		// version 2
		char reserved0;
		uint8_t ora;
		uint8_t orb;

		char reserved[12];
	};
#pragma pack()

	void set_ca2(uint8_t val);
	void set_cb2(uint8_t val);
	void set_irqa(bool val);
	void set_irqb(bool val);
	void cancel_my_events();

public:
	PIA(VM* parent_vm, EMU* parent_emu, const char* identifier);
	~PIA();

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
	void set_context_irqa(DEVICE* device, int id, uint32_t mask) {
		register_output_signal(&outputs_irqa, device, id, mask);
	}
	void set_context_pb(DEVICE* device, int id, uint32_t mask) {
		register_output_signal(&outputs_pb, device, id, mask);
	}
	void set_context_cb2(DEVICE* device, int id, uint32_t mask) {
		register_output_signal(&outputs_cb2, device, id, mask);
	}
	void set_context_irqb(DEVICE* device, int id, uint32_t mask) {
		register_output_signal(&outputs_irqb, device, id, mask);
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
	void debug_regs_info(const _TCHAR *title, _TCHAR *buffer, size_t buffer_len);
#endif
};

#endif /* PIA_MODOKI_H */
