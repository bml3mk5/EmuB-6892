/** @file qt_timer.cpp

	Skelton for retropc emulator
	SDL edition

	@author Sasaji
	@date   2012.2.21

	@brief [ sdl timer ]

	@note
	This code is based on win32_timer.cpp of the Common Source Code Project.
	Author : Takeda.Toshiya
*/

#include "qt_emu.h"

void EMU_OSD::update_timer()
{
	sTime = QDateTime::currentDateTime();
}

void EMU_OSD::get_timer(int *time, size_t size) const
{
/*
	0	year
	1	month
	2	day
	3	day of week
	4	hour
	5	minute
	6	second
	7	milli seconds
*/
	if (size < 8) return;

	QDate dt = sTime.date();
	QTime ti = sTime.time();
	time[0] = dt.year();
	time[1] = dt.month();
	time[2] = dt.day();
	time[3] = dt.dayOfWeek();
	time[4] = ti.hour();
	time[5] = ti.minute();
	time[6] = ti.second();
	time[7] = ti.msec();
}

