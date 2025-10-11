/** @file disk.h

	Skelton for retropc emulator

	@author Takeda.Toshiya
	@date   2006.09.16-

	@brief [ disk image handler ]
*/

#ifndef DISK_H
#define DISK_H

#include "../common.h"
//#include "../d88_defs.h"
//#include "disk_image.h"
//#include "disk_parser.h"
//#include <stdlib.h>

enum en_disk_constant {
	// disk image size
	DISK_BUFFER_SIZE = 0x180000, // 1.5MB
	// track size
	// preamble 146 + 16sectors 5936 + postamble (598)
	TRACK_SIZE_5INCH_2D	= 0x1800,
	// preamble 146 + 26sectors 9646 + postamble (598)
	TRACK_SIZE_8INCH_2D	= 0x2700,
};


class DEVICE;
class FILEIO;
class DISK_IMAGE;

/**
	@brief disk sector handler
*/
class DISK_SECTOR
{
public:
	DISK_SECTOR() {}
	virtual ~DISK_SECTOR() {}

	virtual void clear() = 0;
	virtual void set_sector_data(int pos, uint8_t val) {}
	virtual uint8_t get_sector_data(int pos) const { return 0; }
	virtual int get_sector_size() const { return 0; }
	virtual void set_sector_id(int pos, uint8_t val) {}
	virtual uint8_t get_sector_id(int pos) const { return 0; }

	virtual void set_deleted_mark(bool val) {}
	virtual bool has_deleted_mark() const { return false; }
	virtual bool has_crc_error(int density) const { return false; }

	virtual bool compare_crc16_on_data(int size) const { return false; }
};

/**
	@brief disk track handler
*/
class DISK_TRACK
{
public:
	DISK_TRACK() {}
	virtual ~DISK_TRACK() {}

	virtual void clear() = 0;
	virtual int get_sector_nums() const { return 0; }

	virtual void set_raw_track_data(int pos, uint8_t val) {}
	virtual uint8_t get_raw_track_data(int pos) const { return 0; }
	virtual void set_raw_track_size(int val) {}
	virtual int get_raw_track_size() const { return 0; }
};

/**
	@brief disk drive type
*/
class DRIVE_TYPE
{
public:
	enum en_drive_types {
		DRIVE_TYPE_2D		= 0x00,
		DRIVE_TYPE_2DD		= 0x10,
		DRIVE_TYPE_2HD_300	= 0x20,
		DRIVE_TYPE_2HD_360	= 0x30,
		DRIVE_TYPE_UNK		= 0xff
	};

	en_drive_types m_drive_type;
	int m_round_clock;		///< one round clock (CPU_CLOCK * m_rps) 
	uint8_t m_hiden;		///< 4:normal density, 2:high density 
	uint8_t m_rps;			///< round per sec. 5(300rpm) or 6(360rpm)

public:
	DRIVE_TYPE();
	~DRIVE_TYPE();
	void clear();
	void set(en_drive_types val, int round_clock);
};

/**
	@brief disk image handler
*/
class DISK
{
public:
	enum en_image_types {
		IMAGE_TYPE_D88 = 0,
		IMAGE_TYPE_D88_CONVERTED,
		IMAGE_TYPE_PLAIN,
		IMAGE_TYPE_HFE,
		IMAGE_TYPE_UNK = 0xff
	};
	enum en_delay_times {
		DELAY_WRITE_FRAME	= 300,
	};

protected:
	DEVICE *p_fdc;
	int  m_drive_num;
	DISK_IMAGE *p_image;
	en_image_types m_image_type;
	DRIVE_TYPE m_drive_type;

	uint8_t *p_buffer;
	int  m_buffer_size;

	_TCHAR m_file_path[_MAX_PATH];
	int  m_file_size, m_file_size_orig;
	int  m_file_offset;
	uint32_t m_crc32;
	bool m_last_volume;
	bool m_filename_changed;

	bool m_ejected;
	bool m_write_protected;
	bool m_changed;

	void initialize();
	void clear();
	bool flush(bool protect);
	void rename_file();
	bool write_file(bool new_file, bool is_plain);

	static uint32_t getcrc32(uint8_t data[], int size);
	void allocate_buffer(int buffer_size);
	void release_buffer();
	void attach_disk_image();
	void detach_disk_image();

	DISK();

public:
	DISK(DEVICE *fdc, int drv);
	virtual ~DISK();

	virtual bool open(const _TCHAR *path, int offset, uint32_t flags);
	virtual void close();
	void flush();

	void set_write_protect(bool val);
	bool is_write_protected() const { return m_write_protected; }
	bool is_ejected() const { return m_ejected; }
	bool is_inserted() const;
	bool is_changed() const { return m_changed; }

	bool is_same_file(const _TCHAR *path, int offset);

	void set_image_type(en_image_types val) { m_image_type = val; }
	en_image_types get_image_type() const { return m_image_type; }
	void set_drive_type(DRIVE_TYPE::en_drive_types val, int round_clock);
	const DRIVE_TYPE *get_drive_type() const { return &m_drive_type; }

	void set_buffer_size(int val) { m_buffer_size = val; }
	int get_buffer_size() const { return m_buffer_size; }

	void set_file_size(int val) { m_file_size = val; }
	int get_file_size() const { return m_file_size; }
	int get_file_size_orig() const { return m_file_size_orig; }
	int get_file_offset() const { return m_file_offset; }

	//
	// disk
	//
	int get_number_of_side() const;
	virtual void set_side_position(int side_pos);

	//
	// track (use on reading/writing a track operation)
	//
	int get_current_track_number() const;
	DISK_TRACK *get_current_track();
	int get_number_of_sector(int base = 0) const;
	int get_current_track_size() const;

	//
	// sector
	//
	int get_current_sector_pos(int sector) const;
	DISK_SECTOR *get_current_sector();
	int get_current_sector_size() const;
	enum en_result_search_sector {
		RES_NO_ERROR			 = 0,
		RES_SECTOR_NOT_FOUND	 = 1,
		RES_DISK_NOT_INSERTED	 = 2,
		RES_INVALID_MEDIA_TYPE	 = 3,
		RES_UNMATCH_DENSITY		 = 4,
	};
	void set_deleted_mark(bool val);
	bool has_deleted_mark();

	int get_clock_arrival_sector(int sector_num, int sector_pos, int remain_clock, int timeout_round);
};

#endif /* DISK_H */

