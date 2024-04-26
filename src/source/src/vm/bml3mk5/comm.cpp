/** @file comm.cpp

	HITACHI BASIC MASTER LEVEL3 Mark5 / MB-S1 Emulator 'EmuB-6892/EmuB-S1'
	Skelton for retropc emulator

	@author Sasaji
	@date   2011.08.20 -

	@brief [ comm ]
*/

#include "comm.h"
#include "../../emu.h"
#include "../../config.h"
#include "../../fileio.h"
#include "../acia.h"

void COMM::initialize()
{
	// RS-232C
	m_is_rs = (m_cfg_num ? 1 : 0);
	m_dipswitch = 1;
	m_through = false;
	m_received = false;

	m_register_id = -1;

	m_connect = DISCONNECT;
	m_client_ch = -1;
	m_server_ch = -1;
	m_uart_ch = -1;

	m_send_telcmd_pos = -1;

	register_frame_event(this);
}

void COMM::reset()
{
	m_is_rs = (m_cfg_num ? 1 : 0);
	m_dipswitch = 1;
	m_through = false;
	m_received = false;

	memset(m_send_buff, 0, sizeof(m_send_buff));
	m_send_buff_w_pos = 0;
	m_send_buff_r_pos = 0;
	memset(m_recv_buff, 0, sizeof(m_recv_buff));
	m_recv_buff_w_pos = 0;
	m_recv_buff_r_pos = 0;

	cancel_my_event();
}

void COMM::release()
{
}

void COMM::write_io8(uint32_t addr, uint32_t data)
{
	if (!(addr & 1)) {
		// Write Control Register
		m_cr = data & 0xff;
		if ((m_cr & 3) != 3 && m_is_rs) {
//			connect_socket();
			cancel_my_event();
			register_my_event();
		}
	}
}

// relay
void COMM::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch(id) {
		case SIG_COMM_RS:
			// from memory
			if (data & mask) {
				m_is_rs = 1;
				register_my_event();
			} else {
				m_is_rs = 0;
				cancel_my_event();
			}
			break;
		case ACIA::SIG_ACIA_RXCLK:
		case ACIA::SIG_ACIA_TXCLK:
		case ACIA::SIG_ACIA_RXDATA:
		case ACIA::SIG_ACIA_CTS:
		case ACIA::SIG_ACIA_DCD:
			if (m_is_rs == 0) {
				// through cmt -> acia
				d_ctrl->write_signal(id, data, mask);
			}
			break;
		case ACIA::SIG_ACIA_TXDATA:
			if (m_is_rs == 0) {
				// through acia -> cmt
				if (d_cmt) d_cmt->write_signal(id, data, mask);
			} else {
				send_data(data);
			}
			break;
		case ACIA::SIG_ACIA_DTR:
		case ACIA::SIG_ACIA_RTS:
			if (m_is_rs == 0) {
				// through acia -> cmt
				if (d_cmt) d_cmt->write_signal(id, data, mask);
			}
			break;
		case ACIA::SIG_ACIA_RESET:
			// clear buffer
			m_send_buff_w_pos = 0;
			m_send_buff_r_pos = 0;
			m_recv_buff_w_pos = 0;
			m_recv_buff_r_pos = 0;
			break;
	}

}

void COMM::send_data(uint32_t data)
{
	if (m_send_buff_w_pos < COMM_MAX_BUFF) {
		if (m_through) {
			m_send_buff[m_send_buff_w_pos++] = (data & 0xff);
		} else {
			m_send_buff[m_send_buff_w_pos++] = (data & 0xff) | 0x30;
		}
	}
	if (m_send_buff_r_pos < m_send_buff_w_pos) {
#ifdef USE_SOCKET
		if (m_client_ch >= 0) {
			emu->send_data_tcp(m_client_ch);
		}
#endif
#ifdef USE_UART
		if (m_uart_ch >= 0) {
			emu->send_uart_data(m_uart_ch);
		}
#endif
	}
}

