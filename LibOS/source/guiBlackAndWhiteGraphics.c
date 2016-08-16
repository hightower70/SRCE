/*****************************************************************************/
/* Black and white graphics functions                                        */
/*                                                                           */
/* Copyright (C) 2014-2015 Laszlo Arvai                                      */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/
#include <guiBlackAndWhiteGraphics.h>
#include <drvBlackAndWhiteGraphics.h>
#include <guiCommon.h>
#include <sysString.h>

///////////////////////////////////////////////////////////////////////////////
// Module global variables
static guiRect l_clip_rect;
static guiRect l_refresh_rect;

// system brushes
static uint8_t l_system_brush[guiBRUSH_HEIGHT*guiSYSTEM_BRUSH_COUNT] = 
{	
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		// solid black
	0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55,		// halftone
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff		// solid white brush
};

///////////////////////////////////////////////////////////////////////////////
// Module global variables
uint8_t g_gui_pen_index;
uint8_t g_gui_draw_mode;


///////////////////////////////////////////////////////////////////////////////
/// @brief Initializes black and white graphics engine
void guiGraphicsInitialize(void)
{
	drvGraphicsInitialize();

	// init cliping rectangle
	l_clip_rect.Left	= 0;
	l_clip_rect.Top		= 0;
	l_clip_rect.Right		= guiSCREEN_WIDTH-1;
	l_clip_rect.Bottom	= guiSCREEN_HEIGHT-1;

  // init refresh rect
  l_refresh_rect.Left   = guiINVALID_COORDINATE;
  l_refresh_rect.Top    = guiINVALID_COORDINATE;
  l_refresh_rect.Right  = guiINVALID_COORDINATE;
  l_refresh_rect.Bottom = guiINVALID_COORDINATE;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Opens canvas for drawing. It must be called before any drawing command issued. The drwing is limited (clipped) to the specified area.
/// @param in_left Left edge X coordinate
/// @param in_top Top edge Y coordinate
/// @param in_right Right edge X coordinate
/// @param in_bottom Bottom edge Y coordinate
void guiOpenCanvas( guiCoordinate in_left, guiCoordinate in_top, guiCoordinate in_right, guiCoordinate in_bottom )
{
	guiCoordinate i;

	// swap coordinates

	// x
	if( in_right < in_left )
	{
		i = in_right;
		in_right = in_left;
		in_left = i;
	}

	// y
	if( in_bottom < in_top )
	{
		i = in_bottom;
		in_bottom = in_top;
		in_top = i;
	}

	// check left coordinate
	if( in_left < 0 )
		in_left = 0;

	if( in_left >= guiSCREEN_WIDTH )
		in_left = guiSCREEN_WIDTH - 1;

	// check right coordinate
	if( in_right < 0 )
		in_right = 0;

	if( in_right >= guiSCREEN_WIDTH )
		in_right = guiSCREEN_WIDTH - 1;

	// check top coordinate
	if( in_top < 0 )
		in_top = 0;

	if( in_top >= guiSCREEN_HEIGHT )
		in_top = guiSCREEN_HEIGHT - 1;

	// check bottom coordinate
	if( in_bottom < 0 )
		in_bottom = 0;

	if( in_bottom >= guiSCREEN_HEIGHT )
		in_bottom = guiSCREEN_HEIGHT - 1;

	// store coordinates
	l_refresh_rect.Left		= in_left;
	l_refresh_rect.Top		= in_top;
	l_refresh_rect.Right	= in_right;
	l_refresh_rect.Bottom	= in_bottom;

	// set default clipping rect
	l_clip_rect.Left		= in_left;
	l_clip_rect.Top			=	in_top;
	l_clip_rect.Right		= in_right;
	l_clip_rect.Bottom	= in_bottom;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Closes canvas and refreshes screen content if requested
void guiCloseCanvas(bool in_refresh)
{
  if( in_refresh )
    guiRefreshCanvas();
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Sets clipping rectangle for graphics operation
/// @param in_left Left edge X coordinate
/// @param in_top Top edge Y coordinate
/// @param in_right Right edge X coordinate
/// @param in_bottom Bottom edge Y coordinate
void guiSetClipping(guiCoordinate in_left, guiCoordinate in_top, guiCoordinate in_right, guiCoordinate in_bottom)
{
	guiCoordinate left, top, right, bottom;

	// swap coordiantes
	if(in_left <= in_right)
	{
		left = in_left;
		right = in_right;
	}
	else
	{
		left = in_right;
		right = in_left;
	}

	if(in_top <= in_bottom)
	{
		top = in_top;
		bottom = in_bottom;
	}
	else
	{
		top = in_bottom;
		bottom = in_top;
	}

	// Left
	if(left > l_refresh_rect.Right)
	{
		l_clip_rect.Left = l_refresh_rect.Right;
	}
	else
	{
		if(left >= l_refresh_rect.Left)
			l_clip_rect.Left = left;
		else
			l_clip_rect.Left = l_refresh_rect.Left;
	}

	// Top
	if(top > l_refresh_rect.Bottom)
	{
		l_clip_rect.Bottom = l_refresh_rect.Bottom;
	}
	else
	{
		if(top >= l_refresh_rect.Top)
			l_clip_rect.Top = top;
		else
			l_clip_rect.Top = l_refresh_rect.Top;
	}

	// Right
	if(right < l_refresh_rect.Left)
	{
		l_clip_rect.Right = l_refresh_rect.Left;
	}
	else
	{
		if(right <= l_refresh_rect.Right)
			l_clip_rect.Right = right;
		else
			l_clip_rect.Right = l_refresh_rect.Right;
	}

	// Bottom
	if(bottom < l_refresh_rect.Top)
	{
		l_clip_rect.Bottom = l_refresh_rect.Top;
	}
	else
	{
		if(bottom <= l_refresh_rect.Bottom)
			l_clip_rect.Bottom = bottom;
		else
			l_clip_rect.Bottom = l_refresh_rect.Bottom;
	}
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Sets clipping rectangle for graphics operation
/// @param in_clipping_rect Clipping rectangle
void guiSetClippingRect(guiRect in_clipping_rect)
{
	guiSetClipping(in_clipping_rect.Left, in_clipping_rect.Top, l_clip_rect.Right, l_clip_rect.Bottom);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Resets clipping rectangle to include the whole screen
void guiResetClipping(void)
{
	l_clip_rect = l_refresh_rect;
}

void guiRefreshScreen(void)
{
}

///////////////////////////////////////////////////////////////////////////////
// Update canvas
void guiRefreshCanvas( void )
{
  if( l_refresh_rect.Left != guiINVALID_COORDINATE &&
      l_refresh_rect.Top != guiINVALID_COORDINATE &&
      l_refresh_rect.Right != guiINVALID_COORDINATE &&
      l_refresh_rect.Bottom != guiINVALID_COORDINATE )
  {
	  guiGraphicsUpdateCanvas( l_refresh_rect.Left, l_refresh_rect.Top, l_refresh_rect.Right, l_refresh_rect.Bottom );
  }
}

///////////////////////////////////////////////////////////////////////////////
// Sets Drawing mode
void guiSetDrawMode( uint8_t in_draw_mode )
{
	g_gui_draw_mode = in_draw_mode;
}

///////////////////////////////////////////////////////////////////////////////
// Set pen index
void guiSetPen( uint8_t in_index )
{
	g_gui_pen_index = in_index;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Sets current brush to the system brush
void guiSetSystemBrush( uint8_t in_index )
{
	uint8_t i;

	// check index
	if( in_index >= guiSYSTEM_BRUSH_COUNT )
		return;

	// copy brush data
	for( i = 0; i < guiBRUSH_HEIGHT; i++ )
	{
		g_gui_current_brush[i] = l_system_brush[in_index * guiBRUSH_HEIGHT + i];
	}
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Sets pixel to the foreground color
/// @param in_x X coordinate of the pixel
/// @param in_y Y coordinate of the pixel
void guiSetForegroundPixel(guiCoordinate in_x, guiCoordinate in_y)
{
	if( in_x < l_clip_rect.Left || in_x > l_clip_rect.Right ||
			in_y < l_clip_rect.Top || in_y > l_clip_rect.Bottom )
		return;

	drvGraphicsSetForegroundPixel(in_x, in_y);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Sets pixel to the background color
/// @param in_x X coordinate of the pixel
/// @param in_y Y coordinate of the pixel
void guiSetBackgroundPixel(guiCoordinate in_x, guiCoordinate in_y)
{
	if( in_x < l_clip_rect.Left || in_x > l_clip_rect.Right ||
			in_y < l_clip_rect.Top || in_y > l_clip_rect.Bottom )
		return;

	drvGraphicsSetBackgroundPixel(in_x, in_y);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Draws a horizontal line with the current foreground color
/// @param in_left X coordinate of the left point
/// @param in_right X coordinate of the right point
/// @param in_top Y coordinate of the line
void guiDrawHorizontalLine(guiCoordinate in_left, guiCoordinate in_right, guiCoordinate in_top)
{
	//TODO: clipping
	drvGraphicsDrawHorizontalLine(in_left, in_right, in_top);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Draws a vertical line with the current foreground color
/// @param in_left X coordinate of the line
/// @param in_top Y coordinate of the top point
/// @param in_bottom Y coordinate of the bottom point
void guiDrawVerticalLine(guiCoordinate in_left, guiCoordinate in_top, guiCoordinate in_bottom)
{
	//TODO: clipping
	drvGraphicsDrawVerticalLine(in_left, in_top, in_bottom);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Draws a rectangle with the current foreground color
/// @param in_left X coordinate of the left side
/// @param in_top Y coordinate of the top side
/// @param in_right X coordinate of the right side
/// @param in_bottom Y coordinate of the bottom side
void guiDrawRectangle(guiCoordinate in_left, guiCoordinate in_top, guiCoordinate in_right, guiCoordinate in_bottom)
{
	guiCoordinate swap;
	guiCoordinate left, top, right, bottom;

	// exhange coordinates
	if( in_left > in_right )
	{
		swap = in_left;
		in_left = in_right;
		in_right = swap;
	}

	if( in_top > in_bottom )
	{
		swap = in_top;
		in_top = in_bottom;
		in_bottom = swap;
	}

	// if there is nothing to draw
  if( in_right < l_clip_rect.Left || in_left > l_clip_rect.Right ||
      in_top > l_clip_rect.Bottom || in_bottom < l_clip_rect.Top ||
      in_right < in_left || in_bottom < in_top )
    return;

	// clip
	left = in_left;
	if(left < l_clip_rect.Left)
		left = l_clip_rect.Left;

	right = in_right;
	if(in_right > l_clip_rect.Right)
		right = l_clip_rect.Right;

	// draw top side
	if(in_top >= l_clip_rect.Top && in_top <= l_clip_rect.Bottom)
		drvGraphicsDrawHorizontalLine( left, right, in_top );

	// draw bottom side
	if(in_bottom >= l_clip_rect.Top && in_bottom <= l_clip_rect.Bottom)
		drvGraphicsDrawHorizontalLine( left, right, in_bottom);

	// clip
	top = in_top;
	if(top < l_clip_rect.Top)
		top = l_clip_rect.Top;

	bottom = in_bottom;
	if(bottom > l_clip_rect.Bottom)
		bottom = l_clip_rect.Bottom;

	// draw left side
	if(in_left >= l_clip_rect.Left && in_left <= l_clip_rect.Right)
		drvGraphicsDrawVerticalLine( in_left, top, bottom );

	// draw right side
	if(in_right >= l_clip_rect.Left && in_right <= l_clip_rect.Right)
		drvGraphicsDrawVerticalLine( in_right, top, bottom);
}


///////////////////////////////////////////////////////////////////////////////
/// @brief Fill a rectangle with the current foreground color (including rectangle border)
/// @param in_left X coordinate of the left side
/// @param in_top Y coordinate of the top side
/// @param in_right X coordinate of the right side
/// @param in_bottom Y coordinate of the bottom side
void guiFillRectangle(guiCoordinate in_left, guiCoordinate in_top, guiCoordinate in_right, guiCoordinate in_bottom)
{
	guiCoordinate swap;

	// exhange coordinates
	if( in_left > in_right )
	{
		swap = in_left;
		in_left = in_right;
		in_right = swap;
	}

	if( in_top > in_bottom )
	{
		swap = in_top;
		in_top = in_bottom;
		in_bottom = swap;
	}

  // do clipping
  if( in_left < l_clip_rect.Left )
    in_left = l_clip_rect.Left;

  if( in_right > l_clip_rect.Right )
    in_right = l_clip_rect.Right;

  if( in_top < l_clip_rect.Top )
    in_top = l_clip_rect.Top;

  if( in_bottom > l_clip_rect.Bottom )
    in_bottom = l_clip_rect.Bottom;

  if( in_right < l_clip_rect.Left || in_left > l_clip_rect.Right ||
      in_top > l_clip_rect.Bottom || in_bottom < l_clip_rect.Top ||
      in_right < in_left || in_bottom < in_top )
    return;

	drvGraphicsFillAreaWithBrush(in_left, in_top, in_right, in_bottom);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Draws text at the given coordinate with the selected font
/// @param in_x X coordinate of the text (position depends on the alignment)
/// @param in_y Y coordinate of the text (position depends on the alignment)
/// @param in_string Text to write
void guiDrawText(guiCoordinate in_x, guiCoordinate in_y, sysString in_string)
{
	sysResourceAddress chardata;
	int16_t i;
	sysStringLength len;
	guiCoordinate width;
	guiSize size;
	guiCoordinate offset_x, offset_y;
	guiCoordinate destination_width, destination_height;

	// check font
	if( g_gui_current_font.AsciiTableAddress == 0 )
		return;

	// check align
	if( g_gui_text_align != 0 )
	{
		// get text extent
		size = guiGetTextExtent( in_string );

		// horizontal align
		switch( g_gui_text_align & guiHA_MASK )
		{
			// left align
			case guiHA_LEFT:
				break;

			// center align
			case guiHA_CENTER:
				in_x -= size.Width / 2;
				break;

			// right align
			case guiHA_RIGHT:
				in_x -= size.Width;
				break;
		}

		// vertical align
		switch( g_gui_text_align & guiVA_MASK )
		{
			// top align
			case guiVA_TOP:
				break;

			// center align
			case guiVA_CENTER:
				in_y -= size.Height / 2;
				break;

			// bottom align
			case guiVA_BOTTOM:
				in_y -= size.Height;
				break;
		}
	}

	// draw characters
	len = strGetLength( in_string );
	for( i = 0; i < len; i++ )
	{
		if( in_string[i] >= g_gui_current_font.Minascii && in_string[i] <= g_gui_current_font.Maxascii )
		{
			chardata = drvResourceReadWord( (in_string[i] - g_gui_current_font.Minascii) * sizeof( uint16_t ) + g_gui_current_font.AsciiTableAddress ) + g_gui_current_font.FontAddress;
			width = drvResourceReadByte( chardata++ );

			if(in_x <= l_clip_rect.Right && in_y <= l_clip_rect.Bottom &&
				in_x + width >= l_clip_rect.Left && in_y + g_gui_current_font.Height >= l_clip_rect.Top)
			{
				offset_x = 0;
				offset_y = 0;

				destination_width = l_clip_rect.Right - in_x + 1;
				destination_height = l_clip_rect.Bottom - in_y + 1;

				if(destination_width > width)
					destination_width = width;

				if(destination_height > g_gui_current_font.Height)
					destination_height = g_gui_current_font.Height;

				if(in_x < l_clip_rect.Left)
				{
					offset_x = l_clip_rect.Left - in_x;
					destination_width -= offset_x;
				} 

				if(in_y < l_clip_rect.Top)
				{
					offset_y = l_clip_rect.Top - in_y;
					destination_height -= offset_y;
				}

				drvGraphicsBitBltFromResource(in_x + offset_x, in_y + offset_y, destination_width, destination_height, offset_x, offset_y, width, g_gui_current_font.Height, chardata, 1);
			}

			in_x += width;
		}
	}
}


///////////////////////////////////////////////////////////////////////////////
// Draw resource text
void guiDrawResourceText(guiCoordinate in_x, guiCoordinate in_y, sysResourceAddress in_string_handle)
{
	sysConstString string;
	guiSize size;
	sysResourceAddress chardata;
	int16_t i;
	sysStringLength len;
	guiCoordinate width;
	guiCoordinate offset_x, offset_y;
	guiCoordinate destination_width, destination_height;

	// check handle
	if (in_string_handle == drvRESOURCE_INVALID_ADDRESS)
		return;

	// check font
	if (g_gui_current_font.AsciiTableAddress == 0)
		return;

	// check align
	if (g_gui_text_align != 0)
	{
		// get text extent
		size = guiGetResourceTextExtent(in_string_handle);

		// horizontal align
		switch (g_gui_text_align & guiHA_MASK)
		{
			// left align
			case guiHA_LEFT:
				break;

				// center align
			case guiHA_CENTER:
				in_x -= size.Width / 2;
				break;

				// right align
			case guiHA_RIGHT:
				in_x -= size.Width;
				break;
		}

		// vertical align
		switch (g_gui_text_align & guiVA_MASK)
		{
			// top align
			case guiVA_TOP:
				break;

				// center align
			case guiVA_CENTER:
				in_y -= size.Height / 2;
				break;

				// bottom align
			case guiVA_BOTTOM:
				in_y -= size.Height;
				break;
		}
	}

	// load string
	string = drvResourceGetString(in_string_handle);
	len = drvResourceGetStringLength(in_string_handle);

	// draw characters
	for (i = 0; i < len; i++)
	{
		if (string[i] >= g_gui_current_font.Minascii && string[i] <= g_gui_current_font.Maxascii)
		{
			chardata = drvResourceReadWord((string[i] - g_gui_current_font.Minascii) * sizeof(uint16_t) + g_gui_current_font.AsciiTableAddress) + g_gui_current_font.FontAddress;
			width = drvResourceReadByte(chardata++);

			if (in_x <= l_clip_rect.Right && in_y <= l_clip_rect.Bottom &&
				in_x + width >= l_clip_rect.Left && in_y + g_gui_current_font.Height >= l_clip_rect.Top)
			{
				offset_x = 0;
				offset_y = 0;

				destination_width = l_clip_rect.Right - in_x + 1;
				destination_height = l_clip_rect.Bottom - in_y + 1;

				if (destination_width > width)
					destination_width = width;

				if (destination_height > g_gui_current_font.Height)
					destination_height = g_gui_current_font.Height;

				if (in_x < l_clip_rect.Left)
				{
					offset_x = l_clip_rect.Left - in_x;
					destination_width -= offset_x;
				}

				if (in_y < l_clip_rect.Top)
				{
					offset_y = l_clip_rect.Top - in_y;
					destination_height -= offset_y;
				}

				drvGraphicsBitBltFromResource(in_x + offset_x, in_y + offset_y, destination_width, destination_height, offset_x, offset_y, width, g_gui_current_font.Height, chardata, 1);
			}

			in_x += width;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// Draw bitmap
void guiDrawBitmap( guiCoordinate in_x, guiCoordinate in_y, sysResourceAddress in_bitmap_address )
{
	sysResourceAddress resource_address;
	guiCoordinate width;
	guiCoordinate height;
	uint8_t bpp;

	// get the bitmap address
	resource_address = /*l_bitmap_resource_address + */in_bitmap_address;

	// get bitmap data
	width = drvResourceReadWord( resource_address );
	resource_address += sizeof( uint16_t );

	height = drvResourceReadWord( resource_address );
	resource_address += sizeof( uint16_t );
 
	bpp = drvResourceReadByte( resource_address );
	resource_address += sizeof( uint8_t );

	drvGraphicsBitBltFromResource(in_x, in_y, width, height, 0, 0, width, height, resource_address, bpp);
}
