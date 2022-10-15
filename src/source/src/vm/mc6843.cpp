/** @file mc6843.cpp

	HITACHI BASIC MASTER LEVEL3 Mark5 / MB-S1 Emulator 'EmuB-6892/EmuB-S1'
	Skelton for retropc emulator

	@par Origin
	mess & mb8877.c
	@author Sasaji
	@date   2011.11.05 -

	@brief [ fdc modoki (mc6843) ]
*/

#include "mc6843.h"
#include "floppy_defs.h"
#include "../config.h"
#include "../fileio.h"
#include "../utility.h"

#ifdef _DEBUG_MC6843
#include "../../logging.h"

#define OUT_DEBUG logging->out_debugf
#else
#define OUT_DEBUG dummyf
#endif

#define DRIVE_MASK	(MAX_DRIVE - 1)

#define SEEK_TIMEOUT	600000	

void MC6843::cancel_my_event(int event_no)
{
	if(register_id[event_no] != -1) {
		cancel_event(this, register_id[event_no]);
		register_id[event_no] = -1;
	}
}

void MC6843::register_my_event(int event_no, int wait)
{
	cancel_my_event(event_no);
	register_event(this, event_no, (double)wait, false, &register_id[event_no]);
}

void MC6843::register_search_event(int wait)
{
	cancel_my_event(EVENT_SEARCH);
	register_event_by_clock(this, EVENT_SEARCH, wait, false, &register_id[EVENT_SEARCH]);
}

void MC6843::register_drq_event(int bytes)
{
	int usec = 64 - 8;

	cancel_my_event(EVENT_DRQ);
	register_event(this, EVENT_DRQ, (double)usec, false, &register_id[EVENT_DRQ]);
}

void MC6843::register_lost_event(int bytes)
{
	int usec = 64;
	usec *= (bytes + 1);

	cancel_my_event(EVENT_LOST);
	register_event(this, EVENT_LOST, (double)usec, false, &register_id[EVENT_LOST]);
}

void MC6843::initialize()
{
//	dor = 0;
	dir = 0;
	ctar = 0;
	cmr = 0;
	isr = 0;
	sur = 0;
	stra = 0;
	strb = 0;
	sar = 0;
	gcr = 0;
	ccr = 0;
	ltar = 0;
}

void MC6843::reset()
{
	for(int i = 0; i < MC6843_MAX_EVENT; i++) {
		register_id[i] = -1;
	}

	stepcnt = 0;
	now_seek = false;
	now_search = false;
	head_load = false;

	//
	memset(parse_clk_buf, 0, MC6843_PARSE_BUFFER);
	memset(parse_dat_buf, 0, MC6843_PARSE_BUFFER);
	parse_dat = 0;
	parse_idx = 0;
	ffw_phase = 0;

	// reset registers
	cmr &= 0xf0;
	isr = 0;
	stra &= 0x5c;
	sar = 0;
	strb &= 0x20;

	status_update();
}

void MC6843::cancel_my_events()
{
	for(int i = 0; i < MC6843_MAX_EVENT; i++) {
		cancel_my_event(i);
	}
}

void MC6843::release()
{
}

