/*****************************************************************************/
/* CPU Load monitor                                                          */
/*                                                                           */
/* Copyright (C) 2016 Laszlo Arvai                                           */
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
/* Module local variables                                                    */
/*****************************************************************************/
static sysHighresTimestamp l_period_start;
static sysHighresTimestamp l_load_start;
static uint32_t l_busy_time;

/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Enters monitored section
void sysCPULoadEnterSection(void)
{
	l_load_start = sysHighresTimerGetTimestamp();
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Exists monitored section
/// @param in_busy True if the monitored section was active busy or false if it was idle
void sysCPULoadExitSection(bool in_busy)
{
	if (in_busy)
	{
		l_busy_time += sysHighresTimerGetTimeSince(l_load_start);
	}
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Gets CPU load in percentage (this function must be called at least in every second in order to provide reliable result)
/// @return CPU load in percentage
uint8_t sysCPULoadGetLoad(void)
{
	uint8_t load;
	uint32_t period_length;

	period_length = sysHighresTimerGetTimeSince(l_period_start);

	if (period_length == 0)
		load = 0;
	else
		load = (uint8_t)(l_busy_time * 100 / period_length);

	// restart period
	l_period_start = sysHighresTimerGetTimestamp();
	l_busy_time = 0;

	return load;
}
