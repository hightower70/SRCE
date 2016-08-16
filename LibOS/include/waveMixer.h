/*****************************************************************************/
/* Multiple channel audio sample mixer                                       */
/*                                                                           */
/* Copyright (C) 2015 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

#ifndef __waveMixer_h
#define __waveMixer_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysTypes.h>
#include <drvWavePlayer.h>
#include <drvResources.h>

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/
#define waveMIXER_CHANNEL_COUNT 8
#define waveMIXER_INVALID_CHANNEL 0xff

#define waveMIXER_CS_ACTIVE (1<<31)
#define waveMIXER_CS_LOOP_ENABLED (1<<0)

/*****************************************************************************/
/* Types                                                                     */
/*****************************************************************************/
typedef struct
{
  uint32_t State;
  void* Samples;
  uint32_t SamplesCount;
  uint32_t SampleRate;
  uint32_t Position;
  uint32_t SampleRateAccumulator;
	uint8_t NextActiveChannel;
} waveMixerChannelState;


typedef struct
{
	waveMixerChannelState ChannelState[waveMIXER_CHANNEL_COUNT];
	uint8_t FirstActiveChannel;
} waveMixerState;

/*****************************************************************************/
/* Function prototypes                                                       */
/*****************************************************************************/
void waveMixerInitialize(waveMixerState* in_state);

uint8_t waveMixerPlayWaveFromResource(waveMixerState* in_state, sysResourceAddress in_resouce_adress, uint32_t in_flags);
void waveMixerStopWave(waveMixerState* in_state, uint8_t in_channel_to_stop);

void waveMixerRenderStream(waveMixerState* in_state, drvWavePlayerBufferType* in_render_buffer, uint32_t in_sample_count);

#endif