void MC6843::write_io8(uint32_t addr, uint32_t data)
{
#ifdef _DEBUG
	static const _TCHAR *cmdname[16]={ _T("?"), _T("?"), _T("STZ"), _T("SEK"), _T("SSR"), _T("SSW"), _T("RCR"), _T("SWD")
									, _T("?"), _T("?"), _T("FFR"), _T("FFW"), _T("MSR"), _T("MSW"), _T("?"), _T("?") };
#endif
	switch(addr & 15) {
	case 0:	// DOR
		OUT_DEBUG(_T("MC6843: write DOR  d:%02x"), data);
		write_data_reg(data);
		break;
	case 1:	// CTAR : current track (7bit)
//		ctar = data & 0x7f;
		OUT_DEBUG(_T("MC6843: write CTAR d:%02x -> ctar:%02x"),data,ctar);
		ctar = data;
		break;
	case 2:	// CMR : command

		stra &= ~(FDC_STA_BUSY | FDC_STA_DRQ);

#ifdef _DEBUG
		OUT_DEBUG(_T("MC6843: write CMR  %s d:%02x ctar:%02x cmr:%02x sur:%02x sar:%02x gcr:%02x ccr:%02x ltar:%02x isr:%02x stra:%02x strb:%02x")
		,cmdname[data & 15]
		,data
		,ctar,cmr,sur,sar,gcr,ccr,ltar,isr,stra,strb);
#endif

		switch(data & 15) {
		case FDC_CMD_FFW_END:	// stop ffw command
			cmd_FFW_END();
			break;
		case FDC_CMD_STZ:	// seek track zero
			cmd_STZ();
			break;
		case FDC_CMD_SEK:	// seek
			cmd_SEK();
			break;
		case FDC_CMD_SSR:	// single sector read
			cmd_SSR();
			break;
		case FDC_CMD_SSW:	// single sector write
			cmd_SSW();
			break;
		case FDC_CMD_RCR:	// read CRC
			cmd_RCR();
			break;
		case FDC_CMD_SWD:	// single sector write with delete data mark
			cmd_SWD();
			break;
		case FDC_CMD_FFR:	// free format read
			cmd_FFR();
			break;
		case FDC_CMD_FFW:	// free format write
			cmd_FFW();
			break;
		case FDC_CMD_MSR:	// multi sector read
			cmd_MSR();
			break;
		case FDC_CMD_MSW:	// multi sector write
			cmd_MSW();
			break;
		}
		cmr = data;
		break;
	case 3:	// SUR
		OUT_DEBUG(_T("MC6843: write SUR  d:%02x sur:%02x"),data,sur);
		sur = data & 0xff;
		break;
	case 4:	// SAR
		OUT_DEBUG(_T("MC6843: write SAR  d:%02x sar:%02x"),data,sar);
		sar = data & 0x1f;
		break;
	case 5:	// GCR
		OUT_DEBUG(_T("MC6843: write GCR  d:%02x gcr:%02x"),data,gcr);
		gcr = data & 0x7f;
		break;
	case 6:	// CCR
		OUT_DEBUG(_T("MC6843: write CCR  d:%02x ccr:%02x"),data,ccr);
		ccr = data & 0x03;
		break;
	case 7:	// LTAR
		OUT_DEBUG(_T("MC6843: write LTAR d:%02x ltar:%02x"),data,ltar);
		ltar = data & 0x7f;
		break;
	}
}

uint32_t MC6843::read_io8(uint32_t addr)
{
	uint32_t data = 0;
#ifdef _DEBUG
	static uint32_t prev_addr = 0xff;
	static uint32_t prev_data = 0xff;
#endif

	switch(addr & 15) {
	case 0:	// DIR
		data = read_data_reg();
//		OUT_DEBUG("fdcr dir a:%04x d:%02x"
//			,addr,data);
		break;
	case 1:	// CTAR : current track (7bit)
		data = ctar;
#ifdef _DEBUG
//		if (prev_addr != addr || prev_data != data) {
//			OUT_DEBUG("fdcr ctar a:%04x d:%02x ctar:%02x"
//				,addr,data,ctar);
//		}
#endif
		break;
	case 2:	// ISR
		data = isr;
		// clear status without strb
		isr &= 0x08;
		OUT_DEBUG(_T("MC6843: read isr a:%04x d:%02x isr:%02x")
			,addr,data,isr);
		break;
	case 3:	// STRA
		update_stra();
		data = stra;
#ifdef _DEBUG
		if (prev_addr != addr || prev_data != data) {
			OUT_DEBUG(_T("MC6843: read stra a:%04x d:%02x stra:%02x")
				,addr,data,stra);
			prev_addr = (addr & 15);
			prev_data = data;
		}
#endif
		break;
	case 4:	// STRB
		data = strb;
		// clear status b
		isr &= ~0x08;
		strb &= ~(FDC_STB_DATAERR | FDC_STB_CRCERR | FDC_STB_SECTNF | FDC_STB_SEEKERR | FDC_STB_HARDERR | FDC_STB_WRITEERR | FDC_STB_FILEINO);
		OUT_DEBUG(_T("MC6843: read strb a:%04x d:%02x strb:%02x")
			,addr,data,strb);

		status_update();
		break;
	}

	return data;
}

void MC6843::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch(id) {
		case SIG_MC6843_UPDATESTATUS:
			update_stra();
			break;
		case SIG_CPU_RESET:
			now_reset = (data & mask) ? true : false;
			cancel_my_events();
			reset();
			break;
	}
}

