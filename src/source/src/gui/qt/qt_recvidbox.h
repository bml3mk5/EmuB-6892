/** @file qt_recvidbox.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2019.03.01

	@brief [ qt record video box ]
*/

#ifndef QT_RECVIDBOX_H
#define QT_RECVIDBOX_H

#include <QDialog>
#include <QTabWidget>
#include <QComboBox>


//namespace Ui {
//class MyRecVidBox;
//}

/**
	@brief Record video dialog box
*/
class MyRecVidBox : public QDialog
{
	Q_OBJECT

public:
	explicit MyRecVidBox(QWidget *parent = nullptr);
	~MyRecVidBox();

	int exec();
	void setDatas();

private:
//	Ui::MyRecVidBox *ui;
	QList<int> codnums;
	QList<int> quanums;
	QList<int> enables;

	QTabWidget *tab;

private slots:
	void codeChangedSlot(int idx);
	void qualityChangedSlot(int idx);
};

#endif // QT_RECVIDBOX_H
