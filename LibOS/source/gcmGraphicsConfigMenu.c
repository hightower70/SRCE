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
// Includes
#include <gcmGraphicsConfigMenu.h>
#include <gcmRenderer.h>

///////////////////////////////////////////////////////////////////////////////
// Module global variables
static bool l_active = false;
static gcmMenuInfo l_active_menu;
static uint16_t l_max_value_width;
static uint8_t l_menu_item_count;

///////////////////////////////////////////////////////////////////////////////
// Module local functions
static void gcmRenderScreen(void);
static void gcmUpdateSelectedItem(uint8_t in_new_selected_item_index);
static void gcmProcessSelectionDown(uint8_t in_items_count);
static void gcmProcessSelectionUp(uint8_t in_items_count);

///////////////////////////////////////////////////////////////////////////////
/// @brief Initializes menu information structure (fills with zero)
void gcmMenuInfoStructInit(gcmMenuInfo* out_menu_info)
{
	out_menu_info->Title = sysNULL;
	out_menu_info->MenuItems = sysNULL;
	out_menu_info->ValueTitles = sysNULL;
	out_menu_info->Values = sysNULL;
	out_menu_info->FirstItem = 0;
	out_menu_info->SelectedItem = 0;
	out_menu_info->Rect.Left = 0;
	out_menu_info->Rect.Top = 0;
	out_menu_info->Rect.Right = 0;
	out_menu_info->Rect.Bottom = 0;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Activates menu
/// @param in_menu_info Menu related data structure
void gcmActivate(gcmMenuInfo* in_menu_info)
{
	// parameter check
	if (in_menu_info == sysNULL || in_menu_info->MenuItems == sysNULL || in_menu_info->ValueTitles == sysNULL || in_menu_info->Values == sysNULL)
		return;

	// set active menu
	l_active_menu = *in_menu_info;

	// initialize renderer
	gcmRendererInitialize(in_menu_info);

	// calculate menu item count
	l_menu_item_count = 0;
	while (in_menu_info->MenuItems[l_menu_item_count].MenuTitle != sysNULL)
	{
		l_menu_item_count++;
	}

	l_active = true;

	// render menu screen
	gcmRenderScreen();
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Deactivates menu
void gcmDeactivate(void)
{
	l_active = false;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Gets active flag
/// @return True if menu is active
bool gcmIsActive(void)
{
	return l_active;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Changes current selected menu item's value by the given offset
/// @param in_offset Offset to add to the current value
void gcmCurrentValueChange(int16_t in_offset)
{
	uint32_t old_value;
	uint32_t new_value;

	if (gcmGetItemValueAsUInt(l_active_menu.SelectedItem, &old_value))
	{
		new_value = old_value + in_offset;

		if (in_offset < 0 && old_value < (uint32_t)(-in_offset))
			new_value = 0;

		if (new_value >= l_active_menu.MenuItems[l_active_menu.SelectedItem].ValueTitleCount)
			new_value = l_active_menu.MenuItems[l_active_menu.SelectedItem].ValueTitleCount - 1;

		gcmSetItemValueAsUInt(l_active_menu.SelectedItem, new_value);
	}

	gcmRenderItem(&l_active_menu, l_active_menu.SelectedItem, l_active_menu.SelectedItem - l_active_menu.FirstItem, true);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Moves menu item selection with the given offset
/// @param in_offset Offset to move the menu item selection
void gcmSelectionMove(int16_t in_offset)
{
	switch (in_offset)
	{
		case gcmSELECTION_FIRST:
			gcmProcessSelectionUp(l_active_menu.SelectedItem);
			break;

		case gcmSELECTION_PAGE_UP:
			gcmProcessSelectionUp(gcmRendererGetDisplayableItemCount() - 1);
			break;

		case gcmSELECTION_LAST:
			gcmProcessSelectionDown(l_menu_item_count - gcmRendererGetDisplayableItemCount() - 1);
			break;

		case gcmSELECTION_PAGE_DOWN:
			gcmProcessSelectionDown(gcmRendererGetDisplayableItemCount() - 1);
			break;

		default:
			if (in_offset < 0)
				gcmProcessSelectionUp((uint8_t)(-in_offset));
			else
				gcmProcessSelectionDown((uint8_t)in_offset);
			break;
	}
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Moves selection up by the specified number of items
/// @param in_items_count Number of items to move selection upwards
static void gcmProcessSelectionUp(uint8_t in_items_count)
{
	uint8_t new_selected_file_index;

	if (l_active_menu.SelectedItem == 0)
		return;

	if (l_active_menu.SelectedItem - in_items_count < 0)
		new_selected_file_index = 0;
	else
		new_selected_file_index = l_active_menu.SelectedItem - in_items_count;

	gcmUpdateSelectedItem(new_selected_file_index);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Moves selection down by the specified number of items
/// @param in_items_count Number of items to move selection downwards
static void gcmProcessSelectionDown(uint8_t in_items_count)
{
	uint8_t new_selected_file_index;

	if (l_active_menu.SelectedItem == l_menu_item_count - 1)
		return;

	if (l_active_menu.SelectedItem + in_items_count >= l_menu_item_count)
		new_selected_file_index = l_active_menu.SelectedItem - 1;
	else
		new_selected_file_index = l_active_menu.SelectedItem + in_items_count;

	gcmUpdateSelectedItem(new_selected_file_index);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Updates selected item (removes selection fromthe old item and displays selection on the new item, and updates footer information) 
static void gcmUpdateSelectedItem(uint8_t in_new_selected_item_index)
{
	if (in_new_selected_item_index < l_active_menu.FirstItem)
	{
		// render whole screen
		l_active_menu.FirstItem = in_new_selected_item_index;
		l_active_menu.SelectedItem = in_new_selected_item_index;
		gcmRenderScreen();
	}
	else
	{
		if (in_new_selected_item_index >= l_active_menu.FirstItem + l_menu_item_count)
		{
			// render whole screen
			l_active_menu.FirstItem = in_new_selected_item_index - gcmRendererGetDisplayableItemCount() + 1;
			l_active_menu.SelectedItem = in_new_selected_item_index;
			gcmRenderScreen();
		}
		else
		{
			// render only affected items
			gcmRenderItem(&l_active_menu, l_active_menu.SelectedItem, l_active_menu.SelectedItem - l_active_menu.FirstItem, false);
			l_active_menu.SelectedItem = in_new_selected_item_index;
			gcmRenderItem(&l_active_menu, l_active_menu.SelectedItem, l_active_menu.SelectedItem - l_active_menu.FirstItem, true);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Gets menu item value as unsigned integer
/// @param in_item_index Index of the menu item which value needs to be read
/// @param out_value Pointer to storage to receive the value
/// @return True if operation was successfull, false if value can't be converted to uint or doesn't exists
bool gcmGetItemValueAsUInt(uint8_t in_item_index, uint32_t* out_value)
{
	void* item_value_pointer = l_active_menu.Values[in_item_index];

	switch (l_active_menu.MenuItems[in_item_index].Type)
	{
		case gcmMVT_Bool:
			*out_value = (*(bool*)item_value_pointer) ? 1 : 0;
			return true;

		case gcmMVT_Byte:
			*out_value = *((uint8_t*)item_value_pointer);
			return true;
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Sets menu item value as unsigned integer
/// @param in_item_index Index of the menu item which value needs to be write
/// @param in_value New value
/// @return True if operation was successfull, false if value can't be converted to uint or doesn't exists
bool gcmSetItemValueAsUInt(uint8_t in_item_index, uint32_t in_value)
{
	void* item_value_pointer = l_active_menu.Values[in_item_index];

	switch (l_active_menu.MenuItems[in_item_index].Type)
	{
		case gcmMVT_Bool:
			*((bool*)item_value_pointer) = (in_value > 0) ? true : false;
			return true;

		case gcmMVT_Byte:
			*((uint8_t*)item_value_pointer) = (uint8_t)in_value;
			return true;
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Renders all visible item of a menu
static void gcmRenderScreen(void)
{
	uint8_t item_index = l_active_menu.FirstItem;
	uint8_t screen_index = 0;
	uint8_t displayed_item_count = gcmRendererGetDisplayableItemCount();

	while (screen_index < displayed_item_count)
	{
		if (item_index < l_menu_item_count)
		{
			gcmRenderItem(&l_active_menu, item_index, screen_index, (item_index == l_active_menu.SelectedItem));
		}
		else
		{
			gcmRenderItem(&l_active_menu, gcmINVALID_ITEM_INDEX, screen_index, false);
		}

		screen_index++;
		item_index++;
	}
}
