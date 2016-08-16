/*****************************************************************************/
/* Status LED driver functions                                               */
/*                                                                           */
/* Copyright (C) 2016 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <stm32f4xx_hal.h>
#include <drvHelpers.h>
#include "sysConfig.h"

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/

/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/

/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Status LED driver initialization function
void drvStatLEDInit(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	// Configure status LED pin
	GPIO_InitStruct.Pin = drvLED_LD3_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_LOW;

	HAL_GPIO_Init(drvLED_LD3_PORT, &GPIO_InitStruct);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Set status LED current dimming value
/// @param in_dim Dimming value 0 dark, != 0 lit
void drvStatLEDSetDim(uint8_t in_dim)
{
	if(in_dim == 0)
		drvSetPinHigh(drvLED_LD3_PORT, drvLED_LD3_PIN);
	else
		drvSetPinLow(drvLED_LD3_PORT, drvLED_LD3_PIN);
}
