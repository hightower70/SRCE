/*****************************************************************************/
/* Basic FAT (12,16,32) handling functions                                   */
/*                                                                           */
/* Copyright (C) 2014-2015 Laszlo Arvai                                      */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
//#include <stdlib.h>
#include <sysTypes.h>
#include <sysString.h>
#include <fileVolumes.h>
#include <fileFAT.h>
#include <fileTypes.h>

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/
#define fileFAT_DIRECTORY_ENTRY_LENGTH 32
#define fileFAT_EOC 1
#define fileFAT_FREE 0

#define fileFAT_GOOD_SIGN_0     0x55	 // boot sector or MBR signature first byte
#define fileFAT_GOOD_SIGN_1     0xAA	 // boot sector or MBR signature second byte

#define fileFAT_MBR_LBA 0	// LBA of the MBR sector
#define fileFAT_INVALID_LBA 0xffffffff
#define fileFAT_NUMBER_OF_FAT_TABLES	2


// boot sector element indices

#define fileFAT_BSI_BPS						 11 // bytes per sector
#define fileFAT_BSI_SPC						 13 // sector per cluster
#define fileFAT_BSI_RESRVSEC			 14 // reserved sector count 
#define fileFAT_BSI_FATCOUNT       16 // boot sector FAT count 
#define fileFAT_BSI_ROOTDIRENTS    17 // root directory entry count 
#define fileFAT_BSI_TOTSEC16       19	// 16-bit total sector count 
#define fileFAT_BSI_SPF						 22 // sectors per FAT 
#define fileFAT_BSI_TOTSEC32			 32 // 32-bit total sector count 
#define fileFAT_BSI_FATSZ32				 36 // 32-bit sector per FAT 
#define fileFAT_BSI_BOOTSIG				 38	// boot signature
#define fileFAT_BSI_ROOT_DIR_CLUS	 44 // Cluster number of the root directory
#define fileFAT_BSI_FSTYPE				 54	// file system type string
#define fileFAT_BSI_FAT32_FSTYPE	 82 // FAT32 file system type string
#define	fileFAT_BSI_FAT32_BOOTSIG	 66	// FAT32 boot signature
#define fileFAT_BSI_SIGN_0				510	// signature first byte
#define fileFAT_BSI_SIGN_1				511	// signature second byte

// master boot resord element indices
#define fileFAT_MBRI_PARTITION_TABLE 446 // partition table first entry

// partition table entry positions
#define fileFAT_PTEI_FIRST_SECTOR			1 // CHS address of the first sector
#define fileFAT_PTEI_FILE_SYSTEM_TYPE 4 // file system type address
#define fileFAT_PTEI_FIRST_SECTOR_LBA	8 // LBA of the first sector

#define fileFAT_PARTITION_ENTRY_LENGTH 16

/*****************************************************************************/
/* Module local variables                                                    */
/*****************************************************************************/

/* Sector buffers */
// single buffer mode
#if fileFAT_BUFFERING_MODE == fileFAT_BM_SINGLE
static fatSectorBuffer l_sector_buffer;
#define fileFAT_SYSTEM_SECTOR_BUFFER l_sector_buffer
#define fileFAT_DATA_SECTOR_BUFFER(x) l_sector_buffer

// separated system and file buffer
#elif fileFAT_BUFFERING_MODE == fileFAT_BM_SYSTEM_AND_FILE
static fatSectorBuffer l_system_sector_buffer;
static fatSectorBuffer l_data_sector_buffer;
#define fileFAT_SYSTEM_SECTOR_BUFFER l_system_sector_buffer
#define fileFAT_DATA_SECTOR_BUFFER(x) l_data_sector_buffer

#else
#error Invalid buffer mode
#endif

/* Volume info */
#ifndef fileFAT_MULTI_VOLUME_SUPPORT
fileVolumeInfo g_file_volume_info;
#endif


/*****************************************************************************/
/* Local function prototypes                                                 */
/*****************************************************************************/
static uint8_t fatGetSystemSectorByte(uint16_t in_byte_index);
static uint16_t fatGetSystemSectorWord(uint16_t in_byte_index);
static uint32_t fatGetSystemSectorDWord(uint16_t in_byte_index);

static bool fatCacheVolumeInformation(fileVolumeInfo* out_volume_info);

static bool fatReadSystemSector( uint8_t in_volume_index, uint32_t in_lba );
static void fatFlushSystemSector(uint8_t in_volume_index);
static bool fatReadDirectorySector( fatFile* in_file );
static bool fatReadSystemSectorOfCluster( uint8_t in_volume_index, fatClusterAddress in_cluster_address, uint8_t in_sector );
static fatClusterAddress fatReadFATEntry( uint8_t in_volume_index, fatClusterAddress in_cluster_address );

static bool fatReadDataSector( fatFile* in_file );
static void fatFlushDataSector( fatFile* in_file );

//static bool fatIntReadSectorOfCluster( fatClusterAddress in_cluster, uint8_t in_sector );

//static bool fatIntReadDirectorySector( fatFile* in_file );
static bool fatReadDirectoryEntry( fatFile* in_file );

static bool fatCovertFilenameToFATFilename( fatFile* in_file, uint8_t* out_filename );
static bool fatIsValidFilenameCharacter( char in_char );
static void fatGenerateShortFilename( fatFile* in_file, sysString in_long_name );

static fatClusterAddress fatIntPrepareNextClusterForWrite( fatClusterAddress in_cluster );

//static fatClusterAddress fatIntReadFATEntry( fatClusterAddress in_cluster_address );

#ifndef fileFAT_READ_ONLY_FILESYSTEM
static void fatIntWriteFATEntry( fatClusterAddress in_cluster_address, fatClusterAddress in_new_value  );
static fatClusterAddress fatIntGetFreeCluster(void);
#endif

#ifndef fileFAT_NO_LONG_FILENAME_SUPPORT
static bool fatIntSkipLongFilenameEntries( fatFile* in_file );
#endif

/*****************************************************************************/
/* FAT Volume handling routines                                              */
/*****************************************************************************/
#pragma region FAT Mount and Unmount routines
// <editor-fold defaultstate="collapsed" desc="FAT Mount and Unmount routines">

///////////////////////////////////////////////////////////////////////////////
//! Mount file system
//! \return true if file system is valid and mounted
bool fatMountDrive(uint8_t in_drive_index)
{
	bool success;
	uint8_t type;
	uint8_t i;
	uint8_t volume_index = fileVOLUMES_GET_VOLUME_INDEX_FROM_DRIVE(in_drive_index);
	fileVolumeInfo* volume_info = fileVOLUMES_GET_VOLUME_INFO_FROM_INDEX(in_volume_index);
	uint16_t partition_entry_pos;

	// load first sector
	fileFAT_SYSTEM_SECTOR_BUFFER.LBA = fileFAT_INVALID_LBA;
	success = fatReadSystemSector(volume_index, fileFAT_MBR_LBA);

	// determine if it's the master boot record or boot record

	// check signature
	if (!success ||
		fatGetSystemSectorByte(fileFAT_BSI_SIGN_0) != fileFAT_GOOD_SIGN_0 ||
		fatGetSystemSectorByte(fileFAT_BSI_SIGN_1) != fileFAT_GOOD_SIGN_1)
		return false;

	if (	fatGetSystemSectorByte(fileFAT_BSI_FSTYPE)		 == 'F' &&
				fatGetSystemSectorByte(fileFAT_BSI_FSTYPE + 1) == 'A' &&
				fatGetSystemSectorByte(fileFAT_BSI_FSTYPE + 2) == 'T' &&
				fatGetSystemSectorByte(fileFAT_BSI_FSTYPE + 3) == '1' &&
				fatGetSystemSectorByte(fileFAT_BSI_BOOTSIG) == 0x29)
	{
		volume_info->FATVolumeInfo.VolumeStartLBA = 0;
		success = fatCacheVolumeInformation(volume_info);
	}
	else
	{
		if (	fatGetSystemSectorByte(fileFAT_BSI_FAT32_FSTYPE)	   == 'F' &&
					fatGetSystemSectorByte(fileFAT_BSI_FAT32_FSTYPE + 1) == 'A' &&
					fatGetSystemSectorByte(fileFAT_BSI_FAT32_FSTYPE + 2) == 'T' &&
					fatGetSystemSectorByte(fileFAT_BSI_FAT32_FSTYPE + 3) == '3' &&
					fatGetSystemSectorByte(fileFAT_BSI_FAT32_BOOTSIG) == 0x29)
		{
			volume_info->FATVolumeInfo.VolumeStartLBA = 0;
			success = fatCacheVolumeInformation(volume_info);
		}
		else
		{
			// first sector is MBR
			partition_entry_pos = fileFAT_MBRI_PARTITION_TABLE;
			for (i = 0; i < 4; i++)
			{
				// check if the partition type is acceptable
				type = fatGetSystemSectorByte(partition_entry_pos + fileFAT_PTEI_FILE_SYSTEM_TYPE);

				switch (type)
				{
					case 0x01: // FAT 12
					case 0x04: // FAT 16
					case 0x06:
					case 0x0E:
#ifndef fileFAT_NO_FAT32_SUPPORT // If FAT32 supported.
					case 0x0B: // FAT 32
					case 0x0C:
#endif
						// Get the 32 bit offset to the first partition
						volume_info->FATVolumeInfo.VolumeStartLBA = fatGetSystemSectorDWord(partition_entry_pos + fileFAT_PTEI_FIRST_SECTOR_LBA);

						// load first sector
						success = fatReadSystemSector(volume_index, 0);
						if(success)
							success = fatCacheVolumeInformation(volume_info);

						return success;
				}

				/* If we are here, we didn't find a matching partition.  We
				   should increment to the next partition table entry */
				partition_entry_pos += fileFAT_PARTITION_ENTRY_LENGTH;
			}
		}
	}

	return success;
}

