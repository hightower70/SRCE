/*****************************************************************************/
/* FAT Function wrappers                                                     */
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
#include <drvMassStorage.h>
#include <fileFAT.h>
#include <fileTypes.h>
#include <fileVolumes.h>
#include <fileStandardFunctions.h>


/*****************************************************************************/
/* Local functions                                                           */
/*****************************************************************************/
static bool filePathToFileHandle(sysString in_path, fatFile* out_file);
static bool fileCompareFileNames(sysString in_filename, sysString in_compare_to_name, sysStringLength in_name_length, bool in_wildcard_allowed);

/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/
static fatFile l_file_handles[fileFAT_MAX_HANDLE_COUNT];
int errno;

///////////////////////////////////////////////////////////////////////////////
// Initializes file system
void fileInit(void)
{
	uint8_t i;
	drvMSInitialize(); // init mass storage driver

	// init file handles
	for(i = 0; i < fileFAT_MAX_HANDLE_COUNT; i++)
	{
		l_file_handles[i].OpenMode = fileFAT_OPEN_MODE_UNUSED;
	}
}

/*****************************************************************************/
/* Drive/folder change functions                                             */
/*****************************************************************************/
#pragma region Drive and folder change functions
// <editor-fold defaultstate="collapsed" desc="Drive and folder change functions">

bool fileChangeCurrentDirectory(sysString in_path)
{
	fatFile file;

	if(filePathToFileHandle(in_path, &file))
	{
		// TODO: handle drive letters
		return fatChangeDirectory(&file);
	}
	else
		return false;
}


// </editor-fold>
#pragma endregion

/*****************************************************************************/
/* Directory enumeration functions                                           */
/*****************************************************************************/
#pragma region Directory enumeration
// <editor-fold defaultstate="collapsed" desc="Directory enumeration">

bool fileFindFirstFile(sysString in_path, fileFindData* in_find_data)
{
	bool success;

	success = filePathToFileHandle(in_path, &in_find_data->File);
	if(success)
	{
		success = fatGetFirstDirectoryEntry(&in_find_data->File);

		if(success)
		{
			if( !fatGetLongFilename(&in_find_data->File, in_find_data->FileName, sizeof(in_find_data->FileName)))
			{
				strCopyString(in_find_data->FileName, sizeof(in_find_data->FileName), 0,  in_find_data->File.ShortName );
			}
		}
	}

	return success;
}

bool fileFindNextFile(fileFindData* in_find_data)
{
	bool success;
	
	success = fatGetNextDirectoryEntry(&in_find_data->File);
	if(success)
	{
		if( !fatGetLongFilename(&in_find_data->File, in_find_data->FileName, sizeof(in_find_data->FileName)))
		{
			strCopyString(in_find_data->FileName, sizeof(in_find_data->FileName), 0,  in_find_data->File.ShortName );
		}
	}
	return success;
}

void fileFindClose(fileFindData* in_find_data)
{
}

// </editor-fold>
#pragma endregion

/*****************************************************************************/
/* Internal helper functions                                                 */
/*****************************************************************************/
#pragma region Internal Helper Functions
// <editor-fold defaultstate="collapsed" desc="Internal Helper Functions">

