/*****************************************************************************/
/* Taito Invaders Emulator Pixel Renderer function                           */
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
#include <emuInvaders.h>
#include <guiColorGraphics.h>
#include <emuInvadersResource.h>
#include <guiColors.h>

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/
#define emuINVADERS_PIXEL_COUNT 8


#define SET_PIXEL(address, data, mask, color, background_pointer)  if((data & mask)!=0) { *address = color; }	else {  *address = drvResourceReadWord(background_pointer); }

/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/
static uint16_t l_pixel_buffer[emuINVADERS_PIXEL_COUNT];

// cached colors
static uint16_t l_white_pixel;
static uint16_t l_green_pixel;
static uint16_t l_red_pixel;

// cached bitmap data
static sysResourceAddress l_background_data;
static guiSize l_background_size;

/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/

// Space Invaders Screen Color Overlay
//             < 224 >
// -------------------------------
//|WHITE            ^             |
//|                32             |
//|                 v             |
//|-------------------------------|
//|RED              ^             |
//|                32             |
//|                 v             |
//|-------------------------------|
//|WHITE                          |
//|         < 224 >               |
//|                               |
//|                 ^             |  ^
//|                120            | 256
//|                 v             |  v
//|                               |
//|                               |
//|                               |
//|-------------------------------|
//|GREEN                          |
//| ^                  ^          |
//|56        ^        56          |
//| v       72         v          |
//|____      v      ______________|
//|  ^  |          | ^            |
//|<16> |  < 118 > |16   < 90 >   |
//|  v  |          | v            |
//|WHITE|          |         WHITE|
// -------------------------------

