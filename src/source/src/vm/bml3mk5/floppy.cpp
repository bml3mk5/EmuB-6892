/** @file floppy.cpp

	HITACHI BASIC MASTER LEVEL3 Mark5 Emulator 'EmuB-6892'
	Skelton for retropc emulator

	@author Sasaji
	@date   2011.11.05 -

	@brief [ floppy drive ]
*/

#include <math.h>
#include "floppy.h"
//#include "../../emu.h"
#include "../mc6843.h"
#include "../mb8866.h"
#include "../disk_image.h"
#include "../../emu.h"
#include "../vm.h"
#include "../../logging.h"
#include "../../utility.h"
#include "../../config.h"
#include "../parsewav.h"

#if defined(_DEBUG_FLOPPY) || defined(_DEBUG_MB6843) || defined(_DEBUG_MB8866)
#define OUT_DEBUG logging->out_debugf
#define _DEBUG_FLOPPY_ALL
static uint8_t debug_regr[5];
static int     debug_regr_cnt[5];
#define OUT_DEBUG_REGR(emu, addr, data, msg) out_debug_regr(emu, addr, data, msg)
#else
#define OUT_DEBUG(...)
#define OUT_DEBUG_REGR(...)
#endif

#ifdef _DEBUG
//#define _DEBUG_TIME
#endif

#define DRIVE_MASK	(USE_FLOPPY_DISKS - 1)

// ----------------------------------------------------------------------------

DISK_DRIVE::DISK_DRIVE()
	: DISK()
{
}

DISK_DRIVE::DISK_DRIVE(DEVICE *fdc, int drv)
	: DISK(fdc, drv)
{
	i_side = 0;
	i_track = -1;
	i_dataidx = 0;
	i_secidx = -1;
#ifdef USE_SIG_FLOPPY_ACCESS
	i_access = false;
#endif
	i_ready = 0;
	i_motor_warmup = 0;
	i_head_loading = 0;
	i_delay_write = 0;

	i_shown_media_error = false;
}

DISK_DRIVE::~DISK_DRIVE()
{
}

void DISK_DRIVE::reset()
{
	i_track = -1;
	i_dataidx = 0;
	i_secidx = -1;
#ifdef USE_SIG_FLOPPY_ACCESS
	i_access = false;
#endif
}

void DISK_DRIVE::warm_reset()
{
	i_ready = 0;
	i_motor_warmup = 0;
	i_head_loading = 0;
	i_shown_media_error = false;
}

/// @brief Open a floppy disk image
///
/// @param[in] path : file path
/// @param[in] offset : start position of file
/// @param[in] flags : bit0: 1 = read only  bit3: 1= last volume  bit4: 1 = multi volumed file
/// @return true if success
bool DISK_DRIVE::open(const _TCHAR *path, int offset, uint32_t flags)
{
	bool rc = DISK::open(path, offset, flags);
	if (rc) {
		i_delay_write = 0;
		i_shown_media_error = false;
	}
	return rc;
}

/// @brief Close the floppy disk image
void DISK_DRIVE::close()
{
	DISK::close();	
	i_delay_write = 0;
}

// ----------------------------------------------------------------------------

/// @brief Ready on
///
/// @return true if turn on ready signal
bool DISK_DRIVE::ready_on()
{
	if (i_ready == 0) {
		i_ready = 1;
		if (is_inserted()) {
			i_motor_warmup = 1;
		}
		return true;
	}
	return false;
}

/// @brief Ready off
void DISK_DRIVE::ready_off()
{
	i_ready = 0;
	i_motor_warmup = 0;
}

/// @brief Update ready signal
void DISK_DRIVE::update_ready()
{
	i_ready <<= 1;
	OUT_DEBUG(_T("DISK %d event READY %d clk:%ld"), m_drive_num, i_ready, p_fdc->get_current_clock());
}

/// @brief Drive is ready?
///
/// @return true if ready
bool DISK_DRIVE::is_ready() const
{
	return (is_inserted() && i_ready >= 2);
}

/// @brief Load head
///
/// @param[in] data !=0:load head, ==0 unload head
/// @return true if head loading turn on
bool DISK_DRIVE::set_head_load(uint8_t data)
{
	bool trigger_on = false;
	if (is_inserted()) {
		trigger_on = (data && !i_head_loading);
	}
	if (data) {
		i_head_loading = 120; // frames -> 2sec.
	}
	return trigger_on;
}

/// @brief Now loading head ?
///
/// @return true if head load on
bool DISK_DRIVE::is_head_loading() const
{
	return (i_head_loading != 0);
}

/// @brief Now accessing the disk
///
/// @return true if accessing
bool DISK_DRIVE::now_disk_accessing() const
{
	return (i_head_loading > 0 && i_ready >= 2);
}

/// @brief Update warming up
void DISK_DRIVE::update_motor_warmup()
{
	if (i_motor_warmup) {
		i_motor_warmup <<= 1;
		if (i_motor_warmup > 4) i_motor_warmup = 0;
		OUT_DEBUG(_T("DISK %d event MOTOR WARMUP %d"), m_drive_num, i_motor_warmup);
	}
}

// ----------------------------------------------------------------------------

/// @brief Get specified track
///
/// @param[in] density : 1: double density
/// @param[in] data_start_pos : if >=0 set i_dataidx
/// @return : false : not found or invalid 
/// @note need set i_track and i_side before call this
bool DISK_DRIVE::search_track(int density, int data_start_pos)
{
	if (data_start_pos >= 0) {
		i_dataidx = data_start_pos;
	}
	if (!p_image) {
		return (FLG_ORIG_FDINSERT == 0);
	}
	return p_image->search_track(i_track, i_side, density);
}

/// @brief Verify a track number in each sector on a track
///
/// @param[in] track_num : track number
/// @param[in] density : 1: double density
/// @return true if match
bool DISK_DRIVE::verify_track_number(int track_num, int density)
{
	if (!p_image) {
		return false;
	}

	return p_image->verify_track_number(track_num, i_side, density);
}

/// @brief Make track
///
/// @param[in] density : density
/// @return true if match
bool DISK_DRIVE::make_track(int density)
{
	if (!p_image) {
		return false;
	}

	return p_image->make_track(i_side, density);
}

/// @brief Parse track
///
/// @param[in] density : density
/// @return true if match
bool DISK_DRIVE::parse_track(int density)
{
	if (!p_image) {
		return false;
	}

	return p_image->parse_track(i_side, density);
}

/// @brief Write data to current track
///
/// @param[in] data
void DISK_DRIVE::write_track_data(uint8_t data)
{
	DISK_TRACK *current_track = get_current_track();
	if (current_track && i_dataidx < current_track->get_raw_track_size()) {
		current_track->set_raw_track_data(i_dataidx, data);
		i_dataidx++;
		i_delay_write = DELAY_WRITE_FRAME;
	}
}

/// @brief Read data in current track
///
/// @param[in] default_data
/// @return data
uint8_t DISK_DRIVE::read_track_data(uint8_t default_data)
{
	uint8_t data = default_data;
	DISK_TRACK *current_track = get_current_track();
	if (current_track && i_dataidx < current_track->get_raw_track_size()) {
		data = current_track->get_raw_track_data(i_dataidx);
		i_dataidx++;
	}
	return data;
}

/// @brief Read data repeatedly in current track
///
/// @param[in] default_data
/// @return data
uint8_t DISK_DRIVE::read_track_data_repeat(uint8_t default_data)
{
	uint8_t data = default_data;
	DISK_TRACK *current_track = get_current_track();
	if(current_track) {
		if (i_dataidx >= current_track->get_raw_track_size()) {
			i_dataidx -= current_track->get_raw_track_size();
		}
		data = current_track->get_raw_track_data(i_dataidx);
		i_dataidx++;
	}
	return data;
}

/// @brief Step in/out track
///
/// @param[in] data : < 0x80: increment track position, > 0x80: decrement track position
/// @return true if unchanged track position
bool DISK_DRIVE::step_track_position(uint8_t data)
{
	if (i_track < 0) i_track = 0;

	int cur_track = i_track;

	if (data < 0x80) i_track++;
	else if (data > 0x80) i_track--;

	if (i_track < 0) i_track = 0;
	if (i_track > 255) i_track = 255;

	return (cur_track == i_track);
}

/// @brief Track is 0 ?
///
/// @return true if track is 0
bool DISK_DRIVE::is_track_zero() const
{
	return (i_track == 0);
}

/// @brief Set current position in current track
///
/// @param[in] remain_clock : clock until reach index hole 
void DISK_DRIVE::set_current_pos_in_track(int remain_clock)
{
	DISK_TRACK *current_track = get_current_track();
	if (current_track) {
		double ratio = (double)(m_drive_type.m_round_clock - remain_clock) / m_drive_type.m_round_clock;
		i_dataidx = (int)(ratio * (double)current_track->get_raw_track_size());
	} else {
		i_dataidx = 0;
	}
}

// ----------------------------------------------------------------------------

/// @brief Set the side position
///
/// @param[in] side_pos : side position
void DISK_DRIVE::set_side_position(int side_pos)
{
	i_side = (side_pos & 1) % get_number_of_side();
	DISK::set_side_position(side_pos);
}

/// @brief Get the side position
///
/// @return side position
int DISK_DRIVE::get_side_position() const
{
	int sides = get_number_of_side(); 
	return (sides > 1 ? i_side % sides : -1);
}

// ----------------------------------------------------------------------------

/// @brief Parse sector
///
/// @param[in] density : set 1 if double density
/// @note need set i_side before call this
void DISK_DRIVE::parse_sector(int density)
{
	if (!p_image) {
		return;
	}
	p_image->parse_sector(i_side, density);
}

/// @brief Finds a sector from specified sector number.
///
/// @param[in] density : 1 if double density
/// @param[in] sector_num : sector number or -1 if specify sector position
/// @param[in,out] sector_pos : position of sector in the track. if specify -1 then sector position 0
/// @return DISK::en_result_search_sector
int DISK_DRIVE::search_sector(int density, int sector_num, int &sector_pos)
{
	i_secidx = -1;

	// disk inserted?
	if (!is_inserted()) {
		return RES_DISK_NOT_INSERTED;
	}
	int sts = p_image->search_sector(i_side, density, sector_num, sector_pos);
	if (!sts) {
		i_secidx = (int16_t)sector_pos;
		i_dataidx = 0;
	}
	return sts;
}

