/*****************************************************************************/
/* Simple Graphical Configuration Menu Renderer Functions                    */
/*                                                                           */
/* Copyright (C) 2015 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
// Includes
#include <guiBlackAndWhiteGraphics.h>
#include <gcmRenderer.h>
#include "sysResource.h"

#if !defined(gcmITEM_FONT) || !defined(gcmITEM_GAP)
#error "Graphics Config Menu is not configured."
#endif

#if !defined(gcmTITLE_FONT)
#define gcmTITLE_FONT gcmITEM_FONT
#endif

#define gcmTITLE_BAR_VERTICAL_GAP 2
#define gcmTITLE_TEXT_VERTICAL_GAP 1
#define gcmTITLE_TEXT_HORIZONTAL_GAP 2
#define gcmTITLE_BAR_HORIZONTAL_OFFSET 2

static uint8_t l_displayed_item_count;
static uint8_t l_item_height;
static uint16_t l_max_value_width;

void gcmRendererInitialize(gcmMenuInfo* in_menu_info)
{
	guiSize size;
	uint8_t item_index;
	uint8_t value_index;
	uint8_t value_count;
	uint16_t value_width;

	guiSetFont(gcmITEM_FONT);

	// determine item dimensions
	size = guiGetTextExtent("Hj");

	// item height
	l_item_height = size.Height + gcmITEM_GAP;
	l_displayed_item_count = (in_menu_info->Rect.Bottom - in_menu_info->Rect.Top) / l_item_height;

	// max value width
	item_index = 0;
	value_width = 0;
	while (in_menu_info->MenuItems[item_index].MenuTitle != sysNULL)
	{
		value_index = 0;

		value_index = in_menu_info->MenuItems[item_index].ValueTitleIndex;
		value_count = in_menu_info->MenuItems[item_index].ValueTitleCount;

		while (value_count > 0)
		{
			size = guiGetResourceTextExtent(in_menu_info->ValueTitles[value_index]);

			if (size.Width > value_width)
				value_width = size.Width;

			value_count--;
			value_index++;
		}

		item_index++;
	}
	l_max_value_width = value_width;

}

uint8_t gcmRendererGetDisplayableItemCount(void)
{
	return l_displayed_item_count;
}

void gcmRenderItem(gcmMenuInfo* in_menu_info, uint8_t in_item_index, uint8_t in_screen_index, bool in_selected)
{
	int16_t top = in_menu_info->Rect.Top + in_screen_index * l_item_height;
	uint32_t item_value;
	guiSize item_label_size;
	sysResourceAddress item_label;
	guiSize left_bitmap_size;
	guiSize right_bitmap_size;
	guiCoordinate x;

	if (in_item_index != gcmINVALID_ITEM_INDEX)
	{
		// open canvas
		guiOpenCanvas(0, top, guiSCREEN_WIDTH - 1, top + l_item_height);

		// clear background
		if (in_selected)
		{
			guiSetPen(1);
			guiSetDrawMode(guiDM_Inverse);
			guiSetSystemBrush(2);
		}
		else
		{
			guiSetPen(0);
			guiSetDrawMode(0);
			guiSetSystemBrush(0);
		}

		guiFillRectangle(in_menu_info->Rect.Left, top, in_menu_info->Rect.Right, top + l_item_height - 1);

		// display item text
		guiSetFont(gcmITEM_FONT);
		guiSetTextAlign(guiHA_LEFT | guiVA_TOP);
		guiDrawResourceText(in_menu_info->Rect.Left + 2, top + gcmITEM_GAP / 2, in_menu_info->MenuItems[in_item_index].MenuTitle);

		// display item value
		if (gcmGetItemValueAsUInt(in_item_index, &item_value))
		{
			if (item_value < in_menu_info->MenuItems[in_item_index].ValueTitleCount)
			{
				item_label = in_menu_info->ValueTitles[item_value + in_menu_info->MenuItems[in_item_index].ValueTitleIndex];

				item_label_size = guiGetResourceTextExtent(item_label);
				left_bitmap_size = guiGetBitmapSize(REF_BMP_ARROW_LEFT);
				right_bitmap_size = guiGetBitmapSize(REF_BMP_ARROW_RIGHT);

				x = in_menu_info->Rect.Right - 2 - left_bitmap_size.Width - l_max_value_width / 2;

				guiDrawResourceText(x - item_label_size.Width / 2, top + gcmITEM_GAP / 2, item_label);
				if (in_selected)
				{
					guiDrawBitmap(x - item_label_size.Width / 2 - right_bitmap_size.Width, top + gcmITEM_GAP / 2, REF_BMP_ARROW_RIGHT);
					guiDrawBitmap(x + item_label_size.Width / 2, top + gcmITEM_GAP / 2, REF_BMP_ARROW_LEFT);
				}
			}
		}

		guiSetDrawMode(0);

		// close canvas
		guiCloseCanvas(true);
	}
	else
	{
		// open canvas
		guiOpenCanvas(0, top, guiSCREEN_WIDTH - 1, top + l_item_height);

		guiSetSystemBrush(0);
		guiFillRectangle(in_menu_info->Rect.Left, top, in_menu_info->Rect.Right, top + l_item_height);

		// close canvas
		guiCloseCanvas(true);
	}

}

void gcmRenderBorder(gcmMenuInfo* in_menu_info)
{
	guiSize title_size;
	uint8_t title_bar_height;
	uint8_t title_label_height;

	// do nothing if no title is specified
	if (in_menu_info->Title == sysNULL)
		return;

	guiSetFont(gcmTITLE_FONT);
	title_size = guiGetResourceTextExtent(in_menu_info->Title);

	title_label_height = title_size.Height + 2 * gcmTITLE_TEXT_VERTICAL_GAP;
	title_bar_height = title_label_height + 2 * gcmTITLE_BAR_VERTICAL_GAP;

	// draw border and clear background
	guiOpenCanvas(in_menu_info->Rect.Left - 3, in_menu_info->Rect.Top - 2 - title_bar_height - 1, in_menu_info->Rect.Right + 3, in_menu_info->Rect.Bottom + 3);

	// clear background
	guiSetSystemBrush(0);
	guiFillRectangle(in_menu_info->Rect.Left - 3, in_menu_info->Rect.Top - 2 - title_bar_height - 1, in_menu_info->Rect.Right + 3, in_menu_info->Rect.Bottom + 3);

	// draw title border and background
	guiSetPen(1);
	guiDrawRectangle(in_menu_info->Rect.Left - 2, in_menu_info->Rect.Top - 2 - title_bar_height, in_menu_info->Rect.Right + 2, in_menu_info->Rect.Top - 2);

	guiSetSystemBrush(1);
	guiFillRectangle(in_menu_info->Rect.Left - 1, in_menu_info->Rect.Top - 2 - title_bar_height + 1, in_menu_info->Rect.Right + 1, in_menu_info->Rect.Top - 3);

	// text box
	guiSetSystemBrush(0);
	guiSetPen(1);
	guiFillRectangle(in_menu_info->Rect.Left + gcmTITLE_BAR_HORIZONTAL_OFFSET, in_menu_info->Rect.Top - 2 - gcmTITLE_BAR_VERTICAL_GAP - title_label_height, in_menu_info->Rect.Left + gcmTITLE_BAR_HORIZONTAL_OFFSET + gcmTITLE_TEXT_HORIZONTAL_GAP + title_size.Width + gcmTITLE_TEXT_HORIZONTAL_GAP, in_menu_info->Rect.Top - 2 - gcmTITLE_BAR_VERTICAL_GAP);
	guiDrawRectangle(in_menu_info->Rect.Left + gcmTITLE_BAR_HORIZONTAL_OFFSET, in_menu_info->Rect.Top - 2 - gcmTITLE_BAR_VERTICAL_GAP - title_label_height, in_menu_info->Rect.Left + gcmTITLE_BAR_HORIZONTAL_OFFSET + gcmTITLE_TEXT_HORIZONTAL_GAP + title_size.Width + gcmTITLE_TEXT_HORIZONTAL_GAP, in_menu_info->Rect.Top - 2 - gcmTITLE_BAR_VERTICAL_GAP);

	// write path
	guiDrawResourceText(in_menu_info->Rect.Left + gcmTITLE_BAR_HORIZONTAL_OFFSET + gcmTITLE_TEXT_HORIZONTAL_GAP, in_menu_info->Rect.Top - 2 - title_size.Height - gcmTITLE_BAR_VERTICAL_GAP - gcmTITLE_TEXT_VERTICAL_GAP, in_menu_info->Title);

	// draw border
	guiSetPen(1);
	guiDrawRectangle(in_menu_info->Rect.Left - 2, in_menu_info->Rect.Top - 2, in_menu_info->Rect.Right + 2, in_menu_info->Rect.Bottom + 2);

	guiCloseCanvas(true);
}
