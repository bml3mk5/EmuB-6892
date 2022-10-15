/** @file via.cpp

	HITACHI BASIC MASTER LEVEL3 Mark5 / MB-S1 Emulator 'EmuB-6892/EmuB-S1'
	Skelton for retropc emulator

	@author Sasaji
	@date   2012.06.08 -

	@brief [ via modoki (mcs6522) ]
*/

#include "via.h"
//#include "../../emu.h"
#include "../fileio.h"
#include "../utility.h"

void VIA::initialize()
{
	ca2_register_id = -1;
	cb2_register_id = -1;
	t1_register_id = -1;
	t2_register_id = -1;
	s2_register_id = -1;
}

void VIA::reset()
{
	drb = 0;
	dra = 0;
	ddrb = 0;
	ddra = 0;

	t1l = 0;
	t1c = 0;
	t2l = 0;
	t2c = 0;

	sr  = 0;
	acr = 0;
	pcr = 0;
	ifr = 0;
	ier = 0;

	cra = 0;
	crb = 0;

	ca1 = ca2 = 0;
	cb1 = cb2 = 0;

	t1_pb7 = 0;
	t2_pb6 = 0;

	timer1_clock = 0;
	timer2_clock = 0;
	shift_timer = 0;
	shift_count = 0;

	now_irq = 0;

	ca2_register_id = -1;
	cb2_register_id = -1;
	t1_register_id = -1;
	t2_register_id = -1;
	s2_register_id = -1;
}

void VIA::cancel_my_events()
{
	if (ca2_register_id != -1) cancel_event(this, ca2_register_id);
	if (cb2_register_id != -1) cancel_event(this, cb2_register_id);
	if (t1_register_id != -1) cancel_event(this, t1_register_id);
	if (t2_register_id != -1) cancel_event(this, t2_register_id);
	if (s2_register_id != -1) cancel_event(this, s2_register_id);
	ca2_register_id = -1;
	cb2_register_id = -1;
	t1_register_id = -1;
	t2_register_id = -1;
	s2_register_id = -1;
}

