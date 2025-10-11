/** @file cmt.h

	HITACHI BASIC MASTER LEVEL3 Mark5 Emulator 'EmuB-6892'
	Skelton for retropc emulator

	@author Sasaji
	@date   2011.06.13 -

	@brief [ cmt ]
*/

#ifndef CMT_H
#define CMT_H

#include "../vm_defs.h"
//#include "../../emu.h"
#include "../device.h"
//#include "acia.h"
//#include "parsewav.h"
#include "../noise.h"

#define CMTDATA_BUFFER_SIZE	1024
#define WAVE_PARSE_SIZE		170

#define TAPEWAV_BUFFER_SIZE	2048

class EMU;
class FILEIO;
namespace PARSEWAV {
	class ParseWav;
};

/**
	@brief Cassette Magnetic Tape
*/
class CMT : public DEVICE
{
public:
	/// @brief signals on CMT
	enum SIG_CMT_IDS {
		SIG_CMT_REMOTE		= 1,
		SIG_CMT_BAUD_SEL	= 2
	};

private:
	DEVICE* d_ctrl;

	uint8_t  data_buffer[CMTDATA_BUFFER_SIZE];
	char   w_onedata[CMTDATA_BUFFER_SIZE];
	int    w_onelen;
	char   c_onedata[CMTDATA_BUFFER_SIZE];
	int    c_onelen;
	int    data_read_pos;
	int    data_read_size;
	int    data_write_pos;
	int    data_write_size;
	int	   data_total_pos;
	bool   buffer_overflow;
	int    file_type;	// 0:wav 1:l3c 2:l3c 3:l3 4:t9x
	bool   is_bytedata;	// .l3 = true
	bool   need_header;
	int    data_len;
	uint32_t file_startpos;

	/// This format is undocumented.
	typedef struct t9x_header_st {
		char ident[32];
		int  data1;
		int  data2;
		char reserved[24];
	} t9x_header_t;
	t9x_header_t t9x_header;
	uint8_t  t9x_data;
	int      t9x_dlen;

	FILEIO   *fio;
	PARSEWAV::ParseWav *wav;
	bool play, rec;
//	bool remote;		// remote control
	uint8_t remote_prev;
//	uint8_t baud_sel;		// baud select 0:600 1:1200
	int  baud_rate;
	bool send_ok;		// send to acia
	bool recv_ok;		// receive from acia
	int fast_rotate;	// 0: play/rec 1:fast forward -1:rewind
	bool tmp_playrec; 
	bool txdata_received;
	uint32_t txdata;

	int sample_rate_base;
	int sample_rate;
	int samples_base;
//	int max_vol;
//	int relay_volume;
//	int tape_volume;
//	int relay_wav_enable;
	bool relay_wav_loaded_at_first;
	enum WAV_RELAY_TYPES {
		WAV_RELAY_ON = 0,
		WAV_RELAY_OFF,
		WAV_RELAY_ALL,
	};
//	_TCHAR relay_file[2][16];
//	uint8_t *relay_data[2];
//	int    relay_size[2];
//	int    relay_play_pos;
	int   m_relay_play;
	NOISE m_relay[WAV_RELAY_ALL];

	NOISE_FIFO m_tape_wave;
//	int16_t *tape_wave_buf;
//	int   tape_wave_buf_wpos;
//	int   tape_wave_buf_rpos;
	int   tape_wave_nums;
	int   tape_wave_onoff;

	int   devide;
	int   surplus;
	int   a_prev;
	int   tape_wav_freq;

	int register_id[2];

	//for resume
#pragma pack(1)
	struct vm_state_st {
		int register_id;
		uint8_t flags;
		uint8_t baud_sel; // add version 3

		char  reserved[10];
	};
#pragma pack()
	bool  set_load_param();
	bool  set_save_param();
	int   load_wav_image();
	int   load_image(int dir);
	void  save_image();
	void        ready_data(uint32_t);
	void		set_data(uint32_t);
	uint32_t	get_data();
	uint32_t	get_data_fast();
	int  check_extension(const _TCHAR *);
	int  check_file_format(FILEIO *, int);
	int  set_file_header(FILEIO *, int);

	void register_my_event(int);
	void cancel_my_event(int);

	void load_relay_wav();

	void set_baud(int sel);

	void create_tape_wave();
	void set_tape_wave(uint32_t data);
	void set_tape_one_wave(int cnt, int samp);
	void set_tape_amp(char *c_data, int c_len);
	void set_tape_one_amp(char data);
	void set_tape_direct(int16_t *w_data, int w_len);

public:
	CMT(VM* parent_vm, EMU* parent_emu, const char* identifier) : DEVICE(parent_vm, parent_emu, identifier) {
		set_class_name("CMT");
		d_ctrl = NULL;
	}
	~CMT() {}

	// common functions
	void initialize();
	void reset();
	void release();

	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);

	// unique functions
	void set_context_ctrl(DEVICE* device) {
		d_ctrl = device;
	}
	bool play_datarec(const _TCHAR* filename);
	bool rec_datarec(const _TCHAR* filename);
	void close_datarec();
	void rewind_datarec();
	void fast_forward_datarec();
	void stop_datarec();
	void realmode_datarec();
	bool datarec_opened(bool play_mode);

	void event_callback(int , int);
	void mix(int32_t* buffer, int cnt);
	void set_volume(int decibel_relay, int decibel_tape, bool mute_relay, bool mute_tape);
	void initialize_sound(int rate, int decibel);

	uint32_t get_cmt_mode();

	void update_config();

	void save_state(FILEIO *fp);
	bool load_state(FILEIO *fp);

#ifdef USE_DEBUGGER
	bool debug_write_reg(uint32_t reg_num, uint32_t data);
	bool debug_write_reg(const _TCHAR *reg, uint32_t data);
	void debug_regs_info(const _TCHAR *title, _TCHAR *buffer, size_t buffer_len);
#endif
};

#endif /* CMT_H */
