/** @file mc6843.h

	HITACHI BASIC MASTER LEVEL3 Mark5 / MB-S1 Emulator 'EmuB-6892/EmuB-S1'
	Skelton for retropc emulator

	@par Origin
	mess & mb8877.c
	@author Sasaji
	@date   2011.11.05 -

	@brief [ fdc modoki (mc6843) ]
*/

#ifndef MC6843_H
#define MC6843_H

#include "vm_defs.h"
#include "device.h"

// #define _DEBUG_MC6843

#define MC6843_PARSE_BUFFER	8

class EMU;
//class FLOPPY;

/**
	@brief fdc modoki (mc6843) - Floppy Disk Controller
*/
class MC6843 : public DEVICE
{
public:
	/// @brief signals of MC6843
	enum SIG_MC6843_IDS {
		SIG_MC6843_UPDATESTATUS	= 0
	};

private:
	/// @brief event ids
	enum EVENT_IDS {
		EVENT_SEEK		= 0,
		EVENT_SEARCH	= 1,
		EVENT_MULTI		= 2,
		EVENT_LOST		= 3,
		EVENT_DRQ		= 4,

		MC6843_MAX_EVENT = 5
	};

	/// @brief interrupt status
	enum STI_MASKS {
		FDC_STI_STRB		= 0x08,	///< STRB
		FDC_STI_STSREQ		= 0x04,	///< status sense request
		FDC_STI_SETCOMP		= 0x02,	///< setting time complete
		FDC_STI_CMDCOMP		= 0x01,	///< macro command complete
	};

	/// @brief status code A
	enum STA_MASKS {
		FDC_STA_BUSY		= 0x80,	///< busy
		FDC_STA_INDEX		= 0x40,	///< index hole
		FDC_STA_TRACKNE		= 0x20,	///< track not equal
		FDC_STA_WRITEP		= 0x10,	///< write protect
		FDC_STA_TRACK00		= 0x08,	///< track zero
		FDC_STA_DREADY		= 0x04,	///< drive ready
		FDC_STA_DELETE		= 0x02,	///< delete data mark detected
		FDC_STA_DRQ			= 0x01,	///< data transfar request
	};

	/// @brief status code B
	enum STB_MASKS {
		FDC_STB_HARDERR		= 0x80,	///< hard error
		FDC_STB_WRITEERR	= 0x40,	///< write error
		FDC_STB_FILEINO		= 0x20,	///< file inoperable
		FDC_STB_SEEKERR		= 0x10,	///< seek error
		FDC_STB_SECTNF		= 0x08,	///< sector address undeteced
		FDC_STB_DATANF		= 0x04,	///< data mark undetected
		FDC_STB_CRCERR		= 0x02,	///< crc error
		FDC_STB_DATAERR		= 0x01,	///< data transfar error
	};

	/// @brief command register
	enum CMR_MASKS {
		FDC_CMR_FWF			= 0x10,	///< free format write flag
		FDC_CMR_DMA			= 0x20,	///< DMA flag
		FDC_CMR_ISR3MASK	= 0x40,	///< ISR3 interrupt mask
		FDC_CMR_FUNCMASK	= 0x80,	///< Function interrupt mask
	};

	/// @brief macro commands
	enum CMD_MASKS {
		FDC_CMD_FFW_END = 0x00,	///< stop of multi sector write
		FDC_CMD_STZ		= 0x02,	///< seek track zero
		FDC_CMD_SEK		= 0x03,	///< seek
		FDC_CMD_SSR		= 0x04,	///< single sector read
		FDC_CMD_SSW		= 0x05,	///< single sector write
		FDC_CMD_RCR		= 0x06,	///< read CRC
		FDC_CMD_SWD		= 0x07,	///< single sector write with delete data mark
		FDC_CMD_FFR		= 0x0a,	///< free format read
		FDC_CMD_FFW		= 0x0b,	///< free format write
		FDC_CMD_MSR		= 0x0c,	///< multi sector read
		FDC_CMD_MSW		= 0x0d,	///< multi sector write
	};

private:
	DEVICE *d_fdd;