///////////////////////////////////////////////////////////////////////////////
/// @brief Initializes invaders renderer
void emuInvadersRendererInitialize(void)
{
	// cache pixel color
	l_white_pixel = guiColorToRGB565(guiCOLOR_WHITE);
	l_green_pixel = guiColorToRGB565(guiCOLOR_LIME);
	l_red_pixel = guiColorToRGB565(guiCOLOR_RED);

	// cache bitmap data
	l_background_data = guiGetBitmapData(REF_BMP_BACKGROUND);
	l_background_size = guiGetBitmapSize(REF_BMP_BACKGROUND);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Renders one byte (eight pixels) from the video memory using RGB565 format
void emuInvadersRenderPixels(uint16_t in_memory_address, uint8_t in_data)
{
	guiCoordinate x, y;
	uint16_t* pixel_buffer_address;
	uint16_t pixel_color;
	sysResourceAddress background_data;

	x = in_memory_address / (emuINVADERS_SCREEN_HEIGHT / 8);
	y = emuINVADERS_SCREEN_HEIGHT - in_memory_address % (emuINVADERS_SCREEN_HEIGHT / 8) * 8 - 8;

	background_data = l_background_data + l_background_size.Width * sizeof(uint16_t) * (emuINVADERS_SCREEN_TOP + y) + (emuINVADERS_SCREEN_LEFT + x) * sizeof(uint16_t);

	// pixel color
	if(y < 32)
	{
		pixel_color = l_white_pixel;
	}
	else
	{
		if(y < 64)
		{
			pixel_color = l_red_pixel;
		}
		else
		{
			if(y < 184)
			{
				pixel_color = l_white_pixel;
			}
			else
			{
				if(y < 240)
				{
					pixel_color = l_green_pixel;
				}
				else
				{
					if(x < 16 || x > 134)
					{
						pixel_color = l_white_pixel;
					}
					else
					{
						pixel_color = l_green_pixel;
					}
				}
			}
		}
	}

	// process one byte (8 pixel)
	pixel_buffer_address = &l_pixel_buffer[0];

	SET_PIXEL(pixel_buffer_address, in_data, 0x80, pixel_color, background_data);
	pixel_buffer_address++;
	background_data += l_background_size.Width * sizeof(uint16_t);

	SET_PIXEL(pixel_buffer_address, in_data, 0x40, pixel_color, background_data);
	pixel_buffer_address++;
	background_data += l_background_size.Width * sizeof(uint16_t);

	SET_PIXEL(pixel_buffer_address, in_data, 0x20, pixel_color, background_data);
	pixel_buffer_address++;
	background_data += l_background_size.Width * sizeof(uint16_t);
	
	SET_PIXEL(pixel_buffer_address, in_data, 0x10, pixel_color, background_data);
	pixel_buffer_address++;
	background_data += l_background_size.Width * sizeof(uint16_t);
	
	SET_PIXEL(pixel_buffer_address, in_data, 0x08, pixel_color, background_data);
	pixel_buffer_address++;
	background_data += l_background_size.Width * sizeof(uint16_t);
	
	SET_PIXEL(pixel_buffer_address, in_data, 0x04, pixel_color, background_data);
	pixel_buffer_address++;
	background_data += l_background_size.Width * sizeof(uint16_t);
	
	SET_PIXEL(pixel_buffer_address, in_data, 0x02, pixel_color, background_data);
	pixel_buffer_address++;
	background_data += l_background_size.Width * sizeof(uint16_t);
	
	SET_PIXEL(pixel_buffer_address, in_data, 0x01, pixel_color, background_data);

	guiBitblt(x + emuINVADERS_SCREEN_LEFT, y + emuINVADERS_SCREEN_TOP, 1, emuINVADERS_PIXEL_COUNT, 0, 0, 1, emuINVADERS_PIXEL_COUNT, l_pixel_buffer, 16);
}


///////////////////////////////////////////////////////////////////////////////
/// @brief Renders scanline into line buffer and uses BitBlt to display it
void emuInvadersRenderScanLine(uint16_t in_line_index)
{
	// do nothing
}


#if 0
/*****************************************************************************/
/* Taito Invaders Emulator Scanline Renderer function                        */
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
#include <emuInvaders.h>
#include <guiColorGraphics.h>
#include <emuInvadersResource.h>
#include <guiColors.h>

static uint16_t l_line_buffer[emuINVADERS_SCREEN_HEIGHT];

// Space Invaders Screen Color Overlay
//             < 224 >
// -------------------------------
//|WHITE            ^             |
//|                32             |
//|                 v             |
//|-------------------------------|
//|RED              ^             |
//|                32             |
//|                 v             |
//|-------------------------------|
//|WHITE                          |
//|         < 224 >               |
//|                               |
//|                 ^             |  ^
//|                120            | 256
//|                 v             |  v
//|                               |
//|                               |
//|                               |
//|-------------------------------|
//|GREEN                          |
//| ^                  ^          |
//|56        ^        56          |
//| v       72         v          |
//|____      v      ______________|
//|  ^  |          | ^            |
//|<16> |  < 118 > |16   < 90 >   |
//|  v  |          | v            |
//|WHITE|          |         WHITE|
// -------------------------------

#define SET_PIXEL(address, data, mask, color, background_pointer)  if((data & mask)!=0) { *address = color; }	else { *address = drvResourceReadWord(background_pointer); }

void emuInvadersRenderScanLine(uint8_t* in_destination_buffer, int8_t in_destination_buffer_indecrement, uint16_t in_line_index)
{
	uint16_t pixel_color;
	int16_t invaders_row_index;
	uint8_t screen_data;
	uint8_t* invaders_video_mem;
	uint16_t* line_buffer_address;
	guiDeviceColor white_pixel;
	guiDeviceColor green_pixel;
	guiDeviceColor red_pixel;

	// cache pixel color
	white_pixel = guiColorToDeviceColor(guiCOLOR_WHITE);
	green_pixel = guiColorToDeviceColor(guiCOLOR_GREEN);
	red_pixel = guiColorToDeviceColor(guiCOLOR_RED);

	// init pointers
  invaders_video_mem = &g_cpu_ram[emuINVADERS_VIDEO_RAM_START - emuINVADERS_RAM_START + in_line_index * emuINVADERS_SCREEN_HEIGHT / 8];
	line_buffer_address = &l_line_buffer[emuINVADERS_SCREEN_HEIGHT-1];

	background_data += background_size.Width * sizeof(uint16_t) * (emuINVADERS_SCREEN_TOP + emuINVADERS_SCREEN_HEIGHT-1) + (emuINVADERS_SCREEN_LEFT + in_line_index) * sizeof(uint16_t);

	for(invaders_row_index = emuINVADERS_SCREEN_HEIGHT - 1; invaders_row_index >= 0; invaders_row_index -= 8)
	{
		// get screen data
		screen_data = *invaders_video_mem++;

		// pixel color
		if(invaders_row_index < 32)
		{
			pixel_color = white_pixel;
		}
		else
		{
			if(invaders_row_index < 64)
			{
				pixel_color = red_pixel;
			}
			else
			{
				if(invaders_row_index < 184)
				{
					pixel_color = white_pixel;
				}
				else
				{
					if(invaders_row_index < 240)
					{
						pixel_color = green_pixel;
					}
					else
					{
						if(in_line_index < 16 || in_line_index > 134)
						{
							pixel_color = white_pixel;
						}
						else
						{
							pixel_color = green_pixel;
						}
					}
				}
			}
		}

		// process one byte (8 pixel)
		SET_PIXEL(line_buffer_address, screen_data, 0x01, pixel_color, background_data);
		line_buffer_address--;
		background_data -= background_size.Width * sizeof(uint16_t);
		SET_PIXEL(line_buffer_address, screen_data, 0x02, pixel_color, background_data);
		line_buffer_address--;
		background_data -= background_size.Width * sizeof(uint16_t);
		SET_PIXEL(line_buffer_address, screen_data, 0x04, pixel_color, background_data);
		line_buffer_address--;
		background_data -= background_size.Width * sizeof(uint16_t);
		SET_PIXEL(line_buffer_address, screen_data, 0x08, pixel_color, background_data);
		line_buffer_address--;
		background_data -= background_size.Width * sizeof(uint16_t);
		SET_PIXEL(line_buffer_address, screen_data, 0x10, pixel_color, background_data);
		line_buffer_address--;
		background_data -= background_size.Width * sizeof(uint16_t);
		SET_PIXEL(line_buffer_address, screen_data, 0x20, pixel_color, background_data);
		line_buffer_address--;
		background_data -= background_size.Width * sizeof(uint16_t);
		SET_PIXEL(line_buffer_address, screen_data, 0x40, pixel_color, background_data);
		line_buffer_address--;
		background_data -= background_size.Width * sizeof(uint16_t);
		SET_PIXEL(line_buffer_address, screen_data, 0x80, pixel_color, background_data);
		line_buffer_address--;
		background_data -= background_size.Width * sizeof(uint16_t);
	}

	guiBitblt(in_line_index + emuINVADERS_SCREEN_LEFT, emuINVADERS_SCREEN_TOP, 1, emuINVADERS_SCREEN_HEIGHT, 0, 0, 1, emuINVADERS_SCREEN_HEIGHT, l_line_buffer, 16);
}


#endif