void VIA::write_io8(uint32_t addr, uint32_t data)
{
	uint32_t addr_p = addr & 0x0f;

//	logging->out_debugf("via w %04x=%02x",addr_p,data);
	if (now_reset) return;

	switch (addr_p) {
		case 0:
			// ORB
			drb = (data & ddrb) | (drb & ~ddrb);
			// output to device b
			write_signals(&outputs_pb, data);
			// clear ifr bit4
			ifr = ifr & 0xef;
			if ((pcr & 0xa0) == 0x20) {
				// clear ifr bit3
				ifr = ifr & 0xf7;
			}
			// output to CB2
			if ((pcr & 0xc0) == 0x80) {
				// CB2 goes "low"
				set_cb2(0);
				if ((pcr & 0xe0) == 0xa0) {
					// CB2 returned "high" in one clock later.
					register_event_by_clock(this, EVENT_VIA_CB2, clk_unit, false, &cb2_register_id);
				}
			}

			break;
		case 1:
			// ORA
			dra = (data & ddra) | (dra & ~ddra);
			// output to device a
			write_signals(&outputs_pa, data);
			// clear ifr bit1
			ifr = ifr & 0xfd;
			if ((pcr & 0x0a) == 0x02) {
				// clear ifr bit0
				ifr = ifr & 0xfe;
			}
			// output to CA2
			if ((pcr & 0x0c) == 0x08) {
				// CA2 goes "low"
				set_ca2(0);
				if ((pcr & 0x0e) == 0x0a) {
					// CA2 returned "high" in one clock later.
					register_event_by_clock(this, EVENT_VIA_CA2, clk_unit, false, &ca2_register_id);
				}
			}

			break;
		case 2:
			// DDRB
			ddrb = data & 0xff;
			break;
		case 3:
			// DDRA
			ddra = data & 0xff;
			break;
		case 4:
		case 6:
			// set T1L Low order
			t1l = (t1l & 0xff00) | (data & 0xff);
			break;
		case 5:
			// set T1L High order
			t1l = ((data & 0xff) << 8) | (t1l & 0xff);
			t1c = t1l;
			// clear ifr bit6
			ifr &= 0xbf;
			if (ier & 0x40) {
				set_irq(ifr, 0x40);
			}
			// start count down
			if (acr & ddrb & 0x80) {
				// PB7 goes "low"
				// output to device b
				write_signals(&outputs_pb, drb & 0x7f);
			}
			t1_pb7 = 0;
			if (t1_register_id != -1) {
				cancel_event(this, t1_register_id);
				t1_register_id = -1;
			}
			timer1_clock = get_current_clock();
			register_event_by_clock(this, EVENT_VIA_TIMER1, (t1c + 1) * clk_unit, false, &t1_register_id);

//			logging->out_debugf("via timer1_start clk:%6d t1c:%5d acr:%02x pb7:%d",timer1_clock,t1c,acr,t1_pb7);

			break;
		case 7:
			// set T1L High order
			t1l = ((data & 0xff) << 8) | (t1l & 0xff);
			break;
		case 8:
			// set T2L Low order
			t2l = (t2l & 0xff00) | (data & 0xff);
			break;
		case 9:
			// set T2C High order
			t2c = ((data & 0xff) << 8) | (t2l & 0xff);
			// clear ifr bit5
			ifr &= 0xdf;
			if (ier & 0x20) {
				set_irq(ifr, 0x20);
			}
			// start count down
			t2_pb6 = 1;
			if (t2_register_id != -1) {
				cancel_event(this, t2_register_id);
				t2_register_id = -1;
			}
			if ((acr & 0x20) == 0) {
				// is not pulse mode
				timer2_clock = get_current_clock();
				register_event_by_clock(this, EVENT_VIA_TIMER2, (t2c + 1) * clk_unit, false, &t2_register_id);

//				logging->out_debugf("via timer2_start clk:%6d t2c:%5d acr:%02x",timer2_clock,t2c,acr);
			} else {

//				logging->out_debugf("via timer2_start pluse t2c:%5d acr:%02x",t2c,acr);
			}

			break;
		case 10:
			// SR
			sr = data & 0xff;
			// clear ifr bit2
			ifr &= 0xfb;
			if (ier & 0x04) {
				set_irq(ifr, 0x04);
			}
			// shift start count down
			if (acr & 0x1c) {
				shift_count = 0;

				if ((acr & 0x0c) != 0x0c) {
					// CB1 is output clock
					if (s2_register_id != -1) {
						cancel_event(this, s2_register_id);
					}
					register_event_by_clock(this, EVENT_VIA_SHIFT2_LOW, 2 * clk_unit, false, &s2_register_id);
				}
			}
			break;
		case 11:
			// ACR
			acr = data & 0xff;
			break;
		case 12:
			// PCR
			pcr = data & 0xff;
			if ((pcr & 0xc0) == 0xc0) {
				// CB2 output mode
				set_cb2((pcr & 0x20) ? 1 : 0);
			}
			if ((pcr & 0x0c) == 0x0c) {
				// CA2 output mode
				set_ca2((pcr & 0x02) ? 1 : 0);
			}

			break;
		case 13:
			// IFR
			ifr = data & 0xff;
			// update irq
			set_irq((ifr & 0x7f), (ier & 0x7f));
			break;
		case 14:
			// IER
			if (data & 0x80) {
				ier = data & 0xff;
			} else {
				ier = ~(data & 0x7f);
			}
			// update irq
			set_irq((ifr & 0x7f), (ier & 0x7f));
			break;
		case 15:
			// ORA (no handshake)
			dra = (data & ddra) | (dra & ~ddra);
			break;
	}

}

