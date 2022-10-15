/** @file qt_emu.h

	Skelton for retropc emulator
	SDL edition

	@author Sasaji
	@date   2012.02.21

	@brief [ sdl emulation i/f ]

	@note
	This code is based on the Common Source Code Project.
	Original Author : Takeda.Toshiya
*/

#ifndef QT_EMU_H
#define QT_EMU_H

#include "../../common.h"
#include "../../emu.h"
#include "../../vm/vm.h"
#include "../../logging.h"
#include "../windowmode.h"
#include "qt_screenmode.h"
#include <QDateTime>
#ifdef USE_JOYSTICK
#include <QtGamepad/QGamepad>
#endif

#define SDL_USEREVENT_COMMAND (SDL_USEREVENT + 1)

//class QPixelFormat;
class QImage;
class QPainter;
class QKeyEvent;
class MyAudioOutput;
class QOpenGLWidget;
class QOpenGLContext;
class QMainWindow;

class FIFO;
class FILEIO;
class GUI;
#ifdef USE_LEDBOX
class LedBox;
#endif
#ifdef USE_MESSAGE_BOARD
class MsgBoard;
#endif
class CSurface;
class CPixelFormat;
class REC_VIDEO;
class REC_AUDIO;
#ifdef USE_OPENGL
class COpenGL;
class COpenGLTexture;
#endif

/**
	@brief emulation i/f
*/
class EMU_OSD : public EMU
{
protected:
	QMainWindow *main_window;
private:
	// ----------------------------------------
	// input
	// ----------------------------------------
	/// @name input private methods
	//@{
	void EMU_INPUT();
	void initialize_input();
	void release_input();
	void update_input();

	void initialize_joystick();
	void release_joystick();

#ifdef USE_MOUSE_ABSOLUTE
//	void set_mouse_position();
#endif
	//@}
	/// @name input private members
	//@{
	bool pressed_global_key;

#ifdef USE_JOYSTICK
	int joy_xmin[2], joy_xmax[2];
	int joy_ymin[2], joy_ymax[2];
	QGamepadManager *joymgr;
	QGamepad *joy[2];
#endif
	//@}

private:
	// ----------------------------------------
	// screen
	// ----------------------------------------
	/// @name screen private methods
	//@{
	void EMU_SCREEN();
	void initialize_screen();
	void release_screen();

#ifdef USE_OPENGL
	void initialize_opengl();
	void release_opengl();

	void create_opengl_texture();
	void release_opengl_texture();

	void set_opengl_attr();
	void set_opengl_poly(int width, int height);
#endif /* USE_OPENGL */
	//@}
	/// @name screen private members
	//@{
	// screen settings
	QRect  *screen;
#ifdef USE_OPENGL
	COpenGL *opengl;
#endif
	uint32_t screen_flags;

	VmSize mixed_max_size;	// for OpenGL

	// screen buffer
	scrntype* lpBmp;

	VmRectWH reMix;
	VmRectWH reSuf;

#ifdef USE_OPENGL
	COpenGLTexture *texGLMixed;
//	GLuint mix_texture_name;
	VmRect	rePyl;
	float src_tex_l, src_tex_t, src_tex_r, src_tex_b;
	float src_pyl_l, src_pyl_t, src_pyl_r, src_pyl_b;
	int use_opengl;
	uint8_t next_use_opengl;
#endif
#ifdef USE_LEDBOX
	LedBox *ledbox;
#endif
	//@}

	// ----------------------------------------
	// sound
	// ----------------------------------------
	/// @name sound private methods
	//@{
	void EMU_SOUND();
	void initialize_sound(int rate, int samples, int latency);
	void initialize_sound();
	void release_sound();
	void release_sound_on_emu_thread();
	void start_sound();
	void end_sound();
	//@}
	/// @name sound private members
	//@{
	MyAudioOutput *sound;

	// direct sound
	uint32_t sound_prev_time;
	//@}

	// ----------------------------------------
	// timer
	// ----------------------------------------
	/// @name timer private methods
	//@{
	void update_timer();
	//@}
	/// @name timer private members
	//@{
	QDateTime sTime;
	//@}

	// ----------------------------------------
	// socket
	// ----------------------------------------
#ifdef USE_SOCKET
	/// @name socket private methods
	//@{
	void EMU_SOCKET();
	void initialize_socket();
	void release_socket();
	void update_socket();
	//@}
	/// @name socket private members
	//@{
//	struct st_soc {
//		DEVICE *device;
//		bool is_tcp;
//		int socket_delay;
//		char *recv_buffer;
//		int recv_r_ptr, recv_w_ptr;
//		bool is_server;
//	} socs[SOCKET_MAX];

//	int socket_nums;
	//@}
#endif

	// ----------------------------------------
	// uart
	// ----------------------------------------
#ifdef USE_UART
	/// @name uart private methods
	//@{
	void EMU_UART();
	void initialize_uart();
	void release_uart();
	void update_uart();
	//@}
#endif

//
//
//
public:
	// ----------------------------------------
	// initialize
	// ----------------------------------------
	/// @name initialize
	//@{
	EMU_OSD(const _TCHAR *new_app_path, const _TCHAR *new_ini_path, const _TCHAR *new_res_path);
	~EMU_OSD();
	//@}

