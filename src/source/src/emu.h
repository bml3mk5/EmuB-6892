/** @file emu.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2017.11.01 -

	@brief [ emulation i/f ]

*/

#ifndef EMU_H
#define EMU_H

#include "vm/vm.h"
#include "logging.h"
#include "cchar.h"
#include "osd/windowmode.h"
#include "osd/screenmode.h"
#include "msgs.h"

#ifdef USE_FD1
#include "osd/d88_files.h"
#endif

#ifndef SCREEN_WIDTH_ASPECT
#define SCREEN_WIDTH_ASPECT SCREEN_WIDTH
#endif
#ifndef SCREEN_HEIGHT_ASPECT
#define SCREEN_HEIGHT_ASPECT SCREEN_HEIGHT
#endif
#ifndef WINDOW_WIDTH
#define WINDOW_WIDTH SCREEN_WIDTH_ASPECT
#endif
#ifndef WINDOW_HEIGHT
#define WINDOW_HEIGHT SCREEN_HEIGHT_ASPECT
#endif

#define USE_SCREEN_MIX_SURFACE

#define KEY_STATUS_SIZE 512

#ifdef USE_MEDIA
#define MEDIA_MAX 64
#endif

#ifdef USE_SOCKET
#define SOCKET_MAX 6
#define SOCKET_BUFFER_MAX 0x1000
#endif

#ifdef USE_UART
#define UART_MAX_PORTS	9
#define UART_BUFFER_MAX 0x400
#endif

#ifdef USE_DEBUGGER
#include "debugger_defs.h"
#endif

class FIFOINT;
class FILEIO;
class GUI;
//class LedBox;
#ifdef USE_MESSAGE_BOARD
class MsgBoard;
#endif
class CSurface;
class CPixelFormat;
class REC_VIDEO;
class REC_AUDIO;
class ScreenMode;
class WindowMode;
#ifdef USE_DEBUGGER
class DebuggerThread;
class DebuggerStorage;
class DebuggerSocket;
#endif

/// @ingroup Enums
/// key_mod
enum EnumKeyModFlags {
	KEY_MOD_ALT_KEY = 0x0001,
	KEY_MOD_SHIFT_KEY = 0x0002,
};

/// @ingroup Enums
/// vm_key_status
enum EnumVmKeyStatusFlags {
	VM_KEY_STATUS_KEYBOARD = 0x01,
	VM_KEY_STATUS_AUTOKEY = 0x08,
	VM_KEY_STATUS_VKEYBOARD = 0x10,
	VM_KEY_STATUS_KEYREC = 0x20,
	VM_KEY_STATUS_KEY2JOY = 0x80,
	VM_KEY_STATUS_MASK = 0x7f,
};

/// @ingroup Enums
/// ScreenMode
enum EnumScreenMode {
	NOW_WINDOW = 0,
	NOW_FULLSCREEN = 1,
	NOW_MAXIMIZE = 2,
};

/**
	@brief emulation i/f
*/
class EMU : public LogMessageReceiver
{
protected:
	/// @name related class
	//@{
	VM* vm;
	GUI *gui;
	//@}
	/// @name initialization protected members
	//@{
	bool initialized;
	//@}

	// ----------------------------------------
	// input
	// ----------------------------------------
	/// @name input protected methods
	//@{
	void EMU_INPUT();
	virtual void initialize_input();
	virtual void release_input();
	virtual void update_input();

	virtual void initialize_joystick();
	virtual void release_joystick();

	virtual void initialize_mouse(bool enable);
//#ifdef USE_MOUSE_ABSOLUTE
//	virtual void set_mouse_position();
//#endif

	virtual int  translate_global_key(int code);
	//@}
	/// @name input protected members
	//@{
#ifdef USE_KEYCODE_CONV
	uint8_t keycode_conv[KEY_STATUS_SIZE];
#endif
	/// @brief emu key code status (KEYCODE_**)
	///
	/// key code mapping (b7: on/off b0-b6: count down until release)
	uint8_t key_status[KEY_STATUS_SIZE];

