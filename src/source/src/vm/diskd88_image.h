/** @file diskd88_image.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2025.08.10-

	@brief [ disk d88 image ]
*/

#ifndef DISKD88_IMAGE_H
#define DISKD88_IMAGE_H

#include "../common.h"
#include "../diskd88_defs.h"
#include "disk_image.h"
#include "disk.h"
#ifdef USE_CPTRLIST_SECTOR
#include "../cptrlist.h"
#endif
//#include "disk_parser.h"
//#include <stdlib.h>
#ifdef _DEBUG
//#define _DEBUG_D88_IMAGE
#endif

class FILEIO;
class DISK;
class DISKD88_IMAGE;

/**
	@brief current sector
*/
class DISKD88_SECTOR : public DISK_SECTOR
{
private:
	uint8_t* p_sector_id;
	uint8_t* p_sector_data;
	int m_sector_size;
	uint8_t m_id[6];
	uint8_t m_density;
	uint8_t m_deleted;
	uint8_t m_status;		///< status register 1 of FDC(uPD765)
	uint16_t m_raw_pos;

public:
	DISKD88_SECTOR();
	~DISKD88_SECTOR();

	void clear();
	void set(uint8_t *t);

	void set_sector_data(int pos, uint8_t val);
	uint8_t get_sector_data(int pos) const;
	int get_sector_size() const;
	void set_sector_id(int pos, uint8_t val);
	uint8_t get_sector_id(int pos) const;
	void set_single_density(bool val);
	bool is_single_density() const;

	void set_raw_pos(uint16_t val);
	uint16_t get_raw_pos() const;

	void set_deleted_mark(bool val);
	bool has_deleted_mark() const;
	bool has_crc_error(int density) const;

	bool compare_crc_on_data(int size) const;
};

/**
	@brief current track
*/
class DISKD88_TRACK : public DISK_TRACK
{
private:
	enum {
		MAX_SECTORS = 48,
		MAX_SECTOR_NUMS = 256,
		TRACK_BUFFER_SIZE =	0x40000		// 256KB
	};
	DISKD88_IMAGE *p_parent;
	uint8_t *p_track;						  ///< top position of buffer on current track
	int m_track_num;						  ///< position in the disk
	int m_side_num;							  ///< head position
	int m_track_table_pos;					  ///< position of track table
	int m_sector_nums;						  ///< number of sector
//	uint8_t m_format_type;					  ///< set by host (bit0:density, bit1:hiden, bit2:rpm)

	bool m_valid_sector_pos;
//	uint8_t m_id_c_in_track[MAX_SECTOR_NUMS]; ///< C number in current track
//	uint8_t m_sector_pos[MAX_SECTOR_NUMS];	  ///< sector position in current track
#ifndef USE_SECTOR_LIST
	uint16_t m_sector_raw_pos[MAX_SECTORS];
#else
#ifdef USE_CPTRLIST_SECTOR
	CPtrList<DISKD88_SECTOR> m_sectors;		  ///< sector information
#else
	DISKD88_SECTOR m_sectors[MAX_SECTORS];	  ///< sector information
	int m_sectors_count;					  ///< valid count of sector information
#endif
#endif
	int m_sector_all_size;					  ///< calculated raw size in bytes

	// for raw track data

	uint8_t m_track_buffer[TRACK_BUFFER_SIZE];
	int m_track_size;

public:
	DISKD88_TRACK();
	~DISKD88_TRACK();

	void initialize();
	void clear();
	void set(uint8_t *t, int track_num, int side_num, int track_pos, int sector_nums, int density);
	void set_parent(DISKD88_IMAGE *parent);
	uint8_t *get_buffer();
	int get_track_num() const;
#ifdef USE_SECTOR_LIST
	DISKD88_SECTOR *get_sector_by_pos(int sector_pos);
#endif
	bool verify_track_number(int track, int density, int hiden, int rps);