/// @brief Finds a sector from specified sector number and calculate the time (clocks) required to reach that sector.
///
/// @param[in] density : 1 if double density
/// @param[in] sector_num : sector number or -1 if specify sector position
/// @param[in,out] sector_pos : position of sector in the track. if specify -1 then search nearest sector
/// @param[in] remain_clock : clock until reach index hole
/// @param[in] timeout_round : number of round to occur timeout 
/// @param[out] arrive_clock : clock until reach the sector
/// @return DISK::en_result_search_sector
int DISK_DRIVE::search_sector_and_get_clock(int density, int sector_num, int &sector_pos, int remain_clock, int timeout_round, int &arrive_clock)
{
	i_secidx = -1;

	// disk inserted?
	if (!is_inserted()) {
		return RES_DISK_NOT_INSERTED;
	}
	int sts = p_image->search_sector_and_get_clock(i_side, density, sector_num, sector_pos, remain_clock, timeout_round, arrive_clock);
	if (!sts) {
		i_secidx = (int16_t)sector_pos;
		i_dataidx = 0;
	}
	return sts;
}

/// @brief Write data to current sector
///
/// @param[in] data
void DISK_DRIVE::write_sector_data(uint8_t data)
{
	DISK_SECTOR *current_sector = get_current_sector();
	if (current_sector && i_dataidx < current_sector->get_sector_size()) {
		current_sector->set_sector_data(i_dataidx, data);
		i_dataidx++;
		i_delay_write = DELAY_WRITE_FRAME;
	}
}

/// @brief Read data in current sector
///
/// @param[in] default_data
/// @return id
uint8_t DISK_DRIVE::read_sector_data(uint8_t default_data)
{
	uint8_t data = default_data;
	DISK_SECTOR *current_sector = get_current_sector();
	if (current_sector && i_dataidx < current_sector->get_sector_size()) {
		data = current_sector->get_sector_data(i_dataidx);
		i_dataidx++;
	}
	return data;
}

/// @brief Read id in current sector
///
/// @param[in] default_data
/// @return id
uint8_t DISK_DRIVE::read_sector_id(uint8_t default_data)
{
	uint8_t data = default_data;
	if(i_dataidx < 6) {
		DISK_SECTOR *current_sector = get_current_sector();
		if (current_sector) {
			data = current_sector->get_sector_id(i_dataidx);
			i_dataidx++;
		}
	}
	return data;
}

// ----------------------------------------------------------------------------

/// @return true if head unloaded
bool DISK_DRIVE::update_event()
{
	bool trigger_off = false;
	if (i_head_loading) {
		i_head_loading--;
		trigger_off =  (is_inserted() && i_head_loading == 0);
	}
	// write data
	if (i_delay_write > 0) {
		i_delay_write--;
		if (i_delay_write == 0) {
			flush();
		}
	}
	return trigger_off;
}

void DISK_DRIVE::restore_state(uint8_t density)
{
	if (i_track >= 0) {
		int16_t tmp_dataidx = i_dataidx;
		search_track(density, -1);
		if (i_secidx >= 0) {
			parse_sector(density);
			int sector_pos =  i_secidx;
			search_sector(density, -1, sector_pos);
			i_secidx = sector_pos;
		}
		i_dataidx = tmp_dataidx;
	}
}

// ----------------------------------------------------------------------------

void DISK_DRIVE::save_state(struct vm_state_st &vm_state)
{
	vm_state.i_track = Int16_LE(i_track);
	vm_state.i_side  = Int16_LE(i_side);
	vm_state.i_dataidx = Int16_LE(i_dataidx);
	vm_state.i_secidx = Int16_LE(i_secidx);
	vm_state.i_delay_write = Int16_LE(i_delay_write);
	vm_state.i_ready = i_ready;
	vm_state.i_motor_warmup = i_motor_warmup;
	vm_state.i_head_loading = i_head_loading;
#ifdef USE_SIG_FLOPPY_ACCESS
	if (i_access) vm_state.i_flags |= 1;
#endif
	if (i_shown_media_error) vm_state.i_flags |= 2;
}

void DISK_DRIVE::load_state(const struct vm_state_st &vm_state)
{
	i_track = Int16_LE(vm_state.i_track);
	i_side = Int16_LE(vm_state.i_side);
	i_dataidx = Int16_LE(vm_state.i_dataidx);
	i_secidx = Int16_LE(vm_state.i_secidx);
	i_delay_write = Int16_LE(vm_state.i_delay_write);
	i_ready = vm_state.i_ready;
	i_motor_warmup = vm_state.i_motor_warmup;
	i_head_loading = vm_state.i_head_loading;
#ifdef USE_SIG_FLOPPY_ACCESS
	i_access = (vm_state.i_flags & 1) != 0;
#endif
	i_shown_media_error = false; // (vm_state.i_flags & 2) != 0;
}

// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
void DISK_DRIVE::debug_param_header(_TCHAR *buffer, size_t buffer_len)
{
	UTILITY::tcscat(buffer, buffer_len, _T(" FDD Info.\n  Drv Sts Trk S SeIdx DaIdx Ready Motor HLoad Write\n"));
}

void DISK_DRIVE::debug_param_data(_TCHAR *buffer, size_t buffer_len)
{
	UTILITY::sntprintf(buffer, buffer_len, _T("  %3d %c%c  %3d %d %5d %5d %5u %5u %5u %5d\n")
		, m_drive_num
		, is_inserted() ? _T('I') : _T(' ')
		, is_write_protected() ? _T('P') : _T(' ')
		, i_track
		, i_side
		, i_secidx
		, i_dataidx
		, i_ready
		, i_motor_warmup
		, i_head_loading
		, i_delay_write
	);
}

void DISK_DRIVE::debug_media_info(_TCHAR *buffer, size_t buffer_len)
{
	UTILITY::sntprintf(buffer, buffer_len, _T(" FDD Drive:%d Media Info: "), m_drive_num);
	bool inserted = is_inserted();
	UTILITY::tcscat(buffer, buffer_len, inserted ? _T("Inserted.") : _T("Not Inserted."));
	if (!inserted) {
		UTILITY::tcscat(buffer, buffer_len, _T("\n"));
		return;
	}
	UTILITY::tcscat(buffer, buffer_len, is_write_protected() ? _T(" Write Protected.\n") : _T("\n"));

	p_image->debug_media_info(buffer, buffer_len);
}

#endif /* USE_DEBUGGER */

// ============================================================================

FLOPPY::FLOPPY(VM* parent_vm, EMU* parent_emu, const char* identifier)
 : DEVICE(parent_vm, parent_emu, identifier)
{
	set_class_name("FLOPPY");
	init_output_signals(&outputs_irq);
	init_output_signals(&outputs_drq);
	d_fdc3 = NULL;
	for(int i=0; i<MAX_FDC_NUMS; i++) {
		d_fdc5[i] = NULL;
	}
	for(int i = 0; i < USE_FLOPPY_DISKS; i++) {
		p_disk[i] = new DISK_DRIVE(this, i);
	}
}

void FLOPPY::cancel_my_event(int event_no)
{
	if(m_register_id[event_no] != -1) {
		cancel_event(this, m_register_id[event_no]);
		m_register_id[event_no] = -1;
	}
}

void FLOPPY::register_my_event(int event_no, int wait)
{
	cancel_my_event(event_no);
	register_event(this, event_no, wait, false, &m_register_id[event_no]);
}

void FLOPPY::register_index_hole_event(int event_no, int wait)
{
	cancel_my_event(event_no);
	register_event_by_clock(this, event_no, wait, false, &m_register_id[event_no], &m_index_hole_next_clock);
}

// ----------------------------------------------------------------------------

void FLOPPY::initialize()
{
	// setup/reset floppy drive
	m_ignore_crc = false;

	m_sidereg = 0;
	m_next_sector_pos = -1;

	m_index_hole = 0;
	m_index_hole_next_clock = 0;
	m_head_load = 0;

	for(int i = 0; i < FLOPPY_MAX_EVENT; i++) {
		m_register_id[i] = -1;
	}

	m_wav_fddtype = FLOPPY_WAV_FDD3;
	m_wav_loaded_at_first = false;

	m_noises[FLOPPY_WAV_FDD3][FLOPPY_WAV_SEEK].set_file_name(_T("fddseek3.wav"));
	m_noises[FLOPPY_WAV_FDD3][FLOPPY_WAV_MOTOR].set_file_name(_T("fddmotor3.wav"));
	m_noises[FLOPPY_WAV_FDD3][FLOPPY_WAV_MOTOR].set_loop(true);
	m_noises[FLOPPY_WAV_FDD3][FLOPPY_WAV_HEADON].set_file_name(_T("fddheadon3.wav"));
	m_noises[FLOPPY_WAV_FDD3][FLOPPY_WAV_HEADOFF].set_file_name(_T("fddheadoff3.wav"));
	m_noises[FLOPPY_WAV_FDD5][FLOPPY_WAV_SEEK].set_file_name(_T("fddseek5.wav"));
	m_noises[FLOPPY_WAV_FDD5][FLOPPY_WAV_MOTOR].set_file_name(_T("fddmotor5.wav"));
	m_noises[FLOPPY_WAV_FDD5][FLOPPY_WAV_MOTOR].set_loop(true);
	m_noises[FLOPPY_WAV_FDD5][FLOPPY_WAV_HEADON].set_file_name(_T("fddheadon5.wav"));
	m_noises[FLOPPY_WAV_FDD5][FLOPPY_WAV_HEADOFF].set_file_name(_T("fddheadoff5.wav"));
	m_noises[FLOPPY_WAV_FDD8][FLOPPY_WAV_SEEK].set_file_name(_T("fddseek8.wav"));
	m_noises[FLOPPY_WAV_FDD8][FLOPPY_WAV_MOTOR].set_file_name(_T("fddmotor8.wav"));
	m_noises[FLOPPY_WAV_FDD8][FLOPPY_WAV_MOTOR].set_loop(true);
	m_noises[FLOPPY_WAV_FDD8][FLOPPY_WAV_HEADON].set_file_name(_T("fddheadon8.wav"));
	m_noises[FLOPPY_WAV_FDD8][FLOPPY_WAV_HEADOFF].set_file_name(_T("fddheadoff8.wav"));

	register_frame_event(this);
}

