/** @file disk.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2025.08.10 -

	@note Original author is Takeda.Toshiya at 2006.09.16-

	@brief [ disk image handler ]
*/

#include "disk.h"
#include "../emu.h"
#include "../fileio.h"
#include "../utility.h"
#include "../config.h"
#include "../depend.h"
#include "../labels.h"
#include "disk_image.h"
#include "diskd88_image.h"
#include "diskhfe_image.h"
#include "disk_parser.h"
#include "device.h"
#include "floppy_defs.h"

#define IS_INSERTED (p_image != NULL)

extern EMU *emu;

#if defined(_DEBUG_DISK)
#define OUT_DEBUG logging->out_debugf
#else
#define OUT_DEBUG(...)
#endif

// ----------------------------------------------------------------------------

DRIVE_TYPE::DRIVE_TYPE()
{
	clear();
}

DRIVE_TYPE::~DRIVE_TYPE()
{
}

void DRIVE_TYPE::clear()
{
	m_drive_type = DRIVE_TYPE_UNK;
	m_hiden = 4;		// normal density
	m_rps = 5;			// round per sec.
	m_round_clock = (CPU_CLOCKS / m_rps);
}

void DRIVE_TYPE::set(en_drive_types val, int round_clock)
{
	m_drive_type = val;
	m_round_clock = round_clock;

	switch(m_drive_type) {
	case DRIVE_TYPE_2HD_360:
		m_hiden = 2;
		m_rps = 6;
		break;
	case DRIVE_TYPE_2HD_300:
		m_hiden = 2;
		m_rps = 5;
		break;
	default:
		m_hiden = 4;
		m_rps = 5;
		break;
	}
}

// ----------------------------------------------------------------------------

DISK::DISK()
{
	p_fdc = NULL;
	m_drive_num = -1;
	initialize();
}

/// @param[in] fdc : floppy disk controller
/// @param[in] drv : drive number
DISK::DISK(DEVICE *fdc, int drv)
{
	p_fdc = fdc;
	m_drive_num = drv;
	initialize();
}

DISK::~DISK()
{
	close();
}

/// @brief Initialize valiables
void DISK::initialize()
{
	m_ejected = m_changed = false;
	p_buffer = NULL;
	m_buffer_size = 0;
	p_image = NULL;
	clear();
}

/// @brief Clear valiables
void DISK::clear()
{
	m_image_type = IMAGE_TYPE_UNK;
//	m_inserted = false;
	m_write_protected = false;
	m_file_size = m_file_size_orig = 0;
	m_file_offset = 0;
	m_last_volume = false;
	m_filename_changed = false;
}

/// @brief Set drive type
///
/// @param[in] val : DISK::en_drive_types
/// @param[in] round_clock : one round clock (CPU_CLOCK * 0.2sec) 
void DISK::set_drive_type(DRIVE_TYPE::en_drive_types val, int round_clock)
{
	m_drive_type.set(val, round_clock);
}

/// @brief calculate CRC32
///
/// @param[in] data
/// @param[in] size : data size
/// @return CRC32
uint32_t DISK::getcrc32(uint8_t data[], int size)
{
	uint32_t c, table[256];
	for(int i = 0; i < 256; i++) {
		uint32_t c = i;
		for(int j = 0; j < 8; j++) {
			if(c & 1) {
				c = (c >> 1) ^ 0xedb88320;
			}
			else {
				c >>= 1;
			}
		}
		table[i] = c;
	}
	c = ~0;
	for(int i = 0; i < size; i++) {
		c = table[(c ^ data[i]) & 0xff] ^ (c >> 8);
	}
	return ~c;
}

