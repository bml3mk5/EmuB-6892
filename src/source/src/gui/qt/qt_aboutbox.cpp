/** @file qt_aboutbox.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2016.03.01

	@brief [ qt about box ]
*/

#include "qt_aboutbox.h"
#include "ui_qt_aboutbox.h"
#include "../../version.h"
#include "../../emu.h"
#include "../../osd/qt/qt_parseopt.h"
#include "../../utils.h"
#include "../../main.h"
#include <QtCoreVersion>
#include <QLibraryInfo>
#include <QPainter>

MyAboutBox::MyAboutBox(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::MyAboutBox)
{
	ui->setupUi(this);

	setWindowTitle("About");

	QString str;

	str = APP_NAME;
	str += "\n\n";
	str += "Version ";
	str += APP_VERSION;
	str += " \"";
	str += PLATFORM;
	str += "\"";
#ifdef _DEBUG
	str += " (DEBUG Version)";
#endif
	str += "\n";
	char edi[256];
	emu->get_edition_string(edi, sizeof(edi));
	str += edi;
	str += "\n\n";
	str += " using Qt version ";
	str += QTCORE_VERSION_STR;
	str += "\n";
	str += "  LibPath: ";
	str += QLibraryInfo::location(QLibraryInfo::LibrariesPath);
	str += "\n";
	str += "  LibExecPath: ";
	str += QLibraryInfo::location(QLibraryInfo::LibraryExecutablesPath);
	str += "\n\n";
	str += APP_COPYRIGHT;

	ui->label->setText(str);

	icon = new MyIcon(this);
	icon->setGeometry(8,64,64,64);
}

MyAboutBox::~MyAboutBox()
{
	delete icon;
	delete ui;
}

MyIcon::MyIcon(QWidget *parent) :
	QWidget(parent)
{
	QString iconfile = QTChar::fromTChar(options->get_res_path());
	iconfile += CONFIG_NAME;
    iconfile += ".png";
	img = new QImage(iconfile);
}

MyIcon::~MyIcon()
{
	delete img;
}

void MyIcon::paintEvent(QPaintEvent *)
{
	if (img) {
		QPainter qp(this);
		qp.drawImage(0, 0, *img);
	}
}
