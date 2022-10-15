/** @file ym2203.cpp

	Skelton for retropc emulator

	@author Takeda.Toshiya
	@date   2006.09.15-

	@note
	Modified by Sasaji at 2012.06.20
	@note
	AY-3-8910 functions has gone to AY38910 class

	@brief [ YM2203 / YM2608 ]
*/

#include "ym2203.h"
#include "../emu.h"
#include "../fileio.h"
#include "../utility.h"
#include "../config.h"

/// @brief class names of YM2203
static const char *YM2203_CLASS_NAMES[] = {
	"AY38910", "YM2203", "YM2608", NULL
};
/// @brief class name position of YM2203
enum YM2203_CLASS_NAMES_POS {
	CLASS_NAMES_POS_AY38910 = 0,
	CLASS_NAMES_POS_YM2203,
	CLASS_NAMES_POS_YM2608
};

YM2203::YM2203(VM* parent_vm, EMU* parent_emu, const char* identifier)
	: DEVICE(parent_vm, parent_emu, identifier)
{
	set_class_name(YM2203_CLASS_NAMES[CLASS_NAMES_POS_AY38910]);

	for(int i = 0; i < 2; i++) {
		init_output_signals(&port[i].outputs);
		port[i].wreg = port[i].rreg = 0;//0xff;
	}

	init_output_signals(&outputs_irq);

#ifdef HAS_YM2608
	is_ym2608 = false;
	opna = NULL;
#endif
	opn = NULL;

	base_decibel_fm = base_decibel_psg = 0;
}

void YM2203::initialize()
{
	register_vline_event(this);
	mute = false;
	clock_prev = clock_accum = clock_busy = 0;
}

void YM2203::set_chiptype(bool ym2608)
{
#ifdef HAS_YM2608
	if (is_ym2608 != ym2608) {
		delete opna;
		delete opn;
		opna = NULL;
		opn = NULL;
	}
#endif

#ifdef HAS_YM2608
	is_ym2608 = ym2608;
	set_vm_state_class_name(is_ym2608 ? YM2203_CLASS_NAMES[CLASS_NAMES_POS_YM2608] : YM2203_CLASS_NAMES[CLASS_NAMES_POS_YM2203]);
#else
	set_vm_state_class_name(YM2203_CLASS_NAMES[CLASS_NAMES_POS_YM2203]);
#endif

#ifdef HAS_YM2608
	if(is_ym2608) {
		if (!opna) opna = new FM::OPNA;
	} else
#endif
	{
		if (!opn) opn = new FM::OPN;
	}
}

void YM2203::release()
{
#ifdef HAS_YM2608
	delete opna;
#endif
	delete opn;
}

void YM2203::reset()
{
#ifdef HAS_YM2608
	if(is_ym2608) {
		opna->Reset();
	} else
#endif
	{
		opn->Reset();
	}
	fnum2 = 0;
#ifdef HAS_YM2608
	fnum21 = 0;
#endif
#ifdef USE_DEBUGGER
	memset(debug_regs, 0, sizeof(debug_regs));
#endif

	// stop timer
	timer_event_id = -1;
	this->set_reg(0x27, 0);

	port[0].first = port[1].first = true;
	port[0].wreg = port[1].wreg = 0;//0xff;
#ifdef YM2203_PORT_MODE
	mode = YM2203_PORT_MODE;
#else
	mode = 0;
#endif
	irq_prev = busy = false;
}

#ifdef HAS_YM2608
#define amask (is_ym2608 ? 3 : 1)
#else
#define amask 1
#endif

