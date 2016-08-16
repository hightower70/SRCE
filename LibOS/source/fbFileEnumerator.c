/*****************************************************************************/
/* File enumerator                                                           */
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
#include <fbFileEnumerator.h>
#include <fileStandardFunctions.h>

///////////////////////////////////////////////////////////////////////////////
// Module global variables
static fileFindData l_find_handle;

///////////////////////////////////////////////////////////////////////////////
// Local functions
static bool fbFileEnumerationStore(fileFindData* in_file_data);

///////////////////////////////////////////////////////////////////////////////
/// @brief Starts file enumeration and stores the first file
/// @param in_path Directory where files will be enumerated
/// @return true when enumeratiob was successfully started
bool fbFileEnumerationOpen(sysString in_path)
{
	bool success;

	success = fileFindFirstFile(in_path, &l_find_handle);
	if(success && (l_find_handle.File.Attributes & fileFAT_ATTR_VOLUME_ID) == 0)
			success = fbFileEnumerationStore(&l_find_handle);

	return success;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Enumerates next file
/// @return true when file was found, false when no more files can be found 
bool fbFileEnumertationNext(void)
{
	bool success;

	success = fileFindNextFile(&l_find_handle);

	if(success && (l_find_handle.File.Attributes & fileFAT_ATTR_VOLUME_ID) == 0)
		success = fbFileEnumerationStore(&l_find_handle);

	return success;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Closes file enumeration
void fbFileEnumerationClose(void)
{
	fileFindClose(&l_find_handle);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Stores enumerated file information
/// @param in_file_data File information to store
/// @return true if file information was successfully stored, false when sufficent buffer is not available
static bool fbFileEnumerationStore(fileFindData* in_file_data)
{
	fbFileInformation file_information;

	// create file information
	if(in_file_data->File.Attributes & fileFAT_ATTR_DIRECTORY)
  {
		file_information.Size = 0;
		file_information.Flags = fbFF_FOLDER;
  }
  else
  {
		file_information.Size =	in_file_data->File.Size;
		file_information.Flags = fbFF_FILE;
  }

	// copy datetime
	fatGetFileDateTime(&in_file_data->File, &file_information.DateTime);

	// copy name (only pointer, file buffer will physically store the name)
	file_information.FileName = (sysString)in_file_data->FileName;

	return fbBufferStore(&file_information);
}