/// power on reset
void FLOPPY::reset()
{
	m_wav_fddtype = pConfig->fdd_type - 1;
	if (m_wav_fddtype < 0) m_wav_fddtype = 0;

	load_wav();

	for(int i = 0; i < USE_FLOPPY_DISKS; i++) {
		p_disk[i]->reset();
	}

	set_drive_speed();

	warm_reset(true);
}

/// reset signal
void FLOPPY::warm_reset(bool por)
{
#ifdef _DEBUG_FLOPPY_ALL
	for(int i=0; i<5; i++) {
		debug_regr[i] = 0x55;
		debug_regr_cnt[i] = 0;
	}
#endif

	// NMI OFF
	d_board->write_signal(SIG_CPU_NMI, 0, SIG_NMI_FD_MASK);
	// HALT OFF
	d_board->write_signal(SIG_CPU_HALT, 0, SIG_HALT_FD_MASK);

	if (!por) {
		cancel_my_events();
	} else {
		// events were already canceled by EVENT::reset()
		for(int i = 0; i < FLOPPY_MAX_EVENT; i++) {
			m_register_id[i] = -1;
		}
	}

	for(int i = 0; i < USE_FLOPPY_DISKS; i++) {
		p_disk[i]->warm_reset();
	}

	for(int i = 0; i < MAX_FDC_NUMS; i++) {
		m_drv_num[i] = 0;
		m_drvsel[i] = 0;

		m_motor_on_expand[i] = 0;

		m_fdd5outreg[i] = 0x81;
	}
	m_fdd5outreg_delay = 0;

	m_index_hole = 0;

//	set_drive_speed();

	m_density = 0;
	m_next_sector_pos = -1;

	m_irqflg = false;
	m_irqflgprev = m_irqflg;
	m_drqflg = false;
	m_drqflgprev = m_drqflg;

	m_ignore_write = false;

	for(int ft = 0; ft < FLOPPY_WAV_FDDTYPES; ft++) {
		for(int ty = 0; ty < FLOPPY_WAV_SNDTYPES; ty++) {
			m_noises[ft][ty].stop();
		}
	}

	if (IOPORT_USE_FDD) {
		register_my_event(EVENT_INDEXHOLE_ON, m_delay_index_hole);	// index hole
		if (pConfig->fdd_type != FDD_TYPE_58FDD) {
			// motor off time
			register_my_event(EVENT_MOTOR_TIMEOUT, DELAY_MOTOR_TIMEOUT);	// delay motor off(timeout) (60s)
		}
		d_fdc5[0]->write_signal(MB8866::SIG_MB8866_CLOCKNUM, pConfig->fdd_type == FDD_TYPE_58FDD ? 1 : 0, 1);
	}
}

/// cancel all events
void FLOPPY::cancel_my_events()
{
	for(int i = 0; i < FLOPPY_MAX_EVENT; i++) {
		cancel_my_event(i);
	}
}

/// release memory
void FLOPPY::release()
{
	// release disk image handler
	for(int i = 0; i < USE_FLOPPY_DISKS; i++) {
		delete p_disk[i];
		p_disk[i] = NULL;
	}
}

// ----------------------------------------------------------------------------

/// load wav files for sounding noise like a disk drive
void FLOPPY::load_wav()
{
	// allocation
	const _TCHAR *app_path, *rom_path[2];

	rom_path[0] = pConfig->rom_path.Get();
	rom_path[1] = vm->application_path();

	for (int ft=0; ft<FLOPPY_WAV_FDDTYPES; ft++) {
		for (int ty=0; ty<FLOPPY_WAV_SNDTYPES; ty++) {
			m_noises[ft][ty].alloc(m_sample_rate / 5);	// 0.2sec
		}
	}

	for(int i=0; i<2; i++) {
		app_path = rom_path[i];

		for (int ft=0; ft<FLOPPY_WAV_FDDTYPES; ft++) {
			for (int ty=0; ty<FLOPPY_WAV_SNDTYPES; ty++) {
				if (m_noises[ft][ty].load_wav_file(app_path, m_sample_rate) > 0) {
					logging->out_logf_x(LOG_INFO, CMsg::VSTR_was_loaded, m_noises[ft][ty].get_file_name());
				}
			}
		}
	}

	// load wav file
	for (int ft=0; ft<FLOPPY_WAV_FDDTYPES; ft++) {
		for (int ty=0; ty<FLOPPY_WAV_SNDTYPES; ty++) {
			if (!m_noises[ft][ty].is_enable()) {
				m_noises[ft][ty].clear();
				if (!m_wav_loaded_at_first) {
					OUT_DEBUG(_T("%s couldn't be loaded."), m_noises[ft][ty].get_file_name());
				}
			}
		}
	}

	m_wav_loaded_at_first = true;
}

/// write to io port
///
/// @param[in] addr : address
/// @param[in] data : data (1byte)
void FLOPPY::write_io8(uint32_t addr, uint32_t data)
{
	if (IOPORT_USE_5FDD) {
		// 5inch or 8inch
		switch(addr & 0x1f) {
			case 0x0:
			case 0x1:
			case 0x2:
				// FDC access ok
				if (pConfig->fdd_type != FDD_TYPE_58FDD || (pConfig->fdd_type == FDD_TYPE_58FDD && (m_drvsel[0] & 0x80) == 0)) {
					OUT_DEBUG(_T("fddw a:%05x d:%02x (%02x)"),addr,data,(~data) & 0xff);
					d_fdc5[0]->write_io8(addr & 0xf, data);
				} else {
					OUT_DEBUG(_T("fddw a:%05x d:%02x (%02x) BLOCKED"),addr,data,(~data) & 0xff);
				}
				break;
			case 0x3:
				// FDC access ok or DRQ on
				if (pConfig->fdd_type != FDD_TYPE_58FDD || (pConfig->fdd_type == FDD_TYPE_58FDD && ((m_drvsel[0] & m_fdd5outreg_delay & 0x80) == 0))) {
					OUT_DEBUG(_T("fddw a:%05x d:%02x (%02x)"),addr,data,(~data) & 0xff);
					d_fdc5[0]->write_io8(addr & 0xf, data);
				} else {
					OUT_DEBUG(_T("fddw a:%05x d:%02x (%02x) BLOCKED"),addr,data,(~data) & 0xff);
				}
				break;
			case 0x4:
				set_drive_select(FDC_TYPE_5INCH, data);
				if ((m_drvsel[0] & 0x80) == 0) m_fdd5outreg_delay &= 0x7f;
				OUT_DEBUG(_T("fddw a:%05x d:%02x clk:%ld"),addr,data,get_current_clock());
				break;
			case 0x8:
				// Send HALT signal if DRQ signal on FDC is false. 
				if (pConfig->fdd_type == FDD_TYPE_58FDD && !m_drqflg) {
					OUT_DEBUG(_T("fddw a:%05x d:%02x drq:%02x"),addr,data,m_drqflg);
					d_board->write_signal(SIG_CPU_HALT, SIG_HALT_FD_MASK, SIG_HALT_FD_MASK);
				}
				break;
			case 0xc:
				break;
		}
	} else if (IOPORT_USE_3FDD) {
		// 3inch
		set_drive_select(FDC_TYPE_3INCH, data);
	}
}

#ifdef _DEBUG_FLOPPY_ALL
void out_debug_regr(EMU *emu, uint32_t addr, uint32_t data, const TCHAR *msg)
{
	int n = addr & 7;
	if (debug_regr[n] != data) {
		if (debug_regr_cnt[n] > 0) {
			OUT_DEBUG(_T("fddr a:%05x d:%02x (%02x) repeat %d times"), addr, debug_regr[n], (~debug_regr[n]) & 0xff, debug_regr_cnt[n]);
		}
		OUT_DEBUG(_T("fddr a:%05x d:%02x (%02x)%s clk:%ld"), addr, data, (~data) & 0xff, msg != NULL ? msg : _T(""), emu->get_current_clock());
		debug_regr_cnt[n] = 0;
	} else {
		debug_regr_cnt[n]++;
	}
	debug_regr[n] = data;
}
#endif

/// read from io port
///
/// @param[in] addr : address
/// @return data (1byte)
uint32_t FLOPPY::read_io8(uint32_t addr)
{
	uint32_t data = 0xff;

	if (IOPORT_USE_5FDD) {
		// 5inch or 8inch
		switch(addr & 0x1f) {
			case 0x0:
			case 0x1:
				// FDC access ok
				if (pConfig->fdd_type != FDD_TYPE_58FDD || (pConfig->fdd_type == FDD_TYPE_58FDD && (m_drvsel[0] & 0x80) == 0)) {
					data = d_fdc5[0]->read_io8(addr & 0xf);
					OUT_DEBUG_REGR(emu, addr, data, NULL);
				} else {
					OUT_DEBUG_REGR(emu, addr, data, _T(" BLOCKED"));
				}
				break;
			case 0x2:
				// FDC access ok
				if (pConfig->fdd_type != FDD_TYPE_58FDD || (pConfig->fdd_type == FDD_TYPE_58FDD && (m_drvsel[0] & 0x80) == 0)) {
					data = d_fdc5[0]->read_io8(addr & 0xf);
					OUT_DEBUG_REGR(emu, addr, data, NULL);
				} else {
					// When FDC access mask is ON, send DRQ signal
					data = (m_fdd5outreg[0] | 0x7f);
					m_fdd5outreg_delay = data;
					OUT_DEBUG_REGR(emu, addr, data, _T(" Wait DRQ"));
				}
				break;
			case 0x3:
				// FDC access ok or DRQ on
				if (pConfig->fdd_type != FDD_TYPE_58FDD || (pConfig->fdd_type == FDD_TYPE_58FDD && ((m_drvsel[0] & m_fdd5outreg_delay & 0x80) == 0))) {
					data = d_fdc5[0]->read_io8(addr & 0xf);
					OUT_DEBUG_REGR(emu, addr, data, NULL);
				} else {
					OUT_DEBUG_REGR(emu, addr, data, _T(" BLOCKED"));
				}
				break;
			case 0x4:
				if (pConfig->fdd_type == FDD_TYPE_58FDD) data = 0xff;
				else data = (m_fdd5outreg[0] | 0x7e);
				OUT_DEBUG_REGR(emu, addr, data, NULL);
				break;
			case 0xc:
				// Type
				if (pConfig->fdd_type == FDD_TYPE_58FDD) {
					// always zero
					data = 0x00;
				}
				break;
		}
	} else if (IOPORT_USE_3FDD) {
		// 3inch
		// also update the unit_sel in a reading sequence
		set_drive_select(FDC_TYPE_3INCH, data);
	}

	return data;
}

