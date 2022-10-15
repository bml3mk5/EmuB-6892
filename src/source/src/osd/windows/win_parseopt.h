/** @file win_parseopt.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.05.18 -

	@brief [ parse options ]
*/

#ifndef WIN_PARSE_OPT_H
#define WIN_PARSE_OPT_H

#include "../../common.h"
#include "../../cchar.h"
#include "../../vm/vm_defs.h"
#include <windows.h>

class GUI;

/// @brief parse command line options
class CParseOptions
{
private:
	CTchar app_path;
	CTchar app_name;
	CTchar ini_path;
	CTchar ini_file;

	CTchar *tape_file;
#ifdef USE_FD1
	CTchar *disk_file[USE_DRIVE];
#endif
	CTchar *state_file;
	CTchar *autokey_file;
	CTchar *reckey_file;
#ifdef USE_DEBUGGER
	int debugger_imm_start;
#endif

	int get_options(LPTSTR szCmdLine);

	CParseOptions() {}
	CParseOptions(const CParseOptions &) {}
	CParseOptions &operator=(const CParseOptions &) { return *this; }

public:
	CParseOptions(LPTSTR szCmdLine);
	~CParseOptions();

	void open_recent_file(GUI *gui);

	const _TCHAR *get_app_path();
	const _TCHAR *get_app_name();
	const _TCHAR *get_ini_path();
	const _TCHAR *get_ini_file();
};

#endif /* WIN_PARSE_OPT_H */