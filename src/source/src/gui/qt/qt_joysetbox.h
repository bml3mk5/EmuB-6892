/** @file qt_joysetbox.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2023.01.08

	@brief [ qt volume box ]
*/

#ifndef QT_JOYSETBOX_H
#define QT_JOYSETBOX_H

#include <QDialog>
#include <QLabel>
#include "../../emu.h"
#include "qt_keybindctrl.h"

class MySlider;

/**
	@brief Volume dialog box
*/
class MyJoySettingBox : public QDialog
{
	Q_OBJECT

private:
	void SetData();

	std::vector<MyTableWidget *> tables;
	int curr_tab;
	uint32_t joy_mask;

	MySlider *mash[MAX_JOYSTICKS][KEYBIND_JOY_BUTTONS];
	MySlider *axis[MAX_JOYSTICKS][6];

public:
	explicit MyJoySettingBox(QWidget *parent = nullptr);
	~MyJoySettingBox();

private slots:
	void accept();
	void loadPreset();
	void savePreset();
	void update();
	void tabChanged(int index);
	void toggleAxis(bool checked);
//	void moveSlider1(int num);
};

#endif // QT_JOYSETBOX_H
