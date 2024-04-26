/** @file floppy.h

	HITACHI BASIC MASTER LEVEL3 Mark5 Emulator 'EmuB-6892'
	Skelton for retropc emulator

	@author Sasaji
	@date   2011.11.05 -

	@brief [ floppy drive ]
*/

#ifndef FLOPPY_H
#define FLOPPY_H

#include "../vm_defs.h"
//#include "../../emu.h"
#include "../device.h"
//#include "parsewav.h"
#include "../noise.h"

//#define _DEBUG_FLOPPY

#include "../floppy_defs.h"


#define MAX_FDC_NUMS				1

//class FDC;
class EMU;
class DISK;

/**
	@brief floppy drive
*/
class FLOPPY : public DEVICE
{
private:
	/// @brief event ids
	enum EVENT_IDS {
		EVENT_IRQ			= 0,
		EVENT_DRQ			= 1,

		EVENT_MOTOR_TIMEOUT	= 2,
		EVENT_INDEXHOLE_ON	= 3,
		EVENT_INDEXHOLE_OFF	= 4,

		EVENT_READY_ON_0	= 5,
		EVENT_READY_ON_1	= 6,
		EVENT_READY_ON_2	= 7,
		EVENT_READY_ON_3	= 8,
		EVENT_READY_ON_4	= 9,
		EVENT_READY_ON_5	= 10,
		EVENT_READY_ON_6	= 11,
		EVENT_READY_ON_7	= 12,
		EVENT_MOTOR_OFF		= 13,

		FLOPPY_MAX_EVENT_V1			= 5,
		FLOPPY_MAX_EVENT			= 20
	};

	/// @brief delay time on each modules
	enum DELAY_TIMES {
		/// index hole (2D,2DD) (usually 300rpm -> 200ms)
		DELAY_INDEX_HOLE	  = 200000,	// (us)
		//DELAY_INDEX_HOLE	   = 50000,
		/// index hole (8inch/5inch 2HD) (usually 360rpm -> 166.666ms)
		DELAY_INDEX_HOLE_H	  = 166667,	// (us)

		/// ready on delay time after motor on (us)
		DELAY_READY_ON		 = 1000000,
		/// ready on delay time after motor on (us) (8inch/5inch 2HD) 
		DELAY_READY_ON_H	    = 1000,
		//DELAY_READY_ON_H	  = 500000,

		/// motor warmup time (us)
		DELAY_MOTOR_WARMUP	  = 500000,
		/// motor off delay time (us)
		DELAY_MOTOR_OFF_3	 = 3000000,
		DELAY_MOTOR_OFF_5S	 = 5000000,
		DELAY_MOTOR_OFF_5	= 11000000,
		DELAY_MOTOR_TIMEOUT	= 60000000,

		/// head loaded time 60ms
		HEAD_LOADED_TIME	= 60000,
		HEAD_LOADED_CLOCK	= (HEAD_LOADED_TIME * (CPU_CLOCKS / 1000000)),

		DELAY_WRITE_FRAME	= 300,
	};

private:
	DEVICE *d_fdc3, *d_fdc5[MAX_FDC_NUMS], *d_board;

	/// config
	bool m_ignore_crc;

	/// event
	int m_register_id[FLOPPY_MAX_EVENT];

	//
	uint8_t m_drv_num[MAX_FDC_NUMS];	///< drive number
	uint8_t m_drvsel[MAX_FDC_NUMS];	///< drive select + motor

	uint8_t m_sidereg;	///< head select

	uint8_t m_index_hole;	///< index hole
	uint64_t m_index_hole_next_clock;
	uint8_t m_head_load;	///< head loaded

	uint8_t m_density;	///< density (double = 1)
	uint8_t m_motor_on_expand[MAX_FDC_NUMS];	///< motor on (expand)

	int m_sectorcnt;
	bool m_sectorcnt_cont;

