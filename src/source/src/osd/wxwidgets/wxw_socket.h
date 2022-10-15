/** @file wxw_socket.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.09.01

	@brief [ wxw socket ]
*/

#ifndef WXW_SOCKET_H
#define WXW_SOCKET_H

#include <wx/socket.h>
#include "../../cptrlist.h"

//class EMU;
class DEVICE;
class Connection;
class FIFOCHAR;
class CMutex;

typedef wxIPV4address MyIPaddress;

/**
	@brief Manage a network socket
*/
class CSocket : public wxEvtHandler
{
private:
	Connection *m_conn;
	int m_ch;
	DEVICE *device;
	bool is_tcp;
	wxSocketBase *mSocket;
	MyIPaddress mUDPaddress;
	bool is_server;
	bool m_connecting;

	wxTimer mTimer;
	int mTimerType;

	FIFOCHAR *recv_buffer;
	CMutex *recv_mux;
	FIFOCHAR *send_buffer;
	CMutex *send_mux;

	static int read_and_remove_dup_iac(char *dst, int size, FIFOCHAR *buffer);
	static int write_and_escape_iac(const char *src, int size, FIFOCHAR *buffer);

	void SetUDPAddress(const _TCHAR *hostname, int port);

public:
	CSocket(Connection *conn, int ch);
	~CSocket();

	// event procedures
	void OnAcceptEvent(wxSocketEvent &);
	void OnConnectEvent(wxSocketEvent &);
	void OnSocketEvent(wxSocketEvent &);
	void OnSocketInputEvent(wxSocketEvent &);
	void OnSocketLostEvent(wxSocketEvent &);
	void OnTimerEvent(wxTimerEvent &);

	// function

	void SendToDevice();
	void RecvFromDevice();
	void RecvFromDevice(const _TCHAR *hostname, int port);

	bool InitTCP(DEVICE *dev, bool server);
	bool InitUDP(DEVICE *dev, bool server);

	bool Connect(const wxString &hostname, int port, bool server = false);
//	bool Connect(const wxString &hostname, const wxString &service, bool server);
	bool Connect(wxSocketBase *sock);

	bool ConnectMain(const wxString &hostname, int port);
//	bool ConnectMain(const wxString &hostname, const wxString &service);

	void Disconnect();
	void Shutdown();

	void SendData();
	void RecvData();

	bool IsConnecting() const;

	size_t Send(const char *buffer, size_t len);
	size_t Recv(char *buffer, size_t len);
	size_t SendTo(const char *buffer, size_t len);
	size_t RecvFrom(char *ipaddr, int *port, char *buffer, size_t len);

	void SetTCP(bool val) { is_tcp = val; }
	bool IsTCP() const { return is_tcp; }
	void SetDevice(DEVICE *dev) { device = dev; }
	DEVICE *GetDevice() { return device; }

	FIFOCHAR *GetRecvBuffer() { return recv_buffer; }
	FIFOCHAR *GetSendBuffer() { return send_buffer; }

	enum {
		IDS_ACCEPT_ID = 10,
		IDS_CONNECT_ID,
		IDS_SOCKET_ID,
		IDS_TIMER_ID,

		IDT_CONNECTED,
	};

	DECLARE_EVENT_TABLE()
};

/**
	@brief Manage CSocket list
*/
class Connection : public wxObject
{
private:
	CPtrList<CSocket> socs;

public:
	Connection();
	~Connection();

	void Initialize();

//	bool Connect(int new_ch, const _TCHAR *hostname, int port, bool server = false);
//	bool Connect(const _TCHAR *hostname, int port, bool server = false);

	bool Accept(int ch, wxSocketBase *sock, bool tcp);
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

#endif /* WXW_SOCKET_H */
