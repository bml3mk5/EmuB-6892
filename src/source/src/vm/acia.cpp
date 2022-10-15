/** @file acia.cpp

	HITACHI BASIC MASTER LEVEL3 Mark5 / MB-S1 Emulator 'EmuB-6892/EmuB-S1'
	Skelton for retropc emulator

	@author Sasaji
	@date   2011.06.08 -

	@brief [ acia modoki (mc6850) ]
*/

#include "acia.h"
//#include "../emu.h"
#include "../fileio.h"
#include "../utility.h"

void ACIA::initialize()
{
	clear();
}

void ACIA::reset()
{
}

void ACIA::clear()
{
	cr = 0;
	sr = 0x02;	// tdrf always on
	sr_prev = 0;
	dcd = 0;
	ovrn = 0;
	srmask = 0x74;

	memset(read_buffer, 0, sizeof(read_buffer));
	idx_read_buffer = 0;
	memset(write_buffer, 0, sizeof(write_buffer));
	idx_write_buffer = 0;
	buffer_max = 0;
	rdr_data = 0;
	tdr_data = 0;

	set_irq(false);

	// signal
	set_rts(true);
	set_dtr(true);
}

void ACIA::write_io8(uint32_t addr, uint32_t data)
{
	if (addr & 1) {
		// Write Data
		tdr_data = data;

		// reset tdrf
		sr = sr & 0x7d;
		// reset irq
		set_irq((sr & srmask) != 0);
	} else {
		// Write Control Register
		if ((data & 0x03) == 0x03) {
			// clear all register
			clear();
			write_signals(&outputs_res, 0xffffffff);
		}
		// set the word select
		// startbit + 7bit
		buffer_max=8;
		if (data & 0x10) {
			// 8bit
			buffer_max++;
		}
		if ((data & 0x1c) != 0x10 && (data & 0x1c) != 0x14) {
			// parity bit
			buffer_max++;
		}
		// stop bit
		buffer_max++;
		if ((data & 0x1c) == 0x10 || (data & 0x18) == 0x00) {
			// 2 stop bit
			buffer_max++;
		}

		srmask = ((cr & 0x60) == 0x20) ? srmask | 0x02 : srmask & ~0x02;
		srmask = ((cr & 0x80) != 0) ? srmask | 0x01 : srmask & ~0x01;

		cr = data & 0xff;

	}
//	logging->out_debugf("aw %04x=%02x sr=%02x cr=%02x bmax=%d",addr,data,sr,cr,buffer_max);
}

uint32_t ACIA::read_io8(uint32_t addr)
{
	uint32_t data = 0;

	if (addr & 1) {
		// Read Data
		data = rdr_data;
		rdr_data = 0;

		// reser RDRF
		sr = sr & 0xfe;

		// clear dcd
		if (dcd & 0x80) sr = sr & 0xfb;
		dcd &= 0x7f;

		// clear ovrn
		ovrn = 0;
		sr = sr & 0xdf;
		// reset irq
		set_irq((sr & srmask) != 0);

//		logging->out_debugf("%02x",data);
	} else {
		// Read Status Register
		data = sr;

		dcd |= 0x80;

//		if (ovrn) {
//			// clear irq
//			set_irq(false);
//		}
	}

//	logging->out_debugf("ar %04x=%02x %c sr=%02x",addr,data,(0x20 <= data && data <= 0x7f) ? data : 0x20,sr);

	return data;
}

void ACIA::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch (id) {
		case SIG_ACIA_TXCLK:
			ready_to_send_to_txdata();
			// send to comm device
			if ((sr & 0x02) == 0) {
				if (mask == 1) {
					// 1bit data
					data = split_8bit_data(tdr_data);
//					d_comm->write_signal(SIG_ACIA_TXDATA, data, 1);
					write_signals(&outputs_txd, data);
				} else {
					// 8bit data
//					d_comm->write_signal(SIG_ACIA_TXDATA, tdr_data, 0xff);
					write_signals(&outputs_txd, tdr_data);
					tdr_data = 0;
					sr = sr | 0x02;
				}
				ready_to_write_to_register();
			}
			break;
		case SIG_ACIA_RXCLK:
			ready_to_recieve_from_rxdata();
			break;
		case SIG_ACIA_RXDATA:
//			logging->out_debugf("acia rxdata d=%02x m=%02x sr=%02x",data,mask,sr);
			if (sr & 0x01) {
				// overrun
				error_ovrn();
			} else {
				if (mask == 1) {
					// 1bit data
					parse_1bit_data(data & mask);
				} else {
					// 8bit data
					rdr_data = (data & mask);
					sr = sr | 0x01;	// set RDRF
				}
			}
			ready_to_read_from_register();
			break;
		case SIG_ACIA_CTS:
			if (data & mask) {
				sr = sr & 0xfe;
			} else {
				sr = sr | 0x08;
			}
			break;
		case SIG_ACIA_DCD:
			if (data & mask) {
				sr = sr | 0x04;
				// rising edge
				if ((cr & 0x80) && (dcd & 1) == 0) {
					// send irq interrupt
					set_irq(true);
				}
				dcd = 1;
			} else {
				dcd &= ~1;
			}
			break;
		case SIG_ACIA_ERROR:
			sr = sr | 0x78;
			break;
		case SIG_ACIA_ERR_OVRN:
			error_ovrn();
			break;
	}
}

