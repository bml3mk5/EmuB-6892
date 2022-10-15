/** @file registers.h

	HITACHI BASIC MASTER LEVEL3 Mark5 Emulator 'EmuB-6892'
	Skelton for retropc emulator

	@author Sasaji
	@date   2017.12.01 -

	@brief [ registers on circit ]
*/

#ifndef REGISTERS_H
#define REGISTERS_H

#include "../../common.h"

/// @defgroup MACROS_REGISTERS Macros related to REGISTERS class
/// Macros related to @ref REGISTERS class

/// @ingroup MACROS_REGISTERS
///@{
#define REG_MODE_SEL		registers.reg_mode_sel
#define REG_REMOTE			registers.reg_remote
#define REG_TIME_MASK		registers.reg_time_mask
#define REG_INTERLACE_SEL	registers.reg_interlace_sel
#define REG_BAUD_SEL		registers.reg_baud_sel

#define REG_IGENABLE		registers.reg_igenable
///@}

/**
	@brief Common register on the board

	@sa MACROS_REGISTERS
*/
class REGISTERS
{
public:
	uint8_t reg_mode_sel;		///< 0xffd0 mode sel
	uint8_t reg_remote;			///< 0xffd2 remote for cmt
	uint8_t reg_time_mask;		///< 0xffd4 time mask
	uint8_t reg_interlace_sel;	///< 0xffd6 interlace sel (shifted)  0:noninterlace char  1:interlace char
	uint8_t reg_baud_sel;		///< 0xffd7 baud select 0:600 1:1200

	uint8_t reg_igenable;		///< 0xffe9 write

public:
	REGISTERS();
	virtual ~REGISTERS() {}

	virtual void initialize();
};

extern REGISTERS registers;

#endif /* REGISTERS_H */
