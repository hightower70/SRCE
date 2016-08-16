/*****************************************************************************/
/* File browser application                                                  */
/*                                                                           */
/* Copyright (C) 2015 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

#ifndef __fbFileBrowser_h
#define __fbFileBrowser_h

///////////////////////////////////////////////////////////////////////////////
// Includes
#include <sysTypes.h>
#include <sysString.h>
#include <fbTypes.h>
#include <sysUserInput.h>

///////////////////////////////////////////////////////////////////////////////
// Constants
#define fbSELECTION_FIRST -32767
#define fbSELECTION_PAGE_UP -32766
#define fbSELECTION_LAST 32767
#define fbSELECTION_PAGE_DOWN 32766


///////////////////////////////////////////////////////////////////////////////
// Types
typedef enum 
{
	// inactive or busy states
	fbs_Inactive,
	fbs_ParseStart,
	fbs_Parsing,
	fbs_Sorting,

	// active states
	fbs_Active,
	fbs_HorizontalScrollLeftDelay,
	fbs_HorizontalScrollLeft,
	fbs_HorizontalScrollRightDelay,
	fbs_HorizontalScrollRight,

	fbs_Error
} fbStatus;

///////////////////////////////////////////////////////////////////////////////
// Function prototypes
void fbTask(void);
void fbActivate(sysString in_path);
void fbDeactivate(void);

void fbSelectionMove(int16_t in_offset);
bool fbSelectionEnter(void);

fbStatus fbGetStatus(void);

void fbFileSelectionHandler(void);

fbFileInformation* fbGetSelectedFileInformation(void);
void fbGetSelectedFullFileName(sysString in_filename_buffer, sysStringLength in_buffer_length);

void fbUserInputEventHandler(uint8_t in_device_number, sysUserInputEventCategory in_event_category, sysUserInputEventType in_event_type, uint32_t in_event_param);


void fbWaitIndicatorShow(void);
void fbWaitIndicatorUpdate(void);
void fbWaitIndicatorHide(void);

#endif