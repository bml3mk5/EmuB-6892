/** @file wxw_screenmode.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2019.10.01

	@brief [ screen mode ]
*/

#include <wx/wx.h>
#include "../screenmode.h"
#include "../../vm/vm_defs.h"
#include "../../logging.h"
#include <wx/display.h>
#include <wx/gdicmn.h>


ScreenMode::ScreenMode()
	: ScreenModeBase()
{
}

ScreenMode::~ScreenMode()
{
}

void ScreenMode::Enum()
{
	int w, h, bpp;
	GetDesktopSize(&w, &h, &bpp);
	Enum(w, h, bpp);
}

void ScreenMode::Enum(int desktop_width, int desktop_height, int bits_per_pixel)
{
//	int max_desktop_width = 0;
//	int max_desktop_height = 0;

	major_bits_per_pixel = bits_per_pixel;

	// enumerate screen mode for fullscreen

	// enumerate display device
	// the number of connected display monitors
	int disp_device_count = (int)wxDisplay::GetCount();
	if (disp_device_count > DISP_DEVICE_MAX) {
		disp_device_count = DISP_DEVICE_MAX;
	}
	for(int disp_no = 0; disp_no < disp_device_count; disp_no++) {
		CDisplayDevice *item = new CDisplayDevice();
		wxDisplay disp(disp_no);
		wxRect nre = disp.GetGeometry();
		RECT_IN(item->re, nre.GetLeft(), nre.GetTop(), nre.GetWidth(), nre.GetHeight());
		disp_devices.Add(item);
	}

	for(int disp_no = 0; disp_no < disp_devices.Count(); disp_no++) {
		CDisplayDevice *dd = disp_devices[disp_no];

		wxDisplay disp(disp_no);

		// get video modes
		wxArrayVideoModes vmodes = disp.GetModes();
		wxVideoMode cmode = disp.GetCurrentMode();

		logging->out_debugf(_T("screen_mode: current [%d] %dx%d %dbpp %dHz"), disp_no, cmode.GetWidth(), cmode.GetHeight(), cmode.GetDepth(), cmode.GetRefresh());

		for(int i = 0; i < (int)vmodes.Count() && dd->modes.Count() < VIDEO_MODE_MAX; i++) {
			int w = vmodes[i].GetWidth();
			int h = vmodes[i].GetHeight();
			int d = vmodes[i].GetDepth();
			int r = vmodes[i].GetRefresh();

			if (!dd->modes.IsValidSize(w, h)) {
				logging->out_debugf(_T("screen_mode:-- [%d] %dx%d %dbpp %dHz ignored"), disp_no, w, h, d, r);
				continue;
			}

			int found = dd->modes.Find(w, h);
			if (found >= 0) {
				logging->out_debugf(_T("screen_mode:-- [%d] %dx%d %dbpp %dHz already exist"), disp_no, w, h, d, r);
			} else {
				CVideoMode *item = new CVideoMode();
				item->Set(w, h);
				dd->modes.Add(item);
				logging->out_debugf(_T("screen_mode:%2d [%d] %dx%d %dbpp %dHz"), dd->modes.Count(), disp_no, w, h, d, r);

//				if(max_desktop_width <= w) {
//					max_desktop_width = w;
//				}
//				if(max_desktop_height <= h) {
//					max_desktop_height = h;
//				}
			}
		}
		dd->modes.Sort();

		// if cannot get modes, add default screen size mode.
		if (dd->modes.Count() == 0) {
			CVideoMode *item = new CVideoMode();
			item->Set(dd->re.w, dd->re.h);
			dd->modes.Add(item);
//			wxVideoMode cmode(desktop_width, desktop_height, wxDisplayDepth(), 60);
//			screen_mode_wx[screen_mode_count] = cmode;
//			logging->out_debugf(_T("screen_mode:%2d [%d] %dx%d %dbpp %dHz"), screen_mode_count, 0, cmode.GetWidth(), cmode.GetHeight(), cmode.GetDepth(), cmode.GetRefresh());

//			max_desktop_width = desktop_width;
//			max_desktop_height = desktop_height;
		}
	}
}

void ScreenMode::GetDesktopSize(int *width, int *height, int *bpp)
{
	// desktop size on the primary monitor
	wxDisplaySize(width, height);
	if (bpp) *bpp = wxDisplayDepth();
}