	// registers
//	uint8_t dor;        // reg0   w: disk write operation
	uint8_t dir;        // reg0 r  : disk read operation
	uint8_t ctar;       // reg1 r/w: current track (7bit)
	uint8_t cmr;        // reg2   w: command
	uint8_t isr;        // reg2 r  : interrupt status
	uint8_t sur;        // reg3   w: set-up
	uint8_t stra;       // reg3 r  : status a
	uint8_t strb;       // reg4 r  : status b
	uint8_t sar;        // reg4   w: sector address (5bit)
	uint8_t gcr;        // reg5   w: general count (track  or  sector) (7bit)
	uint8_t ccr;        // reg6   w: crc control (2bit)
	uint8_t ltar;       // reg7   w: logical address track (=track destination) (7bit)

	// event
	int register_id[MC6843_MAX_EVENT];

	// output signals
	outputs_t outputs_irq;

	//
	int data_idx;    // current read/write position in data

	int stepcnt;	// step count
	bool now_seek;		// now seeking?
	bool now_search;	// now searching?
	bool head_load;		// now head load

	// for parse format
	uint8_t parse_clk_buf[MC6843_PARSE_BUFFER];
	uint8_t parse_dat_buf[MC6843_PARSE_BUFFER];
	uint8_t parse_dat;
	int   parse_idx;
	int   ffw_phase;

	//for resume
#pragma pack(1)
	struct vm_state_st {
		int register_id[4];

		uint8_t dir;
		uint8_t ctar;
		uint8_t cmr;
		uint8_t isr;
		uint8_t sur;
		uint8_t stra;
		uint8_t strb;
		uint8_t sar;
		uint8_t gcr;
		uint8_t ccr;
		uint8_t ltar;

		uint8_t now;

		int data_idx;
		int stepcnt;

		// version 2
		int register_id2[1];

		char reserved[8];
	};
#pragma pack()

	void cancel_my_event(int event_no);
	void register_my_event(int event_no, int wait);
	void register_search_event(int wait);
	void register_drq_event(int bytes);
	void register_lost_event(int bytes);
	void cancel_my_events();

	void status_update();
	void cmd_end();

	void cmd_STZ();
	void cmd_SEK();
	void cmd_SSR();
	void cmd_SSW();
	void cmd_RCR();
	void cmd_SWD();
	void cmd_FFR();
	void cmd_FFW();
	void cmd_MSR();
	void cmd_MSW();
	void cmd_FFW_END();

	int  set_delay(uint8_t);

	void event_seek(int);
	void event_search(int);
	void event_search2(int);
	void event_multi(int);
	void event_lost(int);
	void event_drq(int);

	uint8_t read_data_reg();
	void write_data_reg(uint8_t);

	void update_stra();

	void find_track();
	void find_sector(int);

	// irq
	void set_irq(bool val);

	// for free format write
	void parse_twice_format(uint8_t data);
	void parse_plane_format(uint8_t data);
//	void parse_ibm3740_format(uint8_t data);
//	void write_ibm3740_format();

public:
	MC6843(VM* parent_vm, EMU* parent_emu, const char* identifier) : DEVICE(parent_vm, parent_emu, identifier) {
		set_class_name("MC6843");
		init_output_signals(&outputs_irq);
	}
	~MC6843() {}

	// common functions
	void initialize();
	void reset();
	void release();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);

	void write_signal(int id, uint32_t data, uint32_t mask);

	// unique function
	void set_context_irq(DEVICE* device, int id, uint32_t mask) {
		register_output_signal(&outputs_irq, device, id, mask);
	}
	void set_context_fdd(DEVICE* device) {
		d_fdd = device;
	}

	// event callback
	void event_frame();
	void event_callback(int event_id, int err);

	void save_state(FILEIO *fp);
	bool load_state(FILEIO *fp);

#ifdef USE_DEBUGGER
	uint32_t debug_read_io8(uint32_t addr);
	bool debug_write_reg(uint32_t reg_num, uint32_t data);
	bool debug_write_reg(const _TCHAR *reg, uint32_t data);
	void debug_regs_info(_TCHAR *buffer, size_t buffer_len);
#endif
};

#endif /* MC6843_H */
