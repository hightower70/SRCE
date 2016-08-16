/*****************************************************************************/
/* International (Charecter Code Pages) handling                             */
/*                                                                           */
/* Copyright (C) 2015 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/
#ifndef __cpCodePages_h
#define __cpCodePages_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysTypes.h>
#include <sysUserInput.h>

/*****************************************************************************/
/* Types                                                                     */
/*****************************************************************************/
typedef struct
{
	bool Shift;
	bool AltGr;

} cpKeyboardState;


/*****************************************************************************/
/* Functions                                                                 */
/*****************************************************************************/

// Win1250 functions
sysChar cpConvertWin1250(uint8_t in_device_number, sysUserInputEventCategory in_event_category, sysUserInputEventType in_event_type, uint32_t in_event_param);
sysChar cpToUpperWin1250(sysChar in_char);


#endif