/// write a signal
///
/// @param[in] id   : SIG_XXXXXXXX
/// @param[in] data : data (1byte)
/// @param[in] mask : mask (1byte)
void FLOPPY::write_signal(int id, uint32_t data, uint32_t mask)
{
	int fdcnum = (id >> 16);
	id &= 0xffff;

	int ndrv_num = m_drv_num[fdcnum];

	switch (id) {
#ifdef USE_SIG_FLOPPY_ACCESS
		case SIG_FLOPPY_ACCESS:
			p_disk[ndrv_num]->i_access = ((data & 1) != 0);
			break;
#endif
		case SIG_FLOPPY_WRITE:
			if (!m_ignore_write) {
				p_disk[ndrv_num]->write_sector_data(data & mask);
			}
			break;
		case SIG_FLOPPY_WRITE_TRACK:
			if (!m_ignore_write) {
				p_disk[ndrv_num]->write_track_data(data & mask);
			}
			break;
		case SIG_FLOPPY_WRITEDELETE:
			if (!m_ignore_write) {
				p_disk[ndrv_num]->set_deleted_mark((data & 1) != 0);
			}
			break;
		case SIG_FLOPPY_STEP:
			if (!p_disk[ndrv_num]->step_track_position(data)) {
				// fddseek sound turn on
				m_noises[m_wav_fddtype][FLOPPY_WAV_SEEK].play();
			}
			break;
		case SIG_FLOPPY_TRACK_SIZE:
			{
				DISK_TRACK *current_track = p_disk[ndrv_num]->get_current_track();
				if (current_track) {
					if (pConfig->fdd_type == FDD_TYPE_58FDD) {
						// 26sectors max
						current_track->set_raw_track_size(TRACK_SIZE_8INCH_2D);
					} else {
						// 16sectors max
						current_track->set_raw_track_size(TRACK_SIZE_5INCH_2D);
					}
				}
				p_disk[ndrv_num]->i_dataidx = 0;
			}
			break;
#ifdef SET_CURRENT_TRACK_IMMEDIATELY
		case SIG_FLOPPY_CURRENTTRACK:
			p_disk[ndrv_num]->i_track = (data & mask);
			break;
#endif
		case SIG_FLOPPY_HEADLOAD:
			m_head_load = (data & mask);
			if (p_disk[ndrv_num]->set_head_load(m_head_load)) {
				// fddheadon sound turn on
				m_noises[m_wav_fddtype][FLOPPY_WAV_HEADON].play();
			}
			OUT_DEBUG(_T("fdd %d sig HEADLOAD %x UL:%d"),ndrv_num,m_head_load,p_disk[ndrv_num]->i_head_loading);
			// When drive is ready, update timeout
			if (p_disk[ndrv_num]->is_ready()) {
				register_my_event(EVENT_MOTOR_TIMEOUT, DELAY_MOTOR_TIMEOUT);	// delay motor off(timeout) (60s)
			}
			break;
		case SIG_CPU_RESET:
			now_reset = (data & mask) ? true : false;
			warm_reset(false);
			if (now_reset) {
				set_drive_select(IOPORT_USE_3FDD ? FDC_TYPE_3INCH : FDC_TYPE_5INCH, 0);
			}
			break;
	}

	if (IOPORT_USE_5FDD) {
		// for 5inch mini floppy
		fdcnum = (mask >> 16);
		mask &= 0xffff;
		uint8_t nfdd5outreg = m_fdd5outreg[fdcnum];

		switch (id) {
			case SIG_FLOPPY_IRQ:
				m_irqflg = (data & mask) ? true : false;
				if (m_irqflg) {
					nfdd5outreg &= 0xfe;
				} else {
					nfdd5outreg |= 0x01;
				}
				break;
			case SIG_FLOPPY_DRQ:
				m_drqflg = (data & mask) ? true : false;
				if (m_drqflg) {
					nfdd5outreg &= 0x7f;
				} else {
					nfdd5outreg |= 0x80;
				}
				break;
		}

//		if (nfdd5outreg != fdd5outreg) {
//			OUT_DEBUG("fdd sig id:%04x d:%02x -> %02x"
//				,id,fdd5outreg[fdcnum],nfdd5outreg);
//		}

		// interrupt
		switch (id) {
		case SIG_FLOPPY_IRQ:
			if (m_irqflg != m_irqflgprev) {
				if (pConfig->fdd_type == FDD_TYPE_58FDD || (m_drvsel[fdcnum] & 0x40) == 0) {	// nmi non mask
					register_my_event(EVENT_IRQ, 20);	// delay
				}
				m_irqflgprev = m_irqflg;
			}
			break;
		case SIG_FLOPPY_DRQ:
			if (m_drqflg != m_drqflgprev) {
				if (pConfig->fdd_type == FDD_TYPE_58FDD) { // halt release
//					register_my_event(EVENT_DRQ, 20);	// delay
					set_drq(m_drqflg);					// immediate
				}
				m_drqflgprev = m_drqflg;
			}
			break;
		}

		m_fdd5outreg[fdcnum] = nfdd5outreg;
	}
}

/// read a signal
///
/// @param[in] id   : SIG_XXXXXXXX
/// @return data (1byte)
uint32_t FLOPPY::read_signal(int id)
{
	uint32_t data = 0;

	int fdcnum = (id >> 16);
	id &= 0xffff;

	int ndrv_num = m_drv_num[fdcnum];

	switch (id) {
		case SIG_FLOPPY_READ_ID:
			data = p_disk[ndrv_num]->read_sector_id(data);
			break;
		case SIG_FLOPPY_READ:
			data = p_disk[ndrv_num]->read_sector_data(data);
			break;
		case SIG_FLOPPY_READ_TRACK:
			data = p_disk[ndrv_num]->read_track_data(data);
			break;
		case SIG_FLOPPY_READ_TRACK_LOOP:
			data = p_disk[ndrv_num]->read_track_data_repeat(data);
			break;
		case SIG_FLOPPY_WRITEPROTECT:
			data = (p_disk[ndrv_num]->is_write_protected() || m_ignore_write) ? 1 : 0;
			break;
		case SIG_FLOPPY_HEADLOAD:
			data = (p_disk[ndrv_num]->is_head_loading()) ? 1 : 0;
			break;
		case SIG_FLOPPY_READY:
			data = (p_disk[ndrv_num]->is_ready()) ? 1 : 0;
			break;
		case SIG_FLOPPY_TRACK0:
			data = (p_disk[ndrv_num]->is_track_zero()) ? 1 : 0;
			break;
		case SIG_FLOPPY_INDEX:
			data = m_index_hole;
			break;
		case SIG_FLOPPY_DELETED:
			data = p_disk[ndrv_num]->has_deleted_mark() ? 1 : 0;
			break;
#ifdef SET_CURRENT_TRACK_IMMEDIATELY
		case SIG_FLOPPY_CURRENTTRACK:
			data = p_disk[ndrv_num]->i_track;
			break;
#endif
		case SIG_FLOPPY_SECTOR_NUM:
			data = p_disk[ndrv_num]->get_number_of_sector();
			break;
		case SIG_FLOPPY_SECTOR_SIZE:
			data = p_disk[ndrv_num]->get_current_sector_size();
			break;
		case SIG_FLOPPY_TRACK_SIZE:
			data = p_disk[ndrv_num]->get_current_track_size();
			break;
		case SIG_FLOPPY_DENSITY:
			data = m_density;
			break;
	}

	return data;
}

// ----------------------------------------------------------------------------
// event handler
// ----------------------------------------------------------------------------

void FLOPPY::event_frame()
{
	for(int i=0; i<USE_FLOPPY_DISKS; i++) {
		if (p_disk[i]->update_event()) {
			// fddheadoff sound turn on
			m_noises[m_wav_fddtype][FLOPPY_WAV_HEADOFF].play();
		}
	}
}

void FLOPPY::event_callback(int event_id, int err)
{
	int event_no = event_id;

	m_register_id[event_no] = -1;

	switch(event_no) {
		case EVENT_MOTOR_TIMEOUT:
			// motor off
			motor(-1, false);
			break;
		case EVENT_INDEXHOLE_ON:
			// index hole on
			m_index_hole = 1;
			register_index_hole_event(EVENT_INDEXHOLE_OFF, m_limit_index_hole);	// index hole off
			m_index_hole_next_clock += (m_delay_index_hole - m_limit_index_hole); 
			break;
		case EVENT_INDEXHOLE_OFF:
			// index hole off
			m_index_hole = 0;
			for(int i=0; i<USE_FLOPPY_DISKS; i++) {
				p_disk[i]->update_motor_warmup();
			}
			register_index_hole_event(EVENT_INDEXHOLE_ON, m_delay_index_hole - m_limit_index_hole);	// index hole on
			break;
		case EVENT_IRQ:
			cancel_my_event(EVENT_IRQ);
			set_irq(m_irqflg);
			break;
//		case EVENT_DRQ:
//			cancel_my_event(EVENT_DRQ);
//			set_drq(drqflg);
//			break;
		case EVENT_READY_ON_0:
		case EVENT_READY_ON_1:
		case EVENT_READY_ON_2:
		case EVENT_READY_ON_3:
		case EVENT_READY_ON_4:
		case EVENT_READY_ON_5:
		case EVENT_READY_ON_6:
		case EVENT_READY_ON_7:
			{
				int i = event_no - EVENT_READY_ON_0;
				p_disk[i]->update_ready();
			}
			break;
		case EVENT_MOTOR_OFF:
			// motor off
			motor(-1, false);
			break;
	}

}

