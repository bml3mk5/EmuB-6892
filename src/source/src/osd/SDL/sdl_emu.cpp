/** @file sdl_emu.cpp

	Skelton for retropc emulator
	SDL edition

	@author Sasaji
	@date   2012.02.21

	@brief [ sdl emulation i/f ]

	@note
	This code is based on the Common Source Code Project.
	Original Author : Takeda.Toshiya
*/

#include "sdl_emu.h"
#include "../../config.h"
#include "../../fileio.h"
#include "../../gui/gui.h"
//#include "ledbox.h"
#ifdef USE_MESSAGE_BOARD
#include "../../msgboard.h"
#endif
#include "sdl_csurface.h"
#include "../../labels.h"

// ----------------------------------------------------------------------------
// initialize
// ----------------------------------------------------------------------------

EMU_OSD::EMU_OSD(const _TCHAR *new_app_path, const _TCHAR *new_ini_path, const _TCHAR *new_res_path)
	: EMU(new_app_path, new_ini_path, new_res_path)
{
	EMU_INPUT();
	EMU_SCREEN();
	EMU_SOUND();
#ifdef USE_SOCKET
	EMU_SOCKET();
#endif
#ifdef USE_UART
	EMU_UART();
#endif
}

EMU_OSD::~EMU_OSD()
{
}

void EMU_OSD::sleep(uint32_t ms)
{
	CDelay(ms);
}

void EMU_OSD::change_drawing_method(int method)
{
	int count = LABELS::MakeDrawingMethodList(enabled_drawing_method);
	int idx;

	uint8_t prev_method = pConfig->drawing_method;
	if (method >= 0) {
		idx = LABELS::GetDrawingMethodIndex((uint8_t)method);
	} else {
		gui->RestoreDrawingMethod(prev_method);
		idx = (LABELS::GetDrawingMethodIndex(prev_method) + 1) % count;
	}
	uint8_t new_method = LABELS::drawing_method_idx[idx];

	bool disp_msg = (new_method != prev_method);

	// device will change
	if (!gui->StoreDrawingMethod(new_method)) {
		// restart this app to change drawing method
		if (disp_msg) out_infoc_x(LABELS::drawing_method[idx], CMsg::LB_Need_restart_program_RB, 0);
		return;
	}

	// change drawing method immediately

	pConfig->drawing_method = new_method;

	switch(pConfig->drawing_method & DRAWING_METHOD_ALL_MASK) {
#ifdef USE_OPENGL
	case DRAWING_METHOD_OPENGL_MASK:
		restart_screen();
		break;
#endif
	default:
		restart_screen();
		break;
	}

	set_ledbox_position(!is_fullscreen());

	set_msgboard_position();

	if (disp_msg) out_infoc_x(LABELS::drawing_method[idx], 0);
}
