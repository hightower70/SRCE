/*****************************************************************************/
/* System initialization function for win32                                  */
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
#include <sysUserInput.h>
#include <guiColorGraphics.h>
#include <drvWavePlayer.h>
#include <sysHighresTimer.h>
#include "sysConfig.h"


/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief System initialization function
void sysInitialization(void)
{
	sysHighresTimerInit();
	guiColorGraphicsInitialize();
	emuInvadersInitialize();
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Cleans up system
void sysCleanup(void)
{
	drvWavePlayerCleanUp();
}