// ----------------------------------------------------------------------------
uint8_t MC6843::read_data_reg()
{
	int cmd = cmr & 0x0f;

	if ((stra & FDC_STA_DRQ) && cmd == 0) {
		// free format read end
		stra &= ~FDC_STA_BUSY;
		stra &= ~FDC_STA_DRQ;

#ifdef USE_SIG_FLOPPY_ACCESS
		d_fdd->write_signal(SIG_FLOPPY_ACCESS, 1, 1);
#endif

		return dir;
	}
	if((stra & FDC_STA_DRQ) && !now_search) {
		if(cmd == FDC_CMD_SSR || cmd == FDC_CMD_MSR) {

			// read or multisector read
			// read sector 128 bytes max
			dir = d_fdd->read_signal(SIG_FLOPPY_READ);

			data_idx++;
			if(data_idx >= 128) {
				// last data
				if(cmd == FDC_CMD_SSR || gcr == 0) {
					// single sector
					OUT_DEBUG(_T("MC6843: READ : END OF SECTOR"));
					stra &= ~FDC_STA_BUSY;
				} else {
					// multi sector
					OUT_DEBUG(_T("MC6843: READ : END OF SECTOR (SEARCH NEXT)"));
					register_my_event(EVENT_MULTI, set_delay(0xf0));
				}
				cancel_my_event(EVENT_LOST);
			} else {
				// next data
				register_drq_event(1);
			}
			stra &= ~FDC_STA_DRQ;
#ifdef USE_SIG_FLOPPY_ACCESS
			d_fdd->write_signal(SIG_FLOPPY_ACCESS, 1, 1);
#endif
		}
	}
	return dir;
}

// ----------------------------------------------------------------------------
void MC6843::write_data_reg(uint8_t data)
{
//	uint8_t datareg = data;
	int cmd = cmr & 0x0f;

	if (cmd == 0) {
		// free format write end
		stra &= ~FDC_STA_BUSY;
		stra &= ~FDC_STA_DRQ;

#ifdef USE_SIG_FLOPPY_ACCESS
		d_fdd->write_signal(SIG_FLOPPY_ACCESS, 1, 1);
#endif

		return;
	}
	if((stra & FDC_STA_DRQ) && !now_search) {
		if(cmd == FDC_CMD_SSW || cmd == FDC_CMD_MSW || cmd == FDC_CMD_SWD) {

			// write or multisector write
			// write sector 128 bytes max
			if(d_fdd->read_signal(SIG_FLOPPY_WRITEPROTECT)) {
				// write protect
				strb |= FDC_STB_WRITEERR;
				stra &= ~FDC_STA_BUSY;
				stra &= ~FDC_STA_DRQ;
				OUT_DEBUG(_T("MC6843: chg_stat stra:%02x strb:%02x"),stra,strb);
			} else {
				d_fdd->write_signal(SIG_FLOPPY_WRITE, data, 0xff);
				// set deleted mark
				if (cmd == FDC_CMD_SWD) {
					d_fdd->write_signal(SIG_FLOPPY_WRITEDELETE, 1, 1);
				}
			}

			data_idx++;
			if(data_idx >= 128) {
				// last data
				if(cmd == FDC_CMD_SSW || gcr == 0) {
					// single sector
					OUT_DEBUG(_T("MC6843: WRITE : END OF SECTOR"));
					stra &= ~FDC_STA_BUSY;
				} else {
					// multisector
					OUT_DEBUG(_T("MC6843: WRITE : END OF SECTOR (SEARCH NEXT)"));
					register_my_event(EVENT_MULTI, set_delay(0xf0));
				}
				cancel_my_event(EVENT_LOST);
			} else {
				// next data
				register_drq_event(1);
			}
			stra &= ~FDC_STA_DRQ;
#ifdef USE_SIG_FLOPPY_ACCESS
			d_fdd->write_signal(SIG_FLOPPY_ACCESS, 1, 1);
#endif

		} else if (cmd == FDC_CMD_FFW) {

			// free format write
			if (cmr & 0x10) {
				parse_twice_format(data);
			} else {
				parse_plane_format(data);
			}
//			parse_ibm3740_format(data);
//			write_ibm3740_format();

			register_drq_event(1);
			stra &= ~FDC_STA_DRQ;
#ifdef USE_SIG_FLOPPY_ACCESS
			d_fdd->write_signal(SIG_FLOPPY_ACCESS, 1, 1);
#endif

		}
	}
	return;
}

// ----------------------------------------------------------------------------
void MC6843::update_stra()
{
//		int cmd = cmr & 0x0f;

		// status reg
		// disk not inserted, motor stop
		if(d_fdd->read_signal(SIG_FLOPPY_READY)) {
			stra |= FDC_STA_DREADY;
		} else {
			stra &= ~FDC_STA_DREADY;
		}
		// write protect
		if(d_fdd->read_signal(SIG_FLOPPY_WRITEPROTECT)) {
			stra |= FDC_STA_WRITEP;
		} else {
			stra &= ~FDC_STA_WRITEP;
		}

		// track0
		if(d_fdd->read_signal(SIG_FLOPPY_TRACK0)) {
			stra |= FDC_STA_TRACK00;
		}
		else {
			stra &= ~FDC_STA_TRACK00;
		}

		// index hole
		if(d_fdd->read_signal(SIG_FLOPPY_INDEX)) {
			stra |= FDC_STA_INDEX;
		} else {
			stra &= ~FDC_STA_INDEX;
		}

		return;
}

