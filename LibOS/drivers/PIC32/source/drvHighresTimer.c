/*****************************************************************************/
/* High Resolution System timer (1us) routines                               */
/*                                                                           */
/* Copyright (C) 2014-2015 Laszlo Arvai                                      */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
// Includes
#include <sysHighResTimer.h>
#include "sysConfig.h"
#include <plib.h>

#define drvHIGHRES_TIMER_INDEX 2

#define drvHIGHRES_TIMER_TICK_RATE       ((sysCLOCK_FREQUENCY / 2) / (1000000))

// Open timer macro
#define __OpenHighresTimer(timer_index)  OpenTimer ## timer_index
#define _OpenHighresTimer(timer_index)  __OpenHighresTimer(timer_index)
#define OpenHighresTimer _OpenHighresTimer(drvHIGHRES_TIMER_INDEX)

// Interrupt vector macro
#define __HIGHRESTIMER_VECTOR(timer_index)       _TIMER_ ## timer_index ## _VECTOR
#define _HIGHRESTIMER_VECTOR(timer_index) __HIGHRESTIMER_VECTOR(timer_index)
#define HIGHRESTIMER_VECTOR _HIGHRESTIMER_VECTOR(drvHIGHRES_TIMER_INDEX)

// Clear INT flag macro
#define __ClearIntFlag(timer_index)  mT ## timer_index ## ClearIntFlag
#define _ClearIntFlag(timer_index)  __ClearIntFlag(timer_index)
#define ClearIntFlag _ClearIntFlag(drvHIGHRES_TIMER_INDEX)

// Read timer
#define __ReadTimer(timer_index)  ReadTimer ## timer_index
#define _ReadTimer(timer_index)  __ReadTimer(timer_index)
#define ReadTimer _ReadTimer(drvHIGHRES_TIMER_INDEX)

// Config int timer
#define __ConfigIntTimer(timer_index)  ConfigIntTimer ## timer_index
#define _ConfigIntTimer(timer_index)  __ConfigIntTimer(timer_index)
#define ConfigIntTimer _ConfigIntTimer(drvHIGHRES_TIMER_INDEX)

///////////////////////////////////////////////////////////////////////////////
// Module local variables
static volatile uint16_t l_highres_timer_upper_word = 0;

///////////////////////////////////////////////////////////////////////////////
/// @brief Initialize high resolution timer
void drvHighresTimerInit(void)
{
	// initialize timer to 1Mhz (1us)
	OpenHighresTimer(T1_ON | T1_SOURCE_INT | T1_PS_1_1, drvHIGHRES_TIMER_TICK_RATE);

	ConfigIntTimer(T1_INT_ON | T1_INT_PRIOR_3);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Gets high resolution time timestamp
sysHighresTimestamp sysHighresTimerGetTimestamp(void)
{
	uint32_t timestamp;
	uint16_t upper_word;

	do
	{
		upper_word = l_highres_timer_upper_word;
		timestamp = ReadTimer() & 0xffff;
	} while(upper_word != l_highres_timer_upper_word);

  return (((uint32_t)upper_word)<<16) | timestamp;
}

///////////////////////////////////////////////////////////////////////////////
// Interrupt handler
void __ISR(HIGHRESTIMER_VECTOR, ipl3) HighresTimerHandler(void)
{
	// increment uppr word
	l_highres_timer_upper_word++;

	// clear interrupt flag
  ClearIntFlag();
}