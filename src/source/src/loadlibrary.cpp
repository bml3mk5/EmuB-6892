/** @file loadlibrary.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2025.10.19 -

	@brief [ load dynamic library ]
*/

#include "common.h"
#include "loadlibrary.h"
#include "logging.h"
#include "utility.h"

namespace LOAD_LIBRARY
{

#if defined(_WIN32)

HMODULE Load(const char *app_path, const char *lib_base, int version)
{
	char libname[_MAX_PATH];
	libname[0]='\0';
	if (app_path) {
		UTILITY::strcat(libname, _MAX_PATH, app_path);
	}
	UTILITY::strcat(libname, _MAX_PATH, lib_base);
	if (version) {
		UTILITY::sprintf(&libname[strlen(libname)], _MAX_PATH - strlen(libname), "-%d", version);
	}
	UTILITY::strcat(libname, _MAX_PATH, ".dll");
	HMODULE handle = LoadLibraryA(libname);
	if (!handle) {
		logging->out_logf(LOG_DEBUG, "Cannot load %s.", libname);
	} else {
		logging->out_logf(LOG_DEBUG, "Loaded %s.", libname);
	}
	return handle;
}

FARPROC GetAddr(HMODULE handle, LPCSTR func_name)
{
	FARPROC func = GetProcAddress(handle, func_name);
	if (!func) {
		logging->out_logf(LOG_DEBUG, "Cannot get address of %s.", func_name);
	}
	return func;
}

void Unload(HMODULE *handle)
{
	if (*handle) FreeLibrary(*handle);
	*handle = NULL;
}

#else /* !_WIN32 */

#if defined(__APPLE__) && defined(__MACH__)

void *Load(const char *app_path, const char *lib_base, int version)
{
	char libname[_MAX_PATH];
	libname[0]='\0';
	if (app_path) {
		UTILITY::strcat(libname, _MAX_PATH, app_path);
	}
	UTILITY::strcat(libname, _MAX_PATH, "lib");
	UTILITY::strcat(libname, _MAX_PATH, lib_base);
	if (version) {
		UTILITY::sprintf(&libname[strlen(libname)], _MAX_PATH - strlen(libname), ".%d", version);
	}
	UTILITY::strcat(libname, _MAX_PATH, ".dylib");
	void *handle = dlopen(libname, RTLD_NOW | RTLD_GLOBAL);
	if (!handle) {
		logging->out_logf(LOG_DEBUG, _T("Cannot load %s."), libname);
	} else {
		logging->out_logf(LOG_DEBUG, _T("Loaded %s."), libname);
	}
	return handle;
}

#else

void *Load(const char *app_path, const char *lib_base, int version)
{
	char libname[_MAX_PATH];
	libname[0]='\0';
	if (app_path) {
		UTILITY::strcat(libname, _MAX_PATH, app_path);
	}
	UTILITY::strcat(libname, _MAX_PATH, "lib");
	UTILITY::strcat(libname, _MAX_PATH, lib_base);
	UTILITY::strcat(libname, _MAX_PATH, ".so");
	if (version) {
		UTILITY::sprintf(&libname[strlen(libname)], _MAX_PATH - strlen(libname), ".%d", version);
	}
	void *handle = dlopen(libname, RTLD_NOW | RTLD_GLOBAL);
	if (!handle) {
		logging->out_logf(LOG_DEBUG, _T("Cannot load %s."), libname);
	} else {
		logging->out_logf(LOG_DEBUG, _T("Loaded %s."), libname);
	}
	return handle;
}

#endif

void *GetAddr(void *handle, const char *func_name)
{
	void *func = dlsym(handle, func_name);
	if (!func) {
		logging->out_logf(LOG_DEBUG, _T("Cannot get address of %s."), func_name);
	}
	return func;
}

void Unload(void **handle)
{
	if (*handle) dlclose(*handle);
	*handle = NULL;
}

#endif

}; /* namespace LOAD_LIBRARY */
