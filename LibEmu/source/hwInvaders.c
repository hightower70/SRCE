/*****************************************************************************/
/* Taito Space Invaders Hardware Emulator                                    */
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
#include <sysVirtualKeyboardCodes.h>
#include <guiColorGraphics.h>
#include <cpuI8080.h>
#include <emuInvaders.h>
#include <emuInvadersResource.h>
#include <waveMixer.h>
#include <sysHighResTimer.h>
#include "sysConfig.h"

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/
#define emuINVADERS_FRAME_TIME (1000000 / emuINVADERS_FRAME_RATE) // frame time in us
#define emuINVADERS_CYCLES_PER_FRAME (emuINVADERS_CPU_CLOCK / emuINVADERS_FRAME_RATE) // number of CPU clock cycles per frame
#define emuINVADERS_MAX_DROPPED_FRAMES 60

/*****************************************************************************/
/* Global variables                                                          */
/*****************************************************************************/

// RAM memory
uint8_t g_cpu_ram[emuINVADERS_RAM_SIZE];  // 8k RAM

/*****************************************************************************/
/* Module local variables                                                    */
/*****************************************************************************/

// CPU variables
static cpuI8080State l_invaders_cpu;

// timing variables
static uint16_t l_current_scanline = 0;
static sysHighresTimestamp l_half_frame_timestamp;
static uint32_t l_cycles_per_frame;
static bool l_drop_next_frame;
static bool l_drop_frame;
static uint8_t l_droped_frame_counter;

// diagnostics variables
#ifdef emuDIAG_DISPLAY_STATISTICS
static sysHighresTimestamp l_statistics_timestamp;
static uint16_t l_frame_counter;
static uint16_t l_frame_rate;
static uint32_t l_cpu_cycles;
static uint32_t l_cpu_frequency;
static uint16_t l_cpu_load;
static uint32_t l_cpu_load_sum;
static uint16_t l_cpu_load_count;
#endif

// audio variables
static waveMixerState	l_wave_mixer_state;
static uint8_t l_ufo_sound_channel = waveMIXER_INVALID_CHANNEL;

/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Initializes emulator
void emuInvadersInitialize(void)
{
	uint16_t i;

  // Reset CPU
  cpuI8080Reset(&l_invaders_cpu);
  
  // Clears Invaders RAM
  for(i = 0; i < emuINVADERS_RAM_SIZE; i++)
		g_cpu_ram[i] = 0;

	// init screen
	guiDrawBitmapFromResource(0, 0, REF_BMP_BACKGROUND);
	emuInvadersRendererInitialize();

	guiRefreshScreen();

	// init wave
	waveMixerInitialize(&l_wave_mixer_state);

	// init variables
	l_current_scanline = 0;
	l_drop_next_frame = false;
	l_droped_frame_counter = 0;
	l_half_frame_timestamp = sysHighresTimerGetTimestamp();

#ifdef emuDIAG_DISPLAY_STATISTICS
	l_statistics_timestamp = sysHighresTimerGetTimestamp();
	l_frame_counter = 0;
	l_frame_rate = 0;
	l_cpu_cycles = 0;
	l_cpu_frequency = 0;
	l_cpu_load = 0;
	l_cpu_load_sum = 0;
	l_cpu_load_count = 0;
#endif

}

