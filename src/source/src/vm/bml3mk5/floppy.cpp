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
#include "../disk.h"
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
#define OUT_DEBUG dummyf
#define OUT_DEBUG_REGR(emu, addr, data, msg)
#endif

#define DRIVE_MASK	(MAX_DRIVE - 1)

void FLOPPY::cancel_my_event(int event_no)
{
	if(register_id[event_no] != -1) {
		cancel_event(this, register_id[event_no]);
		register_id[event_no] = -1;
	}
}

void FLOPPY::register_my_event(int event_no, int wait)
{
	cancel_my_event(event_no);
	register_event(this, event_no, wait, false, &register_id[event_no]);
}

void FLOPPY::register_index_hole_event(int event_no, int wait)
{
	cancel_my_event(event_no);
	register_event_by_clock(this, event_no, wait, false, &register_id[event_no], &index_hole_next_clock);
}

void FLOPPY::initialize()
{
	// setup/reset floppy drive
	for(int i = 0; i < MAX_DRIVE; i++) {
		disk[i] = new DISK(i);
	}
	for(int i = 0; i < MAX_DRIVE; i++) {
		fdd[i].side = 0;
		fdd[i].track = 0;
		fdd[i].index = 0;
#ifdef USE_SIG_FLOPPY_ACCESS
		fdd[i].access = false;
#endif
		fdd[i].ready = 0;
		fdd[i].motor_warmup = 0;
		fdd[i].head_loading = 0;
		fdd[i].delay_write = 0;

		fdd[i].shown_media_error = false;
	}
	ignore_crc = false;

	sidereg = 0;
	sectorcnt = 0;
	sectorcnt_cont = false;

	index_hole = 0;
	index_hole_next_clock = 0;
	head_load = 0;

	for(int i = 0; i < FLOPPY_MAX_EVENT; i++) {
		register_id[i] = -1;
	}

//	wav_enable = 0;
	m_wav_fddtype = FLOPPY_WAV_FDD3;
	wav_loaded_at_first = false;

#if 0
	for(int ty = 0; ty < FLOPPY_WAV_SNDTYPES; ty++) {
		for(int ft = 0; ft < FLOPPY_WAV_FDDTYPES; ft++) {
			wav_data[ft][ty] = NULL;
			wav_size[ft][ty] = 0;
		}
		wav_play_pos[ty] = 0;
		wav_play[ty] = 0;
	}
	UTILITY::tcscpy(wav_file[0][0], sizeof(wav_file[0][0]) / sizeof(wav_file[0][0][0]), _T("fddseek3.wav"));
	UTILITY::tcscpy(wav_file[0][1], sizeof(wav_file[0][1]) / sizeof(wav_file[0][1][0]), _T("fddmotor3.wav"));
	UTILITY::tcscpy(wav_file[0][2], sizeof(wav_file[0][2]) / sizeof(wav_file[0][2][0]), _T("fddheadon3.wav"));
	UTILITY::tcscpy(wav_file[0][3], sizeof(wav_file[0][3]) / sizeof(wav_file[0][3][0]), _T("fddheadoff3.wav"));
	UTILITY::tcscpy(wav_file[1][0], sizeof(wav_file[1][0]) / sizeof(wav_file[1][0][0]), _T("fddseek5.wav"));
	UTILITY::tcscpy(wav_file[1][1], sizeof(wav_file[1][1]) / sizeof(wav_file[1][1][0]), _T("fddmotor5.wav"));
	UTILITY::tcscpy(wav_file[1][2], sizeof(wav_file[1][2]) / sizeof(wav_file[1][2][0]), _T("fddheadon5.wav"));
	UTILITY::tcscpy(wav_file[1][3], sizeof(wav_file[1][3]) / sizeof(wav_file[1][3][0]), _T("fddheadoff5.wav"));
	UTILITY::tcscpy(wav_file[2][0], sizeof(wav_file[2][0]) / sizeof(wav_file[2][0][0]), _T("fddseek8.wav"));
	UTILITY::tcscpy(wav_file[2][1], sizeof(wav_file[2][1]) / sizeof(wav_file[2][1][0]), _T("fddmotor8.wav"));
	UTILITY::tcscpy(wav_file[2][2], sizeof(wav_file[2][2]) / sizeof(wav_file[2][2][0]), _T("fddheadon8.wav"));
	UTILITY::tcscpy(wav_file[2][3], sizeof(wav_file[2][3]) / sizeof(wav_file[2][3][0]), _T("fddheadoff8.wav"));
#endif
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
	m_wav_fddtype = config.fdd_type - 1;
	if (m_wav_fddtype < 0) m_wav_fddtype = 0;

	load_wav();

	for(int i = 0; i < MAX_DRIVE; i++) {
		fdd[i].track = 0;
		fdd[i].index = 0;
#ifdef USE_SIG_FLOPPY_ACCESS
		fdd[i].access = false;
#endif
		fdd[i].ready = 0;
		fdd[i].motor_warmup = 0;
		fdd[i].head_loading = 0;

		fdd[i].shown_media_error = false;

#if defined(_MBS1)
		// set drive type
		if (config.fdd_type == FDD_TYPE_58FDD) {
			// 5inch 2HD type
			if (i < 2) {
				set_drive_type(i, DRIVE_TYPE_2HD);
			} else {
				set_drive_type(i, DRIVE_TYPE_2D);
			}
		} else {
			set_drive_type(i, DRIVE_TYPE_2D);
		}
#else
		// set drive type
		if (config.fdd_type == FDD_TYPE_58FDD) {
			// 8inch type
			set_drive_type(i, DRIVE_TYPE_2HD);
		} else {
			set_drive_type(i, DRIVE_TYPE_2D);
		}
#endif
	}

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
			register_id[i] = -1;
		}
	}
	for(int i = 0; i < MAX_DRIVE; i++) {
		fdd[i].ready = 0;
		fdd[i].motor_warmup = 0;
		fdd[i].head_loading = 0;
		fdd[i].shown_media_error = false;
	}

	for(int i = 0; i < MAX_FDC_NUMS; i++) {
		drv_num[i] = 0;
		drvsel[i] = 0;

		motor_on_expand[i] = 0;

		fdd5outreg[i] = 0x81;
	}
	fdd5outreg_delay = 0;

	index_hole = 0;

	set_drive_speed();

	density = 0;

	irqflg = false;
	irqflgprev = irqflg;
	drqflg = false;
	drqflgprev = drqflg;

	ignore_write = false;

	for(int ft = 0; ft < FLOPPY_WAV_FDDTYPES; ft++) {
		for(int ty = 0; ty < FLOPPY_WAV_SNDTYPES; ty++) {
//			wav_play[ty] = 0;
			m_noises[ft][ty].stop();
		}
	}

	if (IOPORT_USE_FDD) {
		register_my_event(EVENT_INDEXHOLE_ON, delay_index_hole);	// index hole
		if (config.fdd_type != FDD_TYPE_58FDD) {
			// motor off time
			register_my_event(EVENT_MOTOR_TIMEOUT, DELAY_MOTOR_TIMEOUT);	// delay motor off(timeout) (60s)
		}
		d_fdc5[0]->write_signal(MB8866::SIG_MB8866_CLOCKNUM, config.fdd_type == FDD_TYPE_58FDD ? 1 : 0, 1);
	}
}

