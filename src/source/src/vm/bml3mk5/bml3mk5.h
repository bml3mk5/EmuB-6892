/** @file bml3mk5.h

	HITACHI BASIC MASTER LEVEL3 MARK5 Emulator
	Skelton for retropc emulator

	@author Sasaji
	@date   2011.06.08 -

	@brief [ virtual machine ]
*/

#ifndef BML3MK5_H
#define BML3MK5_H

#include "bml3mk5_defs.h"
#include "../../common.h"
#include "registers.h"

class EMU;
class DEVICE;
class EVENT;

class HD46505;
class AY38910;
//class YM2203;
class MC6809;

class BOARD;
class PIA;
class ACIA;
class DISPLAY;
class KEYBOARD;
class MEMORY;
class SOUND;
class COMM;
class CMT;
class PRINTER;
class TIMER;
class KANJI;
class VIA;
class PSGC;
class PSG9C;
#ifdef USE_FD1
class MC6843;
class MB8866;
class FLOPPY;
#endif
#ifdef USE_RTC
class RTC;
class MSM58321;
#endif
#ifdef USE_DEBUGGER
class DEBUGGER;
#endif
class KEYRECORD;

/// for sleep/resume the machine
#pragma pack(1)
typedef struct vm_state_header_st {
	char header[16];
	uint16_t version;
	uint16_t revision;
	uint32_t param;
	uint16_t emu_major;
	uint16_t emu_minor;
	uint16_t emu_rev;
	char reserved[2];
} vm_state_header_t;
#pragma pack()

/**
	@brief virtual machine for BML3MK5
*/
class VM
{
protected:
	EMU* emu;

	/// @name devices
	//@{
	EVENT* event;

	HD46505* crtc;
	AY38910* psg3[2];
	AY38910* psg9[3];
	MC6809* cpu;

	BOARD* board;
	PIA* pia;
	ACIA* acia;
	DISPLAY* display;
	KEYBOARD* key;
	MEMORY* memory;
	SOUND* sound;
	COMM* comm[MAX_COMM];
	CMT* cmt;
	PRINTER* printer[MAX_PRINTER];
	TIMER* timer;
	KANJI* kanji;
	PIA* psg3_pia;
	PSGC* psg3c;
	VIA* psg9_via;
	PSG9C* psg9c;
#ifdef USE_FD1
	MC6843* fdc3;
	MB8866* fdc5;
	FLOPPY* fdd;
#endif
	PIA* pia_ex;	///< extended printer port
	ACIA* acia_ex;	///< extended comm port

	PIA* pia_ex2;	///< extended PIA
#ifdef USE_RTC
	RTC* rtc;
	MSM58321 *msm58321;
#endif

	DEVICE* dummy;

//#ifdef USE_DEBUGGER
//	DEBUGGER* debugger;
//#endif
	//@}

	KEYRECORD* reckey;

	void change_fdd_type(int num, bool reset);

public:
	DEVICE* first_device;
	DEVICE* last_device;

public:
	// ----------------------------------------
	// initialize
	// ----------------------------------------
	/// @name initialize
	//@{
	VM(EMU* parent_emu);
	~VM();
	//@}
	// ----------------------------------------
	// for emulation class
	// ----------------------------------------
	/// @name drive virtual machine
	//@{
	void run(int split_num);
	double get_frame_rate();
	bool now_skip();
	void update_params();
	void pause(int value);
	//@}
	/// @name draw screen
	//@{
	void set_display_size(int left, int top, int right, int bottom);
	void draw_screen();
	uint64_t update_led();
	//@}
	/// @name sound generation
	//@{
	void initialize_sound(int rate, int samples);
	void reset_sound(int rate, int samples);
	audio_sample_t* create_sound(int* extra_frames, int samples);
	//@}
	/// @name input event from emu
	//@{
	void key_down(int code);
	void key_up(int code);
	void system_key_down(int code);
	void system_key_up(int code);
	//@}
	/// @name socket event from emu
	//@{
//	void network_connected(int ch);
//	void network_disconnected(int ch);
//	void network_writeable(int ch);
//	void network_readable(int ch);
//	void network_accepted(int ch, int new_ch);
//	uint8_t* get_sendbuffer(int ch, int* size, int* flags);
//	void inc_sendbuffer_ptr(int ch, int size);
//	uint8_t* get_recvbuffer0(int ch, int* size0, int* size1, int* flags);
//	uint8_t* get_recvbuffer1(int ch);
//	void inc_recvbuffer_ptr(int ch, int size);
	//@}
	/// @name control menu for user interface
	//@{
	void reset();
	void special_reset();
	void warm_reset(int onoff);
	void update_config();
	void change_dipswitch(int num);
	bool save_state(const _TCHAR* filename);
	bool load_state(const _TCHAR* filename);
#ifdef USE_KEY_RECORD
	bool play_reckey(const _TCHAR* filename);
	bool record_reckey(const _TCHAR* filename);
	void stop_reckey(bool stop_play = true, bool stop_record = true);
#endif
	void change_archtecture(int id, int num, bool reset);
	//@}
#ifdef USE_DATAREC
	/// @name tape menu for user interface
	//@{
	bool play_datarec(const _TCHAR* file_path);
	bool rec_datarec(const _TCHAR* file_path);
	void close_datarec();
	void rewind_datarec();
	void fast_forward_datarec();
	void stop_datarec();
	void realmode_datarec();
	bool datarec_opened(bool play_mode);
	//@}
#endif
#ifdef USE_FD1
	/// @name floppy disk menu for user interface
	//@{
	bool open_floppy_disk(int drv, const _TCHAR* file_path, int offset, uint32_t flags);
	bool close_floppy_disk(int drv, uint32_t flags);
	int  change_floppy_disk(int drv);
	bool floppy_disk_inserted(int drv);
	int  get_floppy_disk_side(int drv);
	void toggle_floppy_disk_write_protect(int drv);
	bool floppy_disk_write_protected(int drv);
	bool is_same_floppy_disk(int drv, const _TCHAR *file_path, int offset);
	//@}
#endif
	/// @name sound menu for user interface
	//@{
	void set_volume();
	//@}
#ifdef USE_PRINTER
	/// @name printer menu for user interface
	//@{
	bool save_printer(int dev, const _TCHAR* filename);
	void clear_printer(int dev);
	int  get_printer_buffer_size(int dev);
	uint8_t* get_printer_buffer(int dev);
	void enable_printer_direct(int dev);
	bool print_printer(int dev);
	void toggle_printer_online(int dev);
	//@}
#endif
	/// @name comm menu for user interface
	//@{
	void enable_comm_server(int dev);
	void enable_comm_connect(int dev, int num);
	bool now_comm_connecting(int dev, int num);
	void send_comm_telnet_command(int dev, int num);
	//@}
	/// @name options menu for user interface
	//@{
	void modify_joytype();
	void save_keybind();
	void clear_joy2joyk_map();
	void set_joy2joyk_map(int num, int idx, uint32_t joy_code);
	//@}

