/** @file wxw_debugger_console.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2019.10.01 -

	@brief [ wxWidgets debugger console ]
*/

#ifndef WXW_DEBUGGER_CONSOLE_H
#define WXW_DEBUGGER_CONSOLE_H

#include "../../vm/vm_defs.h"

#ifdef USE_DEBUGGER

#include "../../common.h"
#include "../../debugger_defs.h"
#include <wx/thread.h>

/**
	@brief Thread to run a debugger
*/
class DebuggerThread : public wxThread
{
public:
	DebuggerThread(debugger_thread_t *param);
	~DebuggerThread();
	bool isRunning() const;
	void start();
	int wait();
private:
	ExitCode Entry();
	debugger_thread_t *p;
};

#endif /* USE_DEBUGGER */
#endif /* WXW_DEBUGGER_CONSOLE_H */

