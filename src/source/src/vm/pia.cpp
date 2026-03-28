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

PIA::PIA(VM* parent_vm, EMU* parent_emu, const char* identifier)
 : DEVICE(parent_vm, parent_emu, identifier)
{
	set_class_name("PIA");
	init_output_signals(&outputs_pa);
	init_output_signals(&outputs_ca2);
	init_output_signals(&outputs_irqa);
	init_output_signals(&outputs_pb);
	init_output_signals(&outputs_cb2);
	init_output_signals(&outputs_irqb);
}

PIA::~PIA()
{
}

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
	ora = 0;
	orb = 0;

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
			if (cra & CRA_DDR_SEL) {
				// output data
//				dra = (data & ddra & 0xff);
				ora = (data & 0xff);
				// output to device a
				write_signals(&outputs_pa, data);

			} else {
				// direction (1 as output)
				ddra = data & 0xff;
			}
			break;
		case 1:
			// pia CRA
			cra = data & CRA_IRQ_CLR;
			// ca2 output
			if ((cra & (CRA_CA2_RISE | CRA_CA2_OUT)) == (CRA_CA2_RISE | CRA_CA2_OUT)) {
				set_ca2((cra & CRA_CA2_IRQEN) ? 1 : 0);
			}
			// irq interrupt
			if ((cra & (CRA_IRQ1 | CRA_CA1_IRQEN)) == (CRA_IRQ1 | CRA_CA1_IRQEN) || (cra & (CRA_IRQ2 | CRA_CA2_IRQEN)) == (CRA_IRQ2 | CRA_CA2_IRQEN)) {
				set_irqa(true);
			}
			break;
		case 2:
			// pia DRB or DDRB
			if (crb & CRB_DDR_SEL) {
//				drb = (data & ddrb & 0xff);
				orb = (data & 0xff);
				// output to device b
				write_signals(&outputs_pb, data);

				if ((crb & (CRB_CB2_RISE | CRB_CB2_OUT)) == CRB_CB2_OUT) {
					// cb2 output reset
					set_cb2(0);
					if (crb & CRB_CB2_IRQEN) {
						// cb2 returned "high" in one clock later.
						register_event_by_clock(this, EVENT_PIA_CB2, 1, false, &ca2_register_id);
					}
				}
			} else {
				// direction (1 as output)
				ddrb = data & 0xff;
			}
			break;
		case 3:
			// pia CRB
			crb = data & CRB_IRQ_CLR;
			// cb2 output
			if ((crb & (CRB_CB2_RISE | CRB_CB2_OUT)) == (CRB_CB2_RISE | CRB_CB2_OUT)) {
				set_cb2((crb & CRB_CB2_IRQEN) ? 1 : 0);
			}
			// irq interrupt
			if ((crb & (CRB_IRQ1 | CRB_CB1_IRQEN)) == (CRB_IRQ1 | CRB_CB1_IRQEN) || (crb & (CRB_IRQ2 | CRB_CB2_IRQEN)) == (CRB_IRQ2 | CRB_CB2_IRQEN)) {
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
			if (cra & CRA_DDR_SEL) {
//				data = (dra & (~ddra) & 0xff);
				// if ddra is 1 (output),
				// the logical AND of dra and ora is put on a bus   
				data = (dra & (ora | ~ddra));
				// clear IRQA in CRA
				cra = cra & CRA_IRQ_CLR;
				set_irqa(false);

				if ((cra & (CRA_CA2_OUT | CRA_CA2_RISE)) == CRA_CA2_OUT) {
					// ca2 output reset
					set_ca2(0);
					if(cra & CRA_CA2_IRQEN) {
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
			if (crb & CRB_DDR_SEL) {
//				data = (drb & (~ddrb) & 0xff);
//				data = (drb & 0xff);
				// if ddrb is 1 (output),
				// drb is ignored and orb is put on a bus
				data = ((drb | ddrb) & (orb | ~ddrb));
				// clear IRQB in CRB
				crb = crb & CRB_IRQ_CLR;
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
			ca_b = (cra & CRA_CA1_RISE) ? 1 : 0;
			if (ca1 != new_c && ca_b == new_c) {	// trigger on/off
				// set cra bit7 (IRQA)
				cra = cra | CRA_IRQ1;
				if (cra & CRA_CA1_IRQEN) {
					// interrupt
					set_irqa(true);
				}
			}
			if (ca1 != new_c && (cra & CRA_CA2_CTRL) == CRA_CA2_OUT) {
				// ca2 goes "high"
				set_ca2(1);
			}
			ca1 = new_c;
			break;
		case SIG_PIA_CA2:
			// ca2
			if (cra & CRA_CA2_OUT) {
				// ca2 output mode


			} else {
				// ca2 input mode
				ca_b = (cra & CRA_CA2_RISE) ? 1 : 0;
				if (ca2 != new_c && ca_b == new_c) {	// trigger on/off
					// set cra bit6 (IRQA)
					cra = cra | CRA_IRQ2;
					if (cra & CRA_CA2_IRQEN) {
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
			cb_b = (crb & CRB_CB1_RISE) ? 1 : 0;
			if (cb1 != new_c && cb_b == new_c) {	// trigger on/off
				// set crb bit7 (IRQB)
				crb = crb | CRB_IRQ1;
				if ((crb & CRB_CB2_CTRL) == CRB_CB2_OUT) {
					// cb2 goes "high"
					set_cb2(1);
				}
				if (crb & CRB_CB1_IRQEN) {
					// interrupt
					set_irqb(true);
				}
			}
			cb1 = new_c;
			break;
		case SIG_PIA_CB2:
			// cb2
			if (crb & CRB_CB2_OUT) {
				// cb2 output mode


			} else {
				// cb2 input mode
				cb_b = (crb & CRB_CB2_RISE) ? 1 : 0;
				if (cb2 != new_c && cb_b == new_c) {	// trigger on/off
					// set crb bit6 (IRQB)
					crb = crb | CRB_IRQ2;
					if (crb & CRB_CB2_IRQEN) {
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
	vm_state_ident.version = Uint16_LE(2);
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
	vm_state.ora = ora;
	vm_state.orb = orb;

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

	if (Uint16_LE(vm_state_i.version) >= 2) {
		ora = vm_state.ora;
		orb = vm_state.orb;
	} else {
		ora = dra;
		orb = drb;
	}

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
			if (cra & CRA_DDR_SEL) {
				// if ddra is 1 (output),
				// the logical AND of dra and ora is put on a bus   
				data = (dra & (ora | ~ddra));
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
			if (crb & CRB_DDR_SEL) {
				// if ddrb is 1 (output),
				// drb is ignored and orb is put on a bus
				data = ((drb | ddrb) & (orb | ~ddrb));
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
	_T("DRA"),
	_T("ORA"),
	_T("DDRA"),
	_T("CRA"),
	_T("DRB"),
	_T("ORB"),
	_T("DDRB"),
	_T("CRB"),
	NULL
};

bool PIA::debug_write_reg(uint32_t reg_num, uint32_t data)
{
	switch(reg_num) {
	case 0:
		dra = data & 0xff;
		return true;
	case 1:
		ora = data & 0xff;
		return true;
	case 2:
		ddra = data & 0xff;
		return true;
	case 3:
		write_io8(1, data);
		return true;
	case 4:
		drb = data & 0xff;
		return true;
	case 5:
		orb = data & 0xff;
		return true;
	case 6:
		ddrb = data & 0xff;
		return true;
	case 7:
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

void PIA::debug_regs_info(const _TCHAR *title, _TCHAR *buffer, size_t buffer_len)
{
	UTILITY::tcscpy(buffer, buffer_len, _T("HD6821/MC6821 ("));
	UTILITY::tcscat(buffer, buffer_len, title);
	UTILITY::tcscat(buffer, buffer_len, _T(") Registers:\n"));
	UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%s):%02X"), 0, c_reg_names[0], dra);
	UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%s):%02X"), 1, c_reg_names[1], ora);
	UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%s):%02X"), 2, c_reg_names[2], ddra);
	UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%s):%02X"), 3, c_reg_names[3], cra);
	UTILITY::tcscat(buffer, buffer_len, _T("\n"));
	UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%s):%02X"), 4, c_reg_names[4], drb);
	UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%s):%02X"), 5, c_reg_names[5], orb);
	UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%s):%02X"), 6, c_reg_names[6], ddrb);
	UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%s):%02X"), 7, c_reg_names[7], crb);
}
#endif
