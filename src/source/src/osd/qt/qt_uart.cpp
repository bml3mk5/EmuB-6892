/** @file qt_uart.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2021.05.05 -

	@brief [ qt uart ]
*/

#include "qt_uart.h"

#ifdef USE_UART
#include "../../emu_osd.h"
#include "../../main.h"
#include "../../gui/qt/qt_gui.h"
#include <QtSerialPort/QSerialPortInfo>
#include <QtSerialPort/QSerialPort>
#include "../../config.h"
#include "../../vm/device.h"
#include "../../utility.h"

#if 0
static const QSerialPort::BaudRate baud_rates[] = {
	QSerialPort::Baud1200,
	QSerialPort::Baud2400,
	QSerialPort::Baud4800,
	QSerialPort::Baud9600,
	QSerialPort::Baud19200,
	QSerialPort::Baud38400,
	QSerialPort::Baud57600,
	QSerialPort::Baud115200,
	QSerialPort::UnknownBaud
};
#endif

void EMU_OSD::EMU_UART()
{
}

void EMU_OSD::initialize_uart()
{
}

void EMU_OSD::release_uart()
{
}

int EMU_OSD::enum_uarts()
{
	mainwindow->coms->Enum();
	return mainwindow->coms->Count();
}

void EMU_OSD::update_uart()
{
	// buffer copy
	for(int i = 0; i < mainwindow->coms->Count(); i++) {
		recv_uart_data(i);
	}
}

void EMU_OSD::get_uart_description(int ch, _TCHAR *buf, size_t size)
{
	mainwindow->coms->GetNameDescription(ch, buf, size);
}

bool EMU_OSD::init_uart(int ch, DEVICE *dev)
{
	if (ch < 0 || mainwindow->coms->Count() <= ch) {
		logging->out_logf(LOG_ERROR, _T("EMU::init_uart: invalid uart channel. ch=%d"), ch);
		return false;
	}
	CommPort *com = mainwindow->coms->Item(ch);
	com->SetDevice(dev);
	return true;
}

bool EMU_OSD::open_uart(int ch)
{
	if (ch < 0 || mainwindow->coms->Count() <= ch) return false;
	return (mainwindow->coms->Item(ch)->Open() != 0);
}

bool EMU_OSD::is_opened_uart(int ch)
{
	if (ch < 0 || mainwindow->coms->Count() <= ch) return false;
	return mainwindow->coms->Item(ch)->IsOpened();
}

void EMU_OSD::close_uart(int ch)
{
	if (ch < 0 || mainwindow->coms->Count() <= ch) return;
	mainwindow->coms->Item(ch)->Close();
}

int EMU_OSD::send_uart_data(int ch, const uint8_t *data, int size)
{
	if (ch < 0 || mainwindow->coms->Count() <= ch) return -1;

	CommPort *com = mainwindow->coms->Item(ch);
	if (!com->IsOpened()) return -1;

	size = com->SendData((const char *)data, size);

	return size;
}

/// @note Use following functions:
/// get_sendbuffer, inc_sendbuffer_ptr
void EMU_OSD::send_uart_data(int ch)
{
	if (ch < 0 || mainwindow->coms->Count() <= ch) return;

	CommPort *com = mainwindow->coms->Item(ch);
	if (!com->IsOpened()) return;

	com->ReadFromDevice();
}

void EMU_OSD::recv_uart_data(int ch)
{
	if (ch < 0 || mainwindow->coms->Count() <= ch) return;

	CommPort *com = mainwindow->coms->Item(ch);
	if (!com->IsOpened()) return;

	com->WriteToDevice();
}

////////////////////////////////////////
// COM port on Qt
////////////////////////////////////////

CommPort::CommPort(int ch)
	: QSerialPort()
{
	m_ch = ch;
	device = NULL;
	recv_mux = NULL;
	send_mux = NULL;

	recv_buffer = NULL;
	send_buffer = NULL;

	connect(this, SIGNAL(readyRead()), this, SLOT(readDataSlot()));
	connect(this, SIGNAL(readyWrite()), this, SLOT(writeDataSlot()));
}

CommPort::~CommPort()
{
	Close();
	delete recv_buffer;
	delete send_buffer;
	delete recv_mux;
	delete send_mux;
}

//CommPort &CommPort::operator=(const CommPort &src)
//{
//	Close();
//	info = src.info;
//	return *this;
//}

