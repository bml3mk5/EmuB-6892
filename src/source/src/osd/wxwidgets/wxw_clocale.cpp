/** @file wxw_clocale.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2019.09.01

	@brief [i18n]
*/

#include "wxw_clocale.h"
#include <wx/filename.h>
//#include <wx/dir.h>
#include "../../fileio.h"
#include "../../utility.h"

static const _TCHAR *sub_dir[] = {
	_T(""),
#ifdef _WIN32
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
	NULL
};


CLocale::CLocale()
	: wxLocale(wxLANGUAGE_DEFAULT)
{
	Clear();
}
CLocale::CLocale(const CLocale &)
	: wxLocale(wxLANGUAGE_DEFAULT)
{
	Clear();
}
CLocale &CLocale::operator=(const CLocale &)
{
	Clear();
	return *this;
}

CLocale::~CLocale()
{
}

CLocale::CLocale(const wxString &new_app_path, const wxString &new_package_name, const wxString &new_language)
	: wxLocale(wxLANGUAGE_DEFAULT)
{
	Clear();
	SetLocale(new_app_path, new_package_name, new_language);
}

void CLocale::Clear()
{
	mRegionLoaded = false;
}

/// @param [in] new_app_path:  application path
/// @param [in] new_package_name:  package name
/// @param [in] new_language:  language name (ex. "Japanese" "English"
bool CLocale::SetLocale(const wxString &new_app_path, const wxString &new_package_name, const wxString &new_language)
{
	if (new_package_name.IsEmpty()) {
		return false;
	}

	// make file path
	if (!FindLocalePath(new_app_path)) {
		return false;
	}

	// read region list
	mRegionLoaded = ReadRegion(mLocalePath.t_str());
	if (!mRegionLoaded) {
		return false;
	}

	return SetLocale(new_package_name, new_language);
}

bool CLocale::SetLocale(const wxString &new_package_name, const wxString &new_language)
{
	mPackageName = new_package_name;

	// find locale
	const CLocaleRegion *region = NULL;
	if (!new_language.IsEmpty()) {
		region = FindRegionByName(new_language);
	}

	// use system locale
	wxString locale;
	if (region == NULL) {
		locale = GetCanonicalName();
		region = FindRegionByLocale(locale.t_str());
	}

	mLocaleName = locale;

	if (region == NULL) {
		mLanguageName.Clear();
		return false;
	}

	mLanguageName = region->GetName();

	// load catalog
	wxTranslations *t = wxTranslations::Get();
	if (t == NULL || !t->IsLoaded(new_package_name)) {
		AddCatalogLookupPathPrefix(mLocalePath);
		AddCatalog(new_package_name);

		t = wxTranslations::Get();
	}

	t->SetLanguage(locale);

	return true;
}

bool CLocale::SetLocale(const wxString &new_locale)
{
	wxTranslations *t = wxTranslations::Get();
	if (!t) return false;

	mLocaleName = new_locale;
	t->SetLanguage(new_locale);
	return true;
}

bool CLocale::UnsetLocale()
{
	wxTranslations::Set(NULL);
	return true;
}

bool CLocale::FindLocalePath(const wxString &new_app_path)
{
	wxFileName new_locale_path;
	bool exist = false;
	for(int i=0; sub_dir[i] != NULL; i++) {
		wxString sdir = new_app_path;
		sdir += wxFileName::GetPathSeparator();
		sdir += wxString(sub_dir[i]);
		sdir += _T("locale");

		new_locale_path = sdir;

		if (new_locale_path.IsDirReadable()) {
			exist = true;
			break;
		}
	}
	if (exist) {
		mLocalePath = new_locale_path.GetFullPath();
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
		while((p = fio.Fgets(buf, _MAX_PATH)) != NULL && mRegion.Count() < 200) {
			p = UTILITY::lskip(buf);
			if (strstr(p, "<!--") != NULL) {
				comment = true;
			}
			if (!comment) {
				switch(phase & 0xff) {
				case 1:
					if (strstr(p, "<LocaleList ") != NULL) {
						phase = 2;
					}
					break;
				case 2:
					if (strstr(p, "</LocaleList>") != NULL) {
						phase = 1;
					} else if (strstr(p, "<Locale ") != NULL) {
						p += 8;
						get_attr(p, "file", file, sizeof(file));
						get_attr(p, "name", name, sizeof(name));
						get_attr(p, "win", win, sizeof(win));
						get_attr(p, "posix", posix, sizeof(posix));

						mRegion.Add(new CLocaleRegion(file, name, win, posix));
					}
					break;
				default:
					if (strstr(p, "<?xml ") != NULL) {
						phase = 1;
					}
					break;
				}
			}
			if (strstr(p, "-->") != NULL) {
				comment = false;
			}
		}
	} while(0);

	return rc;
}

bool CLocale::get_attr(const char *buf, const char *attr, char *out, int len)
{
	memset(out, 0, len);
	const char *p = strstr(buf, attr);
	if (p == NULL) return false;
	p += strlen(attr);
	p = UTILITY::lskip(p);
	if (*p != '=') return false;
	p++;
	p = UTILITY::lskip(p);
	if (*p != '"') return false;
	p++;
	const char *ep = strchr(p, '"');
	if (ep == NULL) return false;
	int nlen = static_cast<int>(len - 1 > ep - p ? ep - p : len - 1);
	strncpy(out, p, nlen);
	return true;
}

const CLocaleRegion *CLocale::FindRegionByName(const wxString &language)
{
	bool match = false;
	CLocaleRegion *itm = NULL;
	for(int i=0; i<mRegion.Count(); i++) {
		itm = mRegion[i];
		if (language == itm->GetName()) {
			match = true;
			break;
		}
	}
	return (match ? itm : NULL);
}

const CLocaleRegion *CLocale::FindRegionByLocale(const wxString &locale)
{
	bool match = false;
	CLocaleRegion *itm = NULL;
	for(int i=0; i<mRegion.Count(); i++) {
		itm = mRegion[i];
		if (locale.Find(itm->GetPosix()) >= 0)
		{
			match = true;
			break;
		}
	}
	return (match ? itm : NULL);
}

const char *CLocale::GetText(const char *str)
{
	return wxGetTranslation(str).c_str();
}

const wchar_t *CLocale::GetText(const wchar_t *str)
{
	return wxGetTranslation(str).wc_str();
}

const _TCHAR *CLocale::GetLocaleName() const
{
	return mLocaleName.t_str();
}

const _TCHAR *CLocale::GetLanguageName() const
{
	return mLanguageName.t_str();
}

void CLocale::ChangeLocaleIfNeed(const CTchar &new_language)
{
//	if (!IsOk()) return;
	if (new_language.Length() <= 0) return;

	if (new_language.MatchString(_T("default"))) {
		UnsetLocale();
		return;
	}

	SetLocale(mPackageName, new_language.Get());
}

/// @brief get locale names on locale directory
bool CLocale::GetLocaleNames(CPtrList<CTchar> &arr) const
{
#if 0
	wxDir dir(mLocalePath);

	if (!dir.IsOpened()) return false;

	wxString fname;
	bool cont = dir.GetFirst(&fname, wxEmptyString, wxDIR_DIRS);
	while(cont) {
		arr.Add(new CTchar(fname.t_str()));
		cont = dir.GetNext(&fname);
	}
	return true;
#endif
	for(int i=0; i<mRegion.Count(); i++) {
		arr.Add(new CTchar(mRegion.Item(i)->GetName().t_str()));
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
CLocale *clocale = NULL;
