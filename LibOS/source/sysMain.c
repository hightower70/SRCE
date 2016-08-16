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

/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/


///////////////////////////////////////////////////////////////////////////////
/// @brief Main entry function
int main(void)
{
	// initialize system
	sysInitialize();

	while(true)
	{
		sysMainTask();
	}

  return 0;
}
