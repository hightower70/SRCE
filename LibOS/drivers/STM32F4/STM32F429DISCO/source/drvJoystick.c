/*****************************************************************************/
/* Standard digital joystick driver for STM32F429DISCO                       */
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
#include <sysTimer.h>
#include <sysUserInput.h>
//#include <drvHelpers.h>
#include <drvJoystick.h>
#include <sysVirtualKeyboardCodes.h>


/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/
#define drvJOYSTICK_BUTTON_SCAN_PERIOD 16
#define drvJOYSTICK_DEVICE_NUMBER 0x80

/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/
static uint32_t l_button_state;
static uint32_t l_prev_button_state;

static sysTimeStamp l_button_scan_timestamp;

/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Initializes joystick driver
void drvJoystickInitialize(void)
{
	uint8_t i;
	GPIO_InitTypeDef   GPIO_InitStructure;

	// init port pins
	i = 0;
	while(g_joystick_button_info[i].KeyCode != sysVKC_NULL)
	{
  	// Configure SPI GPIO pins
    GPIO_InitStructure.Pin				= g_joystick_button_info[i].Pin;
    GPIO_InitStructure.Mode  			= GPIO_MODE_INPUT;
    GPIO_InitStructure.Pull  			= GPIO_NOPULL;
    GPIO_InitStructure.Speed  		= GPIO_SPEED_MEDIUM;
    HAL_GPIO_Init(g_joystick_button_info[i].Port, &GPIO_InitStructure);

		i++;
	}

	// initializing variables
	l_button_state = 0;
	l_prev_button_state = 0;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Updates joystick state and calls input event handler
void drvJoystickAndButtonsUpdate(void)
{
	int one_button_state;
	uint32_t current_button_state;
	uint8_t i;
	uint32_t change_mask;

	// scan buttons periodically
	if(sysTimerGetTimeSince(l_button_scan_timestamp) > drvJOYSTICK_BUTTON_SCAN_PERIOD)
	{
		l_button_scan_timestamp = sysTimerGetTimestamp();

		// get current button state
		i = 0;
		current_button_state = 0;
		while(g_joystick_button_info[i].KeyCode != sysVKC_NULL)
		{
			one_button_state = (HAL_GPIO_ReadPin(g_joystick_button_info[i].Port, g_joystick_button_info[i].Pin) == GPIO_PIN_RESET) ? 0 : 1;
			current_button_state |= ((one_button_state == g_joystick_button_info[i].Pressed)? 1 : 0) << i;
			i++;
		}

		// button debouncing
		if(current_button_state != l_button_state)
		{
			// button is not stabile
			l_button_state = current_button_state;
		}
		else
		{
			// button is stable -> broadcast changes
			change_mask = l_prev_button_state ^ l_button_state;
			if(change_mask != 0)
			{
				i = 0;
				while(g_joystick_button_info[i].KeyCode != sysVKC_NULL)
				{
					if((change_mask & (1<<i)) != 0 )
					{
						sysUserInputEventHandler(drvJOYSTICK_DEVICE_NUMBER, (((l_button_state >> i) & 0x01) != 0)?sysUIEC_Pressed:sysUIEC_Released, sysUIET_Key, g_joystick_button_info[i].KeyCode);
					}

					i++;
				}
			}

			// update prev state
			l_button_state = current_button_state;
			l_prev_button_state = current_button_state;
		}
	}
}