void FLOPPY::cancel_my_events()
{
	for(int i = 0; i < FLOPPY_MAX_EVENT; i++) {
		cancel_my_event(i);
	}
}

void FLOPPY::release()
{
	// release d88 handler
	for(int i = 0; i < MAX_DRIVE; i++) {
		delete disk[i];
	}
#if 0
	for(int ft = 0; ft < FLOPPY_WAV_FDDTYPES; ft++) {
		for(int ty = 0; ty < FLOPPY_WAV_SNDTYPES; ty++) {
			delete[] wav_data[ft][ty];
		}
	}
#endif
}

void FLOPPY::load_wav()
{
	// load wav file
	const _TCHAR *app_path, *rom_path[2];
//	_TCHAR file_path[_MAX_PATH];
//	wav_header_t	 head;
//	wav_fmt_chank_t  fmt;
//	wav_data_chank_t data;
//	size_t			 data_len;
//	size_t           data_max[FLOPPY_WAV_SNDTYPES];
//	PARSEWAV::Util   wavu;

	rom_path[0] = config.rom_path;
	rom_path[1] = vm->application_path();

#if 0
	data_max[FLOPPY_WAV_SEEK] = (sample_rate / 5);	// 0.2sec (seek)
	data_max[FLOPPY_WAV_MOTOR] = (sample_rate / 5);	// 0.2sec (moror)
	data_max[FLOPPY_WAV_HEADON] = (sample_rate / 5);	// 0.2sec (head on)
	data_max[FLOPPY_WAV_HEADOFF] = (sample_rate / 5);	// 0.2sec (head off)
#endif
	for (int ft=0; ft<FLOPPY_WAV_FDDTYPES; ft++) {
		for (int ty=0; ty<FLOPPY_WAV_SNDTYPES; ty++) {
#if 0
			int j = (ft * FLOPPY_WAV_SNDTYPES + ty);
			if ((wav_enable & (1 << j)) == 0) {
				wav_data[ft][ty] = new uint8_t[data_max[ty]];
				memset(wav_data[ft][ty], 128, sizeof(uint8_t) * data_max[ty]);
			}
#endif
			m_noises[ft][ty].alloc(sample_rate / 5);	// 0.2sec
		}
	}

//	FILEIO* fre = new FILEIO();
	for(int i=0; i<2; i++) {
		app_path = rom_path[i];

		for (int ft=0; ft<FLOPPY_WAV_FDDTYPES; ft++) {
			for (int ty=0; ty<FLOPPY_WAV_SNDTYPES; ty++) {
#if 0
				int j = (ft * FLOPPY_WAV_SNDTYPES + ty);
				if ((wav_enable & (1 << j)) == 0) {
					UTILITY::stprintf(file_path, _MAX_PATH, _T("%s%s"), app_path, wav_file[ft][ty]);
					if(fre->Fopen(file_path, FILEIO::READ_BINARY)) {
						if (wavu.CheckWavFormat(*fre, &head, &fmt, &data, &data_len) >= 0) {
							wav_size[ft][ty] = (int)wavu.ReadWavData(*fre, &fmt, data_len, wav_data[ft][ty], sample_rate, 8, data_max[ty]);
							wav_enable |= (1 << j);
							logging->out_logf_x(LOG_INFO, CMsg::VSTR_was_loaded, wav_file[ft][ty]);
						}
						fre->Fclose();
					}
				}
#endif
				if (m_noises[ft][ty].load_wav_file(app_path, sample_rate) > 0) {
					logging->out_logf_x(LOG_INFO, CMsg::VSTR_was_loaded, m_noises[ft][ty].get_file_name());
				}
			}
		}
	}
//	delete fre;

	for (int ft=0; ft<FLOPPY_WAV_FDDTYPES; ft++) {
		for (int ty=0; ty<FLOPPY_WAV_SNDTYPES; ty++) {
#if 0
			int j = (ft * FLOPPY_WAV_SNDTYPES + ty);
			if ((wav_enable & (1 << j)) == 0) {
				wav_size[ft][ty] = 0;
				delete[] wav_data[ft][ty];
				wav_data[ft][ty] = NULL;
				if (!wav_loaded_at_first) {
					OUT_DEBUG(_T("%s couldn't be loaded."),wav_file[ft][ty]);
				}
			}
#endif
			if (!m_noises[ft][ty].is_enable()) {
				m_noises[ft][ty].clear();
				if (!wav_loaded_at_first) {
					OUT_DEBUG(_T("%s couldn't be loaded."), m_noises[ft][ty].get_file_name());
				}
			}
		}
	}
	wav_loaded_at_first = true;
}

void FLOPPY::write_io8(uint32_t addr, uint32_t data)
{
	if (IOPORT_USE_5FDD) {
		// 5inch or 8inch
		switch(addr & 0x1f) {
			case 0x0:
			case 0x1:
			case 0x2:
				// FDC access ok
				if (config.fdd_type != FDD_TYPE_58FDD || (config.fdd_type == FDD_TYPE_58FDD && (drvsel[0] & 0x80) == 0)) {
					OUT_DEBUG(_T("fddw a:%05x d:%02x (%02x)"),addr,data,(~data) & 0xff);
					d_fdc5[0]->write_io8(addr & 0xf, data);
				} else {
					OUT_DEBUG(_T("fddw a:%05x d:%02x (%02x) BLOCKED"),addr,data,(~data) & 0xff);
				}
				break;
			case 0x3:
				// FDC access ok or DRQ on
				if (config.fdd_type != FDD_TYPE_58FDD || (config.fdd_type == FDD_TYPE_58FDD && ((drvsel[0] & fdd5outreg_delay & 0x80) == 0))) {
					OUT_DEBUG(_T("fddw a:%05x d:%02x (%02x)"),addr,data,(~data) & 0xff);
					d_fdc5[0]->write_io8(addr & 0xf, data);
				} else {
					OUT_DEBUG(_T("fddw a:%05x d:%02x (%02x) BLOCKED"),addr,data,(~data) & 0xff);
				}
				break;
			case 0x4:
				set_drive_select(1, data);
				if ((drvsel[0] & 0x80) == 0) fdd5outreg_delay &= 0x7f;
				OUT_DEBUG(_T("fddw a:%05x d:%02x"),addr,data);
				break;
			case 0x8:
				// Send HALT signal if DRQ signal on FDC is false. 
				if (config.fdd_type == FDD_TYPE_58FDD && !drqflg) {
					OUT_DEBUG(_T("fddw a:%05x d:%02x drq:%02x"),addr,data,drqflg);
					d_board->write_signal(SIG_CPU_HALT, SIG_HALT_FD_MASK, SIG_HALT_FD_MASK);
				}
				break;
			case 0xc:
				break;
		}
	} else if (IOPORT_USE_3FDD) {
		// 3inch
		set_drive_select(0, data);
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
		OUT_DEBUG(_T("fddr a:%05x d:%02x (%02x)%s"), addr, data, (~data) & 0xff, msg != NULL ? msg : _T(""));
		debug_regr_cnt[n] = 0;
	} else {
		debug_regr_cnt[n]++;
	}
	debug_regr[n] = data;
}
#endif