void YM2203::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & amask) {
	case 0:
		ch = data;
		// write dummy data for prescaler
		if(0x2d <= ch && ch <= 0x2f) {
			update_count();
			this->set_reg(ch, 0);
			update_interrupt();
			clock_busy = get_current_clock();
			busy = true;
		}
		break;
	case 1:
		if(ch == 7) {
#ifdef YM2203_PORT_MODE
			mode = (data & 0x3f) | YM2203_PORT_MODE;
#else
			mode = data;
#endif
		}
		if (ch == 14) {
			if(port[0].wreg != data || port[0].first) {
				write_signals(&port[0].outputs, data);
				port[0].wreg = data;
				port[0].first = false;
			}
		} else if(ch == 15) {
			if(port[1].wreg != data || port[1].first) {
				write_signals(&port[1].outputs, data);
				port[1].wreg = data;
				port[1].first = false;
			}
		} else if(0x2d <= ch && ch <= 0x2f) {
			// don't write again for prescaler
		} else if(0xa4 <= ch && ch <= 0xa6) {
			// XM8 version 1.20
			fnum2 = data;
		} else {
			update_count();
			// XM8 version 1.20
			if(0xa0 <= ch && ch <= 0xa2) {
				this->set_reg(ch + 4, fnum2);
			}
			this->set_reg(ch, data);
			if(ch == 0x27) {
				update_event();
			}
			update_interrupt();
			clock_busy = get_current_clock();
			busy = true;
		}
		break;
#ifdef HAS_YM2608
	case 2:
		ch1 = data1 = data;
		break;
	case 3:
		if(0xa4 <= ch1 && ch1 <= 0xa6) {
			// XM8 version 1.20
			fnum21 = data;
		} else {
			update_count();
			// XM8 version 1.20
			if(0xa0 <= ch1 && ch1 <= 0xa2) {
				this->set_reg(0x100 | (ch1 + 4), fnum21);
			}
			this->set_reg(0x100 | ch1, data);
			data1 = data;
			update_interrupt();
			clock_busy = get_current_clock();
			busy = true;
		}
		break;
#endif
	}
}

uint32_t YM2203::read_io8(uint32_t addr)
{
	switch(addr & amask) {
	case 0:
		{
			/* BUSY : x : x : x : x : x : FLAGB : FLAGA */
			update_count();
			update_interrupt();
			return read_status();
		}
	case 1:
		if(ch == 14) {
			return (mode & 0x40) ? port[0].wreg : port[0].rreg;
		} else if(ch == 15) {
			return (mode & 0x80) ? port[1].wreg : port[1].rreg;
		}
#ifdef HAS_YM2608
		if(is_ym2608) {
			return opna->GetReg(ch);
		} else
#endif
		return opn->GetReg(ch);
#ifdef HAS_YM2608
	case 2:
		{
			/* BUSY : x : PCMBUSY : ZERO : BRDY : EOS : FLAGB : FLAGA */
			update_count();
			update_interrupt();
			uint32_t status = opna->ReadStatusEx() & ~0x80;
			if(busy) {
				// FIXME: we need to investigate the correct busy period
				if(get_passed_usec(clock_busy) < 8) {
					status |= 0x80;
				} else {
					busy = false;
				}
			}
			return status;
		}
	case 3:
		if(ch1 == 8) {
			return opna->GetReg(0x100 | ch1);
//		} else if(ch1 == 0x0f) {
//			return 0x80; // from mame fm.c
		}
		return data1;
#endif
	}
	return 0xff;
}

void YM2203::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_YM2203_MUTE) {
		mute = ((data & mask) != 0);
	} else if(id == SIG_YM2203_PORT_A) {
		port[0].rreg = (port[0].rreg & ~mask) | (data & mask);
	} else if(id == SIG_YM2203_PORT_B) {
		port[1].rreg = (port[1].rreg & ~mask) | (data & mask);
	} else if (id == SIG_CPU_RESET) {
		now_reset = (data & mask) ? true : false;
		reset();
	}
}

void YM2203::event_vline(int v, int clock)
{
	update_count();
	update_interrupt();
}

void YM2203::event_callback(int event_id, int error)
{
	update_count();
	update_interrupt();
	timer_event_id = -1;
	update_event();
}

void YM2203::update_count()
{
	clock_accum += clock_const * get_passed_clock(clock_prev);
	uint32_t count = (uint32_t)(clock_accum >> 20);
	if(count) {
#ifdef HAS_YM2608
		if(is_ym2608) {
			opna->Count(count);
		} else
#endif
		{
			opn->Count(count);
		}
		clock_accum -= count << 20;
	}
	clock_prev = get_current_clock();
}

void YM2203::update_event()
{
	if(timer_event_id != -1) {
		cancel_event(this, timer_event_id);
		timer_event_id = -1;
	}

	int count;
#ifdef HAS_YM2608
	if(is_ym2608) {
		count = opna->GetNextEvent();
	} else
#endif
	count = opn->GetNextEvent();

	if(count > 0) {
#ifdef HAS_YM2608
		if(is_ym2608) {
			register_event(this, EVENT_FM_TIMER, 1000000.0 / (double)chip_clock * (double)count * 2.0, false, &timer_event_id);
		} else
#endif
		register_event(this, EVENT_FM_TIMER, 1000000.0 / (double)chip_clock * (double)count, false, &timer_event_id);
	}
}