///////////////////////////////////////////////////////////////////////////////
/// @brief Emulator task
void emuInvadersTask(void)
{
	uint8_t free_wave_buffer_index;
	int expected_cycle_per_frame;
	int cycles_left;
	uint32_t half_frame_ellapsed_time;
#ifdef emuDIAG_DISPLAY_STATISTICS
	uint32_t ellapsed_statistics_time;
#endif

	// check if half frame time is elapsed
	if( sysHighresTimerGetTimeSince(l_half_frame_timestamp) >= emuINVADERS_FRAME_TIME / 2)
	{
		sysHighresTimerAddToTimestamp(&l_half_frame_timestamp, emuINVADERS_FRAME_TIME / 2);

		if(l_current_scanline == 0)
		{
			// reset cycle counter
			l_cycles_per_frame = 0;

			// handle frame drop
			if(l_droped_frame_counter >= emuINVADERS_MAX_DROPPED_FRAMES)
			{
				l_drop_next_frame = false;
				l_droped_frame_counter = 0;
			}

			l_drop_frame = l_drop_next_frame;

			if(l_drop_frame)
				l_droped_frame_counter++;

			// render the first half of the screen
			while(l_current_scanline  < emuINVADERS_SCREEN_WIDTH / 2)
			{
				// execute code
				expected_cycle_per_frame = (int)(l_current_scanline + 1) * emuINVADERS_CYCLES_PER_FRAME / emuINVADERS_SCREEN_WIDTH;
				cycles_left = cpuI8080Exec(&l_invaders_cpu, expected_cycle_per_frame - l_cycles_per_frame);
#ifdef emuDIAG_DISPLAY_STATISTICS
				l_cpu_cycles += expected_cycle_per_frame - l_cycles_per_frame - cycles_left;
#endif
				l_cycles_per_frame += expected_cycle_per_frame - l_cycles_per_frame - cycles_left;

				// render scanline
				if(!l_drop_frame)
					emuInvadersRenderScanLine(l_current_scanline);

				// next scanline
				l_current_scanline++;
			}

			// midscreen interrupt
			cpuI8080INT(&l_invaders_cpu, cpuI8080_RST1);
		}
		else
		{
			// render the second half of the screen
			while(l_current_scanline < emuINVADERS_SCREEN_WIDTH)
			{
				// execute code
				expected_cycle_per_frame = (int)(l_current_scanline + 1) * emuINVADERS_CYCLES_PER_FRAME / emuINVADERS_SCREEN_WIDTH;
				cycles_left = cpuI8080Exec(&l_invaders_cpu, expected_cycle_per_frame - l_cycles_per_frame);
#ifdef emuDIAG_DISPLAY_STATISTICS
				l_cpu_cycles += expected_cycle_per_frame - l_cycles_per_frame - cycles_left;
#endif
				l_cycles_per_frame += expected_cycle_per_frame - l_cycles_per_frame - cycles_left;

				// render scanline
				if(!l_drop_frame)
					emuInvadersRenderScanLine(l_current_scanline);

				// next scanline
				l_current_scanline++;
			}

			// vsync interrupt
			cpuI8080INT(&l_invaders_cpu, cpuI8080_RST2);

			// execute code for the vsync
			if(l_cycles_per_frame < emuINVADERS_CYCLES_PER_FRAME)
			{
				cpuI8080Exec(&l_invaders_cpu, emuINVADERS_CYCLES_PER_FRAME - l_cycles_per_frame);
			}

			// first scanline
			l_current_scanline = 0;

#ifdef emuDIAG_DISPLAY_STATISTICS
			if(!l_drop_frame)
				l_frame_counter++;
#endif

			// refresh content of the screen
			if(!l_drop_frame)
				guiRefreshScreen();
		}

		// check for frame drop
		half_frame_ellapsed_time = sysHighresTimerGetTimeSince(l_half_frame_timestamp);
		if(half_frame_ellapsed_time > (emuINVADERS_FRAME_TIME / 2 - 1000))
		{
			l_drop_next_frame = true;
		}
		else
		{
			l_drop_next_frame = false;
			l_droped_frame_counter = 0;
		}

#ifdef emuDIAG_DISPLAY_STATISTICS
		l_cpu_load_sum += half_frame_ellapsed_time * 1000 / (emuINVADERS_FRAME_TIME / 2);
		l_cpu_load_count++;
#endif
	}

	// diagnostics statistics
#ifdef emuDIAG_DISPLAY_STATISTICS
	ellapsed_statistics_time = sysHighresTimerGetTimeSince(l_statistics_timestamp);
	if(ellapsed_statistics_time > 1000000)
	{
		l_frame_rate = (uint16_t)((uint32_t)l_frame_counter * 10000000 / ellapsed_statistics_time); // calculate frame rate / 10
		l_cpu_frequency = (uint32_t)l_cpu_cycles * 1000 / ellapsed_statistics_time; // cpu clock * 1000

		if(l_cpu_load_count > 0)
			l_cpu_load = (uint16_t)(l_cpu_load_sum / l_cpu_load_count);
		else
			l_cpu_load = 0;

		emuDisplayStatistics(l_cpu_frequency, l_frame_rate, l_cpu_load);

		l_cpu_cycles = 0;
		l_frame_counter = 0;
		l_cpu_load_sum = 0;
		l_cpu_load_count = 0;

		l_statistics_timestamp = sysHighresTimerGetTimestamp();
	}
#endif

	// render audio stream
	free_wave_buffer_index = drvWavePlayerGetFreeBufferIndex();
	if(free_wave_buffer_index != waveMIXER_INVALID_CHANNEL)
	{
		waveMixerRenderStream(&l_wave_mixer_state, drvWaveGetBuffer(free_wave_buffer_index), drvWAVEPLAYER_BUFFER_LENGTH);
		drvWavePlayerPlayBuffer(free_wave_buffer_index);
	}
}

/*****************************************************************************/
/* Emulator details                                                          */
/*****************************************************************************/


