/*****************************************************************************/
/* File browser application                                                  */
/*                                                                           */
/* Copyright (C) 2015 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
// Includes
#include <sysTimer.h>
#include <fileTypes.h>
#include <fbFileBrowser.h>
#include <fbFileEnumerator.h>
#include <fbBuffer.h>
#include <fbRenderer.h>
#include <fileUtils.h>
#include <fileStandardFunctions.h>

///////////////////////////////////////////////////////////////////////////////
// Constants
#define WAIT_INDICATOR_DELAY 500	 // delay for wait indicator display in ms
#define WAIT_INDICATOR_ANIMATION_DELAY 80 // delay between two phase of the wait indicator
#define FILE_BROWSER_BATCH_SIZE 16
#define FILE_BROWSER_HORIZONTAL_SCROLL_DELAY 500
#define FILE_BROWSER_HORIZONTAL_SCROLL_SPEED_DELAY 30

///////////////////////////////////////////////////////////////////////////////
// Module local variables
static sysChar l_current_path[fileMAX_PATH];
static fbStatus l_status = fbs_Inactive;
static sysTimeStamp l_wait_indicator_timestamp;
static bool l_wait_indicator_visible = false;
static uint8_t l_wait_indicator_phase;
static uint16_t l_current_file_index;
static uint16_t l_first_visible_file_index;
static uint16_t l_selected_item_horizontal_oversize;
static sysTimeStamp l_horizontal_scroll_timestamp;
static uint16_t l_horizontal_scroll_offset;

///////////////////////////////////////////////////////////////////////////////
// Local function prototypes
static void RenderScreen(uint16_t in_first_visible_item_index);
static void UpdateSelectedItem(uint16_t in_new_selected_file_index);
static void ProcessSelectionUp(uint16_t in_items_count);
static void ProcessSelectionDown(uint16_t in_items_count);
static void UpdateHorizontalScrollState(void);

///////////////////////////////////////////////////////////////////////////////
/// @brief Moves file selection with the given offset
/// @param in_offset Offset to move the file selection
void fbSelectionMove(int16_t in_offset)
{
	switch(in_offset)
	{
		case fbSELECTION_FIRST:
			ProcessSelectionUp(l_current_file_index);
			break;

		case fbSELECTION_PAGE_UP:
			ProcessSelectionUp(fbRendererGetDisplayableItemCount()-1);
			break;

		case fbSELECTION_LAST:
			ProcessSelectionDown(fbBufferGetFileCount() - l_current_file_index - 1);
			break;

		case fbSELECTION_PAGE_DOWN:
			ProcessSelectionDown(fbRendererGetDisplayableItemCount()-1);
			break;

		default:
			if(in_offset < 0)
				ProcessSelectionUp((uint16_t)(-in_offset));
			else
				ProcessSelectionDown((uint16_t)in_offset);
			break;
	}
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Enters the selected folder, returns false when selection is not a folder
/// @return true if file was opened, false when folder was opened
bool fbSelectionEnter(void)
{
	fbFileInformation default_file_information;
	sysChar filename_buffer[fileFAT_MAX_FILENAME_LENGTH+1];
	sysStringLength pos;

	default_file_information.Flags = 0;

	// check if the current selection is a folder
	if((fbBufferGetFileInfo(l_current_file_index)->Flags & BV(fbFF_FOLDER_FILE_BIT)) == fbFF_FOLDER)
	{
		// check if moving upwards
		if( strCompareConstString(fbBufferGetFileInfo(l_current_file_index)->FileName, (sysConstString)"..") == 0 )
		{
			// if moving upward store the current directory name as the default directory
			pos = strFindLastChar(l_current_path, filePATH_SEPARATOR );

			if( pos != sysSTRING_INVALID_POS)
			{
				// store directory name
				strCopyString(filename_buffer, fileFAT_MAX_FILENAME_LENGTH, 0, &l_current_path[pos+1]);
				default_file_information.FileName = filename_buffer;
				default_file_information.Flags = fbFF_DEFAULT;
			}
		}
	
		// if entring directory simply concat paths
		fileConcatPath(l_current_path, fileMAX_PATH, fbBufferGetFileInfo(l_current_file_index)->FileName);

		// clear file buffer
		fbBufferClear();

		// store default file if avaiable
		if( (default_file_information.Flags & fbFF_DEFAULT) != 0)
		{
			fbBufferStore(&default_file_information);
		}

		// start folder parsing
		l_status = fbs_ParseStart;

		return false;
	}
	else
	{
		return true;
	}
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Moves selection up by the specified number of items
/// @param in_items_count Number of items to move selection upwards
static void ProcessSelectionUp(uint16_t in_items_count)
{
	uint16_t new_selected_file_index;

	if(l_current_file_index == 0)
		return;

	if(l_current_file_index - in_items_count < 0)
		new_selected_file_index = 0;
	else
		new_selected_file_index = l_current_file_index - in_items_count;

	UpdateSelectedItem(new_selected_file_index);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Moves selection down by the specified number of items
/// @param in_items_count Number of items to move selection downwards
static void ProcessSelectionDown(uint16_t in_items_count)
{
	uint16_t total_item_count = fbBufferGetFileCount();
	uint16_t new_selected_file_index;
	
	if(l_current_file_index == total_item_count - 1)
		return;

	if(l_current_file_index + in_items_count >= total_item_count)
		new_selected_file_index = total_item_count - 1;
	else
		new_selected_file_index = l_current_file_index + in_items_count;

	UpdateSelectedItem(new_selected_file_index);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Updates selected item (removes selection fromthe old item and displays selection on the new item, and updates footer information) 
static void UpdateSelectedItem(uint16_t in_new_selected_file_index)
{
	if(in_new_selected_file_index < l_first_visible_file_index)
	{
		// render whole screen
		l_first_visible_file_index = in_new_selected_file_index;
		l_current_file_index = in_new_selected_file_index;
		RenderScreen(l_first_visible_file_index);
	}
	else
	{
		if( in_new_selected_file_index >= l_first_visible_file_index + fbRendererGetDisplayableItemCount())
		{
			// render whole screen
			l_first_visible_file_index = in_new_selected_file_index - fbRendererGetDisplayableItemCount() + 1;
			l_current_file_index = in_new_selected_file_index;
			RenderScreen(l_first_visible_file_index);
		}
		else
		{
			// render only affected items
			fbRenderItem(fbBufferGetFileInfo(l_current_file_index), l_current_file_index - l_first_visible_file_index, false, 0);
			l_selected_item_horizontal_oversize = fbRenderItem(fbBufferGetFileInfo(in_new_selected_file_index), in_new_selected_file_index - l_first_visible_file_index, true, 0);
			l_current_file_index = in_new_selected_file_index;
			fbRenderFooter(l_current_file_index+1, fbBufferGetFileCount(), fbBufferGetFileInfo(l_current_file_index));
		}
	}

	// update screen content and scroll state
	fbRefreshScreen();

	UpdateHorizontalScrollState();
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Activates file browser starting from the given path
/// @param in_path start path of the browser
void fbActivate(sysString in_path)
{
	strCopyString(l_current_path, fileMAX_PATH, 0, in_path);
	fbBufferClear();
	fbRendererInit();

	l_status = fbs_ParseStart;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Deactivates file browser
void fbDeactivate(void)
{
	l_status = fbs_Inactive;
}

///////////////////////////////////////////////////////////////////////////////
/// vbrief File browser main task, must be called periodically
void fbTask(void)
{
	uint8_t count;
	bool success;

	// return immediately if there is no activity
	if(l_status == fbs_Inactive)
		return;

	// device mode
	switch(l_status)
	{
		case fbs_ParseStart:
			l_wait_indicator_visible = false;
			if(fbFileEnumerationOpen((sysString)l_current_path))
			{
				l_wait_indicator_timestamp = sysTimerGetTimestamp();
				l_status = fbs_Parsing;
			}
			else
			{
				fbFileEnumerationClose();
				l_status = fbs_Error;
			}
			break;

		case fbs_Parsing:
			fbWaitIndicatorUpdate();
			count = 0;
			success = true;
			while(count < FILE_BROWSER_BATCH_SIZE && success)
			{
				success = fbFileEnumertationNext();
				count++;
			}
			if(!success)
			{
				fbFileEnumerationClose();
				fbBufferSortStart();
				l_status = fbs_Sorting;
			}
			break;

		case fbs_Sorting:
			fbWaitIndicatorUpdate();
			if(fbBufferSort())
			{
				l_status = fbs_Active;
				fbWaitIndicatorHide();

				l_current_file_index = fbGetDefaultFileIndex();
				if(l_current_file_index == fbINVALID_INDEX)
				{
					l_current_file_index = 0;
					l_first_visible_file_index = 0;
				}
				else
				{
					if(l_current_file_index >= fbRendererGetDisplayableItemCount())
					{
						if(l_current_file_index > fbBufferGetFileCount() - fbRendererGetDisplayableItemCount() )
						{
							// current item is on the last page
							l_first_visible_file_index = fbBufferGetFileCount() - fbRendererGetDisplayableItemCount();
						}
						else
						{
							l_first_visible_file_index  = l_current_file_index - fbRendererGetDisplayableItemCount() / 2;
						}
					}
					else
					{
						// current item is on the frist page
						l_first_visible_file_index = 0;
					}
				}

				RenderScreen(l_first_visible_file_index);

				UpdateHorizontalScrollState();
			}
			break;

		case fbs_HorizontalScrollLeftDelay:
			if(sysTimerGetTimeSince(l_horizontal_scroll_timestamp) > FILE_BROWSER_HORIZONTAL_SCROLL_DELAY )
			{
				l_status = fbs_HorizontalScrollLeft;
				l_horizontal_scroll_offset = 0;
				l_horizontal_scroll_timestamp = sysTimerGetTimestamp();
			}
			break;

		case fbs_HorizontalScrollLeft:
			if(sysTimerGetTimeSince(l_horizontal_scroll_timestamp) > FILE_BROWSER_HORIZONTAL_SCROLL_SPEED_DELAY)
			{
				if(l_horizontal_scroll_offset < l_selected_item_horizontal_oversize)
				{
					l_horizontal_scroll_offset++;
					fbRenderItem(fbBufferGetFileInfo(l_current_file_index), l_current_file_index - l_first_visible_file_index, true, l_horizontal_scroll_offset);
				
					fbRefreshScreen();
				}
				else
				{
					l_status = fbs_HorizontalScrollRightDelay;
				}

				l_horizontal_scroll_timestamp = sysTimerGetTimestamp();
			}
			break;

		case fbs_HorizontalScrollRightDelay:
			if(sysTimerGetTimeSince(l_horizontal_scroll_timestamp) > FILE_BROWSER_HORIZONTAL_SCROLL_DELAY )
			{
				l_status = fbs_HorizontalScrollRight;
				l_horizontal_scroll_offset = l_selected_item_horizontal_oversize;
				l_horizontal_scroll_timestamp = sysTimerGetTimestamp();
			}
			break;

		case fbs_HorizontalScrollRight:
			if(sysTimerGetTimeSince(l_horizontal_scroll_timestamp) > FILE_BROWSER_HORIZONTAL_SCROLL_SPEED_DELAY)
			{
				if(l_horizontal_scroll_offset > 0)
				{
					l_horizontal_scroll_offset--;
					fbRenderItem(fbBufferGetFileInfo(l_current_file_index), l_current_file_index - l_first_visible_file_index, true, l_horizontal_scroll_offset);
				
					fbRefreshScreen();
				}
				else
				{
					l_status = fbs_HorizontalScrollLeftDelay;
				}

				l_horizontal_scroll_timestamp = sysTimerGetTimestamp();
			}
			break;
	}
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Gets selected file information
/// @return File information pointer or sysNULL if there is no selected file
fbFileInformation* fbGetSelectedFileInformation(void)
{
	if (l_current_file_index < fbBufferGetFileCount())
		return fbBufferGetFileInfo(l_current_file_index);
	else
		return sysNULL;

}

///////////////////////////////////////////////////////////////////////////////
/// @brief Gets full file name of the selected file
/// @param in_filename_buffer Buffer to receive file name
/// @param in_buffer_length Number of max characters in the file name buffer
void fbGetSelectedFullFileName(sysString in_filename_buffer, sysStringLength in_buffer_length)
{
	if (l_current_file_index < fbBufferGetFileCount())
	{
		// generate full file name
		strCopyString(in_filename_buffer, in_buffer_length, 0, l_current_path);
		fileConcatPath(in_filename_buffer, in_buffer_length, fbBufferGetFileInfo(l_current_file_index)->FileName);
	}
	else
	{
		in_filename_buffer[0] = '\0';
	}
}


///////////////////////////////////////////////////////////////////////////////
/// @brief Gets file browser status
/// @return Current file browser status information
fbStatus fbGetStatus(void)
{
	return l_status;
}


///////////////////////////////////////////////////////////////////////////////
/// @param Updates horizontal scroll state (enables or disables horizontal scroll)
static void UpdateHorizontalScrollState(void)
{
	if(l_selected_item_horizontal_oversize > 0)
	{
		l_status = fbs_HorizontalScrollLeftDelay;
		l_horizontal_scroll_timestamp = sysTimerGetTimestamp();
	}
	else
	{
		l_status = fbs_Active;
	}

	l_horizontal_scroll_offset = 0;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Prepares to show wait indicator (the indicator will be displayed after a short delay)
void fbWaitIndicatorShow(void)
{
	l_wait_indicator_timestamp = sysTimerGetTimestamp();
}

///////////////////////////////////////////////////////////////////////////////
/// @param Shows wait indicator
void fbWaitIndicatorUpdate()
{
	if(l_wait_indicator_visible)
	{
		if(sysTimerGetTimeSince(l_wait_indicator_timestamp) > WAIT_INDICATOR_ANIMATION_DELAY)
		{
			fbRenderWaitIndicatorNext(&l_wait_indicator_phase);
			l_wait_indicator_timestamp = sysTimerGetTimestamp();
		}
	}
	else
	{
		if(sysTimerGetTimeSince(l_wait_indicator_timestamp) > WAIT_INDICATOR_DELAY)
		{
			l_wait_indicator_visible = true;
			fbRenderWaitIndicatorShow();
			l_wait_indicator_phase = 0;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
/// @param Hides wait indicator
void fbWaitIndicatorHide(void)
{
	l_wait_indicator_visible = false;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Renders the whole file browser screen
/// @param in_first_visible_item_index Index of the first visible (top) item 
static void RenderScreen(uint16_t in_first_visible_item_index)
{
	uint16_t visible_item_index;
	uint16_t item_index;
	uint16_t total_visible_item_count = fbRendererGetDisplayableItemCount();
	uint16_t total_item_count = fbBufferGetFileCount();
	uint16_t oversize;

	// render header
	fbRenderHeader((sysString)"",l_current_path);

	// render items
	item_index = in_first_visible_item_index;
	visible_item_index = 0;
	while(visible_item_index < total_visible_item_count && item_index < total_item_count)
	{
		oversize = fbRenderItem(fbBufferGetFileInfo(item_index), visible_item_index, (item_index == l_current_file_index), 0);
		if(item_index == l_current_file_index)
			l_selected_item_horizontal_oversize = oversize;

		item_index++;
		visible_item_index++;
	}

	// clear remaining area (if exists)
	while(visible_item_index < total_visible_item_count)
		fbRenderItem(sysNULL, visible_item_index++, false, 0);

	fbRenderFooter(l_current_file_index+1, fbBufferGetFileCount(), fbBufferGetFileInfo(l_current_file_index));

	fbRefreshScreen();
}
