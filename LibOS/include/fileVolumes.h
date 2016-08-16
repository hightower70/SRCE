/*****************************************************************************/
/*                                                                           */
/*    Domino Operation System Driver Module                                  */
/*    File System Routines                                                   */
/*                                                                           */
/*    Copyright (C) 2006-2014 Laszlo Arvai                                   */
/*                                                                           */
/*    ------------------------------------------------------------------   */
/*    Drives and Volumes handling routines                                   */
/*****************************************************************************/

#ifndef __fileVolumes_h
#define __fileVolumes_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysTypes.h>
#include <fileFAT.h>
#include "sysConfig.h"

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/
#define fileVOLUMES_INVALID_INDEX 0xff

/*****************************************************************************/
/* Types                                                                     */
/*****************************************************************************/
//typedef struct
//{
//}	fileDriveInfo;

typedef struct
{
	uint8_t DriveIndex;
	uint8_t FileSystemType;
	union
	{
		fatVolumeInfo FATVolumeInfo;
	};
} fileVolumeInfo;


/*****************************************************************************/
/* Global variables                                                          */
/*****************************************************************************/

/* Volume info */
#ifndef fileFAT_MULTI_VOLUME_SUPPORT
extern fileVolumeInfo g_file_volume_info;
#endif

/*****************************************************************************/
/* Volume access macros                                                      */
/*****************************************************************************/

#ifdef fileVOLUMES_MULTI_VOLUME_SUPPORT
#define fileGET_CURRENT_VOLUME_INDEX() todo
#define fileREAD_VOLUME_SECTOR(x)
#define fileWRITE_VOLUME_SECTOR(x)

//#define fileREAD_VOLUME_SECTOR(volume,buffer,address) drvMSReadSector(buffer,address)
//#define fileWRITE_VOLUME_SECTOR(volume,buffer,address) drvMSWriteSector(buffer,address)
#else

#define fileVOLUMES_GET_CURRENT_VOLUME_INDEX() (0)
#define fileVOLUMES_GET_VOLUME_INFO_FROM_INDEX(index) (&g_file_volume_info)
#define fileVOLUMES_GET_VOLUME_INDEX_FROM_DRIVE(drive) (0)

#define fileVOLUMES_GET_VOLUME_INDEX_FROM_FILE(file)  (0)
#define fileVOLUMES_SET_FILE_VOLUME_INDEX(file,index) // do nothing when multi volume is not supported
#endif
///////////////////////////////////////////////////////////////////////////////
// Function prototypes
uint8_t fileGetVolumeIndexFromDriveLetter(sysChar in_drive_letter);
bool fileReadSector( uint8_t in_volume_index, uint8_t* out_buffer, uint32_t in_lba );
bool fileWriteSector( uint8_t in_volume_index, uint8_t* in_buffer, uint32_t in_lba );



#endif