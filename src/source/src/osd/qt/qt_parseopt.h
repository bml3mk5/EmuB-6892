/** @file qt_parseopt.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.05.18 -

	@brief [ parse options ]
*/

#ifndef QT_PARSE_OPT_H
#define QT_PARSE_OPT_H

#include "../../common.h"
#include "qt_utils.h"
#include "../../vm/vm_defs.h"

class GUI;

/// @brief parse command line options
class CParseOptions
{
private:
	QTChar app_path;
	QTChar app_name;
	QTChar ini_path;
	QTChar ini_file;
	QTChar res_path;

	QTChar *tape_file;
	QTChar *disk_file[USE_DRIVE];
	QTChar *state_file;
	QTChar *autokey_file;
	QTChar *reckey_file;
#ifdef USE_DEBUGGER
	int debugger_imm_start;
#endif

	int get_options(int ac, char *av[]);


	CParseOptions() {}
	CParseOptions(const CParseOptions &) {}
	CParseOptions &operator=(const CParseOptions &) { return *this; }

public:
	CParseOptions(int ac, char *av[]);
	~CParseOptions();

	void open_recent_file(GUI *gui);

	const _TCHAR *get_app_path();
	const _TCHAR *get_app_name();
	const _TCHAR *get_ini_path();
	const _TCHAR *get_ini_file();
	const _TCHAR *get_res_path();
};

#endif /* QT_PARSE_OPT_H */
