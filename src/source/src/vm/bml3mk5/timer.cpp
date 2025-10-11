/** @file timer.cpp

	HITACHI BASIC MASTER LEVEL3 Mark5 Emulator 'EmuB-6892'
	Skelton for retropc emulator

	@author Sasaji
	@date   2011.06.08 -

	@brief [ timer ]
*/

#include "timer.h"
//#include "../../emu.h"
#include "registers.h"
#include "../../fileio.h"
#include "../../utility.h"

void TIMER::initialize()
{
	timer_irq = 0;
	REG_TIME_MASK = 0;
}

void TIMER::reset()
{
	timer_irq = 0;
	REG_TIME_MASK = 0;
}

void TIMER::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch (id) {
		case SIG_TIMER_VSYNC:
			if (data & mask) update_timer_clock();
			break;
		case SIG_CPU_RESET:
			now_reset = (data & mask) ? true : false;
			break;
	}
}

void TIMER::write_io8(uint32_t addr, uint32_t data)
{
	switch (addr & 0xffff) {
		case ADDR_TIMER_IRQ:
			break;
		case ADDR_TIME_MASK:
			REG_TIME_MASK = data & 0x80;
			if (REG_TIME_MASK & 0x80) {
				timer_irq = 0;
				// Clear FIRQ
				d_board->write_signal(SIG_CPU_FIRQ, 0, SIG_FIRQ_TIMER1_MASK);
			}
			break;
	}
}

uint32_t TIMER::read_io8(uint32_t addr)
{
	uint32_t data = 0;
	switch (addr & 0xffff) {
		case ADDR_TIMER_IRQ:
			data = timer_irq;
			timer_irq = 0;
			// Clear FIRQ
			d_board->write_signal(SIG_CPU_FIRQ, 0, SIG_FIRQ_TIMER1_MASK);
//			logging->out_debugf("tq %d irq:%d firq:%d", data, counter_irq, counter_firq);
			break;
		case ADDR_TIME_MASK:
			break;
	}
	return data;
}

void TIMER::update_timer_clock()
{
	if (!(REG_TIME_MASK & 0x80) && !now_reset) {
		timer_irq = 0x80;		// timer irq set
		// FIRQ interrupt
		d_board->write_signal(SIG_CPU_FIRQ, SIG_FIRQ_TIMER1_MASK, SIG_FIRQ_TIMER1_MASK);
	}
}

// ----------------------------------------------------------------------------
// event handler
// ----------------------------------------------------------------------------

void TIMER::event_frame()
{
}

void TIMER::event_callback(int event_id, int err)
{
}

// ----------------------------------------------------------------------------

void TIMER::save_state(FILEIO *fio)
{
	struct vm_state_st vm_state;

	//
	vm_state_ident.version = Uint16_LE(1);
	vm_state_ident.size = Uint32_LE(sizeof(vm_state_ident) + sizeof(vm_state));

	// copy values
	memset(&vm_state, 0, sizeof(vm_state));
	vm_state.timer_irq = timer_irq;
	vm_state.time_mask = REG_TIME_MASK;

	fio->Fwrite(&vm_state_ident, sizeof(vm_state_ident), 1);
	fio->Fwrite(&vm_state, sizeof(vm_state), 1);
}

bool TIMER::load_state(FILEIO *fio)
{
	vm_state_ident_t vm_state_i;
	struct vm_state_st vm_state;

	READ_STATE_CHUNK(fio, vm_state_i, vm_state);

	timer_irq = vm_state.timer_irq;
	REG_TIME_MASK = vm_state.time_mask;

	return true;
}

// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
uint32_t TIMER::debug_read_io8(uint32_t addr)
{
	uint32_t data = 0;
	switch (addr & 0xffff) {
		case ADDR_TIMER_IRQ:
			data = timer_irq;
			break;
		case ADDR_TIME_MASK:
			break;
	}
	return data;
}

bool TIMER::debug_write_reg(uint32_t reg_num, uint32_t data)
{
	switch(reg_num) {
	case 0:
		timer_irq = (data & 0xc0);
		// FIRQ interrupt
		d_board->write_signal(SIG_CPU_FIRQ, (timer_irq & 0x80) && (REG_TIME_MASK & 0x80) == 0 ? SIG_FIRQ_TIMER1_MASK : 0, SIG_FIRQ_TIMER1_MASK);
		return true;
	case 1:
		write_io8(ADDR_TIME_MASK, data);
		return true;
	}
	return false;
}

static const _TCHAR *c_reg_names[] = {
	_T("TIMER"),
	_T("TIME_MASK"),
	NULL
};

bool TIMER::debug_write_reg(const _TCHAR *reg, uint32_t data)
{
	uint32_t num = find_debug_reg_name(c_reg_names, reg);
	return debug_write_reg(num, data);
}

void TIMER::debug_regs_info(const _TCHAR *title, _TCHAR *buffer, size_t buffer_len)
{
	UTILITY::tcscpy(buffer, buffer_len, title);
	UTILITY::tcscat(buffer, buffer_len, _T(" Registers:\n"));
	UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%s):%02X"), 0, c_reg_names[0], timer_irq);
	UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%s):%02X"), 1, c_reg_names[1], REG_TIME_MASK);
}
#endif

