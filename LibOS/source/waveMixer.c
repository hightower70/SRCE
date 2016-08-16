/*****************************************************************************/
/* Multiple channel audio sample mixer                                       */
/*                                                                           */
/* Copyright (C) 2015 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

/*****************************************************************************/
/* Include                                                                   */
/*****************************************************************************/
#include <waveMixer.h>
#include <drvWavePlayer.h>
#include "sysConfig.h"

/*****************************************************************************/
/* Local function prototypes                                                 */
/*****************************************************************************/
static void drvWavePlayerUnlinkChannelFromPlay(waveMixerState* in_state, uint8_t in_channel);

/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Initializes wave mixer
void waveMixerInitialize(waveMixerState* in_state)
{
  uint8_t channel;

	in_state->FirstActiveChannel = waveMIXER_INVALID_CHANNEL;

  for(channel = 0; channel < waveMIXER_CHANNEL_COUNT; channel++)
  {
    in_state->ChannelState[channel].State = 0;
		in_state->ChannelState[channel].NextActiveChannel = waveMIXER_INVALID_CHANNEL;
  }

	drvWavePlayerInitialize();
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Plays wave file from the resource
/// @param in_channel_info Pointer to the mixel channel inormation array
/// @param in_resource_address Resource address of the wave resource to play
/// @param in_flags Flags for playing
/// @return Channel number which used for playing the wave or waveMIXER_INVALID_CHANNEL if there is no idle channel
uint8_t waveMixerPlayWaveFromResource(waveMixerState* in_state, sysResourceAddress in_resouce_adress, uint32_t in_flags)
{
	uint8_t channel;
	uint8_t format;
	uint32_t sample_count;
	uint32_t sample_rate;

	// find idle channel
	channel = 0;
	while(channel < waveMIXER_CHANNEL_COUNT)
	{
		if((in_state->ChannelState[channel].State & waveMIXER_CS_ACTIVE) == 0)
		{
			// get wave information
			format = drvResourceReadByte(in_resouce_adress);
			in_resouce_adress++;

			// decode format
			switch (format & waveWAVEPLAYER_SAMPLE_RATE_MASK)
			{
			case waveWAVEPLAYER_FORMAT_8000HZ:
				sample_rate = 8000;
				break;

			case waveWAVEPLAYER_FORMAT_11025HZ:
				sample_rate = 11025;
				break;

			case waveWAVEPLAYER_FORMAT_22050HZ:
				sample_rate = 22050;
				break;

			case waveWAVEPLAYER_FORMAT_44100HZ:
				sample_rate = 44100;
				break;

			default:
				sample_rate = 44100;
				break;
			}

			// get sample count
			sample_count = drvResourceReadDWord(in_resouce_adress);
			in_resouce_adress += 4;

			// idle channel found -> update channel info
			in_state->ChannelState[channel].State = in_flags | waveMIXER_CS_ACTIVE;
			in_state->ChannelState[channel].Samples = drvGetResourcePhysicalAddress(in_resouce_adress );
			in_state->ChannelState[channel].SamplesCount = sample_count;
			in_state->ChannelState[channel].SampleRate = sample_rate;
			in_state->ChannelState[channel].Position = 0;
			in_state->ChannelState[channel].SampleRateAccumulator = 0;
			in_state->ChannelState[channel].NextActiveChannel = in_state->FirstActiveChannel;
			in_state->FirstActiveChannel = channel;

			return channel;
		}

		channel++;
	}

	return waveMIXER_INVALID_CHANNEL;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Stops wave playing on the given channel
/// @param in_channel_info Pointer to the mixel channel inormation array
/// @param in_channel_to_stop Channel index to stop
void waveMixerStopWave(waveMixerState* in_state, uint8_t in_channel_to_stop)
{
	// check if channel is valid
	if(in_channel_to_stop < waveMIXER_CHANNEL_COUNT)
	{
		// stop channel
		in_state->ChannelState[in_channel_to_stop].State &= ~waveMIXER_CS_ACTIVE;
		drvWavePlayerUnlinkChannelFromPlay(in_state, in_channel_to_stop);
	}
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Renders audio stream, mixes all active channels and resamples if needed
/// @param in_channel_info Pointer to the mixel channel inormation array
/// @param in_reder_buffer Buffer to render the stream
/// @param in_sample_count Number of samples to render
void waveMixerRenderStream(waveMixerState* in_state, drvWavePlayerBufferType* in_render_buffer, uint32_t in_sample_count)
{																																	  
	uint8_t next_channel_index;
	uint8_t current_channel_index;
	int32_t sample_sum = 0;
	waveMixerChannelState* current_channel;

	// render the specified number of samples
	while(in_sample_count > 0)
	{
		// render all channels
	  sample_sum = 0;
		current_channel_index = in_state->FirstActiveChannel;
		while(current_channel_index != waveMIXER_INVALID_CHANNEL)
		{
			current_channel = &in_state->ChannelState[current_channel_index];
			next_channel_index = current_channel->NextActiveChannel;

			// calculate next sample pos
			current_channel->SampleRateAccumulator += current_channel->SampleRate;

			while(current_channel->SampleRateAccumulator >= drvWAVEPLAYER_SAMPLE_RATE)
			{
				// next sample
				current_channel->Position++;

				// check for samples end
				if(current_channel->Position >= current_channel->SamplesCount)
				{
					// all samples played -> check for loop flag
					if((current_channel->State & waveMIXER_CS_LOOP_ENABLED) != 0)
					{
						// playback is looped -> restart
						current_channel->Position = 0;
					}
					else
					{
						// no loopback enabled -> stop playback of this channel
						current_channel->State &= ~waveMIXER_CS_ACTIVE;
						next_channel_index = current_channel->NextActiveChannel;
						drvWavePlayerUnlinkChannelFromPlay(in_state, current_channel_index);
						break;
					}
				}

				current_channel->SampleRateAccumulator -= drvWAVEPLAYER_SAMPLE_RATE;
			}

			// check if channel is still active
			if((current_channel->State & waveMIXER_CS_ACTIVE) != 0)
			{
				// sum sample
				sample_sum += *(((uint8_t*)(current_channel->Samples)) + current_channel->Position) - 127;
			}

			// next active channel
			current_channel_index = next_channel_index;
		}

		// store sample
		*in_render_buffer = sample_sum * drvWAVEPLAYER_SAMPLE_MULTIPLIER + drvWAVEPLAYER_SAMPLE_OFFSET;
		in_render_buffer++;

		// next rendered sample
		in_sample_count--;
  }
}


///////////////////////////////////////////////////////////////////////////////
/// @brief Unlinks (stops playback) of the selected channel from the play list of channel
/// @param in_channel_info Pointer to the mixel channel inormation array
/// @param in_channel Channel index to unlink
static void drvWavePlayerUnlinkChannelFromPlay(waveMixerState* in_state, uint8_t in_channel)
{
	uint8_t channel;

	if(in_channel == in_state->FirstActiveChannel)
	{
		in_state->FirstActiveChannel = in_state->ChannelState[in_channel].NextActiveChannel;
	}
	else
	{
		channel = in_state->FirstActiveChannel;

		while(in_channel != in_state->ChannelState[channel].NextActiveChannel)
		{
			channel = in_state->ChannelState[channel].NextActiveChannel;
		}

		if(channel != waveMIXER_INVALID_CHANNEL)
		{
			in_state->ChannelState[channel].NextActiveChannel = in_state->ChannelState[in_channel].NextActiveChannel;
		}
	}

	in_state->ChannelState[in_channel].NextActiveChannel = waveMIXER_INVALID_CHANNEL;
}
