/*****************************************************************************/
/* HomeLab III Emulator                                                      */
/*   Screen rendering functions (black and white)                            */
/*                                                                           */
/* Copyright (C) 2015 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
// Includes
#include <emuHomelab.h>
#include <drvBlackAndWhiteGraphics.h>
#include <guiBlackAndWhiteGraphics.h>

///////////////////////////////////////////////////////////////////////////////
// Constants
#define emuHomelab_ROM_CHARACTER_COUNT 256

// character ROM
extern const unsigned char HL3_ch_EPD[];

void emuHomelabStartScreenRefresh(void)
{
	guiOpenCanvas(0, 0, guiSCREEN_WIDTH - 1, guiSCREEN_HEIGHT - 1);
}

void emuHomelabRenderCharacter(uint16_t in_video_memory_address)
{
	uint8_t character_to_display = g_video_ram[in_video_memory_address];
	uint16_t frame_buffer_address = in_video_memory_address / emuHomelab_SCREEN_WIDTH_IN_CHARACTER * emuHomelab_CHARACTER_HEIGHT * guiFRAME_BUFFER_ROW_LENGTH + in_video_memory_address % emuHomelab_SCREEN_WIDTH_IN_CHARACTER;
	uint8_t* frame_buffer_pointer = &g_gui_frame_buffer[frame_buffer_address];
	const uint8_t* character_memory_pointer = &HL3_ch_EPD[character_to_display];
	uint8_t character_row;

	for (character_row = 0; character_row < emuHomelab_CHARACTER_HEIGHT; character_row++)
	{
		*frame_buffer_pointer = *character_memory_pointer;

		frame_buffer_pointer += guiFRAME_BUFFER_ROW_LENGTH;
		character_memory_pointer += emuHomelab_ROM_CHARACTER_COUNT;
	}
}

void emuHomelabEndScreenrefresh(void)
{
	guiCloseCanvas(true);
}

#if 0
void emuHomelabRenderScanLine(uint16_t in_line_index)
{
	int character_memory_row_index = in_line_index / emuHomelab_CHARACTER_HEIGHT;
	int character_row_index = in_line_index % emuHomelab_CHARACTER_HEIGHT;
	uint8_t* frame_buffer_pointer = &g_gui_frame_buffer[in_line_index * guiFRAME_BUFFER_ROW_LENGTH];
	uint8_t* video_memory_pointer = &g_video_ram[character_memory_row_index * emuHomelab_SCREEN_WIDTH_IN_CHARACTER];
	uint16_t pixel_buffer;
	uint8_t pixel_shift;
	int column_index;
	int pixel_byte;
	uint8_t character_to_display;

	if (in_line_index == 0)
		guiOpenCanvas(0, 0, guiSCREEN_WIDTH - 1, guiSCREEN_HEIGHT - 1);

	pixel_buffer = 0;
	pixel_shift = 8;
	for (column_index = 0; column_index < emuHomelab_SCREEN_WIDTH_IN_CHARACTER; column_index++)
	{
		// determine pixels to display
		character_to_display = *video_memory_pointer++;

		pixel_byte = HL3_ch_EPD[character_to_display + character_row_index * emuHomelab_ROM_CHARACTER_COUNT];

		*frame_buffer_pointer++ = pixel_byte;
	}

	if (in_line_index >= emuHomelab_SCREEN_HEIGHT_IN_PIXEL - 1)
		guiCloseCanvas(true);
}
#endif