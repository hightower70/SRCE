/*****************************************************************************/
/* File browser renderer functions                                           */
/*                                                                           */
/* Copyright (C) 2014-2015 Laszlo Arvai                                      */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

#ifndef __fbRenderer_h
#define __fbRenderer_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <guiBlackAndWhiteGraphics.h>
#include <sysTypes.h>
#include <fbTypes.h>
#include "sysConfig.h"

/*****************************************************************************/
/* Function prototypes                                                       */
/*****************************************************************************/
uint16_t fbRendererGetDisplayableItemCount(void);
//sysResourceAddress flrGetFileIcon(fbFileInformation* in_file_info);
uint16_t fbRenderItem(fbFileInformation* in_file_info, uint16_t in_screen_index, bool in_selected, uint16_t in_horizontal_text_offset);
void fbRenderHeader(sysString in_title, sysString in_path);
void fbRenderFooter(uint16_t in_file_index, uint16_t in_file_count, fbFileInformation* in_file_info);
void fbRenderWaitIndicatorShow(void);
void fbRenderWaitIndicatorNext(uint8_t* in_current_phase);
void fbRefreshScreen(void);
void fbRendererInit(void);

void fbRenderGetWaitIndicatorRect(guiCoordinate* out_left, guiCoordinate* out_top, guiCoordinate* out_right, guiCoordinate* out_bottom);

#endif