uint32_t FLOPPY::read_io8(uint32_t addr)
{
	uint32_t data = 0xff;

	if (IOPORT_USE_5FDD) {
		// 5inch or 8inch
		switch(addr & 0x1f) {
			case 0x0:
			case 0x1:
				// FDC access ok
				if (config.fdd_type != FDD_TYPE_58FDD || (config.fdd_type == FDD_TYPE_58FDD && (drvsel[0] & 0x80) == 0)) {
					data = d_fdc5[0]->read_io8(addr & 0xf);
					OUT_DEBUG_REGR(emu, addr, data, NULL);
				} else {
					OUT_DEBUG_REGR(emu, addr, data, _T(" BLOCKED"));
				}
				break;
			case 0x2:
				// FDC access ok
				if (config.fdd_type != FDD_TYPE_58FDD || (config.fdd_type == FDD_TYPE_58FDD && (drvsel[0] & 0x80) == 0)) {
					data = d_fdc5[0]->read_io8(addr & 0xf);
					OUT_DEBUG_REGR(emu, addr, data, NULL);
				} else {
					// When FDC access mask is ON, send DRQ signal
					data = (fdd5outreg[0] | 0x7f);
					fdd5outreg_delay = data;
					OUT_DEBUG_REGR(emu, addr, data, _T(" Wait DRQ"));
				}
				break;
			case 0x3:
				// FDC access ok or DRQ on
				if (config.fdd_type != FDD_TYPE_58FDD || (config.fdd_type == FDD_TYPE_58FDD && ((drvsel[0] & fdd5outreg_delay & 0x80) == 0))) {
					data = d_fdc5[0]->read_io8(addr & 0xf);
					OUT_DEBUG_REGR(emu, addr, data, NULL);
				} else {
					OUT_DEBUG_REGR(emu, addr, data, _T(" BLOCKED"));
				}
				break;
			case 0x4:
				if (config.fdd_type == FDD_TYPE_58FDD) data = 0xff;
				else data = (fdd5outreg[0] | 0x7e);
				OUT_DEBUG_REGR(emu, addr, data, NULL);
				break;
			case 0xc:
				// Type
				if (config.fdd_type == FDD_TYPE_58FDD) {
					// always zero
					data = 0x00;
				}
				break;
		}
	} else if (IOPORT_USE_3FDD) {
		// 3inch
		data = drvsel[0];
	}

	return data;
}

