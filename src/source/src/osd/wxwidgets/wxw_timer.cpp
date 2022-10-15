/** @file wxw_timer.cpp

	Skelton for retropc emulator
	wxWidgets edition

	@author Sasaji
	@date   2012.2.21

	@brief [ wxw timer ]

	@note
	This code is based on win32_timer.cpp of the Common Source Code Project.
	Author : Takeda.Toshiya
*/

//#include <wx/wx.h>
#include "wxw_emu.h"


void EMU_OSD::update_timer()
{
//	GetLocalTime(&sTime);
	sTime = wxDateTime::UNow();
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

	time[0] = sTime.GetYear();
	time[1] = sTime.GetMonth();
	time[2] = sTime.GetDay();
	time[3] = sTime.GetWeekDay();
	time[4] = sTime.GetHour();
	time[5] = sTime.GetMinute();
	time[6] = sTime.GetSecond();
	time[7] = sTime.GetMillisecond();
}

