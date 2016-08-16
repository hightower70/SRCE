/*****************************************************************************/
/* HT1080Z Emulator                                                          */
/*   Screen rendering functions                                              */
/*                                                                           */
/* Copyright (C) 2015 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
// Includes
#include <emuHT1080.h>
#include <drvBlackAndWhiteGraphics.h>
#include <guiBlackAndWhiteGraphics.h>
#include <fileUtils.h>
#include "sysResource.h"

///////////////////////////////////////////////////////////////////////////////
// Constants
#define emuHT1080_ROM_CHARACTER_HEIGHT 16
#define emuHT1080_GRAPHICS_MODE_FLAG 4

///////////////////////////////////////////////////////////////////////////////
// External references

// character ROM
extern const unsigned char ht_s1_chargen_rom[];

///////////////////////////////////////////////////////////////////////////////
// Global variables


///////////////////////////////////////////////////////////////////////////////
// Local variables
static bool l_statistics_displayed = false;

///////////////////////////////////////////////////////////////////////////////
/// @brief Opens whole screen for refresh 
void emuHT1080StartScreenRefresh(void)
{
	guiOpenCanvas(0, 0, guiSCREEN_WIDTH - 1, guiSCREEN_HEIGHT - 1);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Closes screen after refresh and displays the refreshed content
void emuHT1080EndScreenrefresh(void)
{
	guiCloseCanvas(true);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Renders one character at the given video memory address
/// @param in_video_memory_address Video memory address where the character is located for rendering
void emuHT1080RenderCharacter(uint16_t in_video_memory_address)
{
	uint8_t character_to_display = g_video_ram[in_video_memory_address];
	const uint8_t* character_memory_pointer;
	uint8_t character_row;
	uint8_t pixel_byte;
	uint16_t pixel_word;
	uint8_t column = in_video_memory_address % emuHT1080_SCREEN_WIDTH_IN_CHARACTER;
	uint8_t row = in_video_memory_address / emuHT1080_SCREEN_WIDTH_IN_CHARACTER;
	uint8_t nibble_position;
	uint8_t* frame_buffer_pointer = &g_gui_frame_buffer[row * guiFRAME_BUFFER_ROW_LENGTH * emuHT1080_CHARACTER_HEIGHT + column * emuHT1080_CHARACTER_WIDTH / 8];

	// protect no-refresh area
	if (g_screen_no_refresh_area[in_video_memory_address] > 0)
		return;

	// determine mode
	if ((character_to_display & 0x80) == 0)
	{
		character_memory_pointer = &ht_s1_chargen_rom[character_to_display * emuHT1080_ROM_CHARACTER_HEIGHT];
		nibble_position = column % 4;
	}
	else
	{
		nibble_position = column % 4 | emuHT1080_GRAPHICS_MODE_FLAG;
		character_to_display &= 0x3f;
	}

	// different action based on nibble position
	switch (nibble_position)
	{
		// position: 0, character mode
		case 0:
			for (character_row = 0; character_row < emuHT1080_CHARACTER_HEIGHT; character_row++)
			{
				*frame_buffer_pointer = *frame_buffer_pointer & 0x03 | *character_memory_pointer;
				character_memory_pointer++;
				frame_buffer_pointer += guiFRAME_BUFFER_ROW_LENGTH;
			}
			break;

		// position: 0, graphics mode
		case 0 | emuHT1080_GRAPHICS_MODE_FLAG:
			for (character_row = 0; character_row < emuHT1080_CHARACTER_HEIGHT; character_row++)
			{
				// prepare pixel data
				if ((character_row & 0x03) == 0)
				{
					pixel_byte = 0;
					if ((character_to_display & 0x01) != 0)
						pixel_byte |= 0xe0;

					if ((character_to_display & 0x02) != 0)
						pixel_byte |= 0x1c;

					character_to_display >>= 2;
				}

				// store pixel data
				*frame_buffer_pointer = (*frame_buffer_pointer & 0x03) | pixel_byte;
				frame_buffer_pointer += guiFRAME_BUFFER_ROW_LENGTH;
			}
			break;

		// position 1: character mode
		case 1:
			for (character_row = 0; character_row < emuHT1080_CHARACTER_HEIGHT; character_row++)
			{
				pixel_byte = *character_memory_pointer;
				pixel_word = (pixel_byte >> 6) + (((uint16_t)pixel_byte << 10) & 0xf000);
				*(uint16_t*)frame_buffer_pointer = (*(uint16_t*)frame_buffer_pointer & 0x0ffc) | pixel_word;
				character_memory_pointer++;
				frame_buffer_pointer += guiFRAME_BUFFER_ROW_LENGTH;
			}
			break;

		// position 1: graphics mode
		case 1 | emuHT1080_GRAPHICS_MODE_FLAG:
			for (character_row = 0; character_row < emuHT1080_CHARACTER_HEIGHT; character_row++)
			{
				// prepare pixel data
				if ((character_row & 0x03) == 0)
				{
					pixel_word = 0;
					if ((character_to_display & 0x01) != 0)
						pixel_word |= 0x8003;

					if ((character_to_display & 0x02) != 0)
						pixel_word |= 0x7000;

					character_to_display >>= 2;
				}

				// store pixel data
				*(uint16_t*)frame_buffer_pointer = (*(uint16_t*)frame_buffer_pointer & 0x0ffc) | pixel_word;
				frame_buffer_pointer += guiFRAME_BUFFER_ROW_LENGTH;
			}
			break;

		// position 2: character mode
		case 2:
			for (character_row = 0; character_row < emuHT1080_CHARACTER_HEIGHT; character_row++)
			{
				pixel_byte = *character_memory_pointer;
				pixel_word = (pixel_byte >> 4) + (((uint16_t)pixel_byte << 12) & 0xc000);
				*(uint16_t*)frame_buffer_pointer = (*(uint16_t*)frame_buffer_pointer & 0x3ff0) | pixel_word;
				character_memory_pointer++;
				frame_buffer_pointer += guiFRAME_BUFFER_ROW_LENGTH;
			}
			break;

		// position 2: graphics mode
		case 2 | emuHT1080_GRAPHICS_MODE_FLAG:
			for (character_row = 0; character_row < emuHT1080_CHARACTER_HEIGHT; character_row++)
			{
				// prepare pixel data
				if ((character_row & 0x03) == 0)
				{
					pixel_word = 0;
					if ((character_to_display & 0x01) != 0)
						pixel_word |= 0x000e;

					if ((character_to_display & 0x02) != 0)
						pixel_word |= 0xc001;

					character_to_display >>= 2;
				}

				// store pixel data
				*(uint16_t*)frame_buffer_pointer = (*(uint16_t*)frame_buffer_pointer & 0x3ff0) | pixel_word;
				frame_buffer_pointer += guiFRAME_BUFFER_ROW_LENGTH;
			}
			break;

		// position 3: character mode
		case 3:
			for (character_row = 0; character_row < emuHT1080_CHARACTER_HEIGHT; character_row++)
			{
				*frame_buffer_pointer = (*frame_buffer_pointer & 0xc0) | (*character_memory_pointer >> 2);
				character_memory_pointer++;
				frame_buffer_pointer += guiFRAME_BUFFER_ROW_LENGTH;
			}
			break;

			// position 2: graphics mode
		case 3 | emuHT1080_GRAPHICS_MODE_FLAG:
			for (character_row = 0; character_row < emuHT1080_CHARACTER_HEIGHT; character_row++)
			{
				// prepare pixel data
				if ((character_row & 0x03) == 0)
				{
					pixel_byte = 0;
					if ((character_to_display & 0x01) != 0)
						pixel_byte |= 0x38;

					if ((character_to_display & 0x02) != 0)
						pixel_byte |= 0x07;

					character_to_display >>= 2;
				}

				// store pixel data
				*frame_buffer_pointer = (*frame_buffer_pointer & 0xc0) | pixel_byte;
				frame_buffer_pointer += guiFRAME_BUFFER_ROW_LENGTH;
			}
			break;
	}
}


#if 0
void emuHT1080RenderScanLine(uint16_t in_line_index)
{
	int character_memory_row_index = in_line_index / emuHT1080_CHARACTER_HEIGHT;
	int character_row_index = in_line_index % emuHT1080_CHARACTER_HEIGHT;
	uint8_t* frame_buffer_pointer = &g_gui_frame_buffer[in_line_index * guiFRAME_BUFFER_ROW_LENGTH];
	uint8_t* video_memory_pointer = &g_video_ram[character_memory_row_index * emuHT1080_SCREEN_WIDTH_IN_CHARACTER];
	uint16_t pixel_buffer;
	uint8_t pixel_shift;
	int column_index;
	int pixel_byte;
	uint8_t character_to_display;

	if(in_line_index == 0)
		guiOpenCanvas(0, 0, guiSCREEN_WIDTH - 1, guiSCREEN_HEIGHT - 1);

	pixel_buffer = 0;
	pixel_shift = 8;
	for (column_index = 0; column_index < emuHT1080_SCREEN_WIDTH_IN_CHARACTER; column_index++)
	{
		// determine pixels to display
		character_to_display = *video_memory_pointer++;

		if ((character_to_display & 0x80) == 0)
		{
			pixel_byte = ht_s1_chargen_rom[character_to_display * emuHT1080_ROM_CHARACTER_HEIGHT + character_row_index];
		}
		else
		{
			// graphics display
			character_to_display = (character_to_display >> ((character_row_index / 4) * 2)) & 0x3;

			pixel_byte = 0;
			if ((character_to_display & 0x01) != 0)
				pixel_byte |= 0xe0;

			if ((character_to_display & 0x02) != 0)
				pixel_byte |= 0x1c;
		}
		
		// assemble frame buffer byte from pixel data
		pixel_buffer |= pixel_byte << pixel_shift;

		if (pixel_shift <= emuHT1080_CHARACTER_WIDTH)
		{
			*frame_buffer_pointer++ = (pixel_buffer >> 8) & 0xff;
			pixel_buffer <<= 8;
			pixel_shift = pixel_shift + 8 - emuHT1080_CHARACTER_WIDTH;
		}
		else
		{
			pixel_shift -= emuHT1080_CHARACTER_WIDTH;
		}
	}

	if (in_line_index >= emuHT1080_SCREEN_HEIGHT_IN_PIXEL - 1)
		guiCloseCanvas(true);
}

#endif
