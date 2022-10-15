/** @file qt_dialog.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2019.03.01

	@brief [ qt dialog ]
*/

#include "qt_dialog.h"
#include "../../version.h"
#include "../../emu.h"
#include "../../parseopt.h"
#include "../../utils.h"
#include <QtCore>

MyTabWidget::MyTabWidget(QWidget *parent)
	: QTabWidget(parent)
{
}
int MyTabWidget::addTab(QWidget *page, CMsg::Id labelid)
{
	const char *label = CMSGV(labelid);
	return QTabWidget::addTab(page, label);
}
MyGroupBox::MyGroupBox(CMsg::Id titleid, QWidget *parent)
	: QGroupBox(parent)
{
	setTitleById(titleid);
}
void MyGroupBox::setTitleById(CMsg::Id titleid)
{
	const char *title = CMSGV(titleid);
	QGroupBox::setTitle(title);
}
MyCheckBox::MyCheckBox(CMsg::Id textid, QWidget *parent)
	: QCheckBox(parent)
{
	setTextById(textid);
}
void MyCheckBox::setTextById(CMsg::Id textid)
{
	const char *text = CMSGV(textid);
	QCheckBox::setText(text);
}
MyRadioButton::MyRadioButton(CMsg::Id textid, QWidget *parent)
	: QRadioButton(parent)
{
	setTextById(textid);
}
void MyRadioButton::setTextById(CMsg::Id textid)
{
	const char *text = CMSGV(textid);
	QRadioButton::setText(text);
}
MyLabel::MyLabel(CMsg::Id textid, QWidget *parent, Qt::WindowFlags f)
	: QLabel(parent, f)
{
	setTextById(textid);
}
void MyLabel::setTextById(CMsg::Id textid)
{
	const char *text = CMSGV(textid);
	QLabel::setText(text);
}

///
/// \brief MyComboBox::MyComboBox
/// \param nullable parent
///
MyComboBox::MyComboBox(QWidget *parent)
	: QComboBox(parent)
{
}
MyComboBox::MyComboBox(QWidget *parent, const _TCHAR *list[], const QVariant &userData)
	: QComboBox(parent)
{
	for(int i=0; list[i] != nullptr; i++) {
		QComboBox::addItem(list[i], userData);
	}
}
MyComboBox::MyComboBox(QWidget *parent, const CMsg::Id *ids, int selidx, int appendidx, CMsg::Id appendstr, const QVariant &userData)
	: QComboBox(parent)
{
	addItemsById(ids, appendidx, appendstr, userData);
	setCurrentIndex(selidx);
}
MyComboBox::MyComboBox(QWidget *parent, const CPtrList<CTchar> &list, const QVariant &userData)
	: QComboBox(parent)
{
	addItemsById(list, userData);
}
void MyComboBox::addItemsById(const CMsg::Id *ids, int appendidx, CMsg::Id appendstr, const QVariant &userData)
{
	for(int i=0; ids[i] != CMsg::End; i++) {
		if (i == appendidx) {
			QString text;
			text.append(CMSGV(ids[i]));
			text.append(CMSGV(appendstr));
			QComboBox::addItem(text, userData);
		} else {
			addItemById(ids[i], userData);
		}
	}
}
void MyComboBox::addItemsById(const CPtrList<CTchar> &list, const QVariant &userData)
{
	for(int i=0; i<list.Count(); i++) {
		QComboBox::addItem(list.Item(i)->GetN(), userData);
	}
}
void MyComboBox::addItemById(CMsg::Id textid, const QVariant &userData)
{
	const char *text = CMSGV(textid);
	QComboBox::addItem(text, userData);
}
MyPushButton::MyPushButton(CMsg::Id textid, QWidget *parent)
	: QPushButton(parent)
{
	setTextById(textid);
}
void MyPushButton::setTextById(CMsg::Id textid)
{
	const char *text = CMSGV(textid);
	QPushButton::setText(text);
}
MyFileDialog::MyFileDialog(QWidget *parent, CMsg::Id captionid, const QString &directory, const QString &filter)
	: QFileDialog(parent, QString(), directory, filter)
{
	const char *caption = CMSGV(captionid);
	QFileDialog::setLabelText(QFileDialog::LookIn, caption);
}