	/// @brief vm key scan code mapping (provided from vm)
	int vm_key_map[KEY_STATUS_SIZE];

	/// @brief vm key scan code on/off status
	///
	/// bit0: set/reset on keyboard
	/// bit3: set/reset on auto key
	/// bit4: set/reset on virtual key
	/// @see EnumVmKeyStatusFlags
	uint8_t *vm_key_status;
	/// @brief size of vm_key_status
	int   vm_key_status_size;

	/// @brief vm key buffer (store keycode recently)
	FIFOINT* vm_key_history;
#ifdef USE_SHIFT_NUMPAD_KEY
	uint8_t key_converted[KEY_STATUS_SIZE];
	bool key_shift_pressed, key_shift_released;
#endif
	bool lost_focus;

	/// @brief modifier keys
	///
	/// @see EnumKeyModFlags
	int  key_mod;

#ifdef USE_JOYSTICK
	uint32_t joy_status[2];	// joystick #1, #2 (b0 = up, b1 = down, b2 = left, b3 = right, b4-b27 = trigger #1-#24
	int joy_num;
	bool joy_enabled[2];
	bool use_joystick;
#endif

	int mouse_status[3];	// x, y, button (b0 = left, b1 = right)
	int mouse_disabled;
#ifdef USE_MOUSE_ABSOLUTE
	VmPoint mouse_position;
#endif

#ifdef USE_AUTO_KEY
	FIFOINT* autokey_buffer;
	int autokey_phase, autokey_shift;
	uint16_t autokey_code;
	bool autokey_enabled;
#endif
	//@}

	// ----------------------------------------
	// screen
	// ----------------------------------------
	/// @name screen protected methods
	//@{
	void EMU_SCREEN();
	virtual void initialize_screen();
	virtual void release_screen();
	void release_screen_on_emu_thread();
	void set_vm_display_size();
	int  adjust_y_position(int h, int y);
	//@}
	/// @name screen private members
	//@{
	// screen settings
	CPixelFormat *pixel_format;

	CSurface *sufOrigin;

#ifdef USE_SCREEN_ROTATE
	// rotate buffer
	CSurface *sufRotate;
#endif

#ifdef USE_SMOOTH_STRETCH
	// stretch buffer
	CSurface *sufStretch1;
	CSurface *sufStretch2;
#endif
	CSurface *sufSource;
#ifdef USE_SCREEN_MIX_SURFACE
	CSurface *sufMixed;
#endif

	CMutex *mux_update_screen;
	bool screen_changing;

	/// vm draw area
	VmRectWH screen_size;
	VmSize screen_aspect_size;
	/// display size (window client size)
	VmSize display_size;
	/// margin in window client area (window mode only)
	VmRect display_margin;
	/// desktop size on the current monitor
	VmSize desktop_size;
	int desktop_bpp;
	bool screen_size_changed;

	VmRectWH rec_video_size[2];
	VmRectWH rec_video_stretched_size;

	/// vm draw area
	/// if use screen rotation, swap width and height.
	VmRectWH source_size;
	VmSize source_aspect_size;
	VmRectWH stretched_size;
#ifdef USE_SCREEN_ROTATE
	VmMatrix rotate_matrix[4];
#endif
#ifdef USE_SMOOTH_STRETCH
	VmSize stretch_power;
	bool stretch_screen;
#endif
	VmPoint stretched_dest_real;
	VmRectWH mixed_size;
	VmSize mixed_ratio;

	// update flags
#define DISABLE_SURFACE	1
	int  disable_screen;

	bool first_invalidate;
	bool self_invalidate;
	bool skip_frame;
	bool skip_frame_pre;

	// screen mode
	ScreenMode screen_mode;

	int window_mode_power;
	WindowMode window_mode;

