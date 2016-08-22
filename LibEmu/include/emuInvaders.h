/*****************************************************************************/
/* Taito Space Invaders Hardware Emulator                                    */
/*                                                                           */
/* Copyright (C) 2015 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

#ifndef __emuInvaders_h
#define __emuInvaders_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysTypes.h>
#include <sysUserInput.h>
#include "sysConfig.h"

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/

// Memory constants
#define emuINVADERS_RAM_SIZE (8*1024)
#define emuINVADERS_ROM_SIZE (8*1024)
#define emuINVADERS_RAM_START emuINVADERS_ROM_SIZE
#define emuINVADERS_VIDEO_RAM_START 0x2400
#define emuINVADERS_RAM_MIRROR 0x4000

// screen resolution
#define emuINVADERS_SCREEN_WIDTH  224
#define emuINVADERS_SCREEN_HEIGHT 256

#define emuINVADERS_SCREEN_LEFT ((guiSCREEN_WIDTH - emuINVADERS_SCREEN_WIDTH) / 2)		// Display start position (x)
#define emuINVADERS_SCREEN_TOP  ((guiSCREEN_HEIGHT - emuINVADERS_SCREEN_HEIGHT) / 2)	// Display start position (y)

// Timing constants
#define emuINVADERS_CPU_CLOCK 2000000				// CPU clock in MHz
#define emuINVADERS_FRAME_RATE 60						// frame rate 60Hz

/*****************************************************************************/
/* Global variables                                                          */
/*****************************************************************************/
extern uint8_t g_cpu_ram[];
extern const unsigned char g_cpu_rom[];

/*****************************************************************************/
/* Function prototypes                                                       */
/*****************************************************************************/
void emuInvadersInitialize(void);
bool emuInvadersTask(void);
void emuInvadersRenderScanLine(uint16_t in_line_index);

void emuInvadersRendererInitialize(void);
void emuInvadersRenderPixels(uint16_t in_memory_address, uint8_t in_data);
void emuDisplayStatistics(uint32_t in_cpu_clock, uint16_t in_frame_rate, uint16_t in_load);

void emuUserInputEventHandler(uint8_t in_device_number, sysUserInputEventCategory in_event_category, sysUserInputEventType in_event_type, uint32_t in_event_param);


#endif