#if 0
void COMM::recv_data(uint8_t *data, size_t size)
{
	if (is_rs) {
		for(size_t pos = 0; pos < size && recv_buff_w_pos < COMM_MAX_BUFF; pos++) {
			recv_buff[recv_buff_w_pos++] = data[pos];
		}
		received = true;

//		logging->out_debugf("comm recv data size:%d",recv_buff_w_pos - recv_buff_r_pos);
	}
}
#endif

void COMM::register_my_event()
{
	if(m_register_id == -1) {
		m_dipswitch = pConfig->comm_dipswitch[m_cfg_num];
		m_through = pConfig->comm_through[m_cfg_num];

		int baud = (150 << m_dipswitch);	// 1 .. 4
		switch(m_cr & 0x03) {
			case 0:
				baud *= 16;
				break;
			case 1:
				baud *= 4;
				break;
		}
		int bits = 1;
		if (m_through) {
			switch(m_cr & 0x1c) {
				case 0x08:
				case 0x0c:
				case 0x14:
					bits = 10;
					break;
				default:
					bits = 11;
					break;
			}
		}
		int period = (int)(CPU_CLOCKS / baud * bits);
		register_event_by_clock(this, 0, period, true, &m_register_id);
//		logging->out_debugf("COMM: event regist:%d speed: %d baud",register_id,baud);
//	} else {
//		logging->out_debugf("COMM: event already registed: %d",register_id);
	}
}

void COMM::cancel_my_event()
{
	if(m_register_id != -1) {
		cancel_event(this, m_register_id);
//		logging->out_debugf("COMM: event canceld: %d",register_id);
//	} else {
//		logging->out_debugf("COMM: event already canceld: %d",register_id);
	}
	m_register_id = -1;
}


// ----------------------------------------------------------------------------
// event handler
// ----------------------------------------------------------------------------

void COMM::event_frame()
{
#if 0
	if (m_send_buff_r_pos < m_send_buff_w_pos) {
#ifdef USE_SOCKET
		if (m_client_ch >= 0) {
			emu->send_data_tcp(m_client_ch);
		}
#endif
#ifdef USE_UART
		if (m_uart_ch >= 0) {
			emu->send_uart_data(m_uart_ch);
		}
#endif
	}
#endif
	if (m_recv_buff_r_pos > 0 && m_recv_buff_r_pos < m_recv_buff_w_pos) {
//		logging->out_logf(LOG_DEBUG, _T("COMM::event_frame recv_buff num:%d ch:%d w:%d r:%d") ,cfg_num,client_ch,recv_buff_w_pos,recv_buff_r_pos);
		memcpy(&m_recv_buff[0], &m_recv_buff[m_recv_buff_r_pos], m_recv_buff_w_pos - m_recv_buff_r_pos);
		m_recv_buff_w_pos -= m_recv_buff_r_pos;
		m_recv_buff_r_pos = 0;
	}

	// modified baud rate
	if (m_register_id != -1 && (m_dipswitch != pConfig->comm_dipswitch[m_cfg_num] || m_through != pConfig->comm_through[m_cfg_num])) {
		cancel_my_event();
		register_my_event();
	}
}

void COMM::event_callback(int event_id, int err)
{
	// send to acia
	if (d_ctrl != NULL) {
		// full duplex?

		// send to rxclk
		if (m_received) {
//			logging->out_logf(LOG_DEBUG, _T("comm send to acia data=%d pos:%d") ,recv_buff[recv_buff_r_pos],recv_buff_r_pos);

			d_ctrl->write_signal(ACIA::SIG_ACIA_RXCLK, 1, m_through ? 0xff : 1);
			// send data to acia
			d_ctrl->write_signal(ACIA::SIG_ACIA_RXDATA, m_recv_buff[m_recv_buff_r_pos], m_through ? 0xff : 1);
			m_recv_buff_r_pos++;
			if (m_recv_buff_r_pos >= m_recv_buff_w_pos) {
				m_received = false;
			}
//		} else {
//			// no received data yet
//			// send "H" to acia
////			logging->out_log(LOG_DEBUG, _T("COMM::event_callback no received data"));
//			d_ctrl->write_signal(ACIA::SIG_ACIA_RXDATA, 1, 1);
		}

		// receive from acia
		if (m_is_rs) {
//			logging->out_debug("COMM: event_callback: %d", event_id);
			d_ctrl->write_signal(ACIA::SIG_ACIA_TXCLK, 1, m_through ? 0xff : 1);
		}
	}
}

