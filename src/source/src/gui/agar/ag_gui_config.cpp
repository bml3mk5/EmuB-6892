/** @file ag_gui_config.cpp

	Skelton for retropc emulator
	SDL + Ager edition

	@author Sasaji
	@date   2012.3.2

	@brief [ ag_gui_base ]
*/

#include "ag_gui_base.h"
#include "../gui.h"
#include "../../config.h"

namespace GUI_AGAR
{

#define FONT_MAX 3

/// search font for Ager GUI
/// @return 0:found 1:not found (use default font)
int AG_GUI_BASE::ConfigLoad()
{
	char fpath[AG_PATHNAME_MAX];

	struct fdata_st {
		char name[AG_PATHNAME_MAX];
		int  size;
		Uint32 flags;
	} fdata[FONT_MAX];

	memset(fdata, 0, sizeof(fdata));

#if defined(_WIN32)
#ifdef AG_PATHSEPMULTI
	AG_PrtString(agConfig, "font-path", "%s;%s;%s;%s\\fonts"
						,config.font_path.GetN(),emu->resource_path(),emu->application_path(),SDL_getenv("SystemRoot"));
#else
	AG_PrtString(agConfig, "font-path", "%s:%s:%s:%s\\fonts"
						,config.font_path.GetN(),emu->resource_path(),emu->application_path(),SDL_getenv("SystemRoot"));
#endif
	strcpy(fdata[0].name, config.menu_fontname); fdata[0].size = config.menu_fontsize;
	AG_SetInt(agConfig, "font.size", config.menu_fontsize);

#elif defined(linux)
	AG_PrtString(agConfig, "font-path", "%s:"
										"%s:%s:"
										"%s/.fonts:"
										"/usr/share/fonts/truetype:"
										"/usr/share/fonts/opentype:"
										"/usr/share/fonts/opentype/ipafont:"
										"/usr/share/fonts/truetype/freefont:"
										"/usr/X11R6/lib/X11/fonts/TTF"
						,config.font_path.GetN(),emu->resource_path(),emu->application_path(),getenv("HOME"));
	strcpy(fdata[0].name, config.menu_fontname);  fdata[0].size = config.menu_fontsize;
	strcpy(fdata[1].name, "ttf-japanese-gothic.ttf"); fdata[1].size = config.menu_fontsize;
	strcpy(fdata[2].name, "fonts-japanese-gothic.ttf"); fdata[2].size = config.menu_fontsize;
	AG_SetInt(agConfig, "font.size", config.menu_fontsize);

#elif defined(__APPLE__) && defined(__MACH__)
	AG_PrtString(agConfig, "font-path", "%s:"
										"%s:%s:"
										"%s/Library/Fonts:/Library/Fonts:"
										"/System/Library/Fonts"
						,config.font_path.GetN(),emu->resource_path(),emu->application_path(),getenv("HOME"));
	strcpy(fdata[0].name, config.menu_fontname);  fdata[0].size = config.menu_fontsize;
	strcpy(fdata[1].name, "Hiragino Sans GB W3.otf");  fdata[1].size = config.menu_fontsize;
	AG_SetInt(agConfig, "font.size", config.menu_fontsize);

#elif defined(__FreeBSD__)
	AG_PrtString(agConfig, "font-path", "%s:"
										"%s:%s:"
										"%s/.fonts:"
										"/usr/share/fonts/TTF:"
										"/usr/local/share/font-mona-ipa/fonts/:"
										"/usr/local/lib/X11/fonts/TTF"
						,config.font_path.GetN(),emu->resource_path(),emu->application_path(),getenv("HOME"));
	strcpy(fdata[0].name, config.menu_fontname);  fdata[0].size = config.menu_fontsize;
	strcpy(fdata[1].name, "ipagp.ttf");  fdata[1].size = config.menu_fontsize;
	AG_SetInt(agConfig, "font.size", config.menu_fontsize);

#endif

	// search specified fonts
	bool found = false;
	for(int i=0; i< FONT_MAX; i++) {
		if (fdata[i].name[0] == '\0') {
			continue;
		}
		if (AG_ConfigFile("font-path", fdata[i].name, NULL, fpath, AG_PATHNAME_MAX - 1) == 0) {
			// found
			logging->out_logf(LOG_INFO, _T("AG_MENU: use font: %s"), fpath);
			AG_PrtString(agConfig, "font.face", "%s", fdata[i].name);
			AG_SetInt(agConfig, "font.size", fdata[i].size);
			AG_SetUint32(agConfig, "font.flags", fdata[i].flags);
			found = true;
			break;
		} else {
			// not found
			logging->out_logf(LOG_WARN, _T("AG_ConfigFile: %s"), AG_GetError());
		}
	}
	return (found ? 0 : 1);
}

int AG_GUI_BASE::ConfigSave()
{
	return 0;
}

}; /* namespace GUI_AGAR */
