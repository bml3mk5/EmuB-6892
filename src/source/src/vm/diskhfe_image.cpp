/** @file diskhfe_image.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2025.08.10-

	@brief [ disk hfe image ]
*/

#include "diskhfe_image.h"
#include "../emu.h"
#include "../fileio.h"
#include "../utility.h"
#include "../config.h"
#include "../depend.h"
#include "../labels.h"
#include "disk_parser.h"

#ifdef _DEBUG_HFE_IMAGE
#include "../logging.h"
#endif

extern EMU *emu;

// ----------------------------------------------------------------------------

DISKHFE_SECTOR::DISKHFE_SECTOR()
	: DISK_SECTOR()
{
	clear();
}
DISKHFE_SECTOR::~DISKHFE_SECTOR()
{
}
void DISKHFE_SECTOR::clear()
{
	p_sector_id = NULL;
	p_sector_data = NULL;
	m_sector_size = 0;
//	memset(m_id, 0, sizeof(m_id));
//	m_density = 0;
	m_deleted = false;
//	m_status = 0;
}
/// @brief Set sector id from buffer
///
/// @param[in] p : sector id field in track
void DISKHFE_SECTOR::set_id_ptr(uint8_t *p)
{
	// header info
	p_sector_id = p;

	m_id[0] = p[0];
	m_id[1] = p[1];
	m_id[2] = p[2];
	m_id[3] = p[3];
	m_id[4] = p[4];
	m_id[5] = p[5];

	m_sector_size = (128 << p_sector_id[3]);
}

/// @brief Set sector data from buffer
///
/// @param[in] p : sector data field in track
/// @param[in] deleted : deleted mark detected?
void DISKHFE_SECTOR::set_data_ptr(uint8_t *p, bool deleted)
{
	p_sector_data = p;
	m_deleted = deleted;
}

/// @brief Get sector data pointer
///
/// @return pointe to sector data field in track
const uint8_t *DISKHFE_SECTOR::get_data_ptr() const
{
	return p_sector_data;
}

/// @brief Set data in current sector
///
/// @param[in] pos : position
/// @param[in] val : value
void DISKHFE_SECTOR::set_sector_data(int pos, uint8_t val)
{
	p_sector_data[pos] = val;
}

/// @brief Get data in current sector
///
/// @param[in] pos : position
/// @return value
uint8_t DISKHFE_SECTOR::get_sector_data(int pos) const
{
	return p_sector_data[pos];
}

/// @brief Get size in current sector
///
/// @return size
int DISKHFE_SECTOR::get_sector_size() const
{
	return m_sector_size;
}

/// @brief Set id in current sector
///
/// @param[in] pos : position
/// @param[in] val : value
void DISKHFE_SECTOR::set_sector_id(int pos, uint8_t val)
{
	if (0 <= pos && pos < 4) {
//		m_id[pos] = val;
		if (p_sector_id) {
			p_sector_id[pos] = val;
		}
	}
}

/// @brief Get id in current sector
///
/// @param[in] pos : position
/// @return value
uint8_t DISKHFE_SECTOR::get_sector_id(int pos) const
{
	if (0 <= pos && pos < 6) {
		return p_sector_id[pos];
	}
	return 0;
}

/// @brief set deleted mark in current sector
///
/// @param[in] val
void DISKHFE_SECTOR::set_deleted_mark(bool val)
{
	m_deleted = val;
}

/// @brief deleted mark detected in current sector
///
/// @return true if deleted mark detected
bool DISKHFE_SECTOR::has_deleted_mark() const
{
	return m_deleted;
}

/// @brief CRC error detected in current sector
///
/// @param[in] density : 1 if double density
/// @return true if error detected
bool DISKHFE_SECTOR::has_crc_error(int density) const
{
	return !compare_crc_on_data(density, m_sector_size);
}

/// @brief Calculate the CRC on the part of current sector data
/// and compare this result with the end of the data
///
/// @param[in] density : 1 if double density
/// @param[in] size : size of the part of current sector data
/// @return true if match
bool DISKHFE_SECTOR::compare_crc_on_data(int density, int size) const
{
	if (!p_sector_data || !size) {
		// not check if has no sector
		return true;
	}

	uint16_t calc_crc = CRC16_INIT_DATA;
	int st;
	if (density) {
		// double density
		st = -4;	// 0xa1 0xa1 0xa1 0xfb in p_sector_data[-4:-1]
	} else {
		st = -1;	// 0xfb in p_sector_data[-1]
	}
	for(int i=st; i<size; i++) {
		calc_crc = DISK_IMAGE::set_crc16(p_sector_data[i], calc_crc);
	}
	uint16_t data_crc = ((uint16_t)p_sector_data[size] << 8) | p_sector_data[size+1];

	return (calc_crc == data_crc);
}

// ----------------------------------------------------------------------------

DISKHFE_TRACK_INFO::DISKHFE_TRACK_INFO()
{
	clear();
}

DISKHFE_TRACK_INFO::~DISKHFE_TRACK_INFO()
{
}

void DISKHFE_TRACK_INFO::clear()
{
	m_valid = false;
	m_size = 0;
	clear_sector_table();
}

/// @brief Clear sector info table
void DISKHFE_TRACK_INFO::clear_sector_table()
{
	m_sector_valid = false;
#ifdef USE_CPTRLIST_SECTOR
	m_sectors.Clear();
#else
	m_sectors_count = 0;
#endif
//	memset(m_sector_pos, 0, sizeof(m_sector_pos));
}

/// @brief Make sector info table
void DISKHFE_TRACK_INFO::make_sector_table()
{
//	// make sector number to position table
//	for(int pos = 0; pos < m_sectors.Count(); pos++) {
//		int sector_num = m_sectors.Item(pos)->get_sector_id(2);
//		m_sector_pos[sector_num] = (uint8_t)pos;
//	}
	m_sector_valid = true;
}

/// @brief Parse sector in current track (IBM format only)
///
/// @return true if success
bool DISKHFE_TRACK_INFO::parse_sector()
{
	if (m_sector_valid) {
		return true;
	}

	clear_sector_table();

	uint8_t gapmark[16];
	int gap_len = (m_density ? 16 : 8);
//	uint8_t *index_mark = &c_index_mark[m_density][1];
	uint8_t *addr_mark = &c_address_mark[m_density][1];
	uint8_t *data_mark = &c_data_mark[m_density][1];
	uint8_t *delete_mark = &c_deleted_mark[m_density][1];
	int mark_len = c_mark_length[m_density] - 1;

	for(int i=0; i<gap_len; i++) {
		gapmark[i] = c_gap_sign[m_density];
	}

	int phase = 0;
//	int num_of_sector = 0;
//	int sec_size = 0;
	int p;
	bool rc = true;
	DISKHFE_SECTOR *sector = NULL;
	for(p = 0; p < m_size; p++) {
		switch(phase) {
		default:
			// search gap sign
			if (memcmp(&m_data[p], gapmark, gap_len) == 0) {
				p += gap_len;
				phase++;
			}
			break;
		case 1:
			// search id address mark
			if (memcmp(&m_data[p], addr_mark, mark_len) == 0) {
				// set sector
				p += mark_len;

#ifdef USE_CPTRLIST_SECTOR
				sector = new DISKHFE_SECTOR();
				sector->set_id_ptr(&m_data[p]);
				m_sectors.Add(sector);
#else
				if (m_sectors_count < MAX_SECTORS) {
					sector = &m_sectors[m_sectors_count];
					sector->set_id_ptr(&m_data[p]);
					m_sectors_count++;
				}
#endif

//				num_of_sector++;
				phase = 2;
			}
			break;
		case 3:
			// search data mark
			if (memcmp(&m_data[p], data_mark, mark_len) == 0) {
				p += mark_len;

				if (sector) {
					sector->set_data_ptr(&m_data[p], false);
					p += sector->get_sector_size();
				} else {
					// sector id not found ??
					rc = false;
				}
				sector = NULL;
				phase = 0;

			} else if (memcmp(&m_data[p], delete_mark, mark_len) == 0) {
				p += mark_len;

				if (sector) {
					sector->set_data_ptr(&m_data[p], true);
					p += sector->get_sector_size();
				} else {
					// sector id not found ??
					rc = false;
				}
				sector = NULL;
				phase = 0;
			}
			break;
		}
	}

	// make sector number -> position table
	make_sector_table();

	m_sector_valid = true;

	return rc;
}

