/** @file disk_image.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2025.08.10-

	@brief [ disk image ]
*/

#ifndef DISK_IMAGE_H
#define DISK_IMAGE_H

#include "../common.h"

// use crc calculate function
#define USE_CALC_CRC16

#define CRC16_INIT_DATA 0xffff

class DISK;
class DISK_SECTOR;
class DISK_TRACK;
class FILEIO;

/// gap sign
extern uint8_t c_gap_sign[2];
/// 0:single density  1:double density
extern int c_mark_length[2];
/// index mark
extern uint8_t c_index_mark[2][17];
/// address mark
extern uint8_t c_address_mark[2][17];
/// data mark
extern uint8_t c_data_mark[2][17];
/// deleted data mark
extern uint8_t c_deleted_mark[2][17];

/// IBM format length table
struct st_ibm_format_length {
	uint8_t sync;
	// mark length
	uint8_t mark;
	uint8_t gap0;
	// after index mark
	uint8_t gap1;
	// after address mark
	uint8_t gap2;
	// after data mark
	uint8_t gap3;
};
/// IBM format length table
extern struct st_ibm_format_length c_ibm_format_length[2];

/**
	@brief disk image
*/
class DISK_IMAGE
{
protected:
	DISK *p_disk;

	void clear_base();
	void set_base(DISK *disk);

	DISK_IMAGE();
	DISK_IMAGE(const DISK_IMAGE &src);

public:
	DISK_IMAGE(DISK *disk);
	virtual ~DISK_IMAGE();

	virtual void clear() = 0;
	virtual void set(DISK *disk, uint8_t *buffer) = 0;

	virtual bool is_write_protected() const;
	virtual void set_write_protect(bool val);

	virtual bool is_write_supported() const;

	virtual uint8_t get_media_type() const;

	virtual bool write_file(FILEIO *fio, bool new_file, bool is_plain, bool is_last_volume);

	static uint16_t set_crc16(uint8_t data, uint16_t crc);
#ifdef USE_CALC_CRC16
	static uint16_t calc_crc16(uint8_t data, uint16_t crc);
#endif

	//
	// disk
	//
	virtual bool check_tracks();
	virtual int get_number_of_side() const;
	virtual void set_side_position(int side_pos);

	//
	// track (use on reading/writing a track operation)
	//
	virtual int get_current_track_number() const;
	virtual DISK_TRACK *get_current_track();
	virtual int get_number_of_sector() const;
	virtual int get_current_track_size() const;
	virtual bool search_track(int track_pos, int side_pos, int density);
	virtual bool verify_track_number(int track_num, int side_num, int density);
	virtual bool make_track(int side_num, int density);
	virtual bool parse_track(int side_num, int density);

	//
	// sector
    //
	virtual int get_current_sector_pos(int sector) const;
	virtual DISK_SECTOR *get_current_sector();
	virtual int get_current_sector_size() const;
	virtual void parse_sector(int side_num, int density);
	virtual int  search_sector(int side_num, int density, int sector_num, int &sector_pos);
	virtual int  search_sector_and_get_clock(int side_num, int density, int sector_num, int &sector_pos, int remain_clock, int timeout_round, int &arrive_clock);

	virtual int get_clock_arrival_sector(int sector_num, int sector_pos, int remain_clock, int timeout_round);

#ifdef USE_DEBUGGER
	virtual void debug_media_info(_TCHAR *buffer, size_t buffer_len);
#endif /* USE_DEBUGGER */
};

#endif /* DISK_IMAGE_H */

