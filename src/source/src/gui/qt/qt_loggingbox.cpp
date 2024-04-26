/** @file qt_loggingbox.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2024.04.14

	@brief [ qt log box ]
*/

#include "qt_loggingbox.h"
#include "qt_dialog.h"
#include "../../logging.h"
#include "../../msgs.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QTextEdit>

MyLoggingBox::MyLoggingBox(QWidget *parent) :
	QDialog(parent)
{
	p_buffer = NULL;
	m_buffer_size = 0;

	setWindowTitle(CMSG(Log));
	setModal(false);

	QVBoxLayout *vbox_all = new QVBoxLayout(this);

	QLineEdit *txtPath = new QLineEdit(this);
	txtPath->setReadOnly(true);
	txtPath->setMinimumWidth(480);
	vbox_all->addWidget(txtPath);

	txtLog = new QTextEdit(this);
	txtLog->setReadOnly(true);
	txtLog->setMinimumSize(480, 320);
	vbox_all->addWidget(txtLog);

	QHBoxLayout *hbox = new QHBoxLayout();

	MyPushButton *btnUpdate = new MyPushButton(CMsg::Update, this);
	connect(btnUpdate, SIGNAL(pressed()), this, SLOT(setData()));
	hbox->addWidget(btnUpdate);
	MyPushButton *btnClose = new MyPushButton(CMsg::Close, this);
	connect(btnClose, SIGNAL(pressed()), this, SLOT(hide()));
	hbox->addWidget(btnClose);

	vbox_all->addLayout(hbox);

	txtPath->setText(logging->get_log_path());
}

MyLoggingBox::~MyLoggingBox()
{
	delete [] p_buffer;
	p_buffer = NULL;
	m_buffer_size = 0;
}

void MyLoggingBox::hide()
{
	delete [] p_buffer;
	p_buffer = NULL;
	m_buffer_size = 0;
	QDialog::hide();
}

void MyLoggingBox::setData()
{
	if (!p_buffer) {
		m_buffer_size = 1024 * 1024;
		p_buffer = new char[m_buffer_size];
	}
	p_buffer[0] = 0;
	logging->get_log(p_buffer, m_buffer_size);
	txtLog->setText(p_buffer);
}