/// @brief Get track number in sector id
uint8_t DISKHFE_TRACK_INFO::get_id_c(int pos) const
{
#ifdef USE_CPTRLIST_SECTOR
	if (pos < m_sectors.Count()) {
		return m_sectors.Item(pos)->get_sector_id(0);
	}
#else
	if (pos < m_sectors_count) {
		return m_sectors[pos].get_sector_id(0);
	}
#endif
	return 0;
}

/// @brief Verify a track number in each sector on a track
///
/// @param[in] track_num : track number
/// @return true if match
bool DISKHFE_TRACK_INFO::verify_track_number(int track_num) const
{
#ifdef USE_CPTRLIST_SECTOR
	for(int i = 0; i < m_sectors_Count(); i++) {
		if(m_sectors.Item(i)->get_sector_id(0) != track_num) {
			return false;
		}
	}
#else
	for(int i = 0; i < m_sectors_count; i++) {
		if(m_sectors[i].get_sector_id(0) != track_num) {
			return false;
		}
	}
#endif
	return true;
}

/// @brief Get the sector position in current track
///
/// @param[in] sector_num : sector number
/// @return position
uint8_t DISKHFE_TRACK_INFO::get_sector_pos(int sector_num) const
{
	int pos = 0;
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
	return pos;
}

/// @brief Get the sector position in current track
///
/// @param[in] sector_num : sector number
/// @param[in] start_pos : sector position to start to search a sector
/// @return position
uint8_t DISKHFE_TRACK_INFO::get_sector_pos(int sector_num, int start_pos) const
{
	int pos = start_pos;
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
	return pos;
}

/// @brief Get the sector position in current track
///
/// @param[in] sector_num : sector number
/// @param[in] curr_ratio : current head position in a track
/// @return position
uint8_t DISKHFE_TRACK_INFO::get_sector_pos(int sector_num, double curr_ratio) const
{
	int start_pos = get_sector_pos_from_curr_ratio(curr_ratio);
	return get_sector_pos(sector_num, start_pos);
}

/// @brief Get the sector position in current track (ratio per track size)
///
/// @param[in] sector_pos : sector position
/// @return ratio
double DISKHFE_TRACK_INFO::get_sector_ratio_from_pos(int sector_pos) const
{
#ifdef USE_CPTRLIST_SECTOR
	if (m_size > 0 && sector_pos < m_sectors.Count()) {
		const uint8_t *data_ptr = m_sectors.Item(sector_pos)->get_data_ptr();
		if (data_ptr) {
			int data_pos = (int)(data_ptr - m_data);
			return (double)data_pos / m_size;
		} else {
			return 1.0;
		}
	}
#else
	if (m_size > 0 && sector_pos < m_sectors_count) {
		const uint8_t *data_ptr = m_sectors[sector_pos].get_data_ptr();
		if (data_ptr) {
			int data_pos = (int)(data_ptr - m_data);
			return (double)data_pos / m_size;
		} else {
			return 1.0;
		}
	}
#endif
	return 1.0;
}

/// @brief Get the sector position in current track (ratio per track size)
///
/// @param[in] sector : sector number
/// @return ratio
double DISKHFE_TRACK_INFO::get_sector_ratio(int sector) const
{
	int num = get_sector_pos(sector);
	return get_sector_ratio_from_pos(num);
}

/// @brief Get the sector position from ratio
///
/// @param[in] curr_ratio : current ratio
/// @return position
int DISKHFE_TRACK_INFO::get_sector_pos_from_curr_ratio(double curr_ratio) const
{
	int head_pos = (int)(curr_ratio * m_size);
	int pos = 0;
#ifdef USE_CPTRLIST_SECTOR
	for(int i=0; i < m_sectors.Count(); i++) {
		const uint8_t *data_ptr = m_sectors.Item(i)->get_data_ptr();
		if (!data_ptr) continue; 
		int data_pos = (int)(data_ptr - m_data);
		if (head_pos < data_pos) {
			pos = i;
			break;
		}
	}
#else
	for(int i=0; i < m_sectors_count; i++) {
		const uint8_t *data_ptr = m_sectors[i].get_data_ptr();
		if (!data_ptr) continue; 
		int data_pos = (int)(data_ptr - m_data);
		if (head_pos < data_pos) {
			pos = i;
			break;
		}
	}
#endif
	return pos;
}

/// @brief Get the sector position from ratio
///
/// @param[in] ratio
/// @return position
int DISKHFE_TRACK_INFO::get_sector_pos_from_ratio(double ratio) const
{
	int num = get_sector_pos_from_curr_ratio(ratio);
	ratio = get_sector_ratio_from_pos(num);
	return num;
}

// ----------------------------------------------------------------------------

DISKHFE_TRACK::DISKHFE_TRACK()
	: DISK_TRACK()
{
	p_parent = NULL;
	clear();
}

DISKHFE_TRACK::~DISKHFE_TRACK()
{
}

/// @brief Clear valiables
void DISKHFE_TRACK::clear()
{
	p_track = NULL;
	m_track_pos = -1;
	m_track_size = 0;
	m_current_side = 0;
	for(int side=0; side<2; side++) {
		m_info[side].clear();
	}
}

/// @brief Set information of current track
///
/// @param[in] t : pointer to track data
/// @param[in] track_pos : position in track table
/// @param[in] track_size : track size
/// @return false if the same track
bool DISKHFE_TRACK::set(uint8_t *t, int track_pos, int track_size)
{
	if (p_track == t && m_track_pos == track_pos && m_track_size == track_size) {
		// the same track
		return false;
	}

	p_track = t;
	m_track_pos = track_pos;
	m_track_size = track_size;
	for(int side=0; side<2; side++) {
		m_info[side].clear();
	}
	return true;
}

/// @brief Set the pointer to disk image object
///
/// @param[in] parent : disk image
void DISKHFE_TRACK::set_parent(DISKHFE_IMAGE *parent)
{
	p_parent = parent;
}

