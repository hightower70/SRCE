/*****************************************************************************/
/* Typedefs used for file system handling                                    */
/*                                                                           */
/* Copyright (C) 2014-2015 Laszlo Arvai                                      */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

#ifndef __fileTypes_h
#define	__fileTypes_h

///////////////////////////////////////////////////////////////////////////////
// Inckudes
#include <sysTypes.h>

///////////////////////////////////////////////////////////////////////////////
// Constants
#define fileMAX_PATH  256
#define filePATH_SEPARATOR '\\'
#define fileDRIVE_LETTER_SEPARATOR ':'

///////////////////////////////////////////////////////////////////////////////
// Types
typedef enum
{
	fileMS_UNKNOWN,						// media status is unknown or not initialized
	fileMS_OK,								// No errors
  fileMS_NOT_PRESENT,				// Media is not present
  fileMS_ERROR							// Cannot initialize media
} fileMediaStatusType;

typedef struct
{
	fileMediaStatusType Status;
	uint32_t LastLBA;
} fileMediaInfo;

#endif

