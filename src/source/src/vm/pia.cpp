/** @file pia.cpp

	HITACHI BASIC MASTER LEVEL3 Mark5 / MB-S1 Emulator 'EmuB-6892/EmuB-S1'
	Skelton for retropc emulator

	@author Sasaji
	@date   2011.06.16 -

	@brief [ pia modoki (mc6821) ]
*/

#include "pia.h"
//#include "../emu.h"
#include "../fileio.h"
#include "../utility.h"

void PIA::initialize()
{
	ca2_register_id = -1;
	cb2_register_id = -1;
}

void PIA::reset()
{
	cra = 0;
	crb = 0;
	dra = 0;
	drb = 0;
	ddra = 0;
	ddrb = 0;

	ca1 = ca2 = 0;
	cb1 = cb2 = 0;

	now_irqa = false;
	now_irqb = false;

	ca2_register_id = -1;
	cb2_register_id = -1;
}

void PIA::cancel_my_events()
{
	if (ca2_register_id != -1) cancel_event(this, ca2_register_id);
	if (cb2_register_id != -1) cancel_event(this, cb2_register_id);
	ca2_register_id = -1;
	cb2_register_id = -1;
}

void PIA::write_io8(uint32_t addr, uint32_t data)
{
	uint32_t addr_p = addr & 3;

//	logging->out_debugf("pia w %04x=%02x",addr_p,data);
	if (now_reset) return;

	switch (addr_p) {
		case 0:
			// pia DRA or DDRA
			if (cra & 0x04) {
				dra = (data & ddra & 0xff);
				// output to device a
				write_signals(&outputs_pa, data);

			} else {
				ddra = data & 0xff;
			}
			break;
		case 1:
			// pia CRA
			cra = data & 0x3f;
			// ca2 output
			if ((cra & 0x30) == 0x30) {
				set_ca2((cra & 0x08) ? 1 : 0);
			}
			// irq interrupt
			if ((cra & 0x81) == 0x81 || (cra & 0x48) == 0x48) {
				set_irqa(true);
			}
			break;
		case 2:
			// pia DRB or DDRB
			if (crb & 0x04) {
				drb = (data & ddrb & 0xff);
				// output to device b
				write_signals(&outputs_pb, data);

				if ((crb & 0x30) == 0x20) {
					// cb2 output reset
					set_cb2(0);
					if (crb & 0x08) {
						// cb2 returned "high" in one clock later.
						register_event_by_clock(this, EVENT_PIA_CB2, 1, false, &ca2_register_id);
					}
				}
			} else {
				ddrb = data & 0xff;
			}
			break;
		case 3:
			// pia CRB
			crb = data & 0x3f;
			// cb2 output
			if ((crb & 0x30) == 0x30) {
				set_cb2((crb & 0x08) ? 1 : 0);
			}
			// irq interrupt
			if ((crb & 0x81) == 0x81 || (crb & 0x48) == 0x48) {
				set_irqb(true);
			}
			break;
	}

}

uint32_t PIA::read_io8(uint32_t addr)
{
	uint32_t data = 0;
	uint32_t addr_p = addr & 3;

	if (now_reset) return data;

	switch (addr_p) {
		case 0:
			// pia DRA or DDRA
			if (cra & 0x04) {
				data = (dra & (~ddra) & 0xff);
				// clear IRQA in CRA
				cra = cra & 0x3f;
				set_irqa(false);

				if ((cra & 0x30) == 0x20) {
					// ca2 output reset
					set_ca2(0);
					if(cra & 0x08) {
						// ca2 returned "high" in one clock later.
						register_event_by_clock(this, EVENT_PIA_CA2, 1, false, &cb2_register_id);
					}
				}
			} else {
				data = ddra;
			}
			break;
		case 1:
			// pia CRA
			data = cra;
			break;
		case 2:
			// pia DRB or DDRB
			if (crb & 0x04) {
				data = (drb & (~ddrb) & 0xff);
				// clear IRQB in CRB
				crb = crb & 0x3f;
				set_irqb(false);

			} else {
				data = ddrb;
			}
			break;
		case 3:
			// pia CRB
			data = crb;
			break;
	}

//	logging->out_debugf("pia r %04x=%02x",addr_p,data);

	return data;
}