// ----------------------------------------------------------------------------
/// @note called by main thread
void COMM::enable_server()
{
#ifdef USE_SOCKET
	if (!pConfig->comm_server[m_cfg_num]) {
		// connection active as client
		if (m_client_ch != -1) {
			// error
			logging->out_log(LOG_ERROR, _T("Already connecting to another comm server."));
			return;
		}
		// get socket channel
		m_server_ch = emu->get_socket_channel();
		if (m_server_ch < 0) {
			return;
		}
		// start server
		if (!emu->init_socket_tcp(m_server_ch, this, true)) {
			m_server_ch = -1;
			logging->out_log(LOG_ERROR, _T("Network socket initialize failed."));
			return;
		}
		if (!emu->connect_socket(m_server_ch, pConfig->comm_server_host[m_cfg_num].Get(), pConfig->comm_server_port[m_cfg_num], true)) {
			m_server_ch = -1;
			logging->out_log(LOG_ERROR, _T("Cannot start as comm server."));
			return;
		}
//		logging->out_log(LOG_DEBUG, _T("Started comm server."));
		pConfig->comm_server[m_cfg_num] = true;
	} else {
		// stop server
		if (m_client_ch != -1) {
			// disconnect client
			emu->disconnect_socket(m_client_ch);
		}
		emu->disconnect_socket(m_server_ch);
//		logging->out_log(LOG_DEBUG, _T("Stopped comm server."));
		pConfig->comm_server[m_cfg_num] = false;
	}
#endif
}

/// @note called by main thread
void COMM::enable_connect(int num)
{
	if (num == 0) {
		// Ethernet
		if (!emu->is_connecting_socket(m_client_ch)) {
			disconnect_all();
			connect_socket();
		} else {
			disconnect_all();
		}
#ifdef USE_UART
	} else if(num > 0) {
		// COM port on host
		if (!emu->is_opened_uart(num - 1)) {
			disconnect_all();
			connect_uart(num - 1);
		} else {
			disconnect_all();
		}
#endif
	}
}

/// @note called by main thread
bool COMM::now_connecting(int num)
{
	if (num == 0) {
		// Ethernet
		return emu->is_connecting_socket(m_client_ch);
#ifdef USE_UART
	} else if(num > 0) {
		// COM port on host
		return emu->is_opened_uart(num - 1);
#endif
	} else {
		return false;
	}
}

/// @note called by main thread
void COMM::send_telnet_command(int num)
{
	char buf[8];
	int len = 0;
	buf[0]=0;
	if (m_server_ch < 0 || m_client_ch < 0) {
		return;
	}
	switch(num) {
	case 0:
		// WILL/DO BINARY
		len = 6;
		memcpy(buf, "\xff\xfb\x00\xff\xfd\x00", len);
		pConfig->comm_binary[m_cfg_num] = true;
		break;
	case 0x10:
		// WON'T/DON'T BINARY
		len = 6;
		memcpy(buf, "\xff\xfc\x00\xff\xfe\x00", len);
		pConfig->comm_binary[m_cfg_num] = false;
		break;
	case 1:
		// WILL Suppress Go Ahead and WILL ECHO 
		len = 6;
		memcpy(buf, "\xff\xfb\x03\xff\xfb\x01", len);
		break;
	}
	if (len > 0 && m_send_buff_w_pos < (COMM_MAX_BUFF - len)) {
		memcpy(&m_send_buff[m_send_buff_w_pos], buf, len);
		m_send_buff_w_pos += len;
		m_send_telcmd_pos = m_send_buff_w_pos;
	}
}

