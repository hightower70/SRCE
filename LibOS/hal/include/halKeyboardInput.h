/*****************************************************************************/
/* Keyboard input repated HAL functions                                      */
/*                                                                           */
/* Copyright (C) 2016 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/
#ifndef __halKeyboardInput_h
#define __halKeyboardInput_h

/*****************************************************************************/
/* Function prototypes                                                       */
/*****************************************************************************/

void halKeyboardInputInitialize(void);
void halKeyboardInputCleanup(void);
void halKeyboardDispatchEvent(void);

#endif
