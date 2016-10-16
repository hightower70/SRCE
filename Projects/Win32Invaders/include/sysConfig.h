/*****************************************************************************/
/* Configuration for Win32 application                                       */
/*                                                                           */
/* Copyright (C) 2015 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/
#ifndef __sysConfig_h
#define __sysConfig_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysTypes.h>

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/
///////////////////////////////////////////////////////////////////////////////
// Diagnostics
#define emuDIAG_DISPLAY_STATISTICS 1

///////////////////////////////////////////////////////////////////////////////
// GUI Config
#define guiSCREEN_WIDTH 240
#define guiSCREEN_HEIGHT 320

#define guiCOLOR_DEPTH 24

#define guiemuZOOM 1
#define guiemuBACKGROUND_COLOR 0x00000000
#define guiemuFOREGROUND_COLOR 0xffffffff

///////////////////////////////////////////////////////////////////////////////
// Wave config
#define halWAVEPLAYER_SAMPLE_RATE 44100
#define halWAVEPLAYER_SAMPLE_OFFSET 0
#define halWAVEPLAYER_SAMPLE_MULTIPLIER 64

///////////////////////////////////////////////////////////////////////////////
// Resource config
typedef int sysResourceAddress;

#endif