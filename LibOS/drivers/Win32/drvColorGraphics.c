/*****************************************************************************/
/* Color graphics function for device dependent bitmap                       */
/*                                                                           */
/* Copyright (C) 2014 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <Windows.h>
#include <guiTypes.h>
#include <guiColorGraphics.h>
#include "sysConfig.h"

/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/
static BITMAPINFO  l_bitmapinfo;
static int         l_bitmapwidth;       // Width of bitmap in pixels
static int         l_bitmapheight;      // Height of bitmap in pixels
static HANDLE      l_hbm = NULL;        // Bitmap handle
static HDC         l_hdc = NULL;				// Device context handle


/*****************************************************************************/
/* Global variables                                                          */
/*****************************************************************************/
void*	g_gui_screen_pixels;       // Pointer to the (device independent) bitmap data
int   g_gui_screen_line_size;    // Size in bytes of a bitmap scanline

/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Initialize color graphics system
void drvColorGraphicsInitialize(void)
{
    LPVOID  pb;

		// Create memory device context
    l_hdc = CreateCompatibleDC( 0 );

    // Create DIB section to hold bitmap data
    ZeroMemory( &l_bitmapinfo, sizeof(l_bitmapinfo) );
    l_bitmapinfo.bmiHeader.biSize = sizeof(l_bitmapinfo.bmiHeader);
    l_bitmapinfo.bmiHeader.biWidth = guiSCREEN_WIDTH;
    l_bitmapinfo.bmiHeader.biHeight = -guiSCREEN_HEIGHT; // Create top-down bitmap
    l_bitmapinfo.bmiHeader.biPlanes = 1;
    l_bitmapinfo.bmiHeader.biBitCount = 24;
    l_bitmapinfo.bmiHeader.biCompression = BI_RGB;
    l_bitmapinfo.bmiHeader.biSizeImage = 0;
    l_bitmapinfo.bmiHeader.biXPelsPerMeter = 1;
    l_bitmapinfo.bmiHeader.biYPelsPerMeter = 1;
    l_bitmapinfo.bmiHeader.biClrUsed = 0;
    l_bitmapinfo.bmiHeader.biClrImportant = 0;

    l_hbm = CreateDIBSection( l_hdc,
      &l_bitmapinfo,
      DIB_RGB_COLORS,
      &pb,
      0,
      0 );

    // Associate the bitmap to the device context
    SelectObject( l_hdc, l_hbm );

    g_gui_screen_pixels = (LPBYTE)pb;
    g_gui_screen_line_size = 4 * ((guiSCREEN_WIDTH*3 + 3) / 4); // Length of a scan line 
    l_bitmapwidth = guiSCREEN_WIDTH;
    l_bitmapheight = guiSCREEN_HEIGHT;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Cleans-up color graphics system
void drvColorGraphicsCleanup(void)
{
	if (l_hdc != NULL)
	{
		DeleteDC(l_hdc);
		l_hdc = NULL;
	}

	if (l_hbm != NULL)
	{
		DeleteObject(l_hbm);
		l_hbm = NULL;
	}
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Refreshes total screen content
/// @param in_hdc Handle of the window to draw the content
void drvGraphicsRefreshScreen(HDC in_hdc)
{
	StretchDIBits(  in_hdc,
  0, 0,
  guiSCREEN_WIDTH*guiemuZOOM, guiSCREEN_HEIGHT*guiemuZOOM,
  0, 0,
  guiSCREEN_WIDTH, guiSCREEN_HEIGHT,
	g_gui_screen_pixels,
	&l_bitmapinfo,
  DIB_RGB_COLORS,
  SRCCOPY );
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Converts system color to device color
/// @param in_color System color to convert
/// @return Device color
guiDeviceColor guiColorToDeviceColor(guiColor in_color)
{
	return (guiDeviceColor)(((in_color & 0xf80000u) >> 8) | ((in_color & 0x00fc00u) >> 5) | ((in_color & 0x0000f8u) >> 3));
}
