/** @file qt_uart.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2021.05.05 -

	@brief [ qt uart ]
*/

#include "../../vm/vm_defs.h"

#ifdef USE_UART

#ifndef QT_UART_H
#define QT_UART_H

#include "../../emu_osd.h"
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include "../../cptrlist.h"
#include "../../fifo.h"
#include "../../cmutex.h"

/**
	@brief Manage a serial port on host
*/
class CommPort : protected QSerialPort
{
	Q_OBJECT

protected:
	int     m_ch;
	DEVICE *device;
	QSerialPortInfo info;
	FIFOCHAR *recv_buffer;
	FIFOCHAR *send_buffer;
	CMutex *recv_mux;
	CMutex *send_mux;

public:
	explicit CommPort(int ch);
	virtual ~CommPort();

//	CommPort &operator=(const CommPort &src);

	int Open();
	void Close();
	bool IsOpened() const;
	void Clear();
	int ReadFromDevice();
	int SendData(const char *buf, int size);
	int WriteToDevice();
	int RecvData(char *buf, int size);

//	void SetName(const _TCHAR *name);
//	void SetDescription(const _TCHAR *desc);
	void SetInfo(const QSerialPortInfo &ninfo);
	void SetDevice(DEVICE *dev);
	void SetBaudRate(int rate);
	void SetDataBit(int bit);
	void SetParity(int parity);
	void SetStopBit(int bit);
	void SetFlowControl(int flow);

	int GetName(_TCHAR *name, size_t size) const;
	int GetDescription(_TCHAR *desc, size_t size) const;
	int GetNameDescription(_TCHAR *str, size_t size) const;
	DEVICE *GetDevice() const;

	bool CompareName(const _TCHAR *name) const;
	bool CompareName(const QSerialPortInfo &ninfo) const;

//	void Lock();
//	void Unlock();

private slots:
	void readDataSlot();
	void writeDataSlot();

signals:
	void readyWrite();
};

/**
	@brief Manage CommPort list on host
*/
class CommPorts : public CPtrList<CommPort>
{
private:

public:
	CommPorts();
	virtual ~CommPorts();

	int Enum();
	int Find(const TCHAR *name);
	int Find(const QSerialPortInfo &ninfo);

	int GetName(int idx, TCHAR *name, size_t size) const;
	int GetDescription(int idx, TCHAR *name, size_t size) const;
	int GetNameDescription(int idx, TCHAR *str, size_t size) const;
};

#endif /* QT_UART_H */
#endif /* USE_UART */