uint32_t ACIA::read_signal(int id)
{
	uint32_t data = 0;
	switch (id) {
	case SIG_ACIA_SR:
		data = sr;
		break;
	case SIG_ACIA_CR:
		data = cr;
		break;
	}
	return data;
}

uint8_t ACIA::split_8bit_data(uint8_t data)
{
	int h_count = 0;	// H bitcount (use for parity check)

	if (idx_write_buffer == (buffer_max - 1)) {
		// last bit
		sr = sr | 0x02;
		tdr_data = 0;
	}
	if (idx_write_buffer != 0 && idx_write_buffer < buffer_max) {
		return write_buffer[idx_write_buffer++];
	}

	memset(write_buffer, 0, sizeof(write_buffer));

	idx_write_buffer = 0;
	// start bit
	write_buffer[idx_write_buffer++] = 0;
	// data bit
	h_count = 0;
	for(; idx_write_buffer < 8 + ((cr & 0x10) ? 1 : 0); idx_write_buffer++) {
		write_buffer[idx_write_buffer] = (data & (1 << (idx_write_buffer - 1))) ? 1 : 0;
		h_count += write_buffer[idx_write_buffer];
	}
	// parity bit
	if ((cr & 0x1c) != 0x10 && (cr & 0x1c) != 0x14) {
		if (cr & 0x04) {
			// odd parity
			write_buffer[idx_write_buffer] = (h_count & 0x01) ? 1 : 0;
		} else {
			// even parity
			write_buffer[idx_write_buffer] = (h_count & 0x01) ? 0 : 1;
		}
		idx_write_buffer++;
	}
	// stop bit
	write_buffer[idx_write_buffer++] = 1;
	if ((cr & 0x1c) == 0x10 || (cr & 0x18) == 0x00) {
		// 2 stop bit
		write_buffer[idx_write_buffer++] = 1;
	}

	idx_write_buffer=0;

	return write_buffer[idx_write_buffer++];
}



bool ACIA::parse_1bit_data(uint8_t data)
{
	int pos = 0;
	int h_count = 0;	// H bitcount (use for parity check)
	uint8_t final_bit = 0;

	// start bit?
	if (idx_read_buffer == 0 && data != 0) {
		return false;
	}

	read_buffer[idx_read_buffer++] = data;

	if (idx_read_buffer < buffer_max) {
		return true;
	}

	// data bit
	sr = sr & 0xaf;	// clear framing error and parity error
	pos = 1;
	h_count = 0;
	rdr_data = 0;
	for(; pos < 8 + ((cr & 0x10) ? 1 : 0); pos++) {
		rdr_data |= (read_buffer[pos] << (pos - 1));
		h_count += read_buffer[pos];
	}
	// parity bit
	if ((cr & 0x1c) != 0x10 && (cr & 0x1c) != 0x14) {
		if (cr & 0x04) {
			// odd parity
			if (((read_buffer[pos] + h_count) & 0x01) != 1) sr = sr | 0x40;		// parity error
		} else {
			// even parity
			if (((read_buffer[pos] + h_count) & 0x01) != 0) sr = sr | 0x40;		// parity error
		}
		pos++;
	}
	// stop bit
	if (read_buffer[pos++] != 1) sr = sr | 0x10;		// framing error
	if ((cr & 0x1c) == 0x10 || (cr & 0x18) == 0x00) {
		// 2 stop bit
		if (read_buffer[pos++] != 1) sr = sr | 0x10;		// framing error
	}
	final_bit = read_buffer[pos-1];

//	logging->out_debugf("p1bit d=%02x sr=%02x",rdr_data,sr);

	// rdrf turn on if has no error.
	if ((sr & 0x70) == 0) sr = sr | 0x01;

	// clear buffer
	memset(read_buffer, 0, sizeof(read_buffer));
	idx_read_buffer = 0;

	// if frameing error occured and final_bit is 0 then copy final bit to buffer as start_bit.
	if ((sr & 0x10) && final_bit == 0) {
		read_buffer[0] = final_bit;
		idx_read_buffer++;
	}

	return true;
}

// ----------------------------------------------------------------------------
void ACIA::ready_to_write_to_register()
{
	// ready to write by cpu
	if ((sr & 0x02) != 0) {
		if ((cr & 0x60) == 0x20) {
			// send irq interrupt
			set_irq(true);
		}
		// RTS
		set_rts((cr & 0x60) == 0x40 ? true : false);
	}
	sr_prev = sr;
}

void ACIA::ready_to_send_to_txdata()
{
	// send ok
	if ((sr_prev & 0x02) != 0 && (sr & 0x02) == 0) {
		// RTS
		set_rts(true);
	}
	sr_prev = sr;
}