// ----------------------------------------------------------------------------
// irq
// ----------------------------------------------------------------------------
void MC6843::set_irq(bool val)
{
	write_signals(&outputs_irq, val ? 0xffffffff : 0);
	OUT_DEBUG(_T("MC6843: set_irq:%d"),val ? 1 : 0);
}

// ----------------------------------------------------------------------------
// event handler
// ----------------------------------------------------------------------------

void MC6843::event_frame()
{
}

void MC6843::event_callback(int event_id, int err)
{
	int event_no = event_id;

	cancel_my_event(event_no);

	switch(event_no) {
	case EVENT_SEEK:
		event_seek(event_no);
		break;
	case EVENT_SEARCH:
		event_search(event_no);
		break;
	case EVENT_MULTI:
		event_multi(event_no);
		break;
	case EVENT_LOST:
		event_lost(event_no);
		break;
	case EVENT_DRQ:
		event_drq(event_no);
		break;
	}

}

// ----------------------------------------------------------------------------
// called at end of command
void MC6843::cmd_end()
{
	int cmd = cmr & 0x0f;
	if ( cmd == FDC_CMD_STZ || cmd == FDC_CMD_SEK )
	{
		isr |= FDC_STI_SETCOMP;	// set Settling Time Complete
	}
	else
	{
		isr |= FDC_STI_CMDCOMP;	// set Macro Command Complete
	}

	stra &= ~FDC_STA_BUSY; // clear Busy
	cmr  &=  0xf0; // clear command

	status_update();
}

// called after ISR or STRB has changed
void MC6843::status_update()
{
	bool irq = false;

	// ISR bit3
	if ( (cmr & 0x40) || ! strb )	// ISR bit3 mask
		isr &= ~FDC_STI_STRB;
	else
		isr |=  FDC_STI_STRB;

	// interrupts
	if (isr & FDC_STI_STSREQ) irq = true;
	if ((cmr & FDC_CMR_FUNCMASK) == 0) {
		if ( isr & ~FDC_STI_STSREQ ) irq = true;
	}

	// IRQ interrupt
	set_irq(irq);
}

// set delay
int MC6843::set_delay(uint8_t mask)
{
	uint8_t setupreg = sur & mask;
	int delay = FLG_DELAY_FDSEEK ? 0 : ((setupreg & 0xf0) >> 4) * 1024 + (setupreg & 0x0f) * 4096;
	if (delay == 0) delay = 64;

	OUT_DEBUG(_T("MC6843: set_delay:%d"), delay);

	return delay;
}

// ----------------------------------------------------------------------------
/// Seek Track Zero
void MC6843::cmd_STZ()
{
	gcr = 0;
	ctar = 83;
	stepcnt = 83;
	now_seek = true;
	stra |= FDC_STA_BUSY;	// busy

	OUT_DEBUG(_T("MC6843: cmd_STZ HEAD UNLOAD"));
	d_fdd->write_signal(SIG_FLOPPY_HEADLOAD, 0, 1);	// head unload
	head_load = false;

	register_my_event(EVENT_SEEK, set_delay(0xf0));	// seek track
}

/// Seek
void MC6843::cmd_SEK()
{
	stepcnt = 83;
	now_seek = true;
	stra |= FDC_STA_BUSY;	// busy

	OUT_DEBUG(_T("MC6843: cmd_SEK HEAD UNLOAD"));
	d_fdd->write_signal(SIG_FLOPPY_HEADLOAD, 0, 1);	// head unload
	head_load = false;

	register_my_event(EVENT_SEEK, set_delay(0xf0));	// seek track
}


/// Single Sector Read
void MC6843::cmd_SSR()
{
	gcr = 0;

	cmd_MSR();
}

/// Multi Sector Read
void MC6843::cmd_MSR()
{
	stra |= FDC_STA_BUSY;
	// clear status
	stra &= ~(FDC_STA_TRACKNE | FDC_STA_DELETE);
	strb &= ~FDC_STB_DATANF;

	int time =  set_delay(head_load ? 0x01 : 0x0f);
	if (!FLG_DELAY_FDSEARCH) time += d_fdd->get_clock_arrival_sector(0, sar, 0);
	register_search_event(time);
	d_fdd->write_signal(SIG_FLOPPY_HEADLOAD, 1, 1);	// head load
	head_load = true;
}