/// @brief Get the pointer to current track
///
/// @return the pointer to current track
uint8_t *DISKHFE_TRACK::get_buffer()
{
	return p_track;
}

// IBM Format

struct st_code_string {
	int len;
	const char *str;
};

/** GAP code
 @par GAP code (FM)
 FF ->   01   01   01   01   01   01   01   01 \n
 clk   01   01   01   01   01   01   01   01   \n
       0101 0101 0101 0101 0101 0101 0101 0101 \n
 rev   1010 1010 1010 1010 1010 1010 1010 1010 -> AAAAAAAA \n

 @par GAP code (MFM)
 4E ->  0 1  0 0  1 1  1 0 \n
 clk   1 0  0 1  0 0  0 0  \n
       1001 0010 0101 0100 \n
 rev   0100 1001 0010 1010 -> 492A \n
*/
static const struct st_code_string c_bs_gap_code[2]={
	{  4, "\xaa\xaa\xaa\xaa" }, // FM	0xff 0xff
	{  2, "\x49\x2a" }			// MFM  0x4e 0x90
};

/** SYNC code
 SYNC code (FM)
 00 ->   00   00   00   00   00   00   00   00 \n
 clk   01   01   01   01   01   01   01   01   \n
       0100 0100 0100 0100 0100 0100 0100 0100 \n
 rev   0010 0010 0010 0010 0010 0010 0010 0010 -> 22222222 \n

 @par SYNC code (MFM)
 00 ->  0 0  0 0  0 0  0 0 \n
 clk   1 1  1 1  1 1  1 1  \n
       1010 1010 1010 1010 \n
 rev   0101 0101 0101 0101 -> 5555 \n
*/
static const struct st_code_string c_bs_sync_code[2]={
	{  4, "\x22\x22\x22\x22" }, // FM	0x00 0xff
	{  2, "\x55\x55" }			// MFM  0x00 0xff
};
/// SYNC code 4bytes
static const struct st_code_string c_bs_synclong_code[2]={
	{  16, "\x22\x22\x22\x22\x22\x22\x22\x22\x22\x22\x22\x22\x22\x22\x22\x22" }, // FM 	 0x00000000
	{  16, "\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55" }  // MFM  0x0000000000000000
};

/** Index Mark
@par INDEX mark (FM)
 FC ->   01   01   01   01   01   01   00   00 \n
 clk   01   01   0x   01   0x   01   01   01   \n
       0101 0101 0001 0101 0001 0101 0100 0100 \n
 rev   1010 1010 1010 1000 1010 1000 0010 0010 -> aaa8a822 \n

@par PRE IDX (MFM)
 C2 ->  1 1  0 0  0 0  1 0 \n
 clk   0 0  0 1  x 1  0 0  \n
       0101 0010 0010 0100 \n
 rev   0100 1010 0010 0100 -> 4a24 \n

@par INDEX mark (MFM)
 FC ->  1 1  1 1  1 1  0 0 \n
 clk   0 0  0 0  0 0  0 1  \n
       0101 0101 0101 0010 \n
 rev   1010 1010 0100 1010 -> aa4a \n
*/
//static const struct st_code_string c_bs_index_mark[2]={
//	{  4, "\xaa\xa8\xa8\x22" },				    // FM  0xfc       0xd7
//	{  8, "\x4a\x24\x4a\x24\x4a\x24\xaa\x4a" }  // MFM 0xc2c2c2fc 0x14141401
//};

/** Address Mark
@par ID ADDRESS mark (FM)
 FE ->   01   01   01   01   01   01   01   00 \n
 clk   01   01   0x   0x   0x   01   01   01   \n
       0101 0101 0001 0001 0001 0101 0101 0100 \n
 rev   1010 1010 1000 1000 1010 1000 0010 1010 -> aa88a82a \n

 @par PRE AM (MFM)
 A1 ->  1 0  1 0  0 0  0 1 \n
 clk   0 0  0 0  1 x  1 0  \n
       0100 0100 1000 1001 \n
 rev   0010 0010 1001 0001 -> 2291 \n

@par ID ADDRESS mark (MFM)
 FE ->  1 1  1 1  1 1  1 0 \n
 clk   0 0  0 0  0 0  0 0  \n
       0101 0101 0101 0100 \n
 rev   1010 1010 0010 1010 -> aa2a \n
*/
//static const struct st_code_string c_bs_address_mark[2]={
//	{  4, "\xaa\x88\xa8\x2a" },				    // FM  0xfe       0xc7
//	{  8, "\x22\x91\x22\x91\x22\x91\xaa\x2a" }  // MFM 0xa1a1a1fe 0x0a0a0a00
//};

/** Data Mark
@par DATA mark (FM)
 FB ->   01   01   01   01   01   00   01   01 \n
 clk   01   01   0x   0x   0x   01   01   01   \n
       0101 0101 0001 0001 0001 0100 0101 0101 \n
 rev   1010 1010 1000 1000 0010 1000 1010 1010 -> aa8828aa \n

 @par DATA mark (MFM)
 FB ->  1 1  1 1  1 0  1 1 \n
 clk   0 0  0 0  0 0  0 0  \n
       0101 0101 0100 0101 \n
 rev   1010 1010 1010 0010 -> aaa2 \n
*/
//static const struct st_code_string c_bs_data_mark[2]={
//	{  4, "\xaa\x88\x28\xaa" },					// FM  0xfb       0xc7
//	{  8, "\x22\x91\x22\x91\x22\x91\xaa\xa2" }  // MFM 0xa1a1a1fb 0x0a0a0a00
//};

/** Deleted Data Mark
@par Deleted DATA mark (FM)
 F8 ->   01   01   01   01   01   00   00   00 \n
 clk   01   01   0x   0x   0x   01   01   01   \n
       0101 0101 0001 0001 0001 0100 0100 0100 \n
 rev   1010 1010 1000 1000 0010 1000 0010 0010 -> aa882822 \n

 @par Deleted DATA mark (MFM)
 F8 ->  1 1  1 1  1 0  0 0 \n
 clk   0 0  0 0  0 0  1 1  \n
       0101 0101 0100 1010 \n
 rev   1010 1010 0101 0010 -> aa52 \n
*/
//static const struct st_code_string c_bs_deleted_data_mark[2]={
//	{  4, "\xaa\x88\x28\x22" },					// FM  0xf8       0xc7
//	{  8, "\x22\x91\x22\x91\x22\x91\xaa\x52" }  // MFM 0xa1a1a1f8 0x0a0a0a03
//};

/// data list included missing clock (FM)
static const struct st_code_string c_bs_missing_mark_fm[]={
	{  4, "\xaa\x88\xa8\x2a" }, // FM  0xfe       0xc7
	{  4, "\xaa\x88\x28\xaa" }, // FM  0xfb       0xc7
	{  4, "\xaa\x88\x28\x22" },	// FM  0xf8       0xc7
	{  4, "\xaa\xa8\xa8\x22" }, // FM  0xfc       0xd7
	{  0, NULL }
};
/// data list included missing clock (MFM)
static const struct st_code_string c_bs_missing_mark_mfm[]={
	{  6, "\x22\x91\x22\x91\x22\x91" }, // MFM 0xa1a1a1 0x0a0a0a
	{  6, "\x4a\x24\x4a\x24\x4a\x24" }, // MFM 0xc2c2c2 0x141414
	{  0, NULL }
};
/// data list included missing clock
static const struct st_code_string *c_bs_missing_mark[2] = {
	c_bs_missing_mark_fm,
	c_bs_missing_mark_mfm
};

