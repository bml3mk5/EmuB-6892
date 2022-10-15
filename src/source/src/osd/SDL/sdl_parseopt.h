/** @file sdl_parseopt.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.05.18 -

	@brief [ parse options ]
*/

#ifndef SDL_PARSE_OPT_H
#define SDL_PARSE_OPT_H

#include "../../common.h"
#include "../../cchar.h"
#include "../../vm/vm_defs.h"

class GUI;

/// @brief parse command line options
class CParseOptions
{
private:
	CTchar app_path;
	CTchar app_name;
	CTchar ini_path;
	CTchar ini_file;
	CTchar res_path;

	CTchar *tape_file;
	CTchar *disk_file[USE_DRIVE];
	CTchar *state_file;
	CTchar *autokey_file;
	CTchar *reckey_file;
#ifdef USE_DEBUGGER
	int debugger_imm_start;
#endif

	int get_options(int ac, char *av[]);
	static bool get_module_file_name(_TCHAR *path, int size);

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

#endif /* SDL_PARSE_OPT_H */
