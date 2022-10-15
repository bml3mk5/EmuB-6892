/** @file mb8866.cpp

	HITACHI BASIC MASTER LEVEL3 Mark5 / MB-S1 Emulator 'EmuB-6892/EmuB-S1'
	Skelton for retropc emulator

	@par Origin
	mb8877.cpp
	@author Sasaji
	@date   2012.03.24 -

	@brief [ MB8866 modoki (same as MB8877/MB8876) ]
*/

#include "mb8866.h"
#include "floppy_defs.h"
#include "../config.h"
#include "../fileio.h"
#include "../utility.h"

//#define SET_CURRENT_TRACK_IMMEDIATELY 1

#ifdef _DEBUG_MB8866
#include "../../logging.h"

#define OUT_DEBUG logging->out_debugf
#ifdef _DEBUG_MB8866_L2
#define OUT_DEBUG2 logging->out_debugf
#else
#define OUT_DEBUG2 dummyf
#endif
#else
#define OUT_DEBUG  dummyf
#define OUT_DEBUG2 dummyf
#endif

#define DRIVE_MASK		(MAX_DRIVE - 1)

/// seek stepping rate 6msec, 12msec, 20msec, 30msec
static const int seek_wait[2][4] = {
	{6000, 12000, 20000, 30000},	// 1MHz
	{3000,  6000, 10000, 15000},	// 2MHz
};

void MB8866::cancel_my_event(int event)
{
	if(register_id[event] != -1) {
		cancel_event(this, register_id[event]);
		OUT_DEBUG2(_T("FDC\tCancel EVENT:%d id:%d"), event, register_id[event]);
		register_id[event] = -1;
	}
}

void MB8866::register_my_event(int event, double usec)
{
	cancel_my_event(event);
	register_event(this, (event << 8) | cmdtype, usec, false, &register_id[event]);
	OUT_DEBUG2(_T("FDC\tRegist EVENT:%d id:%d w:%.2f"), event, register_id[event], usec);
}

void MB8866::register_seek_event()
{
	register_my_event(EVENT_SEEK, FLG_DELAY_FDSEEK ? 600 : seek_wait[clk_num][cmdreg & 3]);
	register_my_event(EVENT_SEEKEND, 300);
	now_seek = true;
}

void MB8866::register_search_event(int wait)
{
	cancel_my_event(EVENT_SEARCH);
	register_event_by_clock(this, (EVENT_SEARCH << 8) | cmdtype, wait, false, &register_id[EVENT_SEARCH]);
	OUT_DEBUG2(_T("FDC\tRegist EVENT:%d id:%d w:%d"), EVENT_SEARCH, register_id[EVENT_SEARCH], wait);
	now_search = true;
}

void MB8866::register_drq_event(int bytes)
{
	int usec = ((64 >> clk_num) >> density) - 8;	// 8us * 8bit (1MHz, FM) (delay 8)

	cancel_my_event(EVENT_DRQ);
	register_event(this, (EVENT_DRQ << 8) | cmdtype, (double)usec, false, &register_id[EVENT_DRQ]);
}

void MB8866::register_lost_event(int bytes)
{
	int usec = ((64 >> clk_num) >> density);	// 8us * 8bit (1MHz, FM)
	usec *= (bytes + 1);

	cancel_my_event(EVENT_LOST);
	register_event(this, (EVENT_LOST << 8) | cmdtype, (double)usec, false, &register_id[EVENT_LOST]);
}

void MB8866::register_restore_event()
{
	if (register_id[EVENT_RESTORE] == -1) {
		irq_mask = true;
		register_event_by_clock(this, (EVENT_RESTORE << 8) | cmdtype, 32, false, &register_id[EVENT_RESTORE]);
	}
}

void MB8866::initialize()
{
	// config
	ignore_crc = config.ignore_crc;

	// initialize fdc
	seektrk = 0;
	seekvct = true;
	status = cmdreg = trkreg = secreg = datareg = cmdtype = 0;

	density = 0;
	now_irq = false;
	now_drq = false;
	irq_mask = false;
}

void MB8866::release()
{
}

/// power on reset
void MB8866::reset()
{
	for(int i = 0; i < MB8866_REGISTER_IDS; i++) {
		register_id[i] = -1;
	}
	now_search = now_seek = after_seek = false;

	seektrk = 0;
	seekvct = true;
	status = cmdreg = trkreg = secreg = datareg = cmdtype = 0;

	density = 0;
	now_irq = false;
	now_drq = false;
	irq_mask = false;

#ifdef _DEBUG_MB8866
	val0_prev = 0xffff;
	drq_prev = false;
#endif
}

/// master reset
void MB8866::warm_reset()
{
	for(int i = 0; i < MB8866_REGISTER_IDS; i++) {
		cancel_my_event(register_id[i]);
	}
	now_search = now_seek = after_seek = false;
	cmdreg = 3;
	secreg = 1;
	now_irq = false;
	now_drq = false;
	irq_mask = false;

#ifdef _DEBUG_MB8866
	val0_prev = 0xffff;
	drq_prev = false;
#endif

	register_restore_event();
}

void MB8866::cancel_my_events()
{
	for(int i = 0; i < MB8866_REGISTER_IDS; i++) {
		cancel_my_event(i);
	}
}

