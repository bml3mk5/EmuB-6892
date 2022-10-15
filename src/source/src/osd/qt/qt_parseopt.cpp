/** @file qt_parseopt.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.05.18 -

	@brief [ parse options ]
*/

#include "qt_parseopt.h"
#include "../../gui/gui.h"
#include "../../config.h"
#include "../../vm/vm.h"
#include "../../utility.h"
//#include "qt_main.h"
#include "../../depend.h"
#include <QCommandLineParser>

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
	return app_path.toTChar();
}

const _TCHAR *CParseOptions::get_app_name()
{
	return app_name.toTChar();
}

const _TCHAR *CParseOptions::get_ini_path()
{
	return ini_path.toTChar();
}

const _TCHAR *CParseOptions::get_ini_file()
{
	return ini_file.toTChar();
}

const _TCHAR *CParseOptions::get_res_path()
{
	return res_path.toTChar();
}

/**
 *	get and parse options
 */
int CParseOptions::get_options(int ac, char *av[])
{
	QStringList args = qApp->arguments();
#ifdef _DEBUG
	for(int i=0; i<ac; i++) {
		printf("%d %s\n",i,av[i]);
	}
#endif

	// allocate buffer
	tape_file = new QTChar();
	for(int i=0; i<USE_DRIVE; i++) {
		disk_file[i] = new QTChar();
	}
	state_file = new QTChar();
	autokey_file = new QTChar();
	reckey_file = new QTChar();

	_TCHAR tmp_path_1[_MAX_PATH];
	_TCHAR tmp_path_2[_MAX_PATH];

	QTChar arg0(args[0]);
	UTILITY::get_dir_and_basename(arg0.toTChar(), tmp_path_1, tmp_path_2);
	app_path.set(tmp_path_1);
	app_name.set(tmp_path_2);

	// resource path
	UTILITY::tcscat(tmp_path_1, _MAX_PATH, _T(RESDIR));
	UTILITY::slim_path(tmp_path_1, tmp_path_2, _MAX_PATH);
	res_path.set(tmp_path_2);

	QCommandLineParser parser;
	parser.addHelpOption();

#if defined(__APPLE__) && defined(__MACH__)
	// When mac, app_path set upper app folder (ex. foo/bar/baz.app/../)
	UTILITY::tcsncpy(tmp_path_1, _MAX_PATH, app_path.toTChar(), _MAX_PATH);
	UTILITY::add_path_separator(tmp_path_1);
	UTILITY::slim_path(tmp_path_1, tmp_path_2, _MAX_PATH);
	UTILITY::get_ancestor_dir(tmp_path_2, 3);
	app_path = QString::fromUtf8(tmp_path_2);
	// Xcode set debug options
	if (ac >= 3 && strstr(av[1],"NSDocument")) {
		optind += 2;
	}
#endif

	QCommandLineOption opt_i("i","specify ini file.","ini_file");
	parser.addOption(opt_i);
	QCommandLineOption opt_d("d","specify disk image file.","disk_file");
	parser.addOption(opt_d);
	QCommandLineOption opt_t("t","specify tape image file.","tape_file");
	parser.addOption(opt_t);
	QCommandLineOption opt_s("s","specify state file.","state_file");
	parser.addOption(opt_s);
	QCommandLineOption opt_a("a","specify auto key file.","autokey_file");
	parser.addOption(opt_a);
	QCommandLineOption opt_k("k","specify record key file.","reckey_file");
	parser.addOption(opt_k);
#ifdef USE_DEBUGGER
	QCommandLineOption opt_z("z","start debugger immediately.");
	parser.addOption(opt_z);
#endif

	parser.parse(args);

	// check ini file
	if (parser.isSet(opt_i)) {
		ini_file.set(parser.value(opt_i));
		if (UTILITY::check_file_extension(ini_file.toTChar(), _T("ini"))) {
			_TCHAR ini_name[_MAX_PATH];
			UTILITY::get_dir_and_basename(ini_file.toTChar(), tmp_path_1, ini_name);
		}
		UTILITY::add_path_separator(tmp_path_1);
	} else {
		UTILITY::tcscpy(tmp_path_1, _MAX_PATH, app_path.toTChar());
		UTILITY::tcscpy(tmp_path_2, _MAX_PATH, tmp_path_1);
		UTILITY::tcscat(tmp_path_2, _MAX_PATH, _T(CONFIG_NAME));
		UTILITY::tcscat(tmp_path_2, _MAX_PATH, _T(".ini"));
		ini_file.set(tmp_path_2);
	}
	ini_path.set(tmp_path_1);

	if (parser.isSet(opt_d)) {
		int i=0;
		foreach(QString val, parser.values(opt_d)) {
			*disk_file[i] = val;
			i++;
			if (i >= USE_DRIVE) break;
		}
	}
	if (parser.isSet(opt_t)) {
		*tape_file = parser.value(opt_t);
	}
	if (parser.isSet(opt_s)) {
		*state_file = parser.value(opt_s);
	}
	if (parser.isSet(opt_a)) {
		*autokey_file = parser.value(opt_a);
	}
	if (parser.isSet(opt_k)) {
		*reckey_file = parser.value(opt_k);
	}
#ifdef USE_DEBUGGER
	if (parser.isSet(opt_z)) {
		debugger_imm_start = 1;
	}
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
		if (tape_file->length() > 0) {
			gui->PostEtLoadDataRecMessage(tape_file->toTChar());
		}
#ifdef USE_FD1
		// auto open recent file
		for(int drv=(USE_DRIVE-1); drv>=0; drv--) {
			int bank_num = 0;
			path[0] = _T('\0');
			if (disk_file[drv]->length() > 0) {
				// specified file in the option
				UTILITY::tcscpy(path, _MAX_PATH, disk_file[drv]->toTChar());
			} else if ((config.mount_fdd & (1 << drv)) != 0 && config.recent_disk_path[drv].Count() > 0 && config.recent_disk_path[drv][0]->path.Length() > 0) {
				// recent file
				UTILITY::tcscpy(path, _MAX_PATH, config.recent_disk_path[drv][0]->path);
				bank_num = config.recent_disk_path[drv][0]->num;
			}
			if (path[0] != _T('\0')) {
				gui->PostEtOpenFloppyMessage(drv, path, bank_num, 0, false);
			}
		}
#endif
		if (state_file->length() > 0) {
			gui->PostEtLoadStatusMessage(state_file->toTChar());
		}
		if (autokey_file->length() > 0) {
			gui->PostEtLoadAutoKeyMessage(autokey_file->toTChar());
		}
		if (reckey_file->length() > 0) {
			gui->PostEtLoadRecKeyMessage(reckey_file->toTChar());
		}
#ifdef USE_DEBUGGER
		if (debugger_imm_start || config.debugger_imm_start) {
			gui->PostMtOpenDebugger();
		}
#endif
	}

	// release buffer
	delete tape_file; tape_file = nullptr;
	for(int i=0; i<USE_DRIVE; i++) {
		delete disk_file[i]; disk_file[i] = nullptr;
	}
	delete state_file; state_file = nullptr;
	delete autokey_file; autokey_file = nullptr;
	delete reckey_file; reckey_file = nullptr;
}