/// Free Format Read
/// TODO: This command does not read data from the disk.
void MC6843::cmd_FFR()
{
	gcr = 0;
	sar = 1;

	cmd_MSR();
}

/// Read CRC
void MC6843::cmd_RCR()
{
	gcr = 0;

	cmd_MSR();
}

/// Single Sector Write
void MC6843::cmd_SSW()
{
	gcr = 0;

	cmd_MSW();
}

/// Multi Sector Write
void MC6843::cmd_MSW()
{
	stra |= FDC_STA_BUSY;
	// clear status
	stra &= ~(FDC_STA_TRACKNE | FDC_STA_DELETE);
	strb &= ~FDC_STB_DATANF;

	int time =  set_delay(head_load ? 0x01 : 0x0f);
	if (!FLG_DELAY_FDSEARCH) time += d_fdd->get_clock_arrival_sector(0, sar, 0);
	register_search_event(time);
	d_fdd->write_signal(SIG_FLOPPY_HEADLOAD, 1, 1);	// head load
	head_load = true;
}

/// Single Sector Write with Delete Data Mark
void MC6843::cmd_SWD()
{
	gcr = 0;

	cmd_MSW();
}

/// Free Format Write
void MC6843::cmd_FFW()
{
	sar = 1;

	//
	memset(parse_clk_buf, 0, MC6843_PARSE_BUFFER);
	memset(parse_dat_buf, 0, MC6843_PARSE_BUFFER);
	parse_idx = 0;
	ffw_phase = 0;

	stra |= FDC_STA_BUSY;
	// clear status
	stra &= ~(FDC_STA_TRACKNE | FDC_STA_DELETE);
	strb &= ~FDC_STB_DATANF;

	int time =  set_delay(head_load ? 0x01 : 0x0f);
	if (!FLG_DELAY_FDSEARCH) time += d_fdd->get_index_hole_remain_clock();
	register_search_event(time);
	d_fdd->write_signal(SIG_FLOPPY_HEADLOAD, 1, 1);	// head load
	head_load = true;
}

/// stop of Free Format Write
void MC6843::cmd_FFW_END()
{
	if ((cmr & 15) != FDC_CMD_FFW)  return;

	d_fdd->parse_track(0);
}

// ----------------------------------------------------------------------------
void MC6843::event_seek(int event_no)
{
//	int cmd = cmr & 0x0f;

	stepcnt--;
	if (stepcnt < 0) {
		// seek error
		strb |= FDC_STB_SEEKERR;
		OUT_DEBUG(_T("MC6843: chg_stat strb:%02x"),strb);
	} else {
		// seek
		uint32_t seek = 0x80;
		if(gcr > ctar) {
			seek = 0x7f; // plus
		} else if (gcr < ctar) {
			seek = 0xff; // minus
		}
		d_fdd->write_signal(SIG_FLOPPY_STEP, seek, 0xff);
		if (d_fdd->read_signal(SIG_FLOPPY_TRACK0) != 0) {
			ctar = 0;
		} else if (seek < 0x80) {
			ctar++;
		} else if (seek > 0x80) {
			ctar--;
		}
//		ctar = d_fdd->read_signal(SIG_FLOPPY_CURRENTTRACK);

		if(gcr == ctar) {
//			|| (cmd == FDC_CMD_STZ && d_fdd->read_signal(SIG_FLOPPY_TRACK0) != 0)) {
			// match track

			find_track();

			now_seek = false;

			d_fdd->write_signal(SIG_FLOPPY_HEADLOAD, 1, 1);	// head load
//			head_load = true;
		}
		else {
			// seek next track
			register_my_event(event_no, set_delay(0xf0));
			return;
		}
	}

	// seek complete

	// update state
	ctar = gcr;
	gcr = 0;
	sar = 0;

	cmd_end();
}

void MC6843::event_search(int event_no)
{
	find_sector(sar);
	if ((stra & FDC_STA_TRACKNE) || strb != 0) {

		cmd_end();
		OUT_DEBUG(_T("MC6843: event_search HEAD UNLOAD"));
		d_fdd->write_signal(SIG_FLOPPY_HEADLOAD, 0, 1);	// head unload
		head_load = false;

		return;
	}

	event_search2(event_no);

//	cancel_my_event(EVENT_LOST);
//	if(!(strb & FDC_STB_SECTNF)) {
//		register_my_event(EVENT_LOST, SEEK_TIMEOUT);	// set timeout event
//	}
}

