/** @file win_emu.cpp

	Skelton for retropc emulator

	@author Takeda.Toshiya
	@date   2006.08.18 -

	@note
	Modified by Sasaji at 2011.06.17

	@brief [ win32 emulation i/f ]
*/

#include "win_emu.h"
#include "win_main.h"
#include "win_apiex.h"
#include "../../config.h"
#include "../../fileio.h"
#include "../../gui/gui.h"
//#include "ledbox.h"
#ifdef USE_MESSAGE_BOARD
#include "../../msgboard.h"
#endif
#include "win_csurface.h"
#include "../../labels.h"

// ----------------------------------------------------------------------------
// initialize
// ----------------------------------------------------------------------------

EMU_OSD::EMU_OSD(const _TCHAR *new_app_path, const _TCHAR *new_ini_path, const _TCHAR *new_res_path)
	: EMU(new_app_path, new_ini_path, new_res_path)
{
	WIN_API_EX::Load();
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
	WIN_API_EX::Unload();
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
		idx = (LABELS::GetDrawingMethodIndex(prev_method) + 1) % count;
	}
	pConfig->drawing_method = LABELS::drawing_method_idx[idx];

	lock_screen();

	create_mixedsurface();

	switch(pConfig->drawing_method & DRAWING_METHOD_ALL_MASK) {
	case DRAWING_METHOD_DIRECT2D_MASK:
		reset_d2drender(hMainWindow);
		break;

	case DRAWING_METHOD_DIRECT3D_MASK:
		set_d3dpresent_interval();
		reset_d3device(hMainWindow);

		if (enabled_drawing_method & DRAWING_METHOD_DIRECT3D_MASK) {
			if (!(prev_method & DRAWING_METHOD_DIRECT3D_MASK)) {
#ifdef USE_SCREEN_D3D_TEXTURE
				copy_d3dtex_dib(pD3Dorigin->GetD3DTexture(), sufOrigin->GetBuffer(), false);
#else
				copy_d3dsuf_dib(pD3Dorigin->GetD3DSurface(), sufOrigin->GetBuffer(), false);
#endif
			}
		}
		break;
	default:
		if (enabled_drawing_method & DRAWING_METHOD_DIRECT3D_MASK) {
			if (prev_method & DRAWING_METHOD_DIRECT3D_MASK) {
#ifdef USE_SCREEN_D3D_TEXTURE
				copy_d3dtex_dib(pD3Dorigin->GetD3DTexture(), sufOrigin->GetBuffer(), true);
#else
				copy_d3dsuf_dib(pD3Dorigin->GetD3DSurface(), sufOrigin->GetBuffer(), true);
#endif
			}
		}
		break;
	}

	unlock_screen();

	set_ledbox_position(!is_fullscreen());

	set_msgboard_position();

	out_infoc_x(LABELS::drawing_method[idx], 0);
}
