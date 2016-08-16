/*****************************************************************************/
/* File browser application                                                  */
/*  Renderer for color screens                                               */
/*                                                                           */
/* Copyright (C) 2014 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
// Includes
#include <guiColorGraphics.h>
#include <fileUtils.h>
#include <fbRenderer.h>
#include <fbIconMapper.h>
#include <sysConfig.h>
#include <sysResource.h>

///////////////////////////////////////////////////////////////////////////////
// Constats
#define fbSTRING_BUFFER_LENGTH  20

#define fbDEFAULT_FONT REF_FNA_DOS

#define fbBACKGROUND_COLOR 0x2e2e2e
#define fbHEADER_BACKGROUND_COLOR 0x5e5e5e
#define fbDARKSHADE_COLOR 0x1d1d1d
#define fbLIGHTSHADE_COLOR 0x363636

#define fbHEADER_HEIGHT fbFILE_LIST_TOP
#define fbHEADER_TEXT_BACKGROUND_COLOR 0x434343
#define fbHEADER_TEXT_HEIGHT 8
#define fbHEADER_TEXT_LEFT 3
#define fbHEADER_TEXT_COLOR 0xffffff
#define fbHEADER_TEXT_TOP (fbHEADER_HEIGHT - fbHEADER_TEXT_HEIGHT) / 2
#define fbHEADER_FONT fbDEFAULT_FONT

#define fbFILE_LIST_TOP 18
#define fbFILE_LIST_ITEM_HEIGHT 10
#define fbFILE_LIST_DISPLAYED_ITEM_COUNT ((guiSCREEN_HEIGHT - fbFILE_LIST_TOP - fbFOOTER_MIN_HEIGHT) / fbFILE_LIST_ITEM_HEIGHT)
#define fbFILE_LIST_HEIGHT (fbFILE_LIST_ITEM_HEIGHT * fbFILE_LIST_DISPLAYED_ITEM_COUNT)
#define fbFILE_LIST_TEXT_COLOR 0xffffff
#define fbFILE_LIST_SELECTION_COLOR 0x007acc
#define fbFILE_LIST_FONT fbDEFAULT_FONT

#define fbFOOTER_TEXT_HEIGHT 8
#define fbFOOTER_TEXT_LEFT 2
#define fbFOOTER_MIN_HEIGHT 11
#define fbFOOTER_TOP (fbFILE_LIST_TOP + fbFILE_LIST_HEIGHT + 1)
#define fbFOOTER_HEIGHT (guiSCREEN_HEIGHT - fbFOOTER_TOP)
#define fbFOOTER_FONT fbDEFAULT_FONT

///////////////////////////////////////////////////////////////////////////////
// Local functions

///////////////////////////////////////////////////////////////////////////////
/// @brief Gets number of file items to display. It depens on the height of the screen and the height of the header and footer.
/// @return Number of items (lines) to display.
uint16_t fbRendererGetDisplayableItemCount(void)
{
	return fbFILE_LIST_DISPLAYED_ITEM_COUNT;
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
	guiCoordinate top = fbFILE_LIST_TOP + in_screen_index * fbFILE_LIST_ITEM_HEIGHT;
	guiSize icon_size;
	guiSize text_size;
	guiRect clipping_rect;
	int16_t oversize;
	guiSize bracket_size;

	// if empty item must be rendered
	if(in_file_info == sysNULL)
	{
		// open canvas
		guiOpenCanvas( 0, top, guiSCREEN_WIDTH-1, top + fbFILE_LIST_ITEM_HEIGHT);

		// Only clear background
		guiSetForegroundColor(fbBACKGROUND_COLOR);
		guiFillRectangle(0, top, guiSCREEN_WIDTH-1, top + fbFILE_LIST_ITEM_HEIGHT);

		// close canvas
		guiCloseCanvas();

		return 0;
	}
	else
	{
		// init
		bracket_size.Width = 0;
		bracket_size.Height = 0;

		// set background color
		if(in_selected)
		{
			guiSetBackgroundColor(fbFILE_LIST_SELECTION_COLOR);
			guiSetForegroundColor(fbFILE_LIST_SELECTION_COLOR);
		}
		else
		{
			guiSetBackgroundColor(fbBACKGROUND_COLOR);
			guiSetForegroundColor(fbBACKGROUND_COLOR);
		}

		// draw icon
		icon_size.Width = 0;
		icon_size.Height = 0;
		icon = fbGetFileIcon(in_file_info);
		if(icon != sysNULL)
		{
			icon_size = guiGetBitmapSize(icon);
			if(in_horizontal_text_offset == 0)
			{
				guiFillRectangle(0, top, icon_size.Width + 2, top + fbFILE_LIST_ITEM_HEIGHT - 1);

				guiSetForegroundColor(fbFILE_LIST_TEXT_COLOR);
				guiDrawBitmapFromResource(1, top + (fbFILE_LIST_ITEM_HEIGHT - icon_size.Height + 1) / 2, icon);
			}
		}

		// display name
		guiSetFont(fbFILE_LIST_FONT);
		guiSetForegroundColor(fbFILE_LIST_TEXT_COLOR);

		if((in_file_info->Flags & fbFF_FOLDER) != 0)
		{
			bracket_size = guiGetTextExtent("[");
		}

		text_size = guiGetTextExtent(in_file_info->FileName);
		clipping_rect.Left = icon_size.Width + 2;
		clipping_rect.Top = top + 2;
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

		// set colors for clearing areas where is no icon or text
		if(in_selected)
		{
			guiSetForegroundColor(fbFILE_LIST_SELECTION_COLOR);
		}
		else
		{
			guiSetForegroundColor(fbBACKGROUND_COLOR);
		}

		// clear area above text
		guiFillRectangle(icon_size.Width + 3, top, guiSCREEN_WIDTH - 1, top + 1);

		// determine item text oversize
		oversize = (clipping_rect.Left + text_size.Width + 2 * bracket_size.Width) - guiSCREEN_WIDTH;

		if(oversize < 0)
		{
			guiFillRectangle(guiSCREEN_WIDTH + oversize, top, guiSCREEN_WIDTH - 1, top + fbFILE_LIST_ITEM_HEIGHT - 1);

			oversize = 0;
		}

		return oversize;
	}
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Renders header (current path)
/// @param in_title Title string (displayed in the frpont of the path)
/// @param in_path Current path string
void fbRenderHeader(sysString in_title, sysString in_path)
{
	guiSize size;
	sysChar path[fileMAX_PATH+1];
	bool repeat;

	// store path
	strCopyString(path, fileMAX_PATH, 0, in_path);

	// open canvas
	guiOpenCanvas( 0, 0, guiSCREEN_WIDTH-1, fbHEADER_HEIGHT);

	do
	{
		repeat = false;
		size = guiGetTextExtent(path);
		if( size.Width > guiSCREEN_WIDTH - 2 * fbHEADER_TEXT_LEFT - 2 )
		{
			if(!fileShortenDisplayPath(path, fileMAX_PATH))
				repeat = false;
			else
				repeat = true;
		}

	} while(repeat);

	// header background
	guiSetForegroundColor(fbHEADER_BACKGROUND_COLOR);
	guiFillRectangle(0, 0, guiSCREEN_WIDTH - 1, fbHEADER_HEIGHT - 1);

	// header bottom shade
	guiSetForegroundColor(fbDARKSHADE_COLOR);
	guiDrawHorizontalLine(0, fbHEADER_HEIGHT - 1, guiSCREEN_WIDTH - 1);

	// header text area background
	guiSetForegroundColor(fbHEADER_TEXT_BACKGROUND_COLOR);
	guiFillRectangle(fbHEADER_TEXT_LEFT, fbHEADER_TEXT_TOP - 1, guiSCREEN_WIDTH - fbHEADER_TEXT_LEFT, fbHEADER_TEXT_TOP + fbHEADER_TEXT_HEIGHT);

	guiSetForegroundColor(fbDARKSHADE_COLOR);
	guiDrawHorizontalLine(fbHEADER_TEXT_LEFT - 1, fbHEADER_TEXT_TOP - 3, guiSCREEN_WIDTH - fbHEADER_TEXT_LEFT);

	guiSetForegroundColor(fbLIGHTSHADE_COLOR);
	guiDrawRectangle(fbHEADER_TEXT_LEFT - 1, fbHEADER_TEXT_TOP - 2, guiSCREEN_WIDTH - fbHEADER_TEXT_LEFT, fbHEADER_TEXT_TOP + fbHEADER_TEXT_HEIGHT + 1);

	guiSetForegroundColor(fbHEADER_TEXT_COLOR);
	guiSetBackgroundColor(fbHEADER_TEXT_BACKGROUND_COLOR);

	guiSetClipping(fbHEADER_TEXT_LEFT + 1, fbHEADER_TEXT_TOP, guiSCREEN_WIDTH - 1 - fbHEADER_TEXT_LEFT, fbHEADER_TEXT_TOP + fbHEADER_TEXT_HEIGHT);
	guiSetFont(REF_FNA_DOS);
	guiDrawText(fbHEADER_TEXT_LEFT + 1, fbHEADER_TEXT_TOP, path);
	guiResetClipping();

	// close canvas
  guiCloseCanvas();
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Renders footer (file information area)
/// @param in_file_index Index of the selectd file
/// @param in_file_count Total number of files
/// @param in_file_info Information of the current file
void fbRenderFooter(uint16_t in_file_index, uint16_t in_file_count, fbFileInformation* in_file_info)
{
	sysChar buffer[fbSTRING_BUFFER_LENGTH ];
	sysStringLength pos;
	guiSize counter_size;
	guiSize datetime_size;
	guiSize filesize_size;


	// open canvas
	guiOpenCanvas( 0, fbFOOTER_TOP, guiSCREEN_WIDTH-1, guiSCREEN_HEIGHT-1);

	// footer background
	guiSetForegroundColor(fbHEADER_TEXT_BACKGROUND_COLOR);
	guiFillRectangle(0, fbFOOTER_TOP + 1, guiSCREEN_WIDTH - 1, guiSCREEN_HEIGHT - 1);

	guiSetForegroundColor(fbLIGHTSHADE_COLOR);
	guiDrawHorizontalLine(0, fbFOOTER_TOP, guiSCREEN_WIDTH - 1);

	guiSetForegroundColor(fbHEADER_TEXT_COLOR);
	guiSetBackgroundColor(fbHEADER_TEXT_BACKGROUND_COLOR);
	guiSetFont(REF_FNA_DOS);

	// display file index and file count
	pos = strWordToString(buffer, fbSTRING_BUFFER_LENGTH  , in_file_index, 0, 0, 0);
	pos = strCopyConstString(buffer, fbSTRING_BUFFER_LENGTH  , pos, (sysConstString)"/");
	pos = strWordToStringPos(buffer, fbSTRING_BUFFER_LENGTH  , pos, in_file_count, 0, 0, 0);
	counter_size = guiGetTextExtent(buffer);
	guiDrawText( fbFOOTER_TEXT_LEFT, fbFOOTER_TOP + 2 + (guiSCREEN_HEIGHT - fbFOOTER_TOP - fbFOOTER_TEXT_HEIGHT) / 2, buffer);

	if(in_file_info != sysNULL)
	{
		// display file date
		pos = 0;
		pos = sysConvertDateToString(buffer, fbSTRING_BUFFER_LENGTH , pos, &in_file_info->DateTime, '-');
		pos = strCopyConstString(buffer, fbSTRING_BUFFER_LENGTH , pos, (sysConstString)" ");
		pos = strWordToStringPos(buffer, fbSTRING_BUFFER_LENGTH , pos, in_file_info->DateTime.Hour, 2, 0, TS_NO_ZERO_BLANKING);
		pos = strCopyConstString(buffer, fbSTRING_BUFFER_LENGTH , pos, (sysConstString)":");
		pos = strWordToStringPos(buffer, fbSTRING_BUFFER_LENGTH , pos, in_file_info->DateTime.Minute, 2, 0, TS_NO_ZERO_BLANKING);
		datetime_size = guiGetTextExtent(buffer);
		guiDrawText( guiSCREEN_WIDTH - datetime_size.Width, fbFOOTER_TOP + 2 + (guiSCREEN_HEIGHT - fbFOOTER_TOP - fbFOOTER_TEXT_HEIGHT) / 2, buffer);
	
		// display size (only for files)
		if((in_file_info->Flags & BV(fbFF_FOLDER_FILE_BIT)) == fbFF_FILE)
		{
			fileSizeToString(buffer, fbSTRING_BUFFER_LENGTH , in_file_info->Size);
			filesize_size = guiGetTextExtent(buffer);
			guiDrawText( (guiSCREEN_WIDTH + counter_size.Width - datetime_size.Width - filesize_size.Width) / 2, fbFOOTER_TOP + 2 + (guiSCREEN_HEIGHT - fbFOOTER_TOP - fbFOOTER_TEXT_HEIGHT) / 2, buffer);
		}
	}

	// close canvas
  guiCloseCanvas();
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
	uint16_t x1,y1;

	size = guiGetBitmapSize(REF_BMP_PROGRESS1);

	// draw first phase of the bitmap
	x1 = (guiSCREEN_WIDTH/2) - (size.Width/2);
	y1 = (guiSCREEN_HEIGHT/2) - (size.Height/2);

	// open canvas
	//guiOpenCanvas( x1, y1, x2, y2);

	guiDrawBitmapFromResource(x1, y1, REF_BMP_PROGRESS1);

	guiSetForegroundColor(fbLIGHTSHADE_COLOR);
	guiDrawRectangle(x1 - 1, y1 - 1, x1 + size.Width, y1 + size.Height);

	guiSetForegroundColor(fbDARKSHADE_COLOR);
	guiDrawHorizontalLine(x1 - 1, y1 + size.Height + 1, x1 + size.Width);
	guiDrawVerticalLine(x1 + size.Width + 1, y1, y1 + size.Height + 1);

	// close canvas
  guiCloseCanvas();

	guiRefreshScreen();
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Displays current animation phase of the wait indiactor
/// @param in_current_phase Current phase index
void fbRenderWaitIndicatorNext(uint8_t* in_current_phase)
{
	guiSize size;
	uint16_t x1,y1;
	static sysResourceAddress bitmaps[31] = { REF_BMP_PROGRESS1, REF_BMP_PROGRESS2, REF_BMP_PROGRESS3, REF_BMP_PROGRESS4, REF_BMP_PROGRESS5 , REF_BMP_PROGRESS6, REF_BMP_PROGRESS7, REF_BMP_PROGRESS8,
																						REF_BMP_PROGRESS9, REF_BMP_PROGRESS10, REF_BMP_PROGRESS11, REF_BMP_PROGRESS12, REF_BMP_PROGRESS13, REF_BMP_PROGRESS14, REF_BMP_PROGRESS15, REF_BMP_PROGRESS16,
																						REF_BMP_PROGRESS17, REF_BMP_PROGRESS18, REF_BMP_PROGRESS19, REF_BMP_PROGRESS20, REF_BMP_PROGRESS21, REF_BMP_PROGRESS22, REF_BMP_PROGRESS23, REF_BMP_PROGRESS24,
																						REF_BMP_PROGRESS25, REF_BMP_PROGRESS26, REF_BMP_PROGRESS27, REF_BMP_PROGRESS28, REF_BMP_PROGRESS29, REF_BMP_PROGRESS30, REF_BMP_PROGRESS31 };

	if(++(*in_current_phase) >= 31)
		*in_current_phase = 0;

	size = guiGetBitmapSize(bitmaps[*in_current_phase]);

	// draw current phase of the bitmap
	x1 = (guiSCREEN_WIDTH/2) - (size.Width/2);
	y1 = (guiSCREEN_HEIGHT/2) - (size.Height/2);

	// open canvas
	guiOpenCanvas( x1, y1, size.Width, size.Height);

	guiDrawBitmapFromResource(x1, y1, bitmaps[*in_current_phase]);

	// close canvas
	guiCloseCanvas();

	guiRefreshScreen();
}
