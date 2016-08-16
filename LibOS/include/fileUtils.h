#ifndef __fileUtils_h
#define __fileUtils_h

///////////////////////////////////////////////////////////////////////////////
// Includes
#include <sysString.h>
#include <fileTypes.h>

#define fileUTIL_MAX_FILENAME_LENGTH 256

///////////////////////////////////////////////////////////////////////////////
// Function prototypes
sysString fileGetFileNameExtension(sysString in_file_name);
void fileConcatPath(sysString in_path1, sysStringLength in_path1_length, sysString in_path2);
void fileSizeToString(sysString in_buffer, sysStringLength in_buffer_length, uint32_t in_size);
bool fileShortenDisplayPath(sysString in_buffer, sysStringLength in_buffer_length);


#endif