/****************************************************************************
* M E M O R Y   I N T E R F A C E
*
* 0000 +----------------------------------------+
*      | ROM H                                  |
* 0800 +----------------------------------------+
*      | ROM G                                  |
* 1000 +----------------------------------------+
*      | ROM F                                  |
* 1800 +----------------------------------------+
*      | ROM E                                  |
* 2000 +----------------------------------------+
*      | 1Kb Work RAM                           |
* 2400 +----------------------------------------+
*      | 7kb Video RAM                          |
* 4000 +----------------------------------------+
*      | UNUSED                                 |
* FFFF +----------------------------------------+
*
****************************************************************************/

//--------------------------------------------------------------
// Memory write
//--------------------------------------------------------------
void WrI8080(register uint16_t in_address, register uint8_t in_value)
{
  //check range
  if (in_address < emuINVADERS_RAM_START || in_address > emuINVADERS_RAM_MIRROR + emuINVADERS_RAM_SIZE)
    return;

  // write RAM
  if(in_address > emuINVADERS_RAM_MIRROR)
  	in_address -= emuINVADERS_RAM_MIRROR;
  else
  	in_address -= emuINVADERS_RAM_START;

  // write RAM
  g_cpu_ram[in_address] = in_value;

  if(in_address > (emuINVADERS_VIDEO_RAM_START - emuINVADERS_RAM_START) )
  {
  	emuInvadersRenderPixels(in_address - (emuINVADERS_VIDEO_RAM_START - emuINVADERS_RAM_START), in_value);
  }
}

//--------------------------------------------------------------
// Memory read
//--------------------------------------------------------------
uint8_t RdI8080(register uint16_t in_address)
{
  // read ROM
  if (in_address < emuINVADERS_RAM_START)
    return g_cpu_rom[in_address];

  // read RAM
  if(in_address < emuINVADERS_RAM_MIRROR)
    return g_cpu_ram[in_address - emuINVADERS_RAM_START];
  else
  {
    if(in_address - emuINVADERS_RAM_MIRROR < emuINVADERS_RAM_SIZE)
      return g_cpu_ram[in_address - emuINVADERS_RAM_MIRROR];
    else
      return 0xff;
  }
}

/******************************************************************************
* P O R T S
******************************************************************************/

// Port 1 (input) - Bit Description
// 0 - Coin slot (1=coin inserted, automatically resetted after port read?)
// 1 - Two players button
// 2 - One player button
// 3 - always 1
// 4 - Player one - Fire button
// 5 - Player one - Left button
// 6 - Player one - Right button
// 7 - n/a
static volatile uint8_t l_port_in1 = 0x08;

// Port 2 (input) - Bit Description
// 0, 1 - DIP3, DIP5 00 = 3 ships 01 = 4 ships  10 = 5 ships 11 = 6 ships
// 2    - Tilt (0 = normal operation, 1 = tilt)
// 3    - DIP6 0 = extra ship at 1500, 1 = extra ship at 1000
// 4    - Player two - Fire button
// 5    - Player two - Left button
// 6    - Player two - Right button
// 7    - DIP switch: show/hide coin info (0=ON)
static volatile uint8_t l_port_in2 = 0x00;

// Ports used for hardware bit shifting
static uint8_t l_port_out2 = 0;
static uint8_t l_port_out4hi = 0;
static uint8_t l_port_out4lo = 0;

// Port 3 (output) - Bit  Description
// 0 - Spaceship (looped) sound
// 1 - Shot sound
// 2 - Base (your ship) hit sound
// 3 - Invader hit sound
// 4 - Extended play sound
// 5 - n/a
// 6 - n/a
// 7 - n/a
static uint8_t l_port_out3 = 0;

// Port 5 (output) - Bit  Description
// 0 - Invaders walk 1 sound
// 1 - Invaders walk 2 sound
// 2 - Invaders walk 3 sound
// 3 - Invaders walk 4 sound
// 4 - Spaceship hit sound
// 5 - Amplifier enabled/disabled
// 6 - n/a
// 7 - n/a
static uint8_t l_port_out5 = 0;

/****************************************************************************
* I N P U T  /  O U T P U T   P O R T S
****************************************************************************/

//--------------------------------------------------------------
// Port read
//--------------------------------------------------------------
uint8_t InI8080(register uint16_t in_port)
{
  uint8_t retval = 0xff;

  switch(in_port & 0xff)
  {
    case 0:
      retval = 0x0f;
      break;

    case 1:
      retval = l_port_in1;  // port 1 input in_value
      break;

    case 2:
      retval = l_port_in2;
      retval = 0;
      break;

    case 3:
      retval = (uint8_t)(((((uint32_t)l_port_out4hi << 8) | l_port_out4lo) << l_port_out2) >> 8);
      break;
  }

  return retval;
}

