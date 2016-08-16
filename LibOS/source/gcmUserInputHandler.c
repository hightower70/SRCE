/*****************************************************************************/
/* Simple Graphical Configuration Menu Framework                             */
/*                                                                           */
/* Copyright (C) 2015 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
// Include
#include <sysUserInput.h>
#include <sysVirtualKeyboardCodes.h>
#include <gcmGraphicsConfigMenu.h>

///////////////////////////////////////////////////////////////////////////////
/// @brief User input handler function for Graphical Configuration Menu
void gcmUserInputEventHandler(uint8_t in_device_number, sysUserInputEventCategory in_event_category, sysUserInputEventType in_event_type, uint32_t in_event_param)
{
	// if busy or inactive do nothing
	if (!gcmIsActive())
		return;

	if (in_device_number == 1 && in_event_category == sysUIEC_Pressed)
	{
		switch (in_event_type)
		{
			case sysUIET_JoyYAxis:
				if (in_event_param < 0x7f)
				{
					gcmSelectionMove(-1);
				}
				else
				{
					gcmSelectionMove(1);
				}
				break;

			case sysUIET_JoyXAxis:
				if (in_event_param < 0x7f)
				{
					gcmCurrentValueChange(-1);
				}
				else
				{
					gcmCurrentValueChange(1);
				}
				break;

			case sysUIET_Key:
				if (sysVKC_IS_SPECIAL_KEY(in_event_param))
				{
					switch (in_event_param & (~sysVKC_SPECIAL_KEY_FLAG))
					{
						case sysVKC_DOWN:
							gcmSelectionMove(1);
							break;

						case sysVKC_UP:
							gcmSelectionMove(-1);
							break;

						case sysVKC_LEFT:
							gcmCurrentValueChange(-1);
							break;

						case sysVKC_RIGHT:
							gcmCurrentValueChange(1);
							break;

						case sysVKC_PRIOR:
							gcmSelectionMove(gcmSELECTION_PAGE_UP);
							break;

						case sysVKC_NEXT:
							gcmSelectionMove(gcmSELECTION_PAGE_DOWN);
							break;

						case sysVKC_END:
							gcmSelectionMove(gcmSELECTION_LAST);
							break;

						case sysVKC_HOME:
							gcmSelectionMove(gcmSELECTION_FIRST);
							break;
					}
				}
				else
				{
					switch (in_event_param)
					{
						case sysVKC_RETURN:
							break;
					}
				}
				break;
		}
	}
}
