/** @file diskd88_defs.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2019.09.01

	@brief [d88 disk image defines]
*/

#ifndef DISKD88_DEFS_H
#define DISKD88_DEFS_H

#include "common.h"

// d88 media type
#define MEDIA_TYPE_2D	0x00
#define MEDIA_TYPE_2DD	0x10
#define MEDIA_TYPE_2HD	0x20
#define MEDIA_TYPE_UNK	0xff

/// d88 header
typedef struct d88_hdr_st {
	char title[17];
	uint8_t rsrv[9];
	uint8_t protect;
	uint8_t media_type;
	uint32_t size;
	uint32_t trkptr[164];
} d88_hdr_t;

/// d88 sector
typedef struct d88_sct_st {
	uint8_t c, h, r, n;
	uint16_t nsec;
	uint8_t dens, del, stat;
	uint8_t rsrv[5];
	uint16_t size;
} d88_sct_t;

#endif /* DISKD88_DEFS_H */
