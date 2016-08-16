/*****************************************************************************/
/* HomeLab III Emulator                                                      */
/*   main hardware emulation                                                 */
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
#include <fileStandardFunctions.h>
#include <sysUserInput.h>
#include <sysVirtualKeyboardCodes.h>
#include <cpuZ80.h>
#include <emuHomelab.h>
#include <sysHighResTimer.h>
#include <sysConfig.h>
#include <appSettings.h>
#include <cpCodePages.h>

#include <fbFileBrowser.h>

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/
#define emuHomelab_MAX_CYCLES_PER_SCANLINE ((emuHomelab_CPU_CLK + emuHomelab_HSYNC_FREQ - 1)/ emuHomelab_HSYNC_FREQ) // rounded up
#define emuHomelab_SCANLINE_IN_US (1000000 / emuHomelab_HSYNC_FREQ)

#define emuHomelab_KEYBOARD_ROW_COUNT 32	// 16 row but it is used as lower/upper nibble
#define emuHomelab_VSYNC_INDEX 2					// vsync byte index wihthin keyboard RAM
#define emuHomelab_VSYNC_BIT_INDEX 0			// vsync bit index within keyboiard ram byte

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

/*****************************************************************************/
/* Module variables                                                          */
/*****************************************************************************/

// CPU
cpuZ80State l_cpu;

// RAM
uint8_t g_keyboard_ram[emuHomelab_KEYBOARD_ROW_COUNT];
uint8_t g_video_ram[emuHomelab_VIDEO_RAM_SIZE];
uint8_t g_ram[emuHomelab_RAM_SIZE];
uint8_t g_memory_page_index;

// external ROM file reference
extern uint8_t g_rom_bin[];

// timing variables
static uint32_t l_total_cpu_cycles;
static int32_t l_current_cycles_per_frame;
static int l_current_scanline;

static sysHighresTimestamp l_current_timestamp;

// port mirror variables
static uint8_t l_out_port_ff = 0;
static uint8_t l_in_port_ff = 0;

// cassette save variables
static emuCASState l_cas_state = emuCS_LoadStart;
static uint8_t l_cas_buffer;
static uint8_t l_cas_buffer_bit_count = 0;
static fileStream* l_cas_file = sysNULL;
static uint32_t l_cas_clock_timestamp;

static bool l_fast_casette_operation = true;
static bool l_full_speed_emulation = false;
static bool l_screen_refresh_disabled = false;

static bool l_16k_mode = false;

cpKeyboardState g_keyboard_state;

