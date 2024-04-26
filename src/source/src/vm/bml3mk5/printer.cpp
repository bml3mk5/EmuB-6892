/** @file printer.cpp

	HITACHI BASIC MASTER LEVEL3 Mark5 Emulator 'EmuB-6892'
	Skelton for retropc emulator

	@author Sasaji
	@date   2011.06.16 -

	@brief [ printer ]
*/

#include "printer.h"
#include <stdlib.h>
#include "../../emu.h"
#include "../../fileio.h"
#include "../../fifo.h"
#include "../../config.h"
#include "../pia.h"


#define PRINTER_PEND_SIZE	64
//#define PRINTER_DELAY_TIME (CPU_CLOCKS/19200)
#define PRINTER_DELAY_SEC  (CPU_CLOCKS)

void PRINTER::cancel_my_event(int &id)
{
	if(id != -1) {
		cancel_event(this, id);
		id = -1;
	}
}

void PRINTER::register_my_event(double wait, int &id)
{
	cancel_my_event(id);
	register_event(this, SIG_PRINTER_CLOCK, wait, false, &id);
}

void PRINTER::initialize()
{
	fio = new FILEIO();
	data_buffer = new FIFOBYTE();
	buffer_overflow = false;
	send = false;
	strobe = false;
	busdata = 0;

	client_ch = -1;
	connect = 0;
	send_type = 0;
	send_size = 0;
#ifdef USE_PRINTER_PENDSIZE
	pend_size = 0;
#endif
	send_buff = NULL;
	memset(recv_buff, 0x00, sizeof(recv_buff));

	register_id = -1;
}

void PRINTER::reset()
{
	data_buffer->clear();
	buffer_overflow = false;
	strobe = false;
	busdata = 0;
}

void PRINTER::release()
{
	if (data_buffer) {
		delete data_buffer;
		data_buffer = NULL;
	}
	delete fio;
}

void PRINTER::write_io8(uint32_t addr, uint32_t data)
{
}

uint32_t PRINTER::read_io8(uint32_t addr)
{
	return 0;
}

void PRINTER::set_data(uint32_t data)
{
	if (data_buffer->write_pos() >= data_buffer->size()) {
		if (data_buffer->size() < PRNDATA_BUFFER_SIZE_MAX) {
			if (!data_buffer->allocate(data_buffer->size() + PRNDATA_BUFFER_SIZE)) {
				logging->out_logf(LOG_ERROR, _T("%s%d: Realloc failed."), this_class_name, cfg_num);
			}
		} else {
			buffer_overflow = true;
			logging->out_logf(LOG_ERROR, _T("%s%d: Buffer overflow."), this_class_name, cfg_num);
		}
	}
	if (buffer_overflow != true) {
		data_buffer->write(data & 0xff);
	}

#ifdef USE_SOCKET
	if (pConfig->printer_direct[cfg_num]) {
		if (!connect_socket()) {
			return;
		}
		// request sending printer data to server
		send_type = 2;
		send_size = 1;
		send_retry = 0;
		send_buff = data_buffer->data(data_buffer->write_pos()-1);
		if (connect == 3) {
			emu->send_data_tcp(client_ch);
#ifdef USE_PRINTER_PENDSIZE
			pend_size++;
#endif
		}
	}
#endif
//	logging->out_debugf("printer w %04x=%02x",addr,data);
}

void PRINTER::write_signal(int id, uint32_t data, uint32_t mask)
{
	// pia control signal
	switch(id) {
		case SIG_PRINTER_PIA_PB:
			busdata = data;
//			set_data(data);
			break;
		case SIG_PRINTER_PIA_CB2:
			if ((data & mask) == 0) {
				// receive strobe
				strobe = true;
				if (pConfig->printer_online[cfg_num]) {
					register_my_event(pConfig->printer_delay[cfg_num] * 1000.0, register_id);
//					logging->out_debugf("printer reg_id:%d i:%02d d:%02d m:%02d send:%s",register_id,id,data,mask,send ? "true" : "false");
				}
			}
			break;
	}

}