	bool now_resizing;
	EnumScreenMode now_screenmode;
	int  prev_window_mode;
	EnumScreenMode prev_screenmode;
	VmPoint window_dest;			///< backup window position if go fullscreen
	bool first_change_screen;

	// record video
	REC_VIDEO *rec_video;
	bool *now_recording_video;
	//@}

	// ----------------------------------------
	// sound
	// ----------------------------------------
	/// @name sound private methods
	//@{
	void EMU_SOUND();
	virtual void initialize_sound(int rate, int samples, int latency);
	virtual void initialize_sound();
	virtual void release_sound();
	virtual void release_sound_on_emu_thread();
	virtual void start_sound();
	virtual void end_sound();
	//@}
	/// @name sound private members
	//@{
	int sound_rate, sound_samples;
	int sound_latency_half;
	bool sound_ok, sound_started, sound_finished, now_mute;

	// record sound
	REC_AUDIO *rec_audio;
	//@}

	// ----------------------------------------
	// floppy disk
	// ----------------------------------------
#ifdef USE_FD1
	/// @name floppy disk private methods
	//@{
	void initialize_disk_insert();
	void update_disk_insert();
	//@}
#endif
	/// @name floppy disk private members
	//@{
#ifdef USE_FD1
	typedef struct {
		_TCHAR path[_MAX_PATH];
		int offset;
		int wait_count;
		uint32_t flags;
	} disk_insert_t;
	disk_insert_t disk_insert[MAX_DRIVE];

	// d88 bank switch
	D88Files d88_files;
#endif

#define OPEN_DISK_FLAGS_READ_ONLY		0x0001
#define OPEN_DISK_FLAGS_MULTI_VOLUME	0x0010
#define OPEN_DISK_FLAGS_LAST_VOLUME		0x0008
#define OPEN_DISK_FLAGS_FORCELY			0x1000
	//@}


	// ----------------------------------------
	// media
	// ----------------------------------------
#ifdef USE_MEDIA
	/// @name media private methods
	//@{
	void initialize_media();
	void release_media();
	//@}
	/// @name media private members
	//@{
	_TCHAR media_path[MEDIA_MAX][_MAX_PATH];
	int media_cnt;
	bool media_playing;
	//@}
#endif

	// ----------------------------------------
	// timer
	// ----------------------------------------
	/// @name timer private methods
	//@{
	virtual void update_timer() {}
	//@}

	// ----------------------------------------
	// socket
	// ----------------------------------------
#ifdef USE_SOCKET
	/// @name socket private methods
	//@{
	void EMU_SOCKET() {}
	virtual void initialize_socket() {}
	virtual void release_socket() {}
	virtual void update_socket() {}
	//@}
#endif

	// ----------------------------------------
	// uart
	// ----------------------------------------
#ifdef USE_UART
	/// @name uart private methods
	//@{
	void EMU_UART() {}
	virtual void initialize_uart() {}
	virtual void release_uart() {}
	virtual void update_uart() {}
	//@}
#endif

	// ----------------------------------------
	// debugger
	// ----------------------------------------
#ifdef USE_DEBUGGER
	/// @name debugger
	//@{
	void EMU_DEBUGGER();
	void initialize_debugger();
	void release_debugger();
	DebuggerThread *hDebuggerThread;
	debugger_thread_t debugger_thread_param;
	DebuggerStorage *debugger_storage;
	DebuggerSocket *debugger_socket;
	//@}
#endif

	// ----------------------------------------
	// misc
	// ----------------------------------------
	/// @name misc protected methods
	//@{
#ifdef USE_EMU_INHERENT_SPEC
	uint64_t update_led();
#endif
	//@}
	/// @name misc protected members
	//@{
	CTchar app_path;
	CTchar ini_path;
	CTchar res_path;

#define VM_SYSPAUSE_MASK 0x01
#define VM_SUSPEND_MASK  0x02
#define VM_POWEROFF_MASK 0x04
#define VM_USRPAUSE_MASK 0x10

	int vm_pause;

