/*****************************************************************************/
/* Standard digital joystick driver for STM32F429DISCO                       */
/*                                                                           */
/* Copyright (C) 2015 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

#ifndef __drvJoystick_h
#define __drvJoystick_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysTypes.h>
#include <sysUserInput.h>

/*****************************************************************************/
/* Types                                                                     */
/*****************************************************************************/
typedef struct
{
  uint16_t KeyCode;   // keycode to issue when button is pressed
  GPIO_TypeDef* Port; // Port where the button is connected
  const uint16_t Pin; // Pin where the button is connected
  const int Pressed;	// Port bit value when button is pressed (1 - logic high, 0 - logic low for button press)
} drvJoystickButtonInfo;

/*****************************************************************************/
/* External variables                                                        */
/*****************************************************************************/
extern drvJoystickButtonInfo g_joystick_button_info[];

/*****************************************************************************/
/* Function prototypes                                                       */
/*****************************************************************************/
void drvJoystickInitialize(void);
void drvJoystickUpdate(void);


#endif
