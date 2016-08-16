/*****************************************************************************/
/* Mass storeage driver for file system 																		 */
/* (emulated on Win32 using disk image file)                                 */
/*                                                                           */
/* Copyright (C) 2014-2015 Laszlo Arvai                                      */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

#define _CRT_SECURE_NO_WARNINGS
#include <sysTypes.h>
#include <stdio.h>

static FILE* l_file;

void drvMSInitialize(void)
{
	//l_file = fopen("d:\\Projects\\FAT\\sd.bin", "r+b" );
	l_file = fopen("d:\\sd_bin.bin", "r+b" );
}

void drvMSCleanUp(void)
{
	if( l_file != NULL )
		fclose(l_file);

	l_file = NULL;
}

bool drvMSReadSector( uint8_t* out_buffer, uint32_t in_lba )
{
	fseek( l_file, in_lba * 512, SEEK_SET );

	return fread( out_buffer, 1, 512, l_file ) == 512;
}

bool drvMSWriteSector( uint8_t* in_buffer, uint32_t in_lba )
{
	fseek( l_file, in_lba * 512, SEEK_SET );

	return fwrite( in_buffer, 1, 512, l_file ) == 512;
}