uint32_t VIA::read_io8(uint32_t addr)
{
	uint32_t data = 0;
	uint32_t addr_p = addr & 0x0f;

	if (now_reset) return data;

	switch (addr_p) {
		case 0:
			// IRB
			data = (drb & 0xff);
			// clear ifr bit4
			ifr = ifr & 0xef;
			if ((pcr & 0xa0) == 0x20) {
				// clear ifr bit3
				ifr = ifr & 0xf7;
			}
			break;
		case 1:
			// IRA
			data = (dra & 0xff);
			// clear ifr bit1
			ifr = ifr & 0xfd;
			if ((pcr & 0x0a) == 0x02) {
				// clear ifr bit0
				ifr = ifr & 0xfe;
			}
			break;
		case 2:
			// DDRB
			data = ddrb;
			break;
		case 3:
			// DDRA
			data = ddra;
			break;
		case 4:
			// read from T1C Low order
			calc_clock(t1c, timer1_clock);
			data = t1c & 0xff;
			// clear ifr bit6
			ifr &= 0xbf;
			if (ier & 0x40) {
				set_irq(ifr, 0x40);
			}
			break;
		case 5:
			// read from T1C High order
			calc_clock(t1c, timer1_clock);
			data = (t1c & 0xff00) >> 8;
			break;
		case 6:
			// read from T1L Low order
			data = t1l & 0xff;
			break;
		case 7:
			// read from T1L High order
			data = (t1l & 0xff00) >> 8;
			break;
		case 8:
			// read from T2C Low order
			if ((acr & 0x20) == 0) {
				calc_clock(t2c, timer2_clock);
			}
			data = t2c & 0xff;
			// clear ifr bit5
			ifr &= 0xdf;
			if (ier & 0x20) {
				set_irq(ifr, 0x20);
			}
			break;
		case 9:
			// read from T2C High order
			if ((acr & 0x20) == 0) {
				calc_clock(t2c, timer2_clock);
			}
			data = (t2c & 0xff00) >> 8;
			break;
		case 10:
			// SR
			data = sr;
			// clear ifr bit2
			ifr &= 0xfb;
			if (ier & 0x04) {
				set_irq(ifr, 0x04);
			}
			// shift start count down
			if (acr & 0x1c) {
				shift_count = 0;

				if ((acr & 0x0c) != 0x0c) {
					// CB1 is output clock
					if (s2_register_id != -1) {
						cancel_event(this, s2_register_id);
					}
					register_event_by_clock(this, EVENT_VIA_SHIFT2_LOW, 2 * clk_unit, false, &s2_register_id);
				}
			}
			break;
		case 11:
			// ACR
			data = acr;
			break;
		case 12:
			// PCR
			data = pcr;
			break;
		case 13:
			// IFR
			data = ifr;
			break;
		case 14:
			// IER
			data = ier;
			break;
		case 15:
			// IRA (no handshake)
			data = (dra & 0xff);
			break;
	}

//	logging->out_debugf("via r %04x=%02x",addr_p,data);

	return data;
}

