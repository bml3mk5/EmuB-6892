/** @file board.cpp

	HITACHI BASIC MASTER LEVEL3 Mark5 Emulator 'EmuB-6892'
	Skelton for retropc emulator

	@author Sasaji
	@date   2012.06.21 -

	@brief [ main board ]
*/

#include "board.h"
//#include "../../emu.h"
#include "../vm.h"
#include "../../config.h"
#include "../../fileio.h"
#include "../../utility.h"

void BOARD::initialize()
{
	wreset_register_id = -1;
	preset_register_id = -1;
}

void BOARD::reset()
{
	// clear all signals
	now_halt = 1; write_signal(SIG_CPU_HALT, 0, 1);
	now_nmi  = 1; write_signal(SIG_CPU_NMI,  0, 1);
	now_irq  = 1; write_signal(SIG_CPU_IRQ,  0, 1);
	now_firq = 1; write_signal(SIG_CPU_FIRQ, 0, 1);
	now_wreset = 0;
	cancel_my_event(wreset_register_id);
	cancel_my_event(preset_register_id);
	d_cpu->write_signal(SIG_CPU_RESET, 1, 1);
	register_event(this, SIG_BOARD_PRESET_RELEASE, 40000, false, &preset_register_id);
}

void BOARD::cancel_my_event(int &id)
{
	if (id >= 0) cancel_event(this, id);
	id = -1;
}

void BOARD::write_signal(int id, uint32_t data, uint32_t mask)
{
	uint16_t prev = 0;
	switch (id) {
		case SIG_CPU_RESET:
			prev = now_wreset;
			now_wreset = ((data & mask) ? (now_wreset | mask) : (now_wreset & ~mask));
			if (prev == 0 && now_wreset != 0) {
				write_signals(&outputs_reset, 0xffffffff);
			} else if (prev != 0 && now_wreset == 0) {
//				write_signals(&outputs_reset, 0);
				// release RESET after 0.01 sec
				cancel_my_event(wreset_register_id);
				cancel_my_event(preset_register_id);
				register_event(this, SIG_BOARD_WRESET_RELEASE, 10000, false, &wreset_register_id);
			}
//			logging->out_debugf(_T("BOARD: RESET: %X %X"), data, mask);
			break;
		case SIG_CPU_NMI:
			prev = now_nmi;
			now_nmi = ((data & mask) ? (now_nmi | mask) : (now_nmi & ~mask));
			if (prev == 0 && now_nmi != 0) {
				write_signals(&outputs_nmi, now_nmi);
			} else if (prev != 0 && now_nmi == 0) {
				write_signals(&outputs_nmi, now_nmi);
			}
//			logging->out_debugf(_T("BOARD: NMI: %X %X"), data, mask);
			break;
		case SIG_CPU_IRQ:
			prev = now_irq;
			now_irq = ((data & mask) ? (now_irq | mask) : (now_irq & ~mask));
			if (prev == 0 && now_irq != 0) {
				write_signals(&outputs_irq, now_irq);
			} else if (prev != 0 && now_irq == 0) {
				write_signals(&outputs_irq, now_irq);
			}
//			logging->out_debugf(_T("BOARD: IRQ NOW:%X (%X:%X)"), now_irq, data, mask);
			break;
		case SIG_CPU_FIRQ:
			prev = now_firq;
			now_firq = ((data & mask) ? (now_firq | mask) : (now_firq & ~mask));
			if (prev == 0 && now_firq != 0) {
				write_signals(&outputs_firq, now_firq);
			} else if (prev != 0 && now_firq == 0) {
				write_signals(&outputs_firq, now_firq);
			}
//			logging->out_debugf(_T("BOARD: FIRQ NOW:%X (%X:%X)"), now_firq, data, mask);
			break;
		case SIG_CPU_HALT:
			prev = now_halt;
			now_halt = ((data & mask) ? (now_halt | mask) : (now_halt & ~mask));
			if (prev == 0 && now_halt != 0) {
				write_signals(&outputs_halt, now_halt);
			} else if (prev != 0 && now_halt == 0) {
				write_signals(&outputs_halt, now_halt);
			}
//			logging->out_debugf(_T("BOARD: HALT: %X %X"), data, mask);
			break;
	}
}

uint32_t BOARD::read_signal(int id)
{
	uint32_t data = 0xffffffff;
	switch (id) {
		case SIG_CPU_RESET:
			data = now_wreset;
			break;
		case SIG_CPU_NMI:
			data = now_nmi;
			break;
		case SIG_CPU_IRQ:
			data = now_irq;
			break;
		case SIG_CPU_FIRQ:
			data = now_firq;
			break;
		case SIG_CPU_HALT:
			data = now_halt;
			break;
	}
	return data;
}

void BOARD::event_callback(int event_id, int err)
{
	if (event_id == SIG_BOARD_WRESET_RELEASE) {
		write_signals(&outputs_reset, 0);
		wreset_register_id = -1;
	}
	else if (event_id == SIG_BOARD_PRESET_RELEASE) {
		d_cpu->write_signal(SIG_CPU_RESET, 0, 1);
		preset_register_id = -1;
	}
}

uint32_t BOARD::update_led()
{
	return now_wreset ? 0x80 : 0;
}

// ----------------------------------------------------------------------------

