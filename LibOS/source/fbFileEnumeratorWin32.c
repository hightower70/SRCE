///////////////////////////////////////////////////////////////////////////////
// Includes
#include <windows.h>
#include <strsafe.h>
#include <fbBuffer.h>
#include <fbFileEnumerator.h>

///////////////////////////////////////////////////////////////////////////////
// Module global variables
static HANDLE l_find_handle = INVALID_HANDLE_VALUE;

///////////////////////////////////////////////////////////////////////////////
// Local functions
static bool fbFileEnumerationStore(WIN32_FIND_DATA* in_file_data);

///////////////////////////////////////////////////////////////////////////////
// Start enumeration
bool fbFileEnumerationOpen(sysString in_path)
{
	WIN32_FIND_DATA find_data;
	size_t length_of_path;
	TCHAR path_buffer[MAX_PATH];

	// check path length
	StringCchLength((char*)in_path, MAX_PATH, &length_of_path);
  if (length_of_path > (MAX_PATH - 3))
  {
		return false;
	}
	
	StringCchCopy(path_buffer, MAX_PATH, (char*)in_path);
	StringCchCat(path_buffer, MAX_PATH, TEXT("\\*"));

  // Find the first file in the directory.
  l_find_handle = FindFirstFile(path_buffer, &find_data);
  if (l_find_handle  == INVALID_HANDLE_VALUE) 
		return false;

	return fbFileEnumerationStore(&find_data);
}

///////////////////////////////////////////////////////////////////////////////
// Eumerates next file
bool fbFileEnumertationNext(void)
{
	WIN32_FIND_DATA find_data;

	if(FindNextFile(l_find_handle, &find_data))
		return fbFileEnumerationStore(&find_data);

	return false;
}

///////////////////////////////////////////////////////////////////////////////
// Closes ile enumeration
void fbFileEnumerationClose(void)
{
	FindClose(l_find_handle);
}

///////////////////////////////////////////////////////////////////////////////
// Stores enumerated file information
static bool fbFileEnumerationStore(WIN32_FIND_DATA* in_file_data)
{
	fbFileInformation file_information;
	SYSTEMTIME system_time;

	// create file information
	if(in_file_data->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
  {
		file_information.Size = 0;
		file_information.Flags = fbFF_FOLDER;
  }
  else
  {
		file_information.Size =	in_file_data->nFileSizeLow;
		file_information.Flags = fbFF_FILE;
  }

	// copy datetime
	FileTimeToSystemTime(&(in_file_data->ftLastWriteTime), &system_time);
	file_information.DateTime.Year = system_time.wYear;
	file_information.DateTime.Month = (uint8_t)system_time.wMonth;
	file_information.DateTime.Day = (uint8_t)system_time.wDay;
	file_information.DateTime.DayOfWeek = (uint8_t)system_time.wDayOfWeek;

	file_information.DateTime.Hour = (uint8_t)system_time.wHour;
	file_information.DateTime.Minute = (uint8_t)system_time.wMinute;
	file_information.DateTime.Second = (uint8_t)system_time.wSecond;

	// copy name
	file_information.FileName = (sysString)in_file_data->cFileName;

	return fbBufferStore(&file_information);
}