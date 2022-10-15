/** @file wxw_socket.cpp

	Skelton for retropc emulator
	SDL edition

	@author Sasaji
	@date   2013.12.06

	@brief [ wxw socket ]
*/

#include <wx/wx.h>
#include "wxw_emu.h"
#include "../../vm/vm.h"
#include "../../vm/device.h"
#include "wxw_socket.h"
#include "wxw_main.h"
#include "../../fifo.h"
#include "../../cmutex.h"


void EMU_OSD::EMU_SOCKET()
{
	// init sockets
	conn = new Connection();
}

void EMU_OSD::initialize_socket()
{
	conn->Initialize();
}

void EMU_OSD::release_socket()
{
	// release sockets
	delete conn;
	conn = NULL;
}

void EMU_OSD::socket_connected(int ch)
{
	// network is connected
	logging->out_debugf(_T("socket_connected: ch=%d"), ch);
	DEVICE *dev = conn->GetDevice(ch);
	if (dev) dev->network_connected(ch);
//	else vm->network_connected(ch);
}

void EMU_OSD::socket_disconnected(int ch)
{
	// network is disconnected
//	conn->SetDisconnectDelay(ch, 1 /*56*/);
	logging->out_debugf(_T("socket_disconnected: ch=%d"), ch);
	DEVICE *dev = conn->GetDevice(ch);
	if (dev) dev->network_disconnected(ch);
}

void EMU_OSD::socket_writeable(int ch)
{
	// network is writeable
	logging->out_debugf(_T("socket_writeable: ch=%d"), ch);
	DEVICE *dev = conn->GetDevice(ch);
	if (dev) dev->network_writeable(ch);
//	else vm->network_writeable(ch);
}

void EMU_OSD::socket_readable(int ch)
{
	// network is readable
	logging->out_debugf(_T("socket_readable: ch=%d"), ch);
	DEVICE *dev = conn->GetDevice(ch);
	if (dev) dev->network_readable(ch);
//	else vm->network_readable(ch);
}

void EMU_OSD::update_socket()
{
	// buffer copy
	for(int i = 0; i < SOCKET_MAX; i++) {
		CSocket *soc = conn->Item(i);
		soc->SendToDevice();
	}
}

void EMU_OSD::socket_accepted(int ch, int new_ch, bool tcp)
{
	if (ch < 0 || SOCKET_MAX <= ch) return;
	if (new_ch < 0 || SOCKET_MAX <= new_ch) return;

	logging->out_debugf(_T("socket_accepted: ch=%d new_ch=%d"), ch, new_ch);
	DEVICE *dev = conn->GetDevice(ch);
	if (dev) dev->network_accepted(ch, new_ch);
//	else vm->network_accepted(ch, new_ch);
}

/// @note called by main thread
bool EMU_OSD::init_socket_tcp(int ch, DEVICE *dev, bool server)
{
	if (ch < 0 || SOCKET_MAX <= ch) {
		logging->out_logf(LOG_ERROR, _T("EMU_OSD::init_socket_tcp: invalid socket channel. ch=%d"), ch);
		return false;
	}

	CSocket *soc = conn->Item(ch);

	return soc->InitTCP(dev, server);
}

/// @note called by main thread
bool EMU_OSD::init_socket_udp(int ch, DEVICE *dev, bool server)
{
	if (ch < 0 || SOCKET_MAX <= ch) {
		logging->out_logf(LOG_ERROR, _T("EMU_OSD::init_socket_udp: invalid socket channel. ch=%d"), ch);
		return false;
	}

	CSocket *soc = conn->Item(ch);

	return soc->InitUDP(dev, server);
}

bool EMU_OSD::connect_socket(int ch, const _TCHAR *hostname, int port, bool server)
{
	if (ch < 0 || SOCKET_MAX <= ch) {
		logging->out_logf(LOG_ERROR, _T("EMU_OSD::connect_socket: invalid socket channel. ch=%d"), ch);
		return false;
	}

	CSocket *soc = conn->Item(ch);

	return soc->Connect(hostname, port, server);
}

