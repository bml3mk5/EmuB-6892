/** @file kanji.h

	HITACHI BASIC MASTER LEVEL3 Mark5 / MB-S1 Emulator 'EmuB-6892/EmuB-S1'
	Skelton for retropc emulator

	@author Sasaji
	@date   2011.10.19 -

	@brief [ kanji ]
*/

#ifndef KANJI_H
#define KANJI_H

#include "../vm_defs.h"
#include "../device.h"

class EMU;

/**
	@brief kanji ROM
*/
class KANJI : public DEVICE
{
private:
	uint16_t code;
	uint8_t  offset;

	uint8_t font[0x20000];	// 128KB

	int  font_enable;

#ifdef USE_KANJI_JIS2
	uint8_t font2[0x20000];	// 128KB

	int  font2_enable;

	uint8_t en_jis2;
#endif

	//for resume
#pragma pack(1)
	struct vm_state_st {
		uint16_t code;
		uint8_t  offset;
#ifdef USE_KANJI_JIS2
		uint8_t	 en_jis2;
#else
		char  reserved0;
#endif
		char  reserved[12];
	};
#pragma pack()

	void init_kanji_rom(uint8_t *rom, size_t size);

	static const uint8_t krom[32];

public:
	KANJI(VM* parent_vm, EMU* parent_emu, const char* identifier) : DEVICE(parent_vm, parent_emu, identifier)
	{
		set_class_name("KANJI");
	}
	~KANJI() {}

	// common functions
	void initialize();
	void reset();
	void write_signal(int id, uint32_t data, uint32_t mask);
	void event_frame();

	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);

	void event_callback(int event_id, int err);

	void save_state(FILEIO *fio);
	bool load_state(FILEIO *fio);

#ifdef USE_DEBUGGER
	uint32_t debug_read_io8(uint32_t addr);
#endif
};

#endif /* KANJI_H */