void PIA::write_signal(int id, uint32_t data, uint32_t mask)
{
	uint8_t new_c = (data & mask) ? 1 : 0;
	uint8_t ca_b, cb_b;

//	logging->out_debugf("pia sigw %d %d %d",id,data,mask);

	switch (id) {
		case SIG_PIA_PA:
			// pa
			dra = (data & mask & 0xff);
			break;
		case SIG_PIA_CA1:
			// ca1
			ca_b = (cra & 0x02) ? 1 : 0;
			if (ca1 != new_c && ca_b == new_c) {	// trigger on/off
				// set cra bit7 (IRQA)
				cra = cra | 0x80;
				if (cra & 0x01) {
					// interrupt
					set_irqa(true);
				}
			}
			if (ca1 != new_c && (cra & 0x38) == 0x20) {
				// ca2 goes "high"
				set_ca2(1);
			}
			ca1 = new_c;
			break;
		case SIG_PIA_CA2:
			// ca2
			if (cra & 0x20) {
				// ca2 output mode


			} else {
				// ca2 input mode
				ca_b = (cra & 0x10) ? 1 : 0;
				if (ca2 != new_c && ca_b == new_c) {	// trigger on/off
					// set cra bit6 (IRQA)
					cra = cra | 0x40;
					if (cra & 0x08) {
						// interrupt
						set_irqa(true);
					}
				}
				ca2 = new_c;
			}
			break;

		case SIG_PIA_PB:
			// pb
			drb = (data & mask & 0xff);
			break;
		case SIG_PIA_CB1:
			// cb1
			cb_b = (crb & 0x02) ? 1 : 0;
			if (cb1 != new_c && cb_b == new_c) {	// trigger on/off
				// set crb bit7 (IRQB)
				crb = crb | 0x80;
				if ((crb & 0x38) == 0x20) {
					// cb2 goes "high"
					set_cb2(1);
				}
				if (crb & 0x01) {
					// interrupt
					set_irqb(true);
				}
			}
			cb1 = new_c;
			break;
		case SIG_PIA_CB2:
			// cb2
			if (crb & 0x20) {
				// cb2 output mode


			} else {
				// cb2 input mode
				cb_b = (crb & 0x10) ? 1 : 0;
				if (cb2 != new_c && cb_b == new_c) {	// trigger on/off
					// set crb bit6 (IRQB)
					crb = crb | 0x40;
					if (crb & 0x08) {
						// interrupt
						set_irqb(true);
					}
				}
				cb2 = new_c;
			}
			break;
		case SIG_CPU_RESET:
			now_reset = (data & mask) ? true : false;
			cancel_my_events();
			reset();
			break;
	}
}

// ----------------------------------------------------------------------------

void PIA::set_ca2(uint8_t val)
{
	if (ca2 != val) {
		ca2 = val;
		if (cra & 0x20) {
			// ca2 is output
			write_signals(&outputs_ca2, val ? 0xffffffff : 0);
		}
	}
}

void PIA::set_cb2(uint8_t val)
{
	if (cb2 != val) {
		cb2 = val;
		if (crb & 0x20) {
			// cb2 is output
			write_signals(&outputs_cb2, val ? 0xffffffff : 0);
		}
	}
}

void PIA::set_irqa(bool val)
{
	if (now_irqa != val){
		write_signals(&outputs_irqa, val ? 0xffffffff : 0);
		now_irqa = val;
	}
}

void PIA::set_irqb(bool val)
{
	if (now_irqb != val){
		write_signals(&outputs_irqb, val ? 0xffffffff : 0);
		now_irqb = val;
	}
}

// ----------------------------------------------------------------------------
// event handler
// ----------------------------------------------------------------------------

void PIA::event_frame()
{
}

void PIA::event_callback(int event_id, int err)
{
//	logging->out_debugf("pia event_callback %d",event_id);

	switch(event_id) {
		case EVENT_PIA_CA2:
			set_ca2(1);
			ca2_register_id = -1;
			break;
		case EVENT_PIA_CB2:
			set_cb2(1);
			cb2_register_id = -1;
			break;
	}
}

