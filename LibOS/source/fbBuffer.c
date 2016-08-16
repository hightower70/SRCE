/*****************************************************************************/
/* File browser application                                                  */
/*   File information buffer implementation                                  */
/*                                                                           */
/* Copyright (C) 2014 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
// Includes
#include <fbBuffer.h>

///////////////////////////////////////////////////////////////////////////////
// Module global variables
static uint8_t* l_buffer;
static uint32_t l_buffer_size;
static uint16_t l_file_count;
static sysChar* l_file_name_pos;
static fbFileInformation* l_file_info_pos;
static uint8_t l_sort_mode = fbSM_ASCENDING | fbSM_NAME;
static uint16_t l_gap;

///////////////////////////////////////////////////////////////////////////////
/// @brief Sets the memory used for file information buffer
/// @param in_buffer Pointer to the memory area
/// @param in_buffer_size Size of the memory in bytes
void fbBufferSet(uint8_t* in_buffer, uint32_t in_buffer_size)
{
	l_buffer = in_buffer;
	l_buffer_size = in_buffer_size;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Sets sort mode (must be called before sorting the buffer content)
/// @param in_sort_mode Sort mode See FB_SM_* for sort modes
void fbBufferSetSortMode(uint8_t in_sort_mode)
{
	l_sort_mode = in_sort_mode;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Gets number of files stored in the buffer
uint16_t fbBufferGetFileCount(void)
{
	return l_file_count;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Finds default file index. Returns invalid index when there is no default file
/// @return Index of the default file or invalid index of no default file found.
uint16_t fbGetDefaultFileIndex(void)
{
	uint16_t index = 0;

	while(index < l_file_count)
	{
		if((l_file_info_pos[index].Flags & fbFF_DEFAULT) != 0)
			return index;

		index++;
	}

	return fbINVALID_INDEX;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Gets file information at the selected index
/// @param in_index Index of the file information
/// @return Pointer to the File information struct or NULL is index is invalid
fbFileInformation* fbBufferGetFileInfo(uint16_t in_index)
{
	if(in_index >= l_file_count)
		return sysNULL;

	return &l_file_info_pos[in_index];
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Clears buffer content
void fbBufferClear(void)
{
	l_file_count = 0;
	l_file_name_pos = (sysChar*)l_buffer;
	l_file_info_pos = (fbFileInformation*)(l_buffer + l_buffer_size);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Stores file information in the buffer. Adds square barckets to the file name if directory was stored.
/// @param in_file_info File information struct to store
/// @return True if it was success, false if there is no more space for storing
bool fbBufferStore(fbFileInformation* in_file_info)
{
	//sysStringLength length;
	int32_t buffer_free;
	sysString filename;
	fbFileInformation* default_file_information = sysNULL;
	sysStringLength pos;

	// do not store actual directory
	if(strCompareConstString(in_file_info->FileName, (sysConstString)".")	== 0)
		return true;

	// check for default directory flag
	if(l_file_count > 0 && (((fbFileInformation*)(l_buffer + l_buffer_size - sizeof(fbFileInformation)))->Flags & fbFF_DEFAULT) != 0 )
	{
		default_file_information = ((fbFileInformation*)(l_buffer + l_buffer_size - sizeof(fbFileInformation)));
		if(strCompareString(default_file_information->FileName, in_file_info->FileName) == 0)
		{
			filename = default_file_information->FileName;
			*default_file_information = *in_file_info;
			default_file_information->FileName = filename;
			default_file_information->Flags |= fbFF_DEFAULT;

			return true;
		}
	}

	// calculate remaining buffer size
	buffer_free = (int32_t)(l_buffer_size - l_file_count * sizeof(fbFileInformation) - ((uint8_t*)l_file_name_pos - (uint8_t*)l_buffer));

	if(buffer_free < sizeof(fbFileInformation))
		return false;

	// get storage size for file name
	if(buffer_free > sysSTRING_MAX_LENGTH)
		buffer_free = sysSTRING_MAX_LENGTH;

	pos = strCopyString(l_file_name_pos, sysSTRING_MAX_LENGTH, 0, in_file_info->FileName) + 1; // +1 because of the terminating zero

	// check free space
	buffer_free -= pos;
	if(buffer_free < sizeof(fbFileInformation))
		return false;

	// store file info
	l_file_info_pos--;

	*l_file_info_pos = *in_file_info;
	l_file_info_pos->FileName = l_file_name_pos;
	l_file_name_pos += pos;

	// check for memory overflow
	if((uint8_t*)l_file_info_pos <= (uint8_t*)l_file_name_pos)
		return false;

	l_file_count++;

	return true;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Initializes sorting. Must be called before calling sort function.
void fbBufferSortStart(void)
{
	l_gap = l_file_count;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Sorts buffer content based on the selected sort condition. The function must be called periodically until it returns true. 
/// @return True if sort is finished false when the function must be called again.
/// @see fbBufferSetSortMode for details
bool fbBufferSort(void)
{
	fbFileInformation temp;
	bool swapped = false;
	uint16_t index;
	int8_t result;

	// Start a "Comb Sort"

	//reduce gap
	if (l_gap > 1)
		l_gap = l_gap * 10 / 13;

	// sort data
	for (index = 0; index + l_gap < l_file_count; index++) 
	{
		// folder must be at the beginning of the list so compare differently based on the entry type
		if((l_file_info_pos[index].Flags & BV(fbFF_FOLDER_FILE_BIT)) == fbFF_FILE && (l_file_info_pos[index+l_gap].Flags & BV(fbFF_FOLDER_FILE_BIT)) == fbFF_FILE)
		{
			// compare file entries
			switch(l_sort_mode & fbSM_TYPE_MASK)
			{
				case fbSM_NAME:
					result = (int8_t)strCompareStringNoCase(l_file_info_pos[index].FileName, l_file_info_pos[index+l_gap].FileName);
					break;

				case fbSM_DATE:
					result = (int8_t)sysCompareDateTime(&l_file_info_pos[index].DateTime, &l_file_info_pos[index+l_gap].DateTime);
					break;

				case fbSM_SIZE:
					result = (int8_t)(l_file_info_pos[index].Size < l_file_info_pos[index+l_gap].Size) ? -1 : (l_file_info_pos[index].Size > l_file_info_pos[index+l_gap].Size);
					break;
			}
		}
		else
		{
			// compare folder entries (at least one entry is folder)

			// if both folder -> sort by name
			if((l_file_info_pos[index].Flags & BV(fbFF_FOLDER_FILE_BIT)) == fbFF_FOLDER && (l_file_info_pos[index+l_gap].Flags & BV(fbFF_FOLDER_FILE_BIT)) == fbFF_FOLDER)
			{
				// check for parent folder
				if(strCompareConstString(l_file_info_pos[index].FileName, (sysConstString)"..") == 0)
					result = -1;
				else
				{
					if(strCompareConstString(l_file_info_pos[index+l_gap].FileName, (sysConstString)"..") == 0)
						result = 1;
					else
						result = (int8_t)strCompareStringNoCase(l_file_info_pos[index].FileName, l_file_info_pos[index+l_gap].FileName);
				}
			}
			else
			{
				// if one is folder -> put folder first
				if((l_file_info_pos[index].Flags & BV(fbFF_FOLDER_FILE_BIT)) == fbFF_FOLDER)
				{
					result = -1;
				}
				else
				{
					result = 1;
				}

				// compensate sort order -> folder always must be at the beginning
				if((l_sort_mode & fbSM_DIRECTION_MASK) != 0)
					result = -result;
			}
		}

		// reverse direction
		if((l_sort_mode & fbSM_DIRECTION_MASK) != 0)
			result = -result;

		// swap elements
		if(result > 0)
		{
			temp = l_file_info_pos[index];
			l_file_info_pos[index] = l_file_info_pos[index + l_gap];
			l_file_info_pos[index + l_gap] = temp;
			swapped = true;
		}
	}

	return (l_gap == 1 && !swapped);
}