void FLOPPY::write_signal(int id, uint32_t data, uint32_t mask)
{
	int fdcnum = (id >> 16);
	id &= 0xffff;

	int ndrv_num = drv_num[fdcnum];

	switch (id) {
#ifdef USE_SIG_FLOPPY_ACCESS
		case SIG_FLOPPY_ACCESS:
			fdd[ndrv_num].access = (data & 1) ? true : false;
			break;
#endif
		case SIG_FLOPPY_WRITE:
			if(!ignore_write && fdd[ndrv_num].index < disk[ndrv_num]->sector_size) {
				disk[ndrv_num]->sector_data[fdd[ndrv_num].index] = (data & mask);
				fdd[ndrv_num].index++;
				fdd[ndrv_num].delay_write = DELAY_WRITE_FRAME;
			}
			break;
		case SIG_FLOPPY_WRITE_TRACK:
			if(!ignore_write && fdd[ndrv_num].index < disk[ndrv_num]->track_size) {
				disk[ndrv_num]->track[fdd[ndrv_num].index] = (data & mask);
				fdd[ndrv_num].index++;
				fdd[ndrv_num].delay_write = DELAY_WRITE_FRAME;
			}
			break;
		case SIG_FLOPPY_WRITEDELETE:
			if (!ignore_write) {
				disk[ndrv_num]->deleted = (data & 1) ? 0x10 : 0;
			}
			break;
		case SIG_FLOPPY_STEP:
			{
				int cur_track = fdd[ndrv_num].track;

				if (data < 0x80) fdd[ndrv_num].track++;
				else if (data > 0x80) fdd[ndrv_num].track--;

				if (fdd[ndrv_num].track < 0) fdd[ndrv_num].track = 0;
				if (fdd[ndrv_num].track > 255) fdd[ndrv_num].track = 255;

				if (cur_track != fdd[ndrv_num].track) {
					// fddseek sound turn on
					m_noises[m_wav_fddtype][FLOPPY_WAV_SEEK].play();
//					wav_play[FLOPPY_WAV_SEEK] = 1;
//					wav_play_pos[FLOPPY_WAV_SEEK] = 0;
				}
			}
			break;
		case SIG_FLOPPY_TRACK_SIZE:
			if (config.fdd_type == FDD_TYPE_58FDD) {
				// 26sectors max
				disk[ndrv_num]->track_size = TRACK_SIZE_8INCH_2D;
			} else {
				// 16sectors max
				disk[ndrv_num]->track_size = TRACK_SIZE_5INCH_2D;
			}
			fdd[ndrv_num].index = 0;
			break;
#ifdef SET_CURRENT_TRACK_IMMEDIATELY
		case SIG_FLOPPY_CURRENTTRACK:
			fdd[ndrv_num].track = (data & mask);
			break;
#endif
		case SIG_FLOPPY_HEADLOAD:
			head_load = (data & mask);
			if (disk[ndrv_num]->inserted) {
				if (head_load && !fdd[ndrv_num].head_loading) {
					// fddheadon sound turn on
					m_noises[m_wav_fddtype][FLOPPY_WAV_HEADON].play();
//					wav_play[FLOPPY_WAV_HEADON] = 1;
//					wav_play_pos[FLOPPY_WAV_HEADON] = 0;
				}
			}
			if (head_load) {
				fdd[ndrv_num].head_loading = 120;
			}
//			if (config.fdd_type == FDD_TYPE_58FDD) {
//				// 8inch doesn't have motor on/off signal
//				motor(ndrv_num, head_load != 0);
//			}
			OUT_DEBUG(_T("fdd %d sig HEADLOAD %x UL:%d"),ndrv_num,head_load,fdd[ndrv_num].head_loading);
			break;
		case SIG_CPU_RESET:
			now_reset = (data & mask) ? true : false;
			warm_reset(false);
			if (now_reset) {
				set_drive_select(IOPORT_USE_3FDD ? 0 : 1, 0);
			}
			break;
	}

	if (IOPORT_USE_5FDD) {
		// for 5inch mini floppy
		fdcnum = (mask >> 16);
		mask &= 0xffff;
		uint8_t nfdd5outreg = fdd5outreg[fdcnum];

		switch (id) {
			case SIG_FLOPPY_IRQ:
				irqflg = (data & mask) ? true : false;
				if (irqflg) {
					nfdd5outreg &= 0xfe;
				} else {
					nfdd5outreg |= 0x01;
				}
				break;
			case SIG_FLOPPY_DRQ:
				drqflg = (data & mask) ? true : false;
				if (drqflg) {
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
			if (irqflg != irqflgprev) {
				if (config.fdd_type == FDD_TYPE_58FDD || (drvsel[fdcnum] & 0x40) == 0) {	// nmi non mask
					register_my_event(EVENT_IRQ, 20);	// delay
				}
				irqflgprev = irqflg;
			}
			break;
		case SIG_FLOPPY_DRQ:
			if (drqflg != drqflgprev) {
				if (config.fdd_type == FDD_TYPE_58FDD) { // halt release
//					register_my_event(EVENT_DRQ, 20);	// delay
					set_drq(drqflg);					// immediate
				}
				drqflgprev = drqflg;
			}
			break;
		}

		fdd5outreg[fdcnum] = nfdd5outreg;
	}
}

uint32_t FLOPPY::read_signal(int id)
{
	uint32_t data = 0;

	int fdcnum = (id >> 16);
	id &= 0xffff;

	int ndrv_num = drv_num[fdcnum];

	switch (id) {
		case SIG_FLOPPY_READ_ID:
			if(fdd[ndrv_num].index < 6) {
				data = disk[ndrv_num]->id[fdd[ndrv_num].index];
				fdd[ndrv_num].index++;
			}
			break;
		case SIG_FLOPPY_READ:
			if(fdd[ndrv_num].index < disk[ndrv_num]->sector_size) {
				data = disk[ndrv_num]->sector_data[fdd[ndrv_num].index];
				fdd[ndrv_num].index++;
			}
			break;
		case SIG_FLOPPY_READ_TRACK:
			if(fdd[ndrv_num].index < disk[ndrv_num]->track_size) {
				data = disk[ndrv_num]->track[fdd[ndrv_num].index];
				fdd[ndrv_num].index++;
			}
			break;
		case SIG_FLOPPY_WRITEPROTECT:
			data = (disk[ndrv_num]->write_protected || ignore_write) ? 1 : 0;
			break;
		case SIG_FLOPPY_HEADLOAD:
			data = (fdd[ndrv_num].head_loading & 0xff) ? 1 : 0;
			break;
		case SIG_FLOPPY_READY:
			data = (disk[ndrv_num]->inserted && fdd[ndrv_num].ready >= 2) ? 1 : 0;
			break;
		case SIG_FLOPPY_TRACK0:
			data = (fdd[ndrv_num].track == 0) ? 1 : 0;
			break;
		case SIG_FLOPPY_INDEX:
			data = index_hole;
			break;
		case SIG_FLOPPY_DELETED:
			data = disk[ndrv_num]->deleted ? 1 : 0;
			break;
#ifdef SET_CURRENT_TRACK_IMMEDIATELY
		case SIG_FLOPPY_CURRENTTRACK:
			data = fdd[ndrv_num].track;
			break;
#endif
		case SIG_FLOPPY_SECTOR_NUM:
			data = disk[ndrv_num]->sector_nums;
			break;
		case SIG_FLOPPY_SECTOR_SIZE:
			data = disk[ndrv_num]->sector_size;
			break;
		case SIG_FLOPPY_TRACK_SIZE:
			data = disk[ndrv_num]->track_size;
			break;
		case SIG_FLOPPY_DENSITY:
			data = density;
			break;
	}

	return data;
}

// ----------------------------------------------------------------------------
// event handler
// ----------------------------------------------------------------------------

void FLOPPY::event_frame()
{
	for(int i=0; i<MAX_DRIVE; i++) {
		if (fdd[i].head_loading & 0xff) {
			fdd[i].head_loading--;
			if (disk[i]->inserted && fdd[i].head_loading == 0) {
				// fddheadoff sound turn on
				m_noises[m_wav_fddtype][FLOPPY_WAV_HEADOFF].play();
//				wav_play[FLOPPY_WAV_HEADOFF] = 1;
//				wav_play_pos[FLOPPY_WAV_HEADOFF] = 0;
			}
		}
		if (fdd[i].delay_write > 0) {
			fdd[i].delay_write--;
			if (fdd[i].delay_write == 0) {
				disk[i]->flash();
			}
		}
	}
}

void FLOPPY::event_callback(int event_id, int err)
{
	int event_no = event_id;

	register_id[event_no] = -1;

	switch(event_no) {
		case EVENT_MOTOR_TIMEOUT:
			// motor off
			motor(-1, false);
			break;
		case EVENT_INDEXHOLE_ON:
			// index hole on
			index_hole = 1;
			register_index_hole_event(EVENT_INDEXHOLE_OFF, limit_index_hole);	// index hole off
			index_hole_next_clock += (delay_index_hole - limit_index_hole); 
			break;
		case EVENT_INDEXHOLE_OFF:
			// index hole off
			index_hole = 0;
			for(int i=0; i<MAX_DRIVE; i++) {
				if (fdd[i].motor_warmup) {
					fdd[i].motor_warmup <<= 1;
					if (fdd[i].motor_warmup > 4) fdd[i].motor_warmup = 0;
					OUT_DEBUG(_T("fdd %d event MOTOR WARMUP %d"), i, fdd[i].motor_warmup);
				}
			}
			register_index_hole_event(EVENT_INDEXHOLE_ON, delay_index_hole - limit_index_hole);	// index hole on
			break;
		case EVENT_IRQ:
			cancel_my_event(EVENT_IRQ);
			set_irq(irqflg);
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
				fdd[i].ready <<= 1;
				OUT_DEBUG(_T("fdd %d event READY %d"), i, fdd[i].ready);
			}
			break;
		case EVENT_MOTOR_OFF:
			// motor off
			motor(-1, false);
			break;
	}

}

