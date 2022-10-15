/** @file comm.h

	HITACHI BASIC MASTER LEVEL3 Mark5 / MB-S1 Emulator 'EmuB-6892/EmuB-S1'
	Skelton for retropc emulator

	@author Sasaji
	@date   2011.08.20 -

	@brief [ comm ]
*/

#ifndef COMM_H
#define COMM_H

#include "../vm_defs.h"
//#include "../../emu.h"
#include "../device.h"
//#include "acia.h"

#define COMM_MAX_BUFF 320

class EMU;

/**
	@brief Communication (RS-232C)
*/
class COMM : public DEVICE
{
public:
	/// @brief signals on COMM
	enum SIG_COMM_IDS {
		SIG_COMM_RS	= 1,
	};

private:
	DEVICE *d_ctrl, *d_cmt;

	int cfg_num;

	int	is_rs;	// casette = 0 / rs-232c = 1
	uint16_t dipswitch;	// baud speed 
	bool through;	// send/receive 8bit on through mode 

//	uint8_t receive_data;
	bool received;
	uint8_t send_buff[COMM_MAX_BUFF];
	int send_buff_w_pos;
	int send_buff_r_pos;
	uint8_t recv_buff[COMM_MAX_BUFF];
	int recv_buff_w_pos;
	int recv_buff_r_pos;

	uint8_t cr;	// acia control register

	int register_id;

	int connect;
	int client_ch;
	int server_ch;
	int uart_ch;

	int send_telcmd_pos;

	//for resume
#pragma pack(1)
	struct vm_state_st {
		int register_id;
		int	is_rs;

		// version 2
		uint16_t dipswitch;
		uint8_t through;

		char reserved[5];
	};
#pragma pack()

	void register_my_event();
	void cancel_my_event();

	bool connect_socket();
	void disconnect_socket();

	bool connect_uart(int ch);
	void disconnect_uart();

	void disconnect_all();

public:
	COMM(VM* parent_vm, EMU* parent_emu, const char* identifier, int config_num) : DEVICE(parent_vm, parent_emu, identifier) {
		set_class_name("COMM");
		d_ctrl = NULL;
		d_cmt = NULL;
		cfg_num = config_num;
	}
	~COMM() {}

	// common functions
	void initialize();
	void reset();
	void release();

	void write_io8(uint32_t addr, uint32_t data);
	void write_signal(int id, uint32_t data, uint32_t mask);

	// unique functions
	void set_context_ctrl(DEVICE* device) {
		d_ctrl = device;
	}
	void set_context_cmt(DEVICE* device) {
		d_cmt = device;
	}

	void send_data(uint32_t data);
//	void recv_data(uint8_t *data, size_t size);

	void event_frame();
	void event_callback(int , int);

	void enable_server();
	void enable_connect(int num);
	bool now_connecting(int num);
	void send_telnet_command(int num);
	void network_connected(int ch);
	void network_disconnected(int ch);
	void network_writeable(int ch);
	void network_readable(int ch);
	void network_accepted(int ch, int new_ch);
	uint8_t* get_sendbuffer(int ch, int* size, int* flags);
	void inc_sendbuffer_ptr(int ch, int size);
	uint8_t* get_recvbuffer0(int ch, int* size0, int* size1, int* flags);
	uint8_t* get_recvbuffer1(int ch);
	void inc_recvbuffer_ptr(int ch, int size);

	void save_state(FILEIO *fp);
	bool load_state(FILEIO *fp);
};

#endif /* COMM_H */
