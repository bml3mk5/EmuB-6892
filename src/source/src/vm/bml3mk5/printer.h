/** @file printer.h

	HITACHI BASIC MASTER LEVEL3 Mark5 Emulator 'EmuB-6892'
	Skelton for retropc emulator

	@author Sasaji
	@date   2011.06.16 -

	@brief [ printer ]
*/

#ifndef PRINTER_H
#define PRINTER_H

#include "../vm_defs.h"
#include "../device.h"

#undef USE_PRINTER_PENDSIZE
#define PRNDATA_BUFFER_SIZE	0x10000
#define PRNDATA_BUFFER_SIZE_MAX	(PRNDATA_BUFFER_SIZE * 16)

class EMU;
class FILEIO;
class FIFOBYTE;

/**
	@brief printer
*/
class PRINTER : public DEVICE
{
public:
	/// @brief signals on PRINTER
	enum SIG_PRINTER_IDS {
		SIG_PRINTER_CLOCK	= 1,
		SIG_PRINTER_PIA_PB	= 2,
		SIG_PRINTER_PIA_CB2	= 3,
		SIG_PRINTER_DISCONNECT	= 11
	};

private:
	DEVICE* d_ctrl;

	int cfg_num;

	int id_cb1;

	FIFOBYTE *data_buffer;
	bool   buffer_overflow;

	FILEIO* fio;
	bool  send;
	bool strobe;
	uint8_t busdata;

	int   register_id;
	int   register_id_2;

	int   client_ch;
	int   connect;
	int   send_type;
	int   send_size;
#ifdef USE_PRINTER_PENDSIZE
	int   pend_size;
#endif
	uint8_t *send_buff;
	int   send_retry;
	uint8_t recv_buff[4];

	//for resume
#pragma pack(1)
	struct vm_state_st {
		int   register_id;

		char reserved[12];
	};
#pragma pack()

	void  cancel_my_event(int &id);
	void  register_my_event(double wait, int &id);

	void  set_data(uint32_t data);
	void  save_image();
	bool  connect_socket();
	void  disconnect_socket();

public:
	PRINTER(VM* parent_vm, EMU* parent_emu, const char* identifier, int config_num) : DEVICE(parent_vm, parent_emu, identifier) {
		set_class_name("PRINTER");
		d_ctrl = NULL;
		cfg_num = config_num;
	}

	~PRINTER() {}

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
	void set_context_cb1(int id) {
		id_cb1 = id;
	}
	int  get_buffer_size() const;
	uint8_t* get_buffer() const;
	bool set_direct_mode();
	bool save_printer(const _TCHAR* filename);
	void close_printer();
	bool print_printer();
	void toggle_printer_online();
	void network_connected(int ch);
	void network_disconnected(int ch);
	void network_writeable(int ch);
	void network_readable(int ch);
	uint8_t* get_sendbuffer(int ch, int* size, int* flags);
	void inc_sendbuffer_ptr(int ch, int size);
	uint8_t* get_recvbuffer0(int ch, int* size0, int* size1, int* flags);
	uint8_t* get_recvbuffer1(int ch);
	void inc_recvbuffer_ptr(int ch, int size);

	// event callback
	void event_frame();
	void event_callback(int event_id, int err);

	void save_state(FILEIO *fp);
	bool load_state(FILEIO *fp);
};

#endif /* PRINTER_H */
