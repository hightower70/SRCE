/*****************************************************************************/
/* Main Entry Function                                                       */
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
#include <sysMain.h>
#include <drvStatLED.h>
#include <sysTimer.h>
#include <emuInvaders.h>

/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/
static sysTimeStamp l_led_timestamp = 0;
static uint8_t l_led_value = 0;

/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Main task
void sysMainTask(void)
{
	emuInvadersTask();

	// Stat LED
	if(sysTimerGetTimeSince(l_led_timestamp) > 500)
	{
		l_led_timestamp = sysTimerGetTimestamp();

		l_led_value = 1 - l_led_value;

		drvStatLEDSetDim(l_led_value);
	}
}