void MB8866::update_config()
{
	ignore_crc = config.ignore_crc;
}

void MB8866::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 3) {
	case 0:
		// command reg
#ifdef HAS_MB8876
		cmdreg = (~data) & 0xff;
#else
		cmdreg = data;
#endif
		process_cmd();
		break;
	case 1:
		// track reg
#ifdef HAS_MB8876
		trkreg = (~data) & 0xff;
#else
		trkreg = data;
#endif
		OUT_DEBUG(_T("FDCw\tTRACKREG=%d"), trkreg);
		if((status & FDC_ST_BUSY) && data_idx == 0) {
			// track reg is written after command starts
			if(cmdtype == FDC_CMD_RD_SEC || cmdtype == FDC_CMD_RD_MSEC || cmdtype == FDC_CMD_WR_SEC || cmdtype == FDC_CMD_WR_MSEC) {
				process_cmd();
			}
		}
		break;
	case 2:
		// sector reg
#ifdef HAS_MB8876
		secreg = (~data) & 0xff;
#else
		secreg = data;
#endif
		OUT_DEBUG(_T("FDCw\tSECREG=%d"), secreg);
		if((status & FDC_ST_BUSY) && data_idx == 0) {
			// sector reg is written after command starts
			if(cmdtype == FDC_CMD_RD_SEC || cmdtype == FDC_CMD_RD_MSEC || cmdtype == FDC_CMD_WR_SEC || cmdtype == FDC_CMD_WR_MSEC) {
				process_cmd();
			}
		}
		break;
	case 3:
		// data reg
#ifdef HAS_MB8876
		datareg = (~data) & 0xff;
#else
		datareg = data;
#endif
#ifdef _DEBUG_MB8866
		if(!(status & FDC_ST_DRQ)) {
			OUT_DEBUG(_T("FDCw\tDATAREG=%d"), datareg);
		}
#endif
		if((status & FDC_ST_DRQ) && !now_search) {
			if(cmdtype == FDC_CMD_WR_SEC || cmdtype == FDC_CMD_WR_MSEC) {
				// write or multisector write
				if(d_fdd->read_signal(SIG_FLOPPY_WRITEPROTECT | channel)) {
					// write protect
//					status |= FDC_ST_WRITEFAULT;
					status |= FDC_ST_WRITEP;
					status &= ~FDC_ST_BUSY;
					status &= ~FDC_ST_DRQ;
					cancel_my_event(EVENT_LOST);
//					cmdtype = 0;
					set_irq(true);
				} else {
					d_fdd->write_signal(SIG_FLOPPY_WRITE | channel, datareg, 0xff);
					// set deleted mark
					if (cmdreg & 1) {
						d_fdd->write_signal(SIG_FLOPPY_WRITEDELETE | channel, 1, 1);
					}
				}

				data_idx++;

				if(data_idx >= (int)d_fdd->read_signal(SIG_FLOPPY_SECTOR_SIZE | channel)) {
					if(cmdtype == FDC_CMD_WR_SEC) {
						// single sector
						OUT_DEBUG(_T("FDC\tEND OF SECTOR (%d bytes wrote)"), data_idx);
						status &= ~FDC_ST_BUSY;
						cancel_my_event(EVENT_LOST);
//						cmdtype = 0;
						set_irq(true);
					} else {
						// multisector
						OUT_DEBUG(_T("FDC\tEND OF SECTOR (SEARCH NEXT)"));
						cancel_my_event(EVENT_LOST);
						register_my_event(EVENT_MULTI1, 30);
						register_my_event(EVENT_MULTI2, 60);
					}
				} else {
					// next data
					register_drq_event(1);
				}
				status &= ~FDC_ST_DRQ;
#ifdef USE_SIG_FLOPPY_ACCESS
				d_fdd->write_signal(SIG_FLOPPY_ACCESS | channel, 1, 1);
#endif
			}
			else if(cmdtype == FDC_CMD_WR_TRK) {
				// write track
				if(d_fdd->read_signal(SIG_FLOPPY_WRITEPROTECT | channel)) {
					// write protect
//					status |= FDC_ST_WRITEFAULT;
					status |= FDC_ST_WRITEP;
					status &= ~FDC_ST_BUSY;
					status &= ~FDC_ST_DRQ;
					cancel_my_event(EVENT_LOST);
//					cmdtype = 0;
					set_irq(true);

					parse_track();
				} else {
					if (density) {
						if (datareg == 0xf5) {
							// address/data mark (missing clock)
							datareg = 0xa1;
						} else if (datareg == 0xf6) {
							// index mark (missing clock)
							datareg = 0xc2;
						} else if (datareg == 0xf7) {
							// TODO: crc
//							int crc = 0; // calc_crc();
						}
					}
					d_fdd->write_signal(SIG_FLOPPY_WRITE_TRACK | channel, datareg, 0xff);
				}

				data_idx++;

				if(data_idx >= (int)d_fdd->read_signal(SIG_FLOPPY_TRACK_SIZE | channel)) {
					// last data
					OUT_DEBUG(_T("FDC\tEND OF TRACK (%d bytes wrote)"), data_idx);
					status &= ~FDC_ST_BUSY;
					cancel_my_event(EVENT_LOST);
//					cmdtype = 0;
					set_irq(true);

					parse_track();
				} else {
					// next data
					register_drq_event(1);
				}
				status &= ~FDC_ST_DRQ;
#ifdef USE_SIG_FLOPPY_ACCESS
				d_fdd->write_signal(SIG_FLOPPY_ACCESS | channel, 1, 1);
#endif
			}
		}
		if(!(status & FDC_ST_DRQ)) {
			set_drq(false);
		}
		break;
	}
}

