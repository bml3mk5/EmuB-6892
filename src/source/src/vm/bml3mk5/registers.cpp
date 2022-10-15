/** @file registers.cpp

	HITACHI BASIC MASTER LEVEL3 Mark5 Emulator 'EmuB-6892'
	Skelton for retropc emulator

	@author Sasaji
	@date   2017.12.01 -

	@brief [ registers on circit ]
*/

#include "registers.h"

REGISTERS registers;

REGISTERS::REGISTERS()
{
	initialize();
}

void REGISTERS::initialize()
{
	reg_mode_sel = 0;
	reg_remote = 0;
	reg_time_mask = 0;
	reg_interlace_sel = 0;
	reg_baud_sel = 0;
	reg_igenable = 0;
}
