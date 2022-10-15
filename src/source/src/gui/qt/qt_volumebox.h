/** @file qt_volumebox.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2016.03.01

	@brief [ qt volume box ]
*/

#ifndef QT_VOLUMEBOX_H
#define QT_VOLUMEBOX_H

#include <QDialog>
#include <QLabel>
#include "../../config.h"

//namespace Ui {
//class MyVolumeBox;
//}

/**
	@brief Volume dialog box
*/
class MyVolumeBox : public QDialog
{
	Q_OBJECT

public:
	explicit MyVolumeBox(QWidget *parent = nullptr);
	~MyVolumeBox();

private:
//	Ui::MyVolumeBox *ui;
	QLabel *lblVol[VOLUME_NUMS];
	int *p_volume[VOLUME_NUMS];
	bool *p_mute[VOLUME_NUMS];

	void setPtr();

private slots:
	void moveSlider1(int num);
	void changeMute1(int num);
};

#endif // QT_VOLUMEBOX_H