uint32_t MB8866::read_io8(uint32_t addr)
{
	uint32_t val;

	switch(addr & 3) {
	case 0:
		// status reg
		if(cmdtype == FDC_CMD_TYPE4) {
			// now force interrupt
			if(d_fdd->read_signal(SIG_FLOPPY_READY | channel) == 0) {
				status = FDC_ST_NOTREADY;
			} else {
				// MZ-2500 RELICS invites STATUS = 0
				status = 0;
			}
			val = status;
		}
		else if(now_search) {
			// now sector search
			val = FDC_ST_BUSY;
		} else {
			// disk not inserted, motor stop
			if(d_fdd->read_signal(SIG_FLOPPY_READY | channel) == 0) {
				status |= FDC_ST_NOTREADY;
			} else {
				status &= ~FDC_ST_NOTREADY;
			}
			// write protected
			if(cmdtype == FDC_CMD_TYPE1 || cmdtype == FDC_CMD_WR_SEC || cmdtype == FDC_CMD_WR_MSEC || cmdtype == FDC_CMD_WR_TRK) {
				if(d_fdd->read_signal(SIG_FLOPPY_WRITEPROTECT | channel)) {
					status |= FDC_ST_WRITEP;
				} else {
					status &= ~FDC_ST_WRITEP;
				}
			} else {
				status &= ~FDC_ST_WRITEP;
			}
			// track0, index hole
			if(cmdtype == FDC_CMD_TYPE1) {
				if(d_fdd->read_signal(SIG_FLOPPY_TRACK0 | channel)) {
					status |= FDC_ST_TRACK00;
				} else {
					status &= ~FDC_ST_TRACK00;
				}
				if(!(status & FDC_ST_NOTREADY)) {
					if(d_fdd->read_signal(SIG_FLOPPY_INDEX | channel)) {
						status |= FDC_ST_INDEX;
					} else {
						status &= ~FDC_ST_INDEX;
					}
				}
			}
			// show busy a moment
			val = status;
			if(cmdtype == FDC_CMD_TYPE1 && !now_seek) {
				status &= ~FDC_ST_BUSY;
			}
		}

#ifdef _DEBUG_MB8866
//		// request cpu to output debug log
//		if(d_cpu) {
//			d_cpu->write_signal(SIG_CPU_DEBUG | channel, 1, 1);
//		}
		val |= (now_irq ? 0x100 : 0);
		if (val != val0_prev) {
			OUT_DEBUG(_T("FDC\tSTATUS=%04x"), val);
			val0_prev = val;
		}
#endif

		// reset irq/drq
		if(!(status & FDC_ST_DRQ)) {
			set_drq(false);
		}
		set_irq(false);

#ifdef HAS_MB8876
		return (~val) & 0xff;
#else
		return val & 0xff;
#endif
	case 1:
		// track reg
		OUT_DEBUG(_T("FDC\tTRACKREG=%d"), trkreg);
#ifdef HAS_MB8876
		return (~trkreg) & 0xff;
#else
		return trkreg;
#endif
	case 2:
		// sector reg
		OUT_DEBUG(_T("FDC\tSECTORREG=%d"), secreg);
#ifdef HAS_MB8876
		return (~secreg) & 0xff;
#else
		return secreg;
#endif
	case 3:
		// data reg
		if((status & FDC_ST_DRQ) && !now_search) {
			if(cmdtype == FDC_CMD_RD_SEC || cmdtype == FDC_CMD_RD_MSEC) {
				// read or multisector read
				datareg = d_fdd->read_signal(SIG_FLOPPY_READ | channel);

				data_idx++;

				if(data_idx >= (int)d_fdd->read_signal(SIG_FLOPPY_SECTOR_SIZE | channel)) {
					// last data
					if(cmdtype == FDC_CMD_RD_SEC) {
						// single sector
						OUT_DEBUG(_T("FDC\tEND OF SECTOR (%d bytes read)"), data_idx);
						status &= ~FDC_ST_BUSY;
						cancel_my_event(EVENT_LOST);
//						cmdtype = 0;
						set_irq(true);
					} else {
						// multisector
						OUT_DEBUG(_T("FDC\tEND OF SECTOR (SEARCH NEXT)"));
						cancel_my_event(EVENT_LOST);
						register_my_event(EVENT_MULTI1, 30);
						register_my_event(EVENT_MULTI2, 60);
					}
				} else {
					// next data
					register_drq_event(1);
				}
				status &= ~FDC_ST_DRQ;
#ifdef USE_SIG_FLOPPY_ACCESS
				d_fdd->write_signal(SIG_FLOPPY_ACCESS | channel, 1, 1);
#endif
			} else if(cmdtype == FDC_CMD_RD_ADDR) {
				// read address
				datareg = d_fdd->read_signal(SIG_FLOPPY_READ_ID | channel);

				data_idx++;

				if(data_idx >= 6) {
					// last data
					status &= ~FDC_ST_BUSY;
					cancel_my_event(EVENT_LOST);
//					cmdtype = 0;
					set_irq(true);
				} else {
					// next data
					register_drq_event(1);
				}
				status &= ~FDC_ST_DRQ;
#ifdef USE_SIG_FLOPPY_ACCESS
				d_fdd->write_signal(SIG_FLOPPY_ACCESS | channel, 1, 1);
#endif
			} else if(cmdtype == FDC_CMD_RD_TRK) {
				// read track
				datareg = d_fdd->read_signal(SIG_FLOPPY_READ_TRACK | channel);

				data_idx++;

				if(data_idx >= (int)d_fdd->read_signal(SIG_FLOPPY_TRACK_SIZE | channel)) {
					// last data
					OUT_DEBUG(_T("FDC\tEND OF TRACK (%d bytes read)"), data_idx);
					status &= ~FDC_ST_BUSY;
					cancel_my_event(EVENT_LOST);
//					cmdtype = 0;
					set_irq(true);
				} else {
					// next data
					register_drq_event(1);
				}
				status &= ~FDC_ST_DRQ;
#ifdef USE_SIG_FLOPPY_ACCESS
				d_fdd->write_signal(SIG_FLOPPY_ACCESS | channel, 1, 1);
#endif
			}
		}
		if(!(status & FDC_ST_DRQ)) {
			set_drq(false);
		}
//		OUT_DEBUG(_T("FDC\tDATA=%2x"), datareg);
#ifdef HAS_MB8876
		return (~datareg) & 0xff;
#else
		return datareg;
#endif
	}
	return 0xff;
}

