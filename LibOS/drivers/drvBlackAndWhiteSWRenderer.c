/*****************************************************************************/
/* Black and white graphics functions                                        */
/*                                                                           */
/* Copyright (C) 2014-2015 Laszlo Arvai                                      */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
// Includes
#include <guiBlackAndWhiteGraphics.h>
#include <drvBlackAndWhiteGraphics.h>
#include <guiCommon.h>
#include "sysconfig.H"

///////////////////////////////////////////////////////////////////////////////
// Const

///////////////////////////////////////////////////////////////////////////////
// GUI frame buffer
#define guiPIXELS_PER_BYTE 8

#ifndef guiCUSTOM_FRAMEBUFFER
uint8_t g_gui_frame_buffer[guiFRAME_BUFFER_ROW_LENGTH * guiSCREEN_HEIGHT];
#endif


///////////////////////////////////////////////////////////////////////////////
// Local variables

// pixel masks
static const uint8_t l_pixel_mask[8] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
static const uint8_t l_pixel_row_start_mask[8] = { 0xff, 0x7f, 0x3f, 0x1f, 0x0f, 0x07, 0x03, 0x01 };
static const uint8_t l_pixel_row_end_mask[8] = { 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff };

///////////////////////////////////////////////////////////////////////////////
// Global variables

// current brush (default is white)
uint8_t g_gui_current_brush[guiBRUSH_HEIGHT] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

///////////////////////////////////////////////////////////////////////////////
// Functions
void guiBitBltFromROM(	sysResourceAddress in_address,
												guiCoordinate in_dest_x, guiCoordinate in_dest_y,
												guiCoordinate in_source_x, guiCoordinate in_source_y, 
												guiCoordinate in_width, guiCoordinate in_height,
												uint8_t in_bits_per_pixel );

