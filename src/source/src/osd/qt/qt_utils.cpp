/** @file qt_utils.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2016.11.11 -

	@brief [ qt utils ]
*/

#include "qt_utils.h"
#include "../../common.h"
#include "../../utility.h"

QTChar::QTChar()
{
	str = nullptr;
	clear();
}

QTChar::QTChar(const QString &src)
{
	str = nullptr;
	set(src);
}

QTChar::QTChar(const QTChar &src)
{
	str = nullptr;
	set(src);
}

QTChar::~QTChar()
{
	delete [] str;
}

QTChar &QTChar::operator=(const QTChar &src)
{
	set(src);
	return *this;
}

void QTChar::clear()
{
	delete [] str;
	str = new _TCHAR[1];
	str[0] = _T('\0');
}

void QTChar::set(const QString &src)
{
	delete [] str;
#ifdef _UNICODE
	str = new _TCHAR[static_cast<size_t>(src.length()) + 1];
	memset(str, 0, sizeof(wchar_t) * (static_cast<size_t>(src.length()) + 1));
	src.toWCharArray(str);
#else
	QByteArray data = src.toUtf8();
	str = new _TCHAR[static_cast<size_t>(data.length()) + 1];
	UTILITY::tcscpy(str, static_cast<size_t>(data.length()) + 1, data.data());
#endif
}

void QTChar::set(const QTChar &src)
{
	delete [] str;
	size_t size = _tcslen(src.str);
	str = new _TCHAR[size + 1];
	UTILITY::tcscpy(str, size + 1, src.str);
}

void QTChar::set(const _TCHAR *src, int size)
{
	delete [] str;
	if (size < 0) {
		size = static_cast<int>(_tcslen(src));
	}
	str = new _TCHAR[static_cast<size_t>(size) + 1];
	UTILITY::tcscpy(str, static_cast<size_t>(size) + 1, src);
}

QString QTChar::fromTChar(const _TCHAR *src, int size)
{
#ifdef _UNICODE
	return QString::fromWCharArray(src, size);
#else
	return QString::fromUtf8(src, size);
#endif
}

const _TCHAR *QTChar::toTChar()
{
	return str;
}

void QTChar::toTChar(_TCHAR *dst, int size)
{
	UTILITY::tcsncpy(dst, static_cast<size_t>(size), str, static_cast<size_t>(length()));
}

void QTChar::toTChar(_TCHAR *dst, const QString &src, int size)
{
#ifdef _UNICODE
	src.left(size-1).toWCharArray(dst);
#else
	QByteArray data = src.toUtf8();
	UTILITY::tcsncpy(dst, static_cast<size_t>(size), data.data(), static_cast<size_t>(data.length()));
#endif
}

size_t QTChar::length() const
{
	return _tcslen(str);
}
