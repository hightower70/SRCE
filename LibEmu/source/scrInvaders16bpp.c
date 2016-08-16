/*****************************************************************************/
/* Taito Invaders Emulator Scanline Renderer function                        */
/*  (using BitBlt)                                                           */
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
#include <sysString.h>

/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/
static uint16_t l_line_buffer[emuINVADERS_SCREEN_HEIGHT];

/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Renders scanline into line buffer and uses BitBlt to display it
void emuInvadersRenderScanLine(uint16_t in_line_index)
{
	emuInvadersRender16bppScanLine( &l_line_buffer[emuINVADERS_SCREEN_HEIGHT-1], -1, in_line_index);

	guiBitblt(in_line_index + emuINVADERS_SCREEN_LEFT, emuINVADERS_SCREEN_TOP, 1, emuINVADERS_SCREEN_HEIGHT, 0, 0, 1, emuINVADERS_SCREEN_HEIGHT, l_line_buffer, 16);
}

