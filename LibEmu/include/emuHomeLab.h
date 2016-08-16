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
#ifndef __emuHomelab_h
#define __emuHomelab_h

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
#define emuHomelab_CPU_CLK 3000000 // CPU clock in MHz
#define emuHomelab_HSYNC_FREQ 15625 // Horizontal sync freq
#define emuHomelab_TOTAL_SCANLINE_COUNT 320 // total number of scanlines

//////////////////////
// HT1080 memory sizes
#define emuHomelab_RAM_SIZE (64*1024)
#define emuHomelab_ROM_SIZE (16*1024)
#define emuHomelab_EXTENSION_ROM_SIZE (4*1024)
#define emuHomelab_VIDEO_RAM_SIZE (2*1024)
#define emuHomelab_RAM_D_SIZE (16*1024)
#define emuHomelab_RAM_D_OFFSET 0x4000
#define emuHomelab_VIDEO_RAM_START 0xF000
#define emuHomelab_KEYBOARD_START 0xE000
#define emuHomelab_KEYBOARD_SIZE 32
#define emuHomelab_EXTENSION_ROM_START 0xD0000
#define emuHomelab_MEMORY_MIDDLE 0x8000

////////////////////
// Video definitions
#define emuHomelab_SCREEN_WIDTH_IN_CHARACTER 64
#define emuHomelab_SCREEN_HEIGHT_IN_CHARACTER 32

#define emuHomelab_CHARACTER_WIDTH 8
#define emuHomelab_CHARACTER_HEIGHT 8

#define emuHomelab_SCREEN_WIDTH_IN_PIXEL (emuHomelab_SCREEN_WIDTH_IN_CHARACTER * emuHomelab_CHARACTER_WIDTH)
#define emuHomelab_SCREEN_HEIGHT_IN_PIXEL (emuHomelab_SCREEN_HEIGHT_IN_CHARACTER * emuHomelab_CHARACTER_HEIGHT)

#if emuHomelab_SCREEN_WIDTH_IN_PIXEL != guiSCREEN_WIDTH
#error "Invalidscreen width"
#endif

#if emuHomelab_SCREEN_HEIGHT_IN_PIXEL != guiSCREEN_HEIGHT
#error "Invalidscreen height"
#endif

/*****************************************************************************/
/* Function prototypes                                                       */
/*****************************************************************************/

void emuHomelabInitialize(void);
void emuHomelabTask(void);
//void emuHomelabRenderScanLine(uint16_t in_line_index);
void emuHomelabStartScreenRefresh(void);
void emuHomelabRenderCharacter(uint16_t in_video_memory_address);
void emuHomelabEndScreenrefresh(void);
void emuUserInputEventHandler(uint8_t in_device_number, sysUserInputEventCategory in_event_category, sysUserInputEventType in_event_type, uint32_t in_event_param);


/*****************************************************************************/
/* Variables for other modules                                               */
/*****************************************************************************/

extern uint8_t g_ram[];
extern uint8_t g_video_ram[];

#endif