/// @note called by emu thread
uint8_t* COMM::get_sendbuffer(int ch, int* size, int* flags)
{
//	logging->out_logf(LOG_DEBUG, _T("get_sendbuffer: ch:%d client_ch:%d"),ch,client_ch);
	if (ch == m_client_ch || ch == m_uart_ch) {
		*size = (m_send_buff_w_pos - m_send_buff_r_pos);
		*flags = (pConfig->comm_binary[m_cfg_num] && m_send_telcmd_pos < 0) ? 1 : 0;
//		logging->out_logf(LOG_DEBUG, _T("Send buffer. w:%d r:%d size:%d"),send_buff_w_pos,send_buff_r_pos,*size);
		return &m_send_buff[m_send_buff_r_pos];
	} else {
		*size = 0;
		return NULL;
	}
}

/// @note called by emu thread
void COMM::inc_sendbuffer_ptr(int ch, int size)
{
//	logging->out_logf(LOG_DEBUG, _T("inc_sendbuffer_ptr: ch:%d client_ch:%d"),ch,client_ch);
	if (ch == m_client_ch || ch == m_uart_ch) {
		m_send_buff_r_pos += size;
//		logging->out_logf(LOG_DEBUG, _T("Sent buffer. w:%d r:%d size:%d"),send_buff_w_pos,send_buff_r_pos,size);
		if (m_send_buff_r_pos >= m_send_telcmd_pos) {
			m_send_telcmd_pos = -1;
		}
		if (m_send_buff_w_pos <= m_send_buff_r_pos) {
			// all data sent.
			m_send_buff_w_pos = 0;
			m_send_buff_r_pos = 0;
		}
	}
}

/// @note called by emu thread
uint8_t* COMM::get_recvbuffer0(int ch, int* size0, int* size1, int* flags)
{
//	logging->out_logf(LOG_DEBUG, _T("get_recvbuffer0: ch:%d client_ch:%d"),ch,client_ch);
	if (ch == m_client_ch || ch == m_uart_ch) {
		*size0 = (COMM_MAX_BUFF - m_recv_buff_w_pos);
		*size1 = 0;
		*flags = pConfig->comm_binary[m_cfg_num] ? 1 : 0;
//		logging->out_logf(LOG_DEBUG, _T("Recve buffer. w:%d size:%d"),recv_buff_w_pos,*size0);
//		if (*size0 == 0) {
//			if (d_ctrl) d_ctrl->write_signal(ACIA::SIG_ACIA_ERR_OVRN, 1, 1);
//		}
		return &m_recv_buff[m_recv_buff_w_pos];
	} else {
		*size0 = 0;
		*size1 = 0;
		return NULL;
	}
}

/// @note called by emu thread
uint8_t* COMM::get_recvbuffer1(int ch)
{
	if (ch == m_client_ch || ch == m_uart_ch) {
		return m_recv_buff;
	} else {
		return NULL;
	}
}

/// @note called by emu thread
void COMM::inc_recvbuffer_ptr(int ch, int size)
{
	if (ch == m_client_ch || ch == m_uart_ch) {
		if (size > 0) {
			m_recv_buff_w_pos += size;
			m_received = true;
		}
	}
}

/// @note called by main thread
bool COMM::connect_socket()
{
#ifdef USE_SOCKET
	if (!pConfig->comm_server[m_cfg_num] && m_client_ch == -1) {
		// get socket channel
		m_client_ch = emu->get_socket_channel();
		if (m_client_ch < 0) {
			return false;
		}
		// connect
		if (!emu->init_socket_tcp(m_client_ch, this)) {
			m_client_ch = -1;
			logging->out_log(LOG_ERROR, _T("Network socket initialize failed."));
			return false;
		}
		if (!emu->connect_socket(m_client_ch, pConfig->comm_server_host[m_cfg_num].Get(), pConfig->comm_server_port[m_cfg_num])) {
			m_client_ch = -1;
			logging->out_log(LOG_ERROR, _T("Cannot connect to comm server."));
			return false;
		}
		m_connect = CONNECTING;
//		logging->out_log(LOG_DEBUG, _T("Connect comm client."));
	}
	return true;
#else
	return false;
#endif
}

