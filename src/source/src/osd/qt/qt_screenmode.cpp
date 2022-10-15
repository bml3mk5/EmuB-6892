/** @file qt_screenmode.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2019.10.01

	@brief [ screen mode ]
*/

#include "qt_screenmode.h"
#include <QApplication>
#include <QScreen>
#include "../../vm/vm_defs.h"
#include "../../logging.h"
#include "../../main.h"


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

	// enumerate display device
	QList<QScreen *> scrns = qApp->screens();
	int disp_device_count = scrns.count();
	if (disp_device_count > DISP_DEVICE_MAX) {
		disp_device_count = DISP_DEVICE_MAX;
	}
	for(int disp_no = 0; disp_no < disp_device_count; disp_no++) {
		CDisplayDevice *item = new CDisplayDevice();
		QRect nre = scrns[disp_no]->geometry();
		RECT_IN(item->re, nre.left(), nre.top(), nre.width(), nre.height())
		disp_devices.Add(item);
	}

	// enumerate screen mode for fullscreen
	int w,h,bpp;

	for(int disp_no = 0; disp_no < disp_devices.Count(); disp_no++) {
		CDisplayDevice *dd = disp_devices[disp_no];
		QSize sz = scrns[disp_no]->availableSize();
		bpp = scrns[disp_no]->depth();

		for(int i = 0; i < 1 && dd->modes.Count() < VIDEO_MODE_MAX; i++) {
			w = sz.width();
			h = sz.height();

			if (!dd->modes.IsValidSize(w, h)) {
				logging->out_debugf(_T("screen_mode:-- [%d] %dx%d %dbpp ignored"), disp_no, w, h, bpp);
				continue;
			}

			int found = dd->modes.Find(w, h);
			if (found >= 0) {
				logging->out_debugf(_T("screen_mode:-- [%d] %dx%d %dbpp already exist"), disp_no, w, h, bpp);
			} else {
				CVideoMode *item = new CVideoMode();
				item->Set(w, h);
				dd->modes.Add(item);
				logging->out_debugf(_T("screen_mode:%2d [%d] %dx%d %dbpp"), dd->modes.Count(), disp_no, w, h, bpp);

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

//			max_desktop_width = desktop_width;
//			max_desktop_height = desktop_height;
		}
	}
}

void ScreenMode::GetDesktopSize(int *width, int *height, int *bpp)
{
	// desktop size on the primary monitor
	const QScreen *current_desktop = qApp->screens().at(0);
	if (width) *width = current_desktop->size().width();
	if (height) *height = current_desktop->size().height();
	if (bpp) *bpp = current_desktop->depth();
}