int CommPort::Open()
{
	if (info.isNull()) return 0;

	if (!recv_mux) recv_mux = new CMutex();
	if (!send_mux) send_mux = new CMutex();
	if (!recv_buffer) recv_buffer = new FIFOCHAR(UART_BUFFER_MAX);
	if (!send_buffer) send_buffer = new FIFOCHAR(UART_BUFFER_MAX);

	setPort(info);

	recv_mux->lock();
	send_mux->lock();
	if (!open(QIODevice::ReadWrite)) {
		send_mux->unlock();
		recv_mux->unlock();
		return 0;
	}

	SetBaudRate(config.comm_uart_baudrate);
	SetDataBit(config.comm_uart_databit);
	SetParity(config.comm_uart_parity);
	SetStopBit(config.comm_uart_stopbit);
	SetFlowControl(config.comm_uart_flowctrl);

	Clear();

	send_mux->unlock();
	recv_mux->unlock();

	return 1;
}

void CommPort::Close()
{
	if (isOpen()) {
		recv_mux->lock();
		send_mux->lock();
		close();
		send_mux->unlock();
		recv_mux->unlock();
	}
}

bool CommPort::IsOpened() const
{
	return isOpen();
}

void CommPort::Clear()
{
	if (!isOpen()) return;

	// clear buffer
	clear();

	if (recv_buffer) recv_buffer->clear();
	if (send_buffer) send_buffer->clear();
}

int CommPort::ReadFromDevice()
{
	// get send buffer and data size
	int size;
	int flags = 0;
	uint8_t* buf;

	if (!device) {
		return 0;
	}

	// loop until send buffer becomes empty
	while(1) {
		buf = device->get_sendbuffer(m_ch, &size, &flags);
		if(!buf || !size) {
			break;
		}

		if((size = SendData((const char *)buf, size)) <= 0) {
			break;
		}

		device->inc_sendbuffer_ptr(m_ch, size);
	}

	return size;
}

int CommPort::SendData(const char *buf, int size)
{
	if (!isOpen()) {
		return -1;
	}

	send_mux->lock();
	int len = 0;
	int bsize = send_buffer->remain();
	for(int i=0; i<bsize && i<size; i++) {
		send_buffer->write(buf[i]);
		len++;
	}
	send_mux->unlock();
	if (len < 0) {
//		QSerialPort::SerialPortError err = port.error();
		Close();
		return -1;
	}
	emit readyWrite();

	return (int)len;
}

int CommPort::WriteToDevice()
{
	if (!device) {
		return 0;
	}

	// get buffer
	int size0 = 0;
	int size1 = 0;
	int flags = 0;
	uint8_t* buf0 = device->get_recvbuffer0(m_ch, &size0, &size1, &flags);
	uint8_t* buf1 = device->get_recvbuffer1(m_ch);

	int size = 0;
	size = RecvData((char *)buf0, size0);
	if (size1 > 0) {
		size += RecvData((char *)buf1, size1);
	}
	device->inc_recvbuffer_ptr(m_ch, size);

	return size;
}

/// @return written size (bytes)
int CommPort::RecvData(char *buf, int size)
{
	if (!isOpen()) {
		return -1;
	}

	recv_mux->lock();
	int len = 0;
	int bsize = recv_buffer->count();
	for(int i=0; i<bsize && i<size; i++) {
		buf[i] = recv_buffer->read();
		len++;
	}
	recv_mux->unlock();
	if (len < 0) {
		Close();
		return -1;
	}

//	buf[len] = 0;
	return (int)len;
}

//void CommPort::SetName(const _TCHAR *name)
//{
//}

//void CommPort::SetDescription(const _TCHAR *desc)
//{
//}

void CommPort::SetInfo(const QSerialPortInfo &ninfo)
{
	info = ninfo;
}

void CommPort::SetDevice(DEVICE *dev)
{
	device = dev;
}

void CommPort::SetBaudRate(int rate)
{
	setBaudRate(rate);
}

void CommPort::SetDataBit(int bit)
{
	QSerialPort::DataBits b;
	if (bit == 7) {
		b = QSerialPort::Data7;
	} else {
		b = QSerialPort::Data8;
	}
	setDataBits(b);
}

void CommPort::SetParity(int parity)
{
	QSerialPort::Parity p;
	switch(parity) {
	case 1:
		p = QSerialPort::OddParity;
		break;
	case 2:
		p = QSerialPort::EvenParity;
		break;
	default:
		p = QSerialPort::NoParity;
		break;
	}
	setParity(p);
}