bool EMU_OSD::is_connecting_socket(int ch)
{
	if (ch < 0 || SOCKET_MAX <= ch) return false;

	CSocket *soc = conn->Item(ch);

	return soc->IsConnecting();
}

void EMU_OSD::disconnect_socket(int ch)
{
	if (ch < 0 || SOCKET_MAX <= ch) return;

	CSocket *soc = conn->Item(ch);

	soc->Disconnect();
}

int EMU_OSD::get_socket_channel()
{
	return conn->GetEmptySocket();
}

bool EMU_OSD::listen_socket(int ch)
{
	return false;
}

/// @note called by emu thread
void EMU_OSD::send_data_tcp(int ch)
{
	if (ch < 0 || SOCKET_MAX <= ch) return;

	CSocket *soc = conn->Item(ch);

	soc->RecvFromDevice();
}

/// @note called by emu thread
void EMU_OSD::send_data_udp(int ch, const _TCHAR *hostname, int port)
{
	if (ch < 0 || SOCKET_MAX <= ch) return;

	CSocket *soc = conn->Item(ch);

	soc->RecvFromDevice(hostname, port);
}

/// @note called by emu thread
void EMU_OSD::send_data(int ch)
{
	if (ch < 0 || SOCKET_MAX <= ch) return;

	CSocket *soc = conn->Item(ch);

	soc->SendData();
}

#if 0
/// @note usually called by emu thread
/// @note called by main thread at first when connection succeeded.
void EMU_OSD::send_data_(int ch, const _TCHAR *hostname, int port)
{
	if (ch < 0 || SOCKET_MAX <= ch) return;

	CSocket *soc = conn->Item(ch);

	soc->RecvFromDevice(hostname, port, buf, size, flags);
}
#endif

const void *EMU_OSD::get_socket(int ch) const
{
	return NULL;
}





/*******************************************************
 * Network
 */
BEGIN_EVENT_TABLE(CSocket, wxEvtHandler)
	EVT_SOCKET(IDS_ACCEPT_ID, CSocket::OnAcceptEvent)
	EVT_SOCKET(IDS_CONNECT_ID, CSocket::OnConnectEvent)
	EVT_SOCKET(IDS_SOCKET_ID, CSocket::OnSocketEvent)
	EVT_TIMER(IDS_TIMER_ID, CSocket::OnTimerEvent)
END_EVENT_TABLE()

// create new socket
CSocket::CSocket(Connection *conn, int ch)
	: wxEvtHandler()
{
	m_conn = conn;
	m_ch = ch;
	device = NULL;
	mSocket = NULL;
	is_tcp = true;
	is_server = false;
	m_connecting = false;

	recv_buffer = NULL;
	recv_mux = NULL;
	send_buffer = NULL;
	send_mux = NULL;
}

CSocket::~CSocket()
{
	delete recv_buffer;
	delete recv_mux;
	delete send_buffer;
	delete send_mux;
	logging->out_logf(LOG_DEBUG, _T("CSocket::~CSocket: ch=%d: Destroy."), m_ch);
}

int CSocket::read_and_remove_dup_iac(char *dst, int size, FIFOCHAR *buffer)
{
	int pos = 0;
	int rep = 0;
	char cp = 0;
	for(int spos = 0; spos < size; ) {
		char c = buffer->read();
		spos++;
		if (c == (char)0xff && spos == size) {
			if (rep != 1) buffer->rollback();
			break;
		}
		if (cp != (char)0xff || c != (char)0xff) {
			dst[pos++] = c;
			rep = 0;
		}
		if (c == (char)0xff) {
			rep++;
			if (rep >= 2) c = 0;
		}
		cp = c;
	}
	return pos;
}