// ----------------------------------------------------------------------------
/// Select the drive (UNIT sel)
///
/// @param[in] fdc_type: 0:3inch 1:5inch 2:unused
/// @param[in] data: data bus
void FLOPPY::set_drive_select(int fdc_type, uint8_t data)
{
	uint8_t ndrv_num = 0;
	int fdcnum = 0;

	// drive number
	switch(fdc_type) {
	case FDC_TYPE_5INCHEX:
		// 5inch ex always use drive 2 or 3
		ndrv_num = (data & DRIVE_MASK) | 2;
		break;

	case FDC_TYPE_5INCH:
		// 5inch
		ndrv_num = (data & DRIVE_MASK);
		break;

	default:
		// 3inch
		switch(data & 0x0f) {
			case 2:
				ndrv_num = 1;
				break;
			case 4:
				ndrv_num = 2;
				break;
			case 8:
				ndrv_num = 3;
				break;
			default:
				ndrv_num = 0;
				break;
		}
		ndrv_num &= DRIVE_MASK;
		if (m_drv_num[fdcnum] != ndrv_num) {
			// update status
			d_fdc3->write_signal(MC6843::SIG_MC6843_UPDATESTATUS, 0, 0);
		}
		break;
	}
	m_drv_num[fdcnum] = ndrv_num;

//	// set drive speed
//	set_drive_speed();

	// side and motor
	switch(fdc_type) {
	case FDC_TYPE_5INCH:
	case FDC_TYPE_5INCHEX:
		// 5inch
		// side select
		set_disk_side(m_drv_num[fdcnum], (data & 0x10) ? 1 : 0);
		// density
		m_density = (data & 0x20) ? 1 : 0;
		d_fdc5[fdcnum]->write_signal(SIG_FLOPPY_DENSITY, m_density, 1);

		// motor
		if ((data & 0x08) == 0 && m_motor_on_expand[fdcnum] != 0 && pConfig->fdd_type != FDD_TYPE_58FDD) {
			// motor on -> off
			OUT_DEBUG(_T("fdd %d MOTOR OFF REQUEST data:%02x drvsel:%02x"), m_drv_num[fdcnum], data, m_drvsel[fdcnum]);
			register_my_event(EVENT_MOTOR_OFF, DELAY_MOTOR_OFF_5);	// delay motor off
			m_motor_on_expand[fdcnum] = 1;
			m_drvsel[fdcnum] = (data & 0xff);
		} else if ((data & 0x08) != 0 || pConfig->fdd_type == FDD_TYPE_58FDD) {
			// motor on
			motor(m_drv_num[fdcnum], true);
			m_drvsel[fdcnum] = (data & 0xff);
			m_motor_on_expand[fdcnum] = 1;
		} else {
			// motor off
			OUT_DEBUG(_T("fdd %d MOTOR OFF FORCE data:%02x drvsel:%02x"), m_drv_num[fdcnum], data, m_drvsel[fdcnum]);
			motor(-1, false);
		}
		break;

	default:
		// 3inch
		// motor
		if ((data & 0x80) == 0 && m_motor_on_expand[fdcnum] != 0) {
			// motor on -> off
			OUT_DEBUG(_T("fdd %d MOTOR OFF REQUEST data:%02x drvsel:%02x"), m_drv_num[fdcnum], data, m_drvsel[fdcnum]);
			register_my_event(EVENT_MOTOR_OFF, DELAY_MOTOR_OFF_3);	// delay motor off
			m_motor_on_expand[fdcnum] = 1;
			m_drvsel[fdcnum] = (data & 0xff);
		} else if ((data & 0x80) != 0) {
			// motor on
			motor(m_drv_num[fdcnum], true);
			m_drvsel[fdcnum] = (data & 0xff);
			m_motor_on_expand[fdcnum] = 1;
		} else {
			// motor off
			OUT_DEBUG(_T("fdd %d MOTOR OFF FORCE data:%02x drvsel:%02x"), m_drv_num[fdcnum], data, m_drvsel[fdcnum]);
			motor(-1, false);
		}
		break;
	}
}

/// Motor on/off
///
/// @param[in] drv : drive number
/// @param[in] val : on / off
void FLOPPY::motor(int drv, bool val)
{
	if (val) {
		// motor on
		if (pConfig->fdd_type != FDD_TYPE_58FDD) {
			register_my_event(EVENT_MOTOR_TIMEOUT, DELAY_MOTOR_TIMEOUT);	// delay motor off(timeout) (60s)
		}

		// motor sound on
		if (!m_noises[m_wav_fddtype][FLOPPY_WAV_MOTOR].now_playing() && p_disk[drv]->is_inserted()) {
			m_noises[m_wav_fddtype][FLOPPY_WAV_MOTOR].play();
		}

		OUT_DEBUG(_T("fdd %d MOTOR ON REQUEST READY:%d WARMUP:%d"), drv, p_disk[drv]->i_ready, p_disk[drv]->i_motor_warmup);

		// ready on
		if (p_disk[drv]->ready_on()) {
			register_my_event(EVENT_READY_ON_0 + drv, m_delay_ready_on);	// delay ready on (1s)
		}

		cancel_my_event(EVENT_MOTOR_OFF);

		OUT_DEBUG(_T("fdd %d MOTOR ON  READY:%d WARMUP:%d"), drv, p_disk[drv]->i_ready, p_disk[drv]->i_motor_warmup);
	} else {
		// motor off
		m_ignore_write = false;

		// motor sound off
		uint8_t wav_playing = 0;
		for(int i=0; i<USE_FLOPPY_DISKS; i++) {
			if (drv == -1 || drv == i) {
				p_disk[i]->ready_off();
			}
			wav_playing |= p_disk[i]->i_ready;
		}
		if (wav_playing < 2) {
			// reset motor flag when all drives stopped
			for(int n = 0; n < MAX_FDC_NUMS; n++) {
				m_motor_on_expand[n] = 0;
			}

			if (IOPORT_USE_3FDD) {
				// 3inch
				for(int n = 0; n < MAX_FDC_NUMS; n++) {
					m_drvsel[n] &= 0x7f;
				}
			} else if (IOPORT_USE_5FDD) {
				// 5inch
				for(int n = 0; n < MAX_FDC_NUMS; n++) {
					m_drvsel[n] &= 0xf7;
				}
			}
			m_noises[m_wav_fddtype][FLOPPY_WAV_MOTOR].stop();
		}

		OUT_DEBUG(_T("fdd %d MOTOR OFF  SOUND:%d"), drv, wav_playing);
	}
}

void FLOPPY::set_drive_speed()
{
	if (pConfig->fdd_type == FDD_TYPE_58FDD) {
		// 2HD
		m_delay_index_hole = DELAY_INDEX_HOLE_H;	// usec.
		m_delay_ready_on = DELAY_READY_ON_H;
	} else {
		// 2D
		m_delay_index_hole = DELAY_INDEX_HOLE;	// usec.
		m_delay_ready_on = DELAY_READY_ON;
	}
	// conv usec. to cpu clock
	m_delay_index_hole = (int)((double)m_delay_index_hole * CPU_CLOCKS / 1000000.0);
	m_limit_index_hole = (int)(300.0 * CPU_CLOCKS / 1000000.0);

	for(int i = 0; i < USE_FLOPPY_DISKS; i++) {
#if defined(_MBS1)
		// set drive type
		if (pConfig->fdd_type == FDD_TYPE_58FDD) {
			// 5inch 2HD type
			if (i < 2) {
				p_disk[i]->set_drive_type(DRIVE_TYPE::DRIVE_TYPE_2HD_360, m_delay_index_hole);
			} else {
				// TODO
				p_disk[i]->set_drive_type(DRIVE_TYPE::DRIVE_TYPE_2D, m_delay_index_hole);
			}
		} else {
			p_disk[i]->set_drive_type(DRIVE_TYPE::DRIVE_TYPE_2D, m_delay_index_hole);
		}
#else
		// set drive type
		if (pConfig->fdd_type == FDD_TYPE_58FDD) {
			// 8inch type
			p_disk[i]->set_drive_type(DRIVE_TYPE::DRIVE_TYPE_2HD_360, m_delay_index_hole);
		} else {
			p_disk[i]->set_drive_type(DRIVE_TYPE::DRIVE_TYPE_2D, m_delay_index_hole);
		}
#endif
	}
}

// ----------------------------------------------------------------------------
// media handler
// ----------------------------------------------------------------------------
/// get drive number
///
/// @param[in] channel: FDC number(upper 16bit) / drive number (lower 16bit) 
/// @return drive number
int FLOPPY::get_drive_number(int channel) const
{
	int fdcnum = (channel >> 16);
	return m_drv_num[fdcnum];
}

/// search track
///
/// @param[in] channel: FDC number(upper 16bit) / drive number (lower 16bit) 
/// @return true if found
bool FLOPPY::search_track(int channel)
{
	int drvnum = get_drive_number(channel);
	if (drvnum >= USE_FLOPPY_DISKS) return false;
	return p_disk[drvnum]->search_track(m_density, 0);
}

/// verify track number
///
/// @param[in] channel: FDC number(upper 16bit) / drive number (lower 16bit) 
/// @param[in] track_num: track number
/// @return true if found
bool FLOPPY::verify_track_number(int channel, int track_num)
{
	// verify track number
	int drvnum = get_drive_number(channel);
	if (drvnum >= USE_FLOPPY_DISKS) return false;
	return p_disk[drvnum]->verify_track_number(track_num, m_density);
}

/// current track number
///
/// @param[in] channel: FDC number(upper 16bit) / drive number (lower 16bit) 
/// @return track number
int FLOPPY::get_current_track_number(int channel)
{
	int drvnum = get_drive_number(channel);
	if (drvnum >= USE_FLOPPY_DISKS) return 0;
	return p_disk[drvnum]->get_current_track_number();
}

/// make a raw track
///
/// @param[in] channel: FDC number(upper 16bit) 
/// @return true if success
bool FLOPPY::make_track(int channel)
{
	int drvnum = get_drive_number(channel);
	if (drvnum >= USE_FLOPPY_DISKS) return false;
	bool rc = p_disk[drvnum]->make_track(m_density);
	if (rc) {
		switch(pConfig->fdd_type) {
		case FDD_TYPE_3FDD:
			set_current_pos_in_track(drvnum);
			break;
		default:
			break;
		}
	}
	return rc;
}

/// Parse a raw track
///
/// @param[in] channel: FDC number(upper 16bit) 
/// @return true if success
bool FLOPPY::parse_track(int channel)
{
	int drvnum = get_drive_number(channel);
	if (drvnum >= USE_FLOPPY_DISKS) return false;
	bool rc = p_disk[drvnum]->parse_track(m_density);
	return rc;
}

