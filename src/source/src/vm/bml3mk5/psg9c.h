/** @file psg9c.h

	HITACHI BASIC MASTER LEVEL3 Mark5 Emulator 'EmuB-6892'
	Skelton for retropc emulator

	@author Sasaji
	@date   2012.06.08 -

	@brief [ 9voice psg control ]
*/

#ifndef PSG9C_H
#define PSG9C_H

#include "../vm_defs.h"
#include "../device.h"

class EMU;

/**
	@brief 9voice psg control - bridging between PSG(YM2203) and VIA
*/
class PSG9C : public DEVICE
{
public:
	/// @brief signals on PSG9C
	enum SIG_PSG9_IDS {
		SIG_PSG9_VIA_PA	= 0,
		SIG_PSG9_VIA_PB	= 1
	};

private:
	DEVICE *d_psg[3], *d_via;

	uint8_t pa;
	uint8_t pb;

	uint8_t sel;

	//for resume
#pragma pack(1)
	struct vm_state_st {
		uint8_t pa;
		uint8_t pb;

		uint8_t sel;

		char  reserved[13];
	};
#pragma pack()

public:
	PSG9C(VM* parent_vm, EMU* parent_emu, const char* identifier) : DEVICE(parent_vm, parent_emu, identifier) {
		set_class_name("PSG9C");
		d_psg[0] = NULL;
		d_psg[1] = NULL;
		d_psg[2] = NULL;
		d_via = NULL;
	}
	~PSG9C() {}

	// common functions
	void initialize();
	void reset();

	void write_signal(int id, uint32_t data, uint32_t mask);

	// unique functions
	void set_context_psg(DEVICE* device0, DEVICE* device1, DEVICE* device2) {
		d_psg[0] = device0;
		d_psg[1] = device1;
		d_psg[2] = device2;
	}
	void set_context_via(DEVICE* device) {
		d_via = device;
	}

	void save_state(FILEIO *fio);
	bool load_state(FILEIO *fio);
};

#endif /* PSG9C_H */