void CSocket::SendToDevice()
{
	int size = recv_buffer ? recv_buffer->count() : 0;
	if(size > 0) {
		if (device) {
			// get buffer
			int size0 = 0;
			int size1 = 0;
			int flags = 0;
			uint8_t* buf0 = device->get_recvbuffer0(m_ch, &size0, &size1, &flags);
			//	buf0 = vm->get_recvbuffer0(m_ch, &size0, &size1, &flags);
			uint8_t* buf1 = device->get_recvbuffer1(m_ch);
			//	buf1 = vm->get_recvbuffer1(m_ch);

			if(size > size0 + size1) {
				size = size0 + size1;
			}

			if(size <= size0) {
				recv_mux->lock();
				if (!(flags & 1)) {
					recv_buffer->read((char *)buf0, size);
				} else {
					// escape telnet iac
					size = read_and_remove_dup_iac((char *)buf0, size, recv_buffer);
				}
				recv_mux->unlock();
			}
			else {
				recv_mux->lock();
				if (!(flags & 1)) {
					recv_buffer->read((char *)buf0, size0);
					recv_buffer->read((char *)buf1, size - size0);
				} else {
					// escape telnet iac
					size0 = read_and_remove_dup_iac((char *)buf0, size0, recv_buffer);
					size = read_and_remove_dup_iac((char *)buf1, size - size0, recv_buffer);
				}
				recv_mux->unlock();
			}
			device->inc_recvbuffer_ptr(m_ch, size);
			//	vm->inc_recvbuffer_ptr(m_ch, size);
		}
	}
//	else if(socket_delay > 0) {
//		if(--socket_delay == 0) {
//			Disconnect();
//			if (device) device->network_disconnected(m_ch);
//			else vm->network_disconnected(m_ch);
//		}
//	}
}

int CSocket::write_and_escape_iac(const char *src, int size, FIFOCHAR *buffer)
{
	int pos = 0;
	for(int i=0; i<size; i++) {
		buffer->write(src[i]); pos++;
		if (src[i] == (char)0xff) {
			buffer->write(src[i]); pos++;
		}
	}
	return pos;
}

void CSocket::RecvFromDevice()
{
	// get send buffer and data size
	int size;
	int flags = 0;
	uint8_t* buf;

	// loop while send buffer is not empty
	while(device) {
		buf = device->get_sendbuffer(m_ch, &size, &flags);
		//	buf = vm->get_sendbuffer(m_ch, &size);

		if(!buf || !size) {
			break;
		}

		send_mux->lock();
		if (flags & 1) {
			// escape telnet iac
			size = write_and_escape_iac((const char *)buf, size, send_buffer);
		} else {
			size = send_buffer->write((const char *)buf, size);
		}
		send_mux->unlock();

		device->inc_sendbuffer_ptr(m_ch, size);
		//	vm->inc_sendbuffer_ptr(m_ch, size);
	}

	SendData();
}

void CSocket::RecvFromDevice(const _TCHAR *hostname, int port)
{
	SetUDPAddress(hostname, port);
	RecvFromDevice();
}

bool CSocket::InitTCP(DEVICE *dev, bool UNUSED_PARAM(server))
{
	if (IsConnecting()) {
		Disconnect();
	}

	// reserve channel for tcp connection
	is_tcp = true;
	device = NULL;

	if (recv_buffer) recv_buffer->clear();
	if (send_buffer) send_buffer->clear();

	device = dev;

	return true;
}

bool CSocket::InitUDP(DEVICE *dev, bool server)
{
	if (IsConnecting()) {
		Disconnect();
	}

	// reserve channel for udp connection
	is_tcp = false;
	device = NULL;

	is_server = server;

	if (recv_buffer) recv_buffer->clear();
	if (send_buffer) send_buffer->clear();

	device = dev;

	return true;
}

void CSocket::OnTimerEvent(wxTimerEvent& event)
{
	switch(mTimerType) {
	case IDT_CONNECTED:
		// client connected
		emu->socket_connected(m_ch);
		// and writeable
		emu->socket_writeable(m_ch);
		// send data
		emu->send_data(m_ch);
		break;
	}
}