void VIA::write_signal(int id, uint32_t data, uint32_t mask)
{
	uint8_t new_c = (data & mask) ? 1 : 0;
	uint8_t ca_b, cb_b;

//	logging->out_debugf("via sigw %d %d %d",id,data,mask);

	switch (id) {
		case SIG_VIA_PA:
			// pa
			if ((acr & 0x01) != 0 || (ifr & 0x02) == 0) {
				// PA latch enable , but CA1 is not active.
				dra_res = (data & mask & 0xff);
			} else {
				dra = (data & mask & 0xff);
				dra_res = dra;
			}
			break;
		case SIG_VIA_CA1:
			// ca1
			ca_b = (pcr & 0x01) ? 1 : 0;
			if (ca1 != new_c && ca_b == new_c) {	// trigger on/off
				// active edge
				// set ifr bit1
				ifr = ifr | 0x02;
				// set PA
				dra = dra_res;

				if (ier & 0x02) {
					// interrupt
					set_irq(ifr, 0x02);
				}
			}
			ca1 = new_c;
			break;
		case SIG_VIA_CA2:
			// ca2
			if (pcr & 0x08) {
				// ca2 output mode


			} else {
				// ca2 input mode
				ca_b = (pcr & 0x04) ? 1 : 0;
				if (ca2 != new_c && ca_b == new_c) {	// trigger on/off
					// set ifr bit0
					ifr = ifr | 0x01;
					if (ier & 0x01) {
						// interrupt
						set_irq(ifr, 0x01);
					}
				}
				ca2 = new_c;
			}
			break;

		case SIG_VIA_PB:
			// pb
			if ((acr & 0x02) != 0 || (ifr & 0x10) == 0) {
				// PB latch enable , but CB1 is not active.
				drb_res = (drb_res & ddrb) | (data & ~ddrb & mask);
			} else {
				drb = (drb & ddrb) | (data & ~ddrb & mask);
				drb_res = drb;
			}
			// PB6 pulse in Timer2 mode
			if (acr & 0x20) {
				int n_pb6 = (data & 0x40) ? 1 : 0;
				if (t2_pb6 == 0 && n_pb6 == 1) {
					// trigger on
					t2c--;
//					logging->out_debugf("via timer2_pluse t2c:%5d acr:%02x",t2c,acr);

					if (t2c == 0) {
						// finish

						// set ifr bit5
						ifr |= 0x20;
						if (ier & 0x20) {
							set_irq(ifr, 0x20);
						}
//						logging->out_debugf("via timer2_pluse finish t2c:%5d acr:%02x",t2c,acr);
					}
				}
				t2_pb6 = n_pb6;
			}

			break;
		case SIG_VIA_CB1:
			// cb1
			cb_b = (pcr & 0x10) ? 1 : 0;
			if (cb1 != new_c && cb_b == new_c) {	// trigger on/off
				// active edge
				// set ifr bit4
				ifr = ifr | 0x10;
				// set PB
				drb = drb_res;

				if (ier & 0x10) {
					// interrupt
					set_irq(ifr, 0x10);
				}
			}
			if ((acr & 0x0c) == 0x0c) {
				// In shift mode using CB1 input clock
				if (cb1 == 0 && new_c == 1) {
					// CB1 trigger on.
					shift();
				}
			}
			cb1 = new_c;
			break;
		case SIG_VIA_CB2:
			// cb2
			if (pcr & 0x80) {
				// cb2 output mode


			} else {
				// cb2 input mode
				cb_b = (pcr & 0x40) ? 1 : 0;
				if (cb2 != new_c && cb_b == new_c) {	// trigger on/off
					// set ifr bit3
					ifr = ifr | 0x08;
					if (ier & 0x08) {
						// interrupt
						set_irq(ifr, 0x08);
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
		case SIG_VIA_CLOCK_UNIT:
			clk_unit = (data & mask);
			break;
	}
}

// ----------------------------------------------------------------------------

void VIA::set_ca2(uint8_t val)
{
	if (ca2 != val) {
		ca2 = val;
		if (pcr & 0x08) {
			// ca2 is output
			write_signals(&outputs_ca2, val ? 0xffffffff : 0);
		}
	}
}

void VIA::set_cb1(uint8_t val)
{
	if (cb1 != val) {
		cb1 = val;
		// cb1 is output
		write_signals(&outputs_cb1, val ? 0xffffffff : 0);
	}
}

void VIA::set_cb2(uint8_t val)
{
	if (cb2 != val) {
		cb2 = val;
		if ((pcr & 0x80) || (acr & 0x10)) {
			// cb2 is output
			write_signals(&outputs_cb2, val ? 0xffffffff : 0);
		}
	}
}

void VIA::set_irq(uint8_t data, uint8_t mask)
{
//	logging->out_debugf("via irq now:%02x d:%02x m:%02x",now_irq,data,mask);

	if (data & mask) {
		if ((now_irq & data & mask) == 0) {
			// set ifr bit7
			ifr |= 0x80;
			write_signals(&outputs_irq, 0xffffffff);
		}
		now_irq = now_irq | (data & mask);

//		logging->out_debugf("via irq set now:%02x ifr:%02x",now_irq,ifr);
	} else {
		now_irq = (now_irq & ~mask) | (data & mask);
		// all clear
		if (now_irq == 0) {
			// clear ifr bit7
			ifr &= 0x7f;
			write_signals(&outputs_irq, 0);
		}

//		logging->out_debugf("via irq clear now:%02x ifr:%02x",now_irq,ifr);
	}
}

void VIA::calc_clock(int &tc, uint64_t &clock)
{
	uint64_t cur_clock = get_current_clock();
	if (cur_clock >= clock) {
		tc = tc - (int)(cur_clock - clock);
	} else {
		tc = tc - (int)(0xffffffffffffffffLL - cur_clock + clock + 1);
	}
	clock = cur_clock;
}

// ----------------------------------------------------------------------------

void VIA::shift_low()
{
	// CB1 goes "low"
	if ((acr & 0x0c) != 0x0c) {
		set_cb1(0);
	}

//	logging->out_debugf("via shift_low clk:%6d cnt:%d sr:%02x acr:%02x cb1:%02x cb2:%02x",get_current_clock(),shift_count,sr,acr,cb1,cb2);

	if ((acr & 0x0c) == 0 || (acr & 0x0c) == 0x04) {
		// timer set from T2L
		shift_timer = (t2l & 0xff) + 2;
	} else if ((acr & 0x0c) == 0x08) {
		// timer set from CLK
		shift_timer = 1;
	}
	if (s2_register_id != -1) {
		cancel_event(this, s2_register_id);
	}
	register_event_by_clock(this, EVENT_VIA_SHIFT2_HIGH, shift_timer * clk_unit, false, &s2_register_id);
}

void VIA::shift()
{
	if (acr & 0x10) {
		// output to CB2
		uint8_t bit = (sr & 1);
		sr = (sr >> 1) | (bit << 7);
		set_cb2(bit);
	}

	// CB1 goes "high"
	if ((acr & 0x0c) != 0x0c) {
		set_cb1(1);
	}

	if ((acr & 0x10) == 0) {
		// input from CB2
		// shift into register
		sr = (sr << 1) | (cb2 & 1);
	}

	shift_count++;

//	logging->out_debugf("via shift clk:%6d cnt:%d sr:%02x acr:%02x cb1:%02x cb2:%02x",get_current_clock(),shift_count,sr,acr,cb1,cb2);

	if (shift_count < 8 || (acr & 0x1c) == 0x10) {
		// continue

		if ((acr & 0x0c) == 0 || (acr & 0x0c) == 0x04) {
			// timer set from T2L
			shift_timer = (t2l & 0xff) + 2;
		} else if ((acr & 0x0c) == 0x08) {
			// timer set from CLK
			shift_timer = 1;
		}

		if ((acr & 0x0c) != 0x0c) {
			// CB1 clock output
			if (s2_register_id != -1) {
				cancel_event(this, s2_register_id);
			}
			register_event_by_clock(this, EVENT_VIA_SHIFT2_LOW, shift_timer * clk_unit, false, &s2_register_id);
		}
	} else {
		// last

		if ((acr & 0x10) == 0) {
			// CB2 input mode
			if (s2_register_id != -1) {
				cancel_event(this, s2_register_id);
			}
			// set intr
			register_event_by_clock(this, EVENT_VIA_SHIFT2_INTR, clk_unit, false, &s2_register_id);
		} else {
			// CB2 output mode
			// set intr immediately
			set_shift_intr();
		}
	}
}

void VIA::set_shift_intr()
{
//	logging->out_debugf("via set_shift_intr clk:%6d cnt:%d sr:%02x acr:%02x cb1:%02x cb2:%02x",get_current_clock(),shift_count,sr,acr,cb1,cb2);

	// set ifr bit2
	ifr |= 0x04;
	if (ier & 0x04) {
		set_irq(ifr, 0x04);
	}
	if (s2_register_id != -1) {
		cancel_event(this, s2_register_id);
	}
	s2_register_id = -1;
}

// ----------------------------------------------------------------------------
// event handler
// ----------------------------------------------------------------------------

void VIA::event_frame()
{
}

void VIA::event_callback(int event_id, int err)
{

	switch(event_id) {
		case EVENT_VIA_CA2:
			set_ca2(1);
			ca2_register_id = -1;
			break;
		case EVENT_VIA_CB2:
			set_cb2(1);
			cb2_register_id = -1;
			break;
		case EVENT_VIA_TIMER1:
			// time up

			cancel_event(this, t1_register_id);
			t1_register_id = -1;

			// set ifr bit6
			ifr |= 0x40;
			if (ier & 0x40) {
				set_irq(ifr, 0x40);
			}

//			logging->out_debugf("via event_timer1 clk:%6d t1c:%5d acr:%02x pb7:%d",get_current_clock(),t1c,acr,t1_pb7);

			if (acr & 0x40) {
				// free-run mode
				t1c = t1l;
				t1_pb7 = 1 - t1_pb7;
				if (acr & ddrb & 0x80) {
					// PB7 trigger edge
					// output to device b
					write_signals(&outputs_pb, t1_pb7 ? (drb | 0x80) : (drb & 0x7f));
				}
				// set next event
				timer1_clock = get_current_clock();
				register_event_by_clock(this, EVENT_VIA_TIMER1, (t1c + t1_pb7 + 1) * clk_unit, false, &t1_register_id);

//				logging->out_debugf("via event_timer1 next -> clk:%6d t1c:%5d acr:%02x pb7:%d",timer1_clock,t1c,acr,t1_pb7);
			}

			break;
		case EVENT_VIA_TIMER2:
			// time up

			cancel_event(this, t2_register_id);
			t2_register_id = -1;

			// set ifr bit5
			ifr |= 0x20;
			if (ier & 0x20) {
				set_irq(ifr, 0x20);
			}

//			logging->out_debugf("via event_timer2 clk:%6d t2c:%5d acr:%02x",get_current_clock(),t2c,acr);

			break;

		case EVENT_VIA_SHIFT2_LOW:
			// shift mode

			// CB1 clock goes "low"
			shift_low();

			break;
		case EVENT_VIA_SHIFT2_HIGH:
			// shift mode

			// CB1 clock goes "high" and shift register
			shift();

			break;
		case EVENT_VIA_SHIFT2_INTR:
			// shift mode

			// finished shift and interrupt flag set
			set_shift_intr();

			break;
	}
}

// ----------------------------------------------------------------------------
void VIA::save_state(FILEIO *fp)
{
	struct vm_state_st vm_state;

	//
	vm_state_ident.version = Uint16_LE(2);
	vm_state_ident.size = Uint32_LE(sizeof(vm_state_ident) + sizeof(vm_state));

	// copy values
	memset(&vm_state, 0, sizeof(vm_state));
	vm_state.ca2_register_id = Int32_LE(ca2_register_id);
	vm_state.cb2_register_id = Int32_LE(cb2_register_id);
	vm_state.t1_register_id = Int32_LE(t1_register_id);
	vm_state.t2_register_id = Int32_LE(t2_register_id);
	vm_state.s2_register_id = Int32_LE(s2_register_id);

	vm_state.drb = drb;
	vm_state.dra = dra;
	vm_state.ddrb = ddrb;
	vm_state.ddra = ddra;
	vm_state.t1l = Int32_LE(t1l);
	vm_state.t1c = Int32_LE(t1c);
	vm_state.t2l = Int32_LE(t2l);
	vm_state.t2c = Int32_LE(t2c);
	vm_state.sr = sr;
	vm_state.acr = acr;
	vm_state.pcr = pcr;
	vm_state.ifr = ifr;
	vm_state.ier = ier;
	vm_state.cra = cra;
	vm_state.crb = crb;
	vm_state.ca1 = ca1;
	vm_state.ca2 = ca2;
	vm_state.cb1 = cb1;
	vm_state.cb2 = cb2;
	vm_state.drb_res = drb_res;
	vm_state.dra_res = dra_res;
	vm_state.t1_pb7 = t1_pb7;
	vm_state.t2_pb6 = t2_pb6;

	vm_state.now_irq = now_irq;

	// version 2
	vm_state.d.v2.timer1_clock = Uint64_LE(timer1_clock);
	vm_state.d.v2.timer2_clock = Uint64_LE(timer2_clock);
	vm_state.d.v2.shift_count = Int32_LE(shift_count);

	fp->Fwrite(&vm_state_ident, sizeof(vm_state_ident), 1);
	fp->Fwrite(&vm_state, sizeof(vm_state), 1);
}

bool VIA::load_state(FILEIO *fp)
{
	vm_state_ident_t vm_state_i;
	struct vm_state_st vm_state;

	READ_STATE_CHUNK(fp, vm_state_i, vm_state);

	// copy values
	ca2_register_id = Int32_LE(vm_state.ca2_register_id);
	cb2_register_id = Int32_LE(vm_state.cb2_register_id);
	t1_register_id = Int32_LE(vm_state.t1_register_id);
	t2_register_id = Int32_LE(vm_state.t2_register_id);
	s2_register_id = Int32_LE(vm_state.s2_register_id);

	drb = vm_state.drb;
	dra = vm_state.dra;
	ddrb = vm_state.ddrb;
	ddra = vm_state.ddra;
	t1l = Int32_LE(vm_state.t1l);
	t1c = Int32_LE(vm_state.t1c);
	t2l = Int32_LE(vm_state.t2l);
	t2c = Int32_LE(vm_state.t2c);
	sr = vm_state.sr;
	acr = vm_state.acr;
	pcr = vm_state.pcr;
	ifr = vm_state.ifr;
	ier = vm_state.ier;
	cra = vm_state.cra;
	crb = vm_state.crb;
	ca1 = vm_state.ca1;
	ca2 = vm_state.ca2;
	cb1 = vm_state.cb1;
	cb2 = vm_state.cb2;
	drb_res = vm_state.drb_res;
	dra_res = vm_state.dra_res;
	t1_pb7 = vm_state.t1_pb7;
	t2_pb6 = vm_state.t2_pb6;

	now_irq = vm_state.now_irq;

	if (Uint16_LE(vm_state_i.version) == 1) {
		// format version 1
		timer1_clock = Uint32_LE(vm_state.d.v1.timer1_clock);
		timer2_clock = Uint32_LE(vm_state.d.v1.timer2_clock);
		shift_count = Int32_LE(vm_state.d.v1.shift_count);
	} else {
		// format version 2
		timer1_clock = Uint64_LE(vm_state.d.v2.timer1_clock);
		timer2_clock = Uint64_LE(vm_state.d.v2.timer2_clock);
		shift_count = Int32_LE(vm_state.d.v2.shift_count);
	}

	return true;
}

// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
uint32_t VIA::debug_read_io8(uint32_t addr)
{
	uint32_t data = 0;
	uint32_t addr_p = addr & 0x0f;

	if (now_reset) return data;

	switch (addr_p) {
		case 0:
			// IRB
			data = (drb & 0xff);
			break;
		case 1:
			// IRA
			data = (dra & 0xff);
			break;
		case 2:
			// DDRB
			data = ddrb;
			break;
		case 3:
			// DDRA
			data = ddra;
			break;
		case 4:
			data = t1c & 0xff;
			break;
		case 5:
			data = (t1c & 0xff00) >> 8;
			break;
		case 6:
			// read from T1L Low order
			data = t1l & 0xff;
			break;
		case 7:
			// read from T1L High order
			data = (t1l & 0xff00) >> 8;
			break;
		case 8:
			data = t2c & 0xff;
			break;
		case 9:
			// read from T2C High order
			data = (t2c & 0xff00) >> 8;
			break;
		case 10:
			// SR
			data = sr;
			break;
		case 11:
			// ACR
			data = acr;
			break;
		case 12:
			// PCR
			data = pcr;
			break;
		case 13:
			// IFR
			data = ifr;
			break;
		case 14:
			// IER
			data = ier;
			break;
		case 15:
			// IRA (no handshake)
			data = (dra & 0xff);
			break;
	}
	return data;
}

static const _TCHAR *c_reg_names[] = {
	_T("IRB/ORB"),
	_T("IRA/ORA"),
	_T("DDRB"),
	_T("DDRA"),
	_T("T1L"),
	_T("T1C"),
	_T("T2L"),
	_T("T2C"),
	_T("SR"),
	_T("ACR"),
	_T("PCR"),
	_T("IFR"),
	_T("IER"),
	NULL
};

bool VIA::debug_write_reg(uint32_t reg_num, uint32_t data)
{
	switch(reg_num) {
	case 0:
	case 1:
	case 2:
	case 3:
		write_io8(reg_num, data);
		return true;
	case 4:
		t1l = data & 0xffff;
		return true;
	case 5:
		t1c = data & 0xffff;
		return true;
	case 6:
		t2l = data & 0xffff;
		return true;
	case 7:
		t2c = data & 0xffff;
		return true;
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
		write_io8(reg_num+2, data);
		return true;
	}
	return false;
}

bool VIA::debug_write_reg(const _TCHAR *reg, uint32_t data)
{
	uint32_t num = find_debug_reg_name(c_reg_names, reg);
	return debug_write_reg(num, data);
}

void VIA::debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	buffer[0] = _T('\0');
	UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%s):%02X"), 0, c_reg_names[0], drb);
	UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%s):%02X"), 1, c_reg_names[1], dra);
	UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%s):%02X"), 2, c_reg_names[2], ddrb);
	UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%s):%02X"), 3, c_reg_names[3], ddra);
	UTILITY::tcscat(buffer, buffer_len, _T("\n"));
	UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%s):%04X"), 4, c_reg_names[4], t1l);
	UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%s):%04X"), 5, c_reg_names[5], t1c);
	UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%s):%04X"), 6, c_reg_names[6], t2l);
	UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%s):%04X"), 7, c_reg_names[7], t2c);
	UTILITY::tcscat(buffer, buffer_len, _T("\n"));
	UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%s):%02X"), 8, c_reg_names[8], sr);
	UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%s):%02X"), 9, c_reg_names[9], acr);
	UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%s):%02X"), 10, c_reg_names[10], pcr);
	UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%s):%02X"), 11, c_reg_names[11], ifr);
	UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%s):%02X"), 12, c_reg_names[12], ier);
}

#endif