void CommPort::SetStopBit(int bit)
{
	QSerialPort::StopBits b;
	switch(bit){
	case 1:
		b = QSerialPort::OneStop;
		break;
	default:
		b = QSerialPort::TwoStop;
		break;
	}
	setStopBits(b);
}

void CommPort::SetFlowControl(int flow)
{
	QSerialPort::FlowControl f;
	switch(flow) {
	case 1:
		// Xon/off
		f = QSerialPort::SoftwareControl;
		break;
	case 2:
		// Hardware
		f = QSerialPort::HardwareControl;
		break;
	default:
		// none
		f = QSerialPort::NoFlowControl;
		break;
	}
	setFlowControl(f);
}

int CommPort::GetName(_TCHAR *name, size_t size) const
{
	QString str = info.portName();
	memset(name, 0, size);
	UTILITY::tcscpy(name, size, str.toUtf8());
	return (int)_tcslen(name);
}

int CommPort::GetDescription(_TCHAR *desc, size_t size) const
{
	QString str = info.description();
	memset(desc, 0, size);
	UTILITY::tcscpy(desc, size, str.toUtf8());
	return (int)_tcslen(desc);
}

int CommPort::GetNameDescription(_TCHAR *str, size_t size) const
{
	int n = GetName(str, size);
	str[n++] = ' '; str[n++] = '(';
	n += GetDescription(&str[n], size - n);
	str[n++] = ')'; str[n++] = 0;
	return n;
}

DEVICE *CommPort::GetDevice() const
{
	return device;
}

bool CommPort::CompareName(const _TCHAR *name) const
{
	QString str = info.portName();
	return (_tcscmp(str.toUtf8(), name) == 0);
}

bool CommPort::CompareName(const QSerialPortInfo &ninfo) const
{
	return (info.portName() == ninfo.portName());
}

//void CommPort::Lock()
//{
//	if (mux) mux->lock();
//}

//void CommPort::Unlock()
//{
//	if (mux) mux->unlock();
//}

void CommPort::readDataSlot()
{
	recv_mux->lock();
	int len = recv_buffer->remain();
	for(int i=0; i<len; i++) {
		char c;
		if (!getChar(&c)) break;
		recv_buffer->write(c);
	}
	recv_mux->unlock();
}

void CommPort::writeDataSlot()
{
	send_mux->lock();
	int len = send_buffer->count();
	for(int i=0; i<len; i++) {
		if (!putChar(send_buffer->read())) {
			send_buffer->rollback();
			break;
		}
	}
	send_mux->unlock();
}

//
// search in /proc/tty/drivers
//

CommPorts::CommPorts()
	: CPtrList<CommPort>()
{
}

CommPorts::~CommPorts()
{
}

//CommPort &CommPorts::operator[](int idx)
//{
//	return cPorts[idx];
//}

/// Enumrate serial ports
///
/// @return number of serial port
int CommPorts::Enum()
{
	QList<QSerialPortInfo> list = QSerialPortInfo::availablePorts();

	int ch = 0;
	for(int idx = 0; idx < list.count(); idx++) {
		QSerialPortInfo *info = &list[idx];
		if (Find(*info) < 0) {
			CommPort *com = new CommPort(ch);
			com->SetInfo(*info);
			Add(com);
			ch++;
		}
	}

	logging->out_debugf("Enum: count: %d", (int)Count());
	return Count();
}

/// Search serial port by name
///
/// @param[in] name: port name
/// @return >=0: index number / -1: not found
int CommPorts::Find(const TCHAR *name)
{
	int match = -1;
	for(int i=0; i<Count(); i++) {
		if (Item(i)->CompareName(name)) {
			match = i;
			break;
		}
	}
	return match;
}

int CommPorts::Find(const QSerialPortInfo &ninfo)
{
	int match = -1;
	for(int i=0; i<Count(); i++) {
		if (Item(i)->CompareName(ninfo)) {
			match = i;
			break;
		}
	}
	return match;
}

int CommPorts::GetName(int idx, TCHAR *name, size_t size) const
{
	if (idx < Count()) return Item(idx)->GetName(name, size);
	else return 0;
}

int CommPorts::GetDescription(int idx, TCHAR *name, size_t size) const
{
	if (idx < Count()) return Item(idx)->GetDescription(name, size);
	else return 0;
}

int CommPorts::GetNameDescription(int idx, TCHAR *str, size_t size) const
{
	if (idx < Count()) return Item(idx)->GetNameDescription(str, size);
	else return 0;
}

#endif /* USE_UART */

