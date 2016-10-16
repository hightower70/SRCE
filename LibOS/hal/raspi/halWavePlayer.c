/*****************************************************************************/
/* Wave player (Linux ALSA driver)                                           */
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
#include <alsa/asoundlib.h>
#include <halWavePlayer.h>
#include <sysConfig.h>

/*****************************************************************************/
/* Types                                                                     */
/*****************************************************************************/

/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/
static snd_pcm_t *pcm_handle = NULL; // Our device handle 
static const char *device_name = "default"; // The device name
static int16_t l_buffer[halWAVEPLAYER_BUFFER_LENGTH];

/*****************************************************************************/
/* Function implentation                                                    */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Opens wave output device
void halWavePlayerInitialize(void)
{
	int err; 
	snd_pcm_hw_params_t *hw_params;
	snd_pcm_sw_params_t *sw_params;
	snd_pcm_uframes_t buffer_size = halWAVEPLAYER_BUFFER_LENGTH * sizeof(uint16_t);
	snd_pcm_uframes_t period_size = buffer_size / 2;	
	unsigned int sample_rate = halWAVEPLAYER_SAMPLE_RATE;
	
	// Open the device 
	if ((err = snd_pcm_open(&pcm_handle, device_name, SND_PCM_STREAM_PLAYBACK, 0)) < 0) 
	{
		fprintf(stderr, "Cannot open audio device %s (%s)\n", device_name, snd_strerror(err));
		exit(1);
	}

	////////////////////
	// HW param settings
	
	if ((err = snd_pcm_hw_params_malloc(&hw_params)) < 0) 
	{
		fprintf(stderr, "Cannot allocate hardware parameter structure (%s)\n", snd_strerror(err));
		exit(1);
	}
				 
	if ((err = snd_pcm_hw_params_any(pcm_handle, hw_params)) < 0)
	{
		fprintf(stderr, "Cannot initialize hardware parameter structure (%s)\n", snd_strerror(err));
		exit(1);
	}
	
	if ((err = snd_pcm_hw_params_set_access(pcm_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) 
	{
		fprintf(stderr,	"Cannot set access type (%s)\n", snd_strerror(err));
		exit(1);
	}
	
	if ((err = snd_pcm_hw_params_set_format(pcm_handle, hw_params, SND_PCM_FORMAT_S16_LE)) < 0) 
	{
		fprintf(stderr, "Cannot set sample format (%s)\n", snd_strerror(err));
		exit(1);
	}
	
	if ((err = snd_pcm_hw_params_set_rate_near(pcm_handle, hw_params, &sample_rate, 0)) < 0) 
	{
		fprintf(stderr,	"Cannot set sample rate (%s)\n", snd_strerror(err));
		exit(1);
	}
	
	if ((err = snd_pcm_hw_params_set_channels(pcm_handle, hw_params, 1)) < 0) 
	{
		fprintf(stderr, "Cannot set channel count (%s)\n", snd_strerror(err));
		exit(1);
	}
	
	if ((err = snd_pcm_hw_params_set_buffer_size_near(pcm_handle, hw_params, &buffer_size)) < 0) 
	{
		fprintf(stderr, "Cannot set buffer size (%s)\n", snd_strerror(err));
		exit(1);
	}

	if ((err = snd_pcm_hw_params_set_period_size_near(pcm_handle, hw_params, &period_size, NULL)) < 0) 
	{
		fprintf(stderr, "Cannot set period size (%s)\n", snd_strerror(err));
		exit(1);
	}

	if ((err = snd_pcm_hw_params(pcm_handle, hw_params)) < 0) 
	{
		fprintf(stderr, "Cannot set parameters (%s)\n", snd_strerror(err));
		exit(1);
	}

	snd_pcm_hw_params_free(hw_params);

	////////////
	// SW params
	if ((err = snd_pcm_sw_params_malloc(&sw_params)) < 0) 
	{
		fprintf(stderr,	"Cannot allocate software parameters structure (%s)\n",	snd_strerror(err));
		exit(1);
	}
	if ((err = snd_pcm_sw_params_current(pcm_handle, sw_params)) < 0) 
	{
		fprintf(stderr, "Cannot initialize software parameters structure (%s)\n", snd_strerror(err));
		exit(1);
	}
	if ((err = snd_pcm_sw_params_set_avail_min(pcm_handle, sw_params, period_size)) < 0) 
	{
		fprintf(stderr,	"Cannot set minimum available count (%s)\n", snd_strerror(err));
		exit(1);
	}
	if ((err = snd_pcm_sw_params_set_start_threshold(pcm_handle, sw_params, buffer_size - period_size)) < 0) 
	{
		fprintf(stderr,	"Cannot set start mode (%s)\n",	snd_strerror(err));
		exit(1);
	}
	if ((err = snd_pcm_sw_params(pcm_handle, sw_params)) < 0) 
	{
		fprintf(stderr,	"Cannot set software parameters (%s)\n", snd_strerror(err));
		exit(1);
	}
	
	snd_pcm_sw_params_free(sw_params);
		
	if ((err = snd_pcm_prepare(pcm_handle)) < 0) 
	{
		fprintf(stderr,	"Cannot prepare audio interface for use (%s)\n", snd_strerror(err));
		exit(1);
	}	
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Closes wave output device
void halWavePlayerCleanUp(void)
{
	if (pcm_handle != NULL)
	{
		snd_pcm_drop(pcm_handle);
		snd_pcm_close(pcm_handle);
	}
	
	pcm_handle = NULL;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Adds the specified buffer to the playback queue
/// @param in_buffer_index Buffer index to add to the queue
void halWavePlayerPlayBuffer(uint8_t in_buffer_index)
{
	snd_pcm_sframes_t  frames_written;
	
	if (in_buffer_index != 0)
		return;

	frames_written = snd_pcm_writei(pcm_handle, l_buffer, halWAVEPLAYER_BUFFER_LENGTH);
	if (frames_written == -EPIPE) 
	{
		snd_pcm_prepare(pcm_handle);
	}
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Gets next free buffer index
uint8_t halWavePlayerGetFreeBufferIndex(void)
{
	snd_pcm_sframes_t  frames_free;
	
	frames_free = snd_pcm_avail(pcm_handle);
	if (frames_free > halWAVEPLAYER_BUFFER_LENGTH)
		return 0;
	
	if (frames_free > 0)
		return halWAVEPLAYER_INVALID_BUFFER_INDEX;
	
	if (frames_free == -EPIPE) 
	{
		snd_pcm_prepare(pcm_handle);
		return 0;
	}
	
	return halWAVEPLAYER_INVALID_BUFFER_INDEX;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Gets pointer to the wave data section of the given buffer
/// @param in_buffer_index Index of the wave buffer
/// @return Wave data pointer or null if index is invalid
halWavePlayerBufferType* halWaveGetBuffer(uint8_t in_buffer_index)
{
	if (in_buffer_index == 0)
		return l_buffer;
	else
		return sysNULL;
}

