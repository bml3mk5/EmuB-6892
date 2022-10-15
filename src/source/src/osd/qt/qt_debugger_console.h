/** @file qt_debugger_console.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2019.10.01 -

	@brief [ qt debugger console ]
*/

#ifndef QT_DEBUGGER_CONSOLE_H
#define QT_DEBUGGER_CONSOLE_H

#include "../../vm/vm_defs.h"

#ifdef USE_DEBUGGER

#include "../../common.h"
#include "../../debugger_defs.h"
#include <QThread>

/**
	@brief Thread to run a debugger
*/
class DebuggerThread : public QThread
{
	Q_OBJECT

public:
	DebuggerThread(debugger_thread_t *param, QObject *parent = Q_NULLPTR);
protected:
	void run();
private:
	debugger_thread_t *p;
};

#endif /* USE_DEBUGGER */
#endif /* QT_DEBUGGER_CONSOLE_H */

