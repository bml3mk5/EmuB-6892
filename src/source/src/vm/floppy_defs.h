/** @file floppy_defs.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2020.11.05

	@brief [ floppy drive definition ]
*/

#ifndef FLOPPY_DEFS_H
#define FLOPPY_DEFS_H

#include "vm_defs.h"

/// @ingroup Enums
/// @brief signals FDC, FLOPPY, FDD and DISK
enum SIG_FLOPPY_IDS {
#ifdef USE_SIG_FLOPPY_ACCESS
	SIG_FLOPPY_ACCESS				= 0,
#endif
	SIG_FLOPPY_READ_ID				= 1,	///< read id (FDC to DISK)
//	SIG_FLOPPY_READ_ID_TRACK_NUM	= 2,
//	SIG_FLOPPY_READ_ID_HEAD_NUM		= 3,
	SIG_FLOPPY_READ_TRACK_LOOP		= 5,	///< read track (FDC to DISK)
	SIG_FLOPPY_READ					= 6,	///< read sector (FDC to DISK)
	SIG_FLOPPY_READ_TRACK			= 7,	///< read track (FDC to DISK)
	SIG_FLOPPY_WRITE				= 8,	///< write sector (FDC to DISK)
	SIG_FLOPPY_WRITE_TRACK			= 9,	///< write track (FDC to DISK)
	SIG_FLOPPY_WRITEDELETE			= 10,	///< write sector with deleted mark (FDC to DISK)
	SIG_FLOPPY_WRITEPROTECT			= 11,	///< write protect (DISK to FDC)
	SIG_FLOPPY_STEP					= 12,	///< step and dirc (FDC to DISK)
	SIG_FLOPPY_HEADLOAD				= 13,	///< head load (DISK to FDC)
	SIG_FLOPPY_READY				= 14,	///< ready (DISK to FDC)
	SIG_FLOPPY_TRACK0				= 15,	///< track 0 (DISK to FDC)
	SIG_FLOPPY_INDEX				= 16,	///< index hole (DISK to FDC)
	SIG_FLOPPY_DELETED				= 17,	///< deleted mark (DISK to FDC)

#ifdef SET_CURRENT_TRACK_IMMEDIATELY
	SIG_FLOPPY_CURRENTTRACK			= 18,
#endif
	SIG_FLOPPY_SECTOR_NUM			= 19,	///< sector number
	SIG_FLOPPY_SECTOR_SIZE			= 20,	///< sector size
	SIG_FLOPPY_TRACK_SIZE			= 21,	///< track size

	SIG_FLOPPY_IRQ					= 26,	///< IRQ from FDC (for 5inch mini floppy)
	SIG_FLOPPY_DRQ					= 27,	///< DRQ from FDC (for 5inch mini floppy)

	SIG_FLOPPY_DENSITY				= 28	///< density to FDC (for 5inch mini floppy)
};

/// @ingroup Enums
/// @brief status of searching sector
enum SEARCH_SECTOR_STATUS
{
	STS_RECORD_NOT_FOUND		 = 0x0001,
	STS_CRC_ERROR				 = 0x0002,
	STS_DELETED_MARK_DETECTED	 = 0x0004,
	STS_UNMATCH_TRACK_NUMBER	 = 0x0010,
	STS_UNMATCH_SIDE_NUMBER		 = 0x0020,
	STS_UNMATCH_SECTOR_NUMBER	 = 0x0040,
	STS_UNMATCH_SECTOR_SIZE		 = 0x0080
};

#endif /* FLOPPY_DEFS_H */