///////////////////////////////////////////////////////////////////////////////
//! Unmounts file system
void fatUnmountDrive(uint8_t in_drive_index)
{
	uint8_t volume_index = fileVOLUMES_GET_VOLUME_INDEX_FROM_DRIVE(in_drive_index);

	fatFlushSystemSector(volume_index);
}

///////////////////////////////////////////////////////////////////////////////
//!	Caches volume info (the content of the system buffer must be a boot record)
static bool fatCacheVolumeInformation(fileVolumeInfo* out_volume_info)
{
	uint16_t root_dir_sectors;
	uint32_t sectors_per_fat;
	uint32_t total_sectors;
	uint32_t data_sectors;
	uint8_t* sector_buffer;
	uint16_t reserved_sectors;

	sector_buffer = fileFAT_SYSTEM_SECTOR_BUFFER.Buffer;

	// check boot sector
	if(		fatGetSystemSectorWord(fileFAT_BSI_BPS) != fileFAT_SECTOR_LENGTH ||									// check sector length
				fatGetSystemSectorByte(fileFAT_BSI_FATCOUNT) != fileFAT_NUMBER_OF_FAT_TABLES ||			// number of FATs
				fatGetSystemSectorByte(fileFAT_BSI_SIGN_0) != fileFAT_GOOD_SIGN_0 ||								// signature
				fatGetSystemSectorByte(fileFAT_BSI_SIGN_1) != fileFAT_GOOD_SIGN_1)
		return false;
				
	// cache boot block parameters
	out_volume_info->FATVolumeInfo.SectorsPerClaster = fatGetSystemSectorByte(fileFAT_BSI_SPC);
	reserved_sectors = fatGetSystemSectorWord(fileFAT_BSI_RESRVSEC);

	// total sectors
	if( fatGetSystemSectorWord(fileFAT_BSI_TOTSEC16) != 0 )
		total_sectors = fatGetSystemSectorWord(fileFAT_BSI_TOTSEC16);
	else
	{
#ifdef fileFAT_NO_FAT32_SUPPORT
		return false;
#else
		total_sectors = fatGetSystemSectorDWord(fileFAT_BSI_TOTSEC32);
#endif
	}

	// sectors per FAT
	if(fatGetSystemSectorWord(fileFAT_BSI_SPF) != 0)
		sectors_per_fat = fatGetSystemSectorWord(fileFAT_BSI_SPF);
	else
	{
#ifdef fileFAT_NO_FAT32_SUPPORT
		return false;
#else
		sectors_per_fat = fatGetSystemSectorDWord(fileFAT_BSI_FATSZ32);
#endif
	}

	// FAT positions
	out_volume_info->FATVolumeInfo.FirstFATStart = reserved_sectors;

	//FAT32
	out_volume_info->FATVolumeInfo.SecondFATStart = sectors_per_fat + reserved_sectors;
	out_volume_info->FATVolumeInfo.RootDirectoryStart = sectors_per_fat * fileFAT_NUMBER_OF_FAT_TABLES + reserved_sectors;

	root_dir_sectors = (uint16_t)((fatGetSystemSectorWord(fileFAT_BSI_ROOTDIRENTS) * fileFAT_DIRECTORY_ENTRY_LENGTH + fileFAT_SECTOR_LENGTH - 1 ) / fileFAT_SECTOR_LENGTH);

	out_volume_info->FATVolumeInfo.FirstDataSector = out_volume_info->FATVolumeInfo.RootDirectoryStart + root_dir_sectors;

	data_sectors = total_sectors - ( reserved_sectors + (sectors_per_fat * 2) + root_dir_sectors);

	out_volume_info->FATVolumeInfo.DataClasterCount = (fatClusterAddress)(data_sectors / out_volume_info->FATVolumeInfo.SectorsPerClaster);

	if(out_volume_info->FATVolumeInfo.DataClasterCount < 4085)
	{
		// Volume is FAT12
		out_volume_info->FATVolumeInfo.FATType = fileFAT_TYPE_FAT12;
	}
	else
	{
		if(out_volume_info->FATVolumeInfo.DataClasterCount < 65525)
		{
			// Volume is FAT16
			out_volume_info->FATVolumeInfo.FATType = fileFAT_TYPE_FAT16;
		}
		else
		{
			// Volume is FAT32
			out_volume_info->FATVolumeInfo.FATType = fileFAT_TYPE_FAT32;

			// update some information related to FAT32
			out_volume_info->FATVolumeInfo.RootDirectoryStart = fatGetSystemSectorDWord(fileFAT_BSI_ROOT_DIR_CLUS);
			out_volume_info->FATVolumeInfo.FirstDataSector = reserved_sectors + (sectors_per_fat * 2);
		}
	}

	// sets root directory as default
	out_volume_info->FATVolumeInfo.CurrentDirectory.ShortName[0] = filePATH_SEPARATOR;
	out_volume_info->FATVolumeInfo.CurrentDirectory.ShortName[1] = '\0';

	fatChangeDirectory(&out_volume_info->FATVolumeInfo.CurrentDirectory);

	return true;
}
// </editor-fold>
#pragma endregion

/*****************************************************************************/
/* Directory parsing routines                                                */
/*****************************************************************************/
#pragma region Directory parsing routines
// <editor-fold defaultstate="collapsed" desc="Directory parsing routines">

///////////////////////////////////////////////////////////////////////////////
//! Get First Directory Entry of a given folder
//! \param Information of the starting folder
//! \return true If directory entry is exists and folder parameter is updated with the directory information
bool fatGetFirstDirectoryEntry(fatFile* in_file )
{
	// root directory
	if(	!fatChangeDirectory( in_file ) )
		return false;

	// flag the first entry
	in_file->DirectoryEntryIndex = 0xff;

	return fatGetNextDirectoryEntry( in_file );
}

