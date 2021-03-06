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
#include <drvJoystick.h>
#include <sysHighresTimer.h>
#include <emuInvaders.h>
#include <usb_host.h>


/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/
static sysTimeStamp l_led_timestamp = 0;
static uint8_t l_led_value = 0;

#if	sysMEASURE_CPU_LOAD

static sysHighresTimestamp l_cycle_start_timestamp;
#endif

/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Main task
void sysMainTask(void)
{
#if	sysMEASURE_CPU_LOAD
	//l_cycle_start_timestamp = sysGe
#endif


	emuInvadersTask();

	drvJoystickUpdate();

	MX_USB_HOST_Process();

	// Stat LED
	if(sysTimerGetTimeSince(l_led_timestamp) > 500)
	{
		l_led_timestamp = sysTimerGetTimestamp();

		l_led_value = 1 - l_led_value;

		drvStatLEDSetDim(l_led_value);
	}
}


