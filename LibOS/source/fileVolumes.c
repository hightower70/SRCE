/*****************************************************************************/
/*                                                                           */
/*    Domino Operation System Driver Module                                  */
/*    File System Routines                                                   */
/*                                                                           */
/*    Copyright (C) 2006-2014 Laszlo Arvai                                   */
/*                                                                           */
/*    ------------------------------------------------------------------     */
/*    Drives and Volumes handling routines                                   */
/*****************************************************************************/
#include <fileVolumes.h>		
#include <drvMassStorage.h>

bool fileReadSector( uint8_t in_volume_index, uint8_t* out_buffer, uint32_t in_lba )
{
	return drvMSReadSector( out_buffer, in_lba );
}

bool fileWriteSector( uint8_t in_volume_index, uint8_t* in_buffer, uint32_t in_lba )
{
	return drvMSWriteSector( in_buffer, in_lba );
}

#ifdef fileVOLUMES_MULTI_VOLUME_SUPPORT
uint8_t fileGetVolumeIndexFromDriveLetter(sysChar in_drive_letter)
{
}
#else
uint8_t fileGetVolumeIndexFromDriveLetter(sysChar in_drive_letter)
{
	if(g_file_volume_info.FATVolumeInfo.DriveLetter == in_drive_letter)
		return 0;
	else
		return fileVOLUMES_INVALID_INDEX;
}
#endif



#ifdef fileVOLUMES_MULTI_VOLUME_SUPPORT
void fileSetDriveLetterByVolumeIndex(uint8_t in_volume_index, dosChar in_drive_letter)
{
}
#else
void fileSetDriveLetterByVolumeIndex(uint8_t in_volume_index, sysChar in_drive_letter)
{
	if(in_volume_index == 0)
	{
		g_file_volume_info.FATVolumeInfo.DriveLetter = strCharToUpper(in_drive_letter);
	}
}
#endif