void BOARD::save_state(FILEIO *fio)
{
	struct vm_state_st vm_state;

	//
	vm_state_ident.version = Uint16_LE(3);
	vm_state_ident.size = Uint32_LE(sizeof(vm_state_ident) + sizeof(vm_state));

	// save config flags
	memset(&vm_state, 0, sizeof(vm_state));
	vm_state.fdd_type = Int32_LE(vm->get_parami(VM::ParamFddType));
	vm_state.io_port = Int32_LE(vm->get_parami(VM::ParamIOPort));
	vm_state.flags = ((config.sync_irq ? 1 : 0) | (config.realmode_datarec ? 2 : 0)
		| (config.use_power_off ? 4 : 0) | (config.now_power_off ? 8 : 0));

	vm_state.now_halt = Uint16_LE(now_halt);
	vm_state.now_nmi  = Uint16_LE(now_nmi);
	vm_state.now_irq  = Uint16_LE(now_irq);
	vm_state.now_firq = Uint16_LE(now_firq);
	vm_state.now_wreset = Uint16_LE(now_wreset);
	vm_state.wreset_register_id = Int32_LE(wreset_register_id);
	vm_state.preset_register_id = Int32_LE(preset_register_id);

	fio->Fwrite(&vm_state_ident, sizeof(vm_state_ident), 1);
	fio->Fwrite(&vm_state, sizeof(vm_state), 1);
}

bool BOARD::load_state(FILEIO *fio)
{
	vm_state_ident_t vm_state_i;
	struct vm_state_st vm_state;

	READ_STATE_CHUNK(fio, vm_state_i, vm_state);

	// load config flags
	vm->set_parami(VM::ParamFddType, Int32_LE(vm_state.fdd_type));
	vm->set_parami(VM::ParamIOPort, Int32_LE(vm_state.io_port));
	config.sync_irq = ((vm_state.flags & 1) ? true : false);
	config.realmode_datarec = ((vm_state.flags & 2) ? true : false);
	config.use_power_off = ((vm_state.flags & 4) ? true : false);
	config.now_power_off = ((vm_state.flags & 8) ? true : false);

	if (Uint16_LE(vm_state_i.version) >= 2) {
		now_halt = Uint16_LE(vm_state.now_halt);
		now_nmi  = Uint16_LE(vm_state.now_nmi);
		now_irq  = Uint16_LE(vm_state.now_irq);
		now_firq = Uint16_LE(vm_state.now_firq);
		now_wreset = Uint16_LE(vm_state.now_wreset);
	}
	if (Uint16_LE(vm_state_i.version) >= 3) {
		wreset_register_id = Int32_LE(vm_state.wreset_register_id);
		preset_register_id = Int32_LE(vm_state.preset_register_id);
	} else {
		wreset_register_id = -1;
		preset_register_id = -1;
	}

	vm->set_pause(3, config.now_power_off);

	return true;
}

// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
bool BOARD::debug_write_reg(uint32_t reg_num, uint32_t data)
{
	uint32_t prev = 0;
	switch(reg_num) {
		case 0:
			prev = now_wreset;
			now_wreset = data;
			if (prev == 0 && now_wreset != 0) {
				write_signals(&outputs_reset, 0xffffffff);
			} else if (prev != 0 && now_wreset == 0) {
				// release RESET after 0.01 sec
				cancel_my_event(wreset_register_id);
				cancel_my_event(preset_register_id);
				register_event(this, SIG_BOARD_WRESET_RELEASE, 10000, false, &wreset_register_id);
			}
			return true;
		case 1:
			prev = now_nmi;
			now_nmi = data;
			if (prev == 0 && now_nmi != 0) {
				write_signals(&outputs_nmi, now_nmi);
			} else if (prev != 0 && now_nmi == 0) {
				write_signals(&outputs_nmi, now_nmi);
			}
			return true;
		case 2:
			prev = now_irq;
			now_irq = data;
			if (prev == 0 && now_irq != 0) {
				write_signals(&outputs_irq, now_irq);
			} else if (prev != 0 && now_irq == 0) {
				write_signals(&outputs_irq, now_irq);
			}
			return true;
		case 3:
			prev = now_firq;
			now_firq = data;
			if (prev == 0 && now_firq != 0) {
				write_signals(&outputs_firq, now_firq);
			} else if (prev != 0 && now_firq == 0) {
				write_signals(&outputs_firq, now_firq);
			}
			return true;
		case 4:
			prev = now_halt;
			now_halt = data;
			if (prev == 0 && now_halt != 0) {
				write_signals(&outputs_halt, now_halt);
			} else if (prev != 0 && now_halt == 0) {
				write_signals(&outputs_halt, now_halt);
			}
			return true;
	}
	return false;
}

bool BOARD::debug_write_reg(const _TCHAR *reg, uint32_t data)
{
	return false;
}

void BOARD::debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	buffer[0] = _T('\0');
	UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%s):%04X\n"), 0, _T("RES "), now_wreset);
	UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%s):%04X\n"), 1, _T("NMI "), now_nmi);
	UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%s):%04X\n"), 2, _T("IRQ "), now_irq);
	UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%s):%04X\n"), 3, _T("FIRQ"), now_firq);
	UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%s):%04X\n"), 4, _T("HALT"), now_halt);
}
#endif