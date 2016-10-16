/*****************************************************************************/
/* Color graphics driver software rederer for frame buffer mode              */
/*                                                                           */
/* Copyright (C) 2014-2015 Laszlo Arvai                                      */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <guiTypes.h>
#include <drvResources.h>
#include <guiColorGraphics.h>

/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/
static guiDeviceColor l_background_color;
static bool l_transparent_background = false;
static guiDeviceColor l_foreground_color;

extern void*	g_gui_screen_pixels;
extern int		g_gui_screen_line_size;    // Size in bytes of a bitmap scanline

///////////////////////////////////////////////////////////////////////////////
/// @brief Initialize color graphics renderer module
void drvColorGraphicsRendererInitialize(void)
{

}

///////////////////////////////////////////////////////////////////////////////
/// @brief Sets foreground color for drawing operations
/// @param in_color Color for foreground
void guiSetForegroundColor(guiColor in_color)
{
	l_foreground_color = guiColorToDeviceColor(in_color);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Sets background color for drawing operations
/// @param in_color Color for background
void guiSetBackgroundColor(guiColor in_color)
{
	l_background_color = guiColorToDeviceColor(in_color);

	l_transparent_background = guiIS_TRANSPARENT_COLOR(in_color);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Gets the color of the given pixel
/// @param in_x X coordinate of the pixel
/// @param in_y Y coordinate of the pixel
/// @return Color of the pixel
guiColor guiGetPixelColor(guiCoordinate in_x, guiCoordinate in_y)
{
	uint8_t r, g, b;
#if guiCOLOR_DEPTH == 24
	uint8_t* pixel;
#elif guiCOLOR_DEPTH == 16
	guiDeviceColor pixel;
#endif

	if( in_x < 0 || in_x >= guiSCREEN_WIDTH || in_y < 0 || in_y >= guiSCREEN_HEIGHT )
		return 0;

#if guiCOLOR_DEPTH == 24
	pixel = (uint8_t*)g_gui_screen_pixels + in_y * g_gui_screen_line_size + in_x * (guiCOLOR_DEPTH / 8);
	r = *pixel++;
	g = *pixel++;
	b = *pixel;
#elif guiCOLOR_DEPTH == 16
	pixel = *(guiDeviceColor*)((uint8_t*)g_gui_screen_pixels + in_y * g_gui_screen_line_size + in_x * (guiCOLOR_DEPTH / 8));

	r = (pixel >> (6 + 5)) & 0x01F;
	g = (pixel >> 5) & 0x03F;
	b = (pixel) & 0x01F;

	//RGB888 - amplify
	r <<= 3;
	g <<= 2;
	b <<= 3;
#else
#error Invalid color depth
#endif
	
	return r | (g << 8) | (b << 16);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Sets pixel to the foreground color
/// @param in_x X coordinate of the pixel
/// @param in_y Y coordinate of the pixel
void guiSetForegroundPixel(guiCoordinate in_x, guiCoordinate in_y)
{
	uint8_t* pixel;
	
	if( in_x < 0 || in_x >= guiSCREEN_WIDTH || in_y < 0 || in_y >= guiSCREEN_HEIGHT)
		return;

	pixel = (uint8_t*)g_gui_screen_pixels  + in_y * g_gui_screen_line_size  + in_x * (guiCOLOR_DEPTH / 8);

#if guiCOLOR_DEPTH == 24
	*pixel++ = (uint8_t)(l_foreground_color );			// R
	*pixel++ = (uint8_t)(l_foreground_color >> 8);	// G
	*pixel   = (uint8_t)(l_foreground_color >> 16);	// B
#elif guiCOLOR_DEPTH == 16
	*(guiDeviceColor*)pixel = l_foreground_color;
#else
#error Invalid color depth
#endif
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Sets pixel to the background color
/// @param in_x X coordinate of the pixel
/// @param in_y Y coordinate of the pixel
void guiSetBackgroundPixel(guiCoordinate in_x, guiCoordinate in_y)
{
	uint8_t* pixel;
	
	if( l_transparent_background || in_x < 0 || in_x >= guiSCREEN_WIDTH || in_y < 0 || in_y >= guiSCREEN_HEIGHT)
		return;

	pixel = (uint8_t*)g_gui_screen_pixels  + in_y * g_gui_screen_line_size  + in_x * (guiCOLOR_DEPTH / 8);
#if guiCOLOR_DEPTH == 24
	*pixel++ = (uint8_t)(l_background_color );			// R
	*pixel++ = (uint8_t)(l_background_color >> 8);	// G
	*pixel   = (uint8_t)(l_background_color >> 16);	// B
#elif guiCOLOR_DEPTH == 16
	* (guiDeviceColor*)pixel = l_background_color;
#else
#error Invalid color depth
#endif
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Fill rectangle with foreground color
/// @param in_left Left-Top corner X coordinate
/// @param in_top Left-Top corner Y coordinate
/// @param in_width Width of the rectangle
/// @param in_height height of the rectangle
void drvColorGraphicsFillArea(guiCoordinate in_x1, guiCoordinate in_y1, guiCoordinate in_x2, guiCoordinate in_y2)
{
#if guiCOLOR_DEPTH == 24
	uint8_t* pixel;
	uint8_t red, green, blue;
#elif guiCOLOR_DEPTH == 16
	guiDeviceColor* pixel;
#else
#error Invalid color depth
#endif
	guiCoordinate x, y;

	// check coordinate
	if(in_x1 > guiSCREEN_WIDTH)
		in_x1 = guiSCREEN_WIDTH - 1;

	if(in_y1 > guiSCREEN_HEIGHT)
		in_y1 = guiSCREEN_HEIGHT- 1;

	// check size
	if(in_x2 >= guiSCREEN_WIDTH)
		in_x2 = guiSCREEN_WIDTH - 1;

	if(in_y2 >= guiSCREEN_HEIGHT)
		in_y2 = guiSCREEN_HEIGHT - 1;

#if guiCOLOR_DEPTH == 24
	red = (l_foreground_color & 0xff);				// R
	green = (l_foreground_color >> 8) & 0xff; // G
	blue = (l_foreground_color >> 16) & 0xff;	// B

	pixel = (uint8_t*)g_gui_screen_pixels;

	for(y = in_y1; y <= in_y2; y++)
	{
		pixel = (uint8_t*)g_gui_screen_pixels  + y * g_gui_screen_line_size  + in_x1 * (guiCOLOR_DEPTH / 8);
		for(x = in_x1; x <= in_x2; x++)
		{
			*pixel++ = red;
			*pixel++ = green;
			*pixel++ = blue;
		}
	}
#elif guiCOLOR_DEPTH == 16
	for (y = in_y1; y <= in_y2; y++)
	{
		pixel = (guiDeviceColor*)((uint8_t*)g_gui_screen_pixels + y * g_gui_screen_line_size + in_x1 * (guiCOLOR_DEPTH / 8));
		for (x = in_x1; x <= in_x2; x++)
		{
			*pixel++ = l_foreground_color;
		}
	}
#else
#error Invalid color depth
#endif
}


///////////////////////////////////////////////////////////////////////////////
/// @brief Draws a monochrome bitmap from the resource data
/// @param in_x Top-Left X coordinate of the bitmap target area
/// @param in_y Top-Left Y coordinate of the bitmap target area
/// @param in_width Width of the bitmap to display (can be smaller than the real width of the bitmap)
/// @param in_height Height of the bitmap to display (can be smaller than the real height of the bitmap)
/// @param in_resource_address Resource address of the bitmap to display
void drvColorGraphicsBitBltFromResource(guiCoordinate in_destination_x, guiCoordinate in_destination_y, 
																				guiCoordinate in_destination_width, guiCoordinate in_destination_height,
																				guiCoordinate in_source_x, guiCoordinate in_source_y,
																				guiCoordinate in_source_width, guiCoordinate in_source_height, 
																				sysResourceAddress in_source_bitmap, uint8_t in_source_bit_per_pixel)
{
	uint16_t row_byte_count;
	uint8_t bitmap_data;
	uint16_t bitmap_x;
	uint16_t bitmap_y;
	sysResourceAddress source_pixel;
#if guiCOLOR_DEPTH == 24
	uint8_t red, green, blue;
#endif
	uint8_t* destination_pixel;

	switch(in_source_bit_per_pixel)
	{
		case 1:
			row_byte_count = (in_source_width + 7) / 8;
			for(bitmap_y = 0; bitmap_y < in_destination_height; bitmap_y++)
			{
				source_pixel = in_source_bitmap + row_byte_count * (bitmap_y + in_source_y) + in_source_x / 8;
				bitmap_data = drvResourceReadByte(source_pixel);
				bitmap_data <<= (in_source_x % 8);

				for(bitmap_x = 0; bitmap_x < in_destination_width; bitmap_x++)
				{
					if(((bitmap_x + in_source_x) % 8) == 0)
						bitmap_data = drvResourceReadByte(source_pixel++);

					if((bitmap_data & 0x80) == 0)
					{
						guiSetBackgroundPixel(in_destination_x + bitmap_x, in_destination_y + bitmap_y);
					}
					else
					{
						guiSetForegroundPixel(in_destination_x +  bitmap_x, in_destination_y + bitmap_y);
					}

					bitmap_data <<= 1;
				}
			}
			break;

		case 16:
			// TODO: clipping
			row_byte_count = in_source_width * sizeof(uint16_t);
			for(bitmap_y = 0; bitmap_y < in_source_height; bitmap_y++)
			{
				source_pixel = in_source_bitmap + row_byte_count * bitmap_y;
				destination_pixel = (uint8_t*)g_gui_screen_pixels  + (bitmap_y + in_destination_y) * g_gui_screen_line_size  + in_destination_x * (guiCOLOR_DEPTH / 8);

#if guiCOLOR_DEPTH == 24
				for(bitmap_x = 0; bitmap_x < in_source_width; bitmap_x++)
				{
					// color low byte
					bitmap_data = drvResourceReadByte(source_pixel++);
					blue = (bitmap_data & 0x1f) << 3;
					green = (bitmap_data & 0xe0) >> 3;

					// color high byte
					bitmap_data = drvResourceReadByte(source_pixel++);
					green |= (bitmap_data & 0x07) << 5;
					red = bitmap_data & 0xf8;

					// store RGB pixel data
					*destination_pixel++ = blue;
					*destination_pixel++ = green;
					*destination_pixel++ = red;
				}
#elif guiCOLOR_DEPTH == 16
				for (bitmap_x = 0; bitmap_x < in_source_width; bitmap_x++)
				{
					*((guiDeviceColor*)destination_pixel) = drvResourceReadWord(source_pixel);
					source_pixel += 2;
					destination_pixel += 2;
				}
#else
#error Invalid color depth
#endif
			}
			break;
	}
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Draws a monochrome bitmap from the memory
/// @param in_x Top-Left X coordinate of the bitmap target area
/// @param in_y Top-Left Y coordinate of the bitmap target area
/// @param in_width Width of the bitmap to display (can be smaller than the real width of the bitmap)
/// @param in_height Height of the bitmap to display (can be smaller than the real height of the bitmap)
/// @param in_resource_address Resource address of the bitmap to display
void drvColorGraphicsBitBlt(guiCoordinate in_destination_x, guiCoordinate in_destination_y, 
														guiCoordinate in_destination_width, guiCoordinate in_destination_height,
														guiCoordinate in_source_x, guiCoordinate in_source_y,
														guiCoordinate in_source_width, guiCoordinate in_source_height, 
														void* in_source_bitmap, uint8_t in_source_bit_per_pixel)
{
	uint16_t row_byte_count;
	uint8_t bitmap_data;
	uint16_t bitmap_x;
	uint16_t bitmap_y;
	uint8_t* source_pixel;
#if guiCOLOR_DEPTH == 24
	uint8_t red, green, blue;
#endif
	uint8_t* destination_pixel;

	switch(in_source_bit_per_pixel)
	{
		case 1:
			row_byte_count = (in_source_width + 7) / 8;
			for(bitmap_y = 0; bitmap_y < in_destination_height; bitmap_y++)
			{
				source_pixel = (uint8_t*)in_source_bitmap + row_byte_count * (bitmap_y + in_source_y) + in_source_x / 8;
				bitmap_data = *source_pixel;
				bitmap_data <<= (in_source_x % 8);

				for(bitmap_x = 0; bitmap_x < in_destination_width; bitmap_x++)
				{
					if(((bitmap_x + in_source_x) % 8) == 0)
						bitmap_data = *source_pixel++;

					if((bitmap_data & 0x80) == 0)
					{
						guiSetBackgroundPixel(in_destination_x + bitmap_x, in_destination_y + bitmap_y);
					}
					else
					{
						guiSetForegroundPixel(in_destination_x +  bitmap_x, in_destination_y + bitmap_y);
					}

					bitmap_data <<= 1;
				}
			}
			break;

		case 16:
			row_byte_count = in_source_width * sizeof(uint16_t);
			for(bitmap_y = 0; bitmap_y < in_source_height; bitmap_y++)
			{
				source_pixel = (uint8_t*)in_source_bitmap + row_byte_count * bitmap_y;
				destination_pixel = (uint8_t*)g_gui_screen_pixels  + (bitmap_y + in_destination_y) * g_gui_screen_line_size  + in_destination_x * (guiCOLOR_DEPTH / 8);

#if guiCOLOR_DEPTH == 24
				for(bitmap_x = 0; bitmap_x < in_source_width; bitmap_x++)
				{
					// color low byte
					bitmap_data = *source_pixel++;
					blue = (bitmap_data & 0x1f) << 3;
					green = (bitmap_data & 0xe0) >> 3;

					// color high byte
					bitmap_data = *source_pixel++;
					green |= (bitmap_data &0x07) << 5;
					red = bitmap_data & 0xf8;

					// store RGB pixel data
					*destination_pixel++ = blue;
					*destination_pixel++ = green;
					*destination_pixel++ = red;
				}
#elif guiCOLOR_DEPTH == 16
				for (bitmap_x = 0; bitmap_x < in_source_width; bitmap_x++)
				{
					*((guiDeviceColor*)destination_pixel) = *((guiDeviceColor*)source_pixel);
					source_pixel += 2;
					destination_pixel += 2;
				}
#else
#error Invalid color depth
#endif
			}
			break;
	}
}