	int parami[VM::ParamiUnknown];
	void *paramv[VM::ParamvUnknown];

#ifdef USE_MESSAGE_BOARD
	MsgBoard *msgboard;
#endif
	//@}

//
//
//
public:
	// ----------------------------------------
	// initialize
	// ----------------------------------------
	/// @name initialize
	//@{
	EMU(const _TCHAR *new_app_path, const _TCHAR *new_ini_path, const _TCHAR *new_res_path);
	virtual ~EMU();
	void initialize();
	void release();
	void release_on_emu_thread();
	//@}

	// ----------------------------------------
	// misc
	// ----------------------------------------
	/// @name misc
	//@{
	void    application_path(char *path, int len) const;
	const _TCHAR *application_path() const;
	void    initialize_path(char *path, int len) const;
	const _TCHAR *initialize_path() const;
	void    resource_path(char *path, int len) const;
	const _TCHAR *resource_path() const;
#if defined(_UNICODE)
	void    application_path(wchar_t *path, int len) const;
	void    initialize_path(wchar_t *path, int len) const;
	void    resource_path(wchar_t *path, int len) const;
#endif

	bool get_pause(int idx) const;
	void set_pause(int idx, bool val);
	int *get_pause_ptr(void);
	void set_gui(GUI *new_gui);
	GUI *get_gui();

	int get_parami(int id) const;
	void set_parami(int id, int val);
	void *get_paramv(int id) const;
	void set_paramv(int id, void *val);
	void update_params();
	//@}

