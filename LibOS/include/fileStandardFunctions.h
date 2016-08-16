/*****************************************************************************/
/* Standard (stdio like) file access functions                               */
/*                                                                           */
/* Copyright (C) 2015 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

#ifndef __fileStandardFunctions_h
#define __fileStandardFunctions_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysConfig.h>
#ifdef fileUSE_STANDARD_FILE_SYSTEM
#define _CRT_SECURE_NO_WARNINGS 1
#include <stdio.h>
#endif
#include <fileFAT.h>


/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/
// Path length
#ifndef fileMAX_PATH
#define fileMAX_PATH 256
#endif

/*****************************************************************************/
/* Types                                                                     */
/*****************************************************************************/
#ifdef fileUSE_STANDARD_FILE_SYSTEM
typedef FILE fileStream;
typedef size_t fileSize;
#else
typedef fatFile fileStream;
typedef uint32_t fileSize;
#endif

// Data struct for file enumeration
typedef struct 
{
	fileStream File;
	sysChar FileName[fileFAT_MAX_FILENAME_LENGTH+1]; // +1 for the zero terminator
} fileFindData;
	
/*****************************************************************************/
/* Function prototypes                                                       */
/*****************************************************************************/

// File handling functions
#ifdef fileUSE_STANDARD_FILE_SYSTEM
#define fileInit()

#define fileOpen(in_path, in_param) fopen(in_path,in_param)
#define fileClose(in_stream) fclose(in_stream)
#define fileRead(in_buffer, in_element_size, in_element_count, in_stream) fread(in_buffer, in_element_size, in_element_count, in_stream)
#define fileWrite(in_buffer, in_element_size, in_element_count, in_stream) fwrite(in_buffer, in_element_size, in_element_count, in_stream)

#define fileGetPosition(in_stream) ftell(in_stream)
#define fileSetPosition(in_file, in_offset, in_origin) fseek(in_file, in_offset, in_origin)

#define fileEOF(in_file) feof(in_file)
#else
void fileInit(void);

fileStream* fileOpen(sysString in_path, sysString in_param);
void fileClose(fileStream in_stream);
fileSize fileRead(void* in_buffer, fileSize in_element_size, fileSize in_element_count, fileStream* in_stream);

fileSize fileGetPosition(fileStream* in_file);
void fileSetPosition(fileStream* in_file, fileSize in_offset, int in_origin);

bool fileEOF(fileStream* in_file);
#endif

// file enumeration functions
bool fileFindFirstFile(sysString in_path, fileFindData* in_find_data);
bool fileFindNextFile(fileFindData* in_find_data);
void fileFindClose(fileFindData* in_find_data);

// other high level file handling functions
bool fileChangeCurrentDirectory(sysString in_path);

#endif