/// Set head position in a track
///
/// @param[in] drvnum: drive number
void FLOPPY::set_current_pos_in_track(int drvnum)
{
	p_disk[drvnum]->set_current_pos_in_track(get_index_hole_remain_clock(0));
}

// ----------------------------------------------------------------------------

/// parse sector
///
/// @param[in] channel: FDC number(upper 16bit) / drive number (lower 16bit) 
void FLOPPY::parse_sector(int channel)
{
	int drvnum = get_drive_number(channel);
	if (drvnum >= USE_FLOPPY_DISKS) return;
	p_disk[drvnum]->parse_sector(m_density);
}

/// after searching sector
///
/// @param[in] drvnum : drive number
/// @param[in] status
void FLOPPY::disp_message_searched_sector(int drvnum, int status)
{
	if (p_disk[drvnum]->i_shown_media_error) return;
	switch(status) {
	case DISK::RES_INVALID_MEDIA_TYPE:
		// different media type
		logging->out_logf_x(LOG_ERROR, CMsg::The_media_type_in_drive_VDIGIT_is_different_from_specified_one, drvnum); 
		break;
	case DISK::RES_UNMATCH_DENSITY:
		// different density
		logging->out_logf_x(LOG_WARN, CMsg::The_density_in_track_VDIGIT_side_VDIGIT_is_different_from_specified_one, p_disk[drvnum]->i_track, p_disk[drvnum]->i_side); 
		break;
	default:
		break;
	}
	p_disk[drvnum]->i_shown_media_error = true;
}

/// search sector
///
/// @param[in] channel: FDC number(upper 16bit) 
/// @param[in] track_num : track number
/// @param[in] sector_num : sector number
/// @param[in] compare_side : whether compare side number or not
/// @param[in] side_num : side number if compare number
/// @return bit0:Record Not Found / bit1:CRC Error / bit2:Deleted Mark Detected
int FLOPPY::search_sector(int channel, int track_num, int sector_num, bool compare_side, int side_num)
{
	int drvnum = get_drive_number(channel);
	if (drvnum >= USE_FLOPPY_DISKS) return STS_RECORD_NOT_FOUND;
	int status = 0;
	int sector_pos = 0;

	do {
		int sta = p_disk[drvnum]->search_sector(m_density, sector_num, sector_pos);
		if (sta) {
			disp_message_searched_sector(drvnum, sta);
			status = STS_RECORD_NOT_FOUND; // SECTOR NOT FOUND
			break;
		}
		DISK_SECTOR *current_sector = p_disk[drvnum]->get_current_sector();
		if (current_sector) {
			// check track number in id field
			if (current_sector->get_sector_id(0) != (uint8_t)track_num) {
				status |= STS_UNMATCH_TRACK_NUMBER;
				break;
			}
			// check side number in id field
			if(compare_side && current_sector->get_sector_id(1) != (uint8_t)side_num) {
				status |= STS_UNMATCH_SIDE_NUMBER;
				break;
			}
			if (!m_ignore_crc && current_sector->has_crc_error(m_density)) {
				status |= STS_CRC_ERROR;	// CRC ERROR
			}
			if (current_sector->has_deleted_mark()) {
				status |= STS_DELETED_MARK_DETECTED;	// DELETED MARK DETECTED
			}
		}
	} while(0);

	return status;
}

/// search nearest sector
///
/// @param[in] channel: FDC number(upper 16bit) 
/// @return bit0:Record Not Found / bit1:CRC Error / bit2:Deleted Mark Detected
int FLOPPY::search_next_sector(int channel)
{
	int drvnum = get_drive_number(channel);
	if (drvnum >= USE_FLOPPY_DISKS) return STS_RECORD_NOT_FOUND;
	int status = 0;
	int sector_pos = -1;

	do {
		if (FLG_DELAY_FDSEARCH) {
			if (m_next_sector_pos < 0) {
				m_next_sector_pos = 0;
			}
			sector_pos = m_next_sector_pos;
		}
		int sta = p_disk[drvnum]->search_sector(m_density, -1, sector_pos);
		if (sta) {
			disp_message_searched_sector(drvnum, sta);
			status = STS_RECORD_NOT_FOUND; // SECTOR NOT FOUND
			break;
		}
		if (FLG_DELAY_FDSEARCH) {
			m_next_sector_pos++;
			m_next_sector_pos %= p_disk[drvnum]->get_number_of_sector(1);
		}
		DISK_SECTOR *current_sector = p_disk[drvnum]->get_current_sector();
		if (current_sector) {
			if (!m_ignore_crc && current_sector->has_crc_error(m_density)) {
				status |= STS_CRC_ERROR;	// CRC ERROR
			}
			if (current_sector->has_deleted_mark()) {
				status |= STS_DELETED_MARK_DETECTED;	// DELETED MARK DETECTED
			}
		}
	} while(0);

	return status;
}

/// search sector and get clock until reach the sector
///
/// @param[in] channel : FDC number(upper 16bit) 
/// @param[in] track_num : track number
/// @param[in] sector_num : sector number
/// @param[in] compare_side : whether compare side number or not
/// @param[in] side_num : side number if compare number
/// @param[in] delay_clock : offset clock
/// @param[in] timeout_round : number of round to occur timeout
/// @param[out] arrive_clock : clock until reach the sector
/// @param[in] skip_unmatch : skip sector if unmatch track or side number
/// @return bit0:Record Not Found / bit1:CRC Error / bit2:Deleted Mark Detected
int FLOPPY::search_sector_and_get_clock(int channel, int track_num, int sector_num, bool compare_side, int side_num, int delay_clock, int timeout_round, int &arrive_clock, bool skip_unmatch)
{
	int drvnum = get_drive_number(channel);
	if (drvnum >= USE_FLOPPY_DISKS) return STS_RECORD_NOT_FOUND;
	int status = 0;
	int sector_pos = 0;
	int remain_clock = get_index_hole_remain_clock(delay_clock);

	do {
		int sta = p_disk[drvnum]->search_sector_and_get_clock(m_density, sector_num, sector_pos, remain_clock, timeout_round, arrive_clock);
#ifdef _DEBUG_TIME
		logging->out_debugf(_T("FLOPPY::ss_gc: sect_num:%02d sect_pos:%02d delay:%06d remain:%06d arrive:%06d total:%06d")
			, sector_num, sector_pos
			, delay_clock, remain_clock, arrive_clock, arrive_clock + delay_clock
		);
#endif
		arrive_clock += delay_clock;
		if (sta) {
			disp_message_searched_sector(drvnum, sta);
			status = STS_RECORD_NOT_FOUND; // SECTOR NOT FOUND
			break;
		}
		DISK_SECTOR *current_sector = p_disk[drvnum]->get_current_sector();
		if (current_sector) {
			// check track number in id field
			if (current_sector->get_sector_id(0) != (uint8_t)track_num) {
				status |= STS_UNMATCH_TRACK_NUMBER;
				if (skip_unmatch) {
					// recalc clock if skip sector
					arrive_clock = remain_clock + m_delay_index_hole * timeout_round;
				}
				break;
			}
			// check side number in id field
			if(compare_side && current_sector->get_sector_id(1) != (uint8_t)side_num) {
				status |= STS_UNMATCH_SIDE_NUMBER;
				if (skip_unmatch) {
					// recalc clock if skip sector
					arrive_clock = remain_clock + m_delay_index_hole * timeout_round;
				}
				break;
			}
			if (!m_ignore_crc && current_sector->has_crc_error(m_density)) {
				status |= STS_CRC_ERROR;	// CRC ERROR
			}
			if (current_sector->has_deleted_mark()) {
				status |= STS_DELETED_MARK_DETECTED;	// DELETED MARK DETECTED
			}
		}
	} while(0);

	return status;
}

/// @brief Find the nearest sector from the current head position and calculate the time (clocks) required to reach that sector.
///
/// @param[in] channel : FDC number(upper 16bit) 
/// @param[in] delay_clock : offset clock
/// @param[in] timeout_round : number of round to occur timeout
/// @param[out] arrive_clock : clock until reach the sector
/// @return bit0:Record Not Found / bit1:CRC Error / bit2:Deleted Mark Detected
int FLOPPY::search_next_sector_and_get_clock(int channel, int delay_clock, int timeout_round, int &arrive_clock)
{
	int drvnum = get_drive_number(channel);
	if (drvnum >= USE_FLOPPY_DISKS) return STS_RECORD_NOT_FOUND;
	int status = 0;
	int sector_pos = -1;
	int remain_clock = get_index_hole_remain_clock(delay_clock);

	do {
		if (FLG_DELAY_FDSEARCH) {
			if (m_next_sector_pos < 0) {
				m_next_sector_pos = 0;
			}
			sector_pos = m_next_sector_pos;
		}
		// search nearest sector
		int sta = p_disk[drvnum]->search_sector_and_get_clock(m_density, -1, sector_pos, remain_clock, timeout_round, arrive_clock);
#ifdef _DEBUG_TIME
		logging->out_debugf(_T("FLOPPY::sns_gc: sect_pos:%02d delay:%06d remain:%06d arrive:%06d total:%06d")
			, sector_pos
			, delay_clock, remain_clock, arrive_clock, arrive_clock + delay_clock
		);
#endif
		arrive_clock += delay_clock;
		if (sta) {
			disp_message_searched_sector(drvnum, sta);
			status = STS_RECORD_NOT_FOUND; // SECTOR NOT FOUND
			break;
		}
		if (FLG_DELAY_FDSEARCH) {
			m_next_sector_pos++;
			m_next_sector_pos %= p_disk[drvnum]->get_number_of_sector(1);
		}
		DISK_SECTOR *current_sector = p_disk[drvnum]->get_current_sector();
		if (current_sector) {
			if (!m_ignore_crc && current_sector->has_crc_error(m_density)) {
				status |= STS_CRC_ERROR;	// CRC ERROR
			}
			if (current_sector->has_deleted_mark()) {
				status |= STS_DELETED_MARK_DETECTED;	// DELETED MARK DETECTED
			}
		}
	} while(0);

	return status;
} // nc

