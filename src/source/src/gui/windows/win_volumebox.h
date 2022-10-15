/** @file win_volumebox.h

	HITACHI BASIC MASTER LEVEL3 Mark5 Emulator 'EmuB-6892'
	HITACHI MB-S1 Emulator 'EmuB-S1'
	Skelton for retropc emulator

	@author Sasaji
	@date   2013.10.30 -

	@brief [ volume box ]
*/

#ifndef WIN_VOLUMEBOX_H
#define WIN_VOLUMEBOX_H

#include <windows.h>
#include "win_dialogbox.h"
#include "../../config.h"

namespace GUI_WIN
{
/**
	@brief Volume dialog box
*/
class VolumeBox : public CDialogBox
{
private:
	static INT_PTR CALLBACK VolumeBoxProc(HWND, UINT, WPARAM, LPARAM);

	INT_PTR onInitDialog(UINT, WPARAM, LPARAM);
	INT_PTR onHScroll(UINT, WPARAM, LPARAM);
	INT_PTR onVScroll(UINT, WPARAM, LPARAM);
	INT_PTR onCommand(UINT, WPARAM, LPARAM);

	void SetVolume();
	void SetVolumeText(int num);

	int *volumes[VOLUME_NUMS];
	bool *mutes[VOLUME_NUMS];

public:
	VolumeBox(HINSTANCE, CFont *, EMU *, GUI *);
	~VolumeBox();

};

}; /* namespace GUI_WIN */

#endif /* WIN_VOLUMEBOX_H */