///////////////////////////////////////////////////////////////////////////////
//! Get Next Directory Entry of a given folder
//! \param Information of the current folder
//! \return true If directory entry is exists and folder parameter is updated with the directory information
bool fatGetNextDirectoryEntry( fatFile* in_file )
{
	uint8_t i,j;
	uint16_t entry_pos;
	bool first_ext = true;
	bool skip_entry = true;

#ifndef fileFAT_NO_LONG_FILENAME_SUPPORT
	fatClusterAddress DirectoryCluster;
	uint16_t DirectorySector = 0xff;
	uint8_t DirectoryEntryIndex;
#endif

	// do not skip entry in the case of first entry
	if( in_file->DirectoryEntryIndex == 0xff )
	{
		skip_entry = false;
		in_file->DirectoryEntryIndex = 0;
	}

	// find and process directory entry
	while( true )
	{
		// read next entry
		fatReadDirectoryEntry( in_file );
		
		// position of the entry in the buffer
		entry_pos = in_file->DirectoryEntryIndex * fileFAT_DIRECTORY_ENTRY_LENGTH;

		// check first character of name
		switch( fatGetSystemSectorByte(entry_pos) )
		{
			// empty entry
			case 0xe5:
				// invalidate LFN
#ifndef fileFAT_NO_LONG_FILENAME_SUPPORT
				DirectorySector = 0xff;
#endif
				// move to the next entry
				in_file->DirectoryEntryIndex++;
				break;

			// no more entry
			case 0x00:
				// exit
				return false;

			// entry found:
			default:
				if( (fatGetSystemSectorByte(entry_pos+11) & fileFAT_ATTR_LONG_NAME_MASK) != fileFAT_ATTR_LONG_NAME )
				{
					// if not long file name entry
					if( skip_entry )
					{
						// skip this entry because this is same as the previous
						skip_entry = false;
						in_file->DirectoryEntryIndex++;
						continue;
					}

					// get properties
					in_file->Attributes = fatGetSystemSectorByte(entry_pos+11);
					in_file->StartCluster = fatGetSystemSectorWord(entry_pos + 26);
					in_file->Size = fatGetSystemSectorDWord(entry_pos + 28);
					in_file->OpenMode = fileFAT_OPEN_MODE_CLOSED;

#ifndef fileFAT_NO_LONG_FILENAME_SUPPORT
					if( DirectorySector != 0xff )
					{
						in_file->DirectoryCluster = DirectoryCluster;
						in_file->DirectorySector = DirectorySector;
						in_file->DirectoryEntryIndex = DirectoryEntryIndex;
					}
#endif

					// concat filename
					i = 0;
					j = 0;
					for( i = 0; i < 11; i++ )
					{
						if( fatGetSystemSectorByte(entry_pos+i) != 0x20 )
						{
							if( i > 7 && first_ext )
							{
								in_file->ShortName[j++] = '.';
								first_ext = false;
							}

							in_file->ShortName[j++] = fatGetSystemSectorByte(entry_pos+i);
						}
					}
					in_file->ShortName[j] = '\0';
					return true;
				}
				else
				{
					// if this is the last entry -> store the position
#ifndef fileFAT_NO_LONG_FILENAME_SUPPORT
					if( !skip_entry && (fatGetSystemSectorByte(entry_pos) & 0x40) != 0 )
					{
						DirectoryCluster = in_file->DirectoryCluster;
						DirectorySector = in_file->DirectorySector;
						DirectoryEntryIndex = in_file->DirectoryEntryIndex;
					}
#endif

					// long filename entry found
					in_file->DirectoryEntryIndex++;
				}
				break;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
//! Changes directory (enters into the directory). Please note this function is not recursive only entering into one level deeper.
//! \param Information of the directory to enter
//! \return true If directory is exists and changed
bool fatChangeDirectory( fatFile* in_file )
{
	uint8_t volume_index = fileVOLUMES_GET_VOLUME_INDEX_FROM_FILE(in_file);
	fileVolumeInfo* volume_info = fileVOLUMES_GET_VOLUME_INFO_FROM_INDEX(volume_index);

	// check for root directory entry
	if( in_file->ShortName[0] == filePATH_SEPARATOR && in_file->ShortName[1] == '\0' )
	{
		in_file->Attributes = fileFAT_ATTR_DIRECTORY;
		in_file->StartCluster = 0;
	}

	// file must be a directory
	if( ( in_file->Attributes & fileFAT_ATTR_DIRECTORY ) == 0 )
		return false;

	// check if root directory
	if( in_file->StartCluster == 0 )
	{
		// set root directory information
		if(volume_info->FATVolumeInfo.FATType == fileFAT_TYPE_FAT32)
		{
			// FAT32
			in_file->DirectoryCluster  = volume_info->FATVolumeInfo.RootDirectoryStart;
			in_file->DirectorySector = 0;
		}
		else
		{
			// FAT12, FAT16
			in_file->DirectoryCluster  = 0;
			in_file->DirectorySector = volume_info->FATVolumeInfo.RootDirectoryStart;
		}
	}
	else
	{
		// set directory information
		in_file->DirectoryCluster = in_file->StartCluster;
		in_file->DirectorySector = 0;
	}

	// first entry
	in_file->DirectoryEntryIndex = 0;
	in_file->OpenMode = fileFAT_OPEN_MODE_CLOSED;

	return true;
}

///////////////////////////////////////////////////////////////////////////////
//! Gets number of entries in the given directory
//! \param Information of the current folder
//! \return Number of entries
uint16_t fatGetNumberOfEntries( fatFile* in_file )
{
	fatFile file = *in_file;
	uint16_t count = 0;
	uint16_t entry_pos;

	// change directory
	if( !fatChangeDirectory( &file ) )
		return 0;

	// find and process directory entry
	while( true )
	{
		// read next entry
		fatReadDirectoryEntry( &file );
		
		// position of the entry in the buffer
		entry_pos = file.DirectoryEntryIndex * fileFAT_DIRECTORY_ENTRY_LENGTH;

		// check first character of name
		switch( fatGetSystemSectorByte(entry_pos) )
		{
			// empty entry
			case 0xe5:
				break;

			// no more entry
			case 0x00:
				// exit
				return count;

			// entry found:
			default:
				if( (fatGetSystemSectorByte(entry_pos+11) & fileFAT_ATTR_LONG_NAME_MASK) != fileFAT_ATTR_LONG_NAME )
				{
					// count only real entries (skip "." and ".."
					if( fatGetSystemSectorByte(entry_pos) != '.' )
						count++;
				}
				break;
		}

		// next entry
		file.DirectoryEntryIndex++;
	}
}

///////////////////////////////////////////////////////////////////////////////
// Read next directory entry
static bool fatReadDirectoryEntry( fatFile* in_file )
{
	uint8_t volume_index = fileVOLUMES_GET_VOLUME_INDEX_FROM_FILE(in_file);
	fileVolumeInfo* volume_info = fileVOLUMES_GET_VOLUME_INFO_FROM_INDEX(volume_index);

	// read next sector if necessary
	if( in_file->DirectoryEntryIndex >= fileFAT_SECTOR_LENGTH / fileFAT_DIRECTORY_ENTRY_LENGTH )
	{
		in_file->DirectoryEntryIndex = 0;
		in_file->DirectorySector++;

		// if root directory entry
		if( in_file->DirectoryCluster == 0 )
		{
			if( in_file->DirectorySector >= volume_info->FATVolumeInfo.FirstDataSector )
				return false;
		}
		else
		{
			// sub directory
			if( in_file->DirectorySector >= volume_info->FATVolumeInfo.SectorsPerClaster )
			{
				// next cluster
				in_file->DirectoryCluster = fatReadFATEntry(volume_index, in_file->DirectoryCluster );

				if( in_file->DirectoryCluster == 0 )
					return false;

				in_file->DirectorySector = 0;
				in_file->DirectoryEntryIndex = 0;
			}
		}
	}

	// be sure directory sector is in the buffer
	return fatReadDirectorySector( in_file );
}
// </editor-fold>
#pragma endregion
	
/*****************************************************************************/
/* Internal sector read/write routines                                       */
/*****************************************************************************/
#pragma region Internal sector read/write routines
// <editor-fold defaultstate="collapsed" desc="Internal sector read/write routines">

///////////////////////////////////////////////////////////////////////////////
//! Reads system sector (directory/fat table). Before reading it writes back poending changes.
//! @param in_volume_index Volume index
//! @param in_lba LBA of the sector to read
//! @return True if sector was loaded completely
static bool fatReadSystemSector(uint8_t in_volume_index, uint32_t in_lba)
{
	bool success = true;

	in_lba += fileVOLUMES_GET_VOLUME_INFO_FROM_INDEX(in_volume_index)->FATVolumeInfo.VolumeStartLBA;

	// if it's already in the buffer don't do anything
	if( fileFAT_SYSTEM_SECTOR_BUFFER.LBA == in_lba )
		return true;

	// flush the content of the current buffer
	fatFlushSystemSector(in_volume_index);

	// read sector
	fileFAT_SYSTEM_SECTOR_BUFFER.LBA = in_lba;

	success = fileReadSector(in_volume_index, fileFAT_SYSTEM_SECTOR_BUFFER.Buffer, fileFAT_SYSTEM_SECTOR_BUFFER.LBA); 

	return success;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Flushes system sector buffer content to the storage media
/// @param in_volume_index Volume (drive) index
static void fatFlushSystemSector(uint8_t in_volume_index)
{
#ifndef fileFAT_READ_ONLY_FILESYSTEM
	fileVolumeInfo* volume_info = fileFAT_GET_VOLUME_INFO_FROM_INDEX(in_volume_index);

	// return if not modified
	if( !fileFAT_SYSTEM_SECTOR_BUFFER.Modified )
		return;

	// if the sector is fat sector, write it into the second copy of fat
	if( fileFAT_SYSTEM_SECTOR_BUFFER.LBA >= volume_info->FATVolumeInfo.FirstFATStart &&
		fileFAT_SYSTEM_SECTOR_BUFFER.LBA < volume_info->FATVolumeInfo.SecondFATStart )
	{
		fileWriteSector(in_volume_index, fileFAT_SYSTEM_SECTOR_BUFFER.Buffer, fileFAT_SYSTEM_SECTOR_BUFFER.LBA - volume_info->FATVolumeInfo.FirstFATStart + volume_info->FATVolumeInfo.SecondFATStart );
	}

	// write sector
	fileWriteSector(in_volume_index, fileFAT_SYSTEM_SECTOR_BUFFER.Buffer, fileFAT_SYSTEM_SECTOR_BUFFER.LBA);

	// clear flag
	fileFAT_SYSTEM_SECTOR_BUFFER.Modified = false;
#endif
}

///////////////////////////////////////////////////////////////////////////////
//! Reads one sector of a system cluster
//! @param in_volume_index Volume (drive) index
//! @param in_cluster_address Cluster address to read
//! @param in_sector Sector address within cluster
static bool fatReadSystemSectorOfCluster( uint8_t in_volume_index, fatClusterAddress in_cluster_address, uint8_t in_sector )
{
	uint32_t lba;
	fatVolumeInfo* volume_info = &(fileVOLUMES_GET_VOLUME_INFO_FROM_INDEX(in_volume_index)->FATVolumeInfo);

	lba = ((in_cluster_address - 2) * volume_info->SectorsPerClaster) + volume_info->FirstDataSector + in_sector;

	return fatReadSystemSector( in_volume_index, lba );
}

///////////////////////////////////////////////////////////////////////////////
//! Reads Directory sector
static bool fatReadDirectorySector( fatFile* in_file )
{
	uint8_t volume_index = fileVOLUMES_GET_VOLUME_INDEX_FROM_FILE(in_file);

	// check for root directory
	if( in_file->DirectoryCluster == 0 )
		return fatReadSystemSector(volume_index, in_file->DirectorySector); // root
	else
		return fatReadSystemSectorOfCluster(volume_index, in_file->DirectoryCluster, (uint8_t)in_file->DirectorySector); // sub
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Reads current data sector of th egiven file
/// @param in_file File information
/// @return True if sector is/was loaded into the buffer
static bool fatReadDataSector( fatFile* in_file )
{
	uint32_t lba;
	uint8_t volume_index;
	fatVolumeInfo* volume_info;
	bool success;

	volume_index = fileVOLUMES_GET_VOLUME_INDEX_FROM_FILE(in_file);
	volume_info = &(fileVOLUMES_GET_VOLUME_INFO_FROM_INDEX(in_volume_index)->FATVolumeInfo);

	lba = (in_file->CurrentCluster * volume_info->SectorsPerClaster) + volume_info->FirstDataSector + in_file->CurrentSector;

	// if it's already in the buffer don't do anything
	if( fileFAT_DATA_SECTOR_BUFFER( in_file ).LBA == lba )
		return true;

	// flush the content of the current file data buffer
	fatFlushDataSector(in_file);

	// read sector
	fileFAT_DATA_SECTOR_BUFFER( in_file ).LBA = lba;

	success = fileReadSector(volume_index, fileFAT_DATA_SECTOR_BUFFER( in_file ).Buffer, fileFAT_DATA_SECTOR_BUFFER( in_file ).LBA); 

	return success;
}

// </editor-fold>
#pragma endregion

/*****************************************************************************/
/* FAT entry handling functions                                              */
/*****************************************************************************/
#pragma region FAT entry handling functions
// <editor-fold defaultstate="collapsed" desc="FAT entry handling functions">

///////////////////////////////////////////////////////////////////////////////
//! Reads entry from the FAT table
static fatClusterAddress fatReadFATEntry( uint8_t in_volume_index, fatClusterAddress in_cluster_address )
{
	uint32_t entry_byte_address;
	uint32_t fat_sector_lba;
	fatClusterAddress cluster_address;
	uint16_t offset;
	fatVolumeInfo* volume_info = &(fileVOLUMES_GET_VOLUME_INFO_FROM_INDEX(volume_index)->FATVolumeInfo);

	switch( volume_info->FATType )
	{
		//FAT12
		case fileFAT_TYPE_FAT12:
			// calculate sector and offset
			entry_byte_address = in_cluster_address + (in_cluster_address / 2);
			fat_sector_lba = volume_info->FirstFATStart + (entry_byte_address / fileFAT_SECTOR_LENGTH);
			offset = fat_sector_lba % fileFAT_SECTOR_LENGTH;

			// read sector
			if( offset != fileFAT_SECTOR_LENGTH - 1 )
			{
				// normal case
				fatReadSystemSector(in_volume_index, fat_sector_lba );

				cluster_address = fatGetSystemSectorWord(offset);
			}
			else
			{
				// boundary case
				fatReadSystemSector(in_volume_index, fat_sector_lba );

				cluster_address = fatGetSystemSectorByte(fileFAT_SECTOR_LENGTH-1);

				// next sector
				fatReadSystemSector(in_volume_index, fat_sector_lba + 1);

				cluster_address += ((uint16_t)fatGetSystemSectorByte(0) << 8);
			}

			// handle odd&even entries
			if( in_cluster_address & 0x0001)
				cluster_address = cluster_address >> 4; // Cluster number is ODD
			else
				cluster_address = cluster_address & 0x0FFF; // Cluster number is EVEN

			// End of cluster chain
			if( cluster_address >= 0x0FF8 )
				cluster_address = fileFAT_EOC;

			break;

		// FAT16
		case fileFAT_TYPE_FAT16:
			// calculate offset and cluster address
			entry_byte_address = in_cluster_address * sizeof(uint16_t);
			fat_sector_lba = volume_info->FirstFATStart + entry_byte_address / fileFAT_SECTOR_LENGTH;
			offset = fat_sector_lba % fileFAT_SECTOR_LENGTH;

			fatReadSystemSector(in_volume_index, fat_sector_lba );

			// get FAT value
			cluster_address = fatGetSystemSectorWord(offset);

			// End of cluster chain
			if( cluster_address >= 0xFFF8 )
				cluster_address = fileFAT_EOC;

			break;

#ifndef fileFAT_NO_FAT32_SUPPORT
		// FAT32
		case fileFAT_TYPE_FAT32:
			// calculate offset and cluster address
			entry_byte_address = in_cluster_address * sizeof(uint32_t);
			fat_sector_lba = volume_info->FirstFATStart + entry_byte_address / fileFAT_SECTOR_LENGTH;
			offset = fat_sector_lba % fileFAT_SECTOR_LENGTH;

			fatReadSystemSector(in_volume_index, fat_sector_lba );

			// get FAT value
			cluster_address = fatGetSystemSectorDWord(offset);

			// End of cluster chain
			if( cluster_address >= 0x0FFFFFF8 )
				cluster_address = fileFAT_EOC;
			break;
#endif

	}

	return cluster_address;
}

///////////////////////////////////////////////////////////////////////////////
// Write FAT entry
#ifndef fileFAT_READ_ONLY_FILESYSTEM
void fatIntWriteFATEntry( fatClusterAddress in_cluster_address, fatClusterAddress in_new_value  )
{
	fatClusterAddress fat_offset;
	uint32_t fat_sector;
	fatClusterAddress new_cluster_address;
	uint16_t offset;
	uint8_t low_mask, high_mask;

	switch( l_fat_type )
	{
		//FAT12
		case fileFAT_TYPE_FAT12:
			// calculate sector and offset
			fat_offset = in_cluster_address + (in_cluster_address / 2);
			
			fat_sector = l_1st_fat_start + (fat_offset / fileFAT_SECTOR_LENGTH);
			offset = fat_offset % fileFAT_SECTOR_LENGTH;

			new_cluster_address = in_new_value;

			if( new_cluster_address == fileFAT_EOC )
				new_cluster_address = 0x0FFF;

			// read sector
			if( offset != fileFAT_SECTOR_LENGTH - 1 )
			{
				// normal case
				fatIntReadSector( fat_sector );

				// update entry
				if(in_cluster_address & 0x0001)
				{
					new_cluster_address = new_cluster_address << 4; // Cluster number is ODD
					*(uint16_t*)(l_data_sector_buffer+offset) = (*(uint16_t*)(l_data_sector_buffer+offset)) & 0x000F;
				}
				else 
				{
					new_cluster_address = new_cluster_address & 0x0FFF; // Cluster number is EVEN
					*(uint16_t*)(l_data_sector_buffer+offset) = (*(uint16_t*)(l_data_sector_buffer+offset)) & 0xF000;
				}

				*(fatClusterAddress*)(l_data_sector_buffer+offset) = (*(uint16_t*)(l_data_sector_buffer+offset)) | new_cluster_address;
				l_sector_modified = true;
			}
			else
			{
				// boundary case
				if(in_cluster_address & 0x0001)
				{
					new_cluster_address = new_cluster_address << 4; // Cluster number is ODD

					low_mask = 0x0f;
					high_mask = 0x00;
				}
				else 
				{
					new_cluster_address = new_cluster_address & 0x0FFF; // Cluster number is EVEN

					low_mask = 0x00;
					high_mask = 0xf0;
				}

				// low byte
				fatIntReadSector( fat_sector );

				l_data_sector_buffer[fileFAT_SECTOR_LENGTH-1] = (l_data_sector_buffer[fileFAT_SECTOR_LENGTH-1] & low_mask) | LOW(new_cluster_address);
				l_sector_modified = true;

				// next sector
				fatIntReadSector( fat_sector + 1);

				l_data_sector_buffer[0] = (l_data_sector_buffer[0] & high_mask) | HIGH((uint16_t)new_cluster_address);
				l_sector_modified = true;
			}
			break;

		// FAT16
		case fileFAT_TYPE_FAT16:
			// calculate offset and cluster address
			fat_sector = in_cluster_address / (fileFAT_SECTOR_LENGTH / 2 ) + l_1st_fat_start;
			fat_offset = (uint16_t)(in_cluster_address % (fileFAT_SECTOR_LENGTH / 2 ));

			fatIntReadSector( fat_sector );

			// Update value
			new_cluster_address = in_new_value;

			if( new_cluster_address == fileFAT_EOC )
				new_cluster_address = 0xFFFF;

			// set FAT value
			*(fatClusterAddress*)(l_data_sector_buffer + fat_offset) = in_new_value;
			l_sector_modified = true;

			break;

#ifndef fileFAT_NO_FAT32_SUPPORT
		// FAT32
		case fileFAT_TYPE_FAT32:
			// calculate offset and cluster address
			fat_sector = in_cluster_address / (fileFAT_SECTOR_LENGTH / 4 ) + l_1st_fat_start;
			fat_offset = (uint16_t)(in_cluster_address % (fileFAT_SECTOR_LENGTH / 4 ));

			fatIntReadSector( fat_sector );

			// get FAT value
			new_cluster_address = *(uint32_t*)(l_data_sector_buffer + fat_offset);

			// End of cluster chain
			if( new_cluster_address >= 0x0FFFFFF8 )
				new_cluster_address = fileFAT_EOC;
			break;
#endif

	}
}
#endif

#ifndef fileFAT_READ_ONLY_FILESYSTEM
///////////////////////////////////////////////////////////////////////////////
// Get Free Cluster Offset
fatClusterAddress fatIntGetFreeCluster(void)
{
	uint16_t cluster;
	
	cluster = 0;
	while( cluster < l_data_cluster_count )
	{
		if( fatIntReadFATEntry( cluster + 2 ) == fileFAT_FREE )
			return cluster + 2;

		cluster++;
	}

	// invalid address
	return fileFAT_EOC;
}

#endif

// </editor-fold>
#pragma endregion

/*****************************************************************************/
/* Date & time handling                                                      */
/*****************************************************************************/
#pragma region Date & time handling
// <editor-fold defaultstate="collapsed" desc="Date & time handling">

///////////////////////////////////////////////////////////////////////////////
// Get File Date Time
void fatGetFileDateTime( fatFile* in_file, sysDateTime* out_datetime )
{
	uint16_t entry_pos;
	uint16_t data;
	uint8_t volume_index = fileVOLUMES_GET_VOLUME_INDEX_FROM_FILE(in_file);
	fatVolumeInfo* volume_info = &(fileVOLUMES_GET_VOLUME_INFO_FROM_INDEX(volume_index)->FATVolumeInfo);

#ifndef fileFAT_NO_LONG_FILENAME_SUPPORT
	// skip long name entries
	fatFile file = *in_file;
	fatIntSkipLongFilenameEntries( &file );
	entry_pos = file.DirectoryEntryIndex * fileFAT_DIRECTORY_ENTRY_LENGTH;
#else
	// read sector
	fatReadDirectorySector( in_file );

	// position of the entry in the buffer
	entry_pos = in_file->DirectoryEntryIndex * fileFAT_DIRECTORY_ENTRY_LENGTH;
#endif

	// time
	data = fatGetSystemSectorWord(entry_pos + 22);

	out_datetime->Second = 2 * (data & 0x1f);
	out_datetime->Minute = (data >> 5) & 0x3f;
	out_datetime->Hour = (data >> 11) & 0x1f;

	// date
	data = fatGetSystemSectorWord(entry_pos + 24);

	out_datetime->Day = (data & 0x1f);
	out_datetime->Month = (data >> 5 ) & 0x0f;
	out_datetime->Year = 1980 + ((data >> 9 ) & 0x7f);
}

#ifndef fileFAT_READ_ONLY_FILESYSTEM

///////////////////////////////////////////////////////////////////////////////
// Set File Date Time
void fatSetFileDateTime( fatFile* in_file, sysDateTime* in_datetime )
{
	uint16_t entry_pos;
	uint16_t data;

#ifndef fileFAT_NO_LONG_FILENAME_SUPPORT
	// skip long name entries
	fatFile file = *in_file;
	fatIntSkipLongFilenameEntries( &file );
	entry_pos = file.DirectoryEntryIndex * fileFAT_DIRECTORY_ENTRY_LENGTH;
#else
	// read sector
	fatIntReadDirectorySector( in_file );

	// position of the entry in the buffer
	entry_pos = in_file->DirectoryEntryIndex * fileFAT_DIRECTORY_ENTRY_LENGTH;
#endif

	// time
	data = (in_datetime->Second / 2) & 0x1f;
	data += (uint16_t)(in_datetime->Minute & 0x3f) << 5; 
	data += (uint16_t)(in_datetime->Hour & 0x1f) << 11;

	*(uint16_t*)(l_data_sector_buffer + entry_pos + 22) = data;

	// date
	data = (in_datetime->Day & 0x1f);
	data += (uint16_t)(in_datetime->Month & 0x0f) << 5;

	if( in_datetime->Year >= 1980 )
		data += (uint16_t)((in_datetime->Year - 1980) & 0x7f) << 9;

	*(uint16_t*)(l_data_sector_buffer + entry_pos + 24) = data;

	l_sector_modified = true;
}
#endif

// </editor-fold>
#pragma endregion

///////////////////////////////////////////////////////////////////////////////
// Open/Create/Close Functions
///////////////////////////////////////////////////////////////////////////////
#pragma region Open/Create/Close functions

///////////////////////////////////////////////////////////////////////////////
// Open file
bool fatOpen( fatFile* in_file, uint8_t in_open_mode  )
{
	in_file->CurrentPos = 0;
	in_file->CurrentCluster = in_file->StartCluster;
	in_file->CurrentSector = 0;
	in_file->CurrentSectorPos = 0;
	in_file->OpenMode = in_open_mode;

	return true;
}

///////////////////////////////////////////////////////////////////////////////
// Close file
void fatClose( fatFile* in_file )
{
#ifndef fileFAT_READ_ONLY_FILESYSTEM
	uint16_t entry_pos;
#endif

	// if file is not opened
	if( in_file->OpenMode == fileFAT_OPEN_MODE_CLOSED )
		return;

#ifndef fileFAT_READ_ONLY_FILESYSTEM
	// if file was opened for write -> update size
	if( (in_file->OpenFlags & fileFAT_OPEN_FLAG_READWRITE) != 0 )
	{
#ifndef fileFAT_NO_LONG_FILENAME_SUPPORT
		// skip long name entries
		fatFile file = *in_file;
		fatIntSkipLongFilenameEntries( &file );
		entry_pos = file.DirectoryEntryIndex * fileFAT_DIRECTORY_ENTRY_LENGTH;
#else
		// read sector
		fatIntReadDirectorySector( in_file );

		// position of the entry in the buffer
		entry_pos = in_file->DirectoryEntryIndex * fileFAT_DIRECTORY_ENTRY_LENGTH;
#endif

		// update size
		*(uint32_t*)(l_sector_buffer.Buffer + entry_pos + 28) = in_file->Size;
		l_sector_buffer.Modified = true;

		// flush buffer
		fatFlushBuffer();
	}
#endif

	// flag for closed file
	in_file->OpenMode = fileFAT_OPEN_MODE_CLOSED;
}

///////////////////////////////////////////////////////////////////////////////
// Create File
bool fatCreateFile( fatFile* in_directory, fatFile* in_file )
{
#ifndef fileFAT_READ_ONLY_FILESYSTEM
	bool success = true;
	uint16_t entry_pos;
	uint8_t filename[fileMAX_FAT_FILENAME_LENGTH];
	uint8_t i;
	uint8_t attributes;

	// check if file exists
	attributes = in_file->Attributes;
	if( fatIsFileExists( in_directory, in_file ) )
		return false;

	// convert filename to FAT name
	if( !fatIntCovertFilenameToFATFilename( in_file, filename ) )
		return false;

	// change directory
	fatChangeDirectory( in_directory );

	// copy directory info into file struct
	in_file->DirectoryCluster = in_directory->DirectoryCluster;
	in_file->DirectorySector = in_directory->DirectorySector;
	in_file->DirectoryEntryIndex = in_directory->DirectoryEntryIndex;

	// read directory entry
	while( true )
	{
		if( fatIntReadDirectoryEntry( in_file ) )
		{
			entry_pos = in_file->DirectoryEntryIndex * fileFAT_DIRECTORY_ENTRY_LENGTH;

			if( l_data_sector_buffer[entry_pos] == 0xe5 || l_data_sector_buffer[entry_pos] == 0 )
			{
				// clear entry
				for( i = 0; i < fileFAT_DIRECTORY_ENTRY_LENGTH; i++ )
					l_data_sector_buffer[entry_pos+i] = 0;

				// store filename
				for( i = 0; i < fileMAX_FAT_FILENAME_LENGTH; i++ )
					l_data_sector_buffer[entry_pos+i] = filename[i];

				// copy attribute
				l_data_sector_buffer[entry_pos+i] = attributes;

				// flag for modification
				l_sector_modified = true;
				break;
			}
		}
		else
			break;

		in_file->DirectoryEntryIndex++;
	}

	in_file->CurrentCluster = 0;
	in_file->CurrentSector = 0;
	in_file->CurrentSectorPos = 0;
	in_file->CurrentPos = 0;
	in_file->Size = 0;
	in_file->StartCluster = 0;
	in_file->OpenFlags = fileFAT_OPEN_FLAG_READWRITE;

	return success;
#else
	return false;
#endif
}
#pragma endregion

///////////////////////////////////////////////////////////////////////////////
// Read/Write/IsEof/IsFileExists functions
///////////////////////////////////////////////////////////////////////////////
#pragma region Read/Write/IsEof/IsFileExists functions

///////////////////////////////////////////////////////////////////////////////
// Is Eof
bool fatIsEof( fatFile* in_file )
{
	return in_file->CurrentPos >= in_file->Size;
}

///////////////////////////////////////////////////////////////////////////////
// Is file exists
bool fatIsFileExists( fatFile* in_directory, fatFile* in_file )
{
	fatFile file;
	bool found;
	uint8_t i;

	file = *in_directory;
	if( fatChangeDirectory( &file ) )
	{
		found = fatGetFirstDirectoryEntry( &file );

		while( found )
		{
			i = 0;
			while( found && i < fileFAT_MAX_SHORT_FILENAME_LENGTH - 1 )
			{
				if( strCharToUpper(file.ShortName[i]) != strCharToUpper(in_file->ShortName[i]))
					found = false;
				else
					i++;
			}

			if(found)
			{
				*in_file = file;
				return true;
			}

			found = fatGetNextDirectoryEntry( &file );
		}
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////
// Read buffer from file
uint16_t fatRead( fatFile* in_file, uint8_t* out_buffer, uint16_t in_buffer_size )
{
	uint16_t data_to_copy;
	uint16_t pos;
	bool eof = false;
	uint16_t i;
	uint8_t volume_index = fileVOLUMES_GET_VOLUME_INDEX_FROM_FILE(in_file);
	fatVolumeInfo* volume_info = &(fileVOLUMES_GET_VOLUME_INFO_FROM_INDEX(volume_index)->FATVolumeInfo);

	pos = 0;

	while( pos < in_buffer_size && !eof )
	{
		// get the number of bytes to copy
		data_to_copy = fileFAT_SECTOR_LENGTH - in_file->CurrentSectorPos;

		if( data_to_copy > in_file->Size - in_file->CurrentPos )
		{
			data_to_copy = (uint16_t)(in_file->Size - in_file->CurrentPos);
			eof = true;
		}

		if( data_to_copy > in_buffer_size )
			data_to_copy = in_buffer_size;

		// read sector
		fatReadDataSector(in_file);

		// copy bytes
		i = 0;
		while( i < data_to_copy )
		{
			out_buffer[pos++] = fileFAT_DATA_SECTOR_BUFFER(in_file).Buffer[in_file->CurrentSectorPos++];
			i++;
		}

		// update pointers
		in_file->CurrentPos += data_to_copy;

		if( in_file->CurrentSectorPos >= fileFAT_SECTOR_LENGTH && !eof )
		{
			// read next sector
			in_file->CurrentSector++;

			if( in_file->CurrentSector >= volume_info->SectorsPerClaster )
			{
				// move next cluster
				in_file->CurrentCluster = fatReadFATEntry( volume_index, in_file->CurrentCluster );

				if( in_file->CurrentCluster == fileFAT_EOC )
					eof = true;

				in_file->CurrentSector = 0;
			}

			in_file->CurrentSectorPos = 0;
		}
	}

	return pos;
}

///////////////////////////////////////////////////////////////////////////////
// Write from buffer to file
#ifndef fileFAT_READ_ONLY_FILESYSTEM
uint16_t fatWrite( fatFile* in_file, uint8_t* in_buffer, uint16_t in_buffer_size )
{
	fatClusterAddress cluster;
	uint16_t data_to_copy;
	uint16_t pos;
	uint16_t i;
	uint16_t entry_pos;
#ifndef fileFAT_NO_LONG_FILENAME_SUPPORT
	fatFile file;
#endif

	if( in_file->CurrentCluster == 0 )
	{
		// file is empty, try to allocate cluster
		cluster = fatIntGetFreeCluster();
		if( cluster != fileFAT_EOC )
		{
			fatIntWriteFATEntry( cluster, fileFAT_EOC );
	
			// update start cluster
#ifndef fileFAT_NO_LONG_FILENAME_SUPPORT
			// skip long name entries
			file = *in_file;
			fatIntSkipLongFilenameEntries( &file );
			entry_pos = file.DirectoryEntryIndex * fileFAT_DIRECTORY_ENTRY_LENGTH;
#else
			// read sector
			fatIntReadDirectorySector( in_file );

			// position of the entry in the buffer
			entry_pos = in_file->DirectoryEntryIndex * fileFAT_DIRECTORY_ENTRY_LENGTH;
#endif
			// FAT32
			*(fatClusterAddress *)(l_data_sector_buffer + entry_pos + 26 ) = cluster;
			l_sector_modified = true;
			fatFlushSectorBuffer();

			// update in_file
			in_file->StartCluster = cluster;
			in_file->CurrentCluster = cluster;
			in_file->CurrentSector = 0;
			in_file->CurrentSectorPos = 0;
			in_file->CurrentPos = 0;

			// first sector is in buffer (because the contents of the buffer doesn't matter)
			l_lba_of_sector_in_buffer = ((cluster - 2) * l_sectors_per_cluster) + l_first_data_sector;
		}
		else
			return 0;
	}
	else
	{
		fatIntReadSectorOfCluster( in_file->CurrentCluster, (uint8_t)in_file->CurrentSector );
	}

	// copy data from buffer
	pos = 0;
	while( pos < in_buffer_size )
	{
		data_to_copy = fileFAT_SECTOR_LENGTH - in_file->CurrentSectorPos;

		if( data_to_copy > in_buffer_size )
			data_to_copy = in_buffer_size;

		// copy bytes
		i = 0;
		while( i < data_to_copy )
		{
			l_data_sector_buffer[in_file->CurrentSectorPos++] = in_buffer[pos++];
			i++;
		}

		l_sector_modified = true;

		// update pointers
		in_file->CurrentPos += data_to_copy;

		// update size
		if( in_file->CurrentPos > in_file->Size )
			in_file->Size = in_file->CurrentPos;

		// next sector
		if( in_file->CurrentSectorPos >= fileFAT_SECTOR_LENGTH )
		{
			// read next sector
			in_file->CurrentSector++;

			if( in_file->CurrentSector >= l_sectors_per_cluster )
			{
				// move next cluster
				cluster = fatIntPrepareNextClusterForWrite( in_file->CurrentCluster );

				if( cluster == fileFAT_EOC )
					return pos;

				in_file->CurrentSector = 0;
			}

			in_file->CurrentSectorPos = 0;
		}
	}

	return pos;
}
#endif
#pragma endregion 

///////////////////////////////////////////////////////////////////////////////
// Delete file
#ifndef fileFAT_READ_ONLY_FILESYSTEM
bool fatDeleteFile( fatFile* in_file )
{
	fatClusterAddress cluster;
	fatClusterAddress next_cluster;
	uint16_t entry_pos;

	// refuse to delete read only files and volume id and opened files
	if( ( in_file->Attributes & fileFAT_ATTR_READ_ONLY ) != 0 ||
		( in_file->Attributes & fileFAT_ATTR_VOLUME_ID ) != 0 ||
		( in_file->OpenFlags != fileFAT_OPEN_FLAG_CLOSED ) )
		return false;

	// directory must be empty
	if( (in_file->Attributes & fileFAT_ATTR_DIRECTORY ) != 0 )
	{
		// do not delete root directory
		if( in_file->StartCluster == 0 )
			return false;

		// do not delete non empty directories
		if( fatGetNumberOfEntries( in_file ) > 0 )
			return false;
	}

#ifndef fileFAT_NO_LONG_FILENAME_SUPPORT
	do
	{
		// position of the entry in the buffer
		entry_pos = in_file->DirectoryEntryIndex * fileFAT_DIRECTORY_ENTRY_LENGTH;

		l_data_sector_buffer[entry_pos] = 0xe5;
		l_sector_modified = true;
	} while( (l_data_sector_buffer[entry_pos+11] & fileFAT_ATTR_LONG_NAME_MASK) == fileFAT_ATTR_LONG_NAME );
#else
	// Invalidate this entry
	fatIntReadDirectoryEntry( in_file );

	entry_pos = in_file->DirectoryEntryIndex * fileFAT_DIRECTORY_ENTRY_LENGTH;
	l_data_sector_buffer[entry_pos] = 0xe5;
	l_sector_modified = true;
#endif

	// TODO: check for unchaining this cluster

	cluster = in_file->StartCluster;

	// clear FAT chain
	while( cluster != fileFAT_EOC && cluster != fileFAT_FREE )
	{
		next_cluster = fatIntReadFATEntry( cluster );
		fatIntWriteFATEntry( cluster, fileFAT_FREE );
		cluster = next_cluster;
	} 

	return true;
}
#endif


///////////////////////////////////////////////////////////////////////////////
// Prepare next sector for write (allocate next sector if necessary)
#ifndef fileFAT_READ_ONLY_FILESYSTEM
fatClusterAddress fatIntPrepareNextClusterForWrite( fatClusterAddress in_cluster )
{
	fatClusterAddress cluster;
	fatClusterAddress new_cluster;

	// check this entry
	cluster = fatIntReadFATEntry( in_cluster );
	if( cluster == fileFAT_EOC )
	{
		// allocate new cluster
		new_cluster = fatIntGetFreeCluster();
		if( new_cluster == fileFAT_FREE )
		{
			fatIntWriteFATEntry( in_cluster, new_cluster );
			fatIntWriteFATEntry( new_cluster, fileFAT_EOC );

			return new_cluster;
		}
		else
			return fileFAT_EOC;
	}
	else
	{
		fatIntReadSectorOfCluster( cluster, 0 );
	}

	return cluster;
}
#endif

#if 0
#ifndef fileFAT_READ_ONLY_FILESYSTEM

///////////////////////////////////////////////////////////////////////////////
//! Flushes all buffer content to storage media
void fatFlushBuffer(void)
{
	fatFlushDataSectorBuffer();
	fatFlushSystemSectorBuffer();
}

#else
///////////////////////////////////////////////////////////////////////////////
// Flush sector buffer
void fatFlushSectorBuffer(void)
{
}

#endif

#endif
///////////////////////////////////////////////////////////////////////////////
// LONG FILENAME SUPPORT ROUTINES
///////////////////////////////////////////////////////////////////////////////
#pragma region Long Filename Support Routines

#ifndef fileFAT_NO_LONG_FILENAME_SUPPORT

///////////////////////////////////////////////////////////////////////////////
// Gets long file name
bool fatGetLongFilename( fatFile* in_file, sysString out_buffer, uint16_t in_buffer_length )
{
	uint16_t last_char_pos;
	uint16_t entry_pos;
	fatFile file = *in_file;
	uint8_t checksum = 0;
	uint16_t pos;
	uint8_t i;
	uint16_t ch;
	bool found = false;
	uint8_t name_checksum;

	// read sector
	fatReadDirectorySector( &file );
	last_char_pos = 0;

	while( true )
	{
		// position of the entry in the buffer
		entry_pos = in_file->DirectoryEntryIndex * fileFAT_DIRECTORY_ENTRY_LENGTH;

		// if entry is valid
		if( fatGetSystemSectorByte(entry_pos) != 0xe5 && fatGetSystemSectorByte(entry_pos) != 0x00 )
		{
			// and long entry
			if( (fatGetSystemSectorByte(entry_pos+11) & fileFAT_ATTR_LONG_NAME_MASK) == fileFAT_ATTR_LONG_NAME )
			{
				// last entry?
				if( (fatGetSystemSectorByte(entry_pos) & 0x40 ) != 0 )
				{
					checksum = fatGetSystemSectorByte(entry_pos+13);
					found = true;
				}
				else
				{
					// validate checksum
					if(fatGetSystemSectorByte(entry_pos+13) != checksum )
						return false;
				}

				// calculate position
				pos = ((fatGetSystemSectorByte(entry_pos) & ~0x40) - 1) * 13;

				// copy characters
				for( i = 1; i < fileFAT_DIRECTORY_ENTRY_LENGTH; i += 2 )
				{
					if( pos < in_buffer_length )
					{
						ch = fatGetSystemSectorWord(entry_pos+i);

						if( ch == 0xffff )
							out_buffer[pos] = '\0';
						else
							out_buffer[pos]	= strUnicodeToASCIIChar( ch );

						pos++;
					}

					switch( i )
					{
						case 9:
							i+=3;
							break;

						case 24:
							i+=2;
							break;
					}
				}

				if(pos > last_char_pos)
					last_char_pos = pos;

				in_file->DirectoryEntryIndex++;
				fatReadDirectoryEntry( in_file );
			}
			else
			{
				if( found )
				{
					// terminate name	(usually it is already terminated, but when the length is integer multiple of 13 it is not terminated)
					if(last_char_pos < in_buffer_length)
						out_buffer[last_char_pos] = '\0';

					// checksum computation
					name_checksum = 0;
					for( i = 0; i < fileFAT_MAX_FAT_FILENAME_LENGTH; i++)
						name_checksum = ((name_checksum & 1) ? 0x80 : 0) + (name_checksum >> 1) + fatGetSystemSectorByte(entry_pos+i);

					// validate checksum
					if( name_checksum != checksum )
						found = false;
				}

				return found;
			}
		}
		else
			return false;
	}
}

#else

///////////////////////////////////////////////////////////////////////////////
// Gets long file name
bool fatGetLongFilename( fatFile* in_file, char* out_buffer, uint16_t in_buffer_size )
{
	strCopyString(out_buffer, in_buffer_size, 0, in_file->ShortName);

	return false;
}

#endif

#if !defined(fileFAT_NO_LONG_FILENAME_SUPPORT) && !defined(fileFAT_READ_ONLY_FILESYSTEM)
///////////////////////////////////////////////////////////////////////////////
// Sets long filename
bool fatSetLongFilename( fatFile* in_file, sysString in_long_name )
{
	uint8_t entry_count;
	uint8_t entry_index;
	uint8_t available_entry_count;
	uint16_t directory_sector;
	fatClusterAddress directory_address;
	uint8_t name_checksum;
	char fat_filename[fileMAX_FAT_FILENAME_LENGTH];
	uint8_t i;
	fatFile* file;
	uint16_t entry_pos;

	// calculate entry count
	entry_count = ( strGetLength( in_long_name ) + 12 ) / 13;

	// if name is empty -> there is nothing to to
	if( entry_count == 0 )
		return true;

	// generate short name
	fatIntGenerateShortFilename( in_file, in_long_name );

	// convert name to fat filename
	fatIntCovertFilenameToFATFilename( in_file, fat_filename );

	// calculate schecksum 
	name_checksum = 0;
	for( i = 0; i < fileMAX_FAT_FILENAME_LENGTH; i++)
		name_checksum = ((name_checksum & 1) ? 0x80 : 0) + (name_checksum >> 1) + fat_filename[i];

	// try to find free entries
	file = in_file;
	file->DirectoryEntryIndex = 0;
	available_entry_count = 0;
	while( available_entry_count < entry_count )
	{
		// if no more entries
		if( !fatIntReadDirectoryEntry( file ) )
			break;

		// count free entries
		entry_pos = file->DirectoryEntryIndex * fileFAT_DIRECTORY_ENTRY_LENGTH;
		if( l_data_sector_buffer[entry_pos] == 0xe5 || l_data_sector_buffer[entry_pos] == 0x00 )
			available_entry_count++;

		file->DirectoryEntryIndex++;
	}

	return true;
}
#else
///////////////////////////////////////////////////////////////////////////////
// Sets long filename
bool fatSetLongFilename( fatFile* in_file, sysString in_buffer )
{
	return false;
}

#endif

#ifndef fileFAT_NO_LONG_FILENAME_SUPPORT
///////////////////////////////////////////////////////////////////////////////
// Generate short filename
static void fatGenerateShortFilename( fatFile* in_file, sysString in_long_name )
{
	bool lossy = false;
	char ch;
	uint16_t i;
	uint8_t j;
	uint8_t k;
	uint16_t last_dot;
	bool extension_exists;
	uint32_t number;
	int16_t pos;
	sysChar buffer[8];

	i = 0;
	j = 0;

	// create basis name
	while( in_long_name[i] != '\0' && j < 8 )
	{
		// convert character to uppercase ascii character
		ch = strUnicodeToASCIIChar( strCharToUpper( in_long_name[i] ));

		// break at the first '.' character
		if( ch == '.' )
			break;

		// skip spaces
		if( ch != ' ' )
		{
			// replace any invalid character
			if( !fatIsValidFilenameCharacter( ch ) )
			{
				ch = '_';
				lossy = true;
			}

			// store character
			in_file->ShortName[j++] = ch;
		}

		i++;
	}

	// create extension

	// search for the last dot
	last_dot = 0xffff;
	while( in_long_name[i] != '\0' )
	{
		// store position of the last dot
		if( in_long_name[i] == '.' )
			last_dot = i;

		i++;
	}

	// if dot found and there are some more characters after it -> create extension
	if( last_dot != 0xffff )
	{
		i = last_dot + 1;
		extension_exists = false;

		// copy extension
		k = 0;
		while( in_long_name[i] != '\0' && k < 3 )
		{
			// convert character to uppercase ascii character
			ch = strUnicodeToASCIIChar( strCharToUpper( in_long_name[i] ));

			// skip spaces
			if( ch != ' ' )
			{
				// replace any invalid character
				if( !fatIsValidFilenameCharacter( ch ) )
					ch = '_';

				// store dot
				if( !extension_exists )
				{
					extension_exists = true;
					in_file->ShortName[j++] = '.';
				}

				// store character
				in_file->ShortName[j+k] = ch;

				k++;
			}

			i++;
		}

		j = j + k;
	}

	// terminate short name
	if( j < fileFAT_MAX_SHORT_FILENAME_LENGTH )
		in_file->ShortName[j] = '\0';

	// generate numeric tail
	if( lossy || i >= fileFAT_MAX_SHORT_FILENAME_LENGTH )
	{
		// find the position of the dot
		j = 0;

		while(	j < fileFAT_MAX_FAT_FILENAME_LENGTH &&
						in_file->ShortName[j] != '\0' && 
						in_file->ShortName[j] != '.' )
			j++;

		number = 1;

		do
		{
			// update name
			if( number < 10 )
				pos = j - 2;
			else
				if( number < 100 )
					pos = j - 3;
				else 
					if( number < 1000 )
						pos = j - 4;
					else
						if( number < 10000 )
							pos = j - 5;
						else
							if( number < 100000 )
								pos = j - 6;
							else
								pos = j - 7;

			// if there is not enough space before dot -> shift the string
			if( pos < 0 )
			{
				k = fileFAT_MAX_SHORT_FILENAME_LENGTH - 2;

				while( k > 0 )
				{
					in_file->ShortName[k] = in_file->ShortName[k-pos];
					k++;
				}
			}

			// store tilde
			in_file->ShortName[pos] = '~';

			// store number
			pos = strDWordToStringPos(in_file->ShortName, fileFAT_MAX_FAT_FILENAME_LENGTH, pos, number, 0, 0, 0);
			pos = strCopyString(in_file->ShortName, fileFAT_MAX_FAT_FILENAME_LENGTH, pos, (sysString)buffer);

			number++;
		} while( number < 1000000 && fatIsFileExists( in_file, in_file ) );
	}
}
#endif

#pragma endregion

///////////////////////////////////////////////////////////////////////////////
// Internal Functions
///////////////////////////////////////////////////////////////////////////////
#pragma region Internal Functions

///////////////////////////////////////////////////////////////////////////////
/// @brief Flushes the content of the file data sector buffer to the storage media
static void fatFlushDataSector( fatFile* in_file )
{
#ifndef fileFAT_READ_ONLY_FILESYSTEM
	#if filefileFAT_BUFFERING_MODE == fileFAT_BM_DIRECTORY_AND_FILE
	// return if not modified
	if( !l_file_sector_buffer.Modified )
		return;

	// write sector
	drvMSWriteSector( l_file_sector_buffer.Buffer, l_file_sector_buffer.LBA );

	// clear flag
	l_file_sector_buffer.Modified = false;
	#endif
#endif
}

///////////////////////////////////////////////////////////////////////////////
// Skip long file name entries
#ifndef fileFAT_NO_LONG_FILENAME_SUPPORT
bool fatIntSkipLongFilenameEntries( fatFile* in_file )
{
	uint16_t entry_pos;

	// read sector
	fatReadDirectorySector( in_file );

	while( true )
	{
		// position of the entry in the buffer
		entry_pos = in_file->DirectoryEntryIndex * fileFAT_DIRECTORY_ENTRY_LENGTH;

		// if entry is valid
		if( fatGetSystemSectorByte(entry_pos) != 0xe5 && fatGetSystemSectorByte(entry_pos) != 0x00 )
		{
			// and long entry
			if( (fatGetSystemSectorByte(entry_pos+11) & fileFAT_ATTR_LONG_NAME_MASK) == fileFAT_ATTR_LONG_NAME )
			{
				in_file->DirectoryEntryIndex++;
				fatReadDirectoryEntry( in_file );
			}
			else
				return true;
		}
		else
			return false;
	}
}
#endif

///////////////////////////////////////////////////////////////////////////////
// Is Valid Filename Character
bool fatIsValidFilenameCharacter( char in_char )
{
	switch( in_char )
	{
		// invalid characters
		case 0x22:
		case 0x2A:
		case 0x2B:
		case 0x2C:
		case 0x2F:
		case 0x3A:
		case 0x3B:
		case 0x3C:
		case 0x3D:
		case 0x3E:
		case 0x3F:
		case 0x5B:
		case 0x5C:
		case 0x5D:
			return false;

		default:
			if( in_char <= 0x20 || in_char > 0x7f )
				return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
// Convert filename to FAT filename
bool fatIntCovertFilenameToFATFilename( fatFile* in_file, uint8_t* out_filename )
{
	uint8_t i;
	uint8_t pos;
	uint8_t ch;

	// check filename
	if( in_file->ShortName[0] == '\0' || in_file->ShortName[0] == '.' )
		return false;

	// convert filename to the FAT format
	pos = 0xff;
	for( i = 0; i < fileFAT_MAX_SHORT_FILENAME_LENGTH; i++ )
	{
		ch = in_file->ShortName[i];

		if( ch == 0 )
			break;

		if( fatIsValidFilenameCharacter( ch ) )
		{
			if( ch >= 'a' && ch <='z' )
				ch = ch - 'a' + 'A';

			if( ch == '.' )
			{
				if( pos < 8 )
				{
					while( pos < 7 )
					{
						pos++;
						out_filename[pos] = ' ';
					}
					continue;
				}
				else
					return false;
			}

			pos++;

			if( pos >= 11 )
				return false;

			out_filename[pos] = ch;
		}
		else
			return false;
	}

	while( pos < 10 )
	{
		pos++;
		out_filename[pos] = ' ';
	}

	return true;
}

#if 0
///////////////////////////////////////////////////////////////////////////////
//! Reads file sector
//! \param LBA of the sector to read
//! \return true sector is loaded into the buffer
bool fatReadFileSector( uint32_t in_lba )
{
#if filefileFAT_BUFFERING_MODE == fileFAT_BM_SINGLE
	return fatReadSystemSector(in_lba);
#elif filefileFAT_BUFFERING_MODE == fileFAT_BM_SYSTEM_AND_FILE
	// if it's already in the buffer don't do anything
	if( l_file_sector_buffer.LBA == in_lba )
		return true;

	// flush the content of the current buffer
	fatFlushFileSectorBuffer();

	// read sector
	l_file_sector_buffer.LBA = in_lba;

	return drvMSReadSector( l_file_sector_buffer.LBA, in_lba );
#else
	#error Invalid buffer mode
#endif
}

///////////////////////////////////////////////////////////////////////////////
// Read Sector (internal use)
bool fatReadSystemSector( uint32_t in_lba )
{
	if( l_lba_of_sector_in_buffer == in_lba )
		return true;

	l_lba_of_sector_in_buffer = in_lba;

	return msReadSector( g_sector_buffer, in_lba );
}

///////////////////////////////////////////////////////////////////////////////
// Read one sector of cluster
bool fatIntReadSectorOfCluster( fatClusterAddress in_cluster_address, uint8_t in_sector )
{
	uint32_t lba;

	lba = ((in_cluster_address - 2) * l_sectors_per_cluster) + l_first_data_sector + in_sector;

	return fatIntReadSector( lba );
}

#endif

#pragma endregion

#pragma region Data access routines
// <editor-fold defaultstate="collapsed" desc="Data access routines">
/*****************************************************************************/
/* Data access routines                                                      */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
//! Gets word from system sector buffer without causing access violation
static uint8_t fatGetSystemSectorByte(uint16_t in_byte_index)
{
	return fileFAT_SYSTEM_SECTOR_BUFFER.Buffer[in_byte_index];
}

///////////////////////////////////////////////////////////////////////////////
//! Gets word from system sector buffer without causing access violation
static uint16_t fatGetSystemSectorWord(uint16_t in_byte_index)
{
	uint16_t retval;

	retval = fileFAT_SYSTEM_SECTOR_BUFFER.Buffer[in_byte_index];
	retval += ((uint16_t)fileFAT_SYSTEM_SECTOR_BUFFER.Buffer[in_byte_index+1]) << 8;

	return retval;
}

///////////////////////////////////////////////////////////////////////////////
//! Gets double word from system sector buffer without causing access violation
static uint32_t fatGetSystemSectorDWord(uint16_t in_byte_index)
{
	uint32_t retval;

	retval = fileFAT_SYSTEM_SECTOR_BUFFER.Buffer[in_byte_index];
	retval += ((uint32_t)fileFAT_SYSTEM_SECTOR_BUFFER.Buffer[in_byte_index+1]) << 8;
	retval += ((uint32_t)fileFAT_SYSTEM_SECTOR_BUFFER.Buffer[in_byte_index+2]) << 16;
	retval += ((uint32_t)fileFAT_SYSTEM_SECTOR_BUFFER.Buffer[in_byte_index+3]) << 24;

	return retval;
}
// </editor-fold>
#pragma endregion
