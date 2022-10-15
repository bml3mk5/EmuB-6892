/** @file qt_aboutbox.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2016.03.01

	@brief [ qt about box ]
*/

#ifndef QT_ABOUTBOX_H
#define QT_ABOUTBOX_H

#include <QDialog>

namespace Ui {
class MyAboutBox;
}

class MyIcon;

/**
	@brief About dialog box
*/
class MyAboutBox : public QDialog
{
	Q_OBJECT

public:
	explicit MyAboutBox(QWidget *parent = nullptr);
	~MyAboutBox();

private:
	Ui::MyAboutBox *ui;

	MyIcon *icon;
};

/**
	@brief Display any icons on a dialog
*/
class MyIcon : public QWidget
{
	Q_OBJECT

public:
	explicit MyIcon(QWidget *parent = nullptr);
	~MyIcon();

protected:
	void paintEvent(QPaintEvent *event);

private:
	QImage *img;
};

#endif // QT_ABOUTBOX_H
