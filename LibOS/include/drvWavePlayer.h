/*****************************************************************************/
/* Double buffered audio sample playback                                     */
/*                                                                           */
/* Copyright (C) 2015 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

#ifndef __drvWavePlayer_h
#define __drvWavePlayer_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysTypes.h>
#include "sysConfig.h"

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/

// Wave format constants
#define waveWAVEPLAYER_FORMAT_LENGTH_MASK 0x03
#define waveWAVEPLAYER_FORMAT_MONO  0x00
#define waveWAVEPLAYER_FORMAT_STEREO (1 << 2)
#define waveWAVEPLAYER_FORMAT_MONO_STEREO_MASK (1 << 2)
#define waveWAVEPLAYER_FORMAT_8BIT 0
#define waveWAVEPLAYER_FORMAT_16BIT (1 << 3)
#define waveWAVEPLAYER_FORMAT_8BIT_16BIT_MASK (1 << 3)
#define waveWAVEPLAYER_FORMAT_8000HZ 0x00
#define waveWAVEPLAYER_FORMAT_11025HZ 0x10
#define waveWAVEPLAYER_FORMAT_22050HZ 0x20
#define waveWAVEPLAYER_FORMAT_44100HZ 0x30
#define waveWAVEPLAYER_CUSTOM_SAMPLE_RATE 0xf0
#define waveWAVEPLAYER_SAMPLE_RATE_MASK 0xf0

// defaults for buffer constants
#if !defined(drvWAVEPLAYER_BUFFER_LENGTH)
#define drvWAVEPLAYER_BUFFER_LENGTH 2048
#endif

#if !defined(drvWAVEPLAYER_BUFFER_COUNT)
#define drvWAVEPLAYER_BUFFER_COUNT 2
#endif

#define drvWAVEPLAYER_INVALID_BUFFER_INDEX 0xff

/*****************************************************************************/
/* Types                                                                     */
/*****************************************************************************/
typedef enum
{
	drvWP_BS_Empty,
	drvWP_BS_Playing
} drvWavePlayerBufferStatus;

#if !defined(drvWavePlayerBufferType)
typedef int16_t drvWavePlayerBufferType;
#endif

/*****************************************************************************/
/* Function prototypes                                                       */
/*****************************************************************************/
void drvWavePlayerInitialize(void);
void drvWavePlayerCleanUp(void);

void drvWavePlayerPlayBuffer(uint8_t in_buffer_index);
uint8_t drvWavePlayerGetFreeBufferIndex(void);
drvWavePlayerBufferType* drvWaveGetBuffer(uint8_t in_buffer_index);

#endif