///////////////////////////////////////////////////////////////////////////////
/// @brief Initializes Homelab III Computer
void emuHomelabInitialize(void)
{
	uint32_t i;

  // Reset CPU
	cpuReset(&l_cpu);
  
  // Clears RAM
  for(i = 0; i < emuHomelab_RAM_SIZE; i++)
		g_ram[i] = 0;

	for (i = 0; i < emuHomelab_VIDEO_RAM_SIZE; i++)
		g_video_ram[i] = 0;

	for (i = 0; i < emuHomelab_KEYBOARD_ROW_COUNT; i++)
		g_keyboard_ram[i] = 0xff;

	// init emulation variables
	l_total_cpu_cycles = 0;
	l_current_cycles_per_frame = 0;
	l_current_scanline = 0;
	l_current_timestamp = sysHighresTimerGetTimestamp();
	g_memory_page_index = 0;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Main emulation task
void emuHomelabTask(void)
{
	uint16_t cycles_to_execute;
	uint16_t cycles_executed;
	int32_t expected_cycle_per_frame;

	if (sysHighresTimerGetTimeSince(l_current_timestamp) >= emuHomelab_SCANLINE_IN_US || l_full_speed_emulation)
	{
		l_current_timestamp = sysHighresTimerGetTimestamp();

		// calculate cycles to execute
		expected_cycle_per_frame = (emuHomelab_CPU_CLK * (l_current_scanline + 1)) / emuHomelab_HSYNC_FREQ;
		cycles_to_execute = (uint16_t)(expected_cycle_per_frame - l_current_cycles_per_frame);

		// run CPU
		cycles_executed = cpuExecute(&l_cpu, cycles_to_execute);
		l_current_cycles_per_frame += cycles_executed;
		l_total_cpu_cycles += cycles_executed;

		// render scanline
//		if (l_current_scanline < emuHomelab_SCREEN_HEIGHT_IN_PIXEL && !l_screen_refresh_disabled)
//			emuHomelabRenderScanLine(l_current_scanline);

		// handle VSYNC
		if (l_current_scanline >= emuHomelab_SCREEN_HEIGHT_IN_PIXEL)
		{
			g_keyboard_ram[2] |= BV(emuHomelab_VSYNC_BIT_INDEX); // VSYNC bit = 1
			emuHomelabEndScreenrefresh();
		}

		// next scanline
		l_current_scanline++;
		if (l_current_scanline >= emuHomelab_TOTAL_SCANLINE_COUNT)
		{
			// VSYNC -> restart screen rendering
			l_current_scanline = 0;
			l_current_cycles_per_frame = 0;
			g_keyboard_ram[2] &= ~BV(emuHomelab_VSYNC_BIT_INDEX); // VSYNC bit = 0
			emuHomelabStartScreenRefresh();
		}
	}
}

// <editor-fold desc="- Memory handling -">
#pragma region - Memory handling -
/****************************************************************************
* M E M O R Y   I N T E R F A C E
*
*                  Page 0                                            Page 1
* 0000 +----------------------------------------+  +----------------------------------------+ 0000
*      | ROM (Basic interpreter) (16k)          |  | ROM (Basic interpreter) (16k)          |
* 4000 +----------------------------------------+  +----------------------------------------+ 4000
*      | RAM A (16k)                            |  | RAM A (16k)                            |
* 8000 +----------------------------------------+  +----------------------------------------+ 8000
*      | RAM B (16k)                            |  | RAM D (16k)                            |
* C000 +----------------------------------------+  +----------------------------------------+ C000
*      |                                        |  | Ununsed (4k)                           |
* D000 +                                        +  +----------------------------------------+ D000
*      |                                        |  | ROM Expansion (4k)                     |
* E000 + RAM C (16k)                            +  +----------------------------------------+ E000
*      |                                        |  | Keyboard area (4k)                     |
* F000 +                                        +  +----------------------------------------+ F000
*      |                                        |  | Video RAM (twice, 2x2k)                |
* FFFF +----------------------------------------+  +----------------------------------------+ FFFF
*
****************************************************************************/

//--------------------------------------------------------------
// Memory write
//--------------------------------------------------------------
void cpuMemWrite(register uint16_t in_address, register uint8_t in_value)
{
	uint16_t address;

	if (in_address >= 12288)
	{
		g_ram[in_address - 12288] = in_value;

		if(in_address > 3)
		{
			if( g_ram[in_address-1] == '.' && g_ram[in_address-2] == '1')
			{
				in_address = 5;
			}
		}
	}
}

//--------------------------------------------------------------
// Memory read
//--------------------------------------------------------------

uint8_t cpuMemRead(register uint16_t in_address)
{
	if (in_address < emuHomelab_MEMORY_MIDDLE)
	{
		return g_rom_bin[in_address];
	}
	else
	{
		return g_ram[in_address - 12288];
	}
}

#pragma endregion
// </editor-fold>

#pragma region - Port handing -
/******************************************************************************
* P O R T S
******************************************************************************/

//--------------------------------------------------------------
// Port read
//--------------------------------------------------------------
uint8_t cpuIn(register uint16_t in_port)
{
  uint8_t retval = 0xff;

  switch(in_port & 0xff)
  {
    case 0xff:
      break;
  }

  return retval;
}

//--------------------------------------------------------------
// Port write
//--------------------------------------------------------------
void cpuOut(register uint16_t in_port, register uint8_t in_value)
{
	g_memory_page_index = (uint8_t)((in_port >> 7) & 0x01);

	switch (in_port & 0xff)
	{
		case 0xff:
		break;
	}
}
#pragma endregion

#pragma region - Keyboard handling -
/****************************************************************************
* K E Y B O A R D  I N T E R F A C E
*
* Keyboard matrix:
*         D0    D1    D2    D3
* E800  Down    Up Right  Left
* E801    SP    CR
* E802  SYNC   SH1   SH2   Alt
* E803  TAPE    F2    F1
* E804     0     1     2     3
* E805     4     5     6     7
* E806     8     9     :     ;
* E807     ,     =     .     ?
* E808     ^     A     Á     B
* E809     C     D     E     É
* E80A     F     G     H     I
* E80B     J     K     L     M
* E80C     N     O     Ó     Ö
* E80D     P     Q     R     S
* E80E     T     U     Ü     V
* E80F     W     X     Y     Z
*
****************************************************************************/

#define KTE_INVALID		0
#define KTE_NO_MOD		1
#define KTE_SHIFT_ON	2
#define KTE_SHIFT_OFF 3

#define KTE(row,bit,modifier) (uint8_t)(((row<<4)|(bit<<2))|modifier)

#define ROW(x) ((x)<<4)
#define BIT(x) ((x)<<2)

#define GET_ROW(x) (((x)>>4)&0x0f)
#define GET_BIT(x) (((x)>>2)&0x03)
#define GET_MOD(x) ((x)&0x03)


const uint8_t l_keyboard_table[256] =
{
	// 0x00
	KTE(0x00, 0, KTE_INVALID),	// not used
	KTE(0x00, 1, KTE_NO_MOD),		// special key (1): up
	KTE(0x00, 0, KTE_NO_MOD),		// special key (2): down
	KTE(0x00, 3, KTE_NO_MOD),		// special key (3): left
	KTE(0x00, 2, KTE_NO_MOD),		// special key (4): right
	KTE(0x03, 2, KTE_NO_MOD),		// special key (5): F1
	KTE(0x03, 1, KTE_NO_MOD),		// special key (6): F2
	KTE(0x02, 3, KTE_NO_MOD),		// special key (7): ALT

	// 0x08
	KTE(0x00, 3, KTE_SHIFT_ON),	// ctrl-H BS
	KTE(0x00, 1, KTE_SHIFT_ON),	// ctrl-I TAB
	KTE(0x00, 0, KTE_INVALID),	// ctrl-J LF
	KTE(0x00, 0, KTE_INVALID),	// ctrl-K VT
	KTE(0x00, 0, KTE_INVALID),	// ctrl-L FF
	KTE(0x01, 1, KTE_NO_MOD),		// ctrl-M CR
	KTE(0x00, 0, KTE_INVALID),	// ctrl-N SO
	KTE(0x00, 0, KTE_INVALID),	// ctrl-O SI

	// 0x10
	KTE(0x00, 0, KTE_INVALID),	//ctrl-P DLE
	KTE(0x00, 0, KTE_INVALID),	//ctrl-Q DC1
	KTE(0x00, 0, KTE_INVALID),	//ctrl-R DC2
	KTE(0x00, 0, KTE_INVALID),	//ctrl-S DC3
	KTE(0x00, 0, KTE_INVALID),	//ctrl-T DC4
	KTE(0x00, 0, KTE_INVALID),	//ctrl-U NAK
	KTE(0x00, 0, KTE_INVALID),	//ctrl-V SYN
	KTE(0x00, 0, KTE_INVALID),	//ctrl-W ETB

	// 0x18
	KTE(0x00, 0, KTE_INVALID),	//ctrl-X CAN
	KTE(0x00, 0, KTE_INVALID),	//ctrl-Y EM
	KTE(0x00, 0, KTE_INVALID),	//ctrl-Z SUB
	KTE(0x08, 1, KTE_INVALID),	//ESC
	KTE(0x00, 0, KTE_INVALID),	//(invalid) FS
	KTE(0x00, 0, KTE_INVALID),	//(invalid) GS
	KTE(0x00, 0, KTE_INVALID),	//(invalid) RS
	KTE(0x00, 0, KTE_INVALID),	//clear

	// 0x20
	KTE(0x01, 0, KTE_NO_MOD),		// space
	KTE(0x04, 1, KTE_SHIFT_ON),	// !
	KTE(0x04, 2, KTE_SHIFT_ON),	// "
	KTE(0x04, 3, KTE_SHIFT_ON),	// #
	KTE(0x05, 0, KTE_SHIFT_ON),	// $
	KTE(0x05, 1, KTE_SHIFT_ON),	// %
	KTE(0x05, 2, KTE_SHIFT_ON),	// &
	KTE(0x05, 3, KTE_SHIFT_ON),	// '

	// 0x28
	KTE(0x06, 0, KTE_SHIFT_ON),	// (
	KTE(0x06, 1, KTE_SHIFT_ON),	// )
	KTE(0x06, 2, KTE_SHIFT_ON),	// *
	KTE(0x06, 3, KTE_SHIFT_ON),	// +
	KTE(0x07, 0, KTE_SHIFT_OFF),// ,
	KTE(0x07, 1, KTE_SHIFT_ON),	// -
	KTE(0x07, 2, KTE_SHIFT_OFF),// .
	KTE(0x07, 3, KTE_SHIFT_ON),	// /

	// 0x30
	KTE(0x04, 0, KTE_NO_MOD),		// 0
	KTE(0x04, 1, KTE_NO_MOD),		// 1
	KTE(0x04, 2, KTE_NO_MOD),		// 2
	KTE(0x04, 3, KTE_NO_MOD),		// 3
	KTE(0x05, 0, KTE_NO_MOD),		// 4
	KTE(0x05, 1, KTE_NO_MOD),		// 5
	KTE(0x05, 2, KTE_NO_MOD),		// 6
	KTE(0x05, 3, KTE_NO_MOD),		// 7

	// 0x38
	KTE(0x06, 0, KTE_NO_MOD),		// 8
	KTE(0x06, 1, KTE_NO_MOD),		// 9
	KTE(0x06, 2, KTE_SHIFT_OFF),// :
	KTE(0x06, 3, KTE_SHIFT_OFF),// ;
	KTE(0x07, 0, KTE_SHIFT_ON),	// <
	KTE(0x07, 1, KTE_SHIFT_OFF),// =
	KTE(0x07, 2, KTE_SHIFT_ON),	// >
	KTE(0x07, 3, KTE_SHIFT_OFF),// ?

	// 0x40
	KTE(0x00, 0, KTE_INVALID),	//@
	KTE(0x08, 1, KTE_NO_MOD),		//A
	KTE(0x08, 3, KTE_NO_MOD),		//B
	KTE(0x09, 0, KTE_NO_MOD),		//C
	KTE(0x09, 1, KTE_NO_MOD),		//D
	KTE(0x09, 2, KTE_NO_MOD),		//E
	KTE(0x0A, 0, KTE_NO_MOD),		//F
	KTE(0x0A, 1, KTE_NO_MOD),		//G

	// 0x48
	KTE(0x0A, 2, KTE_NO_MOD),		// H
	KTE(0x0A, 3, KTE_NO_MOD),		// I
	KTE(0x0B, 0, KTE_NO_MOD),		// J
	KTE(0x0B, 1, KTE_NO_MOD),		// K
	KTE(0x0B, 2, KTE_NO_MOD),		// L
	KTE(0x0B, 3, KTE_NO_MOD),		// M
	KTE(0x0C, 0, KTE_NO_MOD),		// N
	KTE(0x0C, 1, KTE_NO_MOD),		// O

	// 0x50
	KTE(0x0D, 0, KTE_NO_MOD),		// P
	KTE(0x0D, 1, KTE_NO_MOD),		// Q
	KTE(0x0D, 2, KTE_NO_MOD),		// R
	KTE(0x0D, 3, KTE_NO_MOD),		// S
	KTE(0x0E, 0, KTE_NO_MOD),		// T
	KTE(0x0E, 1, KTE_NO_MOD),		// U
	KTE(0x0E, 3, KTE_NO_MOD),		// V
	KTE(0x0F, 0, KTE_NO_MOD),		// W

	// 0x58
	KTE(0x0F, 1, KTE_NO_MOD),		// X
	KTE(0x0F, 2, KTE_NO_MOD),		// Y
	KTE(0x0F, 3, KTE_NO_MOD),		// Z
	KTE(0x00, 0, KTE_INVALID),	// [
	KTE(0x00, 0, KTE_INVALID),	// \ 
	KTE(0x00, 0, KTE_INVALID),	// ]
	KTE(0x08, 0, KTE_NO_MOD),		// ^
	KTE(0x00, 0, KTE_INVALID),	// _

	// 0x60
	KTE(0x00, 0, KTE_INVALID),	// `
	KTE(0x00, 0, KTE_INVALID),	// a
	KTE(0x00, 0, KTE_INVALID),	// b
	KTE(0x00, 0, KTE_INVALID),	// c
	KTE(0x00, 0, KTE_INVALID),	// d
	KTE(0x00, 0, KTE_INVALID),	// e
	KTE(0x00, 0, KTE_INVALID),	// f
	KTE(0x00, 0, KTE_INVALID),	// g

	// 0x68
	KTE(0x00, 0, KTE_INVALID),	// h
	KTE(0x00, 0, KTE_INVALID),	// i
	KTE(0x00, 0, KTE_INVALID),	// j
	KTE(0x00, 0, KTE_INVALID),	// k
	KTE(0x00, 0, KTE_INVALID),	// l
	KTE(0x00, 0, KTE_INVALID),	// m
	KTE(0x00, 0, KTE_INVALID),	// n
	KTE(0x00, 0, KTE_INVALID),	// o

	// 0x70	
	KTE(0x00, 0, KTE_INVALID),	// p
	KTE(0x00, 0, KTE_INVALID),	// q
	KTE(0x00, 0, KTE_INVALID),	// r
	KTE(0x00, 0, KTE_INVALID),	// s
	KTE(0x00, 0, KTE_INVALID),	// t
	KTE(0x00, 0, KTE_INVALID),	// u
	KTE(0x00, 0, KTE_INVALID),	// v
	KTE(0x00, 0, KTE_INVALID),	// w

	// 0x78
	KTE(0x00, 0, KTE_INVALID),	// x
	KTE(0x00, 0, KTE_INVALID),	// y
	KTE(0x00, 0, KTE_INVALID),	// Z
	KTE(0x00, 0, KTE_INVALID),	// {
	KTE(0x00, 0, KTE_INVALID),	// |
	KTE(0x00, 0, KTE_INVALID),	// }
	KTE(0x00, 0, KTE_INVALID),	// ~
	KTE(0x00, 0, KTE_INVALID),	// DEL

	// 0x80
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),

	// 0x88
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),

	// 0x90
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),

	// 0x98
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),

	// 0xA0
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),

	// 0xA8
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),

	// 0xB0
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),

	// 0xB8
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),

	// 0xC0
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x08, 2, KTE_NO_MOD),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),

	// 0xC8
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x09, 3, KTE_NO_MOD),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),

	// 0xD0
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x0C, 2, KTE_NO_MOD),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x0C, 3, KTE_NO_MOD),
	KTE(0x00, 0, KTE_INVALID),

	// 0xD8
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x0E, 2, KTE_NO_MOD),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),

	// 0xE0
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),

	// 0xE8
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),

	// 0xF0
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),

	// 0xF8
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
	KTE(0x00, 0, KTE_INVALID),
};

