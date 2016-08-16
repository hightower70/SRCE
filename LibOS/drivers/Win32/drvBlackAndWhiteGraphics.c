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
#include <Windows.h>
#include <guiTypes.h>
#include <drvBlackAndWhiteGraphics.h>
#include "sysConfig.h"

///////////////////////////////////////////////////////////////////////////////
// Module global variables
static BYTE l_bitmapheader[ sizeof( BITMAPINFOHEADER ) + 2 * sizeof( RGBQUAD ) ];
static HDC  l_hdc;               // Device context handle for bitmap

///////////////////////////////////////////////////////////////////////////////
// Local function prototypes

///////////////////////////////////////////////////////////////////////////////
// Global variables

///////////////////////////////////////////////////////////////////////////////
// Initialize color graphics system
void drvGraphicsInitialize(void)
{
		RGBQUAD* color;
		BITMAPINFOHEADER* bih;

		//guiInitDisplayEmulatorDriver();

		// Create memory device context
    l_hdc = CreateCompatibleDC( 0 );

    ZeroMemory( &l_bitmapheader, sizeof(l_bitmapheader) );

		bih = (BITMAPINFOHEADER*)l_bitmapheader;
		bih->biSize           = sizeof( BITMAPINFOHEADER );
		bih->biWidth          = guiSCREEN_WIDTH;
		bih->biHeight					= -guiSCREEN_HEIGHT;
		bih->biPlanes         = 1;
		bih->biBitCount       = 1;
		bih->biCompression    = BI_RGB;
		bih->biSizeImage      = guiSCREEN_WIDTH / 8 * guiSCREEN_HEIGHT;
		bih->biXPelsPerMeter  = 0;
		bih->biYPelsPerMeter  = 0;
		bih->biClrUsed        = 0;
		bih->biClrImportant    = 0;
		
		color = (RGBQUAD*)(l_bitmapheader + sizeof(BITMAPINFOHEADER));
		color->rgbRed    = GetRValue( guiemuBACKGROUND_COLOR );
		color->rgbGreen  = GetGValue( guiemuBACKGROUND_COLOR );
		color->rgbBlue  = GetBValue( guiemuBACKGROUND_COLOR );

		color++;
		color->rgbRed    = GetRValue( guiemuFOREGROUND_COLOR );
		color->rgbGreen  = GetGValue( guiemuFOREGROUND_COLOR );
		color->rgbBlue  = GetBValue( guiemuFOREGROUND_COLOR );
}

///////////////////////////////////////////////////////////////////////////////
// Releases resource at exit
void drvGraphicsCleanUp(void)
{
    DeleteDC( l_hdc );
}

///////////////////////////////////////////////////////////////////////////////
// Update emulation window
void drvGraphicsRefreshScreen(HDC in_hdc)
{
	StretchDIBits(	in_hdc,
									0, 0,
									guiSCREEN_WIDTH*guiemuZOOM, guiSCREEN_HEIGHT*guiemuZOOM,
									0, 0,
									guiSCREEN_WIDTH, guiSCREEN_HEIGHT,
									g_gui_frame_buffer,
									(BITMAPINFO*)l_bitmapheader,
									DIB_RGB_COLORS,
									SRCCOPY );
}





