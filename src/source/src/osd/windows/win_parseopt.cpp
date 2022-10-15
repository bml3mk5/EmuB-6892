/** @file win_parseopt.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.05.18 -

	@brief [ parse options ]
*/

#include "win_parseopt.h"
#include "../../gui/gui.h"
#include "../../config.h"
#include "../../utility.h"

CParseOptions::CParseOptions(LPTSTR szCmdLine)
{
#ifdef USE_DEBUGGER
	debugger_imm_start = 0;
#endif
	get_options(szCmdLine);
}

CParseOptions::~CParseOptions()
{
	delete tape_file;
#ifdef USE_FD1
	for(int i=0; i<USE_DRIVE; i++) {
		delete disk_file[i];
	}
#endif
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

/**
 *	get and parse options
 */
int CParseOptions::get_options(LPTSTR szCmdLine)
{
	_TCHAR *optarg[10];
	int     optlen[10];
	int     optargc = 0;

	// allocate buffer
	tape_file = new CTchar();
#ifdef USE_FD1
	for(int i=0; i<USE_DRIVE; i++) {
		disk_file[i] = new CTchar();
	}
#endif
	state_file = new CTchar();
	autokey_file = new CTchar();
	reckey_file = new CTchar();

	_TCHAR tmp_path_1[_MAX_PATH];
	_TCHAR tmp_path_2[_MAX_PATH];

	// set application path and name
	GetModuleFileName(NULL, tmp_path_1, _MAX_PATH);
	UTILITY::get_dir_and_basename(tmp_path_1, tmp_path_2);
	UTILITY::chomp_crlf(tmp_path_1);
	UTILITY::chomp_crlf(tmp_path_2);
	app_path.SetM(tmp_path_1);
	app_name.SetM(tmp_path_2);

	// split options
	bool quoting = false;
	optargc = UTILITY::get_parameters((const _TCHAR *)szCmdLine, _MAX_PATH, (const _TCHAR **)optarg, 10, optlen);

	// parse options
	_TCHAR opt = _T('\0');
	int    optind = 0;
	bool finished = false;
	for(optind = 0; optind < optargc; optind++) {
		if (_tcschr(_T("-/"), optarg[optind][0]) != NULL) {
			opt = optarg[optind][1];
			if (_tcschr(_T("itdsak"), opt) != NULL) {
				continue;
			}
#ifdef USE_DEBUGGER
			if (_tcschr(_T("hHz"), opt) == NULL) {
#else
			if (_tcschr(_T("hH"), opt) == NULL) {
#endif
				opt = _T('?');
			}
		}
		///
		switch(opt) {
			case _T('i'):
				UTILITY::tcsncpy(tmp_path_1, _MAX_PATH, optarg[optind], optlen[optind]);
				ini_file.SetM(tmp_path_1);
				break;
			case _T('d'):
#ifdef USE_FD1
				for(int drv=0; drv<USE_DRIVE; drv++) {
					if (disk_file[drv]->Length() <= 0) {
						UTILITY::tcsncpy(tmp_path_1, _MAX_PATH, optarg[optind], optlen[optind]);
						disk_file[drv]->SetM(tmp_path_1);
						break;
					}
				}
#endif
				break;
			case _T('t'):
				UTILITY::tcsncpy(tmp_path_1, _MAX_PATH, optarg[optind], optlen[optind]);
				tape_file->SetM(tmp_path_1);
				break;
			case _T('s'):
				UTILITY::tcsncpy(tmp_path_1, _MAX_PATH, optarg[optind], optlen[optind]);
				state_file->SetM(tmp_path_1);
				break;
			case _T('a'):
				UTILITY::tcsncpy(tmp_path_1, _MAX_PATH, optarg[optind], optlen[optind]);
				autokey_file->SetM(tmp_path_1);
				break;
			case _T('k'):
				UTILITY::tcsncpy(tmp_path_1, _MAX_PATH, optarg[optind], optlen[optind]);
				reckey_file->SetM(tmp_path_1);
				break;
#ifdef USE_DEBUGGER
			case _T('z'):
				debugger_imm_start = 1;
				break;
#endif
			case _T('h'):
			case _T('H'):
			case _T('?'):
				{
					_TCHAR buf[_MAX_PATH];
					UTILITY::stprintf(buf, _MAX_PATH, _T("Usage: %s "), app_name);
					UTILITY::tcscat(buf, _MAX_PATH, _T("[-h] [-i <ini_file>] [-t <tape_file>] [-d <disk_file>] [-s <state_file>] [-a <autokey_file>] [-k <recordkey_file>]"));
#ifdef USE_DEBUGGER
					UTILITY::tcscat(buf, _MAX_PATH, _T(" [-z]"));
#endif
					UTILITY::tcscat(buf, _MAX_PATH, _T(" [<support_file> ...]"));
					_fputts(buf, stdout);
				}
				exit(0);
				break;
			default:
				finished = true;
				break;
		}
		opt = _T('\0');
		if (finished) break;
	}
	for(; optind < optargc && optarg[optind][0] != _T('\0'); optind++) {
		UTILITY::tcsncpy(tmp_path_1, _MAX_PATH, optarg[optind], optlen[optind]);
		int rc = GUI::CheckSupportedFile(tmp_path_1);
		switch(rc) {
		case 1:
			tape_file->SetM(tmp_path_1);
			break;
		case 2:
#ifdef USE_FD1
			for(int drv=0; drv<USE_DRIVE; drv++) {
				if (disk_file[drv]->Length() <= 0) {
					disk_file[drv]->SetM(tmp_path_1);
					break;
				}
			}
#endif
			break;
		case 3:
			state_file->SetM(tmp_path_1);
			break;
		case 4:
			autokey_file->SetM(tmp_path_1);
			break;
		case 5:
			ini_file.SetM(tmp_path_1);
			break;
		case 6:
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

	return 0;
}

/**
 *	open recent file
 */
void CParseOptions::open_recent_file(GUI *gui)
{
#ifdef USE_FD1
	_TCHAR path[_MAX_PATH];
#endif

	if (gui) {
#ifdef USE_DATAREC
		if (tape_file->Length() > 0) {
			gui->PostEtLoadDataRecMessage(tape_file->Get());
		}
#endif
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
#ifdef USE_FD1
	for(int i=0; i<USE_DRIVE; i++) {
		delete disk_file[i]; disk_file[i] = NULL;
	}
#endif
	delete state_file; state_file = NULL;
	delete autokey_file; autokey_file = NULL;
	delete reckey_file; reckey_file = NULL;
}
