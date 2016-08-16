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

/*****************************************************************************/
/* Macros                                                                    */
/*****************************************************************************/
#define SET_PIXEL(address, data, mask, color, background_pointer)  if((data & mask)!=0) { *address = color; }	else { *address = drvResourceReadWord(background_pointer); }

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


void emuInvadersRender16bppScanLine(uint8_t* in_destination_buffer, int8_t in_destination_buffer_indecrement, uint16_t in_line_index)
{
	uint16_t pixel_color;
	int16_t invaders_row_index;
	uint8_t screen_data;
	uint8_t* invaders_video_mem;
	uint16_t* line_buffer_address;
	sysResourceAddress background_data;
	guiSize background_size;
	guiDeviceColor white_pixel;
	guiDeviceColor green_pixel;
	guiDeviceColor red_pixel;

	// cache pixel color
	white_pixel = guiColorToDeviceColor(guiCOLOR_WHITE);
	green_pixel = guiColorToDeviceColor(guiCOLOR_LIME);
	red_pixel = guiColorToDeviceColor(guiCOLOR_RED);

	// init pointers
  invaders_video_mem = &g_cpu_ram[emuINVADERS_VIDEO_RAM_START - emuINVADERS_RAM_START + in_line_index * emuINVADERS_SCREEN_HEIGHT / 8];
	line_buffer_address = (uint16_t*)in_destination_buffer;
	background_data = guiGetBitmapData(REF_BMP_BACKGROUND);
	background_size = guiGetBitmapSize(REF_BMP_BACKGROUND);

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
		line_buffer_address += in_destination_buffer_indecrement;
		background_data -= background_size.Width * sizeof(uint16_t);
		SET_PIXEL(line_buffer_address, screen_data, 0x02, pixel_color, background_data);
		line_buffer_address += in_destination_buffer_indecrement;
		background_data -= background_size.Width * sizeof(uint16_t);
		SET_PIXEL(line_buffer_address, screen_data, 0x04, pixel_color, background_data);
		line_buffer_address += in_destination_buffer_indecrement;
		background_data -= background_size.Width * sizeof(uint16_t);
		SET_PIXEL(line_buffer_address, screen_data, 0x08, pixel_color, background_data);
		line_buffer_address += in_destination_buffer_indecrement;
		background_data -= background_size.Width * sizeof(uint16_t);
		SET_PIXEL(line_buffer_address, screen_data, 0x10, pixel_color, background_data);
		line_buffer_address += in_destination_buffer_indecrement;
		background_data -= background_size.Width * sizeof(uint16_t);
		SET_PIXEL(line_buffer_address, screen_data, 0x20, pixel_color, background_data);
		line_buffer_address += in_destination_buffer_indecrement;
		background_data -= background_size.Width * sizeof(uint16_t);
		SET_PIXEL(line_buffer_address, screen_data, 0x40, pixel_color, background_data);
		line_buffer_address += in_destination_buffer_indecrement;
		background_data -= background_size.Width * sizeof(uint16_t);
		SET_PIXEL(line_buffer_address, screen_data, 0x80, pixel_color, background_data);
		line_buffer_address += in_destination_buffer_indecrement;
		background_data -= background_size.Width * sizeof(uint16_t);
	}
}


