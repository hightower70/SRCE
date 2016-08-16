/*****************************************************************************/
/* Double buffered audio sample playback for PIC32                           */
/*                                                                           */
/* Copyright (C) 2015 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
// Includes
#include <drvWavePlayer.h>
#include "sysConfig.h"

///////////////////////////////////////////////////////////////////////////////
// Module global variables
uint16_t WavePlayer_buffer1[WAVEPLAYER_BUFFER_LENGTH];
volatile WavePlayerBufferStatus_t WavePlayer_buffer1_used = WavePlayer_BS_Empty;
uint16_t WavePlayer_buffer2[WAVEPLAYER_BUFFER_LENGTH];
volatile WavePlayerBufferStatus_t WavePlayer_buffer2_used = WavePlayer_BS_Empty;


void drvWavePlayerInitialize(void)
{
}

void drvWavePlayerPlayBuffer(uint8_t in_buffer_index)
{
}

 uint8_t drvWavePlayerGetFreeBufferIndex(void)
{
return drvWAVEPLAYER_INVALID_BUFFER_INDEX;
}

drvWavePlayerBufferType* drvWaveGetBuffer(uint8_t in_buffer_index)
{
	return sysNULL;

}