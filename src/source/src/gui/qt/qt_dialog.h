/** @file qt_dialog.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2019.03.01

	@brief [ qt dialog ]
*/

#ifndef QT_DIALOG_H
#define QT_DIALOG_H

#include "../../msgs.h"
#include "../../cchar.h"
#include "../../cptrlist.h"
#include <QTabWidget>
#include <QGroupBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QFileDialog>
#include <QDialogButtonBox>

/**
	@brief Tab control
*/
class MyTabWidget : public QTabWidget
{
	Q_OBJECT

public:
	explicit MyTabWidget(QWidget *parent = nullptr);

	int addTab(QWidget *page, CMsg::Id labelid);
};

/**
	@brief Group box
*/
class MyGroupBox : public QGroupBox
{
	Q_OBJECT

public:
	explicit MyGroupBox(CMsg::Id titleid, QWidget *parent = nullptr);

	void setTitleById(CMsg::Id titleid);
};

/**
	@brief Check button
*/
class MyCheckBox : public QCheckBox
{
	Q_OBJECT

public:
	explicit MyCheckBox(CMsg::Id textid, QWidget *parent = nullptr);

	void setTextById(CMsg::Id textid);
};

/**
	@brief Radio button
*/
class MyRadioButton : public QRadioButton
{
	Q_OBJECT

public:
	explicit MyRadioButton(CMsg::Id textid, QWidget *parent = nullptr);

	void setTextById(CMsg::Id textid);
};

/**
	@brief Label control
*/
class MyLabel : public QLabel
{
	Q_OBJECT

public:
	explicit MyLabel(CMsg::Id textid, QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

	void setTextById(CMsg::Id textid);
};

/**
	@brief Combo box
*/
class MyComboBox : public QComboBox
{
	Q_OBJECT

public:
	explicit MyComboBox(QWidget *parent = nullptr);
	explicit MyComboBox(QWidget *parent, const _TCHAR *list[], const QVariant &userData = QVariant());
	explicit MyComboBox(QWidget *parent, const CMsg::Id *ids, int selidx, int appendidx = -1, CMsg::Id appendstr = CMsg::End, const QVariant &userData = QVariant());
	explicit MyComboBox(QWidget *parent, const CPtrList<CTchar> &list, const QVariant &userData = QVariant());

	void addItemsById(const CMsg::Id *ids, int appendidx = -1, CMsg::Id appendstr = CMsg::End, const QVariant &userData = QVariant());
	void addItemsById(const CPtrList<CTchar> &list, const QVariant &userData = QVariant());
	void addItemById(CMsg::Id textid, const QVariant &userData = QVariant());
};

/**
	@brief Button control
*/
class MyPushButton : public QPushButton
{
	Q_OBJECT

public:
	explicit MyPushButton(CMsg::Id textid, QWidget *parent = nullptr);

	void setTextById(CMsg::Id textid);
};

/**
	@brief File dialog box
*/
class MyFileDialog : public QFileDialog
{
	Q_OBJECT

public:
	explicit MyFileDialog(QWidget *parent = nullptr, CMsg::Id captionid = CMsg::Null, const QString &directory = QString(), const QString &filter = QString());

};

#endif // QT_DIALOG_H
