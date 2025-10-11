/** @file disk_image.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2025.08.10-

	@brief [ disk image ]
*/

#include "disk_image.h"
#include "disk.h"
#include "../emu.h"
#include "../fileio.h"
#include "../utility.h"
#include "../config.h"
#include "../depend.h"
#include "../labels.h"

#ifndef USE_CALC_CRC16
// crc table
static const uint16_t crc16_table[256] = {
	0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7, 0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
	0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6, 0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
	0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485, 0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
	0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4, 0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
	0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823, 0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
	0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12, 0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
	0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41, 0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
	0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70, 0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
	0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f, 0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
	0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e, 0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
	0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d, 0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
	0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c, 0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
	0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab, 0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
	0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a, 0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
	0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9, 0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
	0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8, 0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
};
#endif

/// gap sign
uint8_t c_gap_sign[2]={
	0xff, 0x4e
};

/// 0:single density  1:double density
int c_mark_length[2]={
	7,16
};

/// index mark 0:single density  1:double density
uint8_t c_index_mark[2][17]={
	{ 0,0,0,0,0,0,0xfc,0,0,0,0,0,0   ,0   ,0   ,0   ,0 },
	{ 0,0,0,0,0,0,0   ,0,0,0,0,0,0xc2,0xc2,0xc2,0xfc,0 }
};
/// address mark
uint8_t c_address_mark[2][17]={
	{ 0,0,0,0,0,0,0xfe,0,0,0,0,0,0   ,0   ,0   ,0   ,0 },
	{ 0,0,0,0,0,0,0   ,0,0,0,0,0,0xa1,0xa1,0xa1,0xfe,0 }
};
/// data mark
uint8_t c_data_mark[2][17]={
	{ 0,0,0,0,0,0,0xfb,0,0,0,0,0,0   ,0   ,0   ,0   ,0 },
	{ 0,0,0,0,0,0,0   ,0,0,0,0,0,0xa1,0xa1,0xa1,0xfb,0 }
};
/// deleted data mark
uint8_t c_deleted_mark[2][17]={
	{ 0,0,0,0,0,0,0xf8,0,0,0,0,0,0   ,0   ,0   ,0   ,0 },
	{ 0,0,0,0,0,0,0   ,0,0,0,0,0,0xa1,0xa1,0xa1,0xf8,0 }
};

/// 0:single density  1:double density
struct st_ibm_format_length c_ibm_format_length[2] = {
	{  6, 1, 40, 26, 11, 27 },	// FM
	{ 12, 4, 80, 50, 22, 54 }	// MFM
};

DISK_IMAGE::DISK_IMAGE()
{
//	clear();
}

DISK_IMAGE::DISK_IMAGE(const DISK_IMAGE &)
{
//	clear();
}

DISK_IMAGE::DISK_IMAGE(DISK *disk)
{
//	clear();
//	set(disk, buffer, size, size_orig);
}

DISK_IMAGE::~DISK_IMAGE()
{
}

/// @brief Clear valiables
void DISK_IMAGE::clear_base()
{
	p_disk = NULL;
}

/// @brief Set buffer pointer
///
/// @param[in] disk
void DISK_IMAGE::set_base(DISK *disk)
{
	p_disk = disk;
}

/// @brief Get write protected
///
/// @return true if write protected
bool DISK_IMAGE::is_write_protected() const
{
	return false;
}

/// @brief Set write protect
///
/// @param[in] val : true: set the protection
void DISK_IMAGE::set_write_protect(bool val)
{
}

/// @brief Write operation is supported?
///
/// @return true if writable 
bool DISK_IMAGE::is_write_supported() const
{
	return false;
}

/// @brief Get media type
///
/// @return media type
uint8_t DISK_IMAGE::get_media_type() const
{
	return 0;
}

/// @brief Write a disk image
///
/// @param[in] fio : file handle object
/// @param[in] new_file : save as new file
/// @param[in] is_plain : save as plain image
/// @param[in] is_last_volume : last volume in the file
/// @return true if success
bool DISK_IMAGE::write_file(FILEIO *fio, bool new_file, bool is_plain, bool is_last_volume)
{
	return false;
}

// ----------------------------------------------------------------------------

