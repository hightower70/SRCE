/*****************************************************************************/
/* HT1080Z Emulator                                                          */
/*   main hardware emulation                                                 */
/*                                                                           */
/* Copyright (C) 2015-2016 Laszlo Arvai                                      */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <fileStandardFunctions.h>
#include <sysUserInput.h>
#include <sysVirtualKeyboardCodes.h>
#include <cpuZ80.h>
#include <emuHT1080.h>
#include <sysHighResTimer.h>
#include <sysConfig.h>
#include <sysTimer.h>
#include <appSettings.h>
#include <cpCodePages.h>
#include <fbFileBrowser.h>
#include <guiBlackAndWhiteGraphics.h>
#include <fbRenderer.h>

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/
#define emuHT1080_MAX_CYCLES_PER_SCANLINE ((emuHT1080_CPU_CLK + emuHT1080_HSYNC_FREQ - 1)/ emuHT1080_HSYNC_FREQ) // rounded up
#define emuHT1080_KEYBOARD_ROW_COUNT 8
#define emuHT1080_SCANLINE_IN_US (1000000 / emuHT1080_HSYNC_FREQ)

#define emuPORT_FF_MOTOR_ON_MASK (1<<2)
#define emuPORT_FF_SIGNAL_MASK (3)

#define emuPORT_FF_INPUT_MASK (1<<7)

#define emuCAS_CLOCK_PERIOD_MAX 2500 // max clock period length in us for cassette signal analysis
#define emuCAS_CLOCK_PERIOD_MIN 1500 // min clock period length in us for cassette signal analysis

/*****************************************************************************/
/* Types                                                                     */
/*****************************************************************************/
typedef enum
{
	emuCS_Idle,

	// save states
	emuCS_SaveStart,
	emuCS_SaveWaitForMotorStart,
	emuCS_SaveWaitForClock,
	emuCS_SaveWaitForData,

	// load states
	emuCS_LoadStart,
	emuCS_LoadClock,
	emuCS_LoadData
} emuCASState;

/*****************************************************************************/
/* Local functions                                                           */
/*****************************************************************************/
static uint32_t cpuGetEllapsedTimeSince(uint32_t in_timestamp);
static uint32_t cpuGetEllapsedTimeSinceInMicrosec(uint32_t in_timestamp);
static uint32_t cpuGetTimestamp(void);

static uint8_t emuPixelToCharacterX(guiCoordinate in_coord);
static uint8_t emuPixelToCharacterY(guiCoordinate in_coord);


static void emuCASMotorOn(void);
static void emuCASMotorOff(void);
static void emuCASOut(uint8_t in_pulse);
static void emuCASIn(void);


/*****************************************************************************/
/* Module variables                                                          */
/*****************************************************************************/

// RAM
uint8_t g_ram[emuHT1080_RAM_SIZE];
uint8_t g_video_ram[emuHT1080_VIDEO_RAM_SIZE];
uint8_t g_keyboard_ram[emuHT1080_KEYBOARD_ROW_COUNT];

// CPU
cpuZ80State l_cpu;

// external ROM file reference
extern const unsigned char ht_s1_basic_rom[];
extern const unsigned char ht_s1_basicexpansion_rom[];
extern const unsigned char MODEL1_rom[];
extern const unsigned char level1_rom[];

// timing variables
static uint32_t l_total_cpu_cycles;
static int32_t l_current_cycles_per_frame;
static int l_current_scanline;

static sysHighresTimestamp l_frame_start_timestamp;
static uint32_t l_current_scanline_time;

// port mirror variables
static uint8_t l_out_port_ff = 0;
static uint8_t l_in_port_ff = 0;

// cassette save variables
static emuCASState l_cas_state = emuCS_LoadStart;
static uint8_t l_cas_buffer;
static uint8_t l_cas_buffer_bit_count = 0;
static fileStream* l_cas_file = sysNULL;
static uint32_t l_cas_clock_timestamp;
static bool l_cas_motor_on = false;

// emulation speed variables
static uint32_t l_emulation_speed_cpu_cycles;
static uint32_t l_emulation_speed_vsync_cycles;
uint16_t g_emulation_speed_cpu_freq;
uint16_t g_emulation_speed_vsync_freq;

// screean area refresh disable array
uint8_t g_screen_no_refresh_area[emuHT1080_VIDEO_RAM_SIZE];

/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/

/*****************************************************************************/
/* Emulation routines                                                        */
/*****************************************************************************/
#pragma region - Emulation routines -

