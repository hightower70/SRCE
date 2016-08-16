/*****************************************************************************/
/* Simple Graphical Configuration Menu Renderer Functions                    */
/*                                                                           */
/* Copyright (C) 2015 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

#ifndef __gcmRenderer_h
#define __gcmRenderer_h

///////////////////////////////////////////////////////////////////////////////
// Includes
#include <gcmGraphicsConfigMenu.h>

///////////////////////////////////////////////////////////////////////////////
// Function prototypes
void gcmRendererInitialize(gcmMenuInfo* in_menu_info);
uint8_t gcmRendererGetDisplayableItemCount(void);
void gcmRenderItem(gcmMenuInfo* in_menu_info, uint8_t in_item_index, uint8_t in_screen_index, bool in_selected);
void gcmRenderValue(gcmMenuInfo* in_menu_info, uint8_t in_item_index, uint8_t in_screen_index, bool in_selected);
void gcmRenderBorder(gcmMenuInfo* in_menu_info);

#endif
