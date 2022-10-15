/** @file ay38910.cpp

	Skelton for retropc emulator

	@author Takeda.Toshiya
	@date   2006.09.15-

	@par Origin ym2203.cpp

	@brief [ AY-3-8910 / 2 / 3 ]
*/

#include "ay38910.h"
//#include "../emu.h"
#include "fmgen/psg.h"
#include "../fileio.h"
#include "../utility.h"

void AY38910::initialize()
{
	psg = new PSG(0x80);
	mute = false;
}

void AY38910::release()
{
	delete psg;
}

void AY38910::reset()
{
	psg->Reset();
	psg->SetClock(psg_clock / 2, psg_rate);

#ifdef SUPPORT_AY_3_8910_PORT
	port[0].first = port[1].first = true;
	port[0].wreg = port[1].wreg = 0;//0xff;
#ifdef AY_3_8910_PORT_MODE
	mode = AY_3_8910_PORT_MODE;
#else
	mode = 0;
#endif
#endif
}

#define amask 1

void AY38910::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & amask) {
	case 0:
		ch = data & 0xff;
		break;
	case 1:
#ifdef SUPPORT_AY_3_8910_PORT
		if(ch == 7) {
#ifdef AY_3_8910_PORT_MODE
			mode = (data & 0x3f) | AY_3_8910_PORT_MODE;
#else
			mode = data;
#endif
		} 
		if(ch == 14) {
#ifdef SUPPORT_AY_3_8910_PORT_A
			if(port[0].wreg != data || port[0].first) {
				write_signals(&port[0].outputs, data);
				port[0].wreg = data;
				port[0].first = false;
			}
#endif
		} else if(ch == 15) {
#ifdef SUPPORT_AY_3_8910_PORT_B
			if(port[1].wreg != data || port[1].first) {
				write_signals(&port[1].outputs, data);
				port[1].wreg = data;
				port[1].first = false;
			}
#endif
		}
#endif
		psg->SetReg(ch, data);
		break;
	}
}

uint32_t AY38910::read_io8(uint32_t addr)
{
	switch(addr & amask) {
	case 1:
#ifdef SUPPORT_AY_3_8910_PORT
		if(ch == 14) {
#ifdef SUPPORT_AY_3_8910_PORT_A
			return (mode & 0x40) ? port[0].wreg : port[0].rreg;
#endif
		} else if(ch == 15) {
#ifdef SUPPORT_AY_3_8910_PORT_B
			return (mode & 0x80) ? port[1].wreg : port[1].rreg;
#endif
		}
#endif
		return (ch <= 15 ? psg->GetReg(ch) : 0xff);
	}
	return 0xff;
}

void AY38910::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_AY_3_8910_MUTE) {
		mute = ((data & mask) != 0);
#ifdef SUPPORT_AY_3_8910_PORT_A
	} else if(id == SIG_AY_3_8910_PORT_A) {
		port[0].rreg = (port[0].rreg & ~mask) | (data & mask);
#endif
#ifdef SUPPORT_AY_3_8910_PORT_B
	} else if(id == SIG_AY_3_8910_PORT_B) {
		port[1].rreg = (port[1].rreg & ~mask) | (data & mask);
#endif
	} else if (id == SIG_CPU_RESET) {
		now_reset = (data & mask) ? true : false;
		reset();
	}
}

void AY38910::mix(int32_t* buffer, int cnt)
{
#if 0 // def USE_EMU_INHERENT_SPEC
	memset(buffer_tmp, 0, sizeof(buffer_tmp));
	if (cnt > 20) cnt = 20;
	if(cnt > 0 && !mute) {
		psg->Mix(buffer_tmp, cnt);
	}
	for(int i=0; i<(cnt * 2); i++) {
		*buffer++ += (buffer_tmp[i] * sound_volume / 100);
	}
#else
	if(cnt > 0 && !mute) {
		psg->Mix(buffer, cnt);
	}
#endif
}

void AY38910::set_volume(int decibel_l, int decibel_r, bool vol_mute, int pattern)
{
	mute = vol_mute;
	psg->SetVolume(decibel_l, decibel_r, pattern | 0x80);
}