// ----------------------------------------------------------------------------
/// @param [in] type: 0:3inch 1:5inch 2:unused
/// @param [in] data: unit sel
void FLOPPY::set_drive_select(int type, uint8_t data)
{
	uint8_t ndrv_num = 0;
	int fdcnum = (type == 2 ? 1 : 0);

	if (now_reset) {
		if (type != 0) {
			// 5inch
			data = 0;
		} else {
			// 3inch
			data = 0xff;
		}
	}

	// drive number
	if (type == 2) {
		// 5inch ex always use drive 2 or 3
		ndrv_num = (data & DRIVE_MASK) | 2;

	} else if (type != 0) {
		// 5inch
		ndrv_num = (data & DRIVE_MASK);

//		d_fdc5[0]->write_signal(SIG_MB8866_DRIVEREG, ndrv_num, DRIVE_MASK);
	} else {
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
		if (drv_num[fdcnum] != ndrv_num) {
			// update status
			d_fdc3->write_signal(MC6843::SIG_MC6843_UPDATESTATUS, 0, 0);
		}
	}
	drv_num[fdcnum] = ndrv_num;

	// set drive speed
	set_drive_speed();

	// side and motor
	if (type != 0) {
		// 5inch
		// side select
		set_disk_side(drv_num[fdcnum], (data & 0x10) ? 1 : 0);
//		d_fdc5[0]->write_signal(SIG_MB8866_SIDEREG, data, 0x10);
		// density
		density = (data & 0x20) ? 1 : 0;
		d_fdc5[0]->write_signal(SIG_FLOPPY_DENSITY, density, 1);

		// motor
		if ((data & 0x08) == 0 && motor_on_expand[fdcnum] != 0 && config.fdd_type != FDD_TYPE_58FDD) {
			// motor on -> off
			OUT_DEBUG(_T("fdd %d MOTOR OFF REQUEST data:%02x drvsel:%02x"), drv_num[fdcnum], data, drvsel[fdcnum]);
			register_my_event(EVENT_MOTOR_OFF, DELAY_MOTOR_OFF_5);	// delay motor off
			motor_on_expand[fdcnum] = 1;
			drvsel[fdcnum] = (data & 0xff);
		} else if ((data & 0x08) != 0 || config.fdd_type == FDD_TYPE_58FDD) {
			// motor on
			motor(drv_num[fdcnum], true);
			drvsel[fdcnum] = (data & 0xff);
			motor_on_expand[fdcnum] = 1;
		} else {
			// motor off
			OUT_DEBUG(_T("fdd %d MOTOR OFF FORCE data:%02x drvsel:%02x"), drv_num[fdcnum], data, drvsel[fdcnum]);
			motor(-1, false);
		}
//		d_fdc5[0]->write_signal(SIG_MB8866_MOTOR, drvsel, 0x08);

	} else {
		// 3inch
		// motor
		if ((data & 0x80) == 0 && motor_on_expand[fdcnum] != 0) {
			// motor on -> off
			OUT_DEBUG(_T("fdd %d MOTOR OFF REQUEST data:%02x drvsel:%02x"), drv_num[fdcnum], data, drvsel[fdcnum]);
			register_my_event(EVENT_MOTOR_OFF, DELAY_MOTOR_OFF_3);	// delay motor off
			motor_on_expand[fdcnum] = 1;
			drvsel[fdcnum] = (data & 0xff);
		} else if ((data & 0x80) != 0) {
			// motor on
			motor(drv_num[fdcnum], true);
			drvsel[fdcnum] = (data & 0xff);
			motor_on_expand[fdcnum] = 1;
		} else {
			// motor off
			OUT_DEBUG(_T("fdd %d MOTOR OFF FORCE data:%02x drvsel:%02x"), drv_num[fdcnum], data, drvsel[fdcnum]);
			motor(-1, false);
		}

	}
}

void FLOPPY::motor(int drv, bool val)
{
	if (val) {
		// motor on
		if (config.fdd_type != FDD_TYPE_58FDD) {
			register_my_event(EVENT_MOTOR_TIMEOUT, DELAY_MOTOR_TIMEOUT);	// delay motor off(timeout) (60s)
		}

		// motor sound on
//		if (wav_play[FLOPPY_WAV_MOTOR] == 0 && disk[drv]->inserted) {
//			wav_play[FLOPPY_WAV_MOTOR] = 1;
//		}
		if (!m_noises[m_wav_fddtype][FLOPPY_WAV_MOTOR].now_playing() && disk[drv]->inserted) {
			m_noises[m_wav_fddtype][FLOPPY_WAV_MOTOR].play();
		}

		OUT_DEBUG(_T("fdd %d MOTOR ON REQUEST READY:%d WARMUP:%d"), drv, fdd[drv].ready, fdd[drv].motor_warmup);

		// ready on
		if (fdd[drv].ready == 0) {
			fdd[drv].ready = 1;
			register_my_event(EVENT_READY_ON_0 + drv, delay_ready_on);	// delay ready on (1s)
			if (disk[drv]->inserted) {
				fdd[drv].motor_warmup = 1;
			}
		}

		cancel_my_event(EVENT_MOTOR_OFF);

		OUT_DEBUG(_T("fdd %d MOTOR ON  READY:%d WARMUP:%d"), drv, fdd[drv].ready, fdd[drv].motor_warmup);
	} else {
		// motor off
		ignore_write = false;

//		cancel_my_event(EVENT_MOTOR_OFF);

		// motor sound off
//		wav_play[FLOPPY_WAV_MOTOR] = 0;
		uint8_t wav_playing = 0;
		for(int i=0; i<MAX_DRIVE; i++) {
			if (drv == -1 || drv == i) {
				fdd[i].ready = 0;
				fdd[i].motor_warmup = 0;
			}
//			wav_play[FLOPPY_WAV_MOTOR] |= fdd[i].ready;
			wav_playing |= fdd[i].ready;
		}
//		if (wav_play[FLOPPY_WAV_MOTOR] < 2) {
		if (wav_playing < 2) {
			// reset motor flag when all drives stopped
			for(int n = 0; n < MAX_FDC_NUMS; n++) {
				motor_on_expand[n] = 0;
			}

			if (IOPORT_USE_3FDD) {
				// 3inch
				for(int n = 0; n < MAX_FDC_NUMS; n++) {
					drvsel[n] &= 0x7f;
				}
			} else if (IOPORT_USE_5FDD) {
				// 5inch
				for(int n = 0; n < MAX_FDC_NUMS; n++) {
					drvsel[n] &= 0xf7;
				}
			}
			m_noises[m_wav_fddtype][FLOPPY_WAV_MOTOR].stop();
		}

		OUT_DEBUG(_T("fdd %d MOTOR OFF  SOUND:%d"), drv, wav_playing);
	}
}

void FLOPPY::set_drive_speed()
{
	if (config.fdd_type == FDD_TYPE_58FDD) {
		delay_index_hole = DELAY_INDEX_HOLE_H;
		delay_ready_on = DELAY_READY_ON_H;
	} else {
		delay_index_hole = DELAY_INDEX_HOLE;
		delay_ready_on = DELAY_READY_ON;
	}
	delay_index_hole = (int)((double)delay_index_hole * CPU_CLOCKS / 1000000.0);
	limit_index_hole = (int)(300.0 * CPU_CLOCKS / 1000000.0);

	delay_index_mark = 147 * 8;
	if (config.fdd_type != FDD_TYPE_58FDD) {
		delay_index_mark *= 2;
	}
	delay_index_mark = (int)((double)delay_index_mark * CPU_CLOCKS / 1000000.0);
}

// ----------------------------------------------------------------------------
// media handler
// ----------------------------------------------------------------------------
bool FLOPPY::search_track(int channel)
{
	int fdcnum = (channel >> 16);
	int drvnum = drv_num[fdcnum];

	fdd[drvnum].index = 0;

	return disk[drvnum]->get_track(fdd[drvnum].track, fdd[drvnum].side);
}

bool FLOPPY::verify_track(int channel, int track)
{
	// verify track number
	int fdcnum = (channel >> 16);
	int drvnum = drv_num[fdcnum];
	return disk[drvnum]->verify_track(track);
}

int FLOPPY::get_current_track_number(int channel)
{
	int fdcnum = (channel >> 16);
	int drvnum = drv_num[fdcnum];
	return disk[drvnum]->id_c_in_track[0];
}