	// ----------------------------------------
	// for windows
	// ----------------------------------------
	/// @name drive virtual machine
	//@{
	int frame_interval();
	double get_frame_rate();
	int run(int split_num);
	void update();
	bool now_skip();
	//@}
	// user interface
	/// @name control menu for ui
	//@{
	void update_config();
	void reset();
#ifdef USE_SPECIAL_RESET
	void special_reset();
	void warm_reset(int onoff);
#endif
	void change_dipswitch(int num);
#ifdef USE_POWER_OFF
	void notify_power_off();
#endif
	void change_cpu_power(int num);
#ifdef USE_EMU_INHERENT_SPEC
	void change_sync_irq();
#endif
#ifdef USE_AUTO_KEY
	virtual bool start_auto_key(const char *str);
	virtual bool open_auto_key(const _TCHAR *path);
	virtual void stop_auto_key();
	bool now_auto_key() {
		return autokey_enabled;
	}
#endif
#ifdef USE_EMU_INHERENT_SPEC
	bool save_state(const _TCHAR* file_path);
	bool load_state(const _TCHAR* file_path);
#ifdef USE_KEY_RECORD
	bool play_reckey(const _TCHAR* file_path);
	bool record_reckey(const _TCHAR* file_path);
	void stop_reckey(bool stop_play = true, bool stop_record = true);
#endif
#endif
	//@}
#ifdef USE_FD1
	/// @name floppy disk menu for ui
	//@{
	int  open_disk_main(int drv, const _TCHAR* file_path, int offset, uint32_t flags);
	bool open_disk(int drv, const _TCHAR* file_path, int bank_num, int offset, uint32_t flags);
	void update_disk_info(int drv, const _TCHAR* file_path, int bank_num);
	bool open_disk_by_bank_num(int drv, const _TCHAR* file_path, int bank_num, uint32_t flags, bool multiopen);
	bool open_disk_with_sel_bank(int drv, int bank_num, uint32_t flags = 0);
	void close_disk(int drv, uint32_t flags = 0);
	int  change_disk(int drv);
	int  get_disk_side(int drv);
	void toggle_disk_write_protect(int drv);
	bool disk_write_protected(int drv);
	bool disk_inserted(int drv);
	bool changed_cur_bank(int drv);
	D88File *get_d88_file(int drv) { return &d88_files.GetFile(drv); }
	bool create_blank_floppy_disk(const _TCHAR* file_path, uint8_t type);
	bool is_same_disk(int drv, const _TCHAR *file_path, int bank_num);
	//@}
#endif
#ifdef USE_DATAREC
	/// @name tape menu for ui
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
#ifdef USE_DATAREC_BUTTON
	void push_play();
	void push_stop();
#endif
#ifdef USE_CART1
	void open_cart(int drv, const _TCHAR* file_path);
	void close_cart(int drv);
#endif
#ifdef USE_QD1
	void open_quickdisk(int drv, const _TCHAR* file_path);
	void close_quickdisk(int drv);
	void toggle_quickdisk_write_protect(int drv);
#endif
#ifdef USE_MEDIA
	void open_media(const _TCHAR* file_path);
	void close_media();
#endif
#ifdef USE_BINARY_FILE1
	void load_binary(int drv, const _TCHAR* file_path);
	void save_binary(int drv, const _TCHAR* file_path);
	void close_binary(int drv);
#endif
	/// @name screen menu for ui
	//@{
	virtual void change_screen_mode(int mode);
	virtual void change_maximize_window(int width, int height, bool maximize);
	void change_stretch_screen(int num);
	int get_window_mode_count() const;
	int get_display_device_count() const;
	int get_screen_mode_count(int disp_no) const;
	bool is_fullscreen() const;
	const CWindowMode *get_window_mode(int num) const;
	const CDisplayDevice *get_display_device(int disp_no) const;
	const CVideoMode *get_screen_mode(int disp_no, int num) const;
	VmRectWH *get_screen_record_size(int num);
	void change_pixel_aspect(int mode);
	int  get_pixel_aspect_count();
	int  get_pixel_aspect(int mode, int *wratio, int *hratio);
	virtual void capture_screen();
	virtual bool start_rec_video(int type, int fps_no, bool show_dialog);
	void stop_rec_video();
	void restart_rec_video();
	virtual void record_rec_video();
	void resize_rec_video(int num);
	void change_rec_video_size(int num);
	int  get_rec_video_fps_num();
	bool now_rec_video();
	bool rec_video_enabled(int type);
	const _TCHAR **get_rec_video_codec_list(int type);
	const CMsg::Id *get_rec_video_quality_list(int type);
#ifdef USE_SCANLINE
	void change_screen_scanline(int num);
#endif
#ifdef USE_DIRECT3D
	virtual void change_screen_use_direct3d(int num) {}
#endif
#ifdef USE_OPENGL
	virtual void initialize_opengl() {}
	virtual void release_opengl() {}
	virtual void change_screen_use_opengl(int num) {}
	virtual void change_opengl_attr() {}
	virtual int now_use_opengl() const { return 0; }
#endif
#ifdef USE_AFTERIMAGE
	void change_screen_afterimage(int num);
#endif
#ifdef USE_KEEPIMAGE
	void change_screen_keepimage(int num);
#endif
#ifdef USE_DIRECT3D
	virtual bool enabled_direct3d() const { return false; }
#endif
	//@}
	/// @name sound menu for ui
	//@{
	void start_rec_sound(int type);
	void stop_rec_sound();
	void restart_rec_sound();
	bool now_rec_sound();
	bool *get_now_rec_sound_ptr();
	bool rec_sound_enabled(int type);
	const _TCHAR **get_rec_sound_codec_list(int type);
	void set_volume(int volume);
	//@}
#ifdef USE_PRINTER
	/// @name printer menu for ui
	//@{
	bool save_printer(int dev, const _TCHAR *filename);
	void clear_printer(int dev);
	int  get_printer_buffer_size(int dev);
	uint8_t* get_printer_buffer(int dev);
	void enable_printer_direct(int dev);
	bool print_printer(int dev);
	void toggle_printer_online(int dev);
	//@}
#endif
#ifdef MAX_COMM
	/// @name comm menu for ui
	//@{
	void enable_comm_server(int dev);
	void enable_comm_connect(int dev, int num);
	bool now_comm_connecting(int dev, int num);
	void send_comm_telnet_command(int dev, int num);
	//@}
#endif
#ifdef USE_MESSAGE_BOARD
	/// @name options menu for ui
	//@{
	void show_message_board();
	int  is_shown_message_board();
	void change_use_joypad(int num);
	bool is_enable_joypad(int num);