void MC6843::event_search2(int event_no)
{
	now_search = false;
	// start dma
	if(!(strb & FDC_STB_SECTNF) && (cmr & 0x0f) != FDC_CMD_RCR) {
		register_lost_event(3);
		stra |= FDC_STA_DRQ;
		// if no DMA then Status Sense is set.
		if (!(cmr & FDC_CMR_DMA)) {
			isr |=FDC_STI_STSREQ;
		}
	}

	status_update();
}

void MC6843::event_multi(int event_no)
{
	sar++;
	gcr--;
	cmd_MSR();
}

void MC6843::event_lost(int event_no)
{
	if((cmr & 0xf) != FDC_CMD_FFW && stra & FDC_STA_BUSY) {
		strb |= FDC_STB_DATAERR;
		OUT_DEBUG(_T("MC6843: chg_stat strb:%02x"),strb);

		cmd_end();
		OUT_DEBUG(_T("MC6843: event_lost HEAD UNLOAD"));
		d_fdd->write_signal(SIG_FLOPPY_HEADLOAD, 0, 1);	// head unload
		head_load = false;
	}
}

void MC6843::event_drq(int event_no)
{
	if(stra & FDC_STA_BUSY) {
		register_lost_event(1);
		stra |= FDC_STA_DRQ;
	}
}

// ----------------------------------------------------------------------------
// media handler
// ----------------------------------------------------------------------------
void MC6843::find_track()
{
	if(!d_fdd->search_track(0)) {
		strb |= FDC_STB_SEEKERR;
		OUT_DEBUG(_T("MC6843: chg_stat strb:%02x"),strb);
	}

	return;
}

void MC6843::find_sector(int sect)
{
	int cmd = cmr & 0x0f;

	// clear status
	strb &= ~(FDC_STB_CRCERR | FDC_STB_SECTNF);

	if (sar > 26) {
		// address error
		strb |= FDC_STB_SECTNF;
		OUT_DEBUG(_T("MC6843: chg_stat strb:%02x"),strb);
		return;
	}

	// check track number
//	ctar = d_fdd->read_signal(SIG_FLOPPY_CURRENTTRACK);
	if(!d_fdd->search_track(0)) {
		stra |= FDC_STA_TRACKNE;
		dir = d_fdd->get_current_track_number(0);
		OUT_DEBUG(_T("MC6843: chg_stat stra:%02x"),stra);
		return;
	}

	if (cmd != FDC_CMD_FFR && cmd != FDC_CMD_FFW) {
		// verify track number
		if (!d_fdd->verify_track(0, ltar)) {
//		if (ltar != ctar) {
			stra |= FDC_STA_TRACKNE;
			dir = d_fdd->get_current_track_number(0);
			OUT_DEBUG(_T("MC6843: chg_stat stra:%02x"),stra);
			return;
		}
	}

	// search sector when sector read or write
	if (cmd != FDC_CMD_FFR && cmd != FDC_CMD_FFW) {
		int stat = d_fdd->search_sector(0, ltar, sect, false, 0);
		if (stat & 1) {
			strb |= FDC_STB_SECTNF;
			OUT_DEBUG(_T("MC6843: chg_stat strb:%02x"),strb);
		}
		if (stat & 2) {
			strb |= FDC_STB_CRCERR;
			OUT_DEBUG(_T("MC6843: chg_stat strb:%02x"),strb);
		}
		// delete mark ?
		if (stat & 4) {
			stra |= FDC_STA_DELETE;
			OUT_DEBUG(_T("MC6843: chg_stat stra:%02x"),stra);
		}
	}

	data_idx = 0;

	return;
}

// ----------------------------------------------------------------------------
// for free format write
// ----------------------------------------------------------------------------

