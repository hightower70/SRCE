/*****************************************************************************/
/* Wave player (Win32 wave driver)                                           */
/*                                                                           */
/* Copyright (C) 2015 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <Windows.h>
#include <halWavePlayer.h>

#pragma comment(lib,"Winmm.lib") // Winsock Library

/*****************************************************************************/
/* Types                                                                     */
/*****************************************************************************/

// wave output buffer
typedef struct
{
	bool      Free;
	WAVEHDR    Header;
  halWavePlayerBufferType Buffer[halWAVEPLAYER_BUFFER_LENGTH];
} WaveOutBuffer;

/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/
static HWAVEOUT l_waveout_handle = NULL;
static HANDLE l_waveout_event = NULL;
static WaveOutBuffer l_waveout_buffer[halWAVEPLAYER_BUFFER_COUNT];
static DWORD	l_thread_id			= 0;
static HANDLE	l_thread_handle = NULL;

/*****************************************************************************/
/* Function implentation                                                    */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Opens wave output device
void halWavePlayerInitialize(void)
{
	MMRESULT result;
  WAVEFORMATEX wave_format;
  bool success = true;
	int i;

  // prepare for opening
	ZeroMemory( &wave_format, sizeof(wave_format) );

	wave_format.wBitsPerSample		= 16;
	wave_format.wFormatTag				= WAVE_FORMAT_PCM;
	wave_format.nChannels 				= 1;
	wave_format.nSamplesPerSec		= halWAVEPLAYER_SAMPLE_RATE;
	wave_format.nAvgBytesPerSec		= wave_format.nSamplesPerSec * wave_format.wBitsPerSample / 8;
	wave_format.nBlockAlign 			= wave_format.wBitsPerSample * wave_format.nChannels / 8;

  // open device
  result = waveOutOpen( &l_waveout_handle, WAVE_MAPPER, &wave_format, (DWORD)l_waveout_event, 0, 0 );
	if( result != MMSYSERR_NOERROR )
		success = false;

  // prepare buffers
	if(success)
  {
		for(i = 0; i < halWAVEPLAYER_BUFFER_COUNT; i++)
		{
			ZeroMemory( &l_waveout_buffer[i].Header, sizeof( WAVEHDR ) );
			l_waveout_buffer[i].Header.dwBufferLength = sizeof(l_waveout_buffer[i].Buffer);
			l_waveout_buffer[i].Header.lpData         = (LPSTR)(l_waveout_buffer[i].Buffer);
			l_waveout_buffer[i].Header.dwFlags        = 0;
			l_waveout_buffer[i].Free                  = true;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Closes wave output device
void halWavePlayerCleanUp(void)
{
	int i;

	// close wave out device
	if(l_waveout_handle != NULL)
	{
		// stop wave out
		waveOutReset(l_waveout_handle);
		
		// reset buffers
		for(i = 0; i < halWAVEPLAYER_BUFFER_COUNT; i++)
		{
			if(!l_waveout_buffer[i].Free && (l_waveout_buffer[i].Header.dwFlags & WHDR_DONE) != 0)
			{
				// release header
				waveOutUnprepareHeader(l_waveout_handle, &l_waveout_buffer[i].Header, sizeof(WAVEHDR));
				l_waveout_buffer[i].Free = true;
				l_waveout_buffer[i].Header.dwFlags = 0;
			}
		}

		// close device
		waveOutClose(l_waveout_handle);

		l_waveout_handle = NULL;
	}
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Adds the specified buffer to the playback queue
/// @param in_buffer_index Buffer index to add to the queue
void halWavePlayerPlayBuffer(uint8_t in_buffer_index)
{
	// flag header
	l_waveout_buffer[in_buffer_index].Free = false;

  // prepare header
	waveOutPrepareHeader(l_waveout_handle, &l_waveout_buffer[in_buffer_index].Header, sizeof(WAVEHDR));

	// write header
	waveOutWrite(l_waveout_handle, &l_waveout_buffer[in_buffer_index].Header, sizeof(WAVEHDR));
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Gets next free buffer index
uint8_t halWavePlayerGetFreeBufferIndex(void)
{
	int i;

	// check for free buffer
	for(i = 0; i < halWAVEPLAYER_BUFFER_COUNT; i++)
	{
		if(l_waveout_buffer[i].Free)
		{
			// buffer is free
			return i;
		}
		else
		{
			// buffer is not flagged for free, check if driver already done with the buffer
			if((l_waveout_buffer[i].Header.dwFlags & WHDR_DONE) != 0)
			{
				// release header
				waveOutUnprepareHeader( l_waveout_handle, &l_waveout_buffer[i].Header, sizeof(WAVEHDR));
				l_waveout_buffer[i].Free = true;
				l_waveout_buffer[i].Header.dwFlags = 0;

				return i;
			}
		}
	}

	return halWAVEPLAYER_INVALID_BUFFER_INDEX;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Gets pointer to the wave data section of the given buffer
/// @param in_buffer_index Index of the wave buffer
/// @return Wave data pointer or null if index is invalid
halWavePlayerBufferType* halWaveGetBuffer(uint8_t in_buffer_index)
{
	if (in_buffer_index < halWAVEPLAYER_BUFFER_COUNT)
		return l_waveout_buffer[in_buffer_index].Buffer;
	else
		return sysNULL;
}

