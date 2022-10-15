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
#include "../gui_keybinddata.h"

namespace Ui {
class MyKeybindBox;
}

/**
	@brief Keybind table
*/
class MyTableWidget : public QTableWidget
{
	Q_OBJECT

public:
	MyTableWidget(int tab, KeybindData *data, QWidget *parent = Q_NULLPTR);
//	MyTableWidget(int rows, int columns, QWidget *parent = Q_NULLPTR) : QTableWidget(rows, columns, parent) {}

protected:
	void keyPressEvent(QKeyEvent *event);
//	void keyReleaseEvent(QKeyEvent *event);

private:
	KeybindData *kbdata;
	int tab_num;

public slots:
	void update();

private slots:
	void cellDoubleClick(int row, int column);
};

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
	MyTableWidget *tables[KEYBIND_MAX_NUM];
	KeybindData *kbdata[KEYBIND_MAX_NUM];
	int curr_tab;

	void setData();

private slots:
	void loadPreset();
	void savePreset();
	void accept();
	void update();

};

#endif // QT_KEYBINDBOX_H
