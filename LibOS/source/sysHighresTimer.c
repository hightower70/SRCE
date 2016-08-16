/*****************************************************************************/
/* High resolution timer (1us) routines                                      */
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
#include <sysHighresTimer.h>

/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Initialize high resolution timer
void sysHighresTimerInitialize(void)
{
	drvHighresTimerInitialize();
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Busy wait for the given time (in us)
void sysHighresTimerDelay(uint32_t in_delay_ms)
{
	uint32_t start_time = sysHighresTimerGetTimestamp();
	uint32_t diff_time;
	
	do
	{
		diff_time = sysHighresTimerGetTimestamp() - start_time;
	}	while( diff_time < in_delay_ms );
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Gets elapsed time since a timestamp (in us)
/// @param in_start_time_ms Time from the elapsed time is calculated
/// @return Elapsed time since the given timestamp
sysHighresTimestamp sysHighresTimerGetTimeSince(sysHighresTimestamp in_start_time_us)
{
	sysHighresTimestamp diff_time;

	diff_time = sysHighresTimerGetTimestamp() - in_start_time_us;
	
	return diff_time;
}

