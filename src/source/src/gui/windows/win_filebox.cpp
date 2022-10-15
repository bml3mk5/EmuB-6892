/** @file win_filebox.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.03.21 -

	@brief [ file box ]
*/

#include "win_filebox.h"
#include "../../common.h"
#include "../../cchar.h"
#include "../../utility.h"

namespace GUI_WIN
{

FileBox::FileBox(HWND parent_window)
{
	hWnd = parent_window;
	memset(selected_file, 0, sizeof(selected_file));
}

FileBox::~FileBox()
{
}

bool FileBox::Show(const CMsg::Id *filter, const _TCHAR *title, const _TCHAR *dir, const _TCHAR *ext, bool save, _TCHAR *path)
{
	_TCHAR fle[_MAX_PATH] = _T("");
	_TCHAR app[_MAX_PATH];
	_TCHAR fil[_MAX_PATH];

	OPENFILENAME OpenFileName;
	int rc = 0;

	memset(&OpenFileName, 0, sizeof(OpenFileName));
	OpenFileName.lStructSize = sizeof(OPENFILENAME);
	OpenFileName.hwndOwner = hWnd;
	CTchar ctitle(title);
	OpenFileName.lpstrTitle = ctitle.Get();
	if (filter) {
		int pos = 0;
		fil[0] = _T('\0');
		for(int i=0; filter[i] != 0 && filter[i] != CMsg::End; i++) {
			_TCHAR filter1[_MAX_PATH];
			_TCHAR subext[_MAX_PATH];
			UTILITY::conv_to_api_string(CMSGV(filter[i]), _MAX_PATH, filter1, _MAX_PATH);
			UTILITY::tcscat(&fil[pos], _MAX_PATH - pos, filter1);
			pos += (int)_tcslen(filter1);
			pos++;
			if (pos >= _MAX_PATH) break;
			fil[pos] = _T('\0');
			if (UTILITY::substr_in_bracket(filter1, subext) <= 0) continue;

			UTILITY::tcscat(&fil[pos], _MAX_PATH - pos, subext);
			pos += (int)_tcslen(subext);
			pos++;
			if (pos >= _MAX_PATH) break;
			fil[pos] = _T('\0');
		}
		OpenFileName.lpstrFilter = fil;
	}
	if(ext != NULL && ext[0]) {
		OpenFileName.lpstrDefExt = ext;
	}
	if(dir != NULL && dir[0]) {
		UTILITY::tcscpy(app, _MAX_PATH, dir);
		OpenFileName.lpstrInitialDir = app;
	} else if (path != NULL && path[0]) {
		UTILITY::get_dir_and_basename(path, app, fle);
		OpenFileName.lpstrInitialDir = app;
	} else {
		GetModuleFileName(NULL, app, _MAX_PATH);
		UTILITY::get_parent_dir(app);
		OpenFileName.lpstrInitialDir = app;
	}
	OpenFileName.lpstrFile = fle;
	OpenFileName.nMaxFile = _MAX_PATH;

	if (save) {
		OpenFileName.Flags = OFN_OVERWRITEPROMPT;
		rc = GetSaveFileName(&OpenFileName);
	} else {
		OpenFileName.Flags = 0;
		rc = GetOpenFileName(&OpenFileName);
	}
	flags = OpenFileName.Flags;

	if (rc) {
		UTILITY::tcsncpy(selected_file, _MAX_PATH, OpenFileName.lpstrFile, _MAX_PATH);
		if (path) {
			UTILITY::tcscpy(path, _MAX_PATH, selected_file);
		}
		return true;
	}
	return false;
}

const _TCHAR *FileBox::GetPathM() const
{
#if defined(USE_UTF8_ON_MBCS)
	static _TCHAR tfile[_MAX_PATH];
	UTILITY::conv_from_native_path(selected_file, tfile, _MAX_PATH);
#else
	const _TCHAR *tfile = selected_file;
#endif
	return tfile;
}

}; /* namespace GUI_WIN */