///////////////////////////////////////////////////////////////////////////////
/// @brief Converts file path to fatFile information. 
/// @return Pointer to unsused fatFile struct
static bool filePathToFileHandle(sysString in_path, fatFile* out_file)
{
	sysChar volume_letter;
	sysStringLength pos;
	uint8_t volume_index;
	fileVolumeInfo* volume_info;
	bool status;
	sysStringLength segment_length;
	bool last_segment;
	bool found;
	sysChar long_file_name_buffer[fileMAX_PATH];
	
	// sanity check
	if(in_path[0] == '\0' || out_file == sysNULL)
		return false;

	// check for drive letter and get volume information
	pos = 0;
	if(in_path[1] == fileDRIVE_LETTER_SEPARATOR)
	{
		volume_letter = strCharToUpper(in_path[0]);
		volume_index = fileGetVolumeIndexFromDriveLetter(volume_letter);

		pos = 2; // drive letter + separator
	}
	else
	{
		volume_index = fileVOLUMES_GET_CURRENT_VOLUME_INDEX();
	}

	if(volume_index == fileVOLUMES_INVALID_INDEX)
		return false;

	// start path parsing
	volume_info = fileVOLUMES_GET_VOLUME_INFO_FROM_INDEX(volume_index);
	fileVOLUMES_SET_FILE_VOLUME_INDEX(out_file,volume_index);

	// check if path starts from the root
	if(in_path[pos] == filePATH_SEPARATOR)
	{
		// get root directory
		out_file->ShortName[0] = filePATH_SEPARATOR;
		out_file->ShortName[1] = '\0';
		if( !fatChangeDirectory(out_file))
			return false;

		pos++;
	}
	else
	{
		// get current directory
		*out_file = volume_info->FATVolumeInfo.CurrentDirectory;
	}

	// process whole path
	while(in_path[pos] != '\0')
	{
		// find the length of the current path segment
		segment_length = 0;
		while(in_path[pos+segment_length] != '\0' && in_path[pos+segment_length] != filePATH_SEPARATOR)
			segment_length++;

		last_segment = (in_path[pos+segment_length] == '\0');

		// find files in folder until match occures
		found = false;
		status = fatGetFirstDirectoryEntry(out_file);
		while(status && !found)
		{
			// compare
			found = fileCompareFileNames(out_file->ShortName, &in_path[pos], segment_length, last_segment);

 			if(!found)
			{
				if(fatGetLongFilename(out_file, long_file_name_buffer, fileMAX_PATH))
					found = fileCompareFileNames(long_file_name_buffer, &in_path[pos], segment_length, last_segment);
			}

			if(!found)
				status = fatGetNextDirectoryEntry(out_file);
		}

		if(found)
		{
			pos += segment_length;

			if(!last_segment)
				pos++; // skip separator
		}
		else
			return false;
	}

	return true;
}


///////////////////////////////////////////////////////////////////////////////
/// @brief Gets the first unused file structure or returns null if there are no more handles. @see fileMAX_HANDLE_COUNT to the maximum number of handles.
/// @return Pointer to unsused fatFile struct
static fatFile* fileGetFileHandle(void)
{
	uint8_t i;

	i = 0;

	for(i=0; i < fileFAT_MAX_HANDLE_COUNT; i++)
	{
		if(l_file_handles[i].OpenMode == fileFAT_OPEN_MODE_UNUSED)
			return &l_file_handles[i];
	}

	return sysNULL;
}

static bool fileCompareFileNames(sysString in_filename, sysString in_compare_to_name, sysStringLength in_name_length, bool in_wildcard_allowed)
{
	sysStringLength pos;

	pos = 0;
	while(pos < in_name_length)
	{
		if(in_filename[pos] == '\0' || (strCharToUpper(in_filename[pos]) != strCharToUpper(in_compare_to_name[pos])))
			return false;

		pos++;
	}

	return in_filename[pos] == '\0';
}

#if 0
//**************************************
// Name: ^NEW^ -- wildcard string compare (globbing)
// Description:matches a string against a wildcard string such as "*.*" or "bl?h.*" etc. This is good for file globbing or to match hostmasks.
// By: Jack Handy
//
// Returns:1 on match, 0 on no match.
//
//This code is copyrighted and has// limited warranties.Please see http://www.Planet-Source-Code.com/vb/scripts/ShowCode.asp?txtCodeId=1680&lngWId=3//for details.//**************************************

int wildcmp(char *wild, char *string) {
	char *cp=NULL, *mp=NULL;
 while ((*string) && (*wild != '*')) {
 if ((*wild != *string) && (*wild != '?')) {
return 0;
 }
 wild++;
 string++;
 }
 while (*string) {
 if (*wild == '*') {
if (!*++wild) {
return 1;
}
mp = wild;
cp = string+1;
 } else if ((*wild == *string) || (*wild == '?')) {
wild++;
string++;
 } else {
wild = mp;
string = cp++;
 }
 }
 while (*wild == '*') {
 wild++;
 }
 return !*wild;
}
#endif

// </editor-fold>
#pragma endregion

