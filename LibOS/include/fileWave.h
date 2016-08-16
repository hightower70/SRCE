/*****************************************************************************/
/* Wave file handler routines                                                */
/*                                                                           */
/* Copyright (C) 2015 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

#ifndef __fileWave_h
#define __fileWave_h

///////////////////////////////////////////////////////////////////////////////
// Include
#include <sysTypes.h>
#include <fileStandardFunctions.h>

///////////////////////////////////////////////////////////////////////////////
// Const 
#define fileWAVE_WAVE_BLOCK_LENGTH 65536

#define fileWAVE_RIFF_HEADER_CHUNK_ID		0x46464952 /* 'RIFF' */
#define fileWAVE_RIFF_HEADER_FORMAT_ID	0x45564157 /* 'WAVE' */
#define fileWAVE_CHUNK_ID_FORMAT				0x20746d66 /* 'fmt ' */
#define fileWAVE_CHUNK_ID_DATA					0x61746164 /* 'data' */

///////////////////////////////////////////////////////////////////////////////
// Wave file structs
#include <sysPackStart.h>

// RIFF file header
typedef struct
{
	uint32_t	ChunkID;		// 'RIFF'
	uint32_t ChunkSize;
	uint32_t Format;			// 'WAVE'
} fileWaveRIFFHeader;

// WAVE chunk header
typedef struct
{
	uint32_t	ChunkID;
	uint32_t ChunkSize;
} fileWaveChunkHeader;

// Wave format description
typedef struct
{
	uint16_t	AudioFormat;	// PCM = 1
	uint16_t	NumChannels;
	uint32_t	SampleRate;
	uint32_t	ByteRate;
	uint16_t	BlockAlign;
	uint16_t	BitsPerSample;
} fileWaveFormatChunk;

#include <sysPackEnd.h>

///////////////////////////////////////////////////////////////////////////////
// Global variables


///////////////////////////////////////////////////////////////////////////////
// Functions prototypes
fileStream* fileWaveOpen(sysString in_file_name, fileWaveFormatChunk* out_format_chunk);
bool WFReadSample(int32_t* out_sample);
void fileWaveClose(fileStream* in_stream);

#endif