#ifdef HAS_YM_SERIES
void YM2203::update_interrupt()
{
	bool irq;
#ifdef HAS_YM2608
	if(is_ym2608) {
		irq = opna->ReadIRQ();
	} else
#endif
	irq = opn->ReadIRQ();
	if(!irq_prev && irq) {
		write_signals(&outputs_irq, 0xffffffff);
	} else if(irq_prev && !irq) {
		write_signals(&outputs_irq, 0);
	}
	irq_prev = irq;
}
#endif

void YM2203::mix(int32_t* buffer, int cnt)
{
#if 0 // def USE_EMU_INHERENT_SPEC
	memset(buffer_tmp, 0, sizeof(buffer_tmp));
	if (cnt > 20) cnt = 20;
	if(cnt > 0 && !mute) {
#ifdef HAS_YM2608
		if(is_ym2608) {
			opna->Mix(buffer_tmp, cnt);
		} else
#endif
		opn->Mix(buffer_tmp, cnt);
	}
	for(int i=0; i<(cnt * 2); i++) {
		*buffer++ += (buffer_tmp[i] * sound_volume / 100);
	}
#else
	if(cnt > 0 && !mute) {
#ifdef HAS_YM2608
		if(is_ym2608) {
			opna->Mix(buffer, cnt);
		} else
#endif
		opn->Mix(buffer, cnt);
	}
#endif
}

void YM2203::set_volume(int ch, int decibel_l, int decibel_r, bool vol_mute, int pattern)
{
	if (vol_mute) {
		decibel_l = -192;
		decibel_r = -192;
	}
	if(ch == 0) {
#ifdef HAS_YM2608
		if(is_ym2608) {
			opna->SetVolumeFM(base_decibel_fm + decibel_l, base_decibel_fm + decibel_r);
		} else
#endif
		opn->SetVolumeFM(base_decibel_fm + decibel_l, base_decibel_fm + decibel_r);
	} else if(ch == 1) {
#ifdef HAS_YM2608
		if(is_ym2608) {
			opna->SetVolumePSG(base_decibel_psg + decibel_l, base_decibel_psg + decibel_r, pattern);
		} else
#endif
		opn->SetVolumePSG(base_decibel_psg + decibel_l, base_decibel_psg + decibel_r, pattern);
#ifdef HAS_YM2608
	} else if(ch == 2) {
		if(is_ym2608) {
			opna->SetVolumeADPCM(decibel_l, decibel_r);
		}
	} else if(ch == 3) {
		if(is_ym2608) {
			opna->SetVolumeRhythmTotal(decibel_l, decibel_r);
		}
#endif
	}
}

void YM2203::initialize_sound(int rate, int clock, int samples, int decibel_fm, int decibel_psg, int ptn)
{
#ifdef HAS_YM2608
	if(is_ym2608) {
		const _TCHAR *paths[] = {
			config.rom_path.Get(), emu->application_path(), NULL
		};
		opna->Init(clock, rate, false, paths);
		opna->SetVolumeFM(decibel_fm, decibel_fm);
		opna->SetVolumePSG(decibel_psg, decibel_psg, ptn);
	} else {
#endif
		opn->Init(clock, rate, false, NULL);
		opn->SetVolumeFM(decibel_fm, decibel_fm);
		opn->SetVolumePSG(decibel_psg, decibel_psg, ptn);
#ifdef HAS_YM2608
	}
#endif
#ifdef USE_EMU_INHERENT_SPEC
//	sound_volume = 0;
#endif
	chip_clock = clock;
}

void YM2203::set_reg(uint32_t addr, uint32_t data)
{
#ifdef HAS_YM2608
	if(is_ym2608) {
		opna->SetReg(addr, data);
	} else
#endif
	{
		opn->SetReg(addr, data);
	}

#ifdef USE_DEBUGGER
	uint32_t d_addr = (addr & 0x1ff);
	if (d_addr >= 0x2d && d_addr <= 0x2e) {
		debug_regs[0x2f] = 0;
		debug_regs[d_addr] = 1;
	} else if (d_addr == 0x2f) {
		debug_regs[0x2d] = debug_regs[0x2e] = 0;
		debug_regs[d_addr] = 1;
	} else if (d_addr >= 0x20) {
		debug_regs[d_addr] = (data & 0xff);
	}
#endif
}