	void save_keybind();

	void change_archtecture(int id, int num, bool reset);
	//@}
#endif

	//
	/// @name input device procedures for host machine
	//@{
	virtual int  key_down_up(uint8_t type, int code, long status);
	virtual int  key_down(int code, bool keep_frames);
	virtual void key_up(int code, bool keep_frames);
	void key_lost_focus() {
		lost_focus = true;
	}
	int  system_key_down(int code);
	void system_key_up(int code);
	bool execute_global_keys(int code, uint32_t status);
	bool release_global_keys(int code, uint32_t status);
	virtual void post_command_message(int id);
	virtual uint8_t translate_keysym(uint8_t type, int code, long status, int *new_code, bool *new_keep_frames = NULL);
	virtual uint8_t translate_keysym(uint8_t type, int code, short scan_code, int *new_code, bool *new_keep_frames = NULL);
	void clear_vm_key_map();
	void set_vm_key_map(uint32_t key_code, int vm_scan_code);
//	int get_vm_key_map(uint32_t key_code) const;

	void clear_vm_key_status(uint8_t mask);
	void vm_key_down(int code, uint8_t mask);
	void vm_key_up(int code, uint8_t mask);

	void vkey_key_down(int code, uint8_t mask);
	void vkey_key_up(int code, uint8_t mask);

#ifdef USE_BUTTON
	void press_button(int num);
#endif
	virtual void reset_joystick();
	virtual void update_joystick();

	virtual void update_mouse();
	virtual void enable_mouse(int mode);
	virtual void disable_mouse(int mode);
	void toggle_mouse();
	bool get_mouse_enabled();

#ifdef USE_AUTO_KEY
	virtual void parse_auto_key(const char *buf, int size, int &prev_code, int &prev_mod);
	virtual void parsed_auto_key(int prev_code, int prev_mod);
#endif
	virtual void update_autokey();
	virtual void enable_joystick();
	//@}
	/// @name screen device procedures for host machine
	//@{
	virtual void resume_window_placement();
	virtual bool create_screen(int disp_no, int x, int y, int width, int height, uint32_t flags) { return false; }
	virtual void set_display_size(int width, int height, int power, bool now_window);
	virtual void draw_screen();
	void fill_gray();
	virtual bool mix_screen();
#if defined(USE_WIN)
	virtual void update_screen(HDC hdc) {}
#else
	virtual void update_screen() {}
#endif
	void skip_screen(bool val) {
		skip_frame = val;
	}
	virtual void need_update_screen() {}

