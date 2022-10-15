/** @file ay38910.h

	Skelton for retropc emulator

	@author Takeda.Toshiya
	@date   2006.09.15-

	@par Origin ym2203.h

	@brief [ AY-3-8910 / 2 / 3 ]
*/

#ifndef AY_3_8910_H
#define AY_3_8910_H

#include "vm_defs.h"
#include "device.h"

#if defined(HAS_AY_3_8913)
// both port a and port b are not supported
#elif defined(HAS_AY_3_8912)
// port b is not supported
#define SUPPORT_AY_3_8910_PORT_A
#else
#define SUPPORT_AY_3_8910_PORT_A
#define SUPPORT_AY_3_8910_PORT_B
#endif

#if defined(SUPPORT_AY_3_8910_PORT_A) || defined(SUPPORT_AY_3_8910_PORT_B)
#define SUPPORT_AY_3_8910_PORT
#endif

#ifdef SUPPORT_AY_3_8910_PORT_A
#define SIG_AY_3_8910_PORT_A	0
#endif
#ifdef SUPPORT_AY_3_8910_PORT_B
#define SIG_AY_3_8910_PORT_B	1
#endif
#define SIG_AY_3_8910_MUTE		2

class EMU;
class PSG;

/**
	@brief AY-3-8910/2/3 - PSG Sound Operator
*/
class AY38910 : public DEVICE
{
private:
	PSG* psg;

	int psg_clock;
	int psg_rate;

	uint8_t ch;

#ifdef SUPPORT_AY_3_8910_PORT
	struct {
		uint8_t wreg;
		uint8_t rreg;
		bool first;
		// output signals
		outputs_t outputs;
	} port[2];
	uint8_t mode;
#endif

	bool mute;

#ifdef USE_EMU_INHERENT_SPEC
//	int sound_volume;
//	int32_t buffer_tmp[40];
#endif

#ifdef USE_EMU_INHERENT_SPEC
	//for resume
#pragma pack(1)
	struct vm_state_st {
		uint8_t ch, mode;
		// PSG register only
		union {
			struct {
				uint8_t reg[13];
				struct {
					uint8_t wreg;
					uint8_t rreg;
					uint8_t first;
				} port[2];
				char reserved;
			} v1;
			struct {
				uint8_t reg[14];
				struct {
					uint8_t wreg;
					uint8_t rreg;
					uint8_t first;
				} port[2];
			} v2;
		};
		char reserved[10];
	};
#pragma pack()
#endif

public:
	AY38910(VM* parent_vm, EMU* parent_emu, const char* identifier) : DEVICE(parent_vm, parent_emu, identifier) {
		set_class_name("AY38910");
#ifdef SUPPORT_AY_3_8910_PORT
		for(int i = 0; i < 2; i++) {
			init_output_signals(&port[i].outputs);
			port[i].wreg = port[i].rreg = 0;//0xff;
		}
#endif
	}
	~AY38910() {}

	// common functions
	void initialize();
	void release();
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
	void mix(int32_t* buffer, int cnt);
	void set_volume(int decibel_l, int decibel_r, bool vol_mute, int pattern);
	void save_state(FILEIO *fp);
	bool load_state(FILEIO *fp);

	// unique functions
#ifdef SUPPORT_AY_3_8910_PORT_A
	void set_context_port_a(DEVICE* device, int id, uint32_t mask, int shift) {
		register_output_signal(&port[0].outputs, device, id, mask, shift);
	}
#endif
#ifdef SUPPORT_AY_3_8910_PORT_B
	void set_context_port_b(DEVICE* device, int id, uint32_t mask, int shift) {
		register_output_signal(&port[1].outputs, device, id, mask, shift);
	}
#endif
	void initialize_sound(int rate, int clock, int samples, int decibel_psg, int ptn);

#ifdef USE_DEBUGGER
	uint32_t debug_read_io8(uint32_t addr);
	bool debug_write_reg(uint32_t reg_num, uint32_t data);
	bool debug_write_reg(const _TCHAR *reg, uint32_t data);
	void debug_regs_info(_TCHAR *buffer, size_t buffer_len);
#endif
};

#endif /* AY_3_8910_H */