// ----------------------------------------------------------------------------
void PIA::save_state(FILEIO *fp)
{
	struct vm_state_st vm_state;

	//
	vm_state_ident.version = Uint16_LE(1);
	vm_state_ident.size = Uint32_LE(sizeof(vm_state_ident) + sizeof(vm_state));

	// copy values
	memset(&vm_state, 0, sizeof(vm_state));
	vm_state.ca2_register_id = Int32_LE(ca2_register_id);
	vm_state.cb2_register_id = Int32_LE(cb2_register_id);

	vm_state.cra = cra;
	vm_state.crb = crb;
	vm_state.dra = dra;
	vm_state.drb = drb;
	vm_state.ddra = ddra;
	vm_state.ddrb = ddrb;
	vm_state.ca = ca1 | (ca2 << 4);
	vm_state.cb = cb1 | (cb2 << 4);

	vm_state.now_irq = (now_irqa ? 1 : 0) | (now_irqb ? 2 : 0);

	fp->Fwrite(&vm_state_ident, sizeof(vm_state_ident), 1);
	fp->Fwrite(&vm_state, sizeof(vm_state), 1);
}

bool PIA::load_state(FILEIO *fp)
{
	vm_state_ident_t vm_state_i;
	struct vm_state_st vm_state;

	READ_STATE_CHUNK(fp, vm_state_i, vm_state);

	// copy values
	ca2_register_id = Int32_LE(vm_state.ca2_register_id);
	cb2_register_id = Int32_LE(vm_state.cb2_register_id);

	cra = vm_state.cra;
	crb = vm_state.crb;
	dra = vm_state.dra;
	drb = vm_state.drb;
	ddra = vm_state.ddra;
	ddrb = vm_state.ddrb;
	ca1 = vm_state.ca & 0x0f;
	ca2 = (vm_state.ca >> 4) & 0x0f;
	cb1 = vm_state.cb & 0x0f;
	cb2 = (vm_state.cb >> 4) & 0x0f;

	now_irqa = (vm_state.now_irq & 1) ? true : false;
	now_irqb = (vm_state.now_irq & 2) ? true : false;

	return true;
}

// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
uint32_t PIA::debug_read_io8(uint32_t addr)
{
	uint32_t data = 0;
	uint32_t addr_p = addr & 3;

	if (now_reset) return data;

	switch (addr_p) {
		case 0:
			// pia DRA or DDRA
			if (cra & 0x04) {
				data = (dra & (~ddra) & 0xff);
			} else {
				data = ddra;
			}
			break;
		case 1:
			// pia CRA
			data = cra;
			break;
		case 2:
			// pia DRB or DDRB
			if (crb & 0x04) {
				data = (drb & (~ddrb) & 0xff);
			} else {
				data = ddrb;
			}
			break;
		case 3:
			// pia CRB
			data = crb;
			break;
	}
	return data;
}

static const _TCHAR *c_reg_names[] = {
	_T("DDRA"),
	_T("CRA"),
	_T("DDRB"),
	_T("CRB"),
	NULL
};

bool PIA::debug_write_reg(uint32_t reg_num, uint32_t data)
{
	switch(reg_num) {
	case 0:
		ddra = data & 0xff;
		return true;
	case 1:
		write_io8(1, data);
		return true;
	case 2:
		ddrb = data & 0xff;
		return true;
	case 3:
		write_io8(3, data);
		return true;
	}
	return false;
}

bool PIA::debug_write_reg(const _TCHAR *reg, uint32_t data)
{
	uint32_t num = find_debug_reg_name(c_reg_names, reg);
	return debug_write_reg(num, data);
}

void PIA::debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	buffer[0] = _T('\0');
	UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%s):%02X"), 0, c_reg_names[0], ddra);
	UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%s):%02X"), 1, c_reg_names[1], cra);
	UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%s):%02X"), 2, c_reg_names[2], ddrb);
	UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%s):%02X"), 3, c_reg_names[3], crb);
}
#endif
