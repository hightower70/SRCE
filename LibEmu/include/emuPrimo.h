/*****************************************************************************/
/* HT1080Z Emulator                                                          */
/*   main hardware emulation                                                 */
/*                                                                           */
/* Copyright (C) 2015 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/
#ifndef __emuHT1080_h
#define __emuHT1080_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysTypes.h>
#include <sysUserInput.h>
#include "sysConfig.h"

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/

/////////
// Timing
#define emuHT1080_CPU_CLK 1774080 // CPU clock in MHz
#define emuHT1080_HSYNC_FREQ 15840 // Horizontal sync freq
#define emuHT1080_TOTAL_SCANLINE_COUNT 312 // @50Hz VSYNC freq.
//#define emuHT1080_TOTAL_SCANLINE_COUNT 264 // @60Hz VSYNC freq.

//////////////////////
// HT1080 memory sizes
#define emuHT1080_ROM_SIZE 0x3000 
#define emuHT1080_EXTENSION_ROM_SIZE 0x800
#define emuHT1080_VIDEO_RAM_SIZE 0x400
#define emuHT1080_RAM_SIZE (48*1024)

#define emuHT1080_RAM_START 0x4000
#define emuHT1080_VIDEO_RAM_START 0x3c00
#define emuHT1080_KEYBOARD_START 0x3800
#define emuHT1080_EXTENSION_ROM_START emuHT1080_ROM_SIZE

////////////////////
// Video definitions
#define emuHT1080_SCREEN_WIDTH_IN_CHARACTER 64
#define emuHT1080_SCREEN_HEIGHT_IN_CHARACTER 16

#define emuHT1080_CHARACTER_WIDTH 6
#define emuHT1080_CHARACTER_HEIGHT 12

#define emuHT1080_SCREEN_WIDTH_IN_PIXEL (emuHT1080_SCREEN_WIDTH_IN_CHARACTER * emuHT1080_CHARACTER_WIDTH)
#define emuHT1080_SCREEN_HEIGHT_IN_PIXEL (emuHT1080_SCREEN_HEIGHT_IN_CHARACTER * emuHT1080_CHARACTER_HEIGHT)

/*****************************************************************************/
/* Function prototypes                                                       */
/*****************************************************************************/

void emuHT1080Initialize(void);
void emuHT1080Task(void);
void emuHT1080UserInputEventHandler(uint8_t in_device_number, sysUserInputEventCategory in_event_category, sysUserInputEventType in_event_type, uint32_t in_event_param);
void emuHT1080RefreshScreen(void);

void emuHT1080StartScreenRefresh(void);
void emuHT1080EndScreenrefresh(void);
void emuHT1080RenderCharacter(uint16_t in_video_memory_address);

void emuHT1080UpdateStatistics(uint32_t in_measured_inteval_in_ms);
void emuHT1080ResetStatistics(void);

/*****************************************************************************/
/* Variables for other modules                                               */
/*****************************************************************************/

extern uint8_t g_ram[];
extern uint8_t g_video_ram[];
extern uint8_t g_screen_no_refresh_area[];

extern uint16_t g_emulation_speed_cpu_freq;
extern uint16_t g_emulation_speed_vsync_freq;


#endif