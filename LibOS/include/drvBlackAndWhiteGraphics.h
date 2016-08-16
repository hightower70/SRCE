/*****************************************************************************/
/* Black and White (1bpp) graphics software rendering routines               */
/*                                                                           */
/* Copyright (C) 2014-2015 Laszlo Arvai                                      */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

#ifndef _drvBlackAndWhiteGraphics_h
#define _drvBlackAndWhiteGraphics_h

///////////////////////////////////////////////////////////////////////////////
// Includes
#include <guiTypes.h>

///////////////////////////////////////////////////////////////////////////////
// Constants
#define guiBITS_PER_PIXEL 1

#ifdef _WIN32
// round to 32 bit 
#define guiFRAME_BUFFER_ROW_LENGTH (((guiSCREEN_WIDTH * guiBITS_PER_PIXEL) + 31 ) / 32 * 4 )
#else
#define guiFRAME_BUFFER_ROW_LENGTH ((guiSCREEN_WIDTH + 7)/ 8)
#endif
						
///////////////////////////////////////////////////////////////////////////////
// Global variables
extern uint8_t g_gui_frame_buffer[];
extern uint8_t g_gui_pen_index;
extern uint8_t g_gui_draw_mode;
extern uint8_t g_gui_current_brush[];


///////////////////////////////////////////////////////////////////////////////
// Function prototypes
void drvGraphicsInitialize(void);
void drvGraphicsUpdateCanvas( guiCoordinate in_left, guiCoordinate in_top, guiCoordinate in_right, guiCoordinate in_bottom );

void drvGraphicsSetForegroundPixel(guiCoordinate in_x, guiCoordinate in_y);
void drvGraphicsSetBackgroundPixel(guiCoordinate in_x, guiCoordinate in_y);

void drvGraphicsFillAreaWithBrush(guiCoordinate in_left, guiCoordinate in_top, guiCoordinate in_right, guiCoordinate in_bottom);
void drvGraphicsDrawHorizontalLine( guiCoordinate in_startx, guiCoordinate in_endx, guiCoordinate in_y );
void drvGraphicsDrawVerticalLine( guiCoordinate in_x, guiCoordinate in_starty, guiCoordinate in_endy );

void drvGraphicsBitBltFromResource(	guiCoordinate in_destination_x, guiCoordinate in_destination_y, 
																		guiCoordinate in_destination_width, guiCoordinate in_destination_height,
																		guiCoordinate in_source_x, guiCoordinate in_source_y,
																		guiCoordinate in_source_width, guiCoordinate in_source_height, 
																		sysResourceAddress in_source_bitmap, uint8_t in_source_bit_per_pixel);

void guiGraphicsUpdateCanvas( guiCoordinate in_left, guiCoordinate in_top, guiCoordinate  in_right, guiCoordinate  in_bottom );

#endif