	// ----------------------------------------
	// for each device
	// ----------------------------------------
	/// @name event callbacks
	//@{
	void register_event(DEVICE* device, int event_id, int usec, bool loop, int* register_id);
	void register_event_by_clock(DEVICE* device, int event_id, int clock, bool loop, int* register_id);
	void cancel_event(DEVICE* device, int register_id);
	void register_frame_event(DEVICE* dev);
	void register_vline_event(DEVICE* dev);
	void set_lines_per_frame(int lines);
	//@}
	/// @name clock
	//@{
	uint64_t get_current_clock();
	uint64_t get_passed_clock(uint64_t prev);
//	uint32_t get_pc();
	//@}
	/// @name get devices
	//@{
	DEVICE* get_device(int id);
	DEVICE* get_device(char *name, char *identifier);
	//@}

	/// for EMU::get_parami method
	enum enumParamiId {
		ParamFddType = 0,		///< current FDD type
		ParamIOPort,			///< current I/O port settings
		ParamRecVideoType,
		ParamRecVideoCodec,
		ParamRecVideoQuality,
		ParamRecAudioType,
		ParamRecAudioCodec,
		ParamiUnknown
	};
	/// for EMU::get_paramv method
	enum enumParamvId {
		ParamvDummy = 0,
		ParamvUnknown
	};
	/// for VM::change_archtecture method
	enum enumArchId {
		ArchFddType = 0,
	};
	// ----------------------------------------
	// access to emu class 
	// ----------------------------------------
	/// @name load rom image
	//@{
	static int load_data_from_file(const _TCHAR *file_path, const _TCHAR *file_name
		, uint8_t *data, size_t size
		, const uint8_t *first_data = NULL, size_t first_data_size = 0, size_t first_data_pos = 0
		, const uint8_t *last_data = NULL,  size_t last_data_size = 0, size_t last_data_pos = 0);
	//@}
	/// @name get/set VM specific parameter
	//@{
	int get_parami(enumParamiId id) const;
	void set_parami(enumParamiId id, int val);
	void *get_paramv(enumParamvId id) const;
	void set_paramv(enumParamvId id, void *val);
	//@}
	/// @name misc
	//@{
	const _TCHAR *application_path() const;
	const _TCHAR *initialize_path() const;
	bool get_pause(int idx) const;
	void set_pause(int idx, bool val);
	void get_edition_string(char *buffer, size_t buffer_len) const;
	//@}

#ifdef USE_DEBUGGER
protected:
	// debugger
	enum en_device_name_map {
		DNM_KEYBOARD = 0,
		DNM_CRTC,
		DNM_PIA,
		DNM_ACIA,
		DNM_FDC3,
		DNM_FDC5,
		DNM_CMT,
		DNM_TIMER,
		DNM_FDDUNIT,
		DNM_BOARD,
		DNM_PSG60,
		DNM_PSG61,
		DNM_PSG90,
		DNM_PSG91,
		DNM_PSG92,
		DNM_PIAEX,
		DNM_ACIAEX,
		DNM_VIA,
		DNM_PIAEX2,
		DNM_RTC,
		DNM_MSM58321,
		DNM_EVENT
	};
	static const struct st_device_name_map {
		const _TCHAR *name;
		uint32_t        num;
	} c_device_names_map[];

public:
	// debugger
	int get_cpus() const;
	DEVICE *get_cpu(int index);
	DEVICE *get_memory(int index);

	bool get_debug_device_name(const _TCHAR *param, uint32_t *num, int *idx, const _TCHAR **name);
	void get_debug_device_names_str(_TCHAR *buffer, size_t buffer_len);
	bool debug_write_reg(uint32_t num, uint32_t reg_num, uint32_t data);
	bool debug_write_reg(uint32_t num, const _TCHAR *reg, uint32_t data);
	void debug_regs_info(uint32_t num, _TCHAR *buffer, size_t buffer_len);
#endif

};

#endif /* BML3MK5_H */