void CSocket::OnAcceptEvent(wxSocketEvent& event)
{
	wxSocketBase *sock;

	sock = ((wxSocketServer *)mSocket)->Accept(false);
	if (sock) {
		MyIPaddress addr;
		if (!sock->GetPeer(addr)) {
			logging->out_logf(LOG_DEBUG, _T("CSocket::OnAcceptEvent: ch=%d: New connection from unknown client accepted."), m_ch);
		} else {
			logging->out_logf(LOG_DEBUG, _T("CSocket::OnAcceptEvent: ch=%d: New client connection from %ls:%u accepted"),
				   m_ch, addr.IPAddress().wc_str(), addr.Service());
		}

		if (!m_conn->Accept(m_ch, sock, is_tcp)) {
			sock->Destroy();
			logging->out_logf(LOG_ERROR, _T("CSocket::OnAcceptEvent: ch=%d: New client connection forcelly destroyed."), m_ch);
			return;
		}

	} else {
		logging->out_logf(LOG_ERROR, _T("CSocket::OnAcceptEvent: ch=%d: Accept Error"), m_ch);
		return;
	}
}

void CSocket::OnSocketEvent(wxSocketEvent& event)
{
	switch(event.GetSocketEvent()) {
	case wxSOCKET_INPUT:
		OnSocketInputEvent(event);
		break;
	case wxSOCKET_LOST:
		OnSocketLostEvent(event);
		break;
	default:
		break;
	}
}

void CSocket::OnSocketInputEvent(wxSocketEvent& event)
{
	// receive data
//	emu->recv_data(m_ch);
	RecvData();
	// readable
	emu->socket_readable(m_ch);

	// Enable input events again.
	mSocket->SetNotify(wxSOCKET_LOST_FLAG | wxSOCKET_INPUT_FLAG);
}

void CSocket::OnSocketLostEvent(wxSocketEvent& event)
{
	logging->out_logf(LOG_DEBUG, _T("CSocket::OnSocketLostEvent: ch=%d: lost socket."), m_ch);
	Disconnect();
}

void CSocket::OnConnectEvent(wxSocketEvent& event)
{
	// client connected
	emu->socket_connected(m_ch);
	// and writeable
	emu->socket_writeable(m_ch);
	// send data
	emu->send_data(m_ch);

	wxSocketBase *sock = event.GetSocket();

	// setup receive/lost event
	sock->SetEventHandler(*this, IDS_SOCKET_ID);
	sock->SetNotify(wxSOCKET_INPUT_FLAG | wxSOCKET_LOST_FLAG);
	sock->Notify(true);
}

