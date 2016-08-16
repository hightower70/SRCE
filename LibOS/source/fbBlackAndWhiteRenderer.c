/*****************************************************************************/
/* File browser application                                                  */
/*  Renderer for black and white screens                                     */
/*                                                                           */
/* Copyright (C) 2015 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <fbRenderer.h>
#include <sysResource.h>
#include <fbIconMapper.h>
#include <fileUtils.h>
#include "sysConfig.h"

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/
#define STRING_BUFFER_LENGTH 20

#if !defined(fbDEFAULT_FONT)
#error "File browser is not configured."
#endif

//#define fbDEFAULT_FONT REF_FNA_DOS

#define fbBORDER_WIDTH 1

// header settings
#ifndef fbHEADER_FONT
#define fbHEADER_FONT fbDEFAULT_FONT
#endif

#define fbHEADER_BAR_VERTICAL_GAP 2
#define fbHEADER_BAR_HORIZONTAL_GAP 3
#define fbHEADER_TEXT_HORIZONTAL_GAP 1


// file item settings
#ifndef fbFILE_LIST_FONT
#define fbFILE_LIST_FONT fbDEFAULT_FONT
#endif

#ifndef fbFILE_LIST_ITEM_GAP
#define fbFILE_LIST_ITEM_GAP 0
#endif

// footer settings
#ifndef fbFOOTER_FONT
#define fbFOOTER_FONT fbDEFAULT_FONT
#endif

#define WAIT_INDICATOR_PADDING 5
#define WAIT_INDICATOR_MARGIN 3

/*****************************************************************************/
/* Local variables                                                           */
/*****************************************************************************/
static uint8_t l_header_height;
static uint8_t l_item_height;
static uint8_t l_displayable_item_count;

