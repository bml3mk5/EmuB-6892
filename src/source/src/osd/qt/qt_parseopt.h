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
#include "../parseopt.h"

class GUI;

/// @brief parse command line options
class CParseOptions : public CParseOptionsBase
{
private:
	int get_options(int ac, char *av[]);

	bool get_module_file_name(_TCHAR *path, int size);

	CParseOptions();
	CParseOptions(const CParseOptions &);

public:
	CParseOptions(int ac, char *av[]);
	~CParseOptions();
};

#endif /* QT_PARSE_OPT_H */