	void init_screen_mode();
	void enum_window_mode(int max_width, int max_height);
	void enum_screen_mode(uint32_t flags = 0);
	void find_screen_mode();
	virtual void set_window(int mode, int cur_width, int cur_height);
	void set_desktop_size(int width, int height, int bpp) {
		desktop_size.w = width;
		desktop_size.h = height;
		desktop_bpp = bpp;
	}
	void set_display_margin(int left, int top, int right, int bottom) {
		display_margin.left = left;
		display_margin.top = top;
		display_margin.right = right;
		display_margin.bottom = bottom;
	}
	void lock_screen();
	void unlock_screen();
#ifdef USE_BITMAP
	void reload_bitmap() {
		first_invalidate = true;
	}
#endif
	virtual bool create_offlinesurface() { return false; }
	const CPixelFormat *get_pixel_format() const {
		return pixel_format;
	}

#ifdef USE_OPENGL
	virtual int  get_use_opengl() const { return 0; }
	virtual void set_use_opengl(int val) {}
#endif
//	/// @return library specific value
//	virtual void *get_screen() { return NULL; }
//	/// @return library specific value
//	virtual void *get_window() { return NULL; }
	//@}
	/// @name sound device procedures for host machine
	//@{
	virtual void mute_sound(bool mute);
	virtual uint32_t adjust_sound_pos(uint32_t msec);
	//@}
#ifdef USE_SOCKET
	/// @name socket device procedures for host machine
	//@{
	virtual const void *get_socket(int ch) const { return NULL; }
	virtual void check_socket() {}
	virtual void socket_connected(int ch) {}
	virtual void socket_disconnected(int ch) {}
	virtual void socket_writeable(int ch) {}
	virtual void socket_readable(int ch) {}
	virtual void socket_accept(int ch) {}
	virtual void socket_accepted(int ch, int new_ch, bool tcp) {}
	virtual void send_data(int ch) {}
	virtual void recv_data(int ch) {}
	//@}
#endif
#ifdef USE_UART
	/// @name uart device procedures for host machine
	//@{
	virtual int  enum_uarts() { return 0; }
	virtual void get_uart_description(int idx, _TCHAR *buf, size_t size) {}
	//@}
#endif
#ifdef USE_DEBUGGER
	/// @name debugger procedures for host machine
	//@{
	void open_debugger();
	void close_debugger();
	bool debugger_enabled(int cpu_index);
	void debugger_terminal_accepted();
	DebuggerStorage *get_debugger_storage() { return debugger_storage; }
	DebuggerSocket *get_debugger_socket() { return debugger_socket; }
	bool now_debugging;

	bool debugger_save_image(int width, int height, CSurface *surface);
	//@}
#endif

	// ----------------------------------------
	// for virtual machine
	// ----------------------------------------

	// power off
	void power_off();
	void toggle_power_on_off();

	/// @name input device for vm
	//@{
	uint8_t* key_buffer() {
		return key_status;
	}
#ifdef USE_JOYSTICK
	uint32_t* joy_buffer() {
		return joy_status;
	}
#endif
	int* mouse_buffer() {
		return mouse_status;
	}
	int* get_key_mod_ptr() {
		return &key_mod;
	}
	virtual void recognized_key(uint16_t key_code);
	void set_vm_key_status_buffer(uint8_t *buffer, int size) {
		vm_key_status = buffer;
		vm_key_status_size = size;
	}
	void get_vm_key_status_buffer(uint8_t **buffer, int *size) {
		*buffer = vm_key_status;
		*size = vm_key_status_size;
	}
	FIFOINT *get_vm_key_history() const {
		return vm_key_history;
	}
	//@}
	/// @name screen for vm
	//@{
	virtual scrntype* screen_buffer(int y);
	virtual int screen_buffer_offset();
	void get_rgbformat(uint32_t *r_mask, uint32_t *g_mask, uint32_t *b_mask, uint8_t *r_shift, uint8_t *g_shift, uint8_t *b_shift);
	void get_rgbaformat(uint32_t *r_mask, uint32_t *g_mask, uint32_t *b_mask, uint32_t *a_mask, uint8_t *r_shift, uint8_t *g_shift, uint8_t *b_shift, uint8_t *a_shift);
	void get_rgbcolor(scrntype pixel, uint8_t *r, uint8_t *g, uint8_t *b);
	scrntype map_rgbcolor(uint8_t r, uint8_t g, uint8_t b);
	bool now_skip_frame();
	//@}
	/// @name sound for vm
	//@{
	virtual void lock_sound_buffer() {}
	virtual void unlock_sound_buffer() {}
//	void record_rec_sound(uint8_t *buffer, int samples);
//	void record_rec_sound(int16_t *buffer, int samples);
	void record_rec_sound(int32_t *buffer, int samples);
	//@}
	/// @name timer for vm
	//@{
	virtual void get_timer(int *time, size_t size) const {}
	//@}
#ifdef USE_MEDIA
	/// @name media for vm
	//@{
	int media_count();
	void play_media(int trk);
	void stop_media();
	//@}
#endif
#ifdef USE_SOCKET
	/// @name socket for vm
	//@{
	virtual bool init_socket_tcp(int ch, DEVICE *dev, bool server = false) { return false; }
	virtual bool init_socket_udp(int ch, DEVICE *dev, bool server = false) { return false; }
	virtual bool connect_socket(int ch, uint32_t ipaddr, int port, bool server = false) { return false; }
	virtual bool connect_socket(int ch, const _TCHAR *hostname, int port, bool server = false) { return false; }
	virtual bool is_connecting_socket(int ch) { return false; }
//	virtual bool get_ipaddr(const _TCHAR *hostname, int port, uint32_t *ipaddr) { return false; }
	virtual void disconnect_socket(int ch) {}
	virtual int  get_socket_channel() { return -1; }
	virtual bool listen_socket(int ch) { return false; }
	virtual void send_data_tcp(int ch) {}
	virtual void send_data_udp(int ch, uint32_t ipaddr, int port) {}
	virtual void send_data_udp(int ch, const _TCHAR *hostname, int port) {}
	//@}
#endif
#ifdef USE_UART
	/// @name uart for vm
	//@{
	virtual bool init_uart(int ch, DEVICE *dev) { return false; }
	virtual bool open_uart(int ch) { return false; }
	virtual bool is_opened_uart(int ch) { return false; }
	virtual void close_uart(int ch) {}
	virtual int  send_uart_data(int ch, const uint8_t *data, int size) { return 0; }
	virtual void send_uart_data(int ch) {}
	virtual void recv_uart_data(int ch) {}
	//@}
#endif
	/// @name load rom image
	//@{
	static bool load_data_from_file(const _TCHAR *file_path, const _TCHAR *file_name
		, uint8_t *data, size_t size
		, const uint8_t *first_data = NULL, size_t first_data_size = 0
		, const uint8_t *last_data = NULL,  size_t last_data_size = 0);
	//@}