/// byte shift in buffer
/// @param [in,out] data : data buffer
/// @param [in]     len  : data length
/// @param [in]     shift_cnt : shift count
/// @return data length in buffer after shifted
int DISKHFE_TRACK::shift_bytes(uint8_t *data, int len, int shift_cnt)
{
	if (shift_cnt <= 0) return len;

	int endpos = len - shift_cnt;

	for(int i=0; i<endpos; i++) {
		data[i] = data[i + shift_cnt];
	}
	data[endpos] = 0x00;

	len -= shift_cnt;

	return len;
}

/// bit shift in buffer
/// @param [in,out] data : data buffer
/// @param [in]     len  : data length
/// @param [in]     shift_cnt : shift count
/// @return data length in buffer after shifted
int DISKHFE_TRACK::shift_bits(uint8_t *data, int len, int shift_cnt)
{
	if (shift_cnt <= 0) return len;

	int divn = (shift_cnt >> 3);
	int modn = (shift_cnt & 7);

	// lshift bytes
	if (divn > 0) {
		len = shift_bytes(data, len, divn);
	}

	if (modn == 0) return len;

	// bit shift
	uint8_t carry = 0x00;
	for(int i=len-1; i>=0; i--) {
		uint8_t c = (data[i] << (8-modn));
		data[i] >>= modn;
		data[i] |= carry;
		carry = c;
	}

	return len;
}

/// @brief Decode one data (MFM)
/// @param [in] indata 2bytes
/// @return decorded data
uint8_t DISKHFE_TRACK::decode_mfm_data(const uint8_t *indata)
{
	uint8_t outdata;
	outdata = ((indata[0] & 0x80) >> 3) | ((indata[0] & 0x20)) | ((indata[0] & 0x08) << 3) | ((indata[0] & 0x02) << 6);
	outdata |= ((indata[1] & 0x80) >> 7) | ((indata[1] & 0x20) >> 4) | ((indata[1] & 0x08) >> 1) | ((indata[1] & 0x02) << 2);

	return outdata;
}

/// @brief Decode one data (FM)
/// @param [in] indata 4bytes
/// @return decorded data
uint8_t DISKHFE_TRACK::decode_fm_data(const uint8_t *indata)
{
	uint8_t outdata;
	outdata = ((indata[0] & 0x80) >> 1) | ((indata[0] & 0x08) << 4);
	outdata |= ((indata[1] & 0x80) >> 3) | ((indata[1] & 0x08) << 2);
	outdata |= ((indata[2] & 0x80) >> 5) | ((indata[2] & 0x08));
	outdata |= ((indata[3] & 0x80) >> 7) | ((indata[3] & 0x08) >> 2);

	return outdata;
}

/// @brief Copy bitstream to buffer
///
/// @param [out] dst : buffer
/// @param [in] src : bitstream
/// @param [in] pos : start position in bitstream
/// @param [in] len : length to copy bytes
void DISKHFE_TRACK::copy_bitstream(uint8_t *dst, const uint8_t *src, int pos, int len)
{
	memcpy(dst, &src[pos], len + 1);
	if ((pos & 0xff) + len >= 256) {
		pos += 256;
		int n = 256 - (pos & 0xff);
		memcpy(&dst[n], &src[pos + n], len + 1 - n);
	}
}

#define INCREMENT_POS(pos, len) \
	if ((pos & 0xff) + (len) >= 256) pos += 256; \
	pos += (len);

#define DECREMENT_POS(pos, len) \
	if ((pos & 0xff) < (len)) pos -= 256; \
	pos -= (len);

void DISKHFE_TRACK::decode_bitstream(DISKHFE_TRACK_INFO *info, int idata_pos, int idata_sft, int end_idata_pos, int len)
{
	int density = info->m_density;
	int odata_pos = info->m_size;
	uint8_t *odata = info->m_data;
	uint8_t buf[16];

	int adjust_pos = ((end_idata_pos - idata_pos) % len); 
	if (adjust_pos > 0) {
		// shrink
		DECREMENT_POS(end_idata_pos, adjust_pos);
	}

#ifdef _DEBUG_HFE_DECODE
	_TCHAR indump_str[2][8][12];
	_TCHAR outdump_str[DISKHFE_TRACK_INFO::TRACK_BUFFER_SIZE * 4];
	int inn0 = 0;

	outdump_str[0] = _T('\0');
	logging->out_debugf(_T("I:%d-%d=%d SFT:%d A:%d"), end_idata_pos, idata_pos, end_idata_pos - idata_pos, idata_sft, adjust_pos);
#endif

	for(; idata_pos < end_idata_pos && odata_pos < DISKHFE_TRACK_INFO::TRACK_BUFFER_SIZE;) {
		copy_bitstream(buf, p_track, idata_pos, len);
		shift_bits(buf, len + 1, idata_sft);
		if (density) {
			odata[odata_pos] = decode_mfm_data(buf);
		} else {
			odata[odata_pos] = decode_fm_data(buf);
		}

#ifdef _DEBUG_HFE_DECODE
		if (inn0 < 8) {
			UTILITY::stprintf(indump_str[0][inn0], 12, _T(" %02x%02x%02x%02x"), buf[0],buf[1],buf[2],buf[3]);
			inn0++;
		}
		for(int inn1=0; inn1<7; inn1++) {
			memcpy(indump_str[1][inn1], indump_str[1][inn1+1], 12);
		}
		UTILITY::stprintf(indump_str[1][7], 12, _T(" %02x%02x%02x%02x"), buf[0],buf[1],buf[2],buf[3]);
		UTILITY::sntprintf(outdump_str, DISKHFE_TRACK_INFO::TRACK_BUFFER_SIZE * 4, _T(" %02x"), odata[odata_pos]);
#endif
		INCREMENT_POS(idata_pos, len);
		odata_pos++;
	}

#ifdef _DEBUG_HFE_DECODE
	logging->out_debugf(_T("I:%d O:%d"), idata_pos, odata_pos);
	logging->out_debugf(_T("%s%s%s%s%s%s%s%s")
		,indump_str[0][0],indump_str[0][1],indump_str[0][2],indump_str[0][3]
		,indump_str[0][4],indump_str[0][5],indump_str[0][6],indump_str[0][7]);
	logging->out_debugf(_T("%s%s%s%s%s%s%s%s")
		,indump_str[1][0],indump_str[1][1],indump_str[1][2],indump_str[1][3]
		,indump_str[1][4],indump_str[1][5],indump_str[1][6],indump_str[1][7]);
	logging->out_debug(outdump_str);
#endif

	info->m_size = odata_pos;
}

