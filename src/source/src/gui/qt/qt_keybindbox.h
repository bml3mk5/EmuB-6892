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

namespace Ui {
class MyKeybindBox;
}

/**
	@brief Keybind dialog box
*/
class MyKeybindBox : public QDialog
{
	Q_OBJECT

public:
	explicit MyKeybindBox(QWidget *parent = nullptr);
	~MyKeybindBox();

private:
//	Ui::MyKeybindBox *ui;
	std::vector<MyTableWidget *> tables;
	int curr_tab;
	uint32_t joy_mask;

	void setData();

private slots:
	void loadPreset();
	void savePreset();
	void accept();
	void update();
	void tabChanged(int index);
	void toggleAxis(bool checked);
};

#endif // QT_KEYBINDBOX_H
