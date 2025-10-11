/** @file diskd88_image.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2025.08.10-

	@brief [ disk d88 image ]
*/

#include "diskd88_image.h"
#include "../emu.h"
#include "../fileio.h"
#include "../utility.h"
#include "../config.h"
#include "../depend.h"
#include "../labels.h"
#include "disk_parser.h"

#ifdef _DEBUG_D88_IMAGE
#include "../logging.h"
#endif

#define CALC_TRACK_DATA_POS(track, side) (track * m_number_of_side + side)
#define GET_TRACK_DATA_TBL_PTR(buffer, pos) (buffer + 0x20 + pos * 4 + 0)

extern EMU *emu;

// ----------------------------------------------------------------------------

DISKD88_SECTOR::DISKD88_SECTOR()
	: DISK_SECTOR()
{
	clear();
}
DISKD88_SECTOR::~DISKD88_SECTOR()
{
}
void DISKD88_SECTOR::clear()
{
	p_sector_id = NULL;
	p_sector_data = NULL;
	m_sector_size = 0;
	memset(m_id, 0, sizeof(m_id));
	m_density = 0;
	m_deleted = 0;
	m_status = 0;
	m_raw_pos = 0;
}
/// @brief Set sector info from buffer
///
/// @param[in] t : sector data in d88 track
void DISKD88_SECTOR::set(uint8_t *t)
{
	// header info
	p_sector_id = t;

	m_id[0] = t[0];
	m_id[1] = t[1];
	m_id[2] = t[2];
	m_id[3] = t[3];
	m_density = t[6];
	m_deleted = t[7];
	m_status = t[8];

#if 0
	uint16_t crc = CRC16_INIT_DATA;
	if (!(m_density & 0x40)) {
		// double density
		crc = DISK_IMAGE::set_crc16(0xa1, crc);
		crc = DISK_IMAGE::set_crc16(0xa1, crc);
		crc = DISK_IMAGE::set_crc16(0xa1, crc);
	}
	crc = DISK_IMAGE::set_crc16(0xfe, crc); // AM Address Mark
	crc = DISK_IMAGE::set_crc16(t[0], crc);
	crc = DISK_IMAGE::set_crc16(t[1], crc);
	crc = DISK_IMAGE::set_crc16(t[2], crc);
	crc = DISK_IMAGE::set_crc16(t[3], crc);

	m_id[4] = crc >> 8;
	m_id[5] = crc & 0xff;
#endif

	p_sector_data = t + 0x10;
	m_sector_size = (128 << m_id[3]);
}

/// @brief Set data in current sector
///
/// @param[in] pos : position
/// @param[in] val : value
void DISKD88_SECTOR::set_sector_data(int pos, uint8_t val)
{
	p_sector_data[pos] = val;
}

/// @brief Get data in current sector
///
/// @param[in] pos : position
/// @return value
uint8_t DISKD88_SECTOR::get_sector_data(int pos) const
{
	return p_sector_data[pos];
}

/// @brief Get size in current sector
///
/// @return size
int DISKD88_SECTOR::get_sector_size() const
{
	return m_sector_size;
}

/// @brief Set id in current sector
///
/// @param[in] pos : position
/// @param[in] val : value
void DISKD88_SECTOR::set_sector_id(int pos, uint8_t val)
{
	if (0 <= pos && pos < 4) {
		m_id[pos] = val;
		if (p_sector_id) {
			p_sector_id[pos] = val;
		}
	}
}

/// @brief Get id in current sector
///
/// @param[in] pos : position
/// @return value
uint8_t DISKD88_SECTOR::get_sector_id(int pos) const
{
	if (0 <= pos && pos < 6) {
		return m_id[pos];
	}
	return 0;
}

/// @brief Set single density in current sector
///
/// @param[in] val
void DISKD88_SECTOR::set_single_density(bool val)
{
	m_density = val ? 0x40 : 0;
	if (p_sector_id) {
		p_sector_id[6] = m_density;
	}
}

/// @brief Single density in current sector
///
/// @return true if single density
bool DISKD88_SECTOR::is_single_density() const
{
	return ((m_density & 0x40) != 0);
}

void DISKD88_SECTOR::set_raw_pos(uint16_t val)
{
	m_raw_pos = val;
}

/// @brief Raw position in the track
///
/// @return position
uint16_t DISKD88_SECTOR::get_raw_pos() const
{
	return m_raw_pos;
}

/// @brief set deleted mark in current sector
///
/// @param[in] val
void DISKD88_SECTOR::set_deleted_mark(bool val)
{
	m_deleted = val ? 0x10 : 0;
	if (p_sector_id) {
		p_sector_id[7] = m_deleted;
	}
}

/// @brief deleted mark detected in current sector
///
/// @return true if deleted mark detected
bool DISKD88_SECTOR::has_deleted_mark() const
{
	return (m_deleted == 0x10);
}

/// @brief CRC error detected in current sector
///
/// @param[in] density : 1 if double density (unused)
/// @return true if error detected
bool DISKD88_SECTOR::has_crc_error(int density) const
{
	return ((m_status & 0x20) != 0);
}

/// @brief Calculate the CRC on the part of current sector data
/// and compare this result with the end of the data
///
/// @param[in] size : size of the part of current sector data
/// @return true if match
bool DISKD88_SECTOR::compare_crc_on_data(int size) const
{
	if (size == m_sector_size) {
		// d88 disk image has no crc
		return true;
	} else if ((size + 2) > m_sector_size) {
		// always false
		return false;
	}

	uint16_t calc_crc = CRC16_INIT_DATA;

	if (!(m_density & 0x40)) {
		// double density
		calc_crc = DISK_IMAGE::set_crc16(0xa1, calc_crc);
		calc_crc = DISK_IMAGE::set_crc16(0xa1, calc_crc);
		calc_crc = DISK_IMAGE::set_crc16(0xa1, calc_crc);
	}

	calc_crc = DISK_IMAGE::set_crc16(m_deleted ? 0xf8 : 0xfe, calc_crc);
	for(int i=0; i<size; i++) {
		calc_crc = DISK_IMAGE::set_crc16(p_sector_data[i], calc_crc);
	}
	uint16_t data_crc = ((uint16_t)p_sector_data[size] << 8) | p_sector_data[size+1];

	return (calc_crc == data_crc);
}

// ----------------------------------------------------------------------------

DISKD88_TRACK::DISKD88_TRACK()
	: DISK_TRACK()
{
	p_parent = NULL;
	initialize();
}

DISKD88_TRACK::~DISKD88_TRACK()
{
}

/// @brief Initialize valiables
void DISKD88_TRACK::initialize()
{
	p_track = NULL;
	m_track_num = 0;
	m_side_num = 0;
	m_track_table_pos = -1;
	m_sector_nums = 0;
//	m_format_type = 0;
	m_valid_sector_pos = false;
//	memset(m_id_c_in_track, 0, sizeof(m_id_c_in_track));
//	memset(m_sector_pos, 0, sizeof(m_sector_pos));
#ifndef USE_SECTOR_LIST
	memset(m_sector_raw_pos, 0, sizeof(m_sector_raw_pos));
#endif
	m_sector_all_size = 0;

	m_track_size = TRACK_SIZE_5INCH_2D;
}

/// @brief Clear valiables
void DISKD88_TRACK::clear()
{
	p_track = NULL;
	m_track_num = 0;
	m_side_num = 0;
	m_track_table_pos = -1;
	m_sector_nums = 0;
#ifdef USE_SECTOR_LIST
#ifdef USE_CPTRLIST_SECTOR
	m_sectors.Clear();
#else
	m_sectors_count = 0;
#endif
#endif
}