void emuUserInputEventHandler(uint8_t in_device_number, sysUserInputEventCategory in_event_category, sysUserInputEventType in_event_type, uint32_t in_event_param)
{
	bool pressed = (in_event_category == sysUIEC_Pressed); 
	int keyboard_table_index = -1;
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
			keyboard_table_index = (uint8_t)cpToUpperWin1250(ch);
		}
		else
		{
			// special keys
			switch (in_event_param)
			{
				case sysVKC_SPECIAL_KEY_FLAG |sysVKC_UP:
					keyboard_table_index = 1;
					break;

				case sysVKC_SPECIAL_KEY_FLAG | sysVKC_DOWN:
					keyboard_table_index = 2;
					break;

				case sysVKC_SPECIAL_KEY_FLAG | sysVKC_LEFT:
					keyboard_table_index = 3;
					break;

				case sysVKC_SPECIAL_KEY_FLAG | sysVKC_RIGHT:
					keyboard_table_index = 4;
					break;

				case sysVKC_F5:
					keyboard_table_index = 5;
					break;

				case sysVKC_F6:
					keyboard_table_index = 6;
					break;

				case sysVKC_CAPITAL:
					keyboard_table_index = 7;
					break;
			}
		}

		// set actual modifier state
		if ((current_modifiers & sysMS_SHIFT) == 0)
			g_keyboard_ram[2] |= 2;
		else
			g_keyboard_ram[2] &= ~2;

		// process entry
		if (keyboard_table_index != -1)
		{
			row = GET_ROW(l_keyboard_table[keyboard_table_index]);
			bit = GET_BIT(l_keyboard_table[keyboard_table_index]);
			modifier = GET_MOD(l_keyboard_table[keyboard_table_index]);

			if (modifier != KTE_INVALID)
			{
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
							g_keyboard_ram[2] &= ~2;
							break;

						// force shift off
						case KTE_SHIFT_OFF:
							g_keyboard_ram[2] |= 2;
							break;
					}

					// store key data
					g_keyboard_ram[row] &= ~(1 << bit);
				}
				else
				{
					g_keyboard_ram[row] |= (1 << bit);
				}
			}
		}
	}
}

