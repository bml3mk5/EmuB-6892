/** @file diskhfe_image.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2025.08.10-

	@brief [ disk hfe image ]
*/

#ifndef DISKHFE_IMAGE_H
#define DISKHFE_IMAGE_H

#include "../common.h"
#include "disk_image.h"
#include "disk.h"
#include "../diskhfe_defs.h"
#ifdef USE_CPTRLIST_SECTOR
#include "../cptrlist.h"
#endif
//#include "disk_parser.h"
//#include <stdlib.h>

class FILEIO;
class DISK;
class DISKHFE_IMAGE;

/**
	@brief current sector
*/
class DISKHFE_SECTOR : public DISK_SECTOR
{
private:
	uint8_t* p_sector_id;
	uint8_t* p_sector_data;
	int m_sector_size;
	uint8_t m_id[6];
	bool m_deleted;

public:
	DISKHFE_SECTOR();
	~DISKHFE_SECTOR();

	void clear();
	void set_id_ptr(uint8_t *p);
	void set_data_ptr(uint8_t *p, bool deleted);
	const uint8_t *get_data_ptr() const;

	void set_sector_data(int pos, uint8_t val);
	uint8_t get_sector_data(int pos) const;
	int get_sector_size() const;
	void set_sector_id(int pos, uint8_t val);
	uint8_t get_sector_id(int pos) const;
//	void set_single_density(bool val);
//	bool is_single_density() const;

	void set_deleted_mark(bool val);
	bool has_deleted_mark() const;
	bool has_crc_error(int density) const;

	bool compare_crc_on_data(int density, int size) const;

#ifdef USE_DEBUGGER
	void debug_media_info(int sector_pos, const uint8_t *p_data, _TCHAR *buffer, size_t buffer_len);
#endif /* USE_DEBUGGER */
};

/**
	@brief track
*/
class DISKHFE_TRACK_INFO
{
public:
	enum {
		MAX_SECTORS = 48,
		MAX_SECTOR_NUMS = 256,
		TRACK_BUFFER_SIZE = 0x6200
	};
	bool	m_valid;					  ///< decoded?
	uint8_t m_density;					  ///< bit0: 1 means double density 
	int     m_size;						  ///< decoded size
	uint8_t m_data[TRACK_BUFFER_SIZE];	  ///< decoded data

	bool    m_sector_valid;					///< sector valid
//	uint8_t m_sector_pos[MAX_SECTOR_NUMS];	///< sector position in current track
#ifdef USE_CPTRLIST_SECTOR
	CPtrList<DISKHFE_SECTOR> m_sectors;		///< sector information
#else
	DISKHFE_SECTOR m_sectors[MAX_SECTORS];	///< sector information
	int m_sectors_count;					///< valid count of sector information
#endif

	DISKHFE_TRACK_INFO();
	~DISKHFE_TRACK_INFO();
	void clear();
	void clear_sector_table();
	void make_sector_table();
	bool parse_sector();
	uint8_t get_id_c(int pos) const;
	bool verify_track_number(int track_num) const;

	uint8_t get_sector_pos(int sector_num) const;
	uint8_t get_sector_pos(int sector_num, int start_pos) const;
	uint8_t get_sector_pos(int sector_num, double curr_ratio) const;

	double get_sector_ratio_from_pos(int sector_pos) const;
	double get_sector_ratio(int sector) const;
	int get_sector_pos_from_curr_ratio(double curr_ratio) const;
	int get_sector_pos_from_ratio(double ratio) const;

#ifdef USE_DEBUGGER
	void debug_media_info(int track_pos, int side_pos, _TCHAR *buffer, size_t buffer_len);
#endif /* USE_DEBUGGER */
};

/**
	@brief current track
*/
class DISKHFE_TRACK : public DISK_TRACK
{
private:
	DISKHFE_IMAGE *p_parent;
	uint8_t *p_track;						  ///< top position of buffer on current track
	int m_track_pos;						  ///< position of track table
	int m_track_size;						  ///< position of track size
	int m_current_side;						  ///< current side
//	int m_sector_nums;						  ///< number of sector
//	uint8_t m_id_c_in_track[MAX_SECTOR_NUMS]; ///< C number in current track

	DISKHFE_TRACK_INFO m_info[2];

	// for raw track data

//	uint8_t m_track_buffer[TRACK_BUFFER_SIZE];

	int shift_bytes(uint8_t *data, int len, int shift_cnt);
	int shift_bits(uint8_t *data, int len, int shift_cnt);

	uint8_t decode_mfm_data(const uint8_t *indata);
	uint8_t decode_fm_data(const uint8_t *indata);

	bool make_track_from_bitstream(int side, int density);

	void copy_bitstream(uint8_t *dst, const uint8_t *src, int pos, int len);
	void decode_bitstream(DISKHFE_TRACK_INFO *info, int idata_pos, int idata_sft, int end_idata_pos, int len);

public:
	DISKHFE_TRACK();
	~DISKHFE_TRACK();

	void clear();
	bool set(uint8_t *t, int track_pos, int track_size);
	void set_parent(DISKHFE_IMAGE *parent);
	uint8_t *get_buffer();

	bool decode_track(int side, int density);

	void parse_sector(int side, int density);
	DISKHFE_SECTOR *get_sector_by_pos(int sector_pos);

	int get_sector_nums() const;
	bool verify_track_number(int track_num) const;
	uint8_t get_id_c_in_track(int sector_pos) const;
	uint8_t get_sector_pos(int sector_num) const;
	uint8_t get_sector_pos(int sector_num, int start_pos) const;
	uint8_t get_sector_pos(int sector_num, double curr_ratio) const;

	double get_sector_ratio_from_pos(int sector_pos) const;
	double get_sector_ratio(int sector_num) const;
	int get_sector_pos_from_ratio(double ratio) const;

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
class DISKHFE_IMAGE : public DISK_IMAGE
{
private:
	uint8_t *p_buffer;
	hfe_track_offset_list_t *p_track_list;

	int m_number_of_side;

	DISKHFE_TRACK  m_current_track;
	DISKHFE_SECTOR *p_current_sector;


	bool check_media_type();

	uint8_t* get_track_buffer(int trk, int &size);

	int has_current_track_buffer();
	int set_current_sector(int sector_pos);
	int get_clock_from_sector_pos(int sector_pos, const DRIVE_TYPE *drive_type, int remain_clock, int timeout_round);

	DISKHFE_IMAGE();
	DISKHFE_IMAGE(const DISKHFE_IMAGE &);

public:
	DISKHFE_IMAGE(DISK *disk, uint8_t *buffer);
	virtual ~DISKHFE_IMAGE();

	void clear();
	void set(DISK *disk, uint8_t *buffer);
	uint8_t *get_buffer() { return p_buffer; }

	bool is_write_protected() const;
	void set_write_protect(bool val);
	bool is_write_supported() const;
	uint8_t get_media_type() const;

	uint8_t get_interface_mode() const;
	uint8_t get_encoding(int num) const;

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

#endif /* DISKHFE_IMAGE_H */

