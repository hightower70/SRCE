/*****************************************************************************/
/* Wave file handler routines                                                */
/*                                                                           */
/* Copyright (C) 2015 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
// Includes
#include <fileWave.h>
#include <fileStandardFunctions.h>

/*****************************************************************************/
/* Wave file read functions                                                  */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Opens wavefile for reading
/// @param in_file_name File name to open
/// @param out_format_chunk Wave file format information
/// @param Returns sysNULL when opening fails or fileStream struct if it was success
fileStream* fileWaveOpen(sysString in_file_name, fileWaveFormatChunk* out_format_chunk)
{
	bool data_chunk_found;
	bool success;
	fileWaveRIFFHeader riff_header;
	fileWaveChunkHeader chunk_header;
	fileSize pos;
	fileStream* wave_file = sysNULL;

	// open wave file
	data_chunk_found = false;
	success = true;
	wave_file = fileOpen(in_file_name, "rb");
	if (wave_file != sysNULL)
	{
		// load RIFF header
		if (success)
		{
			fileRead(&riff_header, sizeof(riff_header), 1, wave_file);

			if ((riff_header.ChunkID != fileWAVE_RIFF_HEADER_CHUNK_ID) || (riff_header.Format != fileWAVE_RIFF_HEADER_FORMAT_ID))
			{
				// Error: Invalid file format
				success = false;
			}
		}

		// process chunks
		while (success && !feof(wave_file) && !data_chunk_found)
		{
			// read chunk header
			if (fileRead(&chunk_header, sizeof(chunk_header), 1, wave_file) == 1)
			{
				pos = fileGetPosition(wave_file);

				switch (chunk_header.ChunkID)
				{
					// Format 'fmt ' chunk
				case fileWAVE_CHUNK_ID_FORMAT:
					fileRead(out_format_chunk, sizeof(fileWaveFormatChunk), 1, wave_file);
					break;

					// Data 'data' chunk
				case fileWAVE_CHUNK_ID_DATA:
					data_chunk_found = true;
					break;
				}

				// move to the next chunk
				if (!data_chunk_found)
					fileSetPosition(wave_file, pos + chunk_header.ChunkSize, SEEK_SET);
			}
		}
	}

	return wave_file;
}


///////////////////////////////////////////////////////////////////////////////
/// @brief Closes Wave file
/// @param in_strem Wave file stream to close
void fileWaveClose(fileStream* in_stream)
{
	fileClose(in_stream);
}


