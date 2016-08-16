/*****************************************************************************/
/* Simple Graphical Configuration Menu Framework                             */
/*                                                                           */
/* Copyright (C) 2015 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

#ifndef __gcmGraphicsConfigMenu_h
#define __gcmGraphicsConfigMenu_h

///////////////////////////////////////////////////////////////////////////////
// Includes
#include <sysTypes.h>
#include <guiTypes.h>
#include <sysUserInput.h>
#include "sysConfig.h"

///////////////////////////////////////////////////////////////////////////////
// Constants
#define gcmINVALID_ITEM_INDEX 0xff

///////////////////////////////////////////////////////////////////////////////
// Constants
#define gcmSELECTION_FIRST -32767
#define gcmSELECTION_PAGE_UP -32766
#define gcmSELECTION_LAST 32767
#define gcmSELECTION_PAGE_DOWN 32766

///////////////////////////////////////////////////////////////////////////////
// Types

typedef enum
{
	gcmMVT_Byte,
	gcmMVT_Bool,
	gcmMVT_Callback
} gcmMenuValueType;

typedef struct
{
	sysResourceAddress MenuTitle;
	gcmMenuValueType Type;
	uint8_t ValueTitleIndex;
	uint8_t ValueTitleCount;
} gcmMenuItem;


typedef struct
{
	sysResourceAddress Title;
	gcmMenuItem* MenuItems;
	sysResourceAddress* ValueTitles;
	void** Values;
	uint8_t FirstItem;
	uint8_t SelectedItem;
	guiRect Rect;
} gcmMenuInfo;


///////////////////////////////////////////////////////////////////////////////
// Function prototypes
void gcmMenuInfoStructInit(gcmMenuInfo* out_menu_info);
void gcmActivate(gcmMenuInfo* in_menu_info);
void gcmDeactivate(void);
bool gcmGetItemValueAsUInt(uint8_t in_item_index, uint32_t* out_value);
bool gcmSetItemValueAsUInt(uint8_t in_item_index, uint32_t in_value);
bool gcmIsActive(void);
void gcmSelectionMove(int16_t in_offset);
void gcmCurrentValueChange(int16_t in_offset);
void gcmUserInputEventHandler(uint8_t in_device_number, sysUserInputEventCategory in_event_category, sysUserInputEventType in_event_type, uint32_t in_event_param);

#endif