	uint8_t m_fdd5outreg[MAX_FDC_NUMS];	///< bit7:/DRQ bit0:/IRQ
	uint8_t m_fdd5outreg_delay;			///< bit7:/DRQ for 8inch or 2HD

	bool m_irqflg;
	bool m_irqflgprev;
	bool m_drqflg;
	bool m_drqflgprev;

	// output signals
	outputs_t outputs_irq;
	outputs_t outputs_drq;

	/// drive info
	typedef struct {
		int side;
		int track;
		int index;
#ifdef USE_SIG_FLOPPY_ACCESS
		bool access;
#endif
		uint8_t ready;			///< ready warmup:01 on:11
		uint8_t motor_warmup;	///< motor warming up:1
		uint8_t head_loading;
		int delay_write;

		bool shown_media_error; 
	} fdd_t;
	fdd_t m_fdd[USE_FLOPPY_DISKS];

	// diskette info
	DISK* p_disk[USE_FLOPPY_DISKS];

	// index hole
	int m_delay_index_hole;	///< spent clocks per round (cpu clocks)
	int m_limit_index_hole; ///< width of asserting index signal (cpu clocks)
	int m_delay_index_mark;	///< clocks in index mark area (cpu clocks)
	// ready on
	int m_delay_ready_on;

//	int sector_step[1];

	int m_sample_rate;
//	int max_vol;
//	int wav_volume;
//	int wav_enable;
	int m_wav_fddtype;
	bool m_wav_loaded_at_first;

	/// sound data 1st dimension [0]:3inch [1]:5inch [2]:8inch
	enum WAV_FDDTYPES {
		FLOPPY_WAV_FDDTYPES = 3,
		FLOPPY_WAV_FDD3	= 0,
		FLOPPY_WAV_FDD5	= 1,
		FLOPPY_WAV_FDD8	= 2,
	};
	/// 2nd [0]:seek sound [1]:motor sound [2]:head on [3]:head off
	enum WAV_SNDTYPES {
		FLOPPY_WAV_SNDTYPES = 4,
		FLOPPY_WAV_SEEK		= 0,
		FLOPPY_WAV_MOTOR	= 1,
		FLOPPY_WAV_HEADON	= 2,
		FLOPPY_WAV_HEADOFF	= 3,
	};
//	_TCHAR wav_file[FLOPPY_WAV_FDDTYPES][FLOPPY_WAV_SNDTYPES][20];
//	uint8_t *wav_data[FLOPPY_WAV_FDDTYPES][FLOPPY_WAV_SNDTYPES];
//	int    wav_size[FLOPPY_WAV_FDDTYPES][FLOPPY_WAV_SNDTYPES];
//	// [0]:seek sound [1]:motor sound [2]:head on [3]:head off
//	int    wav_play_pos[FLOPPY_WAV_SNDTYPES];
//	int    wav_play[FLOPPY_WAV_SNDTYPES];
	NOISE  m_noises[FLOPPY_WAV_FDDTYPES][FLOPPY_WAV_SNDTYPES];

	//for resume
#pragma pack(1)
	struct vm_state_st_v1 {
		int register_id[6];
		uint8_t drv_num;
		uint8_t drvsel;
		uint8_t sidereg;
		uint8_t fdd5outreg;
		uint8_t irqflg;
		uint8_t drqflg;

		char reserved[2];
	};
	struct vm_state_st_v2 {
		int register_id[22];
		uint8_t drv_num;
		uint8_t drvsel;
		uint8_t sidereg;
		uint8_t fdd5outreg;
		uint8_t irqflg;
		uint8_t drqflg;

		// version 5
		char reserved0[4];
		// version 5
		uint8_t flags;

		char reserved[13];

		// version 6
		struct NOISE::vm_state_st m_noises[FLOPPY_WAV_FDDTYPES][FLOPPY_WAV_SNDTYPES];
	};
#pragma pack()
	// cannot write when load state from file
	bool m_ignore_write;