/// @note called by main thread
void COMM::disconnect_socket()
{
#ifdef USE_SOCKET
	if (m_client_ch != -1) {
		// disconnect
		emu->disconnect_socket(m_client_ch);
//		logging->out_log(LOG_DEBUG, _T("Disconnect comm client."));
	}
#endif
}
/// callback
/// called by EMU::socket_connected()
void COMM::network_connected(int ch)
{
	if (ch == m_client_ch) {
		m_connect = CONNECTED;
//		pConfig->comm_connect[m_cfg_num] = true;
//		logging->out_logf(LOG_DEBUG, _T("Connected comm. ch:%d"),ch);
	}
}
/// callback
/// called by EMU::disconnect_socket()
void COMM::network_disconnected(int ch)
{
	if (ch == m_client_ch) {
		m_connect = DISCONNECT;
//		pConfig->comm_connect[m_cfg_num] = false;
//		logging->out_logf(LOG_DEBUG, _T("Disonnected comm. ch:%d"),ch);
		m_client_ch = -1;
	} else if (ch == m_server_ch) {
//		logging->out_logf(LOG_DEBUG, _T("Disonnected comm. ch:%d"),ch);
		m_server_ch = -1;
	}
	pConfig->comm_binary[m_cfg_num] = false;
}
void COMM::network_writeable(int ch)
{
	if (ch == m_client_ch) {
//		logging->out_logf(LOG_DEBUG, _T("Writeable comm. ch:%d"),ch);
		m_connect = CONNWRITEABLE;
	}
}
void COMM::network_readable(int ch)
{
}
void COMM::network_accepted(int ch, int new_ch)
{
#ifdef USE_SOCKET
	if (ch == m_server_ch) {
		// close serial port 
		disconnect_uart();

		if (m_client_ch != -1) {
			// Another client is already connected on my server.
			emu->disconnect_socket(new_ch);
		}
		m_client_ch = new_ch;
//		pConfig->comm_connect[m_cfg_num] = true;
	}
#endif
}

bool COMM::connect_uart(int ch)
{
#ifdef USE_UART
	if (!emu->open_uart(ch)) {
		logging->out_log(LOG_ERROR, _T("Cannot open serial port on host."));
		return false;
	}
	emu->init_uart(ch, this);
	m_uart_ch = ch;
	return true;
#else
	return false;
#endif
}

void COMM::disconnect_uart()
{
#ifdef USE_UART
	if (m_uart_ch != -1) {
		emu->close_uart(m_uart_ch);
		m_uart_ch = -1;
	}
#endif
}

void COMM::disconnect_all()
{
	disconnect_socket();
	disconnect_uart();
//	// stop clock and reset acia
//	cancel_my_event();
//	d_ctrl->write_io8(0, 0x03);
}

// ----------------------------------------------------------------------------

void COMM::save_state(FILEIO *fp)
{
	struct vm_state_st vm_state;

	//
	vm_state_ident.version = Uint16_LE(2);
	vm_state_ident.size = Uint32_LE(sizeof(vm_state_ident) + sizeof(vm_state));

	// copy values
	memset(&vm_state, 0, sizeof(vm_state));
	vm_state.register_id = Int32_LE(m_register_id);
	vm_state.is_rs = Int32_LE(m_is_rs);
	vm_state.dipswitch = Uint16_LE(m_dipswitch);
	vm_state.through = m_through ? 1 : 0;

	fp->Fwrite(&vm_state_ident, sizeof(vm_state_ident), 1);
	fp->Fwrite(&vm_state, sizeof(vm_state), 1);
}

bool COMM::load_state(FILEIO *fp)
{
	vm_state_ident_t vm_state_i;
	struct vm_state_st vm_state;

	READ_STATE_CHUNK(fp, vm_state_i, vm_state);

	// copy values
	m_register_id = Int32_LE(vm_state.register_id);
	m_is_rs = Int32_LE(vm_state.is_rs);

	if (vm_state_i.version >= 2) {
		m_dipswitch = Uint16_LE(vm_state.dipswitch);
		m_through = (vm_state.through != 0);
	} else {
		m_dipswitch = pConfig->comm_dipswitch[m_cfg_num];
		m_through = pConfig->comm_through[m_cfg_num];
	}

	return true;
}