void ACIA::ready_to_read_from_register()
{
	// ready to read by cpu
	if ((sr_prev & 0x01) == 0 && (sr & 0x01) != 0) {
		if (cr & 0x80) {
			// send irq interrupt
			set_irq(true);
		}
		// ACIA does not have DTR.
		set_dtr(false);
	}
	sr_prev = sr;
}

void ACIA::ready_to_recieve_from_rxdata()
{
	// ready to recieve
	if ((sr & 0x01) == 0) {
		// ACIA does not have DTR.
		set_dtr(true);
	}
	sr_prev = sr;
}

// ----------------------------------------------------------------------------

void ACIA::set_rts(bool val)
{
	write_signals(&outputs_rts, val ? 0xffffffff : 0);
}

void ACIA::set_dtr(bool val)
{
	write_signals(&outputs_dtr, val ? 0xffffffff : 0);
}

void ACIA::set_irq(bool val)
{
	if (now_irq != val){
		write_signals(&outputs_irq, val ? 0xffffffff : 0);
		now_irq = val;
		sr = val ? sr | 0x80 : sr & 0x7f;
	}
}

// ----------------------------------------------------------------------------

void ACIA::error_ovrn()
{
	sr = sr | 0x20;
	if ((cr & 0x80) && ovrn == 0) {
		// send irq interrupt
		set_irq(true);
	}
	ovrn = 1;
}

// ----------------------------------------------------------------------------
// event handler
// ----------------------------------------------------------------------------

void ACIA::event_frame()
{
}

void ACIA::event_callback(int event_id, int err)
{
}

// ----------------------------------------------------------------------------
void ACIA::save_state(FILEIO *fp)
{
	struct vm_state_st vm_state;

	//
	vm_state_ident.version = Uint16_LE(2);
	vm_state_ident.size = Uint32_LE(sizeof(vm_state_ident) + sizeof(vm_state));

	// copy values
	memset(&vm_state, 0, sizeof(vm_state));
	vm_state.cr = cr;
	vm_state.sr = sr;
	vm_state.sr_prev = sr_prev;

	memcpy(vm_state.read_buffer, read_buffer, 13);
	vm_state.idx_read_buffer = Int32_LE(idx_read_buffer);
	memcpy(vm_state.write_buffer, write_buffer, 13);
	vm_state.idx_write_buffer = Int32_LE(idx_write_buffer);
	vm_state.buffer_max = Int32_LE(buffer_max);
	vm_state.rdr_data = rdr_data;
	vm_state.tdr_data = tdr_data;

	vm_state.now_irq = now_irq ? 1 : 0;

	vm_state.dcd = dcd;

	fp->Fwrite(&vm_state_ident, sizeof(vm_state_ident), 1);
	fp->Fwrite(&vm_state, sizeof(vm_state), 1);
}

bool ACIA::load_state(FILEIO *fp)
{
	vm_state_ident_t vm_state_i;
	struct vm_state_st vm_state;

	READ_STATE_CHUNK(fp, vm_state_i, vm_state);

	// copy values
	cr = vm_state.cr;
	sr = vm_state.sr;
	sr_prev = vm_state.sr_prev;

	srmask = 0x74;
	srmask = ((cr & 0x60) == 0x20) ? srmask | 0x02 : srmask & ~0x02;
	srmask = ((cr & 0x80) != 0) ? srmask | 0x01 : srmask & ~0x01;

	memcpy(read_buffer, vm_state.read_buffer, 13);
	idx_read_buffer = Int32_LE(vm_state.idx_read_buffer);
	memcpy(write_buffer, vm_state.write_buffer, 13);
	idx_write_buffer = Int32_LE(vm_state.idx_write_buffer);
	buffer_max = Int32_LE(vm_state.buffer_max);
	rdr_data = vm_state.rdr_data;
	tdr_data = vm_state.tdr_data;

	if (vm_state_i.version >= 2) {
		dcd = vm_state.dcd;
	} else {
		dcd = (sr & 0x04) ? 1 : 0;
	}

	ovrn = (sr & 0x20) ? 1 : 0;

	now_irq = vm_state.now_irq ? true : false;

	return true;
}

// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
uint32_t ACIA::debug_read_io8(uint32_t addr)
{
	uint32_t data = 0;

	if (addr & 1) {
		// Read Data
		data = rdr_data;

	} else {
		// Read Status Register
		data = sr;

	}
	return data;
}

static const _TCHAR *c_reg_names[] = {
	_T("CR"),
	_T("SR"),
	NULL
};

bool ACIA::debug_write_reg(uint32_t reg_num, uint32_t data)
{
	switch(reg_num) {
	case 0:
		write_io8(0, data);
		return true;
	case 1:
		sr = sr & 0xff;
		set_irq((sr & 0x80) != 0);
		return true;
	}
	return false;
}

bool ACIA::debug_write_reg(const _TCHAR *reg, uint32_t data)
{
	uint32_t num = find_debug_reg_name(c_reg_names, reg);
	return debug_write_reg(num, data);
}

void ACIA::debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	buffer[0] = _T('\0');
	UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%s):%02X"), 0, c_reg_names[0], cr);
	UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%s):%02X"), 1, c_reg_names[1], sr);
}
#endif