bool CSocket::Connect(const wxString &hostname, int port, bool server)
{
	if (IsConnecting()) {
		return true;
	}

	m_connecting = false;
//	emu = parent->GetEMU();
//	m_busy = false;

//	mSocket = NULL;
	is_server = server;

	if (server) {
		// create server socket

		MyIPaddress addr;
		addr.Hostname(hostname);
		addr.Service(port);

		// Create the socket
		mSocket = (wxSocketBase *)new wxSocketServer(addr);
		// We use IsOk() here to see if the server is really listening
		if (!mSocket->IsOk()) {
			logging->out_logf(LOG_ERROR, _T("CSocket::Connect: ch=%d: Couldn't listen at %ls:%d"), m_ch, hostname.wc_str(), port);
			return m_connecting;
		}
		MyIPaddress addrReal;
		if (!mSocket->GetLocal(addrReal)) {
			logging->out_logf(LOG_ERROR, _T("CSocket::Connect: ch=%d: Couldn't get the address we bound to"), m_ch);
			return m_connecting;
		}
		logging->out_logf(LOG_DEBUG, _T("CSocket::Connect: ch=%d: Server listening at %ls:%u"), m_ch, addrReal.IPAddress().wc_str(), addrReal.Service());

		// prepared
//		mTimer.SetOwner(this, IDS_TIMER_ID);
//		mTimerType = IDT_CONNECTED;
//		mTimer.StartOnce(30);

		// wait connection from client
		mSocket->SetEventHandler(*this, IDS_ACCEPT_ID);
		mSocket->SetNotify(wxSOCKET_CONNECTION_FLAG);
		mSocket->Notify(true);

	} else {
		// create client socket

		if (is_tcp) {
			// Create the socket
			mSocket = (wxSocketBase *)new wxSocketClient();
			// IsOK always failed
//			if (!mSocket->IsOk()) {
//				logging->out_logf(LOG_ERROR, _T("CSocket::Connect: ch=%d: Couldn't create socket client."), m_ch);
//				return;
//			}

			// connect to server
			if (!ConnectMain(hostname, port)) {
				logging->out_logf(LOG_ERROR, _T("CSocket::Connect: ch=%d: Couldn't connect to server."), m_ch);
				return m_connecting;
			}
		} else {

			MyIPaddress addr;
			addr.Hostname(hostname);
			addr.Service(port);

			// Create udp socket
			mSocket = (wxSocketBase *)new wxDatagramSocket(addr);
		}

		if (!recv_buffer) recv_buffer = new FIFOCHAR(SOCKET_BUFFER_MAX);
		if (!send_buffer) send_buffer = new FIFOCHAR(SOCKET_BUFFER_MAX);
		if (!recv_mux) recv_mux = new CMutex();
		if (!send_mux) send_mux = new CMutex();
	}

	m_connecting = true;
	return m_connecting;
}
// set socket from accepted sock
bool CSocket::Connect(wxSocketBase *sock)
{
	if (IsConnecting()) {
		return true;
	}

	m_connecting = false;
//	conn = parent;
//	emu = parent->GetEMU();
//	m_busy = false;

	mSocket = sock;
	is_server = false;

	// connected and writeable
	mTimer.SetOwner(this, IDS_TIMER_ID);
	mTimerType = IDT_CONNECTED;
	mTimer.StartOnce(30);

	// setup receive/lost event
	sock->SetEventHandler(*this, IDS_SOCKET_ID);
	sock->SetNotify(wxSOCKET_INPUT_FLAG | wxSOCKET_LOST_FLAG);
	sock->Notify(true);

	if (!recv_buffer) recv_buffer = new FIFOCHAR(SOCKET_BUFFER_MAX);
	if (!send_buffer) send_buffer = new FIFOCHAR(SOCKET_BUFFER_MAX);
	if (!recv_mux) recv_mux = new CMutex();
	if (!send_mux) send_mux = new CMutex();

	logging->out_logf(LOG_DEBUG, _T("CSocket::Connect: ch=%d: Accepted client."), m_ch);

	m_connecting = true;
	return m_connecting;
}

//bool CSocket::ConnectMain(const wxString &hostname, const wxString &service)
//{
//	wxString service = wxString::Format(wxT("%d"),port);
//	return Connect(hostname, service);
//}

bool CSocket::ConnectMain(const wxString &hostname, int port)
{
	MyIPaddress addr;
	addr.Hostname(hostname);
	addr.Service(port);

	if (!((wxSocketClient *)mSocket)->Connect(addr, false)) {
		if (mSocket->LastError() != wxSOCKET_WOULDBLOCK) {
			return false;
		}
	}

	// connection ok
	mSocket->SetEventHandler(*this, IDS_CONNECT_ID);
	mSocket->SetNotify(wxSOCKET_CONNECTION_FLAG);
	mSocket->Notify(true);

	logging->out_logf(LOG_DEBUG, _T("CSocket::ConnectMain: ch=%d: Client connection to %ls:%d"), m_ch, hostname.wc_str(), port);

	return true;
}

void CSocket::Disconnect()
{
	// disconnect
	emu->socket_disconnected(m_ch);
//	conn->OnDestroy(m_ch);

	if (mSocket) {
		mSocket->Close();
		delete mSocket;
	}
	mSocket = NULL;
	device = NULL;
	m_connecting = false;
}

void CSocket::Shutdown()
{
	// destory and delete mSocket
	if (mSocket) mSocket->Destroy();
}

void CSocket::SetUDPAddress(const _TCHAR *hostname, int port)
{
	mUDPaddress.Hostname(hostname);
	mUDPaddress.Service(port);
}