/// @brief Open a floppy disk image
///
/// @param[in] path : file path
/// @param[in] offset : start position of file
/// @param[in] flags : bit0: 1 = read only  bit3: 1= last volume  bit4: 1 = multi volumed file
/// @return true if success
bool DISK::open(const _TCHAR *path, int offset, uint32_t flags)
{
	// check current disk image
	if(IS_INSERTED) {
#ifdef _WIN32
		if(_tcsicmp(m_file_path, path) == 0 && m_file_offset == offset) {
#else
		if(_tcscmp(m_file_path, path) == 0 && m_file_offset == offset) {
#endif
			return true;
		}
		close();
	}

	// open disk image
	FILEIO fio;
	do {
		DISK_PARSER ps(this, &fio, path);
		if(!fio.Fopen(path, FILEIO::READ_BINARY)) {
			break;
		}

		UTILITY::tcscpy(m_file_path, sizeof(m_file_path) / sizeof(m_file_path[0]), path);

		// check file size
		m_file_size = fio.FileLength();
		if (m_file_size == 0) {
			break;
		}

		// check if file protected
		m_write_protected = fio.IsFileProtected(path) | ((flags & OPEN_DISK_FLAGS_READ_ONLY) != 0);

		// is last volume ?
		m_last_volume = ((flags & OPEN_DISK_FLAGS_LAST_VOLUME) != 0);

		// allocate buffer
		allocate_buffer(m_file_size);

		ps.set_buffer(p_buffer, m_buffer_size); 

		// check image file format
		if (ps.parse(offset, m_file_size_orig, m_file_offset)) {
	//		m_inserted true;
			m_changed = true;

			// attach disk image
			attach_disk_image();
		}
	} while(0);

	fio.Fclose();

	if (p_image) {
		m_crc32 = getcrc32(p_buffer, m_file_size);

		m_write_protected |= p_image->is_write_protected();
		if (!p_image->is_write_supported()) {
			m_write_protected = true;
		}

		p_image->check_tracks();
	}

	return IS_INSERTED;
}

/// @brief Close the floppy disk image
void DISK::close()
{
	// write disk image
	if(IS_INSERTED) {
		if (!m_write_protected) {
			flush(m_write_protected);
		}
		m_ejected = true;

		detach_disk_image();
	}

	release_buffer();

	clear();
}

/// @brief Allocate buffer to store disk image
///
/// @param[in] buffer_size
void DISK::allocate_buffer(int buffer_size)
{
	if (p_buffer) return;

	if (buffer_size < DISK_BUFFER_SIZE) {
		buffer_size = DISK_BUFFER_SIZE;
	} else {
		buffer_size += 0x10000;
		buffer_size &= (~0xffff);
	}
	m_buffer_size = buffer_size;
	p_buffer = new uint8_t[m_buffer_size];
}

/// @brief Release buffer
void DISK::release_buffer()
{
	delete [] p_buffer;
	p_buffer = NULL;
	m_buffer_size = 0;
}

/// @brief Attach interface to access the disk image
void DISK::attach_disk_image()
{
	detach_disk_image();

	switch(m_image_type) {
	case IMAGE_TYPE_D88:
	case IMAGE_TYPE_D88_CONVERTED:
	case IMAGE_TYPE_PLAIN:
		p_image = new DISKD88_IMAGE(this, p_buffer);
		break;
	case IMAGE_TYPE_HFE:
		p_image = new DISKHFE_IMAGE(this, p_buffer);
		break;
	default:
		break;
	}
}

/// @brief Detach interface
void DISK::detach_disk_image()
{
	delete p_image;
	p_image = NULL;
}

/// @brief Flush data and write to image file 
void DISK::flush()
{
	// write disk image
	if(IS_INSERTED) {
		if (!m_write_protected) {
			flush(m_write_protected);
		}
	}
}

/// @brief Flush data and write to image file
///
/// @param[in] protect : whether write protection or not
/// @return true if success
bool DISK::flush(bool protect)
{
	if (!p_image) return false;

	// set write protect flag
	p_image->set_write_protect(protect);

	// flush disk image
	bool wrote = true;
	if(m_file_size && getcrc32(p_buffer, m_file_size) != m_crc32) {
		// writable ?
		if (!p_image->is_write_supported()) {
			logging->out_logf_x(LOG_WARN, CMsg::Its_not_supported_to_write_to_the_floppy_image_on_drive_VDIGIT, m_drive_num);
			return false;
		}
		// write image
		bool save_as_plain = false;
		switch(m_image_type) {
		case IMAGE_TYPE_PLAIN:
			save_as_plain = (FLG_SAVE_FDPLAIN != 0);
//			[: through :]
		case IMAGE_TYPE_D88_CONVERTED:
			if (!m_filename_changed && !save_as_plain) {
				// file name is converted d88 extension
				rename_file();
				m_filename_changed = true;
				m_image_type = IMAGE_TYPE_D88_CONVERTED;
			}
			break;
		default:
			break;
		}
		wrote = write_file(m_filename_changed, save_as_plain);
		if (wrote) {
			if (m_filename_changed) {
				// set file path to recent list on menu
				emu->update_floppy_disk_info(m_drive_num, m_file_path, 0);
			}
		} else {
			logging->out_logf_x(LOG_ERROR, CMsg::Floppy_image_on_drive_VDIGIT_couldn_t_be_saved, m_drive_num);
		}
	}
	return wrote;
}

/// @brief Flush disk image to file before set write protect
///
/// @param[in] val : true if set the protection
void DISK::set_write_protect(bool val)
{
	if (val == false) {
		FILEIO fio;
		if(fio.Fopen(m_file_path, FILEIO::READ_BINARY)) {
			val = fio.IsFileProtected(m_file_path);
			fio.Fclose();
		} else {
			val = true;
		}
		if (p_image) {
			// support writing operation?
			if (!p_image->is_write_supported()) {
				logging->out_logf_x(LOG_WARN, CMsg::Its_not_supported_to_write_to_the_floppy_image_on_drive_VDIGIT, m_drive_num);
				val = true;
			}
		}
	}
	if (val == true && m_write_protected == false) {
		// when set write protect, flush disk image to file.
		flush(val);
	}
	m_write_protected = val;
}

/// @brief Inserted the disk?
///
/// @return true if inserted 
bool DISK::is_inserted() const
{
	return IS_INSERTED;
}

/// @brief Rename a file name to new name
///
/// New name is "PREFIX_YYYY-MM-DD_HH-MI-SS.d88"
void DISK::rename_file()
{
//	int tim[8];
	_TCHAR name[_MAX_PATH];

	UTILITY::get_dir_and_basename(m_file_path, name, 9);
	if (_tcslen(name) > 0) {
		UTILITY::tcscat(name, _MAX_PATH, _T("_"));
	}

	size_t len = _tcslen(name);
	UTILITY::create_date_file_path(NULL, &name[len], _MAX_PATH - len, LABELS::blank_floppy_disk_exts); 

	UTILITY::tcscat(m_file_path, sizeof(m_file_path) / sizeof(m_file_path[0]), name);

	m_file_offset = 0;
	m_file_size_orig = m_file_size;

	logging->out_logf_x(LOG_INFO, CMsg::Floppy_image_on_drive_VDIGIT_is_saved_as_the_new_file_VSTR, m_drive_num, name);
	logging->out_logf_x(LOG_DEBUG, CMsg::Save_to_VSTR, m_file_path);
}

/// @brief Write a disk image
///
/// @param[in] new_file : save as new file
/// @param[in] is_plain : save as plain image
/// @return true if success
bool DISK::write_file(bool new_file, bool is_plain)
{
	if (!p_image) {
		return false;
	}
	if (!p_image->is_write_supported()) {
		return false;
	}

	FILEIO fio;
	if(!fio.Fopen(m_file_path, new_file ? FILEIO::WRITE_BINARY : FILEIO::READ_WRITE_BINARY)) {
		return false;
	}

	bool rc = p_image->write_file(&fio, new_file, is_plain, m_last_volume);

	fio.Fclose();
	return rc;
}

/// @brief Is the same file as already opened one?
///
/// @param[in] path : file path
/// @param[in] offset : position of the file data in bytes
/// @return true if the same file
bool DISK::is_same_file(const _TCHAR *path, int offset)
{
	if (!IS_INSERTED) return false;

#ifdef _WIN32
	if(_tcsicmp(m_file_path, path) == 0 && m_file_offset == offset) {
#else
	if(_tcscmp(m_file_path, path) == 0 && m_file_offset == offset) {
#endif
		return true;
	} else {
		return false;
	}
}

// ----------------------------------------------------------------------------

/// @brief Get number of side
///
/// @return number of side
int DISK::get_number_of_side() const
{
	if (!p_image) return 2;
	return p_image->get_number_of_side();
}

/// @brief Set side position
///
/// @param[in] side_pos : side
void DISK::set_side_position(int side_pos)
{
	if (!p_image) return;
	p_image->set_side_position(side_pos);
}

// ----------------------------------------------------------------------------

/// @brief Get current track number
///
/// @return track number
/// @note need call search_track() before invoke this
int DISK::get_current_track_number() const
{
	if (!p_image) return 0;
	return p_image->get_current_track_number();
}

/// @brief Get current track
///
/// @return track info
/// @note need call search_track() before invoke this
DISK_TRACK *DISK::get_current_track()
{
	if (!p_image) return 0;
	return p_image->get_current_track();
}

/// @brief Get number of sector in current track
///
/// @param[in] base : base number
/// @return number of sector
/// @note need call search_track() before invoke this
int DISK::get_number_of_sector(int base) const
{
	if (!p_image) return base;
	int nums = p_image->get_number_of_sector();
	if (nums < base) nums = base;
	return nums;
}

/// @brief Get raw track size on current track
///
/// @return size
/// @note need call search_track() before invoke this
int DISK::get_current_track_size() const
{
	if (!p_image) return 0;
	return p_image->get_current_track_size();
}

// ----------------------------------------------------------------------------

/// @brief Get current sector position
///
/// @return sector position
/// @note need call parse_sector() before invoke this
int DISK::get_current_sector_pos(int sector) const
{
	if (!p_image) return 0;
	return p_image->get_current_sector_pos(sector);
}

/// @brief Get current sector
///
/// @return sector info
/// @note need call parse_sector() before invoke this
DISK_SECTOR *DISK::get_current_sector()
{
	if (!p_image) return NULL;
	return p_image->get_current_sector();
}

/// @brief Get data size of current sector
///
/// @return sector size
/// @note need call parse_sector() before invoke this
int DISK::get_current_sector_size() const
{
	if (!p_image) return 0;
	return p_image->get_current_sector_size();
}

/// @brief Set deleted mark
///
/// @param[in] val
void DISK::set_deleted_mark(bool val)
{
	DISK_SECTOR *current_sector = get_current_sector();
	if(current_sector) {
		current_sector->set_deleted_mark(val);
	}
}

/// @brief Does detect deleted mark?
///
/// @return true if deleted mark detected.
bool DISK::has_deleted_mark()
{
	DISK_SECTOR *current_sector = get_current_sector();
	return (current_sector ? current_sector->has_deleted_mark() : false);
}

/// @brief Calcurate the time (clocks) to reach at the sepcified sector
///
/// @param[in] sector_num : sector number of the sepcified sector or -1 if spcify sector position
/// @param[in] sector_pos : sector position 
/// @param[in] remain_clock : clock until reach index hole 
/// @param[in] timeout_round : number of round to occur timeout 
/// @return clocks
int DISK::get_clock_arrival_sector(int sector_num, int sector_pos, int remain_clock, int timeout_round)
{
	if (!p_image) return m_drive_type.m_round_clock;
	return p_image->get_clock_arrival_sector(sector_num, sector_pos, remain_clock, timeout_round);
}