void AY38910::initialize_sound(int rate, int clock, int samples, int decibel_psg, int ptn)
{
	psg_clock = clock;
	psg_rate = rate;

	psg->SetVolume(decibel_psg, decibel_psg, ptn | 0x80);

#ifdef USE_EMU_INHERENT_SPEC
//	sound_volume = 0;
#endif
}

// ----------------------------------------------------------------------------
#ifdef USE_EMU_INHERENT_SPEC
void AY38910::save_state(FILEIO *fio)
{
	struct vm_state_st vm_state;

	//
	vm_state_ident.version = Uint16_LE(3);
	vm_state_ident.size = Uint32_LE(sizeof(vm_state_ident) + sizeof(vm_state));

	// copy values
	memset(&vm_state, 0, sizeof(vm_state));
	// only save PSG register
	vm_state.ch = ch;
	for(int i=0; i<14; i++) {
		vm_state.v2.reg[i] = psg->GetReg(i);
	}
#ifdef SUPPORT_AY_3_8910_PORT
	vm_state.mode = mode;
	for(int i=0; i<2; i++) {
		vm_state.v2.port[i].wreg = port[i].wreg;
		vm_state.v2.port[i].rreg = port[i].rreg;
		vm_state.v2.port[i].first = port[i].first ? 1 : 0;
	}
#endif
	fio->Fwrite(&vm_state_ident, sizeof(vm_state_ident), 1);
	fio->Fwrite(&vm_state, sizeof(vm_state), 1);
}

bool AY38910::load_state(FILEIO *fio)
{
	vm_state_ident_t vm_state_i;
	struct vm_state_st vm_state;

	bool match = false;
	// compatible old version
	if (find_state_chunk(fio, "YM2203", this_identifier, &vm_state_i)) {
		if (vm_state_i.version <= 2) {
			match = true;
		}
	}
	if (!match && find_state_chunk(fio, &vm_state_i) != true) {
		return true;
	}
	uint32_t i_size = Uint32_LE(vm_state_i.size);
	memset(&vm_state, 0, sizeof(vm_state));
	if (i_size >= (sizeof(vm_state) + sizeof(vm_state_i))) {
		fio->Fread(&vm_state, sizeof(vm_state), 1);
		fio->Fseek(i_size - sizeof(vm_state) - sizeof(vm_state_i), FILEIO::SEEKCUR);
	} else {
		fio->Fread(&vm_state, i_size - sizeof(vm_state_i), 1);
	}

	// only load PSG register
	ch = vm_state.ch;
	if (Uint16_LE(vm_state_i.version) >= 2) {
		for(int i=0; i<14; i++) {
			 psg->SetReg(i, vm_state.v2.reg[i]);
		}
#ifdef SUPPORT_AY_3_8910_PORT
		mode = vm_state.mode;
		for(int i=0; i<2; i++) {
			port[i].wreg = vm_state.v2.port[i].wreg;
			port[i].rreg = vm_state.v2.port[i].rreg;
			port[i].first = vm_state.v2.port[i].first ? true : false;
		}
#endif
	} else {
		for(int i=0; i<13; i++) {
			 psg->SetReg(i, vm_state.v1.reg[i]);
		}
#ifdef SUPPORT_AY_3_8910_PORT
		mode = vm_state.mode;
		for(int i=0; i<2; i++) {
			port[i].wreg = vm_state.v1.port[i].wreg;
			port[i].rreg = vm_state.v1.port[i].rreg;
			port[i].first = vm_state.v1.port[i].first ? true : false;
		}
#endif
	}

	return true;
}
#endif

// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
uint32_t AY38910::debug_read_io8(uint32_t addr)
{
	return read_io8(addr);
}

bool AY38910::debug_write_reg(uint32_t reg_num, uint32_t data)
{
	// psg
	if (reg_num < 14) {
		 psg->SetReg(reg_num, data);
		 return true;
	}
	return false;
}

bool AY38910::debug_write_reg(const _TCHAR *reg, uint32_t data)
{
	return false;
}

void AY38910::debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	buffer[0] = _T('\0');
	// psg
	for(uint32_t i=0; i<14; i++) {
		size_t pos = _tcslen(buffer);
		UTILITY::stprintf(&buffer[pos], buffer_len - pos, _T(" %X:%02X"), i, psg->GetReg(i));
		if ((i % 7) == 7) UTILITY::tcscat(buffer, buffer_len, _T("\n"));
	}
}
#endif