	// ----------------------------------------
	// for windows
	// ----------------------------------------
	/// @name screen menu for ui
	//@{
	void change_screen_mode(int mode);
	void capture_screen();
	bool start_rec_video(int type, int fps_no, bool show_dialog);
	void record_rec_video();
#ifdef USE_OPENGL
	void change_screen_use_opengl(int num);
	void change_opengl_attr();
	int now_use_opengl() const {
		return use_opengl;
	}
#endif
	//@}
	/// @name sound menu for ui
	//@{
	void update_sound();
	//@}

	//
	/// @name input device procedures for host machine
	//@{
    int  key_down_up_(uint8_t type, int code, uint32_t vk_key, uint32_t scan_code, uint32_t mod);
	int  key_down(int code, bool keep_frames);
	void key_up(int code, bool keep_frames);
	uint8_t translate_keysym_(uint8_t type, int code, uint32_t vk_key, uint32_t scan_code, uint32_t mod, int *new_code, bool *new_keep_frames = nullptr);

	void reset_joystick();
	void update_joystick();

	void update_mouse();
	void mouse_down_up(uint8_t type, int buttons, int x, int y);
	void enable_mouse(int mode);
	void disable_mouse(int mode);
	//@}
	/// @name screen device procedures for host machine
	//@{
	void resume_window_placement();
	bool create_screen(int disp_no, int x, int y, int width, int height, uint32_t flags);
	void set_display_size(int width, int height, int power, bool now_window);
	void draw_screen();
	bool mix_screen();
	void update_screen_pa(QPainter *painter);
	void update_screen_gl(QOpenGLContext *context);
	void need_update_screen();
# ifdef USE_OPENGL
	void realize_opengl(QOpenGLWidget *widget);
	void set_mode_opengl(QOpenGLWidget *widget, int w, int h);
	void set_interval_opengl();
#endif

	void set_window(int mode, int cur_width, int cur_height);
	bool create_offlinesurface();

#ifdef USE_OPENGL
	int  get_use_opengl() const {
		return use_opengl;
	}
	void set_use_opengl(int val) {
		use_opengl = val;
	}
#endif
	void set_qt_window(QMainWindow *window);
	QMainWindow *get_qt_window();

	//@}
	/// @name sound device procedures for host machine
	//@{
	void mute_sound(bool mute);
	//@}
#ifdef USE_SOCKET
	/// @name socket device procedures for host machine
	//@{
	const void *get_socket(int ch) const;
	void socket_connected(int ch);
	void socket_disconnected(int ch);
	void socket_writeable(int ch);
	void socket_readable(int ch);
	void socket_accepted(int ch, int new_ch, bool tcp);
	void send_data_(int ch, const _TCHAR *hostname = nullptr, int port = 0);
	void recv_data(int ch);
	//@}
#endif
#ifdef USE_UART
	/// @name uart device procedures for host machine
	//@{
	int  enum_uarts();
	void get_uart_description(int ch, _TCHAR *buf, size_t size);
	//@}
#endif

	// ----------------------------------------
	// for virtual machine
	// ----------------------------------------
	/// @name screen for vm
	//@{
	scrntype* screen_buffer(int y);
	int screen_buffer_offset();
	//@}
	/// @name sound for vm
	//@{
	void lock_sound_buffer();
	void unlock_sound_buffer();
	void record_rec_sound(int32_t *buffer, int samples);
	//@}
	/// @name timer for vm
	//@{
	void get_timer(int *time, size_t size) const;
	//@}
#ifdef USE_SOCKET
	/// @name socket for vm
	//@{
	bool init_socket_tcp(int ch, DEVICE *dev, bool server = false);
	bool init_socket_udp(int ch, DEVICE *dev, bool server = false);
	bool connect_socket(int ch, const _TCHAR *hostname, int port, bool server = false);
	bool is_connecting_socket(int ch);
	void disconnect_socket(int ch);
	int  get_socket_channel();
	bool listen_socket(int ch);
	void send_data_tcp(int ch);
	void send_data_udp(int ch, const _TCHAR *hostname, int port);
	void send_data(int ch);
	//@}
#endif
#ifdef USE_UART
	/// @name uart for vm
	//@{
	bool init_uart(int ch, DEVICE *dev);
	bool open_uart(int ch);
	bool is_opened_uart(int ch);
	void close_uart(int ch);
	int  send_uart_data(int ch, const uint8_t *data, int size);
	void send_uart_data(int ch);
	void recv_uart_data(int ch);
	//@}
#endif
	/// @name send message to ui from vm
	//@{
	void update_ui(int flags);
	//@}
	/// @name for debug
	//@{
	void sleep(uint32_t ms);
	//@}
};

#endif /* QT_EMU_H */
