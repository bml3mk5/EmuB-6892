/** @file qt_socket.cpp

	Skelton for retropc emulator
	SDL edition

	@author Sasaji
	@date   2017.03.01

	@brief [ qt socket ]
*/

#include "qt_emu.h"
#include "../../vm/vm.h"
#include "../../vm/device.h"
#include "../../gui/qt/qt_gui.h"
#include "qt_socket.h"
#include "../../fifo.h"
#include "../../cmutex.h"
#include "qt_utils.h"

void EMU_OSD::EMU_SOCKET()
{
}

void EMU_OSD::initialize_socket()
{
	mainwindow->conn->Initialize();
}

void EMU_OSD::release_socket()
{
}

void EMU_OSD::socket_connected(int ch)
{
	// network is connected
	logging->out_debugf(_T("socket_connected: ch=%d"), ch);
	DEVICE *dev = mainwindow->conn->GetDevice(ch);
	if (dev) dev->network_connected(ch);
//	else vm->network_connected(ch);
}

void EMU_OSD::socket_disconnected(int ch)
{
	// network is disconnected
//	mainwindow->conn->SetDisconnectDelay(ch, 1 /*56*/);
	logging->out_debugf(_T("socket_disconnected: ch=%d"), ch);
	DEVICE *dev = mainwindow->conn->GetDevice(ch);
	if (dev) dev->network_disconnected(ch);
}

void EMU_OSD::socket_writeable(int ch)
{
	// network is writeable
	logging->out_debugf(_T("socket_writeable: ch=%d"), ch);
	DEVICE *dev = mainwindow->conn->GetDevice(ch);
	if (dev) dev->network_writeable(ch);
//	else vm->network_writeable(ch);
}

void EMU_OSD::socket_readable(int ch)
{
	// network is readable
	logging->out_debugf(_T("socket_readable: ch=%d"), ch);
	DEVICE *dev = mainwindow->conn->GetDevice(ch);
	if (dev) dev->network_readable(ch);
//	else vm->network_readable(ch);
}

void EMU_OSD::update_socket()
{
	// buffer copy
	for(int i = 0; i < SOCKET_MAX; i++) {
		CSocket *soc = mainwindow->conn->Item(i);
		soc->SendToDevice();
	}
}

void EMU_OSD::socket_accepted(int ch, int new_ch, bool UNUSED_PARAM(tcp))
{
	if (ch < 0 || SOCKET_MAX <= ch) return;
	if (new_ch < 0 || SOCKET_MAX <= new_ch) return;

	logging->out_debugf(_T("socket_accepted: ch=%d new_ch=%d"), ch, new_ch);
	DEVICE *dev = mainwindow->conn->GetDevice(ch);
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

	CSocket *soc = mainwindow->conn->Item(ch);

	return soc->InitTCP(dev, server);
}

/// @note called by main thread
bool EMU_OSD::init_socket_udp(int ch, DEVICE *dev, bool server)
{
	if (ch < 0 || SOCKET_MAX <= ch) {
		logging->out_logf(LOG_ERROR, _T("EMU_OSD::init_socket_udp: invalid socket channel. ch=%d"), ch);
		return false;
	}

	CSocket *soc = mainwindow->conn->Item(ch);

	return soc->InitUDP(dev, server);
}

bool EMU_OSD::connect_socket(int ch, const _TCHAR *hostname, int port, bool server)
{
	if (ch < 0 || SOCKET_MAX <= ch) {
		logging->out_logf(LOG_ERROR, _T("EMU_OSD::connect_socket: invalid socket channel. ch=%d"), ch);
		return false;
	}

	CSocket *soc = mainwindow->conn->Item(ch);

	return soc->Connect(hostname, port, server);
}

bool EMU_OSD::is_connecting_socket(int ch)
{
	if (ch < 0 || SOCKET_MAX <= ch) return false;

	CSocket *soc = mainwindow->conn->Item(ch);

	return soc->IsConnecting();
}

void EMU_OSD::disconnect_socket(int ch)
{
	if (ch < 0 || SOCKET_MAX <= ch) return;

	CSocket *soc = mainwindow->conn->Item(ch);

	soc->Disconnect();
}

