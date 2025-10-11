/** @file windowmode.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2019.10.01

	@brief [ window mode ]
*/

#include "windowmode.h"
#include "../emu.h"
#include <algorithm>
#include <stdlib.h>
//#include "../main.h"

static const VmSize window_proposal[] = {
	{640,400},{640,480},{800,600},{1024,768},{0,0}
};

CWindowMode::CWindowMode()
{
	magnify = 1.0;
	width = 0;
	height = 0;
}

CWindowMode::CWindowMode(double magnify_, int width_, int height_)
{
	Set(magnify_, width_, height_);
}

CWindowMode::~CWindowMode()
{
}

void CWindowMode::Set(double magnify_, int width_, int height_)
{
	magnify = magnify_;
	width = (int)((double)width_ * magnify_ + 0.5);
	height = (int)((double)height_ * magnify_ + 0.5);
}

bool CWindowMode::Match(double magnify_, int width_, int height_) const
{
	return (magnify == magnify_
		&& width == (int)((double)width_ * magnify_ + 0.5)
		&& height == (int)((double)height_ * magnify_ + 0.5));
}

//

CWindowModes::CWindowModes()
	: CPtrList<CWindowMode>()
{
}

bool CWindowModes::greater(const CWindowMode *a, const CWindowMode *b)
{
	int rc = 0;
	if (!rc) rc = (a->width - b->width);
	if (!rc) rc = (a->height - b->height);
	if (!rc) rc = (int)(a->magnify - b->magnify);
	return (rc < 0);
}

void CWindowModes::Sort()
{
	std::sort(items.begin(), items.end(), greater);
}

int CWindowModes::Find(double magnify_, int width_, int height_) const
{
	for(int i=0; i<Count(); i++) {
		if (Item(i)->Match(magnify_, width_, height_)) {
			return i;
		}
	}
	return -1;
}

//

WindowMode::WindowMode()
{
}

WindowMode::~WindowMode()
{
}

/// enumerate screen mode for window
/// calculate magnify range
/// @param [in] max_width
/// @param [in] max_height
void WindowMode::Enum(int max_width, int max_height)
{
	// enumerate screen mode for window
	int w,h;
	bool minsize = false;
	bool maxsize = false;

	for (int i=0; window_proposal[i].w != 0 && window_modes.Count() < WINDOW_MODE_MAX; i++) {
		w = window_proposal[i].w;
		h = window_proposal[i].h;
		// minimum window size?
		if (w < MIN_WINDOW_WIDTH || h < MIN_WINDOW_HEIGHT) {
			continue;
		}
		if (!minsize) {
			w = MIN_WINDOW_WIDTH;
			h = MIN_WINDOW_HEIGHT;
			i--;
			minsize = true;
		}
		if (w > MAX_WINDOW_WIDTH || h > MAX_WINDOW_HEIGHT) {
			if (!maxsize) {
				w = MAX_WINDOW_WIDTH;
				h = MAX_WINDOW_HEIGHT;
				maxsize = true;
				i--;
			} else {
				break;
			}
		}
		for (double pow = 1.0; pow <= 3.0 && window_modes.Count() < WINDOW_MODE_MAX; pow += 0.5) {
			if ((int)((double)w*pow) <= max_width && (int)((double)h*pow) <= max_height) {
				int found = window_modes.Find(pow, w, h);
				if (found < 0) {
					window_modes.Add(new CWindowMode(pow, w, h));
					logging->out_debugf(_T("window_mode:%d %dx%d x%.1f")
						, window_modes.Count()
						, (int)((double)w * pow + 0.5)
						, (int)((double)h * pow + 0.5)
						, pow
					);
				}
			} else {
				break;
			}
		}
	}
	window_modes.Sort();
}

const CWindowMode *WindowMode::Get(int num) const
{
	if (0 <= num && num < window_modes.Count()) {
		return window_modes[num];
	}
	return NULL;
}

int WindowMode::Find(int width, int height) const
{
	return window_modes.Find(1.0, width, height);
}
