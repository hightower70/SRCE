/*****************************************************************************/
/* Helper functions for STM32F429 drivers                                    */
/*                                                                           */
/* Copyright (C) 2016 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

#ifndef __drvHelpers_h
#define __drvHelpers_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysTypes.h>

/*****************************************************************************/
/* PIN Settings helper macros                                                */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief  Sets pin(s) low
/// @note   Defined as macro to get maximum speed using register access
/// @param  GPIOx: GPIOx PORT where you want to set pin low
/// @param  GPIO_Pin: Select GPIO pin(s). You can select more pins with | (OR) operator to set them low
/// @retval None
#define drvSetPinLow(GPIOx, GPIO_Pin)              ((GPIOx)->BSRR = ((GPIO_Pin)<<16))

///////////////////////////////////////////////////////////////////////////////
/// @brief  Sets pin(s) high
/// @note   Defined as macro to get maximum speed using register access
/// @param  GPIOx: GPIOx PORT where you want to set pin high
/// @param  GPIO_Pin: Select GPIO pin(s). You can select more pins with | (OR) operator to set them high
/// @retval None
#define drvSetPinHigh(GPIOx, GPIO_Pin)             ((GPIOx)->BSRR = (GPIO_Pin))

///////////////////////////////////////////////////////////////////////////////
/// @brief  Sets pin(s) value
/// @note   Defined as macro to get maximum speed using register access
/// @param  GPIOx: GPIOx PORT where you want to set pin value
/// @param  GPIO_Pin: Select GPIO pin(s). You can select more pins with | (OR) operator to set them value
/// @param  val: If parameter is 0 then pin will be low, otherwise high
/// @retval None
#define drvSetPinValue(GPIOx, GPIO_Pin, val)       ((val) ? drvHAL_SetPinHigh(GPIOx, GPIO_Pin) : drvHAL_SetPinLow(GPIOx, GPIO_Pin))

/*****************************************************************************/
/* Function prototypes                                                       */
/*****************************************************************************/
uint32_t drvTimerGetSourceFrequency(uint32_t in_timer_id);
void drvSystemClockConfig(void);

#endif