size_t CSocket::Send(const char *buffer, size_t len)
{
	mSocket->Write(buffer, (wxUint32)len);
	len = mSocket->LastWriteCount();
	if (mSocket->Error()) {
		// error
		logging->out_logf(LOG_DEBUG, _T("CSocket::Send: ch=%d: Socket write error: %d"),
				m_ch, mSocket->LastError());
	}
	return len;
}

size_t CSocket::Recv(char *buffer, size_t len)
{
	mSocket->Read(buffer, (wxUint32)len);
	len = mSocket->LastReadCount();
	if (mSocket->Error()) {
		// error
		logging->out_logf(LOG_DEBUG, _T("CSocket::Recv: ch=%d: Socket read error: %d"),
				m_ch, mSocket->LastError());
	}
	return len;
}

size_t CSocket::SendTo(const char *buffer, size_t len)
{
	if (!is_tcp) return 0;

	((wxDatagramSocket *)mSocket)->SendTo(mUDPaddress, buffer, (wxUint32)len);
	len = mSocket->LastWriteCount();
	if (mSocket->Error()) {
		// error
		logging->out_logf(LOG_DEBUG, _T("CSocket::SendTo: ch=%d: Socket write error: %d"),
				m_ch, mSocket->LastError());
	}
	return len;
}

size_t CSocket::RecvFrom(char *ipaddr, int *port, char *buffer, size_t len)
{
	if (!is_tcp) return 0;

	mSocket->Read(buffer, (wxUint32)len);
	len = mSocket->LastReadCount();
	if (mSocket->Error()) {
		// error
		logging->out_logf(LOG_DEBUG, _T("CSocket:RecvFrom ch=%d: Socket read error: %d"),
				m_ch, mSocket->LastError());
	}
	MyIPaddress addr;
	mSocket->GetPeer(addr);
	if (ipaddr) {
		strcpy(ipaddr, addr.IPAddress().To8BitData());
	}
	if (port) *port = addr.Service();

	return len;
}

void CSocket::SendData()
{
	char buf[1024];

	send_mux->lock();
	int size = send_buffer->read(buf, sizeof(buf));
	send_mux->unlock();

	if(is_tcp) {
		int sent_size = (int)Send(buf, size);
		if(sent_size < size) {
			// error
			return;
		}
//		size = sent_size;
	} else {
		int sent_size = (int)SendTo(buf, size);
		if(sent_size < size) {
			// error
			return;
		}
//		size = sent_size;
	}
}

/// Receive data from socket, and store to first buffer to be able to read from emu devices.
void CSocket::RecvData()
{
	char buf[1024];
	int recv_size = 0;
	if(is_tcp) {
		recv_size = (int)Recv(buf, sizeof(buf));
		if(recv_size <= 0) {
			emu->disconnect_socket(m_ch);
			emu->socket_disconnected(m_ch);
			return;
		}
		recv_mux->lock();
		recv_size = recv_buffer->write(buf, recv_size);
		recv_mux->unlock();
	}
	else {
		char ipaddr[128];
		int port;
		int recv_size = (int)RecvFrom(ipaddr, &port, (buf+8), sizeof(buf) - 8);
		if(recv_size <= 0) {
			emu->disconnect_socket(m_ch);
			emu->socket_disconnected(m_ch);
			return;
		}
		buf[0] = (char)(recv_size >> 8);
		buf[1] = (char)recv_size;
		int val[4];
		int num = sscanf(ipaddr, "%d.%d.%d.%d", &val[0], &val[1], &val[2], &val[3]);
		if (num == 4) {
			buf[2] = (char)val[0];
			buf[3] = (char)val[1];
			buf[4] = (char)val[2];
			buf[5] = (char)val[3];
		}
		buf[6] = (char)port;
		buf[7] = (char)(port >> 8);

		recv_mux->lock();
		recv_size = recv_buffer->write(buf, recv_size + 8);
		recv_mux->unlock();
	}
}

bool CSocket::IsConnecting() const
{
	return m_connecting;
}

/**********************************************************************/

Connection::Connection()
	: wxObject()
{
	for(int i=0; i<SOCKET_MAX; i++) {
		socs.Add(new CSocket(this, i));
	}
}
Connection::~Connection()
{
	for(int i=0; i<socs.Count(); i++) {
		socs.Item(i)->Shutdown();
	}
}