/*****************************************************************************/
/* Rendering functions                                                       */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Gets number of file items to display. It depens on the height of the screen and the height of the header and footer.
/// @return Number of items (lines) to display.
uint16_t fbRendererGetDisplayableItemCount(void)
{
	return l_displayable_item_count;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Initializes rendering engine (calculates and caches dimensions, etc.)
void fbRendererInit(void)
{
	guiSize size;

	// calculate header height
	guiSetFont(fbHEADER_FONT);
	size = guiGetTextExtent("Hj");
	l_header_height = size.Height + 2 * fbHEADER_BAR_VERTICAL_GAP + 2 * fbBORDER_WIDTH + fbBORDER_WIDTH; // text + gap (top, bottom) + rectangle around the text + bottom separator line

	guiSetFont(fbFILE_LIST_FONT);
	size = guiGetTextExtent("Hj");
	l_item_height = size.Height + fbFILE_LIST_ITEM_GAP;

	guiSetFont(fbFOOTER_FONT);
	size = guiGetTextExtent("Hj");

	l_displayable_item_count = (guiSCREEN_HEIGHT - l_header_height - 1 - size.Height - fbBORDER_WIDTH - 1) / l_item_height;

}

///////////////////////////////////////////////////////////////////////////////
/// @brief  Renders one items (file or directory). If null item is specified than empty item will be rendered.
/// @param in_file_info Item to render. When null than empty item (clear background) will be rendered.
/// @param in_screen_index Position of the item on the list (the topmost item has zero index)
/// @param in_selected True if item must be rednered as selected item
/// @param in_horizontal_text_offset Offset of the item text. Used for horizontal item text scrolling.
uint16_t fbRenderItem(fbFileInformation* in_file_info, uint16_t in_screen_index, bool in_selected, uint16_t in_horizontal_text_offset)
{
	sysResourceAddress icon;
	uint16_t top = l_header_height + 1 + in_screen_index * l_item_height;
	guiSize icon_size;
	guiSize bracket_size;
	guiSize text_size;
	guiRect clipping_rect;
	int16_t oversize;

	// sanity check
	if(in_file_info == sysNULL)
	{
		// open canvas
		guiOpenCanvas( 0, top, guiSCREEN_WIDTH-1, top + l_item_height);

		guiSetSystemBrush(0);
		guiFillRectangle(0, top, guiSCREEN_WIDTH-1, top + l_item_height);

		// close canvas
		guiCloseCanvas(true);

		return 0;
	}
	else
	{
		// init
		bracket_size.Width = 0;
		bracket_size.Height = 0;

		// open canvas
		guiOpenCanvas( 0, top, guiSCREEN_WIDTH-1, top + l_item_height);

		guiSetFont(fbFILE_LIST_FONT);
		text_size = guiGetTextExtent("Hj");

		// clear background
		if(in_selected)
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

		guiFillRectangle(0, top, guiSCREEN_WIDTH-1, top + text_size.Height);

		// draw icon
		icon_size.Width = 0;
		icon_size.Height = 0;
		icon = fbGetFileIcon(in_file_info);
		if(icon != sysNULL)
		{
			icon_size = guiGetBitmapSize(icon);
			guiDrawBitmap(1, top + (l_item_height - fbFILE_LIST_ITEM_GAP - icon_size.Height) / 2, icon);
		}

		// display name
		if((in_file_info->Flags & fbFF_FOLDER) != 0)
		{
			bracket_size = guiGetTextExtent("[");
		}

		text_size = guiGetTextExtent(in_file_info->FileName);
		clipping_rect.Left = icon_size.Width + 2;
		clipping_rect.Top = top + fbFILE_LIST_ITEM_GAP / 2;
		clipping_rect.Right = guiSCREEN_WIDTH - 1;
		clipping_rect.Bottom = clipping_rect.Top + text_size.Height;

		guiSetClippingRect(clipping_rect);
		guiDrawText(clipping_rect.Left - in_horizontal_text_offset + bracket_size.Width, clipping_rect.Top, in_file_info->FileName);
		if((in_file_info->Flags & fbFF_FOLDER) != 0)
		{
			guiDrawText(clipping_rect.Left - in_horizontal_text_offset, clipping_rect.Top, "[");
			guiDrawText(clipping_rect.Left - in_horizontal_text_offset + text_size.Width + bracket_size.Width, clipping_rect.Top, "]");
		}
		guiResetClipping();

		guiSetDrawMode(0);

		// close canvas
		guiCloseCanvas(true);

		// determine item text oversize
		oversize = (clipping_rect.Left + text_size.Width + 2 * bracket_size.Width) - guiSCREEN_WIDTH;

		if(oversize < 0)
		{
			//		guiFillRectangle(guiSCREEN_WIDTH + oversize, top, guiSCREEN_WIDTH - 1, top + fbFILE_LIST_ITEM_HEIGHT - 1);

			oversize = 0;
		}
	}

	return oversize;
}


///////////////////////////////////////////////////////////////////////////////
/// @brief Renders header (current path)
/// @param in_title Title string (displayed before the path)
/// @param in_path Current path string
void fbRenderHeader(sysString in_title, sysString in_path)
{
	guiSize size;
	sysChar path[fileMAX_PATH+1];
	bool repeat;

	// calculate coordinates
	guiSetFont(fbHEADER_FONT);
	size = guiGetTextExtent("Hj");

	// store path
	strCopyString(path, fileMAX_PATH, 0, in_path);

	// open canvas
	guiOpenCanvas( 0, 0, guiSCREEN_WIDTH-1, l_header_height);

	// separator line
	guiSetPen(1);
	guiDrawHorizontalLine(0, guiSCREEN_WIDTH-1, l_header_height - 1);

	// background
	guiSetSystemBrush(1);
	guiFillRectangle(0, 0, guiSCREEN_WIDTH-1, l_header_height - 2);

	// empty line
	guiSetPen(0);
	guiDrawHorizontalLine(0, guiSCREEN_WIDTH - 1, l_header_height);

	// shorten path
	do
	{
		repeat = false;
		size = guiGetTextExtent(path);
		if( size.Width > guiSCREEN_WIDTH - 1 - 2 * fbHEADER_BAR_HORIZONTAL_GAP + 2 * fbBORDER_WIDTH + 2 * fbHEADER_TEXT_HORIZONTAL_GAP)
		{
			if(!fileShortenDisplayPath(path, fileMAX_PATH))
				repeat = false;
			else
				repeat = true;
		}

	} while(repeat);

	// text box
	guiSetPen(1);
	guiDrawRectangle(fbHEADER_BAR_HORIZONTAL_GAP, fbHEADER_BAR_VERTICAL_GAP, guiSCREEN_WIDTH - 1 - fbHEADER_BAR_HORIZONTAL_GAP, fbHEADER_BAR_VERTICAL_GAP + size.Height + 2 * fbBORDER_WIDTH - 1);

	guiSetSystemBrush(0);
	guiFillRectangle(fbHEADER_BAR_HORIZONTAL_GAP + fbBORDER_WIDTH, fbHEADER_BAR_VERTICAL_GAP + fbBORDER_WIDTH, guiSCREEN_WIDTH - 1 - fbHEADER_BAR_HORIZONTAL_GAP - fbBORDER_WIDTH, fbHEADER_BAR_VERTICAL_GAP + size.Height + fbBORDER_WIDTH - 1);

	// write path
	guiSetClipping(fbHEADER_BAR_HORIZONTAL_GAP + fbBORDER_WIDTH, fbHEADER_BAR_VERTICAL_GAP + fbBORDER_WIDTH, guiSCREEN_WIDTH - 1 - fbHEADER_BAR_HORIZONTAL_GAP - fbBORDER_WIDTH, fbHEADER_BAR_VERTICAL_GAP + size.Height + fbBORDER_WIDTH - 1);
	guiDrawText(fbHEADER_BAR_HORIZONTAL_GAP + fbBORDER_WIDTH + fbHEADER_TEXT_HORIZONTAL_GAP, fbHEADER_BAR_VERTICAL_GAP + fbBORDER_WIDTH, path);
	guiResetClipping();

	// close canvas
  guiCloseCanvas(true);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Renders footer (file information area)
/// @param in_file_index Index of the selectd file
/// @param in_file_count Total number of files
/// @param in_file_info Information of the current file
void fbRenderFooter(uint16_t in_file_index, uint16_t in_file_count, fbFileInformation* in_file_info)
{
	sysChar buffer[STRING_BUFFER_LENGTH];
	sysStringLength pos;
	guiSize counter_size;
	guiSize datetime_size;
	guiSize filesize_size;
	guiCoordinate footer_top = l_header_height + 1 + l_item_height * l_displayable_item_count;
	guiSize font_size;
	guiCoordinate footer_text_top;

	// open canvas
	guiOpenCanvas( 0, footer_top, guiSCREEN_WIDTH-1, guiSCREEN_HEIGHT-1);

	guiSetSystemBrush(0);
	guiFillRectangle(0, footer_top, guiSCREEN_WIDTH - 1, guiSCREEN_HEIGHT - 1);

	guiSetPen(1);
	guiDrawHorizontalLine(0,guiSCREEN_WIDTH-1, footer_top + 1);

	guiSetPen(1);

	guiSetFont(fbFOOTER_FONT);
	font_size = guiGetTextExtent("Hj");

	// display count
	pos = strWordToString(buffer, STRING_BUFFER_LENGTH, in_file_index, 0, 0, 0);
	pos = strCopyConstString(buffer, STRING_BUFFER_LENGTH, pos, (sysConstString)"/");
	pos = strWordToStringPos(buffer, STRING_BUFFER_LENGTH, pos, in_file_count, 0, 0, 0);
	counter_size = guiGetTextExtent(buffer);
	footer_text_top = footer_top + 1 + fbBORDER_WIDTH + 1 + (guiSCREEN_HEIGHT - footer_top - 1 - font_size.Height) / 2;
	guiDrawText( 1, footer_text_top, buffer);

	if(in_file_info != sysNULL)
	{
		// display file date
		pos = 0;
		pos = sysConvertDateToString(buffer, STRING_BUFFER_LENGTH, pos, &in_file_info->DateTime, '-');
		pos = strCopyConstString(buffer, STRING_BUFFER_LENGTH, pos, (sysConstString)" ");
		pos = strWordToStringPos(buffer, STRING_BUFFER_LENGTH, pos, in_file_info->DateTime.Hour, 2, 0, TS_NO_ZERO_BLANKING);
		pos = strCopyConstString(buffer, STRING_BUFFER_LENGTH, pos, (sysConstString)":");
		pos = strWordToStringPos(buffer, STRING_BUFFER_LENGTH, pos, in_file_info->DateTime.Minute, 2, 0, TS_NO_ZERO_BLANKING);
		datetime_size = guiGetTextExtent(buffer);
		guiDrawText( guiSCREEN_WIDTH - datetime_size.Width, footer_text_top, buffer);
	
		// display size (only for files)
		if((in_file_info->Flags & BV(fbFF_FOLDER_FILE_BIT)) == fbFF_FILE)
		{
			fileSizeToString(buffer, STRING_BUFFER_LENGTH, in_file_info->Size);
			filesize_size = guiGetTextExtent(buffer);
			guiDrawText( (guiSCREEN_WIDTH + counter_size.Width - datetime_size.Width - filesize_size.Width) / 2, footer_text_top, buffer);
		}
	}

	guiSetDrawMode( 0 );

	// close canvas
  guiCloseCanvas(true);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Copies frame buffer content to the display. Calls empty driver function when no frame buffer is used.
void fbRefreshScreen(void)
{
	guiRefreshScreen();
}


///////////////////////////////////////////////////////////////////////////////
/// @brief Displays first animation phase (and border) of the wait indiactor
void fbRenderWaitIndicatorShow(void)
{
	guiSize size;
	guiCoordinate x1,y1;
	guiCoordinate x2,y2;

	// get indicator area
	fbRenderGetWaitIndicatorRect(&x1, &y1, &x2, &y2);

	// open canvas
	guiOpenCanvas( x1, y1, x2, y2);

	// clear background
	guiSetSystemBrush(0);
	guiFillRectangle(x1, y1, x2, y2);

	// draw border
	size = guiGetBitmapSize(REF_BMP_PROGRESS0);

	x1 = (guiSCREEN_WIDTH/2) - (size.Width + 2 * WAIT_INDICATOR_PADDING) / 2;
	y1 = (guiSCREEN_HEIGHT/2) - (size.Height + 2 * WAIT_INDICATOR_PADDING) / 2;

	x2 = (guiSCREEN_WIDTH/2) + (size.Width + 2 * WAIT_INDICATOR_PADDING) / 2;
	y2 = (guiSCREEN_HEIGHT/2) + (size.Height + 2 * WAIT_INDICATOR_PADDING) / 2;

	guiSetPen(1);
	guiDrawRectangle(x1, y1, x2, y2);

	// draw first phase of the bitmap
	x1 = (guiSCREEN_WIDTH/2) - (size.Width/2);
	y1 = (guiSCREEN_HEIGHT/2) - (size.Height/2);

	guiDrawBitmap(x1,y1,REF_BMP_PROGRESS0);

	// close canvas
  guiCloseCanvas(true);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Gets wait indicator area coordinates
void fbRenderGetWaitIndicatorRect(guiCoordinate* out_left, guiCoordinate* out_top, guiCoordinate* out_right, guiCoordinate* out_bottom)
{
	guiSize size;

	size = guiGetBitmapSize(REF_BMP_PROGRESS0);

	// clear background
	*out_left = (guiSCREEN_WIDTH / 2) - (size.Width + 2 * WAIT_INDICATOR_MARGIN + 2 * WAIT_INDICATOR_PADDING) / 2;
	*out_top = (guiSCREEN_HEIGHT / 2) - (size.Height + 2 * WAIT_INDICATOR_MARGIN + 2 * WAIT_INDICATOR_PADDING) / 2;

	*out_right = (guiSCREEN_WIDTH / 2) + (size.Width + 2 * WAIT_INDICATOR_MARGIN + 2 * WAIT_INDICATOR_PADDING) / 2;
	*out_bottom = (guiSCREEN_HEIGHT / 2) + (size.Height + 2 * WAIT_INDICATOR_MARGIN + 2 * WAIT_INDICATOR_PADDING) / 2;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Displays current animation phase of the wait indiactor
/// @param in_current_phase Current phase index
void fbRenderWaitIndicatorNext(uint8_t* in_current_phase)
{
	guiSize size;
	guiCoordinate x1,y1;
	static sysResourceAddress bitmaps[8] = { REF_BMP_PROGRESS0, REF_BMP_PROGRESS1, REF_BMP_PROGRESS2, REF_BMP_PROGRESS3, REF_BMP_PROGRESS4, REF_BMP_PROGRESS5 , REF_BMP_PROGRESS6, REF_BMP_PROGRESS7 };

	if(++(*in_current_phase) >= 8)
		*in_current_phase = 0;

	size = guiGetBitmapSize(bitmaps[*in_current_phase]);

	// draw current phase of the bitmap
	x1 = (guiSCREEN_WIDTH/2) - (size.Width/2);
	y1 = (guiSCREEN_HEIGHT/2) - (size.Height/2);

	// open canvas
	guiOpenCanvas( x1, y1, x1 + size.Width, y1 + size.Height);

	guiDrawBitmap(x1, y1, bitmaps[*in_current_phase]);

	// close canvas
  guiCloseCanvas(true);
}

