/** @file rtc.cpp

	HITACHI BASIC MASTER LEVEL3 Mark5 / MB-S1 Emulator 'EmuB-6892/EmuB-S1'
	Skelton for retropc emulator

	@author Sasaji
	@date   2018.01.01 -

	@brief [ real time clock control ]
*/

#include "rtc.h"
//#include "../../emu.h"
#include "../msm58321.h"
#include "../../fileio.h"
#include "../../utility.h"

#define SEL_CS ((rtc_data & 0x40) != 0)

#define SEL_WRITE ((rtc_reg_sel & 0xc0) == 0x40)
#define SEL_READ ((rtc_reg_sel & 0xc0) == 0x80)
#define SEL_ADDR ((rtc_reg_sel & 0xc0) == 0)

#define SEL_WRITEABLE	((rtc_data & 0xc0) == 0x40)
#define SEL_READABLE	((rtc_data & 0xc0) == 0xc0)

#define NOW_WRITEABLE	(SEL_WRITE && SEL_WRITEABLE)
#define NOW_READABLE	(SEL_READ && SEL_READABLE)

void RTC::initialize()
{
	rtc_reg_sel = 0;
	rtc_data = 0;
}

void RTC::reset()
{
	rtc_reg_sel = 0;
	rtc_data = 0;
}

void RTC::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 3) {
	case 0:
		// rtc register select
		rtc_reg_sel = data & 0xcf;
		// write address to rtc
		d_rtc->write_signal(MSM58321::SIG_MSM58321_READWRITE
			, (SEL_CS ? 0x10 : 0) | (NOW_WRITEABLE ? 0x40 : 0) | (SEL_ADDR ? 0x80 : 0)
			| ((SEL_ADDR ? rtc_reg_sel : rtc_data) & 0xf)
			, 0xff);
		break;
	case 1:
		// rtc write
		rtc_data = data & 0xcf;
		// write data to rtc
		d_rtc->write_signal(MSM58321::SIG_MSM58321_READWRITE
			, (SEL_CS ? 0x10 : 0) | (NOW_READABLE ? 0x20 : 0) | (NOW_WRITEABLE ? 0x40 : 0) | (data & 0xf)
			, 0xff);
		break;
	}
}

uint32_t RTC::read_io8(uint32_t addr)
{
	uint32_t data = 0xff;

	switch(addr & 3) {
	case 2:
		// rtc read
		if (NOW_READABLE) {
			data = (d_rtc->read_signal(MSM58321::SIG_MSM58321_DATA) | 0xf0);
		} else if (SEL_CS && SEL_ADDR) {
			data = ((rtc_reg_sel & 0x0f) | 0xf0);
		} else {
			data = ((rtc_data & 0x0f) | 0xf0);
		}
		break;
	}
	return data;
}

void RTC::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch(id) {
		case SIG_CPU_RESET:
			now_reset = (data & mask) ? true : false;
			if (now_reset) {
				reset();
			}
			break;
	}
}

// ----------------------------------------------------------------------------

#define STATE_VERSION	1

void RTC::save_state(FILEIO *fio)
{
	struct vm_state_st vm_state;

	//
	vm_state_ident.version = Uint16_LE(1);
	vm_state_ident.size = Uint32_LE(sizeof(vm_state_ident) + sizeof(vm_state));

	// copy values
	memset(&vm_state, 0, sizeof(vm_state));

	vm_state.rtc_reg_sel = rtc_reg_sel;
	vm_state.rtc_data = rtc_data;

	fio->Fwrite(&vm_state_ident, sizeof(vm_state_ident), 1);
	fio->Fwrite(&vm_state, sizeof(vm_state), 1);
}

bool RTC::load_state(FILEIO *fio)
{
	vm_state_ident_t vm_state_i;
	struct vm_state_st vm_state;

	READ_STATE_CHUNK(fio, vm_state_i, vm_state);

	rtc_reg_sel = vm_state.rtc_reg_sel;
	rtc_data = vm_state.rtc_data;

	return true;
}

// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
uint32_t RTC::debug_read_io8(uint32_t addr)
{
	return read_io8(addr);
}

bool RTC::debug_write_reg(uint32_t reg_num, uint32_t data)
{
	switch(reg_num) {
	case 0:
	case 1:
		write_io8(reg_num, data);
		break;
	default:
		return false;
	}
	return true;
}

bool RTC::debug_write_reg(const _TCHAR *reg, uint32_t data)
{
	return false;
}

void RTC::debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	buffer[0] = _T('\0');

	UTILITY::sntprintf(buffer, buffer_len, _T(" 0(FF38):%02X 1(FF39):%02X"), rtc_reg_sel, rtc_data);
}
#endif
