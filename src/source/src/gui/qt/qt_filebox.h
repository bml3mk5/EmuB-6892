/** @file qt_filebox.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2022.11.01

	@brief [ qt file box ]
*/

#ifndef QT_FILEBOX_H
#define QT_FILEBOX_H

#include "../../common.h"
#include "../../msgs.h"
#include <QFileDialog>

/**
	@brief file dialog box
*/
class MyFileBox : public QFileDialog
{
	Q_OBJECT

public:
	explicit MyFileBox(QWidget *parent, const QString &caption, bool save, const _TCHAR *directory, const char *filter);
	explicit MyFileBox(QWidget *parent, CMsg::Id caption, bool save, const _TCHAR *directory, const char *filter);
	~MyFileBox();

private:
	void setFilterTypes(const char *filter_str, bool save);

};

#endif // QT_FILEBOX_H