	uint8_t get_id_c_in_track(int sector_pos) const;
	int get_sector_nums() const;
	uint8_t get_sector_pos(int sector_num) const;
	uint8_t get_sector_pos(int sector_num, int start_pos) const;
	uint8_t get_sector_pos(int sector_num, double curr_ratio) const;
	double get_sector_ratio_from_pos(int sector_pos) const;
	double get_sector_ratio(int sector_num) const;
	int get_sector_pos_from_curr_ratio(double curr_ratio) const;
	int get_sector_pos_from_ratio(double &ratio) const;

	bool parse_sector(int density, int hiden, int rps);

	bool make_track(int density, int hiden, int rps);
	bool parse_track2(int density, int hiden, int rps);

	void set_raw_track_data(int pos, uint8_t val);
	uint8_t get_raw_track_data(int pos) const;
	void set_raw_track_size(int val);
	int get_raw_track_size() const;

#ifdef USE_DEBUGGER
	void debug_media_info(_TCHAR *buffer, size_t buffer_len);
#endif /* USE_DEBUGGER */
};

/**
	@brief disk image
*/
class DISKD88_IMAGE : public DISK_IMAGE
{
private:
//	uint8_t m_media_type;
	uint8_t *p_buffer;

	int m_number_of_side;
	int m_current_side;

	DISKD88_TRACK  m_current_track[2];
#ifdef USE_SECTOR_LIST
	DISKD88_SECTOR *p_current_sector;
#else
	DISKD88_SECTOR m_current_sector;
#endif

	bool check_media_type();

	int get_track_offset(int track, int side, int *track_pos);
	uint8_t* get_track_buffer(int track, int side, int *track_pos);

	void shrink_track_space(int start, int size);
	void expand_track_space(int start, int size);
	int find_track_side(int trkside, int *offset);
	void recalc_track_data_table(int start, int offset);
	void trim_track_data_table();
	int calc_track_size(uint8_t *t);
	void add_file_size(int offset);

	int get_current_track_buffer(uint8_t * &t);
	int set_current_sector(uint8_t *t, int sector_pos, uint8_t density);
	int get_clock_from_sector_pos(int sector_pos, const DRIVE_TYPE *drive_type, int remain_clock, int timeout_round);

	DISKD88_IMAGE();
	DISKD88_IMAGE(const DISKD88_IMAGE &);

public:
	DISKD88_IMAGE(DISK *disk, uint8_t *buffer);
	virtual ~DISKD88_IMAGE();

	void clear();
	void set(DISK *disk, uint8_t *buffer);
	uint8_t *get_buffer() { return p_buffer; }

	bool is_write_protected() const;
	void set_write_protect(bool val);
	bool is_write_supported() const;
	uint8_t get_media_type() const;

	bool write_file(FILEIO *fio, bool new_file, bool is_plain, bool is_last_volume);

	//
	// disk
	//
	bool check_tracks();
	int get_number_of_side() const;
	void set_side_position(int side_pos);

	//
	// track (use on reading/writing a track operation)
	//
	int get_current_track_number() const;
	DISK_TRACK *get_current_track();
	int get_number_of_sector() const;
	int get_current_track_size() const;
	bool search_track(int track_pos, int side_pos, int density);
	bool verify_track_number(int track_num, int side_num, int density);
	bool make_track(int side_num, int density);
	bool parse_track(int side_num, int density);
	uint8_t *replace_track(int trksid, const uint8_t *tmp_track, int tmp_track_size);

    //
	// sector
    //
	int get_current_sector_pos(int sector) const;
	DISK_SECTOR *get_current_sector();
	int get_current_sector_size() const;
	void parse_sector(int side_num, int density);
	int  search_sector(int side_num, int density, int sector_num, int &sector_pos);
	int  search_sector_and_get_clock(int side_num, int density, int sector_num, int &sector_pos, int remain_clock, int timeout_round, int &arrive_clock);

	int get_clock_arrival_sector(int sector_num, int sector_pos, int remain_clock, int timeout_round);

#ifdef USE_DEBUGGER
	void debug_media_info(_TCHAR *buffer, size_t buffer_len);
#endif /* USE_DEBUGGER */
};

#endif /* DISKD88_IMAGE_H */