int EMU_OSD::get_socket_channel()
{
	return mainwindow->conn->GetEmptySocket();
}

bool EMU_OSD::listen_socket(int UNUSED_PARAM(ch))
{
	return false;
}

/// @note called by emu thread
void EMU_OSD::send_data_tcp(int ch)
{
	if (ch < 0 || SOCKET_MAX <= ch) return;

	CSocket *soc = mainwindow->conn->Item(ch);

	soc->RecvFromDevice();
}

/// @note called by emu thread
void EMU_OSD::send_data_udp(int ch, const _TCHAR *hostname, int port)
{
	if (ch < 0 || SOCKET_MAX <= ch) return;

	CSocket *soc = mainwindow->conn->Item(ch);

	soc->RecvFromDevice(hostname, port);
}

/// @note usually called by emu thread
/// @note called by main thread at first when connection succeeded.
void EMU_OSD::send_data(int ch)
{
	if (ch < 0 || SOCKET_MAX <= ch) return;

	CSocket *soc = mainwindow->conn->Item(ch);

	soc->SendData();
}

/// Receive data from socket, and store to first buffer to be able to read from emu devices.
/// @note called by main thread
void EMU_OSD::recv_data(int ch)
{
	if (ch < 0 || SOCKET_MAX <= ch) return;

	CSocket *soc = mainwindow->conn->Item(ch);

	soc->RecvData();
}

const void *EMU_OSD::get_socket(int UNUSED_PARAM(ch)) const
{
	return nullptr;
}


/*******************************************************
 * Network
 */
CSocket::CSocket(Connection *conn, int ch)
	: QObject()
{
	m_conn = conn;
	m_ch = ch;
	device = nullptr;
	mSocket = nullptr;
	is_tcp = true;
	mSocketMyself = false;
	mServer = nullptr;
	is_server = false;
	mConnServer = nullptr;
	m_numClient = 0;
	m_connecting = false;
	mUDPport = 0;

	recv_buffer = nullptr;
	recv_mux = nullptr;
	send_buffer = nullptr;
	send_mux = nullptr;
}