/// @brief Set information of current track
///
/// @param[in] t : pointer to track data
/// @param[in] track_num : position in the disk
/// @param[in] side_num : head position
/// @param[in] track_table_pos : position in track table
/// @param[in] sector_nums : number of sector
/// @param[in] format_type set by host
void DISKD88_TRACK::set(uint8_t *t, int track_num, int side_num, int track_table_pos, int sector_nums, int format_type)
{
	if (sector_nums >= MAX_SECTOR_NUMS) {
		sector_nums = MAX_SECTOR_NUMS;
	}

	p_track = t;
	m_track_num = track_num;
	m_side_num = side_num;
	m_track_table_pos = track_table_pos;
	m_sector_nums = sector_nums;
//	m_format_type = (uint8_t)format_type;
	m_valid_sector_pos = false;
	m_sector_all_size = 0;
#ifdef USE_SECTOR_LIST
#ifdef USE_CPTRLIST_SECTOR
	m_sectors.Clear();
#else
	m_sectors_count = 0;
#endif
#endif
}

/// @brief Set the pointer to disk image object
///
/// @param[in] parent : disk image
void DISKD88_TRACK::set_parent(DISKD88_IMAGE *parent)
{
	p_parent = parent;
}

/// @brief Get the pointer to current track
///
/// @return the pointer to current track
uint8_t *DISKD88_TRACK::get_buffer()
{
	return p_track;

}

/// @brief Get position in the disk
///
/// @return the position
int DISKD88_TRACK::get_track_num() const
{
	return m_track_num;
}

#ifdef USE_SECTOR_LIST
/// @brief Get sector
///
/// @return sector information
DISKD88_SECTOR *DISKD88_TRACK::get_sector_by_pos(int sector_pos)
{
#ifdef USE_CPTRLIST_SECTOR
	return m_sectors.Item(sector_pos);
#else
	return &m_sectors[sector_pos];
#endif
}
#endif

/// @brief Get number of sector
///
/// @return number of sector
int DISKD88_TRACK::get_sector_nums() const
{
	return m_sector_nums;
}

/// @brief Verify a track number in each sector on a track
///
/// @param[in] track   : track number
/// @param[in] density : 0:single density(FM) 1:double density(MFM)
/// @param[in] hiden   : 2:high density 4:normal density
/// @param[in] rps     : round per sec.
/// @return true if match
bool DISKD88_TRACK::verify_track_number(int track, int density, int hiden, int rps)
{
	parse_sector(density, hiden, rps);
#ifndef USE_SECTOR_LIST
	uint8_t *t = p_track;
	if (!t) return false;

	for(int i = 0; i < m_sector_nums; i++) {
		if(t[0] != track) {
			return false;
		}
		// next sector
		t += (t[0xe] | (t[0xf] << 8)) + 0x10;
	}
#else
#ifdef USE_CPTRLIST_SECTOR
	for(int i = 0; i < m_sectors_Count(); i++) {
		if(m_sectors.Item(i)->get_sector_id(0) != track) {
			return false;
		}
	}
#else
	for(int i = 0; i < m_sectors_count; i++) {
		if(m_sectors[i].get_sector_id(0) != track) {
			return false;
		}
	}
#endif
#endif
	return true;
}

/// @brief Get ID_C (track) number in current track
///
/// @param[in] sector_pos : sector position
/// @return ID_C (track) number
uint8_t DISKD88_TRACK::get_id_c_in_track(int sector_pos) const
{
#ifndef USE_SECTOR_LIST
	uint8_t *t = p_track;
	if (!t) return 0;

	for(int i = 0; i < sector_pos; i++) {
		// next sector
		t += (t[0xe] | (t[0xf] << 8)) + 0x10;
	}
	return t[0];
#else
#ifdef USE_CPTRLIST_SECTOR
	return m_sectors.Item(sector_pos)->get_sector_id(0);
#else
	return m_sectors[sector_pos].get_sector_id(0);
#endif
#endif
}

/// @brief Get the sector position in current track
///
/// @param[in] sector_num : sector number
/// @return position
uint8_t DISKD88_TRACK::get_sector_pos(int sector_num) const
{
	int pos = 0;
#ifndef USE_SECTOR_LIST
//	return m_sector_pos[sector_num & 0xff];
	uint8_t *t = p_track;
	if (!t) return 0;

	for(int i=0; i < m_sector_nums; i++) {
		if (t[2] == (uint8_t)sector_num) {
			pos = i;
			break;
		}
		// next sector
		t += (t[0xe] | (t[0xf] << 8)) + 0x10;
	}
#else
#ifdef USE_CPTRLIST_SECTOR
	for(int i=0; i < m_sectors.Count(); i++) {
		if (m_sectors.Item(i)->get_sector_id(2) == (uint8_t)sector_num) {
			pos = i;
			break;
		}
	}
#else
	for(int i=0; i < m_sectors_count; i++) {
		if (m_sectors[i].get_sector_id(2) == (uint8_t)sector_num) {
			pos = i;
			break;
		}
	}
#endif
#endif
	return pos;
}

/// @brief Get the sector position in current track
///
/// @param[in] sector_num : sector number
/// @param[in] start_pos : sector position to start to search a sector
/// @return position
uint8_t DISKD88_TRACK::get_sector_pos(int sector_num, int start_pos) const
{
	int pos = start_pos;
#ifndef USE_SECTOR_LIST
	uint8_t *t = p_track;
	if (!t) return 0;

	for(int i=0; i < start_pos; i++) {
		// skip sector
		t += (t[0xe] | (t[0xf] << 8)) + 0x10;
	}
	for(int i=0; i < m_sector_nums; i++) {
		if (t[2] == (uint8_t)sector_num) {
			break;
		}
		// next sector
		t += (t[0xe] | (t[0xf] << 8)) + 0x10;

		pos++;
		if (pos >= m_sector_nums) {
			t = p_track;
			pos = 0;
		}
	}
#else
#ifdef USE_CPTRLIST_SECTOR
	for(int i=0; i < m_sectors.Count(); i++) {
		if (m_sectors.Item(pos)->get_sector_id(2) == (uint8_t)sector_num) {
			break;
		}
		pos++;
		if (pos >= m_sectors.Count()) {
			pos = 0;
		}
	}
#else
	for(int i=0; i < m_sectors_count; i++) {
		if (m_sectors[pos].get_sector_id(2) == (uint8_t)sector_num) {
			break;
		}
		pos++;
		if (pos >= m_sectors_count) {
			pos = 0;
		}
	}
#endif
#endif
	return pos;
}

/// @brief Get the sector position in current track
///
/// @param[in] sector_num : sector number
/// @param[in] curr_ratio : current head position in a track
/// @return position
uint8_t DISKD88_TRACK::get_sector_pos(int sector_num, double curr_ratio) const
{
	int start_pos = get_sector_pos_from_curr_ratio(curr_ratio);
	return get_sector_pos(sector_num, start_pos);
}

/// @brief Get raw position ratio at sector position
///
/// @param[in] sector_pos : sector position
/// @return raw position
double DISKD88_TRACK::get_sector_ratio_from_pos(int sector_pos) const
{
	double ratio = 1.0;
#ifndef USE_SECTOR_LIST
	if (m_sector_all_size > 0) {
		ratio = (double)m_sector_raw_pos[sector_pos] / m_sector_all_size;
	}
#else
#ifdef USE_CPTRLIST_SECTOR
	if (m_sector_all_size > 0 && sector_pos < m_sectors.Count()) {
		int data_pos = m_sectors.Item(i)->get_raw_pos();
		ratio = (double)data_pos / m_sector_all_size;
	}
#else
	if (m_sector_all_size > 0 && sector_pos < m_sectors_count) {
		int data_pos = m_sectors[sector_pos].get_raw_pos();
		ratio = (double)data_pos / m_sector_all_size;
	}
#endif
#endif
	return ratio;
}

