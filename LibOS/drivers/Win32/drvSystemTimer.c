/*****************************************************************************/
/* System timer (1ms) driver                                                 */
/* (emulated on Win32)                                                       */
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
#include <windows.h>

/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Initializes system timer
void drvSystemTimerInitialize(void)
{
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Gets system timer current value (current timestamp)
/// @return System timer value
sysTimeStamp sysTimerGetTimestamp(void)
{
  return (sysTimeStamp)GetTickCount();
}
