/*****************************************************************************/
/* Color graphics driver                                                     */
/*                                                                           */
/* Copyright (C) 2014-2015 Laszlo Arvai                                      */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

#ifndef __drvColorGraphics_h
#define __drvColorGraphics_h

///////////////////////////////////////////////////////////////////////////////
// Includes
#include <guiTypes.h>

///////////////////////////////////////////////////////////////////////////////
// Function prototypes
void drvColorGraphicsInitialize(void);
void drvColorGraphicsCleanup(void);
void drvColorGraphicsRefreshScreen(void);

#endif
