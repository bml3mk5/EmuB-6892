/** @file acia.h

	HITACHI BASIC MASTER LEVEL3 Mark5 / MB-S1 Emulator 'EmuB-6892/EmuB-S1'
	Skelton for retropc emulator

	@author Sasaji
	@date   2011.06.08 -

	@brief [ acia modoki (mc6850) ]
*/

#ifndef ACIA_MODOKI_H
#define ACIA_MODOKI_H

#include "vm_defs.h"
//#include "../../emu.h"
#include "device.h"

class EMU;

/**
	@brief acia modoki (mc6850) - Asynchronous Communication Interface Adapter
*/
class ACIA : public DEVICE
{
public:
	/// @brief signals on ACIA
	enum SIG_ACIA_IDS {
		SIG_ACIA_RXCLK	= 3,
		SIG_ACIA_TXCLK	= 4,

		SIG_ACIA_RXDATA	= 5,
		SIG_ACIA_TXDATA	= 6,

		SIG_ACIA_DTR	= 7,	///< Data Terminal Ready
		SIG_ACIA_RTS	= 8,	///< Request To Send

		SIG_ACIA_CTS	= 9,	///< Clear To Send
		SIG_ACIA_DCD	= 10,	///< Data Carrier Detect

		SIG_ACIA_ERROR		= 12,
		SIG_ACIA_ERR_OVRN	= 13,
		SIG_ACIA_RESET		= 14,

		SIG_ACIA_CR		= 15,	///< read control register
		SIG_ACIA_SR		= 16,	///< read status register
	};

private:
	uint8_t	cr;	// Control Register
	uint8_t	sr;	// Status Register
	uint8_t	sr_prev;
	uint8_t	srmask;	// Interrupt mask on Status Register 

	uint8_t   dcd;
	uint8_t   ovrn;

	// data buffer 0:start bit 1:D0 ... 8:D7 (9:Parity) 9,10-11:stop bit
	uint8_t	read_buffer[13];
	int idx_read_buffer;
	uint8_t	write_buffer[13];
	int idx_write_buffer;
	int buffer_max;
	uint8_t	rdr_data;
	uint8_t	tdr_data;

	bool now_irq;

	//for resume
#pragma pack(1)
	struct vm_state_st {
		uint8_t	cr;
		uint8_t	sr;
		uint8_t	sr_prev;

		uint8_t	read_buffer[13];
		int		idx_read_buffer;
		uint8_t	write_buffer[13];
		int		idx_write_buffer;
		int		buffer_max;
		uint8_t	rdr_data;
		uint8_t	tdr_data;

		uint8_t now_irq;

		//version 2
		uint8_t dcd;

		char reserved[3];
	};
#pragma pack()

	outputs_t outputs_txd;
	outputs_t outputs_rts;
	outputs_t outputs_dtr;
	outputs_t outputs_irq;
	outputs_t outputs_res;

	void clear();
	uint8_t split_8bit_data(uint8_t);
	bool parse_1bit_data(uint8_t);

	void ready_to_write_to_register();
	void ready_to_send_to_txdata();
	void ready_to_read_from_register();
	void ready_to_recieve_from_rxdata();

	void set_rts(bool val);
	void set_dtr(bool val);
	void set_irq(bool val);

	void error_ovrn();
public:
	ACIA(VM* parent_vm, EMU* parent_emu, const char* identifier) : DEVICE(parent_vm, parent_emu, identifier) {
		set_class_name("ACIA");
		init_output_signals(&outputs_txd);
		init_output_signals(&outputs_rts);
		init_output_signals(&outputs_dtr);
		init_output_signals(&outputs_irq);
		init_output_signals(&outputs_res);
	}
	~ACIA() {}

	// common functions
	void initialize();
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);

	void write_signal(int id, uint32_t data, uint32_t mask);
	uint32_t read_signal(int id);

	// unique function
	// ACIA TXDATA
	void set_context_txdata(DEVICE* device, int id, uint32_t mask) {
		register_output_signal(&outputs_txd, device, id, mask);
	}
	// ACIA RTS
	void set_context_rts(DEVICE* device, int id, uint32_t mask) {
		register_output_signal(&outputs_rts, device, id, mask);
	}
	// ACIA DTR (dummy signal)
	void set_context_dtr(DEVICE* device, int id, uint32_t mask) {
		register_output_signal(&outputs_dtr, device, id, mask);
	}
	// IRQ
	void set_context_irq(DEVICE* device, int id, uint32_t mask) {
		register_output_signal(&outputs_irq, device, id, mask);
	}
	// Reset (fire when set 0x03 to control register)
	void set_context_res(DEVICE* device, int id, uint32_t mask) {
		register_output_signal(&outputs_res, device, id, mask);
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

#endif /* ACIA_MODOKI_H */
