/*****************************************************************************/
/* System initialization function for STM32F429-DISCO                       */
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
#include <sysMain.h>
#include <drvHelpers.h>
#include <sysTimer.h>
#include <drvStatLED.h>
#include <drvSDRAM.h>
#include <waveMixer.h>
#include <sysHighresTimer.h>
#include <guiColorGraphics.h>
#include <emuInvaders.h>
#include <drvJoystick.h>
#include <stm32f4xx_hal.h>
#include <usb_host.h>

/*****************************************************************************/
/* Module local functions                                                    */
/*****************************************************************************/

/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief System initialization function
void sysInitialize(void)
{
  HAL_Init();

  // system clock setup
  drvSystemClockConfig();

  // GPIO Ports Clock Enable
  __GPIOA_CLK_ENABLE();
  __GPIOB_CLK_ENABLE();
  __GPIOC_CLK_ENABLE();
  __GPIOD_CLK_ENABLE();
  __GPIOE_CLK_ENABLE();
  __GPIOF_CLK_ENABLE();
  __GPIOG_CLK_ENABLE();
  __GPIOH_CLK_ENABLE();

  __DMA1_CLK_ENABLE();
  __DMA2_CLK_ENABLE();

  sysTimerInitialize();
  sysHighresTimerInitialize();
  drvStatLEDInit();
  drvSDRAMInitialize();
  guiColorGraphicsInitialize();
  drvJoystickInitialize();
  MX_USB_HOST_Init();


  emuInvadersInitialize();
}

void sysUserInputEventHandler(uint8_t in_device_number, sysUserInputEventCategory in_event_category, sysUserInputEventType in_event_type, uint32_t in_event_param)
{
	emuUserInputEventHandler(in_device_number, in_event_category, in_event_type, in_event_param);
}