/// @brief Decode binary data from bitstream
///
/// @param [in] side : side number
/// @param [in] density : 1 if double density
/// @return true if success
bool DISKHFE_TRACK::decode_track(int side, int density)
{
	uint8_t buf[20];

	side &= 1;
	DISKHFE_TRACK_INFO *info = &m_info[side];

	m_current_side = side;

	density &= 1;
	if (info->m_density != density) {
		// different density, so decode again
		info->m_valid = false;
		info->clear_sector_table();
	}
	info->m_density = density;

	// already decoded?
	if (info->m_valid) {
		return true;
	}

	info->m_size = 0;
	const struct st_code_string *gap = &c_bs_gap_code[density];
	const struct st_code_string *sync = &c_bs_sync_code[density];
	const struct st_code_string *synclong = &c_bs_synclong_code[density];
	const struct st_code_string *missmark = c_bs_missing_mark[density];

	int indata_pos = (side * 256);
	int indata_sft = 0;

	int start_indata_pos = indata_pos;
	int start_indata_sft = indata_sft;

	int phase = 0;
	bool found;
	while(indata_pos < m_track_size) {
		switch(phase) {
		default:
			// Search SYNC 4bytes
			copy_bitstream(buf, p_track, indata_pos, synclong->len);
			if (memcmp(buf, synclong->str, synclong->len) == 0) {
				// match
				INCREMENT_POS(indata_pos, synclong->len - sync->len);
				phase = 1;
			} else {
				// unmatch
				INCREMENT_POS(indata_pos, 1);
			}
			break;
		case 1:
			// Search the end of SYNC
			copy_bitstream(buf, p_track, indata_pos, sync->len);
			if (memcmp(buf, sync->str, sync->len) == 0) {
				// match
				INCREMENT_POS(indata_pos, 1);
			} else {
				// unmatch (AM included ?)
				INCREMENT_POS(indata_pos, 3);
				phase = 2;
			}
			break;
		case 2:
			// Include missing clock?
			for(int n=0; n<missmark[0].len && indata_pos < m_track_size; n++) {
				copy_bitstream(buf, p_track, indata_pos, missmark[0].len);
				found = false;
				for(indata_sft = 0; indata_sft<8; indata_sft++) {
					for(int i=0; missmark[i].len != 0; i++) {
						if (memcmp(buf, missmark[i].str, missmark[i].len) == 0) {
							// match
							found = true;
							break;
						}
					}
					if (found) {
						break;
					}
					// bit shift left
					shift_bits(buf, missmark[0].len + 1, 1);
				}
				if (found) {
					break;
				}
				INCREMENT_POS(indata_pos, 1);
			}
			if (found) {
				// include the sync data just before the current position
				DECREMENT_POS(indata_pos, sync->len);
				phase = 3;
			} else {
				// missing clock not found
				phase = 0;
			}
			break;
		case 3:
			// output part of bitstream
			decode_bitstream(info, start_indata_pos, start_indata_sft, indata_pos, gap->len);
			start_indata_pos = indata_pos;
			start_indata_sft = indata_sft;
			phase = 0;
			break;
		}
	}

	// output
	decode_bitstream(info, start_indata_pos, start_indata_sft, indata_pos, gap->len);

	info->m_valid = true;

	return true;
}

/// @brief Get number of sector
///
/// @return number of sector
int DISKHFE_TRACK::get_sector_nums() const
{
#ifdef USE_CPTRLIST_SECTOR
	return m_info[m_current_side].m_sectors.Count();
#else
	return m_info[m_current_side].m_sectors_count;
#endif
}

/// @brief Verify a track number in each sector on a track
///
/// @param[in] track_num : track number
/// @return true if match
bool DISKHFE_TRACK::verify_track_number(int track_num) const
{
	return m_info[m_current_side].verify_track_number(track_num);
}

/// @brief Get ID_C (track) number in current track
///
/// @param[in] sector_pos : sector position
/// @return ID_C (track) number
uint8_t DISKHFE_TRACK::get_id_c_in_track(int sector_pos) const
{
	return m_info[m_current_side].get_id_c(sector_pos);
}

/// @brief Get the sector position in current track
///
/// @param[in] sector_num : sector number
/// @return position
uint8_t DISKHFE_TRACK::get_sector_pos(int sector_num) const
{
	return m_info[m_current_side].get_sector_pos(sector_num);
}

/// @brief Get the sector position in current track
///
/// @param[in] sector_num : sector number
/// @param[in] start_pos : sector position to start to search a sector
/// @return position
uint8_t DISKHFE_TRACK::get_sector_pos(int sector_num, int start_pos) const
{
	return m_info[m_current_side].get_sector_pos(sector_num, start_pos);
}

/// @brief Get the sector position in current track
///
/// @param[in] sector     : sector number
/// @param[in] curr_ratio : current head position in a track
/// @return position
uint8_t DISKHFE_TRACK::get_sector_pos(int sector, double curr_ratio) const
{
	return m_info[m_current_side].get_sector_pos(sector, curr_ratio);
}

/// @brief Get raw position ratio at sector position
///
/// @param[in] sector_pos : sector position
/// @return ratio
double DISKHFE_TRACK::get_sector_ratio_from_pos(int sector_pos) const
{
	return m_info[m_current_side].get_sector_ratio_from_pos(sector_pos);
}

/// @brief Get the sector position in current track (ratio per track size)
///
/// @param[in] sector_num : sector number
/// @return ratio
double DISKHFE_TRACK::get_sector_ratio(int sector_num) const
{
	return m_info[m_current_side].get_sector_ratio(sector_num);
}

/// @brief Get the sector position from ratio
///
/// @param[in] ratio
/// @return position
int DISKHFE_TRACK::get_sector_pos_from_ratio(double ratio) const
{
	return m_info[m_current_side].get_sector_pos_from_ratio(ratio);
}

/// @brief Make a raw track from bitstream
///
/// @param[in] side        : side number
/// @param[in] density : bit0: 0:single density(FM) 1:double density(MFM)
/// @return true if success
bool DISKHFE_TRACK::make_track_from_bitstream(int side, int density)
{
	if (!p_track) {
		return false;
	}
	if (!decode_track(side, density)) {
		return false;
	}
	return true;
}

/// @brief Parse sector in current track
///
/// @param[in] side_num : side number
/// @param[in] density : bit0: 0:single density(FM) 1:double density(MFM)
void DISKHFE_TRACK::parse_sector(int side_num, int density)
{
	make_track_from_bitstream(side_num, density);
	m_info[m_current_side].parse_sector();
}

/// @brief Get sector info
///
/// @param[in] sector_pos : position of sector in the track
/// @return sector info
DISKHFE_SECTOR *DISKHFE_TRACK::get_sector_by_pos(int sector_pos)
{
	DISKHFE_TRACK_INFO *info = &m_info[m_current_side];
#ifdef USE_CPTRLIST_SECTOR
	if (sector_pos < info->m_sectors.Count()) {
		return info->m_sectors.Item(sector_pos);
	}
#else
	if (sector_pos < info->m_sectors_count) {
		return &info->m_sectors[sector_pos];
	}
#endif
	return NULL;
}

/// @brief Set raw track data
///
/// @param[in] pos : position
/// @param[in] val : value
void DISKHFE_TRACK::set_raw_track_data(int pos, uint8_t val)
{
	m_info[m_current_side].m_data[pos] = val;
}