/// @brief Get raw position ratio at sector position
///
/// @param[in] sector_num : sector number
/// @return raw position
double DISKD88_TRACK::get_sector_ratio(int sector_num) const
{
	int sector_pos = get_sector_pos(sector_num);
	return get_sector_ratio_from_pos(sector_pos);
}

/// @brief Get the sector position from ratio
///
/// @param[in] curr_ratio
/// @return position
int DISKD88_TRACK::get_sector_pos_from_curr_ratio(double curr_ratio) const
{
	int head_pos = (int)(curr_ratio * m_sector_all_size);
	int pos = 0;
#ifndef USE_SECTOR_LIST
	for(int i=0; i < m_sector_nums; i++) {
		if (head_pos < m_sector_raw_pos[i]) {
			pos = i;
			break;
		}
	}
#else
#ifdef USE_CPTRLIST_SECTOR
	for(int i=0; i < m_sectors.Count(); i++) {
		int data_pos = m_sectors.Item(i)->get_raw_pos();
		if (head_pos < data_pos) {
			pos = i;
			break;
		}
	}
#else
	for(int i=0; i < m_sectors_count; i++) {
		int data_pos = m_sectors[i].get_raw_pos();
		if (head_pos < data_pos) {
			pos = i;
			break;
		}
	}
#endif
#endif
	return pos;
}

/// @brief Get the sector position from ratio
///
/// @param[in] ratio
/// @return position
int DISKD88_TRACK::get_sector_pos_from_ratio(double &ratio) const
{
	int num = get_sector_pos_from_curr_ratio(ratio);
	ratio = get_sector_ratio_from_pos(num);
	return num;
}

/// @brief Parse sector in current track
///
/// @param[in] density : 0:single density(FM) 1:double density(MFM)
/// @param[in] hiden   : 2:high density 4:normal density
/// @param[in] rps     : round per sec.
/// @return true
bool DISKD88_TRACK::parse_sector(int density, int hiden, int rps)
{
	if (m_valid_sector_pos) return true;

	uint8_t *t = p_track;
	int rawpos = 0;
	struct st_ibm_format_length *gap_len = &c_ibm_format_length[density];
	// length of index mark
	rawpos += gap_len->gap0;
	rawpos += gap_len->sync;
	rawpos += gap_len->mark;	// index mark
	rawpos += gap_len->gap1;
	for(int pos = 0; pos < m_sector_nums; pos++) {
//		m_id_c_in_track[pos] = t[0];
		// sector position on a track
//		m_sector_pos[t[2]] = (pos & 0xff);
		// sector data start position on a raw track
		rawpos += gap_len->sync;
		rawpos += gap_len->mark;	// address mark
		rawpos += 6; // C H R N CRC
		rawpos += gap_len->gap2;
		rawpos += gap_len->sync;
		rawpos += gap_len->mark;	// data mark
#ifndef USE_SECTOR_LIST
		if (pos < MAX_SECTORS) {
			m_sector_raw_pos[pos] = (uint16_t)rawpos;
		}
#else
#ifdef USE_CPTRLIST_SECTOR
		sector = new DISKD88_SECTOR();
		sector->set(t);
		sector->set_raw_pos(rawpos);
		m_sectors.Add(sector);
#else
		if (m_sectors_count < MAX_SECTORS) {
			sector = &m_sectors[m_sectors_count];
			sector->set(t);
			sector->set_raw_pos(rawpos);
			m_sectors_count++;
		}
#endif
#endif
		rawpos += (128 << (t[3] & 7));
		rawpos += 2;	// CRC
		rawpos += gap_len->gap3;
		// next sector
		t += (t[0xe] | (t[0xf] << 8)) + 0x10;
	}
	if (density) {
		m_sector_all_size = 125000 / rps / hiden;
	} else {
		m_sector_all_size = 125000 / rps / hiden / 2;
	}

#ifdef _DEBUG_D88_IMAGE
	logging->out_debugf(_T("D88::parse_sector: C:%d H:%d"),p_track[0],p_track[1]);
#endif

	m_valid_sector_pos = true;
	return true;
}

/// @brief Make a raw track image from d88 track
///
/// @param[in] density : 0:single density(FM) 1:double density(MFM)
/// @param[in] hiden   : 2:high density 4:normal density
/// @param[in] rps     : round per sec.
/// @return true if success
bool DISKD88_TRACK::make_track(int density, int hiden, int rps)
{
	uint8_t *t = p_track;

	if(!t || m_sector_nums < 0 || m_sector_nums > 2048) {
		// invalid
		return false;
	}

	// create dummy track
	for(int i = 0; i < TRACK_BUFFER_SIZE; i++) {
		m_track_buffer[i] = rand();
	}

	// make track image
	struct st_ibm_format_length *gap_len = &c_ibm_format_length[density];
//	int gap3len = 0;
//	if (dden) {
//		gap3len = (m_sector_nums <= 5) ? 0x74 : (m_sector_nums <= 10) ? 0x54 : (m_sector_nums <= 16) ? 0x33 : 0x10;
//		gap3len = (m_sector_nums <= 8) ? 116 : (m_sector_nums <= 15) ? 84 : (m_sector_nums <= 26) ? 54 : 16;
//	} else {
//		gap3len = (m_sector_nums <= 8) ? 58 : (m_sector_nums <= 15) ? 42 : (m_sector_nums <= 26) ? 27 : 8;
//	}
	uint8_t gapmark = c_gap_sign[density];
	int p = 0;
	int len = 0;

	// preamble

	// gap0
	len = gap_len->gap0;
	for(int i = 0; i < gap_len->gap0; i++) {
		m_track_buffer[p++] = gapmark;
	}
	// sync
	len = gap_len->sync;
	for(int i = 0; i < len; i++) {
		m_track_buffer[p++] = 0;
	}
	// index mark
	if (density) {
		m_track_buffer[p++] = 0xc2;
		m_track_buffer[p++] = 0xc2;
		m_track_buffer[p++] = 0xc2;
	}
	m_track_buffer[p++] = 0xfc;
	// gap1
	len = gap_len->gap1;
	for(int i = 0; i < len; i++) {
		m_track_buffer[p++] = gapmark;
	}

	// sectors

	for(int i = 0; i < m_sector_nums; i ++) {
		// sync
		len = gap_len->sync;
		for(int j = 0; j < len; j++) {
			m_track_buffer[p++] = 0;
		}

		// id address mark
		uint16_t crc = CRC16_INIT_DATA;
		if (density) {
			m_track_buffer[p++] = 0xa1;
			m_track_buffer[p++] = 0xa1;
			m_track_buffer[p++] = 0xa1;

			crc = DISK_IMAGE::set_crc16(0xa1, crc);
			crc = DISK_IMAGE::set_crc16(0xa1, crc);
			crc = DISK_IMAGE::set_crc16(0xa1, crc);
		}
		m_track_buffer[p++] = 0xfe;
		m_track_buffer[p++] = t[0];
		m_track_buffer[p++] = t[1];
		m_track_buffer[p++] = t[2];
		m_track_buffer[p++] = t[3];

		crc = DISK_IMAGE::set_crc16(0xfe, crc);
		crc = DISK_IMAGE::set_crc16(t[0], crc);
		crc = DISK_IMAGE::set_crc16(t[1], crc);
		crc = DISK_IMAGE::set_crc16(t[2], crc);
		crc = DISK_IMAGE::set_crc16(t[3], crc);

		m_track_buffer[p++] = crc >> 8;
		m_track_buffer[p++] = crc & 0xff;
		// gap2
		len = gap_len->gap2;
		for(int j = 0; j < len; j++) {
			m_track_buffer[p++] = gapmark;
		}
		// sync
		len = gap_len->sync;
		for(int j = 0; j < len; j++) {
			m_track_buffer[p++] = 0;
		}
		// data mark, deleted mark
		crc = CRC16_INIT_DATA;
		if (density) {
			m_track_buffer[p++] = 0xa1;
			m_track_buffer[p++] = 0xa1;
			m_track_buffer[p++] = 0xa1;

			crc = DISK_IMAGE::set_crc16(0xa1, crc);
			crc = DISK_IMAGE::set_crc16(0xa1, crc);
			crc = DISK_IMAGE::set_crc16(0xa1, crc);
		}
		// deleted ?
		m_track_buffer[p++] = (t[7]) ? 0xf8 : 0xfb;
		crc = DISK_IMAGE::set_crc16((t[7]) ? 0xf8 : 0xfb, crc);

		// data
		int size = t[0xe] | (t[0xf] << 8);
		for(int j = 0; j < size; j++) {
			m_track_buffer[p++] = t[0x10 + j];

			crc = DISK_IMAGE::set_crc16(t[0x10 + j], crc);
		}
		m_track_buffer[p++] = crc >> 8;
		m_track_buffer[p++] = crc & 0xff;
		t += size + 0x10;
		// gap3
		len = gap_len->gap3;
		for(int j = 0; j < len; j++) {
			m_track_buffer[p++] = gapmark;
		}
	}
//	// gap4
//	if (dden) {
//		len = (m_sector_nums <= 8) ? 654 : (m_sector_nums <= 15) ? 400 : (m_sector_nums <= 26) ? 598 : 16;
//	} else {
//		len = (m_sector_nums <= 8) ? 311 : (m_sector_nums <= 15) ? 170 : (m_sector_nums <= 26) ? 247 : 8;
//	}
//	for(int i = 0; i < len && p < TRACK_BUFFER_SIZE; i++) {
//		m_track_buffer[p++] = gapmark;
//	}
	// padding
	if (density) {
		len = 125000 / rps / hiden;
	} else {
		len = 125000 / rps / hiden / 2;
	}
	while(p < len && p < TRACK_BUFFER_SIZE) {
		m_track_buffer[p++] = gapmark;
	}
	m_track_size = p;

	return true;
}

/// @brief Parse raw track data and replace to d88 track
///
/// set_raw_track_data() is stored in m_track_buffer[] as raw track image  
/// convert to d88 track data from raw track data using this function
///
/// @param[in] density : 1 if double density
/// @param[in] hiden : 4:normal 2:high
/// @param[in] rps : round per sec.
/// @return true if success
bool DISKD88_TRACK::parse_track2(int density, int hiden, int rps)
{
	uint8_t gapmark[16];
	int gap_len = (density ? 16 : 8);
//	uint8_t *index_mark = c_index_mark[density];
	uint8_t *addr_mark = c_address_mark[density];
	uint8_t *data_mark = c_data_mark[density];
	uint8_t *delete_mark = c_deleted_mark[density];
	int mark_len = c_mark_length[density];

	for(int i=0; i<gap_len; i++) {
		gapmark[i] = c_gap_sign[density];
	}

	// parse track image
	int p = 0;
	int phase = 0;
	int sec_size = 128;
	int num_of_sector = 0;

	int tmp_track_size = 0;
	uint8_t *tmp_track = new uint8_t[TRACK_BUFFER_SIZE];

	int tp = 0;	// position of tmp_track
	memset(tmp_track, 0, sizeof(uint8_t) * TRACK_BUFFER_SIZE);

	for(p=0; p < m_track_size && p < TRACK_BUFFER_SIZE; p++) {
		switch(phase) {
		default:
//			// search index mark
//			if (memcmp(&m_track_buffer[p], index_mark, mark_len) == 0) {
//				p += mark_len;
//				phase = 1;
//			}
			// search gap sign
			if (memcmp(&m_track_buffer[p], gapmark, gap_len) == 0) {
				p += gap_len;
				phase++;
			}
			break;
		case 1:
			// search id address mark
			if (memcmp(&m_track_buffer[p], addr_mark, mark_len) == 0) {
				// set sector
				p += mark_len;

				memcpy(&tmp_track[tp], &m_track_buffer[p], 4); // C H R N
	//			sector_number = track[p+2]; // sector number
				num_of_sector++;

				// calc sector size from ID N (N is 0 to 7)
				sec_size = (128 << (m_track_buffer[p+3] & 7));
				conv_from_uint16_le(&tmp_track[tp+14], (uint16_t)sec_size); 

				switch(density) {
					case 0:
						tmp_track[tp+6] = 0x40;
						break;
					default:
						tmp_track[tp+6] = 0;
						break;
				}

				phase = 2;
			}
			break;
		case 3:
			// search data mark
			if ((memcmp(&m_track_buffer[p], data_mark, mark_len) == 0
			 ||  memcmp(&m_track_buffer[p], delete_mark, mark_len) == 0)) {
				p += mark_len;

				// deleted mark ?
				tmp_track[tp+7] = (m_track_buffer[p-1] == 0xf8 ? 0x10 : 0);

				// copy sector data
				tp += 16;
				memcpy(&tmp_track[tp], &m_track_buffer[p], sec_size);

				p += sec_size;
				tp += sec_size;

				phase = 0;
			}
			break;
		}
	}

	tmp_track_size = tp;

	// update number of sector
	bool valid = true;
	for(tp=0; tp < tmp_track_size && tp < TRACK_BUFFER_SIZE;) {
		conv_from_uint16_le(&tmp_track[tp+4], (uint16_t)num_of_sector);
		uint16_t next = conv_to_uint16_le(&tmp_track[tp+14]);
		if (next < 128 || next > 8192) {
			// invalid sector size
			valid = false;
			break;
		}
		tp += next + 16;
	}

	if (valid) {
		// replace buffer
		p_track = p_parent->replace_track(m_track_table_pos, tmp_track, tmp_track_size);
		// set information
		m_sector_nums = num_of_sector;
		m_valid_sector_pos = false;
		m_sector_all_size = 0;
#ifdef USE_SECTOR_LIST
#ifdef USE_CPTRLIST_SECTOR
		m_sectors.Clear();
#else
		m_sectors_count = 0;
#endif
#endif
	}

	delete [] tmp_track;

	return valid;
}

/// @brief Set raw track data
///
/// @param[in] pos : position
/// @param[in] val : value
void DISKD88_TRACK::set_raw_track_data(int pos, uint8_t val)
{
	m_track_buffer[pos] = val;
}

/// @brief Get raw track data
///
/// @param[in] pos : position
/// @return value
uint8_t DISKD88_TRACK::get_raw_track_data(int pos) const
{
	return m_track_buffer[pos];
}

/// @brief Set raw track size
///
/// @param[in] val : size
void DISKD88_TRACK::set_raw_track_size(int val)
{
	m_track_size = val;
}

/// @brief Set raw track size
///
/// @return size
int DISKD88_TRACK::get_raw_track_size() const
{
	return m_track_size;
}

// ----------------------------------------------------------------------------

DISKD88_IMAGE::DISKD88_IMAGE()
	: DISK_IMAGE()
{
	clear();
}

DISKD88_IMAGE::DISKD88_IMAGE(const DISKD88_IMAGE &src)
	: DISK_IMAGE(src)
{
	clear();
}

DISKD88_IMAGE::DISKD88_IMAGE(DISK *disk, uint8_t *buffer)
	: DISK_IMAGE(disk)
{
	clear();
	set(disk, buffer);
}

DISKD88_IMAGE::~DISKD88_IMAGE()
{
}

/// @brief Clear valiables
void DISKD88_IMAGE::clear()
{
	DISK_IMAGE::clear_base();

//	m_media_type = MEDIA_TYPE_UNK;
	p_buffer = NULL;
	m_number_of_side = 2;
	m_current_side = 0;

	m_current_track[0].set_parent(this);
	m_current_track[1].set_parent(this);
#ifdef USE_SECTOR_LIST
	p_current_sector = NULL;
#endif
}

/// @brief Set buffer pointer
///
/// @param[in] disk
/// @param[in] buffer
void DISKD88_IMAGE::set(DISK *disk, uint8_t *buffer)
{
	DISK_IMAGE::set_base(disk);

	p_buffer = buffer;
}

/// @brief Get write protected
///
/// @return true if write protected
bool DISKD88_IMAGE::is_write_protected() const
{
	return (p_buffer[0x1a] != 0);
}

/// @brief Set write protect
///
/// @param[in] val : true: set the protection
void DISKD88_IMAGE::set_write_protect(bool val)
{
	// set write protect flag
	if (val) {
		p_buffer[0x1a] |= 0x10;
	} else {
		p_buffer[0x1a] &= ~0x10;
	}
}

/// @brief Write operation is supported?
///
/// @return true if writable 
bool DISKD88_IMAGE::is_write_supported() const
{
	return true;
}

/// @brief Get media type
///
/// @return media type
uint8_t DISKD88_IMAGE::get_media_type() const
{
	if (p_buffer) {
		return p_buffer[0x1b];
	} else {
		return MEDIA_TYPE_UNK;
	}
}

/// @brief Write a disk image
///
/// @param[in] fio : file handle object
/// @param[in] new_file : save as new file
/// @param[in] is_plain : save as plain image
/// @param[in] is_last_volume : last volume in the file
/// @return true if success
bool DISKD88_IMAGE::write_file(FILEIO *fio, bool new_file, bool is_plain, bool is_last_volume)
{
	if (is_plain) {
		// save as plain image
		d88_hdr_t *header = (d88_hdr_t *)p_buffer;

		for(int trkside = 0; trkside < 164; trkside++) {
			int offset = header->trkptr[trkside];
			if (offset == 0) {
				break;
			}
			uint8_t *sect = &p_buffer[offset];
			int sect_nums = ((d88_sct_t *)sect)->nsec;
			for(int s = 0; s < sect_nums; s++) {
				size_t sect_size = ((d88_sct_t *)sect)->size;
				sect += sizeof(d88_sct_t);
				fio->Fwrite(sect, sect_size, 1);
				sect += sect_size;
			}
		}

	} else {
		// save d88 image
		int file_offset = p_disk->get_file_offset();
		int file_size = p_disk->get_file_size();
		int file_size_orig = p_disk->get_file_size_orig();

		fio->Fseek(file_offset, FILEIO::SEEKSET);
		fio->Fwrite(p_buffer, file_size, 1);
		if (is_last_volume && file_size < file_size_orig) {
			// file size is shorter than before, so truncate file size
			trim_track_data_table();
			fio->Ftruncate(file_offset + file_size);
		}
	}
	return true;
}

/// @brief Get specified track offset
///
/// @param[in] track : track number
/// @param[in] side : side number
/// @param[out] track_table_pos : position of track table
/// @return -1: track or side number is out of range.
int DISKD88_IMAGE::get_track_offset(int track, int side, int *track_table_pos)
{
	// search track
	side &= 1;
	if (m_number_of_side <= side) {
		// out of range
		return -1;
	}
	int trkside = CALC_TRACK_DATA_POS(track, side);
	if(!(0 <= trkside && trkside < 164)) {
		if (track_table_pos) *track_table_pos = -1;
		return -1;
	}
	if (track_table_pos) *track_table_pos = trkside;
	int offset = (int)conv_to_uint32_le(GET_TRACK_DATA_TBL_PTR(p_buffer, trkside));
	return offset;
}

/// @brief Get specified track buffer
///
/// @param[in] track : track number
/// @param[in] side : side number
/// @param[out] track_table_pos : position of track table
/// @return : buffer position of specified track in a disk
uint8_t* DISKD88_IMAGE::get_track_buffer(int track, int side, int *track_table_pos)
{
	// search track position
	int offset = get_track_offset(track, side, track_table_pos);

	if(offset <= 0) {
		return NULL;
	}

	// track found
	uint8_t *t = p_buffer + offset;

	return t;
}

/// @brief Shrink one track space
///
/// @param [in] start : track position
/// @param [in] size  : size to subtract
void DISKD88_IMAGE::shrink_track_space(int start, int size)
{
	int offset = (start - size < 0 ? start : size);

	uint8_t *sp = &p_buffer[start];
	uint8_t *dp = sp - offset;
	uint8_t *ep = &p_buffer[p_disk->get_file_size()];
	uint8_t *fp = &p_buffer[p_disk->get_buffer_size()];

	for(;sp != ep && sp != fp; sp++, dp++) {
		*dp = *sp;
	}

	add_file_size(-offset);

	recalc_track_data_table(start, -offset);
}

/// @brief Expand one track space
///
/// @param [in] start : track position
/// @param [in] size  : size to add
void DISKD88_IMAGE::expand_track_space(int start, int size)
{
	int file_size = p_disk->get_file_size();
	int buffer_size = p_disk->get_buffer_size();
	int offset = (file_size + size > buffer_size ? buffer_size - file_size : size);

	uint8_t *sp = &p_buffer[file_size-1];
	uint8_t *dp = sp + offset;
	uint8_t *ep = &p_buffer[start-1];
	uint8_t *fp = &p_buffer[0];

	for(;sp != ep && sp != fp; sp--, dp--) {
		*dp = *sp;
	}

	add_file_size(offset);

	recalc_track_data_table(start, offset);
}

/// @brief Find track position
///
/// @param [in] trkside : track position
/// @param [out] offset : offset size
/// @return -1: not found  0>: position
int DISKD88_IMAGE::find_track_side(int trkside, int *offset) 
{
	uint8_t *sp;
	int findtrkside = -1;

	for(int pos = trkside + 1; pos < 164; pos++) {
		sp = GET_TRACK_DATA_TBL_PTR(p_buffer, pos);
		int trkpos = (int)conv_to_uint32_le(sp);
		if (trkpos != 0) {
			findtrkside = pos;
			if (offset) {
				*offset = trkpos;
			}
			break;
		}
	}

	return findtrkside;
}

/// @brief Update track data table
///
/// @param [in] start : track position
/// @param [in] offset : size to add
void DISKD88_IMAGE::recalc_track_data_table(int start, int offset)
{
	uint8_t *sp;

	// recalc track data table
	for(int pos = 0; pos < 164; pos++) {
		sp = GET_TRACK_DATA_TBL_PTR(p_buffer, pos);
		int trkpos = (int)conv_to_uint32_le(sp);
		if (start <= trkpos) {
			trkpos += offset;
			conv_from_uint32_le(sp, (uint32_t)trkpos);
		}
	}
}

/// @brief Set zero on unused track in track data table
void DISKD88_IMAGE::trim_track_data_table()
{
	int file_size = p_disk->get_file_size();
	uint8_t *sp;

	for(int pos = 0; pos < 164; pos++) {
		sp = GET_TRACK_DATA_TBL_PTR(p_buffer, pos);
		int trkpos = (int)conv_to_uint32_le(sp);
		if (file_size <= trkpos) {
			conv_from_uint32_le(sp, 0);
		}
	}
}

/// @brief Calculate one track size
///
/// @param [in] t : track buffer
/// @return size
int DISKD88_IMAGE::calc_track_size(uint8_t *t)
{
	int size = 0;
	int num_of_sec = conv_to_uint16_le(t + 4);

	for(int n = 0; n < num_of_sec; n++) {
		uint16_t next = conv_to_uint16_le(t + size +14);
		size += next + 16;
	}
	return size;
}

/// @brief Add file size to buffer
///
/// @param[in] offset : size to add
void DISKD88_IMAGE::add_file_size(int offset)
{
	int file_size = p_disk->get_file_size();
	file_size += offset;
	conv_from_uint32_le(p_buffer + 0x1c, (uint32_t)file_size);
	p_disk->set_file_size(file_size);
}

/// @brief Check media type on the disk
///
/// @return true if match
bool DISKD88_IMAGE::check_media_type()
{
	if (FLG_CHECK_FDMEDIA) {
		uint8_t drive_type = p_disk->get_drive_type()->m_drive_type;
		uint8_t media_type = get_media_type();
		switch(drive_type) {
		case DRIVE_TYPE::DRIVE_TYPE_2D:
			return (media_type == MEDIA_TYPE_2D);
		case DRIVE_TYPE::DRIVE_TYPE_2DD:
			return (media_type == MEDIA_TYPE_2DD);
		case DRIVE_TYPE::DRIVE_TYPE_2HD_300:
		case DRIVE_TYPE::DRIVE_TYPE_2HD_360:
			return (media_type == MEDIA_TYPE_2HD);
		default: // case DRIVE_TYPE_UNK:
			return false;
		}
	}
	return true;
}

// ----------------------------------------------------------------------------

/// @brief Check and set the number of side
///
/// @return true if inserted a disk
bool DISKD88_IMAGE::check_tracks()
{
	// search track
	int prev_track = -1;
	int prev_side = -1;
	int max_side = -1;
	int max_trkside = -1;
	for(int trkside = 0; trkside < 164; trkside++) {
		uint32_t offset = conv_to_uint32_le(GET_TRACK_DATA_TBL_PTR(p_buffer, trkside));
		if (offset == 0) continue;

		int track = p_buffer[offset];
		int side = p_buffer[offset+1];
		if (track == prev_track && side == prev_side) {
			// track and side number are duplicate.
			side++;
		}
		if (max_side < side) max_side = side;
		if (max_trkside < trkside) max_trkside = trkside;

		prev_track = track;
		prev_side = side;
	}
	if (max_side < 0) { // no side or unformat
		// default 2 sides
		max_side = 1;
	} else if (max_side == 0) {
		// single side ?
		if (max_trkside >= 40) {
			// double side
			max_side = 1;
		}
	}
	m_number_of_side = max_side + 1;
	if (m_number_of_side > 2) m_number_of_side = 2;

	return true;
}

/// @brief Get number of side
///
/// @return number of side
int DISKD88_IMAGE::get_number_of_side() const
{
	return m_number_of_side;
}

/// @brief Set side position
///
/// @param[in] side_pos : side
void DISKD88_IMAGE::set_side_position(int side_pos)
{
	m_current_side = (side_pos & 1);
}

// ----------------------------------------------------------------------------

/// @brief Get current track number
///
/// @return track number
/// @note need call search_track() before invoke this
int DISKD88_IMAGE::get_current_track_number() const
{
	return m_current_track[m_current_side].get_id_c_in_track(0);
}

/// @brief Get current track
///
/// @return track info
/// @note need call search_track() before invoke this
DISK_TRACK *DISKD88_IMAGE::get_current_track()
{
	return &m_current_track[m_current_side];
}

/// @brief Get number of sector in current track
///
/// @return number of sector
/// @note need call search_track() before invoke this
int DISKD88_IMAGE::get_number_of_sector() const
{
	return m_current_track[m_current_side].get_sector_nums();
}

/// @brief Get raw track size on current track
///
/// @return size
/// @note need call search_track() before invoke this
int DISKD88_IMAGE::get_current_track_size() const
{
	return m_current_track[m_current_side].get_raw_track_size();
}

/// @brief Get specified track
///
/// @param[in] track_pos : track physical position
/// @param[in] side_pos : side physical position
/// @param[in] density : 0:single density 1:double density
/// @return : false : not found or invalid 
bool DISKD88_IMAGE::search_track(int track_pos, int side_pos, int density)
{
	// search track
	m_current_side = (side_pos & 1);
	int err_cnt = 0;
	for(side_pos = 0; side_pos < m_number_of_side; side_pos++) {
		// get buffer
		int track_table_pos;
		uint8_t* t = get_track_buffer(track_pos, side_pos, &track_table_pos);
		uint8_t* curr_t = m_current_track[side_pos].get_buffer();
		if (curr_t && curr_t == t) {
			// same track
			continue;
		}

		// set current sector
		m_current_track[side_pos].clear();

		int sector_nums = 0;
		if (t) {
			// track found
			sector_nums = conv_to_uint16_le(t + 4);
			if(sector_nums < 0 || sector_nums > 2048) {
				// invalid
				err_cnt++;
			}
		} else {
			// track not found
			err_cnt++;
		}
		// set current sector
		m_current_track[side_pos].set(t, track_pos, side_pos, track_table_pos, sector_nums, density);

#ifdef _DEBUG_D88_IMAGE
		logging->out_debugf(_T("D88::search_track: Trk:%d Sid:%d"),track_pos,side_pos);
#endif
	}

	return (err_cnt < m_number_of_side);
}

/// @brief Verify a track number in each sector on a track
///
/// @param[in] track_num : track number
/// @param[in] side_num : side number
/// @param[in] density : 0:single density 1:double density
/// @return true if match track number on all sectors in the track
bool DISKD88_IMAGE::verify_track_number(int track_num, int side_num, int density)
{
	const DRIVE_TYPE *drive_type = p_disk->get_drive_type();
	m_current_side = (side_num & 1);
	return m_current_track[m_current_side].verify_track_number(track_num, density, drive_type->m_hiden, drive_type->m_rps);
}

/// @brief Make raw track
/// convert from d88 track image
///
/// @param[in] side_num : side number
/// @param[in] density : density
/// @return : false : not found or invalid 
bool DISKD88_IMAGE::make_track(int side_num, int density)
{
	const DRIVE_TYPE *drive_type = p_disk->get_drive_type();
	m_current_side = (side_num & 1);
	return m_current_track[m_current_side].make_track(density, drive_type->m_hiden, drive_type->m_rps);
}

/// @brief Parse raw track
/// convert to d88 track image
///
/// @param[in] side_num : side number
/// @param[in] density : density
/// @return : false : not found or invalid 
bool DISKD88_IMAGE::parse_track(int side_num, int density)
{
	const DRIVE_TYPE *drive_type = p_disk->get_drive_type();
	m_current_side = (side_num & 1);
	return m_current_track[m_current_side].parse_track2(density, drive_type->m_hiden, drive_type->m_rps);
}

/// @brief Replace the track
///
/// @param [in] trkside : position of track table
/// @param [in] tmp_track : track data
/// @param [in] tmp_track_size : size of tmp_track
/// @return pointer to buffer in the disk
uint8_t *DISKD88_IMAGE::replace_track(int trkside, const uint8_t *tmp_track, int tmp_track_size)
{
	if (tmp_track_size == 0) return NULL;
	if (trkside < 0) return NULL;

//	int trkside = CALC_TRACK_DATA_POS(trk, side);
	uint32_t *p = (uint32_t *)GET_TRACK_DATA_TBL_PTR(p_buffer, trkside);
	int old_track_size = 0;
	int new_track_size = 0;
	int offset = 0;
	int file_size = p_disk->get_file_size();
	int buffer_size = p_disk->get_buffer_size();
	uint8_t *buf;

	if (*p == 0) {
		// track not exists

		// upper track exist?
		int uptrkside = find_track_side(trkside, &offset);
		if (uptrkside < 0) {
			// add track
			offset = file_size;
			buf = p_buffer + offset;

		} else {
			// upper track exist
			buf = p_buffer + offset;

			// expand space
			expand_track_space(offset, tmp_track_size);
		}

		new_track_size = MIN(tmp_track_size, buffer_size);
		memcpy(buf, tmp_track, new_track_size);

		if (uptrkside < 0) {
			// add file size
			add_file_size(new_track_size);
		}
	} else {
		// track exists
		offset = Uint32_LE(*p);
		buf = p_buffer + offset;
		old_track_size = calc_track_size(buf);

		if (old_track_size > tmp_track_size) {
			// shrink space
			shrink_track_space(offset + old_track_size, old_track_size - tmp_track_size); 
		} else if (old_track_size < tmp_track_size) {
			// expand space
			expand_track_space(offset + old_track_size, tmp_track_size - old_track_size);
		}

		// replace track
		new_track_size = MIN(tmp_track_size, buffer_size);
		memcpy(buf, tmp_track, new_track_size);
	}

	*p = Uint32_LE(offset);

	return buf;
}

// ----------------------------------------------------------------------------

/// @brief Get current sector position
///
/// @return sector number
/// @note need call parse_sector() before invoke this
int DISKD88_IMAGE::get_current_sector_pos(int sector_num) const
{
	return m_current_track[m_current_side].get_sector_pos(sector_num);
}

/// @brief Get current sector
///
/// @return sector info
/// @note need call search_track() and search_sector() before invoke this
DISK_SECTOR *DISKD88_IMAGE::get_current_sector()
{
#ifndef USE_SECTOR_LIST
	return &m_current_sector;
#else
	return p_current_sector;
#endif
}

/// @brief Get number of sector in current track
///
/// @return number of sector
/// @note need call search_track() before invoke this
int DISKD88_IMAGE::get_current_sector_size() const
{
#ifndef USE_SECTOR_LIST
	return m_current_sector.get_sector_size();
#else
	return p_current_sector ? p_current_sector->get_sector_size() : 0;
#endif
}

/// @brief Parse sector in current track
///
/// @param[in] side_num : side number
/// @param[in] density : 1 if double density
void DISKD88_IMAGE::parse_sector(int side_num, int density)
{
	const DRIVE_TYPE *drive_type = p_disk->get_drive_type();

	m_current_side = (side_num & 1);

	m_current_track[m_current_side].parse_sector(density, drive_type->m_hiden, drive_type->m_rps);
}

/// @brief Finds a sector from specified sector number.
///
/// @param[in] side_num : side number
/// @param[in] density : 1 if double density
/// @param[in] sector_num : sector number or -1 if specify sector position
/// @param[in,out] sector_pos : position of sector in the track. if specify -1 then sector position 0
/// @return DISK::en_result_search_sector
int DISKD88_IMAGE::search_sector(int side_num, int density, int sector_num, int &sector_pos)
{
	m_current_side = (side_num & 1);

	// search track
	uint8_t* t;
	int rc = get_current_track_buffer(t);
	if (rc != DISK::RES_NO_ERROR) {
		// track not found
		return rc;
	}

	// track found

	if (sector_num >= 0) {
		sector_pos = m_current_track[side_num & 1].get_sector_pos(sector_num);
	} else if (sector_pos < 0) {
		sector_pos = 0;
	}

	// set current sector and density check
	return set_current_sector(t, sector_pos, density);
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
int DISKD88_IMAGE::search_sector_and_get_clock(int side_num, int density, int sector_num, int &sector_pos, int remain_clock, int timeout_round, int &arrive_clock)
{
	const DRIVE_TYPE *drive_type = p_disk->get_drive_type();

	m_current_side = (side_num & 1);

	if (remain_clock < 0) remain_clock += drive_type->m_round_clock;

	// search track
	uint8_t* t;
	int rc = get_current_track_buffer(t);
	if (rc != DISK::RES_NO_ERROR) {
		// track not found
		arrive_clock = drive_type->m_round_clock * timeout_round + remain_clock;
		return rc;
	}

	// track found

	if (sector_num >= 0) {
		sector_pos = m_current_track[m_current_side].get_sector_pos(sector_num, (double)(drive_type->m_round_clock - remain_clock) / (double)drive_type->m_round_clock);
	} else if (sector_pos < 0) {
		sector_pos = m_current_track[m_current_side].get_sector_pos_from_curr_ratio((double)(drive_type->m_round_clock - remain_clock) / (double)drive_type->m_round_clock);
	}

	// set current sector
	rc = set_current_sector(t, sector_pos, density);
	if (rc != DISK::RES_NO_ERROR) {
		// invalid sector or different density
		arrive_clock = drive_type->m_round_clock * timeout_round + remain_clock;
		return rc;
	}

	// calcurate time
	arrive_clock = get_clock_from_sector_pos(sector_pos, drive_type, remain_clock, timeout_round);

	return DISK::RES_NO_ERROR;
} //

/// @brief Check media type and the buffer of current track 
///
/// @param[out] t : buffer in the track
/// @return DISK::RES_NO_ERROR, DISK::RES_INVALID_MEDIA_TYPE or DISK::RES_SECTOR_NOT_FOUND
int DISKD88_IMAGE::get_current_track_buffer(uint8_t * &t)
{
	t = NULL;

	// valid media type?
	if(!check_media_type()) {
		return DISK::RES_INVALID_MEDIA_TYPE;
	}

	// search track
	t = m_current_track[m_current_side].get_buffer();
	if (!t) {
		// track not found
		return DISK::RES_SECTOR_NOT_FOUND;
	}

	return DISK::RES_NO_ERROR;
}

/// @brief Set the sector as current and check density
///
/// @param[in] t : buffer in the track
/// @param[in] sector_pos : position of sector in the track
/// @param[in] density : 1 if double density
/// @return DISK::RES_NO_ERROR, DISK::RES_SECTOR_NOT_FOUND or DISK::RES_UNMATCH_DENSITY
int DISKD88_IMAGE::set_current_sector(uint8_t *t, int sector_pos, uint8_t density)
{
	if(sector_pos >= m_current_track[m_current_side].get_sector_nums()) {
		return DISK::RES_SECTOR_NOT_FOUND;
	}

#ifndef USE_SECTOR_LIST
	// skip sector
	for(int i = 0; i < sector_pos; i++) {
		t += (t[0xe] | (t[0xf] << 8)) + 0x10;
	}
	m_current_sector.set(t);
#else
	p_current_sector = m_current_track[m_current_side].get_sector_by_pos(sector_pos);
#endif

	// density check
	if (FLG_CHECK_FDDENSITY) {
#ifdef USE_SECTOR_LIST
		uint8_t sector_density = (p_current_sector->is_single_density() ? 0 : 1);
#else
		uint8_t sector_density = (m_current_sector.is_single_density() ? 0 : 1);
#endif
		if (density ^ sector_density) {
			// different density
			return DISK::RES_UNMATCH_DENSITY;
		}
	}
	return DISK::RES_NO_ERROR;
}

/// @brief Calculate the time (clocks) required to reach the sepcified sector position.
///
/// @param[in] sector_pos : position of sector in the track
/// @param[in] drive_type : type of this drive
/// @param[in] remain_clock : clock until reach index hole 
/// @param[in] timeout_round : number of round to occur timeout 
/// @return clock
int DISKD88_IMAGE::get_clock_from_sector_pos(int sector_pos, const DRIVE_TYPE *drive_type, int remain_clock, int timeout_round)
{
	double sector_ratio = m_current_track[m_current_side].get_sector_ratio_from_pos(sector_pos);

	int arrive_clock = (int)(sector_ratio * drive_type->m_round_clock) + remain_clock - drive_type->m_round_clock;
	if (arrive_clock < 0) {
		arrive_clock += drive_type->m_round_clock;
	}

#ifdef _DEBUG_D88_IMAGE
	logging->out_debugf(_T("D88::get_clock_from_sector_pos: curr_ratio:%.4f sect_pos:%02d sect_ratio:%.4f sum:%06d")
		, (double)(drive_type->m_round_clock - remain_clock) / drive_type->m_round_clock
		, sector_pos
		, sector_ratio
		, arrive_clock);
#endif
	return arrive_clock;
}

/// @brief Calculate the time (clocks) required to reach the sepcified sector.
///
/// @param[in] sector_num : sector number or -1 if specify sector position
/// @param[in] sector_pos : position of sector in the track. if specify -1 then search nearest sector
/// @param[in] remain_clock : clock until reach index hole 
/// @param[in] timeout_round : number of round to occur timeout 
/// @return clocks
int DISKD88_IMAGE::get_clock_arrival_sector(int sector_num, int sector_pos, int remain_clock, int timeout_round)
{
	const DRIVE_TYPE *drive_type = p_disk->get_drive_type();

	if (remain_clock < 0) remain_clock += drive_type->m_round_clock;

	double sector_ratio;
	if (sector_num >= 0) {
		// get ratio from specified sector number
		sector_ratio = m_current_track[m_current_side].get_sector_ratio(sector_num);
	} else if (sector_pos < 0) {
		// get current head position
		sector_ratio = (double)(drive_type->m_round_clock - remain_clock) / drive_type->m_round_clock;
		// get nearest sector position
		sector_pos = m_current_track[m_current_side].get_sector_pos_from_ratio(sector_ratio);
	} else {
		// get ratio from specified sector position
		sector_ratio = m_current_track[m_current_side].get_sector_ratio_from_pos(sector_pos);
	}
	if (sector_ratio >= 1.0) {
		// sector not found
		sector_ratio = (double)timeout_round;
	}
	int sum = (int)(sector_ratio * drive_type->m_round_clock) + remain_clock - drive_type->m_round_clock;
	if (sum < 0) {
		sum += drive_type->m_round_clock;
	}
#ifdef _DEBUG_D88_IMAGE
	logging->out_debugf(_T("D88::clock_arrival_sector: curr_ratio:%.4f sect_num:%02d sect_ratio:%.4f sum:%06d")
		, (double)(drive_type->m_round_clock - remain_clock) / drive_type->m_round_clock
		, sector_num
		, sector_ratio
		, sum);
#endif
	return sum;
}

// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
static const _TCHAR *c_image_types[] = {
	_T("D88 Image"),
	_T("D88 Image (Converted)"),
	_T("Plain Image"),
	_T("Unknown Image"),
	NULL
};

void DISKD88_IMAGE::debug_media_info(_TCHAR *buffer, size_t buffer_len)
{
	int image_type = p_disk->get_image_type();
	UTILITY::tcscat(buffer, buffer_len, _T(" "));
	if (image_type < DISK::IMAGE_TYPE_D88 || image_type > DISK::IMAGE_TYPE_PLAIN) {
		image_type = 3;
	}
	UTILITY::tcscat(buffer, buffer_len, c_image_types[image_type]);
	UTILITY::tcscat(buffer, buffer_len, _T(":"));

	uint8_t media_type = get_media_type();
	UTILITY::sntprintf(buffer, buffer_len, _T(" Media Type:0x%02x("), media_type);
	switch(media_type) {
	case MEDIA_TYPE_2D:
		UTILITY::tcscat(buffer, buffer_len, _T("2D"));
		break;
	case MEDIA_TYPE_2DD:
		UTILITY::tcscat(buffer, buffer_len, _T("2DD"));
		break;
	case MEDIA_TYPE_2HD:
		UTILITY::tcscat(buffer, buffer_len, _T("2HD"));
		break;
	default:
		UTILITY::tcscat(buffer, buffer_len, _T("Unknown"));
		break;
	}
	UTILITY::sntprintf(buffer, buffer_len, _T(") Number of Side:%d\n"), m_number_of_side);
	for(int side_pos = 0; side_pos < m_number_of_side; side_pos++) {
		m_current_track[side_pos].debug_media_info(buffer, buffer_len);
	}
}

void DISKD88_TRACK::debug_media_info(_TCHAR *buffer, size_t buffer_len)
{
	UTILITY::sntprintf(buffer, buffer_len, _T("  Current Track Pos:%2d Side:%d Number of Sector:%2d Raw Size:%6d\n")
		, m_track_num
		, m_side_num
		, m_sector_nums
		, m_sector_all_size
	);
	if (m_track_table_pos < 0) {
		UTILITY::tcscat(buffer, buffer_len, _T("   Track is not parsed yet.\n"));
		return;
	}
	uint8_t *t = p_track;
	if (!t) {
		UTILITY::tcscat(buffer, buffer_len, _T("   Track buffer is not found.\n"));
		return;
	}
	if (m_sector_nums <= 0) {
		UTILITY::tcscat(buffer, buffer_len, _T("   Sector is not found.\n"));
		return;
	}
	UTILITY::tcscat(buffer, buffer_len, _T("   Pos   C   H   R   N RawPos Den Del\n"));
	for(int sector_pos = 0; sector_pos < m_sector_nums; sector_pos++) {
		int sector_raw_pos;
#ifndef USE_SECTOR_LIST
			sector_raw_pos = m_sector_raw_pos[sector_pos];
#else
#ifdef USE_CPTRLIST_SECTOR
			sector_raw_pos = m_sectors.Item(sector_pos)->get_raw_pos();
#else
			sector_raw_pos = m_sectors[sector_pos].get_raw_pos();
#endif
#endif
		UTILITY::sntprintf(buffer, buffer_len, _T("   %3d %3u %3u %3u %3u %6d %c   %c\n")
			, sector_pos
			, t[0], t[1], t[2], t[3]
			, sector_raw_pos
			, t[6] ? _T('S') : _T('D')
			, t[7] ? _T('*') : _T(' ')
		);
		// next sector
		t += (t[0xe] | (t[0xf] << 8)) + 0x10;
	}
}
#endif /* USE_DEBUGGER */
