/** @file psgc.h

	HITACHI BASIC MASTER LEVEL3 Mark5 Emulator 'EmuB-6892'
	Skelton for retropc emulator

	@author Sasaji
	@date   2012.06.08 -

	@brief [ psg control ]
*/

#ifndef PSGC_H
#define PSGC_H

#include "../vm_defs.h"
#include "../device.h"

class EMU;

/**
	@brief psg control - bridging between PSG(YM2203) and PIA
*/
class PSGC : public DEVICE
{
public:
	/// @brief signals on PSGC
	enum SIG_PSG_IDS {
		SIG_PSG_PIA_PA	= 0,
		SIG_PSG_PIA_PB	= 1
	};

private:
	DEVICE *d_psg[2], *d_pia;

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
	PSGC(VM* parent_vm, EMU* parent_emu, const char* identifier) : DEVICE(parent_vm, parent_emu, identifier) {
		set_class_name("PSGC");
		d_psg[0] = NULL;
		d_psg[1] = NULL;
		d_pia = NULL;
	}
	~PSGC() {}

	// common functions
	void initialize();
	void reset();

	void write_signal(int id, uint32_t data, uint32_t mask);

	// unique functions
	void set_context_psg(DEVICE* device0, DEVICE* device1) {
		d_psg[0] = device0;
		d_psg[1] = device1;
	}
	void set_context_pia(DEVICE* device) {
		d_pia = device;
	}

	void save_state(FILEIO *fio);
	bool load_state(FILEIO *fio);
};

#endif /* PSGC_H */
