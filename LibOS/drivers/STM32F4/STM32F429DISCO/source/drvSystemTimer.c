/*****************************************************************************/
/* System timer (1ms) driver                                                 */
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
/* Module global variables                                                   */
/*****************************************************************************/
static volatile sysTimeStamp l_timestamp = 0;

///////////////////////////////////////////////////////////////////////////////
/// @brief Initialize system timer
void drvSystemTimerInitialize(void)
{
	// Initialization is done in HAL
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Gets system timer current value (current timestamp)
/// @return System timer value
sysTimeStamp sysTimerGetTimestamp(void)
{
  return l_timestamp;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief System timer increment function
void drvSystemTimerCallback(void)
{
	l_timestamp++;
}