//--------------------------------------------------------------
// Port write
//--------------------------------------------------------------
void OutI8080(register uint16_t in_port,register uint8_t in_value)
{
  switch(in_port & 0xff)
  {
    case 2:
      l_port_out2 = in_value;
      break;

    case 3:
      // Port 3 controls some sounds
      if((in_value & 0x01) && !(l_port_out3 & 0x01))
      {
				l_ufo_sound_channel = waveMixerPlayWaveFromResource(&l_wave_mixer_state, REF_WAV_UFO, waveMIXER_CS_LOOP_ENABLED);
      }

      if(!(in_value & 0x01) && (l_port_out3 & 0x01))
      {
        waveMixerStopWave(&l_wave_mixer_state, l_ufo_sound_channel);
        l_ufo_sound_channel = waveMIXER_INVALID_CHANNEL;
      }

      if( (in_value & 0x02) && !(l_port_out3 & 0x02) ) waveMixerPlayWaveFromResource(&l_wave_mixer_state, REF_WAV_SHOT, 0);
      if( (in_value & 0x04) && !(l_port_out3 & 0x04) ) waveMixerPlayWaveFromResource(&l_wave_mixer_state, REF_WAV_BASE_HIT, 0);
      if( (in_value & 0x08) && !(l_port_out3 & 0x08) ) waveMixerPlayWaveFromResource(&l_wave_mixer_state, REF_WAV_INV_HIT, 0);
      l_port_out3 = in_value;
      break;

    case 4:
      l_port_out4lo = l_port_out4hi;
      l_port_out4hi = in_value;
      break;

    case 5:
        // Port 5 controls sounds
			if( (in_value & 0x01) && !(l_port_out5 & 0x01) ) waveMixerPlayWaveFromResource(&l_wave_mixer_state, REF_WAV_WALK1, 0);
      if( (in_value & 0x02) && !(l_port_out5 & 0x02) ) waveMixerPlayWaveFromResource(&l_wave_mixer_state, REF_WAV_WALK2, 0);
      if( (in_value & 0x04) && !(l_port_out5 & 0x04) ) waveMixerPlayWaveFromResource(&l_wave_mixer_state, REF_WAV_WALK3, 0);
      if( (in_value & 0x08) && !(l_port_out5 & 0x08) ) waveMixerPlayWaveFromResource(&l_wave_mixer_state, REF_WAV_WALK4, 0);
      if( (in_value & 0x10) && !(l_port_out5 & 0x10) ) waveMixerPlayWaveFromResource(&l_wave_mixer_state, REF_WAV_UFO_HIT, 0);
      l_port_out5 = in_value;
      break;
  }
}

//-----------------------------------------------------------------------------
// User input handler
//-----------------------------------------------------------------------------
void emuUserInputEventHandler(uint8_t in_device_number, sysUserInputEventCategory in_event_category, sysUserInputEventType in_event_type, uint32_t in_event_param)
{
	bool pressed = (in_event_category == sysUIEC_Pressed); 

	// handle keyboard events
	if(in_event_type == sysUIET_Key && (in_event_category == sysUIEC_Pressed || in_event_category == sysUIEC_Released) )
	{
		switch(in_event_param)
		{
			// 1 player button
			case sysVKC_1:
				if(pressed)
					l_port_in1 |= 0x04;
				else
		      l_port_in1 &= ~0x04;
				break;

			// 2 player button
			case sysVKC_2:
				if(pressed)
					l_port_in1 |= 0x02;
				else
					l_port_in1 &= ~0x02;
				break;

			// coin insert
			case sysVKC_3:
				if(pressed)
					l_port_in1 |= 0x01;
				else
					l_port_in1 &= ~0x01;
				break;

			// player 1&2 fire
			case sysVKC_SPACE:
				if(pressed)
				{
					l_port_in1 |= 0x10;
					l_port_in2 |= 0x10;
				}
				else
				{
					l_port_in1 &= ~0x10;
					l_port_in2 &= ~0x10;
				}
				break;

			// player 1&2 right
			case sysVKC_RIGHT | sysVKC_SPECIAL_KEY_FLAG:
				if(pressed)
				{
					l_port_in1 |= 0x40;
					l_port_in2 |= 0x40;
				}
				else
				{
					l_port_in1 &= ~0x40;
					l_port_in2 &= ~0x40;
				}
				break;

			// player 1&2 left
			case sysVKC_LEFT | sysVKC_SPECIAL_KEY_FLAG:
				if(pressed)
				{
					l_port_in1 |= 0x20;
					l_port_in2 |= 0x20;
				}
				else
				{
					l_port_in1 &= ~0x20;
					l_port_in2 &= ~0x20;
				}
				break;
		}
	}
}