/// search sector
///
/// @param[in] fdcnum : FDC number
/// @param[in] drvnum : drive number
/// @param[in] index  : position of target sector in track
/// @return bit0:Record Not Found / bit1:CRC Error / bit2:Deleted Mark Detected
int FLOPPY::search_sector_main(int fdcnum, int drvnum, int index)
{
	int status = 0;

	do {
		int sta = disk[drvnum]->get_sector_by_index(fdd[drvnum].track, fdd[drvnum].side, index);
		if (sta) {
			if (sta == 3 && !fdd[drvnum].shown_media_error) {
				// different media type
				logging->out_logf_x(LOG_ERROR, CMsg::The_media_type_in_drive_VDIGIT_is_different_from_specified_one, drvnum); 
				fdd[drvnum].shown_media_error = true;
			}
			status = 1; // SECTOR NOT FOUND
			break;
		}
		// check density
		if (FLG_CHECK_FDDENSITY) {
			if (disk[drvnum]->density) {
				uint8_t sector_density = (((*disk[drvnum]->density) & 0x40) ? 0 : 1);
				if (density ^ sector_density) {
					if (!fdd[drvnum].shown_media_error) {
						logging->out_logf_x(LOG_ERROR, CMsg::The_density_in_track_VDIGIT_side_VDIGIT_is_different_from_specified_one, fdd[drvnum].track, fdd[drvnum].side); 
						fdd[drvnum].shown_media_error = true;
					}
					status = 1; // SECTOR NOT FOUND
					break;
				}
			}
		}

		fdd[drvnum].index = 0;

		if (disk[drvnum]->status && !ignore_crc) {
			status |= 2;	// CRC ERROR
		}
		if (disk[drvnum]->deleted) {
			status |= 4;	// DELETED MARK DETECTED
		}
	} while(0);

	return status;
}

/// search sector
///
/// @param[in] channel: FDC number(upper 16bit) 
/// @return bit0:Record Not Found / bit1:CRC Error / bit2:Deleted Mark Detected
int FLOPPY::search_sector(int channel)
{
	int fdcnum = (channel >> 16);
	int drvnum = drv_num[fdcnum];

	int sector_num = disk[drvnum]->sector_nums;
	if (sector_num <= 0) sector_num = 16;
	sectorcnt = (sectorcnt % sector_num);

	return search_sector_main(fdcnum, drvnum, sectorcnt);
}

//int FLOPPY::search_sector(int channel, int sect)
//{
//	return search_sector(channel, sect, false, 0);
//}

/// search sector
///
/// @param[in] channel: FDC number(upper 16bit) 
/// @param[in] track  : track number
/// @param[in] sect   : sector number
/// @param[in] compare_side : whether compare side number or not
/// @param[in] side   : side number if compare number
/// @return bit0:Record Not Found / bit1:CRC Error / bit2:Deleted Mark Detected
int FLOPPY::search_sector(int channel, int track, int sect, bool compare_side, int side)
{
	int fdcnum = (channel >> 16);
	int drvnum = drv_num[fdcnum];
	int sector_num = disk[drvnum]->sector_nums;
	if (sector_num <= 0) sector_num = 16;
	if (sect < 256) {
		sectorcnt = disk[drvnum]->sector_pos[sect];
		sectorcnt = (sectorcnt % sector_num);
	}

	int status = search_sector_main(fdcnum, drvnum, sectorcnt);
	do {
		if (status) {
			break;
		}

		// check track in id field
		if(disk[drvnum]->id[0] != track) {
			status = 1;
			break;
		}
		// check sector in id field
		if(disk[drvnum]->id[2] != sect) {
			status = 1;
			break;
		}
		// check side in id field
		if(compare_side && disk[drvnum]->id[1] != side) {
			status = 1;
			break;
		}
	} while(0);


	return status;
}

bool FLOPPY::make_track(int channel)
{
	int fdcnum = (channel >> 16);
	int drvnum = drv_num[fdcnum];
	return disk[drvnum]->make_track(fdd[drvnum].track, fdd[drvnum].side, density);
}

bool FLOPPY::parse_track(int channel)
{
	int fdcnum = (channel >> 16);
	int drvnum = drv_num[fdcnum];
//	return disk[drvnum]->parse_track(fdd[drvnum].track, fdd[drvnum].side, density);
	return disk[drvnum]->parse_track2(fdd[drvnum].track, fdd[drvnum].side, density);
}

// ----------------------------------------------------------------------------

int FLOPPY::get_head_loading_clock(int channel)
{
	int fdcnum = (channel >> 16);
	return (fdd[drv_num[fdcnum]].head_loading & 0xff) ? 200 : HEAD_LOADED_CLOCK;
}

int FLOPPY::get_index_hole_remain_clock()
{
	int64_t sum = 0;
	sum = index_hole_next_clock - get_current_clock();
	if (sum < 0) sum = 0;
	return (int)sum;
}

int FLOPPY::calc_index_hole_search_clock(int channel)
{
	int sum = get_head_loading_clock(channel);
	int idx_sum = 0;
	sectorcnt_cont = false;
	if (!FLG_DELAY_FDSEARCH) {
		idx_sum = get_index_hole_remain_clock();
		if (sum >= idx_sum) sum = idx_sum;
		else sum = idx_sum + delay_index_hole;
	}

//	logging->out_debugf(_T("calc_index_hole_search_clock: idx:%06d sum:%06d"), idx_sum, sum);
	return sum;
}

int FLOPPY::get_clock_arrival_sector(int channel, int sect, int delay)
{
	int fdcnum = (channel >> 16);
	int sector_num = disk[drv_num[fdcnum]]->sector_nums;
	if (sector_num <= 0) sector_num = 16;
	int sector_pos = 0;
	if (sect < 256) {
		sector_pos = disk[drv_num[fdcnum]]->sector_pos[sect];
		sector_pos = (sector_pos % sector_num);
	}

	int sect_sum = sector_pos * (delay_index_hole - delay_index_mark) / sector_num;
	int idx_sum = get_index_hole_remain_clock() - delay;
	int sum = sect_sum + delay_index_mark + idx_sum - delay_index_hole;
	if (sum < 0) sum += delay_index_hole;
	sum += delay;

//	logging->out_debugf(_T("get_clock_arrival_sector: sect:%02d %06d idx:%06d sum:%06d"), sect, sect_sum, delay_index_hole - idx_sum, sum);
	return (sum);
}