void MB8866::write_dma_io8(uint32_t addr, uint32_t data)
{
	write_io8(3, data);
}

uint32_t MB8866::read_dma_io8(uint32_t addr)
{
	return read_io8(3);
}

void MB8866::write_signal(int id, uint32_t data, uint32_t mask)
{
	if (id == SIG_FLOPPY_DENSITY) {
		density = (data & mask) ? 1 : 0;
	} else if (id == SIG_MB8866_CLOCKNUM) {
		set_context_clock_num((int)data);
	} else if (id == SIG_CPU_RESET) {
		now_reset = (data & mask) ? true : false;
		cancel_my_events();
		if (!now_reset) warm_reset();
	}
}

uint32_t MB8866::read_signal(int ch)
{
	// get access status
	uint32_t stat = 0;
#ifdef USE_SIG_FLOPPY_ACCESS
	d_fdd->write_signal(SIG_FLOPPY_ACCESS | channel, 0, 1);
#endif

	return stat;
}

void MB8866::event_callback(int event_id, int err)
{
	int event = event_id >> 8;
	int cmd = event_id & 0xff;
	register_id[event] = -1;

	// cancel event if the command is finished or other command is executed
	if(cmd != cmdtype) {
		if(event == EVENT_SEEK) {
			now_seek = false;
		}
		else if(event == EVENT_SEARCH) {
			now_search = false;
		}
		return;
	}

	OUT_DEBUG2(_T("FDC\tFire EVENT:%d id:%04x"),event,event_id);
	switch(event) {
	case EVENT_SEEK:
		{
			bool trksame = false;
			if ((cmdreg & 0xe0) == 0) {
				// restore or seek
				seekvct = (seektrk > (int)trkreg) ? false : true;
				trksame = (seektrk == (int)trkreg);
			}
			d_fdd->write_signal(SIG_FLOPPY_STEP | channel, (trksame ? 0x80 : (seekvct ? 0xff : 0x7f)), 0xff);
// trkreg can update only when track_00 signal is ON.
//			trkreg = d_fdd->read_signal(SIG_FLOPPY_CURRENTTRACK | channel);
			if ((cmdreg & 0xe0) == 0 || (cmdreg & 0x10) != 0) {
				// restore or seek
				// step/in/out with setting u flag
				if (!seekvct) {
					// priority first
					trkreg++;
				} else if (d_fdd->read_signal(SIG_FLOPPY_TRACK0 | channel) != 0) {
					// priority second
					trkreg = 0;
				} else if (!trksame) {
					trkreg--;
				}
			}
			if(seektrk == (int)trkreg || (cmdreg & 0xe0) != 0) {
//				|| ((cmdreg & 0xf0) == 0 && d_fdd->read_signal(SIG_FLOPPY_TRACK0 | channel) != 0)) {
				// match track or step end
				status |= verify_track();
				now_seek = false;
				set_irq(!irq_mask);
				irq_mask = false;
			} else {
				// next step
				register_my_event(EVENT_SEEK, seek_wait[clk_num][cmdreg & 3] + err);
			}
		}
		break;
	case EVENT_SEEKEND:
//		if((uint32_t)seektrk == d_fdd->read_signal(SIG_FLOPPY_CURRENTTRACK | channel)) {
		if((cmdreg & 0xe0) == 0 && seektrk == (int)trkreg) {
			// auto update
// trkreg can update only when track_00 signal is ON.
//			if((cmdreg & 0x10) || ((cmdreg & 0xf0) == 0)) {
//				trkreg = d_fdd->read_signal(SIG_FLOPPY_CURRENTTRACK | channel);
//			}
			if((cmdreg & 0xf0) == 0) {
				datareg = 0;
			}
			status |= verify_track();
			now_seek = false;
			cancel_my_event(EVENT_SEEK);
			set_irq(!irq_mask);
			irq_mask = false;
		}
		break;
	case EVENT_SEARCH:
		now_search = false;
#ifndef SEARCH_SECTOR_IMMEDIATELY
		// search sector
		if (cmdtype == FDC_CMD_RD_SEC || cmdtype == FDC_CMD_RD_MSEC
		 || cmdtype == FDC_CMD_WR_SEC || cmdtype == FDC_CMD_WR_MSEC) {
			if(cmdreg & 2) {
				status |= search_sector(((cmdreg & 8) ? 1 : 0), true);
			} else {
				status |= search_sector(0, false);
			}
		}
#endif
#ifndef SEARCH_ADDRESS_IMMEDIATELY
		// search address
		if (cmdtype == FDC_CMD_RD_ADDR) {
			status |= search_addr();
		}
#endif
#ifndef MAKE_TRACK_IMMEDIATELY
		// make track
		if (cmdtype == FDC_CMD_RD_TRK) {
			make_track();
		}
#endif
		// start dma
		if(!FLG_ORIG_FDDRQ) {
			if (!(status & (FDC_ST_RECNFND | FDC_ST_CRCERR))) {
				status |= FDC_ST_DRQ;
				register_lost_event(3);
				set_drq(true);
			} else {
				// error end
				set_irq(true);
			}
		}
		break;
	case EVENT_DRQ:
		if(status & FDC_ST_BUSY) {
			status |= FDC_ST_DRQ;
			register_lost_event(1);
			set_drq(true);
//			OUT_DEBUG(_T("FDC\tDRQ ON"));
		}
		break;
	case EVENT_TYPE4:
		cmdtype = FDC_CMD_TYPE4;
		break;
	case EVENT_MULTI1:
		secreg++;
		break;
	case EVENT_MULTI2:
		if(cmdtype == FDC_CMD_RD_MSEC) {
			cmd_readdata();
		} else if(cmdtype == FDC_CMD_WR_MSEC) {
			cmd_writedata();
		}
		break;
	case EVENT_LOST:
		if(status & FDC_ST_BUSY) {
			OUT_DEBUG(_T("FDC\tLOST DATA (data_idx:%d)"), data_idx);
			status |= FDC_ST_LOSTDATA;
			status &= ~FDC_ST_BUSY;
			//status &= ~FDC_ST_DRQ;
			set_irq(true);
			//set_drq(false);
		}
		break;
	case EVENT_RESTORE:
		cmd_restore();
		break;
	}
}

