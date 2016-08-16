/*****************************************************************************/
/* System timer (1ms) routines for PIC32 microcontroller                     */
/*                                                                           */
/* Copyright (C) 2014-2015 Laszlo Arvai                                      */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
// Includes
#include <plib.h>
#include <sysTypes.h>
#include <sysTimer.h>
#include "sysConfig.h"

// Calculate clock rate for the 1ms (1000 interrupts/second)
#define CORE_TICK_RATE       ((sysCLOCK_FREQUENCY / 2) / (sysTIMER_TICKS_PER_SECOND))

///////////////////////////////////////////////////////////////////////////////
// Module local variables
static volatile sysTimeStamp l_system_time = 0;

///////////////////////////////////////////////////////////////////////////////
// Initialize system timer
void drvSystemTimerInit(void)
{
  // Open Core Timer
  OpenCoreTimer(CORE_TICK_RATE);
  _CP0_BIC_DEBUG(_CP0_DEBUG_COUNTDM_MASK);

  // set up the core timer interrupt with a prioirty of 2 and zero sub-priority
  mConfigIntCoreTimer((CT_INT_ON | CT_INT_PRIOR_2 | CT_INT_SUB_PRIOR_0));
}

///////////////////////////////////////////////////////////////////////////////
// Interrupt handler
void __ISR(_CORE_TIMER_VECTOR, ipl2) CoreTimerHandler(void)
{
  // clear the interrupt flag
  mCTClearIntFlag();

  // update the period
  UpdateCoreTimer(CORE_TICK_RATE);

  // increment TickCount
  l_system_time++;
}

///////////////////////////////////////////////////////////////////////////////
// Get system timer
sysTimeStamp sysTimerGetTimestamp(void)
{
  return l_system_time;
}
