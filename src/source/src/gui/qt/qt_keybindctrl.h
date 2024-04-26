/** @file qt_keybindctrl.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2024.02.04

	@brief [ qt keybind control ]
*/

#ifndef QT_KEYBINDCTRL_H
#define QT_KEYBINDCTRL_H

#include <QDialog>
#include <QTableWidget>
#include "../gui_keybinddata.h"
#include <vector>

class QTimer;

/**
	@brief Keybind table
*/
class MyTableWidget : public QTableWidget
{
	Q_OBJECT

public:
	MyTableWidget(int tab_num, QWidget *parent = Q_NULLPTR);
	~MyTableWidget();

	void setData();
	void loadPreset(int num);
	void savePreset(int num);

protected:
	void keyPressEvent(QKeyEvent *event);
//	void keyReleaseEvent(QKeyEvent *event);

private:
	KeybindData *kbdata;
	int m_tab_num;

	QTimer *timer;

	void clearCellByVkCode(int code);
	void clearCell(QTableWidgetItem *itm);
	void setKeyCell(QTableWidgetItem *itm, int code);
	void setJoyCell(QTableWidgetItem *itm, uint32_t code0, uint32_t code1);

public slots:
	void update();
	void updateJoy();

private slots:
	void cellDoubleClick(int row, int column);
};

#endif // QT_KEYBINDCTRL_H