void Connection::Initialize()
{
}

#if 0
bool Connection::Connect(const _TCHAR *hostname, int port, bool server)
{
	int new_ch = GetSocketChannel();
	if (new_ch < 0) {
		return false;
	}
	return Connect(new_ch, hostname, port, server);
}
bool Connection::Connect(int new_ch, const _TCHAR *hostname, int port, bool server)
{
	if (new_ch < 0 || new_ch >= mSocket.Count() || mSocket[new_ch]->IsOk()) {
		logging->out_logf(LOG_ERROR, _T("Connection: new_ch:%d Already used."), new_ch);
		return false;
	}
	if (!mSocket[new_ch]->Connect(hostname, port, server)) {
		return false;
	}
	return true;
}
#endif
bool Connection::Accept(int ch, wxSocketBase *sock, bool tcp)
{
	int new_ch = GetEmptySocket();
	if (new_ch < 0) {
		return false;
	}
	if (!socs.Item(new_ch)->Connect(sock)) {
		return false;
	}

	CSocket *soc = socs.Item(ch);
	CSocket *new_soc = socs.Item(new_ch);

	new_soc->SetTCP(tcp);
	new_soc->SetDevice(soc->GetDevice());

	emu->socket_accepted(ch, new_ch, tcp);
	return true;
}
#if 0
void Connection::Disconnect(int ch)
{
	if (ch >= 0 && ch < mSocket.Count() && mSocket[ch]->IsOk()) {
		mSocket[ch]->Disconnect();
	}
}
#endif
int Connection::GetEmptySocket() const
{
	int new_ch = -1;
	for(int i = 0; i < socs.Count(); i++) {
		if (!socs[i]->IsConnecting()) {
			new_ch = i;
			break;
		}
	}
	if (new_ch < 0) {
		logging->out_log(LOG_ERROR, _T("Connection: Socket is full."));
	}
	return new_ch;
}
#if 0
size_t Connection::Send(int ch, const char *buffer, size_t len)
{
	if (ch >= 0 && ch < mSocket.Count() && mSocket[ch]->IsOk()) {
		return mSocket[ch]->Send(buffer, len);
	} else {
		return 0;
	}
}
size_t Connection::Recv(int ch, char *buffer, size_t len)
{
	if (ch >= 0 && ch < mSocket.Count() && mSocket[ch]->IsOk()) {
		return mSocket[ch]->Recv(buffer, len);
	} else {
		return 0;
	}
}
size_t Connection::SendTo(int ch, const _TCHAR *hostname, int port, const void *buffer, size_t len)
{
	if (ch >= 0 && ch < mSocket.Count() && mSocket[ch]->IsOk()) {
		return mSocket[ch]->SendTo(hostname, port, buffer, len);
	} else {
		return 0;
	}
}
size_t Connection::RecvFrom(int ch, char *ipaddr, int *port, void *buffer, size_t len)
{
	if (ch >= 0 && ch < mSocket.Count() && mSocket[ch]->IsOk()) {
		return mSocket[ch]->RecvFrom(ipaddr, port, buffer, len);
	} else {
		return 0;
	}
}
#endif
//void Connection::OnDestroy(int ch)
//{
//	if (ch >= 0 && ch < mSocket.Count() && mSocket[ch]->IsOk()) {
////		delete mSocket[ch];
////		mSocket[ch]=NULL;
////		mSocketUsed[ch]=false;
//	}
//}
CSocket *Connection::Item(int ch)
{
//	if (0 < ch || mSocket.Count() <= ch) return NULL;
	return socs[ch];
}

void Connection::SetDevice(int ch, DEVICE *dev)
{
//	if (ch < 0 || SOCKET_MAX <= ch) return;
	socs.Item(ch)->SetDevice(dev);
}

DEVICE *Connection::GetDevice(int ch)
{
//	if (ch < 0 || SOCKET_MAX <= ch) return NULL;
	return socs.Item(ch)->GetDevice();
}
