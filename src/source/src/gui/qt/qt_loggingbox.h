/** @file qt_loggingbox.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2024.04.14

	@brief [ qt log box ]
*/

#ifndef QT_LOGGINGBOX_H
#define QT_LOGGINGBOX_H

#include <QDialog>

class QTextEdit;

/**
	@brief Log dialog box
*/
class MyLoggingBox : public QDialog
{
	Q_OBJECT

public:
	explicit MyLoggingBox(QWidget *parent = nullptr);
	~MyLoggingBox();

private:
	QTextEdit *txtLog;

	char *p_buffer;
	int m_buffer_size;

private slots:
	void setData();

public slots:
	void hide();
};

#endif // QT_LOGGINGBOX_H
