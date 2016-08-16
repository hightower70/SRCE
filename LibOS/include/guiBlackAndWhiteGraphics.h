/*****************************************************************************/
/* Black and White (1bpp) graphics software rendering routines               */
/*                                                                           */
/* Copyright (C) 2014-2015 Laszlo Arvai                                      */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

#ifndef __guiBlackAndWhiteGraphics_h
#define __guiBlackAndWhiteGraphics_h

///////////////////////////////////////////////////////////////////////////////
// Includes
#include <guiTypes.h>
#include <guiCommon.h>

///////////////////////////////////////////////////////////////////////////////
// Const
#define guiBRUSH_HEIGHT 8
#define guiSYSTEM_BRUSH_COUNT 3

///////////////////////////////////////////////////////////////////////////////
// Function prototypes

void guiBlackAndWhiteGraphicsInitialize(void);

void guiClearScreen(void);
void guiRefreshScreen(void);

// canvas functions
void guiOpenCanvas( guiCoordinate in_left, guiCoordinate in_top, guiCoordinate in_right, guiCoordinate in_bottom );
void guiCloseCanvas( bool in_refresh );
void guiRefreshCanvas( void );

void guiSetClipping(guiCoordinate in_left, guiCoordinate in_top, guiCoordinate in_right, guiCoordinate in_bottom);
void guiSetClippingRect(guiRect in_clipping_rect);
void guiResetClipping(void);

void guiSetPen( uint8_t in_index );
void guiSetDrawMode( uint8_t in_draw_mode );

void guiSetClipping(guiCoordinate in_left, guiCoordinate in_top, guiCoordinate in_right, guiCoordinate in_bottom);
void guiSetClippingRect(guiRect in_clipping_rect);
void guiResetClipping(void);

void guiDrawPixel( guiCoordinate in_x, guiCoordinate in_y );

// Line functions
void guiDrawLine( guiCoordinate in_startx, guiCoordinate in_starty, guiCoordinate in_endx, guiCoordinate in_endy );
void guiDrawHorizontalLine( guiCoordinate in_startx, guiCoordinate in_endx, guiCoordinate in_y );
void guiDrawVerticalLine( guiCoordinate in_x, guiCoordinate in_starty, guiCoordinate in_endy );

// Rectangle functions
void guiDrawRectangle( guiCoordinate in_left, guiCoordinate in_top, guiCoordinate in_right, guiCoordinate in_bottom );
void guiFillRectangle( guiCoordinate in_left, guiCoordinate in_top, guiCoordinate in_right, guiCoordinate in_bottom );

// Bitmap functions
void guiDrawBitmap( guiCoordinate in_x, guiCoordinate in_y, sysResourceAddress in_bitmap_address );

void guiSetSystemBrush( uint8_t in_index );

// Text functions
void guiDrawText(guiCoordinate in_x, guiCoordinate in_y, sysString in_string);
void guiDrawResourceText(guiCoordinate in_x, guiCoordinate in_y, sysResourceAddress in_string);
//void guiSetTextAlign( uint8_t in_align );
//void guiSetFont( sysResourceAddress in_font_handle );
//guiSize guiGetTextExtent( sysString in_string );


#endif