static const uint8_t address_mark_dat_tbl[8] =
	{ 0xcf, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

void MC6843::parse_twice_format(uint8_t data)
{
	uint8_t clk = 0;
	uint8_t dat = 0;

	parse_idx = 1 - parse_idx;

	// shift to 4bit right
	for (int i=(MC6843_PARSE_BUFFER-2); i >= 0; i--) {
		parse_clk_buf[i+1] = (parse_clk_buf[i+1] >> 4);
		parse_dat_buf[i+1] = (parse_dat_buf[i+1] >> 4);
		parse_clk_buf[i+1] |= ((parse_clk_buf[i] & 0x0f) << 4);
		parse_dat_buf[i+1] |= ((parse_dat_buf[i] & 0x0f) << 4);
	}
	parse_clk_buf[0] = (parse_clk_buf[0] >> 4);
	parse_dat_buf[0] = (parse_dat_buf[0] >> 4);

	clk = (data & 0x80) | ((data & 0x20) << 1) | ((data & 0x08) << 2) | ((data & 0x02) << 3);
	dat = ((data & 0x40) << 1) | ((data & 0x10) << 2) | ((data & 0x04) << 3) | ((data & 0x01) << 4);

	parse_clk_buf[0] |= clk;
	parse_dat_buf[0] |= dat;

	parse_dat = ((parse_dat_buf[0] & 0x0f) << 4) | ((parse_dat_buf[0] & 0xf0) >> 4);

	// index mark check
	if (memcmp(parse_dat_buf, address_mark_dat_tbl, 7) == 0) {
		parse_idx = 1;
	}

	if (parse_idx == 1) {
		d_fdd->write_signal(SIG_FLOPPY_WRITE_TRACK, parse_dat, 0xff);
	}

//	OUT_DEBUG("fdcffw c:%02x d:%02x",clk,dat);
}

void MC6843::parse_plane_format(uint8_t data)
{
	d_fdd->write_signal(SIG_FLOPPY_WRITE_TRACK, data, 0xff);
}

#if 0
// reverse
uint8_t address_mark_clk_tbl[3][8]={
	{ 0x7d, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00 },
	{ 0x7c, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00 },
	{ 0x7c, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00 }
};
uint8_t address_mark_dat_tbl[3][8]={
	{ 0xcf, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0xef, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0xbf, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
};

void MC6843::parse_ibm3740_format(uint8_t data)
{
	uint8_t clk = 0;
	uint8_t dat = 0;

	parse_idx = 1 - parse_idx;

	// shift to 4bit right
	for (int i=(MC6843_PARSE_BUFFER-2); i >= 0; i--) {
		parse_clk_buf[i+1] = (parse_clk_buf[i+1] >> 4);
		parse_dat_buf[i+1] = (parse_dat_buf[i+1] >> 4);
		parse_clk_buf[i+1] |= ((parse_clk_buf[i] & 0x0f) << 4);
		parse_dat_buf[i+1] |= ((parse_dat_buf[i] & 0x0f) << 4);
	}
	parse_clk_buf[0] = (parse_clk_buf[0] >> 4);
	parse_dat_buf[0] = (parse_dat_buf[0] >> 4);

	clk = ((data & 0x80) >> 4) | ((data & 0x20) >> 3) | ((data & 0x08) >> 2) | ((data & 0x02) >> 1);
	dat = ((data & 0x40) >> 3) | ((data & 0x10) >> 2) | ((data & 0x04) >> 1) | (data & 0x01);

	parse_clk_buf[0] |= (clk << 4);
	parse_dat_buf[0] |= (dat << 4);

	parse_dat = ((parse_dat_buf[0] & 0x0f) << 4) | ((parse_dat_buf[0] & 0xf0) >> 4);

	for (int i=0; i < 3; i++) {
		if (memcmp(parse_clk_buf, address_mark_clk_tbl[i], 7) == 0
			&& memcmp(parse_dat_buf, address_mark_dat_tbl[i], 7) == 0) {
			ffw_phase = i + 1;
			parse_idx = 0;
			break;
		}
	}

//	OUT_DEBUG("fdcffw c:%02x d:%02x",clk,dat);
}

void MC6843::write_ibm3740_format()
{
	if (parse_idx == 0) {
		if (ffw_phase == 0) {
			data_idx = 0;
		} else if (ffw_phase == 1) { // index mark
			ffw_phase = 0;
		} else if (ffw_phase == 2) {	// id mark
			if (data_idx == 3) {
				// search sector
				if(d_fdd->search_sector(0, parse_dat)) {
					ffw_phase = 0;
				}
			}
			data_idx++;
			if (data_idx >= 4) ffw_phase = 0;
		} else if (ffw_phase == 3) {	// data mark
			if (data_idx > 0) {
				if(d_fdd->read_signal(SIG_FLOPPY_WRITEPROTECT)) {
					// write protect
					strb |= FDC_STB_WRITEERR;
					stra &= ~FDC_STA_BUSY;
					stra &= ~FDC_STA_DRQ;

					ffw_phase = 0;
				} else {
					d_fdd->write_signal(SIG_FLOPPY_WRITE, parse_dat, 0xff);
				}
			}
			data_idx++;
			if (data_idx > 128) ffw_phase = 0;
		}
	}
}
#endif

// ----------------------------------------------------------------------------
void MC6843::save_state(FILEIO *fp)
{
	struct vm_state_st vm_state;

	//
	vm_state_ident.version = Uint16_LE(2);
	vm_state_ident.size = Uint32_LE(sizeof(vm_state_ident) + sizeof(vm_state));

	// copy values
	memset(&vm_state, 0, sizeof(vm_state));
	for(int i=0; i<4; i++) {
		vm_state.register_id[i] = Int32_LE(register_id[i]);
	}
	vm_state.dir = dir;
	vm_state.ctar = ctar;
	vm_state.cmr = cmr;
	vm_state.isr = isr;
	vm_state.sur = sur;
	vm_state.stra = stra;
	vm_state.strb = strb;
	vm_state.sar = sar;
	vm_state.gcr = gcr;
	vm_state.ccr = ccr;
	vm_state.ltar = ltar;

	vm_state.now = (now_search ? 2 : 0) | (now_seek ? 1 : 0);

	vm_state.data_idx = Int32_LE(data_idx);
	vm_state.stepcnt = Int32_LE(stepcnt);

	vm_state.register_id2[0] = Int32_LE(register_id[4]);

	fp->Fwrite(&vm_state_ident, sizeof(vm_state_ident), 1);
	fp->Fwrite(&vm_state, sizeof(vm_state), 1);
}

bool MC6843::load_state(FILEIO *fp)
{
	vm_state_ident_t vm_state_i;
	struct vm_state_st vm_state;

	READ_STATE_CHUNK(fp, vm_state_i, vm_state);

	// copy values
	for(int i=0; i<4; i++) {
		register_id[i] = Int32_LE(vm_state.register_id[i]);
	}
	dir = vm_state.dir;
	ctar = vm_state.ctar;
	cmr = vm_state.cmr;
	isr = vm_state.isr;
	sur = vm_state.sur;
	stra = vm_state.stra;
	strb = vm_state.strb;
	sar = vm_state.sar;
	gcr = vm_state.gcr;
	ccr = vm_state.ccr;
	ltar = vm_state.ltar;

	now_search = (vm_state.now & 2) ? true : false;
	now_seek   = (vm_state.now & 1) ? true : false;

	data_idx = Int32_LE(vm_state.data_idx);
	stepcnt = Int32_LE(vm_state.stepcnt);

	if (Uint16_LE(vm_state_i.version) >= 2) {
		register_id[4] = Int32_LE(vm_state.register_id2[0]);
	}

	return true;
}

// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
uint32_t MC6843::debug_read_io8(uint32_t addr)
{
	uint32_t data = 0;
	switch(addr & 15) {
	case 0:	// DIR
		data = dir;
		break;
	case 1:	// CTAR : current track (7bit)
		data = ctar;
		break;
	case 2:	// ISR
		data = isr;
		break;
	case 3:	// STRA
		data = stra;
		break;
	case 4:	// STRB
		data = strb;
		break;
	}
	return data;
}

static const _TCHAR *c_reg_names[] = {
	_T("CTAR"),
	_T("CMR"),
	_T("ISR"),
	_T("SUR"),
	_T("STRA"),
	_T("STRB"),
	_T("SAR"),
	_T("GCR"),
	_T("CCR"),
	_T("LTAR"),
	NULL
};

bool MC6843::debug_write_reg(uint32_t reg_num, uint32_t data)
{
	switch(reg_num) {
	case 0:
		write_io8(1, data);
		return true;
	case 1:
		write_io8(2, data);
		return true;
	case 2:
		isr = data & 0xff;
		return true;
	case 3:
		write_io8(3, data);
		return true;
	case 4:
		stra = data & 0xff;
		return true;
	case 5:
		strb = data & 0xff;
		return true;
	case 6:
		write_io8(4, data);
		return true;
	case 7:
		write_io8(5, data);
		return true;
	case 8:
		write_io8(6, data);
		return true;
	case 9:
		write_io8(7, data);
		return true;
	}
	return false;
}

bool MC6843::debug_write_reg(const _TCHAR *reg, uint32_t data)
{
	uint32_t num = find_debug_reg_name(c_reg_names, reg);
	return debug_write_reg(num, data);
}

void MC6843::debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	buffer[0] = _T('\0');
	UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%s):%02X"), 0, c_reg_names[0], ctar);
	UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%s):%02X"), 1, c_reg_names[1], cmr);
	UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%s):%02X"), 2, c_reg_names[2], isr);
	UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%s):%02X"), 3, c_reg_names[3], sur);
	UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%s):%02X"), 4, c_reg_names[4], stra);
	UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%s):%02X"), 5, c_reg_names[5], strb);
	UTILITY::tcscat(buffer, buffer_len, _T("\n"));
	UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%s):%02X"), 6, c_reg_names[6], sar);
	UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%s):%02X"), 7, c_reg_names[7], gcr);
	UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%s):%02X"), 8, c_reg_names[8], ccr);
	UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%s):%02X"), 9, c_reg_names[9], ltar);
}
#endif
