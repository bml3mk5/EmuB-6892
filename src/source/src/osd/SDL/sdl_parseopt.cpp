/** @file sdl_parseopt.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.05.18 -

	@brief [ parse options ]
*/

#include "sdl_parseopt.h"
#include "../../gui/gui.h"
#include "../../config.h"
#include "../../utility.h"
//#include "sdl_main.h"
#include "../../depend.h"
#ifndef _MSC_VER
#include <getopt.h>
#endif

CParseOptions::CParseOptions(int ac, char *av[])
{
#ifdef USE_DEBUGGER
	debugger_imm_start = 0;
#endif
	get_options(ac, av);
}

CParseOptions::~CParseOptions()
{
	delete tape_file;
	for(int i=0; i<USE_DRIVE; i++) {
		delete disk_file[i];
	}
	delete state_file;
	delete autokey_file;
	delete reckey_file;
}

const _TCHAR *CParseOptions::get_app_path()
{
	return app_path;
}

const _TCHAR *CParseOptions::get_app_name()
{
	return app_name;
}

const _TCHAR *CParseOptions::get_ini_path()
{
	return ini_path;
}

const _TCHAR *CParseOptions::get_ini_file()
{
	return ini_file;
}

const _TCHAR *CParseOptions::get_res_path()
{
	return res_path;
}

/**
 *	get and parse options
 */