/// Calculate the time (clocks) to reach at the sepcified sector
///
/// @param[in] channel : fdc number 
/// @param[in] sector_num : sector number
/// @param[in] delay_clock : offset clock
/// @param[in] timeout_round : number of round to occur timeout
/// @return clocks
int FLOPPY::get_clock_arrival_sector(int channel, int sector_num, int delay_clock, int timeout_round)
{
	int drvnum = get_drive_number(channel);
	if (drvnum >= USE_FLOPPY_DISKS) return m_delay_index_hole;
	m_next_sector_pos = -1;
	int arrive_clock = p_disk[drvnum]->get_clock_arrival_sector(sector_num, -1, get_index_hole_remain_clock(delay_clock), timeout_round);
	arrive_clock += delay_clock;

//	logging->out_debugf(_T("get_clock_arrival_sector: sect:%02d delay:%d sum:%06d"), sector_num, delay_clock, arrive_clock);
	return arrive_clock;
}

/// Calculate the time (clocks) to reach the next sector
///
/// @param[in] channel : fdc number 
/// @param[in] delay_clock : offset clock
/// @param[in] timeout_round : number of round to occur timeout
/// @return clocks
int FLOPPY::get_clock_next_sector(int channel, int delay_clock, int timeout_round)
{
	int drvnum = get_drive_number(channel);
	if (drvnum >= USE_FLOPPY_DISKS) return m_delay_index_hole;

	int arrive_clock = p_disk[drvnum]->get_clock_arrival_sector(-1, -1, get_index_hole_remain_clock(delay_clock), timeout_round);
	arrive_clock += delay_clock;

//	logging->out_debugf(_T("get_clock_next_sector: cur_clk:%lld sum:%06d"), get_current_clock(), arrive_clock);
	return arrive_clock;
}

// ----------------------------------------------------------------------------

/// Get the clock until the disk head is loaded
///
/// @param[in] channel : fdc number 
/// @param[in] delay_time : delay_time (us) 
/// @return clocks
int FLOPPY::get_head_loading_clock(int channel, int delay_time)
{
	int drvnum = get_drive_number(channel);
	if (drvnum >= USE_FLOPPY_DISKS) return HEAD_LOADED_CLOCK;
	return (p_disk[drvnum]->is_head_loading()) ? (delay_time * CPU_CLOCKS / 1000000) : HEAD_LOADED_CLOCK;
}

/// Calculate the time (clocks) to reach the index hole
///
/// @param[in] channel : fdc number 
/// @param[in] delay_time : delay_time (us) 
/// @return clocks
int FLOPPY::get_index_hole_search_clock(int channel, int delay_time)
{
	int delay_clock = get_head_loading_clock(channel, delay_time);
	int arrive_clock = get_index_hole_remain_clock(delay_clock);
	m_next_sector_pos = -1;

//	logging->out_debugf(_T("calc_index_hole_search_clock: idx:%06d sum:%06d"), idx_sum, sum);
	return arrive_clock;
}

/// Get the clock until the disk drive detects the index hole 
///
/// @param[in] delay_clock
/// @return clock that is less than one round clock
int FLOPPY::get_index_hole_remain_clock(int delay_clock)
{
	int64_t sum = m_index_hole_next_clock - get_current_clock() - delay_clock;
	while (sum < 0) {
		// add one round clock
		sum += m_delay_index_hole;
	}
	return (int)sum;
}

// ----------------------------------------------------------------------------
// irq / drq
// ----------------------------------------------------------------------------
void FLOPPY::set_irq(bool val)
{
	// NMI
	write_signals(&outputs_irq, val ? 0xffffffff : 0);

	if (pConfig->fdd_type == FDD_TYPE_58FDD && val == true) {
		// clear HALT signal
		d_board->write_signal(SIG_CPU_HALT, 0, SIG_HALT_FD_MASK);
	}
}

void FLOPPY::set_drq(bool val)
{
	// Always relase HALT signal
	write_signals(&outputs_drq, 0);
}

// ----------------------------------------------------------------------------
// user interface
// ----------------------------------------------------------------------------
/// Open a floppy disk image
///
/// @param[in] drv : drive number
/// @param[in] path : file path
/// @param[in] offset : start position of file
/// @param[in] flags : bit0: 1 = read only  bit3: 1= last volume  bit4: 1 = multi volumed file
/// @return true if success
bool FLOPPY::open_disk(int drv, const _TCHAR *path, int offset, uint32_t flags)
{
	if(drv < USE_FLOPPY_DISKS) {
		bool rc = p_disk[drv]->open(path, offset, flags);
		return rc;
	} else {
		return false;
	}
}

/// Close the floppy disk image
///
/// @param[in] drv : drive number
/// @param[in] flags : OPEN_DISK_FLAGS_FORCELY
/// @return true if success
bool FLOPPY::close_disk(int drv, uint32_t flags)
{
	if(drv < USE_FLOPPY_DISKS) {
		if (flags & OPEN_DISK_FLAGS_FORCELY) {
			p_disk[drv]->close();
		} else {
			p_disk[drv]->close();
			set_disk_side(drv, 0);
			motor(drv, false);
		}
	}
	return true;
}

/// Change the surface (side) of the disk on the drive
///
/// @param[in] drv : drive number
/// @return side number
int FLOPPY::change_disk(int drv)
{
	if(drv < USE_FLOPPY_DISKS) {
		set_disk_side(drv, 1 - p_disk[drv]->i_side);
		motor(drv, false);
		return p_disk[drv]->i_side;
	} else {
		return 0;
	}
}

/// Set the side number on the drive
///
/// @param[in] drv : drive number
/// @param[in] side : side number
void FLOPPY::set_disk_side(int drv, int side)
{
	p_disk[drv]->set_side_position(side);
	BIT_ONOFF(m_sidereg, 1 << drv, side);
}

/// Get the side number on the drive
///
/// @param[in] drv : drive number
/// @return side number
int FLOPPY::get_disk_side(int drv)
{
	return p_disk[drv]->get_side_position();
}

/// Is the disk inserted in the drive?
///
/// @param[in] drv : drive number
/// @return true if inserted
bool FLOPPY::disk_inserted(int drv)
{
	if(drv < USE_FLOPPY_DISKS) {
		return p_disk[drv]->is_inserted();
	}
	return false;
}

uint8_t FLOPPY::fdc_status()
{
	return 0;
}

/// Toggle write protection in the disk on the drive?
///
/// @param[in] drv : drive number
void FLOPPY::toggle_disk_write_protect(int drv)
{
	if(drv < USE_FLOPPY_DISKS) {
		if (m_ignore_write) {
			m_ignore_write = false;
			p_disk[drv]->set_write_protect(true);
		}
		p_disk[drv]->set_write_protect(!(p_disk[drv]->is_write_protected()));
	}
}

/// Is the disk protected writing on the drive?
///
/// @param[in] drv : drive number
/// @return true if write protected
bool FLOPPY::disk_write_protected(int drv)
{
	if(drv < USE_FLOPPY_DISKS) {
		return (p_disk[drv]->is_write_protected() || m_ignore_write);
	}
	return true;
}

/// Is the same disk inserted in the drive?
///
/// @param[in] drv : drive number
/// @param[in] file_path : the file
/// @param[in] offset : offset in the file
/// @return true if the same file
bool FLOPPY::is_same_disk(int drv, const _TCHAR *file_path, int offset)
{
	if(drv < USE_FLOPPY_DISKS) {
		return p_disk[drv]->is_same_file(file_path, offset);
	}
	return false;
}

/// Is the disk already inserted in another drive?
///
/// @param[in] drv : drive number
/// @param[in] file_path : the file
/// @param[in] offset : offset in the file
/// @return -1 if no match
int FLOPPY::inserted_disk_another_drive(int drv, const _TCHAR *file_path, int offset)
{
	int match = -1;
	for(int i=0; i<USE_FLOPPY_DISKS; i++) {
		if (i == drv) continue;
		if (p_disk[i]->is_same_file(file_path, offset)) {
			match = i;
			break;
		}
	}
	return match;
}

// ----------------------------------------------------------------------------

/// for led indicator
///
/// @return b3-b0: drive select, b4-b7: 0:green led, 1:red led b8-11:inserted?
uint16_t FLOPPY::get_drive_select()
{
	uint16_t data = 0;

	if (!pConfig->now_power_off) {
		switch(pConfig->fdd_type) {
		case FDD_TYPE_3FDD:
			// 3inch
			if (m_motor_on_expand[0]) {
				data = (m_drvsel[0] & 0x0f) | ((m_sidereg & 0x0f) << 4);
			}
			break;
		case FDD_TYPE_5FDD:
		case FDD_TYPE_58FDD:
			// 5inch or 8inch
			for (int fdcnum = 0; fdcnum < 1; fdcnum++) {
				int i = m_drv_num[fdcnum];
				if (p_disk[i]->now_disk_accessing()) {
					data |= (0x11 << i);
				}
			}
			break;
		}
	}

	// inserted diskette ?
	for(int i=0; i<USE_FLOPPY_DISKS; i++) {
		if (p_disk[i]->is_inserted()) {
			data |= (0x100 << i);
		}
	}

	return data;
}

// ----------------------------------------------------------------------------
void FLOPPY::mix(int32_t* buffer, int cnt)
{
	for(int ty=0; ty<FLOPPY_WAV_SNDTYPES; ty++) {
		m_noises[m_wav_fddtype][ty].mix(buffer, cnt);
	}

}

void FLOPPY::set_volume(int decibel, bool vol_mute)
{
	int wav_volume = (int)(16384.0 * pow(10.0, decibel / 40.0));
	if (vol_mute) wav_volume = 0;

	for(int ft=0; ft<FLOPPY_WAV_FDDTYPES; ft++) {
		for(int ty=0; ty<FLOPPY_WAV_SNDTYPES; ty++) {
			m_noises[ft][ty].set_volume(wav_volume, wav_volume);
		}
	}
}

void FLOPPY::initialize_sound(int rate, int decibel)
{
	m_sample_rate = rate;
	set_volume(decibel, false);

	// load wav file
	load_wav();
}