/// @brief Calcrate CRC16
///
/// @param[in] data : new data
/// @param[in] crc  : calculated CRC previously
/// @return calculated CRC with new data  
uint16_t DISK_IMAGE::set_crc16(uint8_t data, uint16_t crc)
{
#ifndef USE_CALC_CRC16
	crc = (uint16_t)((crc << 8) ^ crc16_table[(uint8_t)(crc >> 8) ^ data);
#else
	crc = calc_crc16(data, crc);
#endif
	return crc;
}

#ifdef USE_CALC_CRC16
/// @brief Calcrate CRC16
///
/// @param[in] data : new data
/// @param[in] crc  : calculated CRC previously
/// @return calculated CRC with new data  
uint16_t DISK_IMAGE::calc_crc16(uint8_t data, uint16_t crc)
{
	for (int count = 7; count >= 0; count--) {
		uint16_t bit = (((crc >> 15) ^ (data >> 7)) & 1);
		crc <<= 1;
		crc |= bit;
		data <<= 1;
		if (bit) {
			crc ^= 0x1020;
		}
	}
	return crc;
}
#endif

// ----------------------------------------------------------------------------

/// @brief Check and set the number of side
///
/// @return true if inserted a disk
bool DISK_IMAGE::check_tracks()
{
	return false;
}

/// @brief Get number of side
///
/// @return number of side
int DISK_IMAGE::get_number_of_side() const
{
	return 2;
}

/// @brief Set side position
///
/// @param[in] side_pos : side
void DISK_IMAGE::set_side_position(int side_pos)
{
}

// ----------------------------------------------------------------------------

/// @brief Get current track number
///
/// @return track number
/// @note need call search_track() before invoke this
int DISK_IMAGE::get_current_track_number() const
{
	return 0;
}

/// @brief Get current track
///
/// @return track info
/// @note need call search_track() before invoke this
DISK_TRACK *DISK_IMAGE::get_current_track()
{
	return NULL;
}

/// @brief Get number of sector in current track
///
/// @return number of sector
/// @note need call search_track() before invoke this
int DISK_IMAGE::get_number_of_sector() const
{
	return 0;
}

/// @brief Get raw track size on current track
///
/// @return size
/// @note need call search_track() before invoke this
int DISK_IMAGE::get_current_track_size() const
{
	return 0;
}

/// @brief Get specified track
///
/// @param[in] track_pos : track physical position
/// @param[in] side_pos : side physical position
/// @param[in] density : 0:single density 1:double density
/// @return : false : not found or invalid 
bool DISK_IMAGE::search_track(int track_pos, int side_pos, int density)
{
	return false;
}

/// @brief Verify a track number in each sector on a track
///
/// @param[in] track_num : track number
/// @param[in] side_num : side number
/// @param[in] density : density
/// @return true if match
bool DISK_IMAGE::verify_track_number(int track_num, int side_num, int density)
{
	return false;
}

/// @brief Make raw track
///
/// @param[in] side_num : side number
/// @param[in] density : density
/// @return : false : not found or invalid 
bool DISK_IMAGE::make_track(int side_num, int density)
{
	return false;
}

/// @brief Parse raw track
///
/// @param[in] side_num : side number
/// @param[in] density : density
/// @return : false : not found or invalid 
bool DISK_IMAGE::parse_track(int side_num, int density)
{
	return false;
}

// ----------------------------------------------------------------------------

/// @brief Get current sector position
///
/// @param[in] sector_num : sector number
/// @return sector position
/// @note need call parse_sector() before invoke this
int DISK_IMAGE::get_current_sector_pos(int sector_num) const
{
	return 0;
}

/// @brief Get current sector
///
/// @return sector info
/// @note need call search_track() and search_sector() before invoke this
DISK_SECTOR *DISK_IMAGE::get_current_sector()
{
	return NULL;
}

/// @brief Get number of sector in current track
///
/// @return number of sector
/// @note need call search_track() before invoke this
int DISK_IMAGE::get_current_sector_size() const
{
	return 0;
}

/// @brief Parse sector in current track
///
/// @param[in] side_num : side number
/// @param[in] density : 1 if double density
void DISK_IMAGE::parse_sector(int side_num, int density)
{
	return;
}

/// @brief Finds a sector from specified sector number.
///
/// @param[in] side_num : side number
/// @param[in] density : 1 if double density
/// @param[in] sector_num : sector number or -1 if specify sector position
/// @param[in,out] sector_pos : position of sector in the track. if specify -1 then sector position 0
/// @return DISK::en_result_search_sector
int DISK_IMAGE::search_sector(int side_num, int density, int sector_num, int &sector_pos)
{
	return DISK::RES_DISK_NOT_INSERTED;
} //

/// @brief Finds a sector from specified sector number and calculate the time (clocks) required to reach that sector.
///
/// @param[in] side_num : side number
/// @param[in] density : 1 if double density
/// @param[in] sector_num : sector number or -1 if specify sector position
/// @param[in,out] sector_pos : position of sector in the track. if specify -1 then search nearest sector
/// @param[in] remain_clock : clock until reach index hole
/// @param[in] timeout_round : number of round to occur timeout 
/// @param[out] arrive_clock : clock until reach the sector
/// @return DISK::en_result_search_sector
int DISK_IMAGE::search_sector_and_get_clock(int side_num, int density, int sector_num, int &sector_pos, int remain_clock, int timeout_round, int &arrive_clock)
{
	return DISK::RES_DISK_NOT_INSERTED;
} //

/// @brief Calculate the time (clocks) required to reach the sepcified sector.
///
/// @param[in] sector_num : sector number or -1 if specify sector position
/// @param[in] sector_pos : position of sector in the track. if specify -1 then search nearest sector
/// @param[in] remain_clock : clock until reach index hole 
/// @param[in] timeout_round : number of round to occur timeout 
/// @return clocks
int DISK_IMAGE::get_clock_arrival_sector(int sector_num, int sector_pos, int remain_clock, int timeout_round)
{
	return 1;
}

// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
void DISK_IMAGE::debug_media_info(_TCHAR *buffer, size_t buffer_len)
{
}
#endif /* USE_DEBUGGER */