bool PRINTER::save_printer(const _TCHAR* filename)
{
	close_printer();

	if(fio->Fopen(filename, FILEIO::WRITE_BINARY)) {
		save_image();
	} else {
		return false;
	}
	close_printer();
	return true;
}

void PRINTER::close_printer()
{
	// close file
	fio->Fclose();
}

void PRINTER::save_image()
{
	uint8_t d[2];

	d[1]=0;
	// binary image
	for(int i=0; i<data_buffer->write_pos(); i++) {
		// reverse bit
		d[0] = data_buffer->peek(i);
		d[0] = ~d[0];
		fio->Fwrite(d, sizeof(uint8_t), 1);
	}
}

int PRINTER::get_buffer_size() const
{
	return data_buffer->write_pos();
}

uint8_t* PRINTER::get_buffer() const
{
	return data_buffer->data(0);
}

bool PRINTER::set_direct_mode()
{
	if (!pConfig->printer_direct[cfg_num]) {
		send_size = 0;
		send_buff = get_buffer();

		if (!connect_socket()) {
			return false;
		}
		pConfig->printer_direct[cfg_num] = true;

	} else {
		disconnect_socket();
		send_type = 0;
		pConfig->printer_direct[cfg_num] = false;

	}
	return true;
}

bool PRINTER::print_printer()
{
	if (send_type == 2) {
		return false;
	}

	send_type = 1;
	send_size = get_buffer_size();
	send_buff = get_buffer();

	if (!connect_socket()) {
		return false;
	}
#ifdef USE_SOCKET
	if (connect == 3) {
		// already connected and writeable to socket
		emu->send_data_tcp(client_ch);
#ifdef USE_PRINTER_PENDSIZE
		pend_size = 0;
#endif
	}
#endif
	return true;
}

/// toggle on/offline to printer
void PRINTER::toggle_printer_online()
{
	pConfig->printer_online[cfg_num] = !pConfig->printer_online[cfg_num];

	if (strobe && pConfig->printer_online[cfg_num]) {
		register_my_event(pConfig->printer_delay[cfg_num] * 1000.0, register_id);
	}
}

uint8_t* PRINTER::get_sendbuffer(int ch, int* size, int* flags)
{
	if (ch == client_ch) {
		*size = send_size;
		return send_buff;
	} else {
		*size = 0;
		return NULL;
	}
}
void PRINTER::inc_sendbuffer_ptr(int ch, int size)
{
	if (ch == client_ch) {
		if (send_type == 1) {
			send_size = (get_buffer_size() - size);
		} else if (send_type == 2) {
			if (size == 0) {
				// retry to send data later
				send_retry++;
				if (send_retry > 10) {
					// sending abort
					size = 1;
				}
			} else {
				send_retry = 0;
			}
			send_size = (1 - size);
		} else {
			send_size = 0;
		}
		send_buff += size;
		if (send_size <= 0) {
			if (send_type == 1) {
				register_event_by_clock(this, SIG_PRINTER_DISCONNECT, PRINTER_DELAY_SEC, false, &register_id_2);
				send_type = 0;
			}
			send_size = 0;
			send_buff = get_buffer();
		}
	}
}
uint8_t* PRINTER::get_recvbuffer0(int ch, int* size0, int* size1, int* flags)
{
	if (ch == client_ch) {
		*size0 = 4;
		*size1 = 0;
		return recv_buff;
	} else {
		*size0 = 0;
		*size1 = 0;
		return NULL;
	}
}

uint8_t* PRINTER::get_recvbuffer1(int ch)
{
	if (ch == client_ch) {
		return recv_buff;
	} else {
		return NULL;
	}
}

void PRINTER::inc_recvbuffer_ptr(int ch, int size)
{
#ifdef USE_PRINTER_PENDSIZE
	if (ch == client_ch) {
		if (size > 0 && pend_size >= PRINTER_PEND_SIZE && send_type == 2) {
			// receive ACK signal from mpprinter
			// and send ACK signal to computer
			register_my_event(1, register_id);
			pend_size = 0;
		}
	}
#endif
}

