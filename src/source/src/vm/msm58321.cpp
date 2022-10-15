/** @file msm58321.cpp

	Skelton for retropc emulator

	@author Takeda.Toshiya
	@date   2008.05.02-

	@brief [ MSM58321/MSM5832 ]
*/

#include "msm58321.h"
//#include "../emu.h"
#include "../fileio.h"
#include "../utility.h"

#ifndef MSM58321_START_DAY
#define MSM58321_START_DAY 0
#endif
#ifndef MSM58321_START_YEAR
#define MSM58321_START_YEAR 0
#endif

void MSM58321::initialize()
{
	// init rtc
	memset(regs, 0, sizeof(regs));
	regs[5] = 8; // 24h
	regs[15] = 0x0f;
	wreg = regnum = 0;
	cs = false;
	rd = wr = addr_wr = busy = false;
	hold = 0;
	count_1024hz = count_1s = count_1m = count_1h = 0;
	register_id = -1;
	register_id_pulse = -1;

	reset();
}

// power on reset
// set current host time to register again
void MSM58321::reset()
{
	cur_time.GetHostTime();
	read_from_cur_time();

	// register events
	if (register_id != -1) cancel_event(this, register_id);
#if defined(HAS_MSM5832) || !defined(USE_MSM58321_BUSY)
	register_event(this, EVENT_INC, 1000000.0, true, &register_id);
#else
	register_event(this, EVENT_BUSY, 1000000.0, true, &register_id);
#endif
	if (register_id_pulse != -1) cancel_event(this, register_id_pulse);
	register_event(this, EVENT_PULSE, 1000000.0 / 8192.0, true, &register_id_pulse);	// 122.1 usec
}

void MSM58321::enable(bool value)
{
	if (!value) {
		// disable this device
		if (register_id != -1) cancel_event(this, register_id);
		register_id = -1;
		if (register_id_pulse != -1) cancel_event(this, register_id_pulse);
		register_id_pulse = -1;
	}
	DEVICE::enable(value);
}

void MSM58321::event_callback(int event_id, int err)
{
	if(event_id == EVENT_BUSY) {
		set_busy(true);
		register_event(this, EVENT_INC, 430, false, NULL);
	}
	else if(event_id == EVENT_INC) {
		if(!hold) {
			cur_time.Increment();
#ifdef USE_MSM58321_OUTPUTS_DATA
			read_from_cur_time();
			if(regnum <= 12) {
				output_data();
			}
#endif
		}
		set_busy(false);
	}
	else if(event_id == EVENT_PULSE) {
		if(++count_1024hz == 4) {
			count_1024hz = 0;
			regs[15] ^= 1;
		}
		if(++count_1s == 8192) {
			count_1s = 0;
			regs[15] &= ~2;
		} else {
			regs[15] |= 2;
		}
		if(++count_1m == 60 * 8192) {
			count_1m = 0;
			regs[15] &= ~4;
		} else {
			regs[15] |= 4;
		}
		if(++count_1h == 3600 * 8192) {
			count_1h = 0;
			regs[15] &= ~8;
		} else {
			regs[15] |= 8;
		}
		regs[14] = regs[15];
#ifdef USE_MSM58321_OUTPUTS_DATA
		if(regnum == 14 || regnum == 15) {
			output_data();
		}
#endif
	}
}

void MSM58321::read_from_cur_time()
{
	// update clock
	cur_time.GetCurrTime();
	int sec = cur_time.GetSec();
	int min = cur_time.GetMin();
	int hour = cur_time.GetHour();
	int day = cur_time.GetDay();
	int month = cur_time.GetMonth();
	int year = cur_time.GetYear();
	int dow = cur_time.GetDayOfWeek();
	int ampm = (hour > 11) ? 4 : 0;

	hour = (regs[5] & 8) ? hour : (hour % 12);

	regs[ 0] = TO_BCD_LO(sec);
	regs[ 1] = TO_BCD_HI(sec);
	regs[ 2] = TO_BCD_LO(min);
	regs[ 3] = TO_BCD_HI(min);
	regs[ 4] = TO_BCD_LO(hour);
	regs[ 5] = TO_BCD_HI(hour) | ampm | (regs[5] & 8);
	regs[ 6] = dow;
	regs[ 7] = TO_BCD_LO(day - MSM58321_START_DAY);
	regs[ 8] = TO_BCD_HI(day - MSM58321_START_DAY) | (regs[8] & 0x0c);
	regs[ 9] = TO_BCD_LO(month);
	regs[10] = TO_BCD_HI(month);
	regs[11] = TO_BCD_LO(year - MSM58321_START_YEAR);
	regs[12] = TO_BCD_HI(year - MSM58321_START_YEAR);
}

