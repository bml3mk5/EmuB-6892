/** @file qt_clocale.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2014.12.01

	@brief [i18n]
*/

#ifndef QT_CLOCALE_H
#define QT_CLOCALE_H

#include "../../depend.h"
#include <QApplication>
#include <QString>
#include <QTranslator>
#include "../../cchar.h"
#include "../../cptrlist.h"

const char *gettext(const char *str);
const wchar_t *wgettext(const wchar_t *str);

/// Specify a text that xgettext should get and translate.
/// (When use xgettext, set --keyword=_TX.)

#if defined(_UNICODE)
#define _tgettext wgettext
#define _(text) L##text
#define _TX(text) L##text
#else
#define _tgettext gettext
#define _(text) text
#define _TX(text) text
#endif

class CLocaleRegion;

/**
	@brief Language Localization
*/
class CLocale : public QObject
{
private:
	QTranslator mTranslator;

	CTchar mLanguageName;
	CTchar mLocaleName;
	QString mPackageName;
	QString mLocalePath;

	CPtrList<CLocaleRegion> mRegion;
	bool mRegionLoaded;

	/// Cannot copy
	CLocale(const CLocale &);
	CLocale &operator=(const CLocale &);

	bool FindLocalePath(const QString &new_app_path);
	const CLocaleRegion *FindRegionByName(const QString &language);
	const CLocaleRegion *FindRegionByLocale(const QString &locale);
	bool ReadRegion(const _TCHAR *filename);

	static char *ltrim(char *buf);
	static const char *ltrim(const char *buf);
	static bool get_attr(const char *buf, const char *attr, char *out, int len);

public:
	CLocale();
	CLocale(const QString &new_app_path, const QString &new_package_name, const QString &new_language);
	~CLocale();
	void Clear();

	bool SetLocale(const QString &new_app_path, const QString &new_package_name, const QString &new_language);
	bool SetLocale(const QString &new_package_name, const QString &new_language);
	bool SetLocale(const QString &new_locale);
	void UnsetLocale();

	const _TCHAR *GetLocaleName() const;
	const _TCHAR *GetLanguageName() const;

	void ChangeLocaleIfNeed(const CTchar &new_language);

	bool GetLocaleNames(CPtrList<CTchar> &arr) const;
	bool GetLocaleNamesWithDefault(CPtrList<CTchar> &arr) const;
	int  SelectLocaleNameIndex(const CPtrList<CTchar> &arr, const CTchar &name) const;
	void ChooseLocaleName(const CPtrList<CTchar> &arr, int selidx, CTchar &name) const;
};

/**
	@brief Locale Region
*/
class CLocaleRegion
{
private:
	QString file;
	QString name;
	QString win;
	QString posix;

public:
	CLocaleRegion(const char *n_file, const char *n_name, const char *n_win, const char *n_posix);

	const QString &GetFile() const { return file; }
	const QString &GetName() const { return name; }
	const QString &GetWin() const { return win; }
	const QString &GetPosix() const { return posix; }
};

// set in main.
extern CLocale *clocale;

#endif /* _QT_CLOCALE_H_ */