/// @brief Get raw track data
///
/// @param[in] pos : position
/// @return value
uint8_t DISKHFE_TRACK::get_raw_track_data(int pos) const
{
	return m_info[m_current_side].m_data[pos];
}

/// @brief Set raw track size
///
/// @param[in] val : size
void DISKHFE_TRACK::set_raw_track_size(int val)
{
	m_info[m_current_side].m_size = val;
}

/// @brief Set raw track size
///
/// @return size
int DISKHFE_TRACK::get_raw_track_size() const
{
	return m_info[m_current_side].m_size;
}


// ----------------------------------------------------------------------------

DISKHFE_IMAGE::DISKHFE_IMAGE()
	: DISK_IMAGE()
{
	clear();
}

DISKHFE_IMAGE::DISKHFE_IMAGE(const DISKHFE_IMAGE &src)
	: DISK_IMAGE(src)
{
	clear();
}

DISKHFE_IMAGE::DISKHFE_IMAGE(DISK *disk, uint8_t *buffer)
	: DISK_IMAGE(disk)
{
	clear();
	set(disk, buffer);
}

DISKHFE_IMAGE::~DISKHFE_IMAGE()
{
}

/// @brief Clear valiables
void DISKHFE_IMAGE::clear()
{
	DISK_IMAGE::clear_base();

//	m_media_type = MEDIA_TYPE_UNK;
	p_buffer = NULL;
	p_track_list = NULL;
	m_number_of_side = 2;

	m_current_track.set_parent(this);
	p_current_sector = NULL;
}

/// @brief Set buffer pointer
///
/// @param[in] disk
/// @param[in] buffer
void DISKHFE_IMAGE::set(DISK *disk, uint8_t *buffer)
{
	DISK_IMAGE::set_base(disk);

	p_buffer = buffer;

	hfe_header_t *header = (hfe_header_t *)p_buffer;
	int track_list_offset = Uint16_LE(header->track_list_offset) * 512;

	p_track_list = (hfe_track_offset_list_t *)(p_buffer + track_list_offset);
}

/// @brief Get write protected
///
/// @return true if write protected
bool DISKHFE_IMAGE::is_write_protected() const
{
	hfe_header_t *header = (hfe_header_t *)p_buffer;

	return (header->write_allowed == 0);
}

/// @brief Set write protect
///
/// @param[in] val : true: set the protection
void DISKHFE_IMAGE::set_write_protect(bool val)
{
	hfe_header_t *header = (hfe_header_t *)p_buffer;

	// set write protect flag
	header->write_allowed = val ? 0 : 0xff;
}

/// @brief Write operation is supported?
///
/// @return true if writable 
bool DISKHFE_IMAGE::is_write_supported() const
{
	return false;
}

/// @brief Get media type
///
/// @return media type
uint8_t DISKHFE_IMAGE::get_media_type() const
{
	return 0;
}

/// @brief Get interface mode
///
/// @return interface mode
uint8_t DISKHFE_IMAGE::get_interface_mode() const
{
	hfe_header_t *header = (hfe_header_t *)p_buffer;
	return header->interface_mode;
}

/// @brief Get encoding mode
///
/// @param[in] num : -1:default encoding 0:encoding on track0 and side0 1:encoding on track0 and side1
/// @return encoding mode
uint8_t DISKHFE_IMAGE::get_encoding(int num) const
{
	hfe_header_t *header = (hfe_header_t *)p_buffer;
	switch(num) {
	case 0:
		return header->track0s0_encode_enable != 0xff ?header->track0s0_encode : DISALE_ENCODING;
	case 1:
		return header->track0s1_encode_enable != 0xff ?header->track0s1_encode : DISALE_ENCODING;
	default:
		return header->encoding;
	}
}

/// @brief Write a disk image
///
/// @param[in] fio : file handle object
/// @param[in] new_file : save as new file
/// @param[in] is_plain : save as plain image
/// @param[in] is_last_volume : last volume in the file
/// @return true if success
bool DISKHFE_IMAGE::write_file(FILEIO *fio, bool new_file, bool is_plain, bool is_last_volume)
{
	// not supported
	return false;
}

/// @brief Get specified track buffer
///
/// @param[in] track : track number
/// @param[out] size : track size
/// @return : buffer position of specified track in a disk
uint8_t* DISKHFE_IMAGE::get_track_buffer(int track, int &size)
{
	// search track
	if (!p_track_list) {
		return NULL;
	}

	int offset = Uint16_LE(p_track_list->at[track].offset);
	if (offset >= 65535) {
		// track not found
		return NULL;
	}

	size = Uint16_LE(p_track_list->at[track].track_len);
	if (size <= 0 || size >= 65535) {
		// track not found
		return NULL;
	}

	// track found
	offset *= 512;
	uint8_t *t = p_buffer + offset;

	return t;
}

