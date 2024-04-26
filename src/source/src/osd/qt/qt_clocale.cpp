/** @file qt_clocale.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2014.12.01

	@brief [i18n]
*/

#include "qt_clocale.h"
#include "../../fileio.h"
#include "../../utility.h"
#include <QObject>
#include <QFileInfo>

static const _TCHAR *sub_dir[] = {
	_T(""),
#ifdef Q_OS_WIN
	_T("..\\"),
	_T("..\\..\\"),
	_T("..\\..\\..\\"),
	_T("..\\..\\..\\..\\"),
#else
	_T("../"),
	_T("../../"),
	_T("../../../"),
	_T("../../../../"),
#endif
	nullptr
};

#if defined(_UNICODE)

#include <QChar>

const wchar_t *_tgettext(const wchar_t *str)
{
	return str;
}

#else

const char *_tgettext(const char *str)
{
	return str;
	// QObject::tr(str).toUtf8().data();
}

#endif

//

CLocale::CLocale()
	: QObject()
{
	Clear();
}
CLocale::CLocale(const CLocale &)
	: QObject()
{
	Clear();
}
CLocale &CLocale::operator=(const CLocale &)
{
	Clear();
	return *this;
}
CLocale::CLocale(const QString &new_app_path, const QString &new_package_name, const QString &new_language)
	: QObject()
{
	Clear();
	SetLocale(new_app_path, new_package_name, new_language);
}

CLocale::~CLocale()
{
}

void CLocale::Clear()
{
	mRegionLoaded = false;
}

bool CLocale::SetLocale(const QString &new_app_path, const QString &new_package_name, const QString &new_language)
{
	if (new_package_name.isEmpty()) {
		return false;
	}

	// make file path
	if (!FindLocalePath(new_app_path)) {
		return false;
	}

	// read region list
	mRegionLoaded = ReadRegion(mLocalePath.toUtf8().data());
	if (!mRegionLoaded) {
		return false;
	}

	return SetLocale(new_package_name, new_language);
}

bool CLocale::SetLocale(const QString &new_package_name, const QString &new_language)
{
	mPackageName = new_package_name;

	// find locale
	const CLocaleRegion *region = nullptr;
	if (!new_language.isEmpty()) {
		region = FindRegionByName(new_language);
	}

	QLocale locale;
	if (region == nullptr) {
		// use system locale
		locale = QLocale::system();
		QString language = locale.bcp47Name();
		region = FindRegionByLocale(language);
	} else {
		locale = QLocale(region->GetPosix());
	}

	if (region == nullptr) {
		mLanguageName.Clear();
		return false;
	}

	mLanguageName.SetN(region->GetName().toUtf8().data());


	// load catalog
	bool trans_ok = mTranslator.load(locale, new_package_name, "_", mLocalePath, ".qm");
	if (trans_ok) {
		qApp->installTranslator(&mTranslator);
	}

	return trans_ok;
}

bool CLocale::SetLocale(const QString &new_locale)
{
	mLocaleName.SetN(new_locale.toUtf8().data());
	QLocale locale(new_locale);
	bool trans_ok = mTranslator.load(locale, mPackageName, "_", mLocalePath, ".qm");
	if (trans_ok) {
		qApp->installTranslator(&mTranslator);
	}
	return trans_ok;
}

void CLocale::UnsetLocale()
{
	qApp->removeTranslator(&mTranslator);
}

bool CLocale::FindLocalePath(const QString &new_app_path)
{
	QFileInfo new_locale_path;
	bool exist = false;
	for(int i=0; sub_dir[i] != nullptr; i++) {
		QString sdir = new_app_path;
#ifdef Q_OS_WIN
		sdir += _T("\\");
#else
		sdir += _T("/");
#endif
		sdir += sub_dir[i];
		sdir += _T("locale");

		new_locale_path = sdir;

		if (new_locale_path.isDir()) {
			exist = true;
			break;
		}
	}
	if (exist) {
		mLocalePath = new_locale_path.canonicalFilePath();
	}
	return exist;
}


bool CLocale::ReadRegion(const _TCHAR *filename)
{
	bool rc = true;
	FILEIO fio;

	_TCHAR fpath[_MAX_PATH];
	char buf[_MAX_PATH];
	char *p;

	char file[32];
	char name[64];
	char win[64];
	char posix[64];

	do {
		UTILITY::tcscpy(fpath, _MAX_PATH, filename);
		UTILITY::add_path_separator(fpath, _MAX_PATH);
		UTILITY::tcscat(fpath, _MAX_PATH, _T("list.xml"));

		if (!fio.Fopen(fpath, FILEIO::READ_ASCII)) {
			rc = false;
			break;
		}

		mRegion.Clear();

		int phase = 0;
		bool comment = false;

		// read XML
		while((p = fio.Fgets(buf, _MAX_PATH)) != nullptr && mRegion.Count() < 200) {
			p = UTILITY::lskip(buf);
			if (strstr(p, "<!--") != nullptr) {
				comment = true;
			}
			if (!comment) {
				switch(phase & 0xff) {
				case 1:
					if (strstr(p, "<LocaleList ") != nullptr) {
						phase = 2;
					}
					break;
				case 2:
					if (strstr(p, "</LocaleList>") != nullptr) {
						phase = 1;
					} else if (strstr(p, "<Locale ") != nullptr) {
						p += 8;
						get_attr(p, "file", file, sizeof(file));
						get_attr(p, "name", name, sizeof(name));
						get_attr(p, "win", win, sizeof(win));
						get_attr(p, "posix", posix, sizeof(posix));

						mRegion.Add(new CLocaleRegion(file, name, win, posix));
					}
					break;
				default:
					if (strstr(p, "<?xml ") != nullptr) {
						phase = 1;
					}
					break;
				}
			}
			if (strstr(p, "-->") != nullptr) {
				comment = false;
			}
		}
	} while(0);

	return rc;
}

