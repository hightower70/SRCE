/*****************************************************************************/
/* Joystick button mapping                                                   */
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
#include <stm32f4xx_hal.h>
#include <drvJoystick.h>
#include <sysVirtualKeyboardCodes.h>

/*****************************************************************************/
/* Global variables                                                          */
/*****************************************************************************/

// Button mapping
drvJoystickButtonInfo g_joystick_button_info[] =
{
//  KeyCode                                Port         Pin  Pressed
  {sysVKC_3,                              GPIOA, GPIO_PIN_0,       1},
  {sysVKC_1,                              GPIOA, GPIO_PIN_9,       0},
  {sysVKC_2,                              GPIOA, GPIO_PIN_10,      0},
  {sysVKC_LEFT | sysVKC_SPECIAL_KEY_FLAG, GPIOE, GPIO_PIN_4,       0},
  {sysVKC_RIGHT| sysVKC_SPECIAL_KEY_FLAG, GPIOE, GPIO_PIN_5,       0},
  {sysVKC_UP | sysVKC_SPECIAL_KEY_FLAG,   GPIOE, GPIO_PIN_2,       0},
  {sysVKC_DOWN | sysVKC_SPECIAL_KEY_FLAG, GPIOE, GPIO_PIN_3,       0},
  {sysVKC_SPACE,                          GPIOE, GPIO_PIN_6,       0},

  {sysVKC_NULL, 0, 0, 0 }
};
