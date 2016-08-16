/*****************************************************************************/
/* Basic FAT (12,16,32) handling functions                                   */
/*                                                                           */
/* Copyright (C) 2014-2015 Laszlo Arvai                                      */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

#ifndef __fileFAT_h
#define __fileFAT_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysTypes.h>
#include <sysDateTime.h>
#include "sysConfig.h"

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/
#define fileFAT_MAX_SHORT_FILENAME_LENGTH			12
#define fileFAT_MAX_FAT_FILENAME_LENGTH				11
#define fileFAT_MAX_LFN_LENGTH								256
#define fileFAT_SECTOR_LENGTH									512

// FAT type
#define fileFAT_TYPE_FAT12 1
#define fileFAT_TYPE_FAT16 2
#define fileFAT_TYPE_FAT32 3

// Open mode
#define fileFAT_OPEN_MODE_UNUSED		0x00
#define fileFAT_OPEN_MODE_CLOSED		0x01
#define fileFAT_OPEN_MODE_READONLY	0x02
#define fileFAT_OPEN_MODE_READWRITE	0x03

// attributes
#define fileFAT_ATTR_READ_ONLY	0x01
#define fileFAT_ATTR_HIDDEN			0x02
#define fileFAT_ATTR_SYSTEM			0x04
#define fileFAT_ATTR_VOLUME_ID	0x08
#define fileFAT_ATTR_DIRECTORY	0x10
#define fileFAT_ATTR_ARCHIVE		0x20
#define fileFAT_ATTR_LONG_NAME (fileFAT_ATTR_READ_ONLY | \
														fileFAT_ATTR_HIDDEN | \
														fileFAT_ATTR_SYSTEM | \
														fileFAT_ATTR_VOLUME_ID)

#define fileFAT_ATTR_LONG_NAME_MASK		(fileFAT_ATTR_READ_ONLY | \
																	fileFAT_ATTR_HIDDEN | \
																	fileFAT_ATTR_SYSTEM | \
																	fileFAT_ATTR_VOLUME_ID | \
																	fileFAT_ATTR_DIRECTORY | \
																	fileFAT_ATTR_ARCHIVE )

// buffering mode
#define fileFAT_BM_SINGLE										1	// only one buffer is used
#define fileFAT_BM_SYSTEM_AND_FILE					2 // separated buffer is used for directory/fat and file data
#define fileFAT_BM_SYSTEM_AND_PER_FILE			3 // one buffer is used for directory/fat and all files have separated buffer

// if not defined then set default buffering mode
#ifndef fileFAT_BUFFERING_MODE
#define fileFAT_BUFFERING_MODE fileFAT_BM_SINGLE
#endif

#ifndef fileFAT_NO_LONG_FILENAME_SUPPORT
#define fileFAT_MAX_FILENAME_LENGTH fileFAT_MAX_LFN_LENGTH
#else
#define fileFAT_MAX_FILENAME_LENGTH fileFAT_MAX_SHORT_FILENAME_LENGTH
#endif

/*****************************************************************************/
/* Macros                                                                    */
/*****************************************************************************/


/*****************************************************************************/
/* Types                                                                     */
/*****************************************************************************/

// Define cluster address type
#ifndef fileFAT_NO_FAT32_SUPPORT
typedef uint32_t fatClusterAddress;	/// Cluster address type
#else
typedef uint16_t fatClusterAddress;	/// Cluster address type
#endif

// Sector data buffer
struct _fatSectorBuffer
{
	uint8_t Buffer[fileFAT_SECTOR_LENGTH];
	uint32_t LBA;
	#ifndef fileREAD_ONLY_FILESYSTEM
	bool Modified;
	#endif
};
typedef struct _fatSectorBuffer fatSectorBuffer;

// File information
struct _fatFile
{
	// file properties
	sysChar ShortName[fileFAT_MAX_FAT_FILENAME_LENGTH+1];
	uint8_t Attributes;
	uint32_t Size;
	uint8_t OpenMode;

	// start cluster
	fatClusterAddress StartCluster;

	// current file position
	uint32_t CurrentPos;
	fatClusterAddress CurrentCluster;
	uint16_t CurrentSector;
	uint16_t CurrentSectorPos;

	// directory enrty position
	fatClusterAddress DirectoryCluster;
	uint16_t DirectorySector;
	uint8_t DirectoryEntryIndex;

	#if fileFAT_BUFFERING_MODE == fileFAT_BM_SYSTEM_AND_PER_FILE
	fatSectorBuffer* DataBuffer;
	#endif

	#if fileFAT_MULTI_DRIVE_SUPPORT
	uint8_t VolumeIndex;
	#endif
};
typedef struct _fatFile fatFile;

typedef struct
{
	uint32_t VolumeStartLBA;							// LBA address of the sector where volume starts
	uint8_t FATType;											// Type of the FAT (FAT12, FAT16, FAT32)
	uint8_t SectorsPerClaster;						// Size of one claster in sectors
	uint16_t FirstFATStart;								// Sector address of the first FAT table (relative to the volume start)
	uint16_t SecondFATStart;							// Sector address of the second FAT table (relative to the volume start)
	uint16_t RootDirectoryStart;					// Sector address (FAT12, FAT16) of cluster address (FAT32) of the root directory
	uint16_t FirstDataSector;							// Sector address of the first data sector
	fatClusterAddress DataClasterCount;		// Number of data cluster count
	fatSectorBuffer SystemSectorBuffer;		// Sector buffer for system (directory, fat, boot) sectory
	sysChar DriveLetter;									// Assigned drive letter
	fatFile CurrentDirectory;							// File information about the current directory
} fatVolumeInfo;


/*****************************************************************************/
/* Function prototypes                                                       */
/*****************************************************************************/

bool fatMountDrive(uint8_t in_drive_index);
void fatUnmountDrive(uint8_t in_drive_index);

void fileSetDriveLetterByVolumeIndex(uint8_t in_volume_index, sysChar in_drive_letter);

bool fatGetFirstDirectoryEntry( fatFile* in_find_data );
bool fatGetNextDirectoryEntry( fatFile* in_find_data );
void fatGetFileDateTime( fatFile* in_file, sysDateTime* out_datetime );
bool fatChangeDirectory( fatFile* in_file );
uint16_t fatGetNumberOfEntries( fatFile* in_file );

bool fatOpen( fatFile* in_file, uint8_t in_open_flags );
bool fatIsEof( fatFile* in_file );
uint16_t fatRead( fatFile* in_file, uint8_t* out_buffer, uint16_t in_buffer_size );
void fatClose( fatFile* in_file );
void fatFlushBuffer(void);
bool fatIsFileExists( fatFile* in_directory, fatFile* in_file );

#ifndef fileFAT_READ_ONLY_FILESYSTEM
bool fatCreateFile( fatFile* in_directory, fatFile* in_file );
bool fatDeleteFile( fatFile* in_file );
void fatSetFileDateTime( fatFile* in_file, sysDateTime* out_datetime );
uint16_t fatWrite( fatFile* in_file, uint8_t* in_buffer, uint16_t in_buffer_size );
#endif

// long filename support routines
bool fatGetLongFilename( fatFile* in_file, sysString out_buffer, uint16_t in_buffer_length );

#ifndef fileREAD_ONLY_FILESYSTEM
bool fatSetLongFilename( fatFile* in_file, sysString in_long_name );
#endif

#endif