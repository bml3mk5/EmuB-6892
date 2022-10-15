/** @file qt_socket.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2017.03.01

	@brief [ qt socket ]
*/

#ifndef QT_SOCKET_H
#define QT_SOCKET_H

#include "../../common.h"
#include <QAbstractSocket>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QTcpServer>
#include <QHostInfo>
#include <QTimer>
#include "../../cptrlist.h"

//class EMU;
class DEVICE;
class Connection;
class FIFOCHAR;
class CMutex;

/**
	@brief Manage a network socket
*/
class CSocket : public QObject
{
	Q_OBJECT

private:
	Connection *m_conn;
	int m_ch;
	DEVICE *device;
	bool is_tcp;
	QAbstractSocket *mSocket;
	bool mSocketMyself;
	QTcpServer *mServer;
	bool is_server;
	bool m_connecting;
	CSocket *mConnServer;
	int m_numClient;
	QHostAddress mUDPaddress;
	quint16 mUDPport;

	QTimer mTimer;
	int mTimerType;

	FIFOCHAR *recv_buffer;
	CMutex *recv_mux;
	FIFOCHAR *send_buffer;
	CMutex *send_mux;

	void IncreaseClient();
	void DecreaseClient();
	int numClient();

	static int read_and_remove_dup_iac(char *dst, int size, FIFOCHAR *buffer);
	static int write_and_escape_iac(const char *src, int size, FIFOCHAR *buffer);

	void SetUDPAddress(const _TCHAR *hostname, int port);

public:
	CSocket(Connection *conn, int ch);
	~CSocket();

	// slots
public slots:
	void onAcceptSlot();
	void onConnectSlot();
//	void onDisconnectSlot();
	void onStateChangedSlot(QAbstractSocket::SocketState state);
	void onReadSlot();
	void onWriteSlot();
	void onTimerSlot();

public:

	void SendToDevice();
	void RecvFromDevice();
	void RecvFromDevice(const _TCHAR *hostname, int port);

	bool InitTCP(DEVICE *dev, bool server);
	bool InitUDP(DEVICE *dev, bool server);

	bool Connect(const _TCHAR *hostname, int port, bool server = false);
	bool Connect(CSocket *server, QAbstractSocket *sock);

	bool ConnectMain(const _TCHAR *hostname, int port);
	void Disconnect();
	void Shutdown();

	void SendData();
	void RecvData();

	bool IsConnecting() const;

	int Send(const char *buffer, int len);
	int Recv(char *buffer, int len);
	size_t SendTo(const char *buffer, size_t len);
	size_t RecvFrom(char *ipaddr, int *port, char *buffer, size_t len);

	void SetTCP(bool val) { is_tcp = val; }
	bool IsTCP() const { return is_tcp; }
	FIFOCHAR *GetRecvBuffer() { return recv_buffer; }
	FIFOCHAR *GetSendBuffer() { return send_buffer; }
	void SetDevice(DEVICE *dev) { device = dev; }
	DEVICE *GetDevice() { return device; }

signals:
	void readyWrite();
};

/**
	@brief Manage CSocket list
*/
class Connection : public QObject
{
	Q_OBJECT

private:
	CPtrList<CSocket> socs;

public:
	Connection();
	~Connection();

	void Initialize();

//	bool Connect(const _TCHAR *hostname, int port, bool server = false);
//	bool Connect(int new_ch, const _TCHAR *hostname, int port, bool server = false);
	bool Accept(int ch, CSocket *server, QAbstractSocket *sock);
	int  GetEmptySocket() const;
//	void Disconnect(int ch);
//	size_t Send(int ch, const char *buffer, size_t len);
//	size_t Recv(int ch, char *buffer, size_t len);
//	size_t SendTo(int ch, const _TCHAR *hostname, int port, const void *buffer, size_t len);
//	size_t RecvFrom(int ch, char *ipaddr, int *port, void *buffer, size_t len);

//	void OnDestroy(int ch);

	CSocket *Item(int ch);

	void SetDevice(int ch, DEVICE *dev);
	DEVICE *GetDevice(int ch);
};

#endif /* QT_SOCKET_H */