void MSM58321::write_to_cur_time()
{
	cur_time.SetSec(regs[0] + (regs[1] & 7) * 10);
	cur_time.SetMin(regs[2] + (regs[3] & 7) * 10);
	int hour = regs[4] + (regs[5] & 3) * 10;
	if((regs[5] & 8) == 0 && (regs[5] & 4) != 0) {
		hour += 12;
	}
	cur_time.SetHour(hour);
//	cur_time.SetDayOfWeek(regs[6] & 7);
	cur_time.SetDay(regs[7] + (regs[8] & 3) * 10 + MSM58321_START_DAY);
	cur_time.SetMonth(regs[9] + (regs[10] & 1) * 10);
	cur_time.SetYear(regs[11] + regs[12] * 10 + MSM58321_START_YEAR);
	cur_time.CommitTime();

	// restart event
	if (register_id != -1) cancel_event(this, register_id);
#if defined(HAS_MSM5832) || !defined(USE_MSM58321_BUSY)
	register_event(this, EVENT_INC, 1000000.0, true, &register_id);
#else
	register_event(this, EVENT_BUSY, 1000000.0, true, &register_id);
#endif
}

void MSM58321::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_MSM58321_DATA) {
		wreg = (data & mask) | (wreg & ~mask);
	}
#if 0
	else if(id == SIG_MSM58321_CS) {
		bool next = ((data & mask) != 0);
//		if(!cs && next) {
//			if(wr) {
//				regs[regnum] = wreg & 0x0f;
//				if(regnum <= 12) {
//					write_to_cur_time();
//				}
//			}
//			if(addr_wr) {
//				regnum = wreg & 0x0f;
//			}
//		}
		cs = next;
		output_data();
	} else if(id == SIG_MSM58321_READ) {
		rd = ((data & mask) != 0);
		output_data();
	} else if(id == SIG_MSM58321_WRITE) {
		bool next = ((data & mask) != 0);
		if(!wr && next && cs) {
			regs[regnum] = wreg & 0x0f;
			if(regnum <= 12) {
				write_to_cur_time();
			}
		}
		wr = next;
	} else if(id == SIG_MSM58321_ADDR_WRITE) {
		bool next = ((data & mask) != 0);
		if(addr_wr && !next && cs) {
			regnum = wreg & 0x0f;
			output_data();
		}
		addr_wr = next;
	}
#else
	else if(id == SIG_MSM58321_READWRITE) {
		// bit7-4:select signals bit3-0:data
		// chip select
		cs = ((data & 0x10) != 0);
		// read select
		rd = ((data & 0x20) != 0);
		// write select
		wr = ((data & 0x40) != 0);
		// addr select
		addr_wr = ((data & 0x80) != 0);

		if (cs) {
			// top priority to write address
			if (addr_wr) {
				wreg = (data & (mask & 0xf)) | (wreg & ~(mask & 0xf));
				regnum = wreg & 0x0f;
			} else if (wr) {
				if(hold == 0 && regnum <= 12) {
					read_from_cur_time();
				}
				wreg = (data & (mask & 0xf)) | (wreg & ~(mask & 0xf));
				regs[regnum] = wreg & 0x0f;
				if(regnum <= 12) {
					write_to_cur_time();
				}
			}
		}
#ifdef USE_MSM58321_OUTPUTS_DATA
		output_data();
#endif
	}
#endif
#ifdef HAS_MSM5832
	else if(id == SIG_MSM5832_ADDR) {
		regnum = (data & mask) | (regnum & ~mask);
		output_data();
	}
	else if(id == SIG_MSM5832_HOLD) {
		uint8_t next = ((data & mask) != 0 ? hold | 0x01 : hold & ~0x01);
		if (hold == 0 && next != 0) {
			cur_time.StoreHoldTime();
		} else if (hold != 0 && next == 0) {
			cur_time.ReleaseHoldTime();
		}
		hold = next;
	}
#endif
}

uint32_t MSM58321::read_signal(int ch)
{
	if(ch == SIG_MSM58321_DATA) {
		if (!hold && regnum < 12) {
			read_from_cur_time();
		}
		return regs[regnum];
	}
	return 0;
}

#ifdef USE_MSM58321_OUTPUTS_DATA
void MSM58321::output_data()
{
	if(cs && rd) {
		write_signals(&outputs_data, regs[regnum]);
	}
}
#endif

void MSM58321::set_busy(bool val)
{
#if !defined(HAS_MSM5832) && defined(USE_MSM58321_BUSY)
	if(busy != val) {
		write_signals(&outputs_busy, busy ? 0 : 0xffffffff);	// negative
	}
#endif
	busy = val;
}

