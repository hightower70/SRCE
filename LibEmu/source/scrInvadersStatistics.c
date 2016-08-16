/*****************************************************************************/
/* Taito Invaders Emulator Statistics Display Renderer                       */
/*                                                                           */
/* Copyright (C) 2016 Laszlo Arvai                                           */
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
#include <sysString.h>

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/
#define STRING_BUFFER_LENGTH 64

/*****************************************************************************/
/* Function implementations                                                  */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Renders emulator statistics
void emuDisplayStatistics(uint32_t in_cpu_clock, uint16_t in_frame_rate, uint16_t in_load)
{
	guiSize text_size;

	sysChar buffer[STRING_BUFFER_LENGTH];
	sysStringLength pos;

	guiSetForegroundColor(guiCOLOR_WHITE);
	guiSetBackgroundColor(guiCOLOR_BLACK);

	guiSetTextAlign(guiHA_LEFT | guiVA_TOP);
	guiSetFont(REF_FNA_DEFAULT);

	pos = strCopyConstString(buffer, STRING_BUFFER_LENGTH, 0, (sysConstString)"CPU:");
	pos = strWordToStringPos(buffer, STRING_BUFFER_LENGTH, pos, in_cpu_clock, 5, 3, TS_RIGHT_ADJUSTMENT);
	pos = strCopyConstString(buffer, STRING_BUFFER_LENGTH, pos, (sysConstString)"MHz@");
	pos = strWordToStringPos(buffer, STRING_BUFFER_LENGTH, pos, in_frame_rate, 4, 1, TS_RIGHT_ADJUSTMENT);
	pos = strCopyConstString(buffer, STRING_BUFFER_LENGTH, pos, (sysConstString)"Hz Load:");
	pos = strWordToStringPos(buffer, STRING_BUFFER_LENGTH, pos, in_load, 4, 1, TS_RIGHT_ADJUSTMENT);
	pos = strCopyConstString(buffer, STRING_BUFFER_LENGTH, pos, (sysConstString)"%");

	text_size = guiGetTextExtent(buffer);

	guiDrawText(0, guiSCREEN_HEIGHT - text_size.Height, buffer);
}
