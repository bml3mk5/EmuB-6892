/** @file loadlibrary.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2025.10.19 -

	@brief [ load dynamic library ]
*/

#ifndef LOADLIBRARY_H
#define LOADLIBRARY_H

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace LOAD_LIBRARY
{

#define LOAD_LIB(handle, app_path, lib_base, version) \
	handle = LOAD_LIBRARY::Load(app_path, lib_base, version); \
	if (!handle) continue;

#define GET_ADDR(func, func_type, handle, func_name) \
	func = (func_type)LOAD_LIBRARY::GetAddr(handle, func_name); \
	if (!func) continue;

#define GET_ADDR_OPTIONAL(func, func_type, handle, func_name) \
	func = (func_type)LOAD_LIBRARY::GetAddr(handle, func_name);

#define UNLOAD_LIB(handle) \
	LOAD_LIBRARY::Unload(&handle)

#define CHECK_VERSION(current, require, libname) { \
	if (current < require) { \
		logging->out_logf(LOG_DEBUG, "Cannot use %s because version is different from %d.", libname, (int)require); \
		continue; \
	} \
}

#if defined(_WIN32)

extern HMODULE Load(const char *app_path, const char *lib_base, int version);
extern FARPROC GetAddr(HMODULE handle, LPCSTR func_name);
extern void Unload(HMODULE *handle);

#else /* !_WIN32 */

extern void *Load(const char *app_path, const char *lib_base, int version);
extern void *GetAddr(void *handle, const char *func_name);
extern void Unload(void **handle);

#endif

}; /* namespace LOAD_LIBRARY */

#endif /* LOADLIBRARY_H */
