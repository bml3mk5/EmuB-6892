/** @file qt_utils.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2016.11.11 -

	@brief [ qt utils ]
*/

#ifndef QT_UTILS_H
#define QT_UTILS_H

#include <QString>
#include <QImage>
#include "../../common.h"

/**
	@brief translate TCHAR and QString
*/
class QTChar
{
public:
	QTChar();
	QTChar(const QString &src);
	QTChar(const QTChar &src);
	~QTChar();
	QTChar &operator=(const QTChar &src);

	void clear();
	void set(const QString &src);
	void set(const QTChar &src);
	void set(const _TCHAR *src, int size = -1);
	static QString fromTChar(const _TCHAR *src, int size = -1);
	const _TCHAR *toTChar();
	void toTChar(_TCHAR *dst, int size);
	static void toTChar(_TCHAR *dst, const QString &src, int size);

	size_t length() const;

private:
	_TCHAR *str;

};

#endif // QT_UTILS_H

