/*****************************************************************************/
/* High Resolution System timer (1us) Linux hal driver                       */
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
#include <stdio.h>
#include <sys/time.h>
#include <sysHighresTimer.h>

/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Initializes high resolution timer
void halHighresTimerInit(void)
{
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Gets timestamp of the high resolution timer
/// @return timestamp value
sysHighresTimestamp sysHighresTimerGetTimestamp(void)
{
	sysHighresTimestamp timestamp;
	struct timeval tv;
	
	gettimeofday(&tv, NULL);
	
	timestamp = ((sysHighresTimestamp)tv.tv_sec) * 1000000 + tv.tv_usec;

	return timestamp;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Gets ellapsed time in us since the timestamp
/// @param in_timstamp Timestamp of the start time
/// @return Time difference between current time and start time in us
uint32_t sysHighresTimerGetTimeSince(sysHighresTimestamp in_timestamp)
{
	sysHighresTimestamp diff_time;

	diff_time = sysHighresTimerGetTimestamp() - in_timestamp;
	
	return (uint32_t)diff_time;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Adds value to the high resolution timer timestamp
/// @param in_timestamp Timestamp value to increase
/// @param in_value_in_us
void sysHighresTimerAddToTimestamp(sysHighresTimestamp* in_timestamp, uint32_t in_value_in_us)
{
	*in_timestamp += in_value_in_us;
}