bool PRINTER::connect_socket()
{
#ifdef USE_SOCKET
	if (connect == 0) {
		// get empty socket
		client_ch = emu->get_socket_channel();
		if (client_ch < 0) {
			return false;
		}
		// connect
		if (!emu->init_socket_tcp(client_ch, this)) {
			logging->out_log(LOG_ERROR, _T("Network socket initialize failed."));
			return false;
		}
		if (!emu->connect_socket(client_ch, pConfig->printer_server_host[cfg_num].Get(), pConfig->printer_server_port[cfg_num])) {
			logging->out_log(LOG_ERROR, _T("Cannot connect to printer server."));
			return false;
		}
		connect |= 1;
	}
	return true;
#else
	return false;
#endif
}
void PRINTER::disconnect_socket()
{
#ifdef USE_SOCKET
	if (!pConfig->printer_direct[cfg_num]) {
		// disconnect
		emu->disconnect_socket(client_ch);
	}
#endif
}
void PRINTER::network_connected(int ch)
{
	if (ch == client_ch) {
		connect |= 1;
	}
}
void PRINTER::network_disconnected(int ch)
{
	if (ch == client_ch) {
		connect = 0;
		send_type = 0;
		client_ch = -1;
	}
}
void PRINTER::network_writeable(int ch)
{
	if (ch == client_ch) {
		connect |= 2;
	}
}
void PRINTER::network_readable(int ch)
{
}

// ----------------------------------------------------------------------------
// event handler
// ----------------------------------------------------------------------------

void PRINTER::event_frame()
{
}

void PRINTER::event_callback(int event_id, int err)
{
//	logging->out_debugf("printer event send:%s",send ? "true" : "false");
	switch(event_id) {
		case SIG_PRINTER_CLOCK:
			if (send_type == 2) {
				if (send_retry > 0) {
					// retry to send
#ifdef USE_SOCKET
					emu->send_data_tcp(client_ch);
#endif
					if (pConfig->printer_online[cfg_num]) {
						register_my_event(pConfig->printer_delay[cfg_num] * 1000.0, register_id);
					}
				} else {
#ifdef USE_PRINTER_PENDSIZE
					if (pend_size < PRINTER_PEND_SIZE)
#endif
					{
						// set data
						set_data(busdata);

						// send ACK signal
						d_ctrl->write_signal(id_cb1,1,1);
						d_ctrl->write_signal(id_cb1,0,1);

						cancel_my_event(register_id);
						send_type = 0;
						strobe = false;
					}
				}
			} else {
				// set data
				set_data(busdata);

				// send ACK signal
				d_ctrl->write_signal(id_cb1,1,1);
				d_ctrl->write_signal(id_cb1,0,1);

				cancel_my_event(register_id);
				strobe = false;
			}
			break;
		case SIG_PRINTER_DISCONNECT:
			disconnect_socket();
			cancel_event(this, register_id_2);
			register_id_2 = -1;
			break;
	}
}

// ----------------------------------------------------------------------------
void PRINTER::save_state(FILEIO *fp)
{
	struct vm_state_st vm_state;

	//
	vm_state_ident.version = Uint16_LE(1);
	vm_state_ident.size = Uint32_LE(sizeof(vm_state_ident) + sizeof(vm_state));

	// copy values
	memset(&vm_state, 0, sizeof(vm_state));
	vm_state.register_id = Int32_LE(register_id);

	fp->Fwrite(&vm_state_ident, sizeof(vm_state_ident), 1);
	fp->Fwrite(&vm_state, sizeof(vm_state), 1);
}

bool PRINTER::load_state(FILEIO *fp)
{
	vm_state_ident_t vm_state_i;
	struct vm_state_st vm_state;

	READ_STATE_CHUNK(fp, vm_state_i, vm_state);

	// copy values
	register_id = Int32_LE(vm_state.register_id);

	return true;
}
