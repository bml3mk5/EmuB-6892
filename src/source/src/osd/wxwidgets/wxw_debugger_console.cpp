/** @file wxw_debugger_console.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2019.10.01 -

	@brief [ wxWidgets debugger console ]
*/

#include "../../vm/vm_defs.h"

#ifdef USE_DEBUGGER

#include "../../depend.h"
#include "wxw_debugger_console.h"
#include "../debugger_console.h"
#include <wx/timer.h>

wxThread::ExitCode DebuggerThread::Entry()
{
	p->running = true;

	// initialize
	DebuggerConsole *dc = new DebuggerConsole(p);

	dc->Process();

	// release
	delete dc;

	p->running = false;

	return 0;
}

DebuggerThread::DebuggerThread(debugger_thread_t *param)
	: wxThread(wxTHREAD_JOINABLE)
{
	p = param;
}

DebuggerThread::~DebuggerThread()
{
}

void DebuggerThread::start()
{
	this->Run();
}

int DebuggerThread::wait()
{
	int status = 0;
	status = (int)(intptr_t)this->Wait();
	return status;
}

bool DebuggerThread::isRunning() const
{
	return this->IsRunning();
}

#endif /* USE_DEBUGGER */
