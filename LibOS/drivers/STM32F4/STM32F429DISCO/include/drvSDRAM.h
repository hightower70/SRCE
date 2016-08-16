/*****************************************************************************/
/* Driver for SDRAM interface                                                */
/*                                                                           */
/* Copyright (C) 2016 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

#ifndef __drvSDRAM_h
#define __drvSDRAM_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysTypes.h>
#include <stm32f4xx_hal.h>
#include "sysConfig.h"

/*****************************************************************************/
/* Function prototypes                                                       */
/*****************************************************************************/
bool drvSDRAMInitialize(void);

#define drvSDRAMWriteByte(address, value)       (*(__IO uint8_t *) (drvSDRAM_START_ADDRESS + (address)) = (value))
#define drvSDRAMReadByte(address)               (*(__IO uint8_t *) (drvSDRAM_START_ADDRESS + (address)))

#endif
