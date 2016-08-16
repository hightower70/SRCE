/*****************************************************************************/
/* User inpout (keyboard, joystick) general handler                          */
/*                                                                           */
/* Copyright (C) 2015 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/
#ifndef __sysUserInput_h
#define __sysUserInput_h

///////////////////////////////////////////////////////////////////////////////
// Includes
#include <sysTypes.h>

///////////////////////////////////////////////////////////////////////////////
// Types

/// User Input Event Category
typedef enum
{
		sysUIEC_Pressed,
		sysUIEC_Repeated,
		sysUIEC_Released
} sysUserInputEventCategory;

/// User Input Event Type
typedef  enum
{
	sysUIET_Key,

	sysUIET_JoyXAxis,
	sysUIET_JoyYAxis,
	sysUIET_JoyButton
} sysUserInputEventType;

// Keyboard Modifiers state
#define sysMS_ALT				(1<<0)
#define sysMS_CTRL			(1<<1)
#define sysMS_SHIFT			(1<<2)
#define sysMS_NUM_LOCK	(1<<3)
#define sysMS_CAPS_LOCK	(1<<4)

///////////////////////////////////////////////////////////////////////////////
// Function prototypes
void sysUserInputEventHandler(uint8_t in_device_number, sysUserInputEventCategory in_event_category, sysUserInputEventType in_event_type, uint32_t in_event_param);
uint32_t sysUserInputGetModifiersState(uint8_t in_device_number);

#endif