int CParseOptions::get_options(int ac, char *av[])
{
	int opt = 0;
#ifdef _DEBUG
	for(int i=0; i<ac; i++) {
		printf("%d %s\n",i,av[i]);
	}
#endif

	// allocate buffer
	tape_file = new CTchar();
	for(int i=0; i<USE_DRIVE; i++) {
		disk_file[i] = new CTchar();
	}
	state_file = new CTchar();
	autokey_file = new CTchar();
	reckey_file = new CTchar();

	_TCHAR tmp_path_1[_MAX_PATH];
	_TCHAR tmp_path_2[_MAX_PATH];

	// set application path and name
	_TCHAR **w_av;
#ifdef _UNICODE
	w_av = new _TCHAR*[ac];
	for(int i=0; i<ac; i++) {
		w_av[i] = new _TCHAR[_MAX_PATH];
		UTILITY::conv_mbs_to_wcs(av[i], (int)strlen(av[i]), w_av[i], _MAX_PATH);
	}
#else
	w_av = av;
#endif
	if (get_module_file_name(tmp_path_1, _MAX_PATH)) {
		UTILITY::get_dir_and_basename(tmp_path_1, tmp_path_2);
	} else {
		UTILITY::get_dir_and_basename(w_av[0], tmp_path_1, tmp_path_2);
	}
	UTILITY::chomp_crlf(tmp_path_1);
	UTILITY::chomp_crlf(tmp_path_2);
	app_path.SetM(tmp_path_1);
	app_name.SetM(tmp_path_2);

	// resource path
	UTILITY::tcsncat(tmp_path_1, _MAX_PATH, _T(RESDIR), _MAX_PATH);
	UTILITY::slim_path(tmp_path_1, tmp_path_2, _MAX_PATH);
	res_path.SetM(tmp_path_2);

#if defined(__APPLE__) && defined(__MACH__)
	// When mac, app_path set upper app folder (ex. foo/bar/baz.app/../)
	UTILITY::tcsncpy(tmp_path_1, _MAX_PATH, app_path.Get(), _MAX_PATH);
	UTILITY::add_path_separator(tmp_path_1);
	UTILITY::slim_path(tmp_path_1, tmp_path_2, _MAX_PATH);
	UTILITY::get_ancestor_dir(tmp_path_2, 3);
	app_path.Set(tmp_path_2);
	// Xcode set debug options
	if (ac >= 3 && strstr(av[1],"NSDocument")) {
		optind += 2;
	}
#endif

#if !defined(_MSC_VER)
#ifdef USE_DEBUGGER
	while ((opt = getopt(ac, av, "hzp:a:d:i:s:t:k:")) != -1) {
#else
	while ((opt = getopt(ac, av, "hp:a:d:i:s:t:k:")) != -1) {
#endif
		///
		switch (opt) {
		case 'i':
			UTILITY::tcscpy(tmp_path_1, _MAX_PATH, optarg);
			ini_file.SetM(tmp_path_1);
			break;
		case 'd':
			for(int drv=0; drv<USE_DRIVE; drv++) {
				if (disk_file[drv]->Length() <= 0) {
					memset(tmp_path_1, 0, sizeof(_TCHAR) * _MAX_PATH);
					UTILITY::tcscpy(tmp_path_1, _MAX_PATH, optarg);
					disk_file[drv]->SetM(tmp_path_1);
					break;
				}
			}
			break;
		case 't':
			UTILITY::tcscpy(tmp_path_1, _MAX_PATH, optarg);
			tape_file->SetM(tmp_path_1);
			break;
		case 's':
			UTILITY::tcscpy(tmp_path_1, _MAX_PATH, optarg);
			state_file->SetM(tmp_path_1);
			break;
		case 'a':
			UTILITY::tcscpy(tmp_path_1, _MAX_PATH, optarg);
			autokey_file->SetM(tmp_path_1);
			break;
		case 'k':
			UTILITY::tcscpy(tmp_path_1, _MAX_PATH, optarg);
			reckey_file->SetM(tmp_path_1);
			break;
		case 'p':
			break;
#ifdef USE_DEBUGGER
		case 'z':
			debugger_imm_start = 1;
			break;
#endif
		case 'h':
//		case '?':
			UTILITY::tcscpy(tmp_path_1, _MAX_PATH, _T("Usage: "));
			UTILITY::tcscat(tmp_path_1, _MAX_PATH, app_name.GetM());
			UTILITY::tcscat(tmp_path_1, _MAX_PATH, _T(" [-h] [-i <ini_file>] [-t <tape_file>] [-d <disk_file>] [-s <state_file>] [-a <autokey_file>] [-k <recordkey_file>]"));
#ifdef USE_DEBUGGER
			UTILITY::tcscat(tmp_path_1, _MAX_PATH, _T(" [-z]"));
#endif
			UTILITY::tcscat(tmp_path_1, _MAX_PATH, _T(" [<support_file> ...]"));
			_fputts(tmp_path_1, stdout);
			exit(0);
			break;
		default:
			break;
		}
	}
#else
	int optind = 1;
#endif
	for (;optind < ac && av[optind][0] != '\0'; optind++) {
		int rc = GUI::CheckSupportedFile(w_av[optind]);
		switch(rc) {
		case 1:
			// tape file
			UTILITY::tcscpy(tmp_path_1, _MAX_PATH, w_av[optind]);
			tape_file->SetM(tmp_path_1);
			break;
		case 2:
			for(int drv=0; drv<USE_DRIVE; drv++) {
				if (disk_file[drv]->Length() <= 0) {
					UTILITY::tcscpy(tmp_path_1, _MAX_PATH, w_av[optind]);
					disk_file[drv]->SetM(tmp_path_1);
					break;
				}
			}
			break;
		case 3:
			UTILITY::tcscpy(tmp_path_1, _MAX_PATH, w_av[optind]);
			state_file->SetM(tmp_path_1);
			break;
		case 4:
			UTILITY::tcscpy(tmp_path_1, _MAX_PATH, w_av[optind]);
			autokey_file->SetM(tmp_path_1);
			break;
		case 5:
			UTILITY::tcscpy(tmp_path_1, _MAX_PATH, w_av[optind]);
			ini_file.SetM(tmp_path_1);
			break;
		case 6:
			UTILITY::tcscpy(tmp_path_1, _MAX_PATH, w_av[optind]);
			reckey_file->SetM(tmp_path_1);
			break;
		default:
			break;
		}
	}

	// check ini file
	if (ini_file.Length() > 0) {
		if (UTILITY::check_file_extension(ini_file.Get(), _T("ini"))) {
			_TCHAR ini_name[_MAX_PATH];
			UTILITY::get_dir_and_basename(ini_file.Get(), tmp_path_1, ini_name);
		}
		UTILITY::add_path_separator(tmp_path_1);
	} else {
		UTILITY::tcscpy(tmp_path_1, _MAX_PATH, app_path.Get());
		UTILITY::tcscpy(tmp_path_2, _MAX_PATH, tmp_path_1);
		UTILITY::tcscat(tmp_path_2, _MAX_PATH, _T(CONFIG_NAME));
		UTILITY::tcscat(tmp_path_2, _MAX_PATH, _T(".ini"));
		ini_file.Set(tmp_path_2);
	}
	ini_path.Set(tmp_path_1);

#ifdef _UNICODE
	for(int i=0; i<ac; i++) {
		delete w_av[i];
	}
	delete w_av;
#endif

	return 0;
}

/**
 *	open recent file
 */
void CParseOptions::open_recent_file(GUI *gui)
{
	_TCHAR path[_MAX_PATH];

	if (gui) {
		if (tape_file->Length() > 0) {
			gui->PostEtLoadDataRecMessage(tape_file->Get());
		}
#ifdef USE_FD1
		// auto open recent file
		for(int drv=(USE_DRIVE-1); drv>=0; drv--) {
			path[0] = _T('\0');
			int bank_num = 0;
			if (disk_file[drv]->Length() > 0 && disk_file[drv]->MatchString(path) != 0) {
				// specified file in the option
				path[0] = _T('\0');
				UTILITY::tcscpy(path, _MAX_PATH, disk_file[drv]->Get());
			} else if ((config.mount_fdd & (1 << drv)) != 0 && config.recent_disk_path[drv].Count() > 0 && config.recent_disk_path[drv][0]->path.Length() > 0) {
				// recent file
				path[0] = _T('\0');
				UTILITY::tcscpy(path, _MAX_PATH, config.recent_disk_path[drv][0]->path.Get());
				bank_num = config.recent_disk_path[drv][0]->num;
			}
			if (path[0] != _T('\0')) {
				gui->PostEtOpenFloppyMessage(drv, path, bank_num, 0, false);
			}
		}
#endif
		if (state_file->Length() > 0) {
			gui->PostEtLoadStatusMessage(state_file->Get());
		}
		if (autokey_file->Length() > 0) {
			gui->PostEtLoadAutoKeyMessage(autokey_file->Get());
		}
		if (reckey_file->Length() > 0) {
			gui->PostEtLoadRecKeyMessage(reckey_file->Get());
		}
#ifdef USE_DEBUGGER
		if (debugger_imm_start || config.debugger_imm_start) {
			gui->PostMtOpenDebugger();
		}
#endif
	}

	// release buffer
	delete tape_file; tape_file = NULL;
	for(int i=0; i<USE_DRIVE; i++) {
		delete disk_file[i]; disk_file[i] = NULL;
	}
	delete state_file; state_file = NULL;
	delete autokey_file; autokey_file = NULL;
	delete reckey_file; reckey_file = NULL;
}

bool CParseOptions::get_module_file_name(_TCHAR *path, int size)
{
#if defined(_WIN32)
	return (::GetModuleFileName(NULL, path, size) > 0);
#else
	return false;
#endif
}
