/** @file qt_debugger_console.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2019.10.01 -

	@brief [ qt debugger console ]
*/

#include "../../vm/vm_defs.h"

#ifdef USE_DEBUGGER

#include "qt_debugger_console.h"
#include "../debugger_console.h"

DebuggerThread::DebuggerThread(debugger_thread_t *param, QObject *parent)
	: QThread(parent)
{
	p = param;
}

void DebuggerThread::run()
{
	p->running = true;

	// initialize
	DebuggerConsole *dc = new DebuggerConsole(p);

	dc->Process();

	// release
	delete dc;

	p->running = false;
}

#endif /* USE_DEBUGGER */