// ----------------------------------------------------------------------------
// command
// ----------------------------------------------------------------------------

void MB8866::process_cmd()
{
#ifdef _DEBUG_MB8866
	static const _TCHAR *cmdstr[0x10] = {
		_T("RESTORE "),	_T("SEEK    "),	_T("STEP    "),	_T("STEP    "),
		_T("STEP IN "),	_T("STEP IN "),	_T("STEP OUT"),	_T("STEP OUT"),
		_T("RD DATA "),	_T("RD DATA "),	_T("WR DATA "),	_T("WR DATA "),
		_T("RD ADDR "),	_T("FORCEINT"),	_T("RD TRACK"),	_T("WR TRACK")
	};
	OUT_DEBUG(_T("FDC\tCMD=%2xh (%s) DATA=%2xh TRK=%3d SEC=%2d"), cmdreg, cmdstr[cmdreg >> 4], datareg, trkreg, secreg);
#endif

	cancel_my_event(EVENT_TYPE4);
	set_irq(false);

	switch(cmdreg & 0xf0) {
	// type-1
	case 0x00:
		cmd_restore();
		break;
	case 0x10:
		cmd_seek();
		break;
	case 0x20:
	case 0x30:
		cmd_step();
		break;
	case 0x40:
	case 0x50:
		cmd_stepin();
		break;
	case 0x60:
	case 0x70:
		cmd_stepout();
		break;
	// type-2
	case 0x80:
	case 0x90:
		cmd_readdata();
		break;
	case 0xa0:
	case 0xb0:
		cmd_writedata();
		break;
	// type-3
	case 0xc0:
		cmd_readaddr();
		break;
	case 0xe0:
		cmd_readtrack();
		break;
	case 0xf0:
		cmd_writetrack();
		break;
	// type-4
	case 0xd0:
		cmd_forceint();
		break;
	default:
		break;
	}
}

void MB8866::cmd_restore()
{
	// type-1 restore
	cmdtype = FDC_CMD_TYPE1;
	status = FDC_ST_BUSY;
	if (cmdreg & 8) {
		status |= FDC_ST_HEADENG;
	}
	d_fdd->write_signal(SIG_FLOPPY_HEADLOAD | channel, (cmdreg & 8) ? 1 : 0, 1);
	trkreg = 0xff;

	seektrk = 0;
	seekvct = true;

	register_seek_event();
}