///////////////////////////////////////////////////////////////////////////////
/// @brief Initializes HT1080Z Computer
void emuInitialize(void)
{
	emuReset();
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Main emulation task
void emuTask(void)
{
	uint16_t cycles_to_execute;
	uint16_t cycles_executed;
	int32_t expected_cycle_per_frame;
	bool full_speed = g_application_settings.FullSpeed || (l_cas_motor_on && g_application_settings.FastCassetteOperation);

 	if (sysHighresTimerGetTimeSince(l_frame_start_timestamp) >= l_current_scanline_time || full_speed)
	{
		// calculate cycles to execute
		expected_cycle_per_frame = (emuHT1080_CPU_CLK * (l_current_scanline + 1)) / emuHT1080_HSYNC_FREQ;
		cycles_to_execute = (uint16_t)(expected_cycle_per_frame - l_current_cycles_per_frame);

		// run CPU
		cycles_executed = cpuExecute(&l_cpu, cycles_to_execute);
		l_current_cycles_per_frame += cycles_executed;
		l_total_cpu_cycles += cycles_executed;
		l_emulation_speed_cpu_cycles += cycles_executed;

		// render scanline
		if (l_current_scanline == emuHT1080_SCREEN_HEIGHT_IN_PIXEL)
		{
			emuHT1080EndScreenrefresh();
		}

		// next scanline
		l_current_scanline++;
		if (l_current_scanline >= emuHT1080_TOTAL_SCANLINE_COUNT)
		{
			// VSYNC -> restart screen rendering
			l_current_scanline = 0;
			l_current_cycles_per_frame = 0;
			l_emulation_speed_vsync_cycles++;
			emuHT1080StartScreenRefresh();
			l_frame_start_timestamp = sysHighresTimerGetTimestamp();
			l_current_scanline_time = 0;
		}

		// update time
		l_current_scanline_time = l_current_scanline * 1000000 / emuHT1080_HSYNC_FREQ;

		// handle cas input
		if (l_cas_state >= emuCS_LoadStart)
			emuCASIn();
	}

#if 0
	uint8_t free_wave_buffer_index;

	// check if half frame time is ellapsed
	if(sysHighresTimerGetTimeSince(l_half_frame_timestamp) >= emuINVADERS_FRAME_TIME / 2)
	{
		l_half_frame_timestamp = sysHighresTimerGetTimestamp();

		if(l_current_scanline == 0)
		{
			// reset cycle counter
			l_cycles_per_frame = 0;

			// render the first half of the screen
			while(l_current_scanline  < emuINVADERS_SCREEN_WIDTH / 2)
			{
				// execute code
				l_cycles_per_frame += emuINVADERS_CYCLES_PER_LINE;
				l_cycles_per_frame -= cpuI8080Exec(&l_invaders_cpu, emuINVADERS_CYCLES_PER_LINE);

				// render scanline
				if(frame == 0)
					emuInvadersRenderScanLine(l_current_scanline);

				// next scanline
				l_current_scanline++;
			}

			// midscreeninterrupt
			cpuI8080INT(&l_invaders_cpu, cpuI8080_RST1);
		}
		else
		{
			// render the second half of the screen
			while(l_current_scanline  < emuINVADERS_SCREEN_WIDTH)
			{
				// execute code
				l_cycles_per_frame += emuINVADERS_CYCLES_PER_LINE;
				l_cycles_per_frame -= cpuI8080Exec(&l_invaders_cpu, emuINVADERS_CYCLES_PER_LINE);

				// render scanline
				if(frame == 0)
					emuInvadersRenderScanLine(l_current_scanline);

				// next scanline
				l_current_scanline++;
			}

			if(frame == 2)
			{
				frame = 0;
			}
			else
			{
	//			frame++;
			}


			// vsync interrupt
			cpuI8080INT(&l_invaders_cpu, cpuI8080_RST2);

			// execure code for the vsync
			if(l_cycles_per_frame < emuINVADERS_CYCLES_PER_FRAME)
			{
				cpuI8080Exec(&l_invaders_cpu, emuINVADERS_CYCLES_PER_FRAME - l_cycles_per_frame);
			}

			// first scanline
			l_current_scanline = 0;

			// refresh content of the screen
			guiRefreshScreen();
		}
	}

	// render audio stream
	free_wave_buffer_index = drvWavePlayerGetFreeBufferIndex();
	if(free_wave_buffer_index != waveMIXER_INVALID_CHANNEL)
	{
		waveMixerRenderStream(l_wave_mixer_info, drvWaveGetBuffer(free_wave_buffer_index), drvWAVEPLAYER_BUFFER_LENGTH);
		drvWavePlayerPlayBuffer(free_wave_buffer_index);
	}
#endif
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Reset the computer (cold start)
void emuReset(void)
{
	uint16_t i;

	// Reset CPU
	cpuReset(&l_cpu);

	// Clears RAM
	for (i = 0; i < emuHT1080_RAM_SIZE; i++)
		g_ram[i] = 0;

	for (i = 0; i < emuHT1080_VIDEO_RAM_SIZE; i++)
	{
		g_video_ram[i] = 32;
		g_screen_no_refresh_area[i] = 0;
		emuHT1080RenderCharacter(i);
	}

	// init emulation variables
	l_cas_motor_on = false;
	l_total_cpu_cycles = 0;
	l_current_cycles_per_frame = 0;
	l_current_scanline = 0;
	l_emulation_speed_cpu_cycles = 0;
	l_emulation_speed_vsync_cycles = 0;
	g_emulation_speed_cpu_freq = 0;
	g_emulation_speed_vsync_freq = 0;

	if (l_cas_file != sysNULL)
	{
		fileClose(l_cas_file);
		l_cas_file = sysNULL;
	}
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Issues NMI (warm reset)
void emuNMI(void)
{
	cpuInt(&l_cpu, INT_NMI);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Defines an area where emulation screen refresh is prohibited
/// @param in_left Left character coorindate of the area (inclusive)
/// @param in_top Top character coorindate of the area (inclusive)
/// @param in_right Right character coorindate of the area (inclusive)
/// @param in_bottom Bottom character coorindate of the area (inclusive)
void emuDisableScreenRefresh(uint8_t in_left, uint8_t in_top, uint8_t in_right, uint8_t in_bottom)
{
	uint8_t row, column;

	for (row = in_top; row <= in_bottom; row++)
	{
		for (column = in_left; column <= in_right; column++)
		{
			g_screen_no_refresh_area[row * emuHT1080_SCREEN_WIDTH_IN_CHARACTER + column]++;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Reenables emulation screen refresh for the given area
/// @param in_left Left character coorindate of the area (inclusive)
/// @param in_top Top character coorindate of the area (inclusive)
/// @param in_right Right character coorindate of the area (inclusive)
/// @param in_bottom Bottom character coorindate of the area (inclusive)
void emuEnableScreenRefresh(uint8_t in_left, uint8_t in_top, uint8_t in_right, uint8_t in_bottom)
{
	uint8_t row, column;
	uint16_t address;

	for (row = in_top; row <= in_bottom; row++)
	{
		for (column = in_left; column <= in_right; column++)
		{
			address = row * emuHT1080_SCREEN_WIDTH_IN_CHARACTER + column;
			if (g_screen_no_refresh_area[address] > 0)
				g_screen_no_refresh_area[address]--;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Shows wait indicator (and disables emulator screen refresh)
void emuWaitIndicatorShow(void)
{
	guiCoordinate left, top, right, bottom;
	uint8_t scr_left, scr_top, scr_right, scr_bottom;

	fbRenderGetWaitIndicatorRect(&left, &top, &right, &bottom);

	scr_left = emuPixelToCharacterX(left);
	scr_top = emuPixelToCharacterY(top);
	scr_right = emuPixelToCharacterX(right);
	scr_bottom = emuPixelToCharacterY(bottom);

	emuDisableScreenRefresh(scr_left, scr_top, scr_right, scr_bottom);

	fbWaitIndicatorShow();
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Hides wait indicator (and enables emulator screen refresh)
void emuWaitIndicatorHide(void)
{
	guiCoordinate left, top, right, bottom;
	uint8_t scr_left, scr_top, scr_right, scr_bottom;

	fbRenderGetWaitIndicatorRect(&left, &top, &right, &bottom);

	scr_left = emuPixelToCharacterX(left);
	scr_top = emuPixelToCharacterY(top);
	scr_right = emuPixelToCharacterX(right);
	scr_bottom = emuPixelToCharacterY(bottom);

	emuEnableScreenRefresh(scr_left, scr_top, scr_right, scr_bottom);

	fbWaitIndicatorHide();
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Converts from pixel coordinates to character coordinates in X direction
/// @param in_coord Coordinate in pixels to convert
/// @return Converted (character) coordinate 
static uint8_t emuPixelToCharacterX(guiCoordinate in_coord)
{
	return (uint8_t )(in_coord / emuHT1080_CHARACTER_WIDTH);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Converts from pixel coordinates to character coordinates in Y direction
/// @param in_coord Coordinate in pixels to convert
/// @return Converted (character) coordinate 
static uint8_t emuPixelToCharacterY(guiCoordinate in_coord)
{
	return (uint8_t )(in_coord / emuHT1080_CHARACTER_HEIGHT);
}

#pragma endregion 

/*****************************************************************************/
/* Statistics routines                                                       */
/*****************************************************************************/
#pragma region - Statistics routines -
void emuResetStatistics(void)
{
	l_emulation_speed_cpu_cycles = 0;
	l_emulation_speed_vsync_cycles = 0;
}

void emuUpdateStatistics(uint32_t in_measured_inteval_in_ms)
{
	g_emulation_speed_vsync_freq = l_emulation_speed_vsync_cycles * 10000 / in_measured_inteval_in_ms;
	g_emulation_speed_cpu_freq = l_emulation_speed_cpu_cycles / 10 / in_measured_inteval_in_ms;
}
#pragma endregion

/*****************************************************************************/
/* Video routines                                                            */
/*****************************************************************************/
#pragma region - Video routines -
///////////////////////////////////////////////////////////////////////////////
/// @brief Render (refresh) the whole screen content
void emuRefreshScreen(void)
{
	uint16_t address;

	emuHT1080StartScreenRefresh();
	for (address = 0; address < emuHT1080_VIDEO_RAM_SIZE; address++)
		emuHT1080RenderCharacter(address);
	emuHT1080EndScreenrefresh();
}
#pragma endregion

/****************************************************************************
* M E M O R Y   I N T E R F A C E
****************************************************************************/
#pragma region - Memory handling -
/****************************************************************************
*
* 0000 +----------------------------------------+
*      | ROM (Basic interpreter)                |
* 3000 +----------------------------------------+
*      | ROM (Extension)                        |
* 3800 +----------------------------------------+
*      | Reserved for keyboard                  |
* 3C00 +----------------------------------------+
*      | Video display memory                   |
* 4000 +----------------------------------------+
*      | 16Kb RAM                               |
* 8000 +----------------------------------------+
*      | Expansion                              |
* FFFF +----------------------------------------+
*
****************************************************************************/

//--------------------------------------------------------------
// Memory write
//--------------------------------------------------------------
void cpuMemWrite(register uint16_t in_address, register uint8_t in_value)
{
	// write RAM
	if (in_address >= emuHT1080_RAM_START)
	{
		g_ram[in_address - emuHT1080_RAM_START] = in_value;
		if (in_address == 27686)
			in_address = 27686;
		return;
	}

	// write video ram
	if (in_address >= emuHT1080_VIDEO_RAM_START)
	{
		// character display
		//if ((in_value & 0xa0) != 0)
		//	in_value &= 0xbf;
		//else
		//	in_value |= 0x40;
		
		//if (in_value < 32)
		//	in_value |= 0x40;

		if ((in_value & 0x40) == 0)
		{
			if ((in_value & 0xa0) == 0)
				in_value |= 0x40;
		}

		if(g_video_ram[in_address - emuHT1080_VIDEO_RAM_START] != in_value)
		{
			g_video_ram[in_address - emuHT1080_VIDEO_RAM_START] = in_value;
			emuHT1080RenderCharacter(in_address - emuHT1080_VIDEO_RAM_START);
		}
		return;
	}

	// write to other area (e.g. ROM) is useless
}

//--------------------------------------------------------------
// Memory read
//--------------------------------------------------------------
uint8_t cpuMemRead(register uint16_t in_address)
{
	uint8_t data;
	uint8_t i;

	// read RAM
	if (in_address >= emuHT1080_RAM_START)
		return g_ram[in_address - emuHT1080_RAM_START];

	// read ROM
	if (in_address < emuHT1080_ROM_SIZE)
		return  ht_s1_basic_rom[in_address];//MODEL1_rom[in_address]; //level1_rom[in_address];  ht_s1_basic_rom[in_address];

	if (in_address >= emuHT1080_KEYBOARD_START)
	{
		if (in_address >= emuHT1080_VIDEO_RAM_START)
		{
			// video RAM
			return g_video_ram[in_address - emuHT1080_VIDEO_RAM_START];
		}
		else
		{
			data = 0;

			//  emulate wired or of keyboard rows
			for (i = 0; i < 8; i++)
			{
				if ((in_address & (1 << i)) != 0)
				{
					data |= g_keyboard_ram[i];
				}
			}

			return data;
		}
	}
	else
	{
		// expansion ROM
		return ht_s1_basicexpansion_rom[in_address - emuHT1080_EXTENSION_ROM_START];
	}
}
#pragma endregion

/******************************************************************************
* P O R T S
******************************************************************************/
#pragma region - Port handing -

//--------------------------------------------------------------
// Port read
//--------------------------------------------------------------
uint8_t cpuIn(register uint16_t in_port)
{
  uint8_t retval = 0xff;

  switch(in_port & 0xff)
  {
    case 0xff:
      retval = l_in_port_ff;
      break;
  }

  return retval;
}

//--------------------------------------------------------------
// Port write
//--------------------------------------------------------------
void cpuOut(register uint16_t in_port, register uint8_t in_value)
{
	switch (in_port & 0xff)
	{
		case 0xff:
			// check for motor state change
			if (((l_out_port_ff ^ in_value) & emuPORT_FF_MOTOR_ON_MASK) != 0)
			{
				// motor status changed, check new status
				if ((in_value & emuPORT_FF_MOTOR_ON_MASK) != 0)
				{
					emuCASMotorOn();
				}
				else
				{
					emuCASMotorOff();
				}
			}

			// check signal value
			if (((l_out_port_ff ^ in_value) & emuPORT_FF_SIGNAL_MASK) != 0)
			{
				if ((in_value & emuPORT_FF_MOTOR_ON_MASK) != 0)
				{
					emuCASOut(in_value & emuPORT_FF_SIGNAL_MASK);
				}
				else
				{
				}
			}

			// port write clears input value
			l_in_port_ff &= ~emuPORT_FF_INPUT_MASK;

			// store port value
			l_out_port_ff = in_value;
		break;

		case 0xfe:
		break;
	}
}
#pragma endregion

/****************************************************************************
* K E Y B O A R D  H A N D L I N G
****************************************************************************/
#pragma region - Keyboard handling -
/****************************************************************************
* Keyboard matrix:
*       d7 d6 d5 d6 d3 d2 d1 d0
* 3801   G  F  E  D  C  B  A  @
* 3802   O  N  M  L  K  J  I  H
* 3804   W  V  U  T  S  R  Q  P
* 3808   -  -  -  -  -  Z  Y  X
* 3810  7' 6& 5% 4$ 3# 2" 1!  0
* 3820  /? .> -= ,< ;+ :* 9) 8(
* 3840  sp ri le dn up br cl en
* 3880  -  -  -  -  -  -  -  sh
*
****************************************************************************/

#define KTE_INVALID   0
#define KTE_NO_MOD    1
#define KTE_SHIFT_ON  2
#define KTE_SHIFT_OFF 3

#define KTE(row, bit, modifier) (uint8_t)(((row << 5) | (bit << 2)) | modifier)

#define ROW(x) ((x) << 5)
#define BIT(x) ((x) << 2)

#define GET_ROW(x) (((x) >> 5) & 0x07)
#define GET_BIT(x) (((x) >> 2) & 0x07)
#define GET_MOD(x) ((x)& 0x03)

static const uint8_t l_keyboard_table[128] =
{
	KTE( 0, 0, KTE_INVALID),	//NUL
	KTE( 0, 0, KTE_INVALID),	//ctrl-A SOH
	KTE( 0, 0, KTE_INVALID),	//ctrl-B STX
	KTE( 0, 0, KTE_INVALID),	//ctrl-C ETX
	KTE( 0, 0, KTE_INVALID),	//ctrl-D EOT
	KTE( 0, 0, KTE_INVALID),	//ctrl-E ENQ
	KTE( 0, 0, KTE_INVALID),	//ctrl-F ACK
	KTE( 0, 0, KTE_INVALID),	//ctrl-G BEL
	KTE( 6, 5, KTE_NO_MOD),		//ctrl-H BS
	KTE( 0, 0, KTE_INVALID),	//ctrl-I TAB
	KTE( 0, 0, KTE_INVALID),	//ctrl-J LF
	KTE( 0, 0, KTE_INVALID),	//ctrl-K VT
	KTE( 0, 0, KTE_INVALID),	//ctrl-L FF
	KTE( 6, 0, KTE_NO_MOD),		//ctrl-M CR
	KTE( 0, 0, KTE_INVALID),	//ctrl-N SO
	KTE( 0, 0, KTE_INVALID),	//ctrl-O SI
	KTE( 0, 0, KTE_INVALID),	//ctrl-P DLE
	KTE( 0, 0, KTE_INVALID),	//ctrl-Q DC0
	KTE( 0, 0, KTE_INVALID),	//ctrl-R DC0
	KTE( 0, 0, KTE_INVALID),	//ctrl-S DC0
	KTE( 0, 0, KTE_INVALID),	//ctrl-T DC0
	KTE( 0, 0, KTE_INVALID),	//ctrl-U NAK
	KTE( 0, 0, KTE_INVALID),	//ctrl-V SYN
	KTE( 0, 0, KTE_INVALID),	//ctrl-W ETB
	KTE( 0, 0, KTE_INVALID),	//ctrl-X CAN
	KTE( 0, 0, KTE_INVALID),	//ctrl-Y EM
	KTE( 0, 0, KTE_INVALID),	//ctrl-Z SUB
	KTE( 0, 0, KTE_INVALID),	//ESC
	KTE( 0, 0, KTE_INVALID),	//FS
	KTE( 0, 0, KTE_INVALID),	//GS
	KTE( 0, 0, KTE_INVALID),	//RS
	KTE( 0, 0, KTE_INVALID),	//clear

	KTE( 6, 7, KTE_NO_MOD),		//space
	KTE( 4, 1, KTE_SHIFT_ON),	//!
	KTE( 4, 2, KTE_SHIFT_ON),	//"
	KTE( 4, 3, KTE_SHIFT_ON),	//#
	KTE( 4, 4, KTE_SHIFT_ON),	//$
	KTE( 4, 5, KTE_SHIFT_ON),	//%
	KTE( 4, 6, KTE_SHIFT_ON),	//&
	KTE( 4, 7, KTE_SHIFT_ON),	//'
	KTE( 5, 0, KTE_SHIFT_ON),	//(
	KTE( 5, 1, KTE_SHIFT_ON),	//)
	KTE( 5, 2, KTE_SHIFT_ON),	//*
	KTE( 5, 3, KTE_SHIFT_ON),	//+
	KTE( 5, 4, KTE_SHIFT_OFF),//,
	KTE( 5, 5, KTE_SHIFT_OFF),//-
	KTE( 5, 6, KTE_SHIFT_OFF),//.
	KTE( 5, 7, KTE_SHIFT_OFF),///
	KTE( 4, 0, KTE_SHIFT_OFF),//0
	KTE( 4, 1, KTE_SHIFT_OFF),//1
	KTE( 4, 2, KTE_SHIFT_OFF),//2
	KTE( 4, 3, KTE_SHIFT_OFF),//3
	KTE( 4, 4, KTE_SHIFT_OFF),//4
	KTE( 4, 5, KTE_SHIFT_OFF),//5
	KTE( 4, 6, KTE_SHIFT_OFF),//6
	KTE( 4, 7, KTE_SHIFT_OFF),//7
	KTE( 5, 0, KTE_SHIFT_OFF),//8
	KTE( 5, 1, KTE_SHIFT_OFF),//9
	KTE( 5, 2, KTE_SHIFT_OFF),//:
	KTE( 5, 3, KTE_SHIFT_OFF),//;
	KTE( 5, 4, KTE_SHIFT_ON),	//<
	KTE( 5, 5, KTE_SHIFT_ON),	//=
	KTE( 5, 6, KTE_SHIFT_ON),	//>
	KTE( 5, 7, KTE_SHIFT_ON),	//?

	KTE( 0, 0, KTE_NO_MOD),		//@
	KTE( 0, 1, KTE_NO_MOD),		//A
	KTE( 0, 2, KTE_NO_MOD),		//B
	KTE( 0, 3, KTE_NO_MOD),		//C
	KTE( 0, 4, KTE_NO_MOD),		//D
	KTE( 0, 5, KTE_NO_MOD),		//E
	KTE( 0, 6, KTE_NO_MOD),		//F
	KTE( 0, 7, KTE_NO_MOD),		//G
	KTE( 1, 0, KTE_NO_MOD),		//H
	KTE( 1, 1, KTE_NO_MOD),		//I
	KTE( 1, 2, KTE_NO_MOD),		//J
	KTE( 1, 3, KTE_NO_MOD),		//K
	KTE( 1, 4, KTE_NO_MOD),		//L
	KTE( 1, 5, KTE_NO_MOD),		//M
	KTE( 1, 6, KTE_NO_MOD),		//N
	KTE( 1, 7, KTE_NO_MOD),		//O
	KTE( 2, 0, KTE_NO_MOD),		//P
	KTE( 2, 1, KTE_NO_MOD),		//Q
	KTE( 2, 2, KTE_NO_MOD),		//R
	KTE( 2, 3, KTE_NO_MOD),		//S
	KTE( 2, 4, KTE_NO_MOD),		//T
	KTE( 2, 5, KTE_NO_MOD),		//U
	KTE( 2, 6, KTE_NO_MOD),		//V
	KTE( 2, 7, KTE_NO_MOD),		//W
	KTE( 3, 0, KTE_NO_MOD),		//X
	KTE( 3, 1, KTE_NO_MOD),		//Y
	KTE( 3, 2, KTE_NO_MOD),		//Z
	KTE( 0, 0, KTE_INVALID),	//[
	KTE( 0, 0, KTE_INVALID),	//\ 
	KTE( 0, 0, KTE_INVALID),	//]
	KTE( 0, 0, KTE_INVALID),	//^
	KTE( 0, 0, KTE_INVALID),	//_

	KTE( 0, 0, KTE_INVALID),	//`
	KTE( 0, 1, KTE_NO_MOD),		//a
	KTE( 0, 2, KTE_NO_MOD),		//b
	KTE( 0, 3, KTE_NO_MOD),		//c
	KTE( 0, 4, KTE_NO_MOD),		//d
	KTE( 0, 5, KTE_NO_MOD),		//e
	KTE( 0, 6, KTE_NO_MOD),		//f
	KTE( 0, 7, KTE_NO_MOD),		//g
	KTE( 1, 0, KTE_NO_MOD),		//h
	KTE( 1, 1, KTE_NO_MOD),		//i
	KTE( 1, 2, KTE_NO_MOD),		//j
	KTE( 1, 3, KTE_NO_MOD),		//k
	KTE( 1, 4, KTE_NO_MOD),		//l
	KTE( 1, 5, KTE_NO_MOD),		//m
	KTE( 1, 6, KTE_NO_MOD),		//n
	KTE( 1, 7, KTE_NO_MOD),		//o
	KTE( 2, 0, KTE_NO_MOD),		//p
	KTE( 2, 1, KTE_NO_MOD),		//q
	KTE( 2, 2, KTE_NO_MOD),		//r
	KTE( 2, 3, KTE_NO_MOD),		//s
	KTE( 2, 4, KTE_NO_MOD),		//t
	KTE( 2, 5, KTE_NO_MOD),		//u
	KTE( 2, 6, KTE_NO_MOD),		//v
	KTE( 2, 7, KTE_NO_MOD),		//w
	KTE( 3, 0, KTE_NO_MOD),		//x
	KTE( 3, 1, KTE_NO_MOD),		//y
	KTE( 3, 2, KTE_NO_MOD),		//Z
	KTE( 0, 0, KTE_INVALID),	//{
	KTE( 0, 0, KTE_INVALID),	//|
	KTE( 0, 0, KTE_INVALID),	//)
	KTE( 0, 0, KTE_INVALID),	//~
	KTE( 0, 0, KTE_INVALID) 	//DEL
};

void emuUserInputEventHandler(uint8_t in_device_number, sysUserInputEventCategory in_event_category, sysUserInputEventType in_event_type, uint32_t in_event_param)
{
	bool pressed = (in_event_category == sysUIEC_Pressed);
	uint8_t keyboard_table_entry = KTE(0, 0, KTE_INVALID);
	sysChar ch;
	uint32_t current_modifiers;
	uint8_t row;
	uint8_t bit;
	uint8_t modifier;
	bool shift_state = false;

	// convert to character (if possible)
	ch = cpConvertWin1250(in_device_number, in_event_category, in_event_type, in_event_param);
	current_modifiers = sysUserInputGetModifiersState(in_device_number);

	// handle keyboard events
	if (in_event_type == sysUIET_Key && (in_event_category == sysUIEC_Pressed || in_event_category == sysUIEC_Released))
	{
		if (ch != sysVKC_NULL)
		{
			// if it can be converter to character
			ch = cpToUpperWin1250(ch);

			if ((uint8_t)ch < 128 && (uint8_t)ch >= 0)
			{
				keyboard_table_entry = l_keyboard_table[(uint8_t)ch];
			}
		}
		else
		{
			// special keys
			switch (in_event_param)
			{
				case sysVKC_SPECIAL_KEY_FLAG | sysVKC_UP:
					keyboard_table_entry = KTE(6, 3, KTE_NO_MOD);
					break;

				case sysVKC_SPECIAL_KEY_FLAG | sysVKC_DOWN:
					keyboard_table_entry = KTE(6, 4, KTE_NO_MOD);
					break;

				case sysVKC_SPECIAL_KEY_FLAG | sysVKC_LEFT:
					keyboard_table_entry = KTE(6, 5, KTE_NO_MOD);
					break;

				case sysVKC_SPECIAL_KEY_FLAG | sysVKC_RIGHT:
					keyboard_table_entry = KTE(6, 6, KTE_NO_MOD);
					break;

				case sysVKC_ESCAPE:
					keyboard_table_entry = KTE(6, 2, KTE_NO_MOD);
					break;

				case sysVKC_SPECIAL_KEY_FLAG | sysVKC_END:
					keyboard_table_entry = KTE(6, 1, KTE_NO_MOD);
					break;
			}
		}

		// set actual modifier state
		if ((current_modifiers & sysMS_SHIFT) == 0)
			g_keyboard_ram[7] &= ~1;
		else
			g_keyboard_ram[7] |= 1;

		// process entry
		modifier = GET_MOD(keyboard_table_entry);
		if (modifier != KTE_INVALID)
		{
			row = GET_ROW(keyboard_table_entry);
			bit = GET_BIT(keyboard_table_entry);

			// store in the keyboard data matrix
			if (in_event_category == sysUIEC_Pressed)
			{
				// pressed
				// activate required modifiers
				switch (modifier)
				{
					// no change
					case KTE_NO_MOD:
						break;

						// force shift on
					case KTE_SHIFT_ON:
						g_keyboard_ram[7] |= 1;
						break;

						// force shift off
					case KTE_SHIFT_OFF:
						g_keyboard_ram[7] &= ~1;
						break;
				}

				// store key data
				g_keyboard_ram[row] |= (1 << bit);
			}
			else
			{
				g_keyboard_ram[row] &= ~(1 << bit);
			}
		}
	}
}

#pragma endregion

/****************************************************************************
* T I M I N G    R O U T I N E S
****************************************************************************/
#pragma region - Timing rutines -

///////////////////////////////////////////////////////////////////////////////
/// @brief Gets current CPU timestamp in cycle count
static uint32_t cpuGetTimestamp(void)
{
	return l_total_cpu_cycles + l_cpu.ICount;
}


///////////////////////////////////////////////////////////////////////////////
/// @brief Gets elapsed CPU time since the given timestamp
/// @param in_timestamp CPU cycle timestamp to calculate time from
/// @return Elapsed cycle count
static uint32_t cpuGetEllapsedTimeSince(uint32_t in_timestamp)
{
	uint32_t total_cpu_cycles = l_total_cpu_cycles + l_cpu.ICount;

	return total_cpu_cycles - in_timestamp;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Gets elapsed CPU time since the given timestamp in microsec (ellapsed time should be less than 4sec for the correct operation)
/// @param in_timestamp CPU cycle timestamp to calculate time from
/// @return Elapsed time in microsec
static uint32_t cpuGetEllapsedTimeSinceInMicrosec(uint32_t in_timestamp)
{
	uint32_t ellapsed_cycles = l_total_cpu_cycles + l_cpu.ICount - in_timestamp;

	return ellapsed_cycles * 1000 / (emuHT1080_CPU_CLK / 1000);
}

#pragma endregion

/*****************************************************************************/
/* C A S E T T E   I N T E R F A C E                                         */
/*****************************************************************************/
#pragma region - Cassette routines -

///////////////////////////////////////////////////////////////////////////////
/// @brief Handles cassette motor on operation
static void emuCASMotorOn(void)
{
	switch (l_cas_state)
	{
		case emuCS_Idle:
		case emuCS_LoadStart:
			// open cas file if it is not opened
			if(l_cas_file == sysNULL)
				l_cas_file = fileOpen(g_application_settings.CassetteFileName, "rb");

			if (l_cas_file != sysNULL)
			{
				// read first byte and initialize loading
				fileRead(&l_cas_buffer, sizeof(l_cas_buffer), 1, l_cas_file);
				l_cas_clock_timestamp = cpuGetTimestamp();
				l_in_port_ff &= ~emuPORT_FF_INPUT_MASK; // clock pulse
				l_cas_buffer_bit_count = 8;
				l_cas_state = emuCS_LoadData;
			}
			else
			{
				l_cas_state = emuCS_Idle;
			}
			break;

		case emuCS_SaveStart:
			// create file
			l_cas_file = fileOpen("test.cas", "wb");

			// start data decoding
			l_cas_state = emuCS_SaveWaitForClock;
			l_cas_buffer_bit_count = 0;
			l_cas_buffer = 0;

			break;
	}

	// handle fast cassette operation
	if (g_application_settings.FastCassetteOperation)
	{
		emuWaitIndicatorShow();
	}

	l_cas_motor_on = true;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Handles motor off operaton of casette operation
static void emuCASMotorOff(void)
{
	switch (l_cas_state)
	{
		case emuCS_SaveWaitForData:
			if (l_cas_buffer_bit_count > 0)
			{
				l_cas_buffer <<= 8 - l_cas_buffer_bit_count;
				fileWrite(&l_cas_buffer, sizeof(l_cas_buffer), 1, l_cas_file);
			}

			fileClose(l_cas_file);
			l_cas_file = sysNULL;
			break;
	}

	l_cas_motor_on = false;
	l_cas_state = emuCS_Idle;

	emuWaitIndicatorHide();
	emuRefreshScreen();
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Handles cassette output pulse (save)
/// @param in_pulse Pulse state 0 - zero, 2 - minus, 1,3 - plus pulse level
static void emuCASOut(uint8_t in_pulse)
{
	// do nothing when cas interface is idle
	if (l_cas_state == emuCS_Idle)
		return;

	// handle only positive pulse
	if (in_pulse == 1)
	{
		uint32_t pulse_length = cpuGetEllapsedTimeSinceInMicrosec(l_cas_clock_timestamp);

		if (pulse_length < emuCAS_CLOCK_PERIOD_MAX)
		{
			// valid pulse detected
			if (l_cas_state == emuCS_SaveWaitForData)
			{
				// insert new bit into the save buffer
				l_cas_buffer <<= 1;

				// check for data/clock pulse
				if (pulse_length < emuCAS_CLOCK_PERIOD_MIN)
				{
					// data pulse received
					l_cas_buffer |= 1;
					l_cas_state = emuCS_SaveWaitForClock;
				}
				else
				{
					// there was no data pulse, this is the next clock pulse
					l_cas_state = emuCS_SaveWaitForData;
					l_cas_clock_timestamp = cpuGetTimestamp();
				}

				// increment bit count and handle when all bits of byte is received
				l_cas_buffer_bit_count++;
				if (l_cas_buffer_bit_count >= 8)
				{
					fileWrite(&l_cas_buffer, sizeof(l_cas_buffer), 1, l_cas_file);
					l_cas_buffer_bit_count = 0;
				}
			}
			else
			{
				// clock detected wait for data
				l_cas_state = emuCS_SaveWaitForData;
				l_cas_clock_timestamp = cpuGetTimestamp();
			}
		}
		else
		{
			// first clock pulse received 
			l_cas_clock_timestamp = cpuGetTimestamp();
			l_cas_state = emuCS_SaveWaitForData;
		}
	}
}


///////////////////////////////////////////////////////////////////////////////
/// @brief Handles cassette input pulse (load)
static void emuCASIn(void)
{
	uint32_t ellapsed_time_since_clock;

	if (l_cas_state <= emuCS_LoadStart)
		return;

	// update busy indicator
	if(g_application_settings.FastCassetteOperation)
		fbWaitIndicatorUpdate();

	// time since clock pulse
	ellapsed_time_since_clock = cpuGetEllapsedTimeSinceInMicrosec(l_cas_clock_timestamp);

	switch (l_cas_state)
	{
		case emuCS_LoadClock:
			// handle data
			if (ellapsed_time_since_clock > 1300)
			{
				if ((l_cas_buffer & 0x80) != 0)
					l_in_port_ff |= emuPORT_FF_INPUT_MASK;

				l_cas_buffer <<= 1;
				l_cas_buffer_bit_count--;
				if (l_cas_buffer_bit_count == 0)
				{
					if (l_cas_file != sysNULL)
					{
						if (fileRead(&l_cas_buffer, sizeof(l_cas_buffer), 1, l_cas_file) != 1)
						{
							l_cas_buffer = 0;
							fileClose(l_cas_file);
							l_cas_file = sysNULL;
						}
					}
					l_cas_buffer_bit_count = 8;
				}

				l_cas_state = emuCS_LoadData;
			}
			break;

		case emuCS_LoadData:
			// data loaded -> handle clock
			if (ellapsed_time_since_clock > 2000)
			{
				l_cas_clock_timestamp = cpuGetTimestamp();
				l_in_port_ff |= emuPORT_FF_INPUT_MASK;
				l_cas_state = emuCS_LoadClock;
			}
			break;
	}
}
#pragma endregion
