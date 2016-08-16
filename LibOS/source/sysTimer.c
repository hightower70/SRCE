/*****************************************************************************/
/* System timer (1ms) routines                                               */
/*                                                                           */
/* Copyright (C) 2014-2015 Laszlo Arvai                                      */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysTimer.h>

/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Initializes system timer
void sysTimerInitialize(void)
{
	drvSystemTimerInitialize();
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Busy wait for the given time (in ms)
void sysTimerDelay(uint32_t in_delay_ms)
{
	uint32_t start_time = sysTimerGetTimestamp();
	uint32_t diff_time;
	
	do
	{
		diff_time = sysTimerGetTimestamp() - start_time;
	}	while( diff_time < in_delay_ms );
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Gets elapsed time since a timestamp (in ms)
/// @param in_start_time_ms Time from the elapsed time is calculated
/// @return Elapsed time since the given timestamp
sysTimeStamp sysTimerGetTimeSince(sysTimeStamp in_start_time_ms)
{
	sysTimeStamp diff_time;

	diff_time = sysTimerGetTimestamp() - in_start_time_ms;
	
	return diff_time;
}