	void warm_reset(bool);

	inline void cancel_my_event(int);
	inline void register_my_event(int, int);
	void register_index_hole_event(int, int);
	void cancel_my_events();

	enum FDC_TYPES {
		FDC_TYPE_3INCH = 0,
		FDC_TYPE_5INCH,
		FDC_TYPE_5INCHEX,
	};
	void set_drive_select(int fdc_type, uint8_t data);

	// irq/dma
	void set_irq(bool val);
	void set_drq(bool val);

	int  search_sector_main(int fdcnum, int drvnum, int index);

	void load_wav();

	void motor(int drv, bool val);

	void set_drive_speed();

public:
	FLOPPY(VM* parent_vm, EMU* parent_emu, const char* identifier) : DEVICE(parent_vm, parent_emu, identifier) {
		set_class_name("FLOPPY");
		init_output_signals(&outputs_irq);
		init_output_signals(&outputs_drq);
		d_fdc3 = NULL;
		for(int i=0; i<MAX_FDC_NUMS; i++) {
			d_fdc5[i] = NULL;
		}
	}
	~FLOPPY() {}

	// common functions
	void initialize();
	void reset();
	void release();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
	uint32_t read_signal(int id);

	// unique functions
	void set_context_fdc(DEVICE* device3, DEVICE* device5) {
		d_fdc3 = device3;
		d_fdc5[0] = device5;
	}
	void set_context_irq(DEVICE* device, int id, uint32_t mask) {
		register_output_signal(&outputs_irq, device, id, mask);
	}
	void set_context_drq(DEVICE* device, int id, uint32_t mask) {
		register_output_signal(&outputs_drq, device, id, mask);
	}
	void set_context_board(DEVICE* device) {
		d_board = device;
	}

	// event callback
	void event_frame();
	void event_callback(int event_id, int err);

	bool search_track(int channel);
	bool verify_track(int channel, int track);
	int  get_current_track_number(int channel);
	int  search_sector(int channel);
//	int  search_sector(int channel, int sect);
	int  search_sector(int channel, int track, int sect, bool compare_side, int side);
	bool make_track(int channel);
	bool parse_track(int channel);

	int get_head_loading_clock(int channel);
	int get_index_hole_remain_clock();
	int calc_index_hole_search_clock(int channel);
	int get_clock_arrival_sector(int channel, int sect, int delay);
	int get_clock_next_sector(int channel, int delay);
	int calc_sector_search_clock(int channel, int sect);
	int calc_next_sector_clock(int channel);

	// user interface
	bool open_disk(int drv, const _TCHAR *path, int offset, uint32_t flags);
	bool close_disk(int drv, uint32_t flags);
	int  change_disk(int drv);
	void set_disk_side(int drv, int side);
	int  get_disk_side(int drv);
	bool disk_inserted(int drv);
	void set_drive_type(int drv, uint8_t type);
	uint8_t get_drive_type(int drv);
	uint8_t fdc_status();
	void toggle_disk_write_protect(int drv);
	bool disk_write_protected(int drv);
	bool is_same_disk(int drv, const _TCHAR *file_path, int offset);
	int  inserted_disk_another_drive(int drv, const _TCHAR *file_path, int offset);

	uint16_t get_drive_select();

	void mix(int32_t* buffer, int cnt);
	void set_volume(int decibel, bool vol_mute);
	void initialize_sound(int rate, int decibel);

	void save_state(FILEIO *fp);
	bool load_state(FILEIO *fp);

#ifdef USE_DEBUGGER
	uint32_t debug_read_io8(uint32_t addr);
	bool debug_write_reg(uint32_t reg_num, uint32_t data);
	bool debug_write_reg(const _TCHAR *reg, uint32_t data);
	void debug_regs_info(_TCHAR *buffer, size_t buffer_len);
#endif
};

#endif /* FLOPPY_H */

