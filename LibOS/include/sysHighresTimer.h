/*****************************************************************************/
/* High Resolution System timer (1us) routines                               */
/*                                                                           */
/* Copyright (C) 2014-2015 Laszlo Arvai                                      */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

#ifndef __sysHighresTimer_h
#define __sysHighresTimer_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysTypes.h>
#ifdef _WIN32

#include <Windows.h>
typedef LARGE_INTEGER sysHighresTimestamp;

#else

typedef uint32_t sysHighresTimestamp;
#endif

/*****************************************************************************/
/* Function prototypes                                                       */
/*****************************************************************************/
void sysHighresTimerInitialize(void);
void drvHighresTimerInitialize(void);
sysHighresTimestamp sysHighresTimerGetTimestamp(void);
uint32_t sysHighresTimerGetTimeSince(sysHighresTimestamp in_timestamp_in_us);
void sysHighresTimerAddToTimestamp(sysHighresTimestamp* in_timestamp, uint32_t in_value_in_us);

#endif
