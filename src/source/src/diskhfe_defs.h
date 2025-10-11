/** @file diskhfe_defs.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2025.08.10

	@brief [hfe disk image defines]
*/

#ifndef DISKHFE_DEFS_H
#define DISKHFE_DEFS_H

#include "common.h"

#define DISK_HFE_HEADER "HXCPICFE"
#define DISK_HFE_HEADV3 "HXCHFEV3"

// HFE format defines
// from libhxcfe\sources\libhxcfe.h

// interface mode
#define IBMPC_DD_FLOPPYMODE				0x00
#define IBMPC_HD_FLOPPYMODE				0x01
#define ATARIST_DD_FLOPPYMODE			0x02
#define ATARIST_HD_FLOPPYMODE			0x03
#define AMIGA_DD_FLOPPYMODE				0x04
#define AMIGA_HD_FLOPPYMODE				0x05
#define CPC_DD_FLOPPYMODE				0x06
#define GENERIC_SHUGGART_DD_FLOPPYMODE	0x07
#define IBMPC_ED_FLOPPYMODE				0x08
#define MSX2_DD_FLOPPYMODE				0x09
#define C64_DD_FLOPPYMODE				0x0A
#define EMU_SHUGART_FLOPPYMODE			0x0B
#define S950_DD_FLOPPYMODE				0x0C
#define S950_HD_FLOPPYMODE				0x0D
#define S950_DD_HD_FLOPPYMODE			0x0E
#define IBMPC_DD_HD_FLOPPYMODE			0x0F
#define QUICKDISK_FLOPPYMODE			0x10
#define FLOPPYMODE_MAX					0x11
#define DISABLE_FLOPPYMODE				0xFE
#define UNKNOWN_FLOPPYMODE				0xFF

// encoding
#define ISOIBM_MFM_ENCODING				0x00
#define AMIGA_MFM_ENCODING				0x01
#define ISOIBM_FM_ENCODING				0x02
#define EMU_FM_ENCODING					0x03
#define TYCOM_FM_ENCODING				0x04
#define MEMBRAIN_MFM_ENCODING			0x05
#define APPLEII_GCR1_ENCODING			0x06
#define APPLEII_GCR2_ENCODING			0x07
#define APPLEII_HDDD_A2_GCR1_ENCODING	0x08
#define APPLEII_HDDD_A2_GCR2_ENCODING	0x09
#define ARBURGDAT_ENCODING				0x0A
#define ARBURGSYS_ENCODING				0x0B
#define AED6200P_MFM_ENCODING			0x0C
#define NORTHSTAR_HS_MFM_ENCODING		0x0D
#define HEATHKIT_HS_FM_ENCODING			0x0E
#define DEC_RX02_M2FM_ENCODING			0x0F
#define APPLEMAC_GCR_ENCODING			0x10
#define QD_MO5_ENCODING					0x11
#define C64_GCR_ENCODING				0x12
#define VICTOR9K_GCR_ENCODING			0x13
#define MICRALN_HS_FM_ENCODING			0x14
#define CENTURION_MFM_ENCODING			0x15
#define ENCODING_MAX					0x16
#define UNKNOWN_ENCODING				0xFF

#define DISALE_ENCODING					0xFE

#pragma pack(1)
/// HxC HFE header (512bytes) (LE)
typedef struct st_hfe_header {
	char     signature[8];
	uint8_t  revision;				// always 0
	uint8_t  tracks;
	uint8_t  sides;
	uint8_t  encoding;
	uint16_t bit_rate;
	uint16_t rpm;

	uint8_t  interface_mode;
	uint8_t  dnu;
	uint16_t track_list_offset;		// Offset of the track list LUT in block of 512bytes
	uint8_t  write_allowed;
	uint8_t  single_step;
	uint8_t  track0s0_encode_enable;
	uint8_t  track0s0_encode;
	uint8_t  track0s1_encode_enable;
	uint8_t  track0s1_encode;

	char     reserved[486];
} hfe_header_t;

/// HxC HFE track offset (LE)
typedef struct st_hfe_track_offset {
	uint16_t offset;	// Offset of the track data in block of 512bytes
	uint16_t track_len;
} hfe_track_offset_t;

/// HxC HFE track offset LUT (up to 1024bytes) (LE)
typedef struct st_hfe_track_offset_list {
	hfe_track_offset_t at[256];
} hfe_track_offset_list_t;
#pragma pack()

#endif /* DISKHFE_DEFS_H */