	/// @name send message to ui from vm
	//@{
	void update_ui(int flags);
	//@}
	/// @name string from vm
	//@{
	void get_edition_string(char *buffer, size_t buffer_size) const;
	//@}
	/// @name for debug
	//@{
	uint64_t get_current_clock();
	virtual void sleep(uint32_t ms);
	//@}

	/// @name disp message methods
	//@{
	void send_log_message(int level, const _TCHAR *levelstr, const _TCHAR *msg);

	void out_msg(const _TCHAR* msg, bool set, int sec);
	void out_msg(const _TCHAR* msg, bool set = true);
	void out_info(const _TCHAR* msg, bool set, int sec);
	void out_info(const _TCHAR* msg, bool set = true);
	void out_infof(const _TCHAR* format, ...);
	void out_infov(const _TCHAR* format, va_list ap);

	void out_msg_x(const _TCHAR* msg, bool set, int sec);
	void out_msg_x(const _TCHAR* msg, bool set = true);
	void out_info_x(const _TCHAR* msg, bool set, int sec);
	void out_info_x(const _TCHAR* msg, bool set = true);
	void out_infof_x(const _TCHAR* format, ...);
	void out_infoc_x(const _TCHAR* msg1, ...);

	void out_infov(CMsg::Id msg_id, va_list ap);

	void out_msg_x(CMsg::Id msg_id, bool set, int sec);
	void out_msg_x(CMsg::Id msg_id, bool set = true);
	void out_info_x(CMsg::Id msg_id, bool set, int sec);
	void out_info_x(CMsg::Id msg_id, bool set = true);
	void out_infof_x(CMsg::Id msg_id, ...);
	void out_infoc_x(CMsg::Id msg_id, ...);

#ifdef USE_MESSAGE_BOARD
	MsgBoard *get_msgboard() { return msgboard; }
#endif
	//@}
};

#ifdef USE_PERFORMANCE_METER
extern int gdPMvalue;
#endif

#endif /* EMU_H */
