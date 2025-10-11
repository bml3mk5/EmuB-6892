/** @file qt_keybindbox.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2016.03.01

	@brief [ qt keybind box ]
*/

#ifndef QT_KEYBINDBOX_H
#define QT_KEYBINDBOX_H

#include <QDialog>
#include <QTableWidget>
#include "qt_keybindctrl.h"
#include <vector>

class MyTabWidget;

namespace Ui {
class MyKeybindBox;
}

/**
	@brief Keybind dialog box
*/
class MyKeybindBox : public MyKeybindBaseBox
{
	Q_OBJECT

public:
	explicit MyKeybindBox(QWidget *parent = nullptr);
	~MyKeybindBox();

private:
	void setData();
};

#endif // QT_KEYBINDBOX_H