bool CLocale::get_attr(const char *buf, const char *attr, char *out, int len)
{
	memset(out, 0, static_cast<size_t>(len));
	const char *p = strstr(buf, attr);
	if (p == nullptr) return false;
	p += strlen(attr);
	p = UTILITY::lskip(p);
	if (*p != '=') return false;
	p++;
	p = UTILITY::lskip(p);
	if (*p != '"') return false;
	p++;
	const char *ep = strchr(p, '"');
	if (ep == nullptr) return false;
	int nlen = static_cast<int>(len - 1 > ep - p ? ep - p : len - 1);
	memcpy(out, p, static_cast<size_t>(nlen));
	return true;
}

const CLocaleRegion *CLocale::FindRegionByName(const QString &language)
{
	bool match = false;
	CLocaleRegion *itm = nullptr;
	for(int i=0; i<mRegion.Count(); i++) {
		itm = mRegion[i];
		if (language == itm->GetName()) {
			match = true;
			break;
		}
	}
	return (match ? itm : nullptr);
}

const CLocaleRegion *CLocale::FindRegionByLocale(const QString &locale)
{
	bool match = false;
	CLocaleRegion *itm = nullptr;
	for(int i=0; i<mRegion.Count(); i++) {
		itm = mRegion[i];
		if (locale.indexOf(itm->GetPosix()) == 0) {
			match = true;
			break;
		} else if (itm->GetPosix().indexOf(locale) == 0) {
			match = true;
			break;
		}
	}
	return (match ? itm : nullptr);
}

const _TCHAR *CLocale::GetLocaleName() const
{
	return mLocaleName.Get();
}

const _TCHAR *CLocale::GetLanguageName() const
{
	return mLanguageName.Get();
}

void CLocale::ChangeLocaleIfNeed(const CTchar &new_language)
{
	if (new_language.Length() <= 0) return;

	if (new_language.MatchString(_T("default"))) {
		UnsetLocale();
		return;
	}

	SetLocale(mPackageName, new_language.Get());
}

bool CLocale::GetLocaleNames(CPtrList<CTchar> &arr) const
{
#if 0
	QDir dir(locale_path);

	if (!dir.exists()) return false;

	QStringList filters;
	filters.append("*.qm");
	QStringList list = dir.entryList(filters, QDir::Files);
	for(int i=0; i<list.count(); i++) {
		QString fname = list[i];
		int st = fname.indexOf('_');
		int ed = fname.indexOf('.');
		if (st > 0 && ed > st) {
			fname = fname.mid(st + 1, ed - st - 1);
			arr.Add(new CTchar(fname.toUtf8().data()));
		}
	}
	return true;
#endif
	for(int i=0; i<mRegion.Count(); i++) {
		arr.Add(new CTchar(mRegion.Item(i)->GetName().toUtf8().data()));
	}
	return true;
}

bool CLocale::GetLocaleNamesWithDefault(CPtrList<CTchar> &arr) const
{
	arr.Add(new CTchar(_T("System dependent")));
	arr.Add(new CTchar(_T("Default (English)")));
	return GetLocaleNames(arr);
}

int CLocale::SelectLocaleNameIndex(const CPtrList<CTchar> &arr, const CTchar &name) const
{
	int selidx = 0;
	if (!name.MatchString(_T("default"))) {
		for(int i=0; i<arr.Count(); i++) {
			bool match = false;
			match = arr.Item(i)->MatchString(name.Get());
			if (match) {
				selidx = i;
				break;
			}
		}
	} else {
		selidx = 1;
	}
	return selidx;
}

void CLocale::ChooseLocaleName(const CPtrList<CTchar> &arr, int selidx, CTchar &name) const
{
	if (selidx == 0) {
		name.Set(_T(""));
	} else if (selidx == 1) {
		name.Set(_T("default"));
	} else {
		name.Set(arr.Item(selidx)->Get());
	}
}

//

CLocaleRegion::CLocaleRegion(const char *n_file, const char *n_name, const char *n_win, const char *n_posix)
{
	file = n_file;
	name = n_name;
	win = n_win;
	posix = n_posix;
}

// set in main.
CLocale *clocale = nullptr;