CSocket::~CSocket()
{
	// destory and delete mSocket
	m_connecting = false;
	device = nullptr;
//	if (mSocket) {
//		mSocket->disconnect();
//		if (mSocketMyself) delete mSocket;
//		mSocket = nullptr;
//	}
//	mSocketMyself = false;
//	if (mServer) {
//		mServer->disconnect();
//		delete mServer;
//		mServer = nullptr;
//	}
	delete recv_buffer;
	delete recv_mux;
	delete send_buffer;
	delete send_mux;
//	logging->out_logf(LOG_DEBUG, _T("CSocket::~CSocket: ch=%d: Destroy."), m_ch);
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

// create new socket
bool CSocket::Connect(const _TCHAR *hostname, int port, bool server)
{
	if (IsConnecting()) {
		return true;
	}

	m_connecting = false;

	mTimer.stop();

//	mSocket = nullptr;
//	mServer = nullptr;

//	mConnServer = nullptr;
//	m_numClient = 0;

	if (server) {
		// create server socket

		QHostInfo info = QHostInfo::fromName(QTChar::fromTChar(hostname));
		if (info.error() != QHostInfo::NoError) {
//			QTChar name(hostname);
			logging->out_logf(LOG_ERROR, _T("CSocket%d: Couldn't resolve at %ls:%d"), m_ch, hostname, port);
			return false;
		}

		// Create the socket
//		QTcpServer *new_sock = new QTcpServer(emu->get_window());
		QTcpServer *new_sock = new QTcpServer();
		if (!new_sock->listen(info.addresses().at(0), static_cast<quint16>(port))) {
			logging->out_logf(LOG_ERROR, _T("CSocket%d: Couldn't listen."), m_ch);
			return false;
		}
		is_server = true;
		mServer = new_sock;

		// acceptable only one client
		mServer->setMaxPendingConnections(1);

		logging->out_logf(LOG_DEBUG, _T("CSocket%d: Server listening."), m_ch);

		// wait connection from client
		connect(mServer, SIGNAL(newConnection()), this, SLOT(onAcceptSlot()));

	} else {
		// create client socket
		is_server = false;

		if (is_tcp) {
			// Create the socket
//			QTcpSocket *new_sock = new QTcpSocket(emu->get_window());
			QTcpSocket *new_sock = new QTcpSocket();
			// IsOK always failed
//			if (!mSocket->IsOk()) {
//				logging->out_logf(LOG_ERROR, _T("CSocket%d: Couldn't create socket client."), m_ch);
//				return;
//			}
			mSocket = new_sock;
			mSocketMyself = true;

			// connect to server
			if (!ConnectMain(hostname, port)) {
				logging->out_logf(LOG_ERROR, _T("CSocket%d: Couldn't connect to server."), m_ch);
				return false;
			}

			// connected and writeable
			connect(&mTimer, SIGNAL(timeout()), this, SLOT(onTimerSlot()));
			mTimer.start(30);

			// setup receive event
			connect(mSocket, SIGNAL(readyRead()), this, SLOT(onReadSlot()));
			connect(this, SIGNAL(readyWrite()), this, SLOT(onWriteSlot()));

		} else {

//			QUdpSocket *new_sock = new QUdpSocket(emu->get_window());
			QUdpSocket *new_sock = new QUdpSocket();

			// Create udp socket
			mSocket = new_sock;
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
bool CSocket::Connect(CSocket *server, QAbstractSocket *sock)
{
	if (IsConnecting()) {
		return true;
	}

	m_connecting = false;
//	emu = parent->GetEMU();
//	m_busy = false;

	mSocket = sock;
	mSocketMyself = false;
//	mServer = nullptr;
	is_server = false;
	mConnServer = server;

	if (mConnServer) {
		if (mConnServer->numClient() < 1) {
			mConnServer->IncreaseClient();
		} else {
			// no more accept
			return false;
		}
	}

	// connected and writeable
	connect(&mTimer, SIGNAL(timeout()), this, SLOT(onTimerSlot()));
	mTimer.start(30);

	if (!recv_buffer) recv_buffer = new FIFOCHAR(SOCKET_BUFFER_MAX);
	if (!send_buffer) send_buffer = new FIFOCHAR(SOCKET_BUFFER_MAX);
	if (!recv_mux) recv_mux = new CMutex();
	if (!send_mux) send_mux = new CMutex();

	// setup receive event
	connect(mSocket, SIGNAL(readyRead()), this, SLOT(onReadSlot()));
	connect(this, SIGNAL(readyWrite()), this, SLOT(onWriteSlot()));

	// connection ok
//	connect(mSocket, SIGNAL(connected()), this, SLOT(onConnectSlot()));
//	onConnectSlot();
//	connect(mSocket, SIGNAL(disconnected()), this, SLOT(onDisconnectSlot()));
	connect(mSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(onStateChangedSlot(QAbstractSocket::SocketState)));

//	sock->SetEventHandler(*this, IDS_SOCKET_ID);
//	sock->SetNotify(wxSOCKET_INPUT_FLAG | wxSOCKET_LOST_FLAG);
//	sock->Notify(true);

	logging->out_logf(LOG_DEBUG, _T("CSocket%d: Accepted client."), m_ch);

	m_connecting = true;
	return m_connecting;
}

bool CSocket::ConnectMain(const _TCHAR *hostname, int port)
{
	if (!mSocket) {
		return false;
	}

	QString qhostname(QTChar::fromTChar(hostname));
	QHostInfo info = QHostInfo::fromName(qhostname);
	if (info.error() != QHostInfo::NoError) {
		return false;
	}

	mSocket->connectToHost(qhostname, static_cast<quint16>(port));
	mSocket->waitForConnected(500);

	// connection ok
	connect(mSocket, SIGNAL(connected()), this, SLOT(onConnectSlot()));
//	connect(mSocket, SIGNAL(disconnected()), this, SLOT(onDisconnectSlot()));
	connect(mSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(onStateChangedSlot(QAbstractSocket::SocketState)));

//	QTChar name(hostname);
	logging->out_logf(LOG_DEBUG, _T("CSocket%d: Client connection to %ls:%d"), m_ch, hostname, port);

	return true;
}

void CSocket::Disconnect()
{
	// disconnect
	emu->socket_disconnected(m_ch);
//	conn->OnDestroy(m_ch);
	m_connecting = false;
	device = nullptr;
	delete mServer;
	mServer = nullptr;
	if (mSocket) {
		mSocket->disconnectFromHost();
		if (mSocketMyself) delete mSocket;
	}
	mSocket = nullptr;
	mSocketMyself = false;
	if (mConnServer) {
		if (mConnServer->numClient() > 0) {
			mConnServer->DecreaseClient();
		}
	}
	mConnServer = nullptr;
	m_numClient = 0;
}

void CSocket::Shutdown()
{
}

void CSocket::SetUDPAddress(const _TCHAR *hostname, int port)
{
	QString qhostname(QTChar::fromTChar(hostname));
	QHostInfo info = QHostInfo::fromName(qhostname);
	if (!info.addresses().isEmpty()) {
		mUDPaddress = info.addresses().at(0);
	}
	mUDPport = (quint16)port;
}

void CSocket::IncreaseClient()
{
	m_numClient++;
}

void CSocket::DecreaseClient()
{
	m_numClient--;
}
int CSocket::numClient()
{
	return m_numClient;
}

void CSocket::onTimerSlot()
{
	mTimer.stop();

	// client connected
	emu->socket_connected(m_ch);
	// and writeable
	emu->socket_writeable(m_ch);
	// send data
	emu->send_data(m_ch);
}

void CSocket::onAcceptSlot()
{
	if (!mServer) return;

	QTcpSocket *sock = mServer->nextPendingConnection();
	if (sock) {
		QString hostname = sock->peerName();
		if (hostname.length() == 0) {
			logging->out_logf(LOG_DEBUG, _T("CSocket%d: New connection from unknown client accepted."), m_ch);
		} else {
			QTChar name(hostname);
			int port = sock->peerPort();
			logging->out_logf(LOG_DEBUG, _T("CSocket%d: New client connection from %ls:%u accepted"),
                   m_ch, name.toTChar(), port);
		}
//		QIODevice::OpenMode mode = sock->openMode();

		if (!m_conn->Accept(m_ch, this, sock)) {
//			sock->close();
			logging->out_logf(LOG_ERROR, _T("CSocket%d: New client connection forcelly destroyed."), m_ch);
			return;
		}

	} else {
		logging->out_logf(LOG_ERROR, _T("CSocket%d: Accept Error"), m_ch);
		return;
	}
}

void CSocket::onReadSlot()
{
//	// receive data
//	emu->recv_data(m_ch);

	// store to buffer
	char buf[1024];
	int size = Recv(buf, sizeof(buf));
	recv_buffer->write(buf, size);

	// readable
	emu->socket_readable(m_ch);
}

void CSocket::onWriteSlot()
{
	char buf[1024];
	int size = send_buffer->read(buf, sizeof(buf));
	Send(buf, size);
}

void CSocket::onConnectSlot()
{
	if (!mSocket) return;

	// client connected
	emu->socket_connected(m_ch);
	// and writeable
	emu->socket_writeable(m_ch);
	// send data
	emu->send_data(m_ch);

	logging->out_logf(LOG_DEBUG, _T("CSocket%d: onConnectSlot"), m_ch);
}

//void CSocket::onDisconnectSlot()
//{
//	if (!mSocket) return;
//
//	logging->out_logf(LOG_DEBUG, _T("CSocket%d: onDisconnectSlot"), m_ch);
//}

void CSocket::onStateChangedSlot(QAbstractSocket::SocketState state)
{
	if (!mSocket || !m_connecting) return;

	if (state == QAbstractSocket::SocketState::UnconnectedState) {
		Disconnect();
	}

	logging->out_logf(LOG_DEBUG, _T("CSocket%d: onStateChangedSlot"), m_ch);
}

int CSocket::Send(const char *buffer, int len)
{
	if (!mSocket) return 0;

	len = static_cast<int>(mSocket->write(buffer, len));
	if (len < 0) {
		// error
		QAbstractSocket::SocketError err = mSocket->error();
		QTChar errstr(mSocket->errorString());
		logging->out_logf(LOG_DEBUG, _T("CSocket::Send: ch=%d: Socket write error:%d %s"),
				m_ch, err, errstr.toTChar());
		Disconnect();
		len = 0;
	} else {
		mSocket->flush();
	}
	return len;
}
int CSocket::Recv(char *buffer, int len)
{
	if (!mSocket) return 0;

	len = static_cast<int>(mSocket->read(buffer, len));
	if (len < 0) {
		// error
		QAbstractSocket::SocketError err = mSocket->error();
		QTChar errstr(mSocket->errorString());
		logging->out_logf(LOG_DEBUG, _T("CSocket::Recv: ch=%d: Socket read error:%d %s"),
				m_ch, err, errstr.toTChar());
		Disconnect();
		len = 0;
	}
	return len;
}
size_t CSocket::SendTo(const char *buffer, size_t len)
{
	if (!is_tcp) return 0;

	len = ((QUdpSocket *)mSocket)->writeDatagram(buffer, len, mUDPaddress, mUDPport);
	if (len < 0) {
		// error
		QAbstractSocket::SocketError err = mSocket->error();
		QTChar errstr(mSocket->errorString());
		logging->out_logf(LOG_DEBUG, _T("CSocket::SendTo: ch=%d: Socket read error:%d %s"),
				m_ch, err, errstr.toTChar());
		Disconnect();
		len = 0;
	}
	return len;
}
size_t CSocket::RecvFrom(char *ipaddr, int *port, char *buffer, size_t len)
{
	if (!is_tcp) return 0;

	QHostAddress qaddr;
	quint16 qport = 0;
	len = ((QUdpSocket *)mSocket)->readDatagram(buffer, len, &qaddr, &qport);
	if (len < 0) {
		// error
		QAbstractSocket::SocketError err = mSocket->error();
		QTChar errstr(mSocket->errorString());
		logging->out_logf(LOG_DEBUG, _T("CSocket::RecvFrom: ch=%d: Socket read error:%d %s"),
				m_ch, err, errstr.toTChar());
		Disconnect();
		len = 0;
	} else {
		if (ipaddr) *ipaddr = qaddr.toIPv4Address();
		if (port) *port = qport;
	}
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
	: QObject()
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

bool Connection::Accept(int ch, CSocket *server, QAbstractSocket *sock)
{
	int new_ch = GetEmptySocket();
	if (new_ch < 0) {
		return false;
	}
	if (!socs.Item(new_ch)->Connect(server, sock)) {
		return false;
	}

	DEVICE *dev = GetDevice(ch);
	SetDevice(new_ch, dev);

	emu->socket_accepted(ch, new_ch, socs.Item(new_ch)->IsTCP());
	return true;
}
#if 0
void Connection::Disconnect(int ch)
{
	if (ch >= 0 && ch < mSocket.Count()) {
		mSocket[ch]->Disconnect();
	}
}
#endif
int Connection::GetEmptySocket() const
{
	int new_ch = -1;
	for(int i = 0; i < socs.Count(); i++) {
		if (!socs.Item(i)->IsConnecting()) {
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
	if (ch >= 0 && ch < mSocketMax && mSocket[ch]->IsOk()) {
		return mSocket[ch]->Send(buffer, len);
	} else {
		return 0;
	}
}
size_t Connection::Recv(int ch, char *buffer, size_t len)
{
	if (ch >= 0 && ch < mSocketMax && mSocket[ch]->IsOk()) {
		return mSocket[ch]->Recv(buffer, len);
	} else {
		return 0;
	}
}
size_t Connection::SendTo(int ch, const _TCHAR *hostname, int port, const void *buffer, size_t len)
{
	if (ch >= 0 && ch < mSocketMax && mSocket[ch]->IsOk()) {
		return mSocket[ch]->SendTo(hostname, port, buffer, len);
	} else {
		return 0;
	}
}
size_t Connection::RecvFrom(int ch, char *ipaddr, int *port, void *buffer, size_t len)
{
	if (ch >= 0 && ch < mSocketMax && mSocket[ch]->IsOk()) {
		return mSocket[ch]->RecvFrom(ipaddr, port, buffer, len);
	} else {
		return 0;
	}
}
#endif
//void Connection::OnDestroy(int ch)
//{
//	if (ch >= 0 && ch < mSocketMax) {
//		mSocket[ch]->Disconnect();
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