/// @brief Check media type on the disk
///
/// @return true if match
bool DISKHFE_IMAGE::check_media_type()
{
	if (FLG_CHECK_FDMEDIA) {
		uint8_t drive_type = p_disk->get_drive_type()->m_drive_type;
		hfe_header_t *header = (hfe_header_t *)p_buffer;
		int bit_rate = Uint16_LE(header->bit_rate);
		switch(drive_type) {
		case DRIVE_TYPE::DRIVE_TYPE_2D:
		case DRIVE_TYPE::DRIVE_TYPE_2DD:
			return (125 < bit_rate && bit_rate <= 250);
		case DRIVE_TYPE::DRIVE_TYPE_2HD_300:
		case DRIVE_TYPE::DRIVE_TYPE_2HD_360:
			return (250 < bit_rate && bit_rate <= 500);
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
bool DISKHFE_IMAGE::check_tracks()
{
	hfe_header_t *header = (hfe_header_t *)p_buffer;

	m_number_of_side = header->sides;
	if (m_number_of_side > 2) m_number_of_side = 2;

	return true;
}

/// @brief Get number of side
///
/// @return number of side
int DISKHFE_IMAGE::get_number_of_side() const
{
	return m_number_of_side;
}

/// @brief Set side position
///
/// @param[in] side_pos : side
void DISKHFE_IMAGE::set_side_position(int side_pos)
{
	// nothing to do
}

// ----------------------------------------------------------------------------

/// @brief Get current track number
///
/// @return track number
/// @note need call search_track() before invoke this
int DISKHFE_IMAGE::get_current_track_number() const
{
	return m_current_track.get_id_c_in_track(0);
}

/// @brief Get current track
///
/// @return track info
/// @note need call search_track() before invoke this
DISK_TRACK *DISKHFE_IMAGE::get_current_track()
{
	return &m_current_track;
}

/// @brief Get number of sector in current track
///
/// @return number of sector
/// @note need call search_track() before invoke this
int DISKHFE_IMAGE::get_number_of_sector() const
{
	return m_current_track.get_sector_nums();
}

/// @brief Get raw track size on current track
///
/// @return size
/// @note need call search_track() before invoke this
int DISKHFE_IMAGE::get_current_track_size() const
{
	return m_current_track.get_raw_track_size();
}

/// @brief Get specified track
///
/// @param[in] track_pos : track physical position
/// @param[in] side_pos : side physical position
/// @param[in] density : 0:single density 1:double density
/// @return : false : not found or invalid 
bool DISKHFE_IMAGE::search_track(int track_pos, int side_pos, int density)
{
	// search track
	int size = 0;
	uint8_t* t = get_track_buffer(track_pos, size);
	if (t) {
		// track found

		// set current track
		if (m_current_track.set(t, track_pos, size)) {
			p_current_sector = NULL;
		}
		// decode track data
		if (!m_current_track.decode_track(side_pos, density)) {
			// track not found
			return false;
		}
	} else {
		// track not found
		m_current_track.set(t, track_pos, size);
		return false;
	}
	return true;
}

/// @brief Verify a track number in each sector on a track
///
/// @param[in] track_num : track number
/// @param[in] side_num : side number
/// @param[in] density : 0:single density 1:double density
/// @return true if match track number on all sectors in the track
bool DISKHFE_IMAGE::verify_track_number(int track_num, int side_num, int density)
{
	return m_current_track.verify_track_number(track_num);
}

/// @brief Make raw track
/// (not supported)
///
/// @param[in] side_num : side number
/// @param[in] density : density
/// @return true
bool DISKHFE_IMAGE::make_track(int side_num, int density)
{
	return true;
}

/// @brief Parse raw track
/// (not supported)
///
/// @param[in] side_num : side number
/// @param[in] density : density
/// @return true
bool DISKHFE_IMAGE::parse_track(int side_num, int density)
{
	return true;
}

// ----------------------------------------------------------------------------

/// @brief Get current sector position
///
/// @return sector number
/// @note need call parse_sector() before invoke this
int DISKHFE_IMAGE::get_current_sector_pos(int sector) const
{
	return m_current_track.get_sector_pos(sector);
}

/// @brief Get current sector
///
/// @return sector info
/// @note need call search_track() and search_sector() before invoke this
DISK_SECTOR *DISKHFE_IMAGE::get_current_sector()
{
	return p_current_sector;
}

/// @brief Get number of sector in current track
///
/// @return number of sector
/// @note need call search_track() before invoke this
int DISKHFE_IMAGE::get_current_sector_size() const
{
	return p_current_sector ? p_current_sector->get_sector_size() : 0;
}

/// @brief Parse sector in current track
///
/// @param[in] side_num : side number
/// @param[in] density : 1 if double density
void DISKHFE_IMAGE::parse_sector(int side_num, int density)
{
	// parse track and find sector
	m_current_track.parse_sector(side_num, density);
}

/// @brief Finds a sector from specified sector number.
///
/// @param[in] side_num : side number
/// @param[in] density : 1 if double density
/// @param[in] sector_num : sector number or -1 if specify sector position
/// @param[in,out] sector_pos : position of sector in the track. if specify -1 then sector position 0
/// @return DISK::en_result_search_sector
int DISKHFE_IMAGE::search_sector(int side_num, int density, int sector_num, int &sector_pos)
{
	// search track
	int rc = has_current_track_buffer();
	if (rc != DISK::RES_NO_ERROR) {
		// track not found
		return rc;
	}

	// track found

	if (sector_num >= 0) {
		sector_pos = m_current_track.get_sector_pos(sector_num);
	} else if (sector_pos < 0) {
		sector_pos = 0;
	}

	// set current sector
	return set_current_sector(sector_pos);
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
int DISKHFE_IMAGE::search_sector_and_get_clock(int side_num, int density, int sector_num, int &sector_pos, int remain_clock, int timeout_round, int &arrive_clock)
{
	const DRIVE_TYPE *drive_type = p_disk->get_drive_type();

	if (remain_clock < 0) remain_clock += drive_type->m_round_clock;

	// search track
	int rc = has_current_track_buffer();
	if (rc != DISK::RES_NO_ERROR) {
		// track not found
		arrive_clock = drive_type->m_round_clock * timeout_round + remain_clock;
		return rc;
	}

	// track found

	if (sector_num >= 0) {
		sector_pos = m_current_track.get_sector_pos(sector_num, (double)(drive_type->m_round_clock - remain_clock) / (double)drive_type->m_round_clock);
	} else if (sector_pos < 0) {
		sector_pos = m_current_track.get_sector_pos_from_ratio((double)(drive_type->m_round_clock - remain_clock) / (double)drive_type->m_round_clock);
	}

	// set current sector
	rc = set_current_sector(sector_pos);
	if (rc != DISK::RES_NO_ERROR) {
		arrive_clock = drive_type->m_round_clock * timeout_round + remain_clock;
		return rc;
	}

	// calcurate time
	arrive_clock = get_clock_from_sector_pos(sector_pos, drive_type, remain_clock, timeout_round);

	return DISK::RES_NO_ERROR;
} //

/// @brief Check media type and the buffer of current track 
///
/// @return DISK::RES_NO_ERROR, DISK::RES_INVALID_MEDIA_TYPE or DISK::RES_SECTOR_NOT_FOUND
int DISKHFE_IMAGE::has_current_track_buffer()
{
	// valid media type?
	if(!check_media_type()) {
		return DISK::RES_INVALID_MEDIA_TYPE;
	}

	// search track
	uint8_t* t = m_current_track.get_buffer();
	if (!t) {
		// track not found
		return DISK::RES_SECTOR_NOT_FOUND;
	}

	return DISK::RES_NO_ERROR;
}

/// @brief Set the sector as current and check density
///
/// @param[in] sector_pos : position of sector in the track
/// @return DISK::RES_NO_ERROR or DISK::RES_SECTOR_NOT_FOUND
int DISKHFE_IMAGE::set_current_sector(int sector_pos)
{
	if(sector_pos >= m_current_track.get_sector_nums()) {
		return DISK::RES_SECTOR_NOT_FOUND;
	}

	// set current sector
	p_current_sector = m_current_track.get_sector_by_pos(sector_pos);

	return DISK::RES_NO_ERROR;
}

/// @brief Calculate the time (clocks) required to reach the sepcified sector position.
///
/// @param[in] sector_pos : position of sector in the track
/// @param[in] drive_type : type of this drive
/// @param[in] remain_clock : clock until reach index hole 
/// @param[in] timeout_round : number of round to occur timeout 
/// @return clock
int DISKHFE_IMAGE::get_clock_from_sector_pos(int sector_pos, const DRIVE_TYPE *drive_type, int remain_clock, int timeout_round)
{
	double sector_ratio = m_current_track.get_sector_ratio_from_pos(sector_pos);

	int arrive_clock = (int)(sector_ratio * drive_type->m_round_clock) + remain_clock - drive_type->m_round_clock;
	if (arrive_clock < 0) {
		arrive_clock += drive_type->m_round_clock;
	}
#ifdef _DEBUG_HFE_IMAGE
	logging->out_debugf(_T("HFE::get_clock_from_sector_pos: curr_ratio:%.4f sect_pos:%02d sect_ratio:%.4f sum:%06d")
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
int DISKHFE_IMAGE::get_clock_arrival_sector(int sector_num, int sector_pos, int remain_clock, int timeout_round)
{
	const DRIVE_TYPE *drive_type = p_disk->get_drive_type();

	if (remain_clock < 0) remain_clock += drive_type->m_round_clock;

	double sector_ratio;
	if (sector_num >= 0) {
		// get ratio from specified sector number
		sector_ratio = m_current_track.get_sector_ratio(sector_num);
	} else if (sector_pos < 0) {
		// get current head position
		sector_ratio = (double)(drive_type->m_round_clock - remain_clock) / drive_type->m_round_clock;
		// get nearest sector position
		sector_pos = m_current_track.get_sector_pos_from_ratio(sector_ratio);
	} else {
		// get ratio from specified sector position
		sector_ratio = m_current_track.get_sector_ratio_from_pos(sector_pos);
	}
	if (sector_ratio >= 1.0) {
		// sector not found
		sector_ratio = (double)timeout_round;
	}
	int sum = (int)(sector_ratio * drive_type->m_round_clock) + remain_clock - drive_type->m_round_clock;
	if (sum < 0) {
		sum += drive_type->m_round_clock;
	}
#ifdef _DEBUG_HFE_IMAGE
	logging->out_debugf(_T("HFE::clock_arrival_sector: curr_ratio:%.4f sect_num:%02d sect_ratio:%.4f sum:%06d")
		, (double)(drive_type->m_round_clock - remain_clock) / drive_type->m_round_clock
		, sector_num
		, sector_ratio
		, sum);
#endif
	return sum;
}

// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
static const _TCHAR *c_interface_modes[] = {
	_T("IBMPC DD"),
	_T("IBMPC HD"),
	_T("ATARIST DD"),
	_T("ATARIST HD"),
	_T("AMIGA DD"),
	_T("AMIGA HD"),
	_T("CPC DD"),
	_T("GENERIC SHUGGART DD"),
	_T("IBMPC ED"),
	_T("MSX2 DD"),
	_T("C64 DD"),
	_T("EMU SHUGART"),
	_T("S950 DD"),
	_T("S950 HD"),
	_T("S950 DD HD"),
	_T("IBMPC DD HD"),
	_T("QUICKDISK"),
	_T("Unknown"),
	NULL
};

static const _TCHAR *c_encoding[] = {
	_T("ISOIBM MFM"),
	_T("AMIGA MFM"),
	_T("ISOIBM FM"),
	_T("EMU FM"),
	_T("TYCOM FM"),
	_T("MEMBRAIN MFM"),
	_T("APPLEII GCR1"),
	_T("APPLEII GCR2"),
	_T("APPLEII HDDD A2 GCR1"),
	_T("APPLEII HDDD A2 GCR2"),
	_T("ARBURGDAT"),
	_T("ARBURGSYS"),
	_T("AED6200P MFM"),
	_T("NORTHSTAR HS MFM"),
	_T("HEATHKIT HS FM"),
	_T("DEC RX02 M2FM"),
	_T("APPLEMAC GCR"),
	_T("QD MO5"),
	_T("C64 GCR"),
	_T("VICTOR9K GCR"),
	_T("MICRALN HS FM"),
	_T("CENTURION MFM"),
	_T("Unknown"),
	_T("Disable"),
	NULL
};

void DISKHFE_IMAGE::debug_media_info(_TCHAR *buffer, size_t buffer_len)
{
	UTILITY::tcscat(buffer, buffer_len, _T(" HFE Image: "));
	UTILITY::sntprintf(buffer, buffer_len, _T("Number of Side:%d\n"), m_number_of_side);

	uint8_t interface_mode = get_interface_mode();
	UTILITY::sntprintf(buffer, buffer_len, _T("  Interface Mode:0x%02x("), interface_mode);
	if (interface_mode > FLOPPYMODE_MAX) {
		interface_mode = FLOPPYMODE_MAX;
	}
	UTILITY::tcscat(buffer, buffer_len, c_interface_modes[interface_mode]);
	UTILITY::tcscat(buffer, buffer_len, _T(")\n"));

	for(int num=-1; num < 2; num++) {
		uint16_t encoding = get_encoding(num);
		UTILITY::tcscat(buffer, buffer_len, _T("  Encoding"));
		if (num >= 0) {
			UTILITY::sntprintf(buffer, buffer_len, _T(" on Track0 and Side%d"), num);
		}
		UTILITY::sntprintf(buffer, buffer_len, _T(":0x%02x("), encoding);
		if (encoding == DISALE_ENCODING) {
			encoding = (ENCODING_MAX + 1);
		} else if (encoding > ENCODING_MAX) {
			encoding = ENCODING_MAX;
		}
		UTILITY::tcscat(buffer, buffer_len, c_encoding[encoding]);
		UTILITY::tcscat(buffer, buffer_len, _T(")\n"));
	}

	m_current_track.debug_media_info(buffer, buffer_len);
}

void DISKHFE_TRACK::debug_media_info(_TCHAR *buffer, size_t buffer_len)
{
	for(int side_pos = 0; side_pos < 2; side_pos++) {
		m_info[side_pos].debug_media_info(m_track_pos, side_pos, buffer, buffer_len);
	}
}

void DISKHFE_TRACK_INFO::debug_media_info(int track_pos, int side_pos, _TCHAR *buffer, size_t buffer_len)
{
	int sector_nums = 0;
#ifdef USE_CPTRLIST_SECTOR
	sector_nums = m_sectors.Count();
#else
	sector_nums = m_sectors_count;
#endif

	UTILITY::sntprintf(buffer, buffer_len, _T("  Current Track Pos:%2d Side:%d Number of Sector:%2d Raw Size:%6d Density:%c\n")
		, track_pos
		, side_pos
		, sector_nums
		, m_size
		, m_density ? _T('D') : _T('S')
	);
	if (!m_valid) {
		UTILITY::tcscat(buffer, buffer_len, _T("   Track is not parsed yet.\n"));
		return;
	}
	if (sector_nums <= 0) {
		UTILITY::tcscat(buffer, buffer_len, _T("   Sector is not found.\n"));
		return;
	}
	UTILITY::tcscat(buffer, buffer_len, _T("   Pos   C   H   R   N  IDPos DatPos Del\n"));
	for(int sector_pos = 0; sector_pos < sector_nums; sector_pos++) {
#ifdef USE_CPTRLIST_SECTOR
		m_sectors.Item(sector_pos)->debug_media_info(sector_pos, m_data, buffer, buffer_len);
#else
		m_sectors[sector_pos].debug_media_info(sector_pos, m_data, buffer, buffer_len);
#endif
	}
}

void DISKHFE_SECTOR::debug_media_info(int sector_pos, const uint8_t *p_data, _TCHAR *buffer, size_t buffer_len)
{
	UTILITY::sntprintf(buffer, buffer_len, _T("   %3d %3u %3u %3u %3u %6d %6d %c\n")
		, sector_pos
		, m_id[0], m_id[1], m_id[2], m_id[3]
		, p_sector_id ? (p_sector_id - p_data) : -1
		, p_sector_data ? (p_sector_data - p_data) : -1
		, m_deleted ? _T('*') : _T(' ')
	);
}
#endif /* USE_DEBUGGER */
