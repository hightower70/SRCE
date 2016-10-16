/*****************************************************************************/
/* Windows main function using graphics window to emulate display            */
/*                                                                           */
/* Copyright (C) 2015 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysMain.h>

/*****************************************************************************/
/* Windows main function using graphics window to emulate display            */
/*                                                                           */
/* Copyright (C) 2015 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysMain.h>
#include <halKeyboardInput.h>
#include <sysUserInput.h>
#include <emuInvaders.h>
#include <sysVirtualKeyboardCodes.h>

/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/

static bool l_exit_requested = false;


///////////////////////////////////////////////////////////////////////////////
/// @brief Main entrance function for Raspberry PI Raspbian
int main(int argc, char* argv[])
{
	
	sysInitialization();

	while (!l_exit_requested)
	{
		halKeyboardDispatchEvent();
		sysMainTask();
	}
	
	drvColorGraphicsCleanup();
	sysCleanup();

}

void sysUserInputEventHandler(uint8_t in_device_number, sysUserInputEventCategory in_event_category, sysUserInputEventType in_event_type, uint32_t in_event_param)
{
	if (in_event_category == sysUIEC_Pressed && in_event_type == sysUIET_Key && in_event_param == sysVKC_ESCAPE)
		l_exit_requested = true;
	else
		emuUserInputEventHandler(in_device_number, in_event_category, in_event_type, in_event_param);
}