void MB8866::cmd_seek()
{
	// type-1 seek
	cmdtype = FDC_CMD_TYPE1;
	status = FDC_ST_BUSY;
	if (cmdreg & 8) {
		status |= FDC_ST_HEADENG;
	}
	d_fdd->write_signal(SIG_FLOPPY_HEADLOAD | channel, (cmdreg & 8) ? 1 : 0, 1);
	after_seek = ((cmdreg & 8) ? false : true);

#if 0
	seektrk = fdc[drvreg].track + datareg - trkreg;
#else
	seektrk = datareg;
#endif
//	seektrk = (seektrk > 83) ? 83 : (seektrk < 0) ? 0 : seektrk;
	seekvct = (datareg > trkreg) ? false : true;

	register_seek_event();
}

void MB8866::cmd_step()
{
	// type-1 step
	if(seekvct) {
		cmd_stepout();
	} else {
		cmd_stepin();
	}
}

void MB8866::cmd_stepin()
{
	// type-1 step in
	cmdtype = FDC_CMD_TYPE1;
	status = FDC_ST_BUSY;
	if (cmdreg & 8) {
		status |= FDC_ST_HEADENG;
	}
	d_fdd->write_signal(SIG_FLOPPY_HEADLOAD | channel, (cmdreg & 8) ? 1 : 0, 1);
	after_seek = ((cmdreg & 8) ? false : true);

//	seektrk = d_fdd->read_signal(SIG_FLOPPY_CURRENTTRACK | channel);
//	seektrk = (trkreg < 255) ? trkreg + 1 : 255;
	seekvct = false;

	register_seek_event();
}

void MB8866::cmd_stepout()
{
	// type-1 step out
	cmdtype = FDC_CMD_TYPE1;
	status = FDC_ST_BUSY;
	if (cmdreg & 8) {
		status |= FDC_ST_HEADENG;
	}
	d_fdd->write_signal(SIG_FLOPPY_HEADLOAD | channel, (cmdreg & 8) ? 1 : 0, 1);
	after_seek = ((cmdreg & 8) ? false : true);

//	seektrk = d_fdd->read_signal(SIG_FLOPPY_CURRENTTRACK | channel);
//	seektrk = (trkreg > 0) ? trkreg - 1 : 0;
	seekvct = true;

	register_seek_event();
}

void MB8866::cmd_readdata()
{
	// type-2 read data
#ifdef SET_CURRENT_TRACK_IMMEDIATELY
	d_fdd->write_signal(SIG_FLOPPY_CURRENTTRACK | channel, trkreg, 0xff);
#endif

	cmdtype = (cmdreg & 0x10) ? FDC_CMD_RD_MSEC : FDC_CMD_RD_SEC;
#ifdef SEARCH_SECTOR_IMMEDIATELY
	if(cmdreg & 2) {
		status = search_sector(((cmdreg & 8) ? 1 : 0), true);
	} else {
		status = search_sector(0, false);
	}
	if(!(status & FDC_ST_RECNFND)) {
		status |= FDC_ST_BUSY;
	}
#else
	status = FDC_ST_BUSY;
#endif

	int time = d_fdd->calc_sector_search_clock(channel, secreg);
	register_search_event(time);
	cancel_my_event(EVENT_LOST);

	d_fdd->write_signal(SIG_FLOPPY_HEADLOAD | channel, 1, 1);
}

void MB8866::cmd_writedata()
{
	// type-2 write data
#ifdef SET_CURRENT_TRACK_IMMEDIATELY
	d_fdd->write_signal(SIG_FLOPPY_CURRENTTRACK | channel, trkreg, 0xff);
#endif

	cmdtype = (cmdreg & 0x10) ? FDC_CMD_WR_MSEC : FDC_CMD_WR_SEC;
#ifdef SEARCH_SECTOR_IMMEDIATELY
	if(cmdreg & 2) {
		status = search_sector(((cmdreg & 8) ? 1 : 0), true);
	}
	else {
		status = search_sector(0, false);
	}
	status &= ~FDC_ST_RECTYPE;
	if(!(status & FDC_ST_RECNFND)) {
		status |= FDC_ST_BUSY;
	}
#else
	status = FDC_ST_BUSY;
#endif

	int time = d_fdd->calc_sector_search_clock(channel, secreg);
	register_search_event(time);
	cancel_my_event(EVENT_LOST);

	d_fdd->write_signal(SIG_FLOPPY_HEADLOAD | channel, 1, 1);
}

void MB8866::cmd_readaddr()
{
	// type-3 read address
#ifdef SET_CURRENT_TRACK_IMMEDIATELY
	d_fdd->write_signal(SIG_FLOPPY_CURRENTTRACK | channel, trkreg, 0xff);
#endif

	cmdtype = FDC_CMD_RD_ADDR;
#ifdef SEARCH_ADDRESS_IMMEDIATELY
	status = search_addr();
	if(!(status & FDC_ST_RECNFND)) {
		status |= FDC_ST_BUSY;
	}
#else
	status = FDC_ST_BUSY;
#endif

	int time = d_fdd->calc_next_sector_clock(channel);
	register_search_event(time);
	cancel_my_event(EVENT_LOST);

	d_fdd->write_signal(SIG_FLOPPY_HEADLOAD | channel, 1, 1);
}

void MB8866::cmd_readtrack()
{
	// type-3 read track
#ifdef SET_CURRENT_TRACK_IMMEDIATELY
	d_fdd->write_signal(SIG_FLOPPY_CURRENTTRACK | channel, trkreg, 0xff);
#endif

	cmdtype = FDC_CMD_RD_TRK;
	status = FDC_ST_BUSY;

	d_fdd->write_signal(SIG_FLOPPY_TRACK_SIZE | channel, 1, 1);
	data_idx = 0;
#ifdef MAKE_TRACK_IMMEDIATELY
	make_track();
#endif

	int time = d_fdd->calc_index_hole_search_clock(channel);
	register_search_event(time);
	cancel_my_event(EVENT_LOST);

	d_fdd->write_signal(SIG_FLOPPY_HEADLOAD | channel, 1, 1);
}

void MB8866::cmd_writetrack()
{
	// type-3 write track
#ifdef SET_CURRENT_TRACK_IMMEDIATELY
	d_fdd->write_signal(SIG_FLOPPY_CURRENTTRACK | channel, trkreg, 0xff);
#endif

	cmdtype = FDC_CMD_WR_TRK;
	status = FDC_ST_BUSY;

	d_fdd->write_signal(SIG_FLOPPY_TRACK_SIZE | channel, 1, 1);
	data_idx = 0;

	int time = d_fdd->calc_index_hole_search_clock(channel);
	register_search_event(time);
	cancel_my_event(EVENT_LOST);

	d_fdd->write_signal(SIG_FLOPPY_HEADLOAD | channel, 1, 1);
}

void MB8866::cmd_forceint()
{
	// type-4 force interrupt
#if 0
	if(!disk[drvreg]->inserted || !motor) {
		status = FDC_ST_NOTREADY | FDC_ST_HEADENG;
	} else {
		status = FDC_ST_HEADENG;
	}
	cmdtype = FDC_CMD_TYPE4;
#else
	if(cmdtype == 0 || cmdtype == 4) {
		status = 0;
		cmdtype = FDC_CMD_TYPE1;
	}
	status &= ~FDC_ST_BUSY;
#endif

	// force interrupt if bit0-bit3 is high
	if(cmdreg & 0x0f) {
		set_irq(true);
	}
	now_seek = false;
	cancel_my_event(EVENT_SEEK);
	cancel_my_event(EVENT_SEEKEND);
	now_search = false;
	cancel_my_event(EVENT_SEARCH);
	cancel_my_event(EVENT_TYPE4);
	cancel_my_event(EVENT_MULTI1);
	cancel_my_event(EVENT_MULTI2);
	cancel_my_event(EVENT_LOST);
	cancel_my_event(EVENT_DRQ);
	register_my_event(EVENT_TYPE4, 100);
}

// ----------------------------------------------------------------------------
// media handler
// ----------------------------------------------------------------------------

uint8_t MB8866::verify_track()
{
	if(!d_fdd->search_track(channel)) {
		return FDC_ST_SEEKERR;
	}

	// verify track number
	if(!(cmdreg & 4)) {
		return 0;
	}

	d_fdd->write_signal(SIG_FLOPPY_HEADLOAD | channel, 1, 1);

	if(!d_fdd->verify_track(channel, trkreg)) {
		return FDC_ST_SEEKERR;
	}
	return 0;
}

uint8_t MB8866::search_sector(int side, bool compare)
{
	// get track
	if(!d_fdd->search_track(channel)) {
//		if (FLG_ORIG_FDDRQ) set_irq(true);
		return FDC_ST_RECNFND;
	}

	// scan sectors
	int stat = d_fdd->search_sector(channel, trkreg, secreg, compare, side);
	if (stat != 1) {
		data_idx = 0;
		return ((stat & 4) ? FDC_ST_RECTYPE : 0) | ((stat & 2) ? FDC_ST_CRCERR : 0);
	}
	// sector not found
//	if (FLG_ORIG_FDDRQ) set_irq(true);
	return FDC_ST_RECNFND;
}

uint8_t MB8866::search_addr()
{
	// get track
	if(!d_fdd->search_track(channel)) {
//		if (FLG_ORIG_FDDRQ) set_irq(true);
		return FDC_ST_RECNFND;
	}

	// get sector
	int stat = d_fdd->search_sector(channel);
	if (stat != 1) {
		data_idx = 0;
		return ((status == 2) ? FDC_ST_CRCERR : 0);
	}

	// sector not found
//	if (FLG_ORIG_FDDRQ) set_irq(true);
	return FDC_ST_RECNFND;
}

bool MB8866::make_track()
{
	return d_fdd->make_track(channel);
}

bool MB8866::parse_track()
{
	return d_fdd->parse_track(channel);
}

// ----------------------------------------------------------------------------
// irq / drq
// ----------------------------------------------------------------------------

void MB8866::set_irq(bool val)
{
	now_irq = val;
	write_signals(&outputs_irq, val ? 0xffffffff : 0);
#ifdef _DEBUG_MB8866_L2
	if (val) {
		OUT_DEBUG(_T("FDC\tset_irq=true status:%02x"), status);
		val0_prev = 0xffff;
	}
#endif
}

void MB8866::set_drq(bool val)
{
	now_drq = val;
	write_signals(&outputs_drq, val ? 0xffffffff : 0);
#ifdef _DEBUG_MB8866_L2
	if (val != drq_prev) {
		OUT_DEBUG(_T("FDC\tset_drq=%s status:%02x"), val ? _T("true") : _T("false"), status);
		drq_prev = val;
	}
#endif
}