// ----------------------------------------------------------------------------
void FLOPPY::save_state(FILEIO *fp)
{
	struct vm_state_st_v2 vm_state;

	//
	vm_state_ident.version = Uint16_LE(7);
	vm_state_ident.size = Uint32_LE(sizeof(vm_state_ident) + sizeof(vm_state));

	// copy values
	memset(&vm_state, 0, sizeof(vm_state));
	for(int i=0; i<22 && i<FLOPPY_MAX_EVENT; i++) {
		vm_state.register_id[i] = Int32_LE(m_register_id[i]);
	}
	vm_state.drv_num = m_drv_num[0];
	vm_state.drvsel  = m_drvsel[0];
	vm_state.sidereg = m_sidereg;
	vm_state.fdd5outreg = m_fdd5outreg[0];
	vm_state.irqflg  = m_irqflg ? 1 : 0;
	vm_state.drqflg  = m_drqflg ? 1 : 0;
	vm_state.flags   = m_density;
	vm_state.fdd5outreg_delay = m_fdd5outreg_delay; // version 7

	// sound
	for(int ft=0; ft<FLOPPY_WAV_FDDTYPES; ft++) {
		for(int ty=0; ty<FLOPPY_WAV_SNDTYPES; ty++) {
			m_noises[ft][ty].save_state(vm_state.m_noises[ft][ty]);
		}
	}

	// fdd info (version 7)
	for(int i=0; i<USE_FLOPPY_DISKS; i++) {
		p_disk[i]->save_state(vm_state.m_fdd[i]);
	}

	fp->Fwrite(&vm_state_ident, sizeof(vm_state_ident), 1);
	fp->Fwrite(&vm_state, sizeof(vm_state), 1);
}

bool FLOPPY::load_state(FILEIO *fp)
{
	vm_state_ident_t vm_state_i;
	struct vm_state_st_v1 vm_state_v1;
	struct vm_state_st_v2 vm_state;

	FIND_STATE_CHUNK(fp, vm_state_i);

	// copy values
	if (Uint16_LE(vm_state_i.version) == 1) {
		READ_STATE_CHUNK(fp, vm_state_i, vm_state_v1);

		for(int i=0; i<FLOPPY_MAX_EVENT_V1; i++) {
			m_register_id[i] = Int32_LE(vm_state_v1.register_id[i]);
		}
		for(int i=FLOPPY_MAX_EVENT_V1; i<FLOPPY_MAX_EVENT; i++) {
			m_register_id[i] = -1;
		}

		m_drv_num[0] = vm_state_v1.drv_num;
		m_drvsel[0]  = vm_state_v1.drvsel;
		m_sidereg = vm_state_v1.sidereg;
		m_fdd5outreg[0] = vm_state_v1.fdd5outreg;
		m_irqflg  = vm_state_v1.irqflg ? true : false;
		m_drqflg  = vm_state_v1.drqflg ? true : false;

		// cannot write to disk when load state from file.
		m_next_sector_pos = -1;
		m_ignore_write = true;

	} else {
		READ_STATE_CHUNK(fp, vm_state_i, vm_state);

		for(int i=0; i<22 && i<FLOPPY_MAX_EVENT; i++) {
			m_register_id[i] = Int32_LE(vm_state.register_id[i]);
		}

		m_drv_num[0] = vm_state.drv_num;
		m_drvsel[0]  = vm_state.drvsel;
		m_sidereg = vm_state.sidereg;
		m_fdd5outreg[0] = vm_state.fdd5outreg;
		m_irqflg  = vm_state.irqflg ? true : false;
		m_drqflg  = vm_state.drqflg ? true : false;

		if ((Uint16_LE(vm_state_i.version) & 0x2f) >= 7) {
			m_fdd5outreg_delay = (vm_state.fdd5outreg_delay);
		} else {
			m_fdd5outreg_delay = (m_fdd5outreg[0] & 0x80);
		}
	}
	// shift event id
	if ((Uint16_LE(vm_state_i.version) & 0x2f) < 4) {
		for(int i=FLOPPY_MAX_EVENT-1; i>1; i--) {
			m_register_id[i] = m_register_id[i-1];
		}
	}
	if ((Uint16_LE(vm_state_i.version) & 0x2f) >= 5) {
		m_density = vm_state.flags;
	}

	m_irqflgprev = m_irqflg;
	m_drqflgprev = m_drqflg;

	// sound off (version 6)
	for(int ft=0; ft<FLOPPY_WAV_FDDTYPES; ft++) {
		for(int ty=0; ty<FLOPPY_WAV_SNDTYPES; ty++) {
			if ((Uint16_LE(vm_state_i.version) & 0x2f) >= 6) {
				m_noises[ft][ty].load_state(vm_state.m_noises[ft][ty]);
			} else {
				m_noises[ft][ty].stop();
			}
		}
	}

	// fdd type is set on MEMORY class
	set_drive_speed();

	for(int n = 0; n < MAX_FDC_NUMS; n++) {
		m_motor_on_expand[n] = 0;
	}

	// fdd info (version 7)
	if ((Uint16_LE(vm_state_i.version) & 0x2f) >= 7) {
		for(int i=0; i<USE_FLOPPY_DISKS; i++) {
			p_disk[i]->load_state(vm_state.m_fdd[i]);
		}
		// set current track and sector
		for(int i=0; i<USE_FLOPPY_DISKS; i++) {
			p_disk[i]->restore_state(m_density);
		}
	}

	return true;
}

// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
uint32_t FLOPPY::debug_read_io8(uint32_t addr)
{
	uint32_t data = 0xff;

	if (IOPORT_USE_5FDD) {
		// 5inch or 8inch
		switch(addr & 0x1f) {
			case 0x0:
			case 0x1:
				// FDC access ok
				if (pConfig->fdd_type != FDD_TYPE_58FDD || (pConfig->fdd_type == FDD_TYPE_58FDD && (m_drvsel[0] & 0x80) == 0)) {
					data = d_fdc5[0]->debug_read_io8(addr & 0xf);
				}
				break;
			case 0x2:
				// FDC access ok
				if (pConfig->fdd_type != FDD_TYPE_58FDD || (pConfig->fdd_type == FDD_TYPE_58FDD && (m_drvsel[0] & 0x80) == 0)) {
					data = d_fdc5[0]->debug_read_io8(addr & 0xf);
				} else {
					// When FDC access mask is ON, send DRQ signal
					data = (m_fdd5outreg[0] | 0x7f);
				}
				break;
			case 0x3:
				// FDC access ok or DRQ on
				if (pConfig->fdd_type != FDD_TYPE_58FDD || (pConfig->fdd_type == FDD_TYPE_58FDD && ((m_drvsel[0] & m_fdd5outreg_delay & 0x80) == 0))) {
					data = d_fdc5[0]->debug_read_io8(addr & 0xf);
				}
				break;
			case 0x4:
				if (pConfig->fdd_type == FDD_TYPE_58FDD) data = 0xff;
				else data = (m_fdd5outreg[0] | 0x7e);
				break;
			case 0xc:
				// Type
				if (pConfig->fdd_type == FDD_TYPE_58FDD) {
					// always zero
					data = 0x00;
				}
				break;
		}
	} else if (IOPORT_USE_3FDD) {
		// 3inch
		data = m_drvsel[0];
	}

	return data;
}

bool FLOPPY::debug_write_reg(uint32_t reg_num, uint32_t data)
{
	if (IOPORT_USE_5FDD) {
		// 5inch or 8inch
		switch (reg_num) {
		case 0:
			m_drvsel[0] = data;
			return true;
		}
	} else if (IOPORT_USE_3FDD) {
		// 3inch
		switch (reg_num) {
		case 0:
			m_drvsel[0] = data;
			return true;
		}
	}
	return false;
}

static const _TCHAR *c_reg_names[] = {
	_T("FDD3_UNITSEL"),
	_T("FDD5_2D_UNITSEL"),
	_T("FDD8_2D_UNITSEL"),
	NULL
};

bool FLOPPY::debug_write_reg(const _TCHAR *reg, uint32_t data)
{
	uint32_t num = find_debug_reg_name(c_reg_names, reg);
	switch(num) {
	case 0:
		if (IOPORT_USE_3FDD) {
			return debug_write_reg(num, data);
		}
		break;
	case 1:
		if (IOPORT_USE_5FDD) {
			if (pConfig->fdd_type != FDD_TYPE_58FDD) {
				num = 0;
				return debug_write_reg(num, data);
			}
		}
		break;
	case 2:
		if (IOPORT_USE_5FDD) {
			if (pConfig->fdd_type == FDD_TYPE_58FDD) {
				num = 0;
				return debug_write_reg(num, data);
			}
		}
		break;
	default:
		break;
	}
	return false;
}

void FLOPPY::debug_regs_info(const _TCHAR *title, _TCHAR *buffer, size_t buffer_len)
{
	UTILITY::tcscpy(buffer, buffer_len, title);
	UTILITY::tcscat(buffer, buffer_len, _T(" Registers:\n"));
	if (IOPORT_USE_5FDD) {
		// 5inch or 8inch
		if (pConfig->fdd_type == FDD_TYPE_58FDD) {
			UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%s):%02X"), 0, c_reg_names[2], m_drvsel[0]);
		} else {
			UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%s):%02X"), 0, c_reg_names[1], m_drvsel[0]);
		}
	} else if (IOPORT_USE_3FDD) {
		// 3inch
		UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%s):%02X"), 0, c_reg_names[0], m_drvsel[0]);
	} else {
		// no fdd
		UTILITY::tcscat(buffer, buffer_len, _T(" FDD is not available."));
	}
}

void FLOPPY::debug_status_info(int type, _TCHAR *buffer, size_t buffer_len)
{
	buffer[0] = _T('\0');
	switch(type) {
	case VM::DNM_FDDINFO:
		// fdd information
		DISK_DRIVE::debug_param_header(buffer, buffer_len);
		for(int i=0; i<USE_FLOPPY_DISKS; i++) {
			p_disk[i]->debug_param_data(buffer, buffer_len);
		}
		break;
	case VM::DNM_FDD0:
	case VM::DNM_FDD1:
	case VM::DNM_FDD2:
	case VM::DNM_FDD3:
		{
			int i = type - VM::DNM_FDD0;
			if (i < USE_FLOPPY_DISKS) {
				p_disk[i]->debug_media_info(buffer, buffer_len);
			} else {
				UTILITY::sntprintf(buffer, buffer_len, _T(" FDD Drive:%d Not Supported."), i);
			}
		}
		break;
	default:
		break;
	}
}

#endif /* USE_DEBUGGER */