void MSM58321::pause(int value)
{
	uint8_t next = (value ? hold | 0x02 : hold & ~0x02);
	if (hold == 0 && next != 0) {
		// hold start
		cur_time.StoreHoldTime();
	} else if (hold != 0 && next == 0) {
		// hold end
		cur_time.ReleaseHoldTime();
	}
	hold = next;
}

#define STATE_VERSION	1

void MSM58321::save_state(FILEIO* fio)
{
	struct vm_state_st vm_state;
	size_t state_size = 0;

	//
	vm_state_ident.version = Uint16_LE(STATE_VERSION);

	// copy values
	memset(&vm_state, 0, sizeof(vm_state));

	// reserved header
	fio->Fwrite(&vm_state_ident, sizeof(vm_state_ident), 1);

	vm_state.register_id = Int32_LE(register_id);
	for(int i=0; i<16; i++) {
		vm_state.regs[i] = Int32_LE(regs[i]);
	}
	vm_state.wreg = wreg;
	vm_state.regnum = regnum;
	vm_state.sigs = ((cs ? 1 : 0) | (rd ? 2 : 0) | (wr ? 4 : 0) | (addr_wr ? 8 : 0) | (busy ? 16 : 0));
	vm_state.hold = hold;
	vm_state.count_1024hz = Int32_LE(count_1024hz);
	vm_state.count_1s = Int32_LE(count_1s);
	vm_state.count_1m = Int32_LE(count_1m);
	vm_state.count_1h = Int32_LE(count_1h);

	cur_time.SaveState(fio, &state_size);

	fio->Fwrite(&vm_state, sizeof(vm_state), 1);

	// set total size
	state_size += sizeof(vm_state);
	vm_state_ident.size = Uint32_LE((uint32_t)(sizeof(vm_state_ident) + state_size));

	// overwrite header
	fio->Fseek(-(long)(state_size + sizeof(vm_state_ident)), FILEIO::SEEKCUR);
	fio->Fwrite(&vm_state_ident, sizeof(vm_state_ident), 1);
	fio->Fseek((long)state_size, FILEIO::SEEKCUR);
}

bool MSM58321::load_state(FILEIO* fio)
{
	vm_state_ident_t vm_state_i;
	struct vm_state_st vm_state;

	if (find_state_chunk(fio, &vm_state_i) != true) {
		return true;
	}

	if(Uint16_LE(vm_state_i.version) != STATE_VERSION) {
		return true;
	}

	if (!cur_time.LoadState(fio)) {
		return true;
	}

	fio->Fread(&vm_state, sizeof(vm_state), 1);
	register_id = Int32_LE(vm_state.register_id);
	for(int i=0; i<16; i++) {
		regs[i] = Int32_LE(vm_state.regs[i]);
	}
	wreg = vm_state.wreg;
	regnum = vm_state.regnum;
	cs = ((vm_state.sigs & 1) != 0);
	rd = ((vm_state.sigs & 2) != 0);
	wr = ((vm_state.sigs & 4) != 0);
	addr_wr = ((vm_state.sigs & 8) != 0);
	busy = ((vm_state.sigs & 16) != 0);
	hold = vm_state.hold;
	count_1024hz = Int32_LE(vm_state.count_1024hz);
	count_1s = Int32_LE(vm_state.count_1s);
	count_1m = Int32_LE(vm_state.count_1m);
	count_1h = Int32_LE(vm_state.count_1s);

	return true;
}

// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
bool MSM58321::debug_write_reg(uint32_t reg_num, uint32_t data)
{
	if (reg_num < 16) {
		if(reg_num <= 12) {
			read_from_cur_time();
		}
		regs[reg_num] = data & 0x0f;
		if(reg_num <= 12) {
			write_to_cur_time();
		}
		return true;
	}
	return false;
}

static const _TCHAR *c_reg_names[] = {
	_T("SEC01"),
	_T("SEC10"),
	_T("MIN01"),
	_T("MIN10"),
	_T("HOU01"),
	_T("HOU10"),
	_T("WEEK"),
	_T("DAY01"),
	_T("DAY10"),
	_T("MON01"),
	_T("MON10"),
	_T("YEA01"),
	_T("YEA10"),
	_T("D"),
	_T("E"),
	_T("F"),
	NULL
};

bool MSM58321::debug_write_reg(const _TCHAR *reg, uint32_t data)
{
	uint32_t num = find_debug_reg_name(c_reg_names, reg);
	return debug_write_reg(num, data);
}

void MSM58321::debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	buffer[0] = _T('\0');

	for(uint32_t i=0; i<16; i++) {
		UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%-5s):%X"), i, c_reg_names[i], regs[i]);
		if ((i % 7) == 6) UTILITY::tcscat(buffer, buffer_len, _T("\n"));
	}
}
#endif
