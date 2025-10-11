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
#include "../../config.h"
#include "qt_keybindctrl.h"

class MyTabWidget;
class MySlider;
class MyCheckBox;
class MyRadioButton;

/**
	@brief Volume dialog box
*/
class MyJoySettingBox : public MyKeybindBaseBox
{
	Q_OBJECT

private:
	void setData();

#if defined(USE_PIAJOYSTICK) || defined(USE_KEY2JOYSTICK)
	MySlider *mash[MAX_JOYSTICKS][KEYBIND_JOY_BUTTONS];
	MySlider *axis[MAX_JOYSTICKS][6];
#endif
#if defined(USE_PIAJOYSTICK) || defined(USE_KEY2PIAJOYSTICK)
#ifdef USE_JOYSTICKBIT
	MyCheckBox *chkPiaJoyNeg;
	MyRadioButton *radPiaJoyConn[Config::PIAJOY_CONN_TO_MAX];
#else
	MyCheckBox *chkPiaJoyNoIrq;
#endif
#endif
#if defined(USE_PSGJOYSTICK) || defined(USE_KEY2PSGJOYSTICK)
#ifdef USE_JOYSTICKBIT
	MyCheckBox *chkPsgJoyNeg;
#endif
#endif

public:
	explicit MyJoySettingBox(QWidget *parent = nullptr);
	~MyJoySettingBox();
};

#endif // QT_JOYSETBOX_H