uint32_t YM2203::read_status()
{
	uint32_t status;

	/* BUSY : x : x : x : x : x : FLAGB : FLAGA */
#ifdef HAS_YM2608
	if(is_ym2608) {
		status = opna->ReadStatus() & ~0x80;
	} else
#endif
	{
		status = opn->ReadStatus() & ~0x80;
	}
	if(busy) {
		// from PC-88 machine language master bible (XM8 version 1.00)
#ifdef HAS_YM2608
		if (get_passed_usec(clock_busy) < (is_ym2608 ? 4.25 : 2.13)) {
#else
		if (get_passed_usec(clock_busy) < 2.13) {
#endif
			status |= 0x80;
		} else {
			busy = false;
		}
	}
	return status;
}

#ifdef HAS_YM2608
uint32_t YM2203::read_status_ex()
{
	uint32_t status;

	/* BUSY : x : PCMBUSY : ZERO : BRDY : EOS : FLAGB : FLAGA */
	status = opna->ReadStatusEx() & ~0x80;
	if(busy) {
		// from PC-88 machine language master bible (XM8 version 1.00)
		if (get_passed_usec(clock_busy) < (is_ym2608 ? 4.25 : 2.13)) {
			status |= 0x80;
		} else {
			busy = false;
		}
	}
	return status;
}
#endif

void YM2203::update_timing(int new_clocks, double new_frames_per_sec, int new_lines_per_frame)
{
#ifdef HAS_YM2608
	if(is_ym2608) {
		clock_const = (uint32_t)((double)chip_clock * 1024.0 * 1024.0 / (double)new_clocks / 2.0 + 0.5);
	} else
#endif
	clock_const = (uint32_t)((double)chip_clock * 1024.0 * 1024.0 / (double)new_clocks + 0.5);
}

// ----------------------------------------------------------------------------
#ifdef USE_EMU_INHERENT_SPEC
void YM2203::save_state(FILEIO *fio)
{
	struct vm_state_st vm_state;
	size_t state_size = 0;

	//
	vm_state_ident.version = Uint16_LE(3);

	// copy values
	memset(&vm_state, 0, sizeof(vm_state));

	// reserved header
	fio->Fwrite(&vm_state_ident, sizeof(vm_state_ident), 1);

	vm_state.v3.ch = ch;
	vm_state.v3.fnum2 = fnum2;
#ifdef HAS_YM2608
	vm_state.v3.ch1 = ch1;
	vm_state.v3.data1 = data1;
	vm_state.v3.fnum21 = fnum21;
#endif

	vm_state.v3.mode = mode;
	for(int i=0; i<2; i++) {
		vm_state.v3.port[i].wreg = port[i].wreg;
		vm_state.v3.port[i].rreg = port[i].rreg;
		vm_state.v3.port[i].first = port[i].first ? 1 : 0;
	}
	vm_state.v3.irq_prev = irq_prev ? 1 : 0;
	vm_state.v3.mute = mute ? 1 : 0;
	vm_state.v3.busy = busy ? 1 : 0;

	vm_state.v3.chip_clock = Int32_LE(chip_clock);
	vm_state.v3.clock_prev = Uint64_LE(clock_prev);
	vm_state.v3.clock_accum = Uint64_LE(clock_accum);
	vm_state.v3.clock_const = Uint64_LE(clock_const);
	vm_state.v3.clock_busy = Uint64_LE(clock_busy);
	vm_state.v3.timer_event_id = Int32_LE(timer_event_id);

	fio->Fwrite(&vm_state, sizeof(vm_state), 1);

#ifdef HAS_YM2608
	if(is_ym2608) {
		opna->SaveState((void *)fio, &state_size);
	} else
#endif
	{
		opn->SaveState((void *)fio, &state_size);
	}

	// set total size
	state_size += sizeof(vm_state);
	vm_state_ident.size = Uint32_LE((uint32_t)(sizeof(vm_state_ident) + state_size));

	// overwrite header
	fio->Fseek(-(long)(state_size + sizeof(vm_state_ident)), FILEIO::SEEKCUR);
	fio->Fwrite(&vm_state_ident, sizeof(vm_state_ident), 1);
	fio->Fseek((long)state_size, FILEIO::SEEKCUR);

}

bool YM2203::load_state(FILEIO *fio)
{
	vm_state_ident_t vm_state_i;
	struct vm_state_st vm_state;

	int match = 0;
	for(int i=0; YM2203_CLASS_NAMES[i] != NULL; i++) {
		if (find_state_chunk(fio, YM2203_CLASS_NAMES[i], this_identifier, &vm_state_i)) {
			if (vm_state_i.version >= 3) {
				match = i + 1;
#ifdef HAS_YM2608
				if ((is_ym2608 && i == CLASS_NAMES_POS_YM2608)
				 || (!is_ym2608 && i == CLASS_NAMES_POS_YM2203)) {
					 // match device
					 break;
				}
#else
				if (i == CLASS_NAMES_POS_YM2203) {
					 // match device
					 break;
				}
#endif
				match = 0;
			} else {
				if (i == CLASS_NAMES_POS_YM2203) {
					match = 1;	// PSG
					break;
				}
			}
		}
	}

	if (!match) {
		return true;
	}

	uint32_t i_size = Uint32_LE(vm_state_i.size);
	memset(&vm_state, 0, sizeof(vm_state));
	if (i_size >= (sizeof(vm_state) + sizeof(vm_state_i))) {
		fio->Fread(&vm_state, sizeof(vm_state), 1);
	} else {
		fio->Fread(&vm_state, i_size - sizeof(vm_state_i), 1);
	}

	if (match == 1) {
		// read AY38910 parameter
		struct vm_state_psg_st *vm_state_psg = (struct vm_state_psg_st *)&vm_state;
#ifdef HAS_YM2608
		if (is_ym2608) {
			opna->SetReg(0x2d, 0);
			opna->SetReg(0x28, 0);
			opna->SetReg(0x28, 1);
			opna->SetReg(0x28, 2);
		} else
#endif
		{
			opn->SetReg(0x2d, 0);
			opn->SetReg(0x28, 0);
			opn->SetReg(0x28, 1);
			opn->SetReg(0x28, 2);
		}
		ch = vm_state_psg->ch;
		if (Uint16_LE(vm_state_i.version) >= 2) {
#ifdef HAS_YM2608
			if (is_ym2608) {
				for(int i=0; i<14; i++) {
					 opna->SetReg(i, vm_state_psg->v2.reg[i]);
				}
			} else
#endif
			{
				for(int i=0; i<14; i++) {
					 opn->SetReg(i, vm_state_psg->v2.reg[i]);
				}
			}
#ifdef SUPPORT_YM2203_PORT
			mode = vm_state_psg->mode;
			for(int i=0; i<2; i++) {
				port[i].wreg = vm_state_psg->v2.port[i].wreg;
				port[i].rreg = vm_state_psg->v2.port[i].rreg;
				port[i].first = vm_state_psg->v2.port[i].first ? true : false;
			}
#endif
		} else {
#ifdef HAS_YM2608
			if (is_ym2608) {
				for(int i=0; i<13; i++) {
					 opna->SetReg(i, vm_state_psg->v1.reg[i]);
				}
			} else
#endif
			{
				for(int i=0; i<13; i++) {
					 opn->SetReg(i, vm_state_psg->v1.reg[i]);
				}
			}
#ifdef SUPPORT_YM2203_PORT
			mode = vm_state_psg->mode;
			for(int i=0; i<2; i++) {
				port[i].wreg = vm_state_psg->v1.port[i].wreg;
				port[i].rreg = vm_state_psg->v1.port[i].rreg;
				port[i].first = vm_state_psg->v1.port[i].first ? true : false;
			}
#endif
		}
	} else {
		ch = vm_state.v3.ch;
		fnum2 = vm_state.v3.fnum2;
#ifdef HAS_YM2608
		ch1 = vm_state.v3.ch1;
		data1 = vm_state.v3.data1;
		fnum21 = vm_state.v3.fnum21;
#endif

		mode = vm_state.v3.mode;
		for(int i=0; i<2; i++) {
			port[i].wreg = vm_state.v3.port[i].wreg;
			port[i].rreg = vm_state.v3.port[i].rreg;
			port[i].first = (vm_state.v3.port[i].first == 1);
		}
		irq_prev = (vm_state.v3.irq_prev == 1);
		mute = (vm_state.v3.mute == 1);
		busy = (vm_state.v3.busy == 1);

		chip_clock = Int32_LE(vm_state.v3.chip_clock);
		clock_prev = Uint64_LE(vm_state.v3.clock_prev);
		clock_accum = Uint64_LE(vm_state.v3.clock_accum);
		clock_const = Uint64_LE(vm_state.v3.clock_const);
		clock_busy = Uint64_LE(vm_state.v3.clock_busy);
		timer_event_id = Int32_LE(vm_state.v3.timer_event_id);

#ifdef HAS_YM2608
		if(is_ym2608) {
			opna->LoadState((void *)fio);
		} else
#endif
		{
			opn->LoadState((void *)fio);
		}
	}

	return true;
}
#endif

// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
uint32_t YM2203::debug_read_io8(uint32_t addr)
{
	return read_io8(addr);
}

void YM2203::debug_write_data8(int type, uint32_t addr, uint32_t data)
{
#ifdef HAS_YM2608
	if(is_ym2608) {
		opna->WriteRAMDirect(addr, data);
	}
#endif
}

uint32_t YM2203::debug_read_data8(int type, uint32_t addr)
{
	uint32_t data = 0xff;
#ifdef HAS_YM2608
	if(is_ym2608) {
		data = opna->ReadRAMDirect(addr);
	}
#endif
	return data;
}

uint32_t YM2203::debug_physical_addr_mask(int type)
{
	uint32_t data = 0;
#ifdef HAS_YM2608
	if(is_ym2608) {
		data = 0x3fffff;
	}
#endif
	return data;
}

bool YM2203::debug_write_reg(uint32_t reg_num, uint32_t data)
{
	 set_reg(reg_num, data);
	 return true;
}

bool YM2203::debug_write_reg(const _TCHAR *reg, uint32_t data)
{
	return false;
}

void YM2203::debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	int val = 0;

	buffer[0] = _T('\0');

	// for opn/opna register
	for(uint32_t m=0;
#ifdef HAS_YM2608
		m<(uint32_t)(is_ym2608 ? 2 : 1);
#else
		m<1;
#endif
		m++) {
		UTILITY::sntprintf(buffer, buffer_len, _T(" --- addr:%01Xxx\n"), m);
		// status
#ifdef HAS_YM2608
		if (m == 1) {
			val = (int)read_status_ex();
			UTILITY::sntprintf(buffer, buffer_len, _T(" STATUS:%02X\n"), val);
		} else
#endif
		{
			val = (int)read_status();
			UTILITY::sntprintf(buffer, buffer_len, _T(" STATUS:%02X\n"), val);
		}
	for(uint32_t l=0; l<256; l++) {
		bool valid = false;
		bool lf = false;
		uint32_t ad = m * 256 + l;

		if (ad < 0x20) {
#ifdef HAS_YM2608
			val = is_ym2608 ? (int)opna->GetReg(ad) : (int)opn->GetReg(ad);
#else
			val = (int)opn->GetReg(ad);
#endif
		} else {
			val = debug_regs[ad];
		}

		if (ad < 0x10
		|| (ad >= 0x24 && ad <= 0x28)
		|| (ad >= 0x2d && ad <= 0x2f)
		|| (ad >= 0x30 && ad < 0xb3 && (ad & 3) != 3)
#ifdef HAS_YM2608
		|| (is_ym2608 && ((ad >= 0x10 && ad <= 0x12)
			|| (ad >= 0x18 && ad <= 0x1d)
			|| (ad == 0x22)
			|| (ad == 0x29)
			|| (ad >= 0xb4 && ad <= 0xb6)
			|| (ad >= 0x100 && ad <= 0x110)
			|| (ad >= 0x130 && ad <= 0x1b6 && (ad & 3) != 3)))
#endif
		) {
			valid = true;
		}
		if (ad == 7
		|| ((ad & 0x0f) == 0x0f)
		|| (ad >= 0x30 && ad < 0x100 && ((ad & 0x0f) == 0x0e))
#ifdef HAS_YM2608
		|| (is_ym2608 && ((ad == 0x1d)
			|| (ad == 0xb6)
			|| (ad == 0x107)
			|| (ad == 0x110)
			|| (ad >= 0x130 && ((ad & 0x0f) == 0x0e))))
#endif
		) {
			lf = true;
		}

		if (!valid) continue;

		UTILITY::sntprintf(buffer, buffer_len, _T(" %02X:%02X"), l, val);
		if (_tcslen(buffer) + 6 > buffer_len) break;
		if (lf) UTILITY::tcscat(buffer, buffer_len, _T("\n"));
	}
	}
}
#endif
