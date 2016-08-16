/*****************************************************************************/
/* High Resolution System timer (1us) Win32 driver                           */
/*                                                                           */
/* Copyright (C) 2014-2015 Laszlo Arvai                                      */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
// Includes
#include <sysHighresTimer.h>

static LARGE_INTEGER l_frequency;

void sysHighresTimerInit(void)
{
	QueryPerformanceFrequency(&l_frequency);
}

sysHighresTimestamp sysHighresTimerGetTimestamp(void)
{
	LARGE_INTEGER timestamp;
	QueryPerformanceCounter(&timestamp);

	return timestamp;
}

uint32_t sysHighresTimerGetTimeSince(sysHighresTimestamp in_timestamp)
{
	LARGE_INTEGER current_time;
  LARGE_INTEGER ellapsed_microseconds;
   
  QueryPerformanceCounter(&current_time);
	ellapsed_microseconds.QuadPart = current_time.QuadPart - in_timestamp.QuadPart;

	ellapsed_microseconds.QuadPart *= 1000000;
	ellapsed_microseconds.QuadPart /= l_frequency.QuadPart;

	return (uint32_t)ellapsed_microseconds.QuadPart;
}

void sysHighresTimerAddToTimestamp(sysHighresTimestamp* in_timestamp, uint32_t in_value_in_us)
{
	LARGE_INTEGER add;

	add.QuadPart = l_frequency.QuadPart * in_value_in_us;
	add.QuadPart /= 1000000;

	in_timestamp->QuadPart += add.QuadPart;
}