///////////////////////////////////////////////////////////////////////////////
/// @brief Sets pixel to the foreground color
/// @param in_x X coordinate of the pixel
/// @param in_y Y coordinate of the pixel
void drvGraphicsSetForegroundPixel(guiCoordinate in_x, guiCoordinate in_y)
{
	uint16_t address;

	address = in_y * guiFRAME_BUFFER_ROW_LENGTH + in_x / guiPIXELS_PER_BYTE;

	g_gui_frame_buffer[address] |= l_pixel_mask[in_x % guiPIXELS_PER_BYTE];
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Sets pixel to the background color
/// @param in_x X coordinate of the pixel
/// @param in_y Y coordinate of the pixel
void drvGraphicsSetBackgroundPixel(guiCoordinate in_x, guiCoordinate in_y)
{
	uint16_t address;

	address = in_y * guiFRAME_BUFFER_ROW_LENGTH + in_x / guiPIXELS_PER_BYTE;

	g_gui_frame_buffer[address] &= ~(l_pixel_mask[in_x % guiPIXELS_PER_BYTE]);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Fills rectangle with the current brush
/// @param in_left Left-Top corner X coordinate
/// @param in_top Left-Top corner Y coordinate
/// @param in_width Width of the rectangle
/// @param in_height height of the rectangle
void drvGraphicsFillAreaWithBrush(guiCoordinate in_left, guiCoordinate in_top, guiCoordinate in_right, guiCoordinate in_bottom)
{
  guiCoordinate y;
	guiCoordinate i;
	uint16_t address1;
	uint16_t address2;
	uint8_t mask1;
	uint8_t mask2;

  // calculate addresses and masks
	address1	=	address2 = in_top * guiFRAME_BUFFER_ROW_LENGTH;
	address1	+= in_left / guiPIXELS_PER_BYTE;
	mask1			= l_pixel_row_start_mask[in_left % guiPIXELS_PER_BYTE];
	address2	+= in_right / guiPIXELS_PER_BYTE;
	mask2			= l_pixel_row_end_mask[in_right % guiPIXELS_PER_BYTE];
	if( address1 == address2 )
	{
    mask1 = mask1 & mask2;
		mask2 = ~mask1;

	  for( y = in_top; y <= in_bottom; y++ )
    {
			g_gui_frame_buffer[address1] = (g_gui_current_brush[y % guiBRUSH_HEIGHT] & mask1) | (g_gui_frame_buffer[address1] & mask2);

      address1 += guiFRAME_BUFFER_ROW_LENGTH;
		}
	}
	else
	{
    for( y = in_top; y <= in_bottom; y++ )
    {
			g_gui_frame_buffer[address1] = (g_gui_current_brush[y % guiBRUSH_HEIGHT] & mask1) | (g_gui_frame_buffer[address1] & ~mask1) ;

			for( i = address1 + 1; i < address2; i++ )
				g_gui_frame_buffer[i] = g_gui_current_brush[y % guiBRUSH_HEIGHT];

			g_gui_frame_buffer[address2] = (g_gui_current_brush[y % guiBRUSH_HEIGHT] & mask2) | (g_gui_frame_buffer[address2] & ~mask2) ;

      address1 += guiFRAME_BUFFER_ROW_LENGTH;
      address2 += guiFRAME_BUFFER_ROW_LENGTH;
    }
	}
}

///////////////////////////////////////////////////////////////////////////////
// Draw horizontal line
void drvGraphicsDrawHorizontalLine( guiCoordinate in_startx, guiCoordinate in_endx, guiCoordinate in_y )
{
	uint16_t address;
	uint16_t address1;
	uint16_t address2;
	uint8_t mask1;
	uint8_t mask2;

	// draw the line
	address1	=	address2 = in_y * guiFRAME_BUFFER_ROW_LENGTH;
	address1	+= in_startx / guiPIXELS_PER_BYTE;
	mask1			= l_pixel_row_start_mask[in_startx % guiPIXELS_PER_BYTE];
	address2	+= in_endx / guiPIXELS_PER_BYTE;
	mask2			= l_pixel_row_end_mask[in_endx % guiPIXELS_PER_BYTE];
	if( address1 == address2 )
	{
		switch( g_gui_pen_index )
		{
			// backgound pen
			case 0:
				g_gui_frame_buffer[address1] &= ~(mask1 & mask2);
				break;

			// foreground pen
			case 1:
				g_gui_frame_buffer[address1] |= mask1 & mask2;
				break;
		}
	}
	else
	{
		switch( g_gui_pen_index )
		{
			// background pen
			case 0:
				g_gui_frame_buffer[address1] &= ~mask1;
	
				for( address = address1 + 1; address < address2; address++ )
					g_gui_frame_buffer[address] = 0;

				g_gui_frame_buffer[address2] &= ~mask2;

				break;

			// foreground pen
			case 1:
				g_gui_frame_buffer[address1] |= mask1;
	
				for( address = address1 + 1; address < address2; address++ )
					g_gui_frame_buffer[address] = 0xff;

				g_gui_frame_buffer[address2] |= mask2;

				break;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// Draw vertical lines
void drvGraphicsDrawVerticalLine( guiCoordinate in_x, guiCoordinate in_starty, guiCoordinate in_endy )
{
	uint16_t address1;
	uint8_t mask1;

	// draw the line
  address1	=	in_starty * guiFRAME_BUFFER_ROW_LENGTH + in_x / guiPIXELS_PER_BYTE;
  mask1			= l_pixel_mask[in_x % guiPIXELS_PER_BYTE];

  switch( g_gui_pen_index )
  {
	  // backgound pen
	  case 0:
		  mask1 = ~mask1;
		  while( in_starty <= in_endy )
		  {
			  g_gui_frame_buffer[address1] &= mask1;
			  address1 += guiFRAME_BUFFER_ROW_LENGTH;
			  in_starty++;
		  }
		  break;

	  // foreground pen
	  case 1:
		  while( in_starty <= in_endy )
		  {
			  g_gui_frame_buffer[address1] |= mask1;
			  address1 += guiFRAME_BUFFER_ROW_LENGTH;
			  in_starty++;
		  }
		  break;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Bit-block-transfer for ROM bitmaps
void drvGraphicsBitBltFromResource(	guiCoordinate in_destination_x, guiCoordinate in_destination_y, 
																		guiCoordinate in_destination_width, guiCoordinate in_destination_height,
																		guiCoordinate in_source_x, guiCoordinate in_source_y,
																		guiCoordinate in_source_width, guiCoordinate in_source_height, 
																		sysResourceAddress in_source_bitmap, uint8_t in_source_bit_per_pixel)
{
	int16_t source_scan_line_length;
	int16_t scan_line;
	int16_t destination_address;
	int16_t destination_row_end_address;
	int16_t destination_row_start_address;
	sysResourceAddress source_row_start_address;
	sysResourceAddress source_row_end_address;
	sysResourceAddress source_address;
	uint8_t source_start_mask;
	uint8_t source_end_mask;
	uint8_t destination_start_mask;
	uint8_t destination_end_mask;
	int8_t shift;
	int8_t real_shift;
	uint16_t data;
	uint8_t invert_mask;
	bool transparent_mode;
	bool invert_mode;

	// draw only 1bpp images
	if(in_source_bit_per_pixel != 1)
		return;

	// init
	source_scan_line_length = (in_source_bit_per_pixel * in_source_width + 7 ) / 8;

	// calculate destination addresses
	destination_row_start_address	=	destination_row_end_address = in_destination_y * guiFRAME_BUFFER_ROW_LENGTH;
	destination_row_start_address += in_destination_x / guiPIXELS_PER_BYTE;
	destination_row_end_address += (in_destination_x + in_destination_width-1) / guiPIXELS_PER_BYTE;

	source_row_end_address = source_row_start_address = in_source_bitmap + in_source_y * source_scan_line_length;
	source_row_start_address += in_source_x / guiPIXELS_PER_BYTE;
	source_row_end_address += (in_source_x + in_destination_width - 1) / guiPIXELS_PER_BYTE;

	// get masks
	source_start_mask = l_pixel_row_start_mask[in_destination_x % guiPIXELS_PER_BYTE];
	destination_start_mask = ~source_start_mask;
	source_end_mask = l_pixel_row_end_mask[(in_destination_x + in_destination_width - 1) % guiPIXELS_PER_BYTE];
	destination_end_mask = ~source_end_mask;

	// calculate bit shift
	//shift = (in_destination_x % guiPIXELS_PER_BYTE) - (in_source_x % 8);
	shift = (in_destination_x - in_source_x) % guiPIXELS_PER_BYTE;

//	printf("s=%d srsa=%d ssm=%x sem=%x dsm=%x dem=%x",shift, source_row_start_address, source_start_mask, source_end_mask, destination_start_mask, destination_end_mask );

	// activate transparent mode
	if( (g_gui_draw_mode & guiDM_Transparent) != 0 )
	{
		transparent_mode = true;

		destination_start_mask	= 0xff;
		destination_end_mask		= 0xff;
	}
	else
	{
		transparent_mode = false;
	}

	// activate invert mode
	if( (g_gui_draw_mode & guiDM_Inverse) != 0 )
	{
		invert_mask = 0xff;
		invert_mode = true;
	}
	else
	{
		invert_mask = 0;
		invert_mode = false;
	}

	// do bitblt
	for( scan_line = 0; scan_line < in_destination_height; scan_line++ )
	{
		source_address = source_row_start_address;
		destination_address = destination_row_start_address;

		if( destination_row_start_address == destination_row_end_address )
		{
			data =  ( drvResourceReadByte( source_row_start_address ) << 8 ) +
							( drvResourceReadByte( source_row_end_address ) );

			if( shift > 0 )
			{
				data >>= shift;
			}
			else
			{
				data <<= -shift;
				data = data >> 8;
			}

			if( transparent_mode )
			{
				if( invert_mode )
					g_gui_frame_buffer[destination_row_start_address] = (g_gui_frame_buffer[destination_row_start_address] & ( ~(data & source_start_mask & source_end_mask)));
				else
					g_gui_frame_buffer[destination_row_start_address] = (g_gui_frame_buffer[destination_row_start_address] | ( data & source_start_mask & source_end_mask));
			}
			else
				g_gui_frame_buffer[destination_row_start_address] = (g_gui_frame_buffer[destination_row_start_address] & (destination_start_mask | destination_end_mask)) | ( (data ^ invert_mask) & source_start_mask & source_end_mask);
		}
		else
		{
			// first byte
			if( shift > 0 )
			{
				data = ( drvResourceReadByte( source_address++ ) ) << 8;
				data >>= shift;
				real_shift = 8 - shift;
			}
			else
			{
				data = (( drvResourceReadByte( source_address++ )  ) << 8 );
				
				if( source_row_start_address != source_row_end_address )
					data |= drvResourceReadByte( source_address++ );
				else
					data |= data >> 8;

				real_shift = -shift;

				data <<= real_shift;
			}

			if( invert_mode )
			{
				if( transparent_mode )
					g_gui_frame_buffer[destination_address] = (g_gui_frame_buffer[destination_address]) & ( ~((data >> 8 ) & source_start_mask) );
				else
					g_gui_frame_buffer[destination_address] = (g_gui_frame_buffer[destination_address] & destination_start_mask ) | ( (~(data >> 8 )) & source_start_mask );
			}
			else
				g_gui_frame_buffer[destination_address] = (g_gui_frame_buffer[destination_address] & destination_start_mask) | ( (data >> 8 ) & source_start_mask );
	
			destination_address++;

			// intermediate bytes
			while( destination_address < destination_row_end_address )
			{
				data <<= 8;

				data |= drvResourceReadByte( source_address++ ) << real_shift;

				if( invert_mode )
				{
					if( transparent_mode )
						g_gui_frame_buffer[destination_address] = g_gui_frame_buffer[destination_address] | (~(data >> 8 ));
					else
						g_gui_frame_buffer[destination_address] = ~(data >> 8 );
				}
				else
					g_gui_frame_buffer[destination_address] = (uint8_t)(data>>8);

				destination_address++;
			}

			// last byte
			data <<= 8;
			data |= ( drvResourceReadByte( source_row_end_address ) ) << real_shift;

			if( invert_mode )
			{
				if( transparent_mode)
					g_gui_frame_buffer[destination_address] = (g_gui_frame_buffer[destination_address]) & ( ~((data >> 8 ) & source_end_mask) );
				else
					g_gui_frame_buffer[destination_address] = (g_gui_frame_buffer[destination_address] & destination_end_mask) | ((~(data >> 8 )) & source_end_mask );
			}
			else
				g_gui_frame_buffer[destination_address] = (g_gui_frame_buffer[destination_address] & destination_end_mask) | (( data >> 8 ) & source_end_mask );

			destination_address++;
		}

		destination_row_start_address += guiFRAME_BUFFER_ROW_LENGTH;
		destination_row_end_address += guiFRAME_BUFFER_ROW_LENGTH;
		source_row_start_address += source_scan_line_length;
		source_row_end_address += source_scan_line_length;
	}
}



#if 0
///////////////////////////////////////////////////////////////////////////////
// Draw line
void guiDrawLine( dosInt16 in_startx, dosInt16 in_starty, dosInt16 in_endx, dosInt16 in_endy )
{
	dosInt16 i;
	dosInt16 address1;
	dosInt16 address2;
	uint8_t mask1;
	dosInt16 dx, dy;
	dosInt16 dx_abs, dy_abs;

	/**************************/
	/* handle horizontal line */
	/**************************/
	if( in_starty == in_endy )
	{
    guiDrawHorizontalLine( in_startx, in_endx, in_starty );
	}
	else
	{
		/*************************/
		/* handle vertical lines */
		/*************************/
		if( in_startx == in_endx )
		{
      guiDrawVerticalLine( in_startx, in_starty, in_endy );
		}
		else
		{
			/*************************/
			/* handle any other line */
			/*************************/
			dx = in_endx - in_startx;
			dy = in_endy - in_starty;

			///////////////////////
			// do cliping 
			///////////////////////
			// 'start' coordinates clipping
			//
			// (startx <-> left side)
			if( in_startx < l_clip_rect.Left )
			{
				// if the line is outside
				if( in_endx < l_clip_rect.Left )
					return;
				
				// try to find clip point
				i = l_clip_rect.Left - in_startx;

				dy_abs = in_starty + (dosInt16)(( ((dosInt32)i) * 256 * dy / dx ) / 256);

				if( dy_abs >= l_clip_rect.Top && dy_abs <= l_clip_rect.Bottom )
				{
					in_startx = l_clip_rect.Left;
					in_starty = dy_abs;
				}
			}
			else
			{
				// startx <-> right side
				if( in_startx > l_clip_rect.Right )
				{
					// if the line is outside
					if( in_endx > l_clip_rect.Right )
						return;

					// try to find clip ponit
					i = l_clip_rect.Right - in_startx;

					dy_abs = in_starty + (dosInt16)((  ((dosInt32)i) * 256 * dy / dx ) / 256);
	
					if( dy_abs >= l_clip_rect.Top && dy_abs <= l_clip_rect.Bottom )
					{
						in_startx = l_clip_rect.Right;
						in_starty = dy_abs;
					}
				}
			}

			// do clipping (starty <-> top side)
			if( in_starty < l_clip_rect.Top )
			{
				// if the line is outside
				if( in_endy < l_clip_rect.Top )
					return;
				
				// try to find clip point
				i = l_clip_rect.Top - in_starty;

				dx_abs = in_startx + (dosInt16)(( ((dosInt32)i) * 256 * dx / dy ) / 256);

				if( dx_abs >= l_clip_rect.Left && dx_abs <= l_clip_rect.Right )
				{
					in_startx = dx_abs;
					in_starty = l_clip_rect.Top;
				}
			}
			else
			{
				// starty <-> bottom side
				if( in_starty > l_clip_rect.Bottom )
				{
					// if the line is outside
					if( in_endy > l_clip_rect.Bottom )
						return;

					// try to find clip ponit
					i = l_clip_rect.Bottom - in_starty;

					dx_abs = in_startx + (dosInt16)(( ((dosInt32)i) * 256 * dx / dy ) / 256);
		
					if( dx_abs >= l_clip_rect.Left && dx_abs <= l_clip_rect.Right )
					{
						in_startx = dx_abs;
						in_starty = l_clip_rect.Bottom;
					}
				}
			}

			///////////////////////
			// 'end' coordinates clipping
			//
			// (endx<->left side)
			if( in_endx < l_clip_rect.Left )
			{
				// try to find clip point
				i = in_endx - l_clip_rect.Left;

				dy_abs = in_endy - (dosInt16)(( ((dosInt32)i) * 256 * dy / dx ) / 256);

				if( dy_abs >= l_clip_rect.Top && dy_abs <= l_clip_rect.Bottom )
				{
					in_endx = l_clip_rect.Left;
					in_endy = dy_abs;
				}
			}
			else
			{
				// endx <-> right side
				if( in_endx > l_clip_rect.Right )
				{
					// try to find clip ponit
					i = in_endx - l_clip_rect.Right;

					dy_abs = in_endy - (dosInt16)(( ((dosInt32)i) * 256 * dy / dx ) / 256);

					if( dy_abs >= l_clip_rect.Top && dy_abs <= l_clip_rect.Bottom )
					{
						in_endx = l_clip_rect.Right;
						in_endy = dy_abs;
					}
				}
			}

			// do clipping (endy <-> top side)
			if( in_endy < l_clip_rect.Top )
			{
				// try to find clip point
				i = in_endy - l_clip_rect.Top;

				dx_abs = in_endx - (dosInt16)(( ((dosInt32)i) * 256 * dx / dy ) / 256);

				if( dx_abs >= l_clip_rect.Left && dx_abs <= l_clip_rect.Right )
				{
					in_endx = dx_abs;
					in_endy = l_clip_rect.Top;
				}
			}
			else
			{
				// starty <-> bottom side
				if( in_endy > l_clip_rect.Bottom )
				{
					// try to find clip ponit
					i =  in_endy - l_clip_rect.Bottom;

					dx_abs = in_endx - (dosInt16)(( ((dosInt32)i) * 256 * dx / dy ) / 256);

					if( dx_abs >= l_clip_rect.Left && dx_abs <= l_clip_rect.Right )
					{
						in_endx = dx_abs;
						in_endy = l_clip_rect.Bottom;
					}
				}
			}

			// check again, it it sill outside, their is nothing to draw
			if( in_startx < l_clip_rect.Left || in_startx > l_clip_rect.Right ||
					in_starty < l_clip_rect.Top || in_starty > l_clip_rect.Bottom ||
					in_endx < l_clip_rect.Left || in_endx > l_clip_rect.Right ||
					in_endy < l_clip_rect.Top || in_endy > l_clip_rect.Bottom )
				return;

			// update slope
			dx = in_endx - in_startx;
			dy = in_endy - in_starty;

			// if it's only one pixel
			if( dx == 0 && dy == 0 )
			{
				guiDrawPixel( in_startx, in_starty );
				return;
			}

			if( dx < 0 )
				dx_abs = -dx;
			else
				dx_abs = dx;

			if( dy < 0 )
				dy_abs = -dy;
			else
				dy_abs = dy;

			// decide main axis
			if( dy_abs > dx_abs )
			{
				//////////////////
				// go along Y axis

				// get mask index (i), mask (mask1) and pixel address (address1)
				i = in_startx % guiguiPIXELS_PER_BYTE;
				if( l_pen_index == 0 )
					mask1 = ~l_pixel_mask[i];
				else
					mask1	= l_pixel_mask[i];

				address1 = in_starty * guiFRAME_BUFFER_ROW_LENGTH + in_startx/guiguiPIXELS_PER_BYTE;

				// initializie fractional part (address2)
				address2 = dy_abs/2;

				// draw the line
				while( 1 ) // for efficiency leave the loop using break
				{
					// do selected operation (or, and)
					if( l_pen_index == 0 )
						g_gui_frame_buffer[address1] &= mask1;
					else
						g_gui_frame_buffer[address1] |= mask1;

					// calculate fractional part
					address2 += dx_abs;

					// overflow of fractional part -> do X step
					if( address2 >= dy_abs )
					{
						address2 -= dy_abs;// remove integer part

						// get new address and mask index
						if( dx < 0 )
						{
							if( i == 0 )
							{
								i = 7;
								address1--;
							}
							else
								i--;
						}
						else
						{
							if( i == 7 )
							{
								i = 0;
								address1++;
							}
							else
								i++;
						}

						// get new mask
						if( l_pen_index == 0 )
							mask1 = ~l_pixel_mask[i];
						else
							mask1	= l_pixel_mask[i];
					}

					// exit from the loop
					if( in_starty == in_endy )
						break;

					// next line
					if( dy > 0 )
					{
						in_starty++;
						address1 += guiFRAME_BUFFER_ROW_LENGTH; 
					}
					else
					{
						in_starty--;
						address1 -= guiFRAME_BUFFER_ROW_LENGTH; 
					}
				}
			}
			else
			{
				//////////////////
				// go along X axis

				// get mask index (i), mask (mask1) and pixel address (address1)
				i = in_startx % guiPIXELS_PER_BYTE;
				mask1	= l_pixel_mask[i];
				address1 = in_starty * guiFRAME_BUFFER_ROW_LENGTH + in_startx/ guiPIXELS_PER_BYTE;

				// initializie fractional part (address2)
				address2 = dx_abs / 2;

				// finally draw the line
				while( 1 ) // for efficiency leave the loop using break
				{
					address2 += dy_abs;

					if( address2 >= dx_abs )
					{
						address2 -= dx_abs; // remove integer part

						// put pixels
						if( l_pen_index == 0 )
							g_gui_frame_buffer[address1] &= ~mask1;
						else
							g_gui_frame_buffer[address1] |= mask1;

						mask1 = 0;

						// new address
						if( dy < 0 )
							address1 -= guiFRAME_BUFFER_ROW_LENGTH;
						else
							address1 += guiFRAME_BUFFER_ROW_LENGTH;
					}

					// X step
					if( dx < 0 )
					{
						in_startx--;

						if( i == 0 )
						{
							// store pixels
							if( l_pen_index == 0 )
								g_gui_frame_buffer[address1] &= ~mask1;
							else
								g_gui_frame_buffer[address1] |= mask1;

							// new mask address
							i = 7;
							address1--;
							mask1 = 0;
						}
						else
							i--;
					}
					else
					{
						in_startx++;

						if( i == 7 )
						{
							// store pixels
							if( l_pen_index == 0 )
								g_gui_frame_buffer[address1] &= ~mask1;
							else
								g_gui_frame_buffer[address1] |= mask1;

							// new mask address
							i = 0;
							mask1 = 0;
							address1++;
						}
						else
							i++;
					}

					mask1 |= l_pixel_mask[i];

					// put the final pixel and leave the loop
					if( in_startx == in_endx )
					{
							if( l_pen_index == 0 )
								g_gui_frame_buffer[address1] &= ~mask1;
							else
								g_gui_frame_buffer[address1] |= mask1;

							break;
					}
				}
			}
		}
	}
}
#endif

