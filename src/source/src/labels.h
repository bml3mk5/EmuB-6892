/** @file labels.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2021.04.18 -

	@brief [ labels ]
*/

#ifndef LABELS_H
#define LABELS_H

#include "common.h"
#include "msgs.h"

namespace LABELS {

extern const CMsg::Id tabs[];

extern const CMsg::Id io_port[];
extern const uint8_t io_port_pos[];

extern const CMsg::Id sys_mode[];
extern const CMsg::Id fdd_type[];

extern const CMsg::Id correct[];
extern const _TCHAR *correct_amp[];

extern const _TCHAR *sound_rate[];
extern const _TCHAR *sound_bits[];

extern const _TCHAR *disp_skew[];

#ifdef USE_DIRECT3D
extern const CMsg::Id d3d_use[];
extern const CMsg::Id d3d_filter[];
#endif

#ifdef USE_OPENGL
extern const CMsg::Id opengl_use[];
extern const CMsg::Id opengl_filter[];
#endif

extern const CMsg::Id led_show[];
extern const CMsg::Id led_pos[];

extern const _TCHAR *capture_fmt[];

extern const CMsg::Id comm_baud[];

extern const _TCHAR *comm_uart_baudrate[];
extern const _TCHAR *comm_uart_databit[];
extern const CMsg::Id comm_uart_parity[];
extern const _TCHAR *comm_uart_stopbit[];
extern const CMsg::Id comm_uart_flowctrl[];

#if defined(_MBS1)
extern const CMsg::Id exram_size[];
extern const _TCHAR *fmopn_clock[];
extern const CMsg::Id fmopn_irq[];
//extern const CMsg::Id expsg_type[];
extern const CMsg::Id type_of_soundcard[];

#if defined(USE_Z80B_CARD)
extern const CMsg::Id z80bcard_irq[];
#endif
#endif

extern const CMsg::Id volume[];

extern const CMsg::Id keybind_col[][2];
extern const CMsg::Id keybind_tab[];
extern const CMsg::Id keybind_btn[];
extern const CMsg::Id keybind_combi[];

}; /* namespace LABELS */

#endif /* LABELS_H */

