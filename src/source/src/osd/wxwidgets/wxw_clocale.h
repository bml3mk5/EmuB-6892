/** @file wxw_clocale.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2019.09.01

	@brief [i18n]
*/

#ifndef WXW_CLOCALE_H
#define WXW_CLOCALE_H

#include "../../depend.h"
#include <wx/translation.h>
#include <wx/intl.h>
#include "../../cchar.h"
#include "../../cptrlist.h"

#define gettext(text)  wxGetTranslation(text).utf8_str()
#define wgettext(text) wxGetTranslation(text).wc_str()

#ifdef _UNICODE
#define _tgettext wgettext
#else
#define _tgettext gettext
#endif

#ifdef _
#undef _
#ifdef _UNICODE
#define _(text) wgettext(text)
#else
#define _(text) gettext(text)
#endif
#endif

/// Specify a text that xgettext should get and translate.
/// (When use xgettext, set --keyword=_TX.)
#define _TX _T

class CLocaleRegion;

/**
	@brief Language Localization
*/
class CLocale : public wxLocale
{
private:
	wxString mLanguageName;
	wxString mLocaleName;
	wxString mPackageName;
	wxString mLocalePath;

	CPtrList<CLocaleRegion> mRegion;
	bool mRegionLoaded;

	/// Cannot copy
	CLocale(const CLocale &);
	CLocale &operator=(const CLocale &);

	bool FindLocalePath(const wxString &new_app_path);
	const CLocaleRegion *FindRegionByName(const wxString &language);
	const CLocaleRegion *FindRegionByLocale(const wxString &locale);
	bool ReadRegion(const _TCHAR *filename);

	static bool get_attr(const char *buf, const char *attr, char *out, int len);

public:
	CLocale();
	CLocale(const wxString &new_app_path, const wxString &new_package_name, const wxString &new_language);
	~CLocale();
	void Clear();

	bool SetLocale(const wxString &new_app_path, const wxString &new_package_name, const wxString &new_language);
	bool SetLocale(const wxString &new_package_name, const wxString &new_language);
	bool SetLocale(const wxString &new_locale);
	bool UnsetLocale();

	const char    *GetText(const char *str);
	const wchar_t *GetText(const wchar_t *str);

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
	wxString file;
	wxString name;
	wxString win;
	wxString posix;

public:
	CLocaleRegion(const char *n_file, const char *n_name, const char *n_win, const char *n_posix);

	const wxString &GetFile() const { return file; }
	const wxString &GetName() const { return name; }
	const wxString &GetWin() const { return win; }
	const wxString &GetPosix() const { return posix; }
};

// set in main.
extern CLocale *clocale;

#endif /* WXW_CLOCALE_H */