#pragma endregion

#pragma region - Timing rutines -
/****************************************************************************
* T I M I N G    R O U T I N E S   
****************************************************************************/

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

	return ellapsed_cycles * 1000 / (emuHomelab_CPU_CLK / 1000);
}

#pragma endregion

#pragma region - Cassette routines -
/****************************************************************************
* C A S E T T E   I N T E R F A C E
****************************************************************************/
#if 0

///////////////////////////////////////////////////////////////////////////////
/// @brief Handles cassette motor on operation
static void emuCASMotorOn(void)
{
	// do nothing when cas interface is idle
	if (l_cas_state == emuCS_Idle)
		return;

	// start saving
	if (l_cas_state == emuCS_SaveStart)
	{
		// create file
		l_cas_file = fileOpen("test.cas", "wb");

		// start data decoding
		l_cas_state = emuCS_SaveWaitForClock;
		l_cas_buffer_bit_count = 0;
		l_cas_buffer = 0;
	}

	// start loading
	if (l_cas_state == emuCS_LoadStart)
	{
		// load cas file
		l_cas_file = fileOpen("galaxy.cas", "rb");

		fileRead(&l_cas_buffer, sizeof(l_cas_buffer), 1, l_cas_file);
		l_cas_buffer_bit_count = 8;
		l_cas_state = emuCS_LoadClock;
		l_in_port_ff |= emuPORT_FF_INPUT_MASK; // clock pulse
		l_cas_clock_timestamp = cpuGetTimestamp();

		// handle fast cassette operation
		if (g_application_settings.FastCassetteOperation)
		{
			l_full_speed_emulation = true;
			l_screen_refresh_disabled = true;
			
			//fbWaitIndicatorShow();
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Handles motor off operaton of casette operation
static void emuCASMotorOff(void)
{
	// do nothing when cas interface is idle
	if (l_cas_state == emuCS_Idle)
		return;

	l_full_speed_emulation = g_application_settings.FullSpeed;
	l_screen_refresh_disabled = false;

	fbWaitIndicatorHide();

	// motor was turned off
	if (l_cas_file != sysNULL)
	{
		if (l_cas_buffer_bit_count > 0 && l_cas_state == emuCS_SaveWaitForData)
		{
			l_cas_buffer <<= 8 - l_cas_buffer_bit_count;
			fileWrite(&l_cas_buffer, sizeof(l_cas_buffer), 1, l_cas_file);
		}

		fileClose(l_cas_file);
	}

	l_cas_file = sysNULL;
	l_cas_state = emuCS_Idle;
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


static void emuCASIn(void)
{
	uint32_t ellapsed_time_since_clock;

	if (l_cas_state <= emuCS_LoadStart)
		return;

	if(l_screen_refresh_disabled)
		fbWaitIndicatorUpdate();

	ellapsed_time_since_clock = cpuGetEllapsedTimeSinceInMicrosec(l_cas_clock_timestamp);

	switch (l_cas_state)
	{
		case emuCS_LoadClock:
			// handle data
			if (ellapsed_time_since_clock > 800)
			{
				if ((l_cas_buffer & 0x80) != 0)
					l_in_port_ff |= emuPORT_FF_INPUT_MASK;

				l_cas_buffer <<= 1;
				l_cas_buffer_bit_count--;
				if (l_cas_buffer_bit_count == 0)
				{
					if (fileRead(&l_cas_buffer, sizeof(l_cas_buffer), 1, l_cas_file) != 1)
					{
						l_cas_buffer = 0;
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

#endif

#pragma endregion