int FLOPPY::get_clock_next_sector(int channel, int delay)
{
	int fdcnum = (channel >> 16);
	int sector_num = disk[drv_num[fdcnum]]->sector_nums;
	if (sector_num <= 0) sector_num = 16;

	int sect_sum = (delay_index_hole - delay_index_mark) / sector_num;
	int idx_sum = get_index_hole_remain_clock() - delay;
	if (idx_sum < 0) idx_sum += delay_index_hole;
	int sector_pos = sector_num - (idx_sum / sect_sum);
	if (sector_pos < 0) sector_pos = 0;

	if (FLG_DELAY_FDSEARCH && sectorcnt_cont) {
		sector_pos = sectorcnt + 1;
	}

	sector_pos %= sector_num;

	sectorcnt = sector_pos;
	sectorcnt_cont = true;

	if (FLG_DELAY_FDSEARCH) return 200;

	sect_sum = sector_pos * sect_sum;
	int sum = sect_sum + delay_index_mark + idx_sum - delay_index_hole;
	if (sum < 0) sum += delay_index_hole;
	sum += delay;

//	logging->out_debugf(_T("get_clock_next_sector: sect_pos:%02d cur_clk:%lld next_clk:%lld idx:%06d sect_sum:%06d sum:%06d"), sector_pos, get_current_clock(), index_hole_next_clock, idx_sum, sect_sum, sum);
	return (sum);
}

int FLOPPY::calc_sector_search_clock(int channel, int sect)
{
	int sum_clk = 0;
	sectorcnt_cont = false;
	if (!FLG_DELAY_FDSEARCH) sum_clk = get_clock_arrival_sector(channel, sect, get_head_loading_clock(channel));
//	logging->out_debugf(_T("calc_sector_search_clock: sect:%02d hld:%06d clk:%06d"), sect, sum_hld, sum_clk);
	return sum_clk;
}

int FLOPPY::calc_next_sector_clock(int channel)
{
	return get_clock_next_sector(channel, get_head_loading_clock(channel));
}

