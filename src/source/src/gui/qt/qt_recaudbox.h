/** @file qt_recaudbox.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2019.03.01

	@brief [ qt record audio box ]
*/

#ifndef QT_RECAUDBOX_H
#define QT_RECAUDBOX_H

#include <QDialog>
#include <QTabWidget>
#include <QComboBox>


//namespace Ui {
//class MyRecAudBox;
//}

/**
	@brief Record audio dialog box
*/
class MyRecAudBox : public QDialog
{
	Q_OBJECT

public:
	explicit MyRecAudBox(QWidget *parent = nullptr);
	~MyRecAudBox();

	int exec();
	void setDatas();

private:
//	Ui::MyRecAudBox *ui;
	QList<int> codnums;
//	QList<int> quanums;
	QList<int> enables;

	QTabWidget *tab;

private slots:
	void codeChangedSlot(int idx);
	void qualityChangedSlot(int idx);
};

#endif // QT_RECAUDBOX_H