// ----------------------------------------------------------------------------
void MB8866::save_state(FILEIO *fp)
{
	struct vm_state_st vm_state;

	//
	vm_state_ident.version = Uint16_LE(3);
	vm_state_ident.size = Uint32_LE(sizeof(vm_state_ident) + sizeof(vm_state));

	// copy values
	memset(&vm_state, 0, sizeof(vm_state));
	for(int i=0; i<7; i++) {
		vm_state.register_id[i] = Int32_LE(register_id[i]);
	}
	vm_state.status = status;
	vm_state.cmdreg = cmdreg;
	vm_state.trkreg = trkreg;
	vm_state.secreg = secreg;
	vm_state.datareg = datareg;
	vm_state.cmdtype = cmdtype;
	vm_state.seektrk = Int32_LE(seektrk);
	vm_state.data_idx = Int32_LE(data_idx);
	vm_state.flags = (seekvct ? 8 : 0) | (now_search ? 4 : 0) | (after_seek ? 2 : 0) | (now_seek ? 1 : 0);
	vm_state.flags2 = density;

	vm_state.register_id2[0] = Int32_LE(register_id[7]);
	vm_state.register_id3[0] = Int32_LE(register_id[8]);

	fp->Fwrite(&vm_state_ident, sizeof(vm_state_ident), 1);
	fp->Fwrite(&vm_state, sizeof(vm_state), 1);
}

bool MB8866::load_state(FILEIO *fp)
{
	vm_state_ident_t vm_state_i;
	struct vm_state_st vm_state;

	READ_STATE_CHUNK(fp, vm_state_i, vm_state);

	status = vm_state.status;
	cmdreg = vm_state.cmdreg;
	trkreg = vm_state.trkreg;
	secreg = vm_state.secreg;
	datareg = vm_state.datareg;
	cmdtype = vm_state.cmdtype;
	seektrk = Int32_LE(vm_state.seektrk);
	data_idx = Int32_LE(vm_state.data_idx);
	seekvct = (vm_state.flags & 8) ? true : false;
	now_search = (vm_state.flags & 4) ? true : false;
	after_seek = (vm_state.flags & 2) ? true : false;
	now_seek = (vm_state.flags & 1) ? true : false;

	// copy values
	for(int i=0; i<7; i++) {
		register_id[i] = Int32_LE(vm_state.register_id[i]);
	}
	if (Uint16_LE(vm_state_i.version) >= 2) {
		density = vm_state.flags2;
		register_id[7] = Int32_LE(vm_state.register_id2[0]);
	}
	if (Uint16_LE(vm_state_i.version) >= 3) {
		register_id[8] = Int32_LE(vm_state.register_id3[0]);
	}

	return true;
}

// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
uint32_t MB8866::debug_read_io8(uint32_t addr)
{
	uint32_t val;

	switch(addr & 3) {
	case 0:
		// status reg
		if(cmdtype == FDC_CMD_TYPE4) {
			val = status;
		}
		else if(now_search) {
			// now sector search
			val = FDC_ST_BUSY;
		}
		else {
			// show busy a moment
			val = status;
		}
#ifdef HAS_MB8876
		return (~val) & 0xff;
#else
		return val & 0xff;
#endif
	case 1:
		// track reg
#ifdef HAS_MB8876
		return (~trkreg) & 0xff;
#else
		return trkreg;
#endif
	case 2:
		// sector reg
#ifdef HAS_MB8876
		return (~secreg) & 0xff;
#else
		return secreg;
#endif
	case 3:
		// data reg
#ifdef HAS_MB8876
		return (~datareg) & 0xff;
#else
		return datareg;
#endif
	}
	return 0xff;
}

static const _TCHAR *c_reg_names[] = {
	_T("STR"),
	_T("CR"),
	_T("TR"),
	_T("SCR"),
	_T("DR"),
	NULL
};

bool MB8866::debug_write_reg(uint32_t reg_num, uint32_t data)
{
	switch(reg_num) {
	case 0:
		status = data & 0xff;
		return true;
	case 1:
	case 2:
	case 3:
#ifdef HAS_MB8876
		data = ~data;
#endif
		write_io8(reg_num - 1, data);
		return true;
	case 4:
#ifdef HAS_MB8876
		data = ~data;
#endif
		datareg = data & 0xff;
		return true;
	}
	return false;
}

bool MB8866::debug_write_reg(const _TCHAR *reg, uint32_t data)
{
	uint32_t num = find_debug_reg_name(c_reg_names, reg);
	return debug_write_reg(num, data);
}

void MB8866::debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	buffer[0] = _T('\0');
	UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%s):%02X"), 0, c_reg_names[0], status);
	UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%s):%02X"), 1, c_reg_names[1], cmdreg);
	UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%s):%02X"), 2, c_reg_names[2], trkreg);
	UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%s):%02X"), 3, c_reg_names[3], secreg);
	UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%s):%02X"), 4, c_reg_names[4], datareg);
	UTILITY::sntprintf(buffer, buffer_len, _T("  %s:%d"), _T("DRQ"), now_drq ? 1 : 0);
	UTILITY::sntprintf(buffer, buffer_len, _T("  %s:%d"), _T("IRQ"), now_irq ? 1 : 0);
}


#endif