// ----------------------------------------------------------------------------
// irq / drq
// ----------------------------------------------------------------------------
void FLOPPY::set_irq(bool val)
{
	// NMI
	write_signals(&outputs_irq, val ? 0xffffffff : 0);

	if (config.fdd_type == FDD_TYPE_58FDD && val == true) {
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
bool FLOPPY::open_disk(int drv, const _TCHAR *path, int offset, uint32_t flags)
{
	if(drv < MAX_DRIVE) {
		bool rc = disk[drv]->open(path, offset, flags);
#if 0
		if (IOPORT_USE_5FDD) {
			// 5inch
			if (rc) {
				motor(drv, true);
				register_my_event(EVENT_MOTOR_OFF, DELAY_MOTOR_OFF_5S);	// delay motor off
			}
		}
#endif
		if (rc) {
			if (!(flags & OPEN_DISK_FLAGS_FORCELY)) {
				for(int i=0; i<MAX_DRIVE; i++) {
					if (i == drv) continue;
					if (disk[i]->is_same_file(path, offset)) {
						int drvmin = (i < drv ? i : drv);
						int drvmax = (i < drv ? drv : i);
						logging->out_logf_x(LOG_WARN, CMsg::There_is_the_same_disk_in_drive_VDIGIT_and_VDIGIT, drvmin, drvmax);
						break;
					}
				}
			}
			fdd[drv].delay_write = 0;
			fdd[drv].shown_media_error = false;
		}
		return rc;
	} else {
		return false;
	}
}

bool FLOPPY::close_disk(int drv, uint32_t flags)
{
	if(drv < MAX_DRIVE) {
		if (flags & OPEN_DISK_FLAGS_FORCELY) {

			disk[drv]->close();

		} else {

			disk[drv]->close();
			set_disk_side(drv, 0);
			motor(drv, false);
			fdd[drv].delay_write = 0;

		}
	}
	return true;
}

int FLOPPY::change_disk(int drv)
{
	if(drv < MAX_DRIVE) {
		set_disk_side(drv, 1 - fdd[drv].side);
		motor(drv, false);
		return fdd[drv].side;
	} else {
		return 0;
	}
}

void FLOPPY::set_disk_side(int drv, int side)
{
	fdd[drv].side = (side & 1) % disk[drv]->num_of_side;
	if (fdd[drv].side) {
		sidereg |= (1 << drv);
	} else {
		sidereg &= ~(1 << drv);
	}
}

int FLOPPY::get_disk_side(int drv)
{
	return (disk[drv]->num_of_side > 1 ? fdd[drv].side % disk[drv]->num_of_side : -1);
}

bool FLOPPY::disk_inserted(int drv)
{
	if(drv < MAX_DRIVE) {
		return disk[drv]->inserted;
	}
	return false;
}

void FLOPPY::set_drive_type(int drv, uint8_t type)
{
	if(drv < MAX_DRIVE) {
		disk[drv]->set_drive_type(type);
	}
}

uint8_t FLOPPY::get_drive_type(int drv)
{
	if(drv < MAX_DRIVE) {
		return disk[drv]->get_drive_type();
	}
	return DRIVE_TYPE_UNK;
}

uint8_t FLOPPY::fdc_status()
{
	return 0;
}

void FLOPPY::toggle_disk_write_protect(int drv)
{
	if(drv < MAX_DRIVE) {
		if (ignore_write) {
			ignore_write = false;
			disk[drv]->set_write_protect(true);
		}
		disk[drv]->set_write_protect(!(disk[drv]->write_protected));
	}
}

bool FLOPPY::disk_write_protected(int drv)
{
	if(drv < MAX_DRIVE) {
		return (disk[drv]->write_protected || ignore_write);
	}
	return true;
}

bool FLOPPY::is_same_disk(int drv, const _TCHAR *file_path, int offset)
{
	if(drv < MAX_DRIVE) {
		return disk[drv]->is_same_file(file_path, offset);
	}
	return false;
}

// ----------------------------------------------------------------------------
uint16_t FLOPPY::get_drive_select()
{
	// for led indicator
	// b3-b0: drive select, b4-b7: 0:green led, 1:red led b8-11:inserted?
	uint16_t data = 0;

	if (!config.now_power_off) {
		switch(config.fdd_type) {
		case FDD_TYPE_3FDD:
			// 3inch
			if (motor_on_expand[0]) {
				data = (drvsel[0] & 0x0f) | ((sidereg & 0x0f) << 4);
			}
			break;
		case FDD_TYPE_5FDD:
		case FDD_TYPE_58FDD:
			// 5inch or 8inch
			int i = drv_num[0];
			if (fdd[i].head_loading > 0 && fdd[i].ready >= 2) {
				data |= ((1 << i) | (0x10 << i));
			}
			break;
		}
	}

	// inserted diskette ?
	for(int i=0; i<MAX_DRIVE; i++) {
		if (disk[i]->inserted) {
			data |= (1 << (8 + i));
		}
	}

	return data;
}

// ----------------------------------------------------------------------------
void FLOPPY::mix(int32_t* buffer, int cnt)
{
//	int32_t *buffer_start = buffer;
//	int fdd_type = (config.fdd_type > 0) ? (config.fdd_type - 1) : 0;

	for(int ty=0; ty<FLOPPY_WAV_SNDTYPES; ty++) {
#if 0
		if (wav_play[ty] && wav_size[m_wav_fddtype][ty]) {
			buffer = buffer_start;
			for(int i=0; i<cnt; i++) {
				*buffer++ += ((int)wav_data[m_wav_fddtype][ty][wav_play_pos[ty]] - 128) * wav_volume / 128;
				*buffer++ += ((int)wav_data[m_wav_fddtype][ty][wav_play_pos[ty]] - 128) * wav_volume / 128;
				wav_play_pos[ty]++;

				if (wav_play_pos[ty] >= wav_size[m_wav_fddtype][ty]) {
					wav_play_pos[ty] = 0;
					if (ty != FLOPPY_WAV_MOTOR) {
						// motor sound play loop.
						// seek sound play only one time.
						wav_play[ty] = 0;
						break;
					}
				}
			}
		}
#endif
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
	sample_rate = rate;
//	max_vol = volume;
	set_volume(decibel, false);

	// load wav file
	load_wav();
}

// ----------------------------------------------------------------------------
void FLOPPY::save_state(FILEIO *fp)
{
	struct vm_state_st_v2 vm_state;

	//
	vm_state_ident.version = Uint16_LE(6);
	vm_state_ident.size = Uint32_LE(sizeof(vm_state_ident) + sizeof(vm_state));

	// copy values
	memset(&vm_state, 0, sizeof(vm_state));
	for(int i=0; i<22 && i<FLOPPY_MAX_EVENT; i++) {
		vm_state.register_id[i] = Int32_LE(register_id[i]);
	}
	vm_state.drv_num = drv_num[0];
	vm_state.drvsel  = drvsel[0];
	vm_state.sidereg = sidereg;
	vm_state.fdd5outreg = fdd5outreg[0];
	vm_state.irqflg  = irqflg ? 1 : 0;
	vm_state.drqflg  = drqflg ? 1 : 0;
	vm_state.flags   = density;

	// sound
	for(int ft=0; ft<FLOPPY_WAV_FDDTYPES; ft++) {
		for(int ty=0; ty<FLOPPY_WAV_SNDTYPES; ty++) {
			m_noises[ft][ty].save_state(vm_state.m_noises[ft][ty]);
		}
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
			register_id[i] = Int32_LE(vm_state_v1.register_id[i]);
		}
		for(int i=FLOPPY_MAX_EVENT_V1; i<FLOPPY_MAX_EVENT; i++) {
			register_id[i] = -1;
		}

		drv_num[0] = vm_state_v1.drv_num;
		drvsel[0]  = vm_state_v1.drvsel;
		sidereg = vm_state_v1.sidereg;
		fdd5outreg[0] = vm_state_v1.fdd5outreg;
		irqflg  = vm_state_v1.irqflg ? true : false;
		drqflg  = vm_state_v1.drqflg ? true : false;

		// cannot write to disk when load state from file.
		sectorcnt = 0;
		sectorcnt_cont = false;
		ignore_write = true;

	} else {
		READ_STATE_CHUNK(fp, vm_state_i, vm_state);

		for(int i=0; i<22 && i<FLOPPY_MAX_EVENT; i++) {
			register_id[i] = Int32_LE(vm_state.register_id[i]);
		}

		drv_num[0] = vm_state.drv_num;
		drvsel[0]  = vm_state.drvsel;
		sidereg = vm_state.sidereg;
		fdd5outreg[0] = vm_state.fdd5outreg;
		fdd5outreg_delay = (fdd5outreg[0] & 0x80);
		irqflg  = vm_state.irqflg ? true : false;
		drqflg  = vm_state.drqflg ? true : false;

	}
	// shift event id
	if (Uint16_LE(vm_state_i.version) < 4) {
		for(int i=FLOPPY_MAX_EVENT-1; i>1; i--) {
			register_id[i] = register_id[i-1];
		}
	}
	if (Uint16_LE(vm_state_i.version) >= 5) {
		density = vm_state.flags;
	}

	irqflgprev = irqflg;
	drqflgprev = drqflg;

	// sound off
	for(int ft=0; ft<FLOPPY_WAV_FDDTYPES; ft++) {
		for(int ty=0; ty<FLOPPY_WAV_SNDTYPES; ty++) {
//			wav_play[ty] = 0;
			if (Uint16_LE(vm_state_i.version) >= 6) {
				m_noises[ft][ty].load_state(vm_state.m_noises[ft][ty]);
			} else {
				m_noises[ft][ty].stop();
			}
		}
	}

	// fdd type is set on MEMORY class
	set_drive_speed();

	for(int n = 0; n < MAX_FDC_NUMS; n++) {
		motor_on_expand[n] = 0;
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
				if (config.fdd_type != FDD_TYPE_58FDD || (config.fdd_type == FDD_TYPE_58FDD && (drvsel[0] & 0x80) == 0)) {
					data = d_fdc5[0]->debug_read_io8(addr & 0xf);
				}
				break;
			case 0x2:
				// FDC access ok
				if (config.fdd_type != FDD_TYPE_58FDD || (config.fdd_type == FDD_TYPE_58FDD && (drvsel[0] & 0x80) == 0)) {
					data = d_fdc5[0]->debug_read_io8(addr & 0xf);
				} else {
					// When FDC access mask is ON, send DRQ signal
					data = (fdd5outreg[0] | 0x7f);
				}
				break;
			case 0x3:
				// FDC access ok or DRQ on
				if (config.fdd_type != FDD_TYPE_58FDD || (config.fdd_type == FDD_TYPE_58FDD && ((drvsel[0] & fdd5outreg_delay & 0x80) == 0))) {
					data = d_fdc5[0]->debug_read_io8(addr & 0xf);
				}
				break;
			case 0x4:
				if (config.fdd_type == FDD_TYPE_58FDD) data = 0xff;
				else data = (fdd5outreg[0] | 0x7e);
				break;
			case 0xc:
				// Type
				if (config.fdd_type == FDD_TYPE_58FDD) {
					// always zero
					data = 0x00;
				}
				break;
		}
	} else if (IOPORT_USE_3FDD) {
		// 3inch
		data = drvsel[0];
	}

	return data;
}

bool FLOPPY::debug_write_reg(uint32_t reg_num, uint32_t data)
{
	if (IOPORT_USE_5FDD) {
		// 5inch or 8inch
		switch (reg_num) {
		case 0:
			drvsel[0] = data;
			return true;
		}
	} else if (IOPORT_USE_3FDD) {
		// 3inch
		switch (reg_num) {
		case 0:
			drvsel[0] = data;
			return true;
		}
	}
	return false;
}

bool FLOPPY::debug_write_reg(const _TCHAR *reg, uint32_t data)
{
	return false;
}

void FLOPPY::debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	buffer[0] = _T('\0');
	if (IOPORT_USE_5FDD) {
		// 5inch or 8inch
		if (config.fdd_type == FDD_TYPE_58FDD) {
			UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%s):%02X"), 0, _T("FDD8 2D UNITSEL"), drvsel[0]);
		} else {
			UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%s):%02X"), 0, _T("FDD5 2D UNITSEL"), drvsel[0]);
		}
	} else if (IOPORT_USE_3FDD) {
		// 3inch
		UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%s):%02X"), 0, _T("FDD3 UNITSEL"), drvsel[0]);
	} else {
		// no fdd
		UTILITY::tcscat(buffer, buffer_len, _T(" FDD is not available."));
	}
}

#endif

