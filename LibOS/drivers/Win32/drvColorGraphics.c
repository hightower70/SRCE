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
/* Types                                                                     */
/*****************************************************************************/
#pragma pack(1)
struct BITMAPINFOWITHMASK
{
	BITMAPINFOHEADER BitmapInfoHeader;
	DWORD RedMask;
	DWORD GreenMask;
	DWORD BlueMask;
};
#pragma pack()
/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/
static struct BITMAPINFOWITHMASK l_bitmap;
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
    ZeroMemory( &l_bitmap, sizeof(l_bitmap) );

#if guiCOLOR_DEPTH == 24
		l_bitmap.BitmapInfoHeader.biSize = sizeof(l_bitmap.BitmapInfoHeader);
		l_bitmap.BitmapInfoHeader.biCompression = BI_RGB;
#elif guiCOLOR_DEPTH == 16
		l_bitmap.BitmapInfoHeader.biSize = sizeof(l_bitmap);
		l_bitmap.BitmapInfoHeader.biCompression = BI_BITFIELDS;
#else
#error Unknown color depth
#endif

		l_bitmap.BitmapInfoHeader.biWidth = guiSCREEN_WIDTH;
		l_bitmap.BitmapInfoHeader.biHeight = -guiSCREEN_HEIGHT; // Create top-down bitmap
		l_bitmap.BitmapInfoHeader.biPlanes = 1;
		l_bitmap.BitmapInfoHeader.biBitCount = guiCOLOR_DEPTH;
		l_bitmap.BitmapInfoHeader.biSizeImage = 0;
		l_bitmap.BitmapInfoHeader.biXPelsPerMeter = 1;
		l_bitmap.BitmapInfoHeader.biYPelsPerMeter = 1;
		l_bitmap.BitmapInfoHeader.biClrUsed = 0;
		l_bitmap.BitmapInfoHeader.biClrImportant = 0;

		l_bitmap.RedMask = 0xF800;
		l_bitmap.GreenMask = 0x7E0;
		l_bitmap.BlueMask = 0x1F;
//		l_bitmap.RedMask = 0xFF0000;
//		l_bitmap.GreenMask = 0xFF00;
//		l_bitmap.BlueMask = 0xFF;

    l_hbm = CreateDIBSection( l_hdc,
      (BITMAPINFO*)&l_bitmap,
      DIB_RGB_COLORS,
      &pb,
      0,
      0 );

    // Associate the bitmap to the device context
    SelectObject( l_hdc, l_hbm );

    g_gui_screen_pixels = (LPBYTE)pb;
    g_gui_screen_line_size = 4 * ((guiSCREEN_WIDTH* (guiCOLOR_DEPTH / 8)) / 4); // Length of a scan line 
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
	(BITMAPINFO*)&l_bitmap,
  DIB_RGB_COLORS,
  SRCCOPY );
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Converts system color to device color
/// @param in_color System color to convert
/// @return Device color
guiDeviceColor guiColorToDeviceColor(guiColor in_color)
{
#if guiCOLOR_DEPTH == 24
	return in_color;
#elif guiCOLOR_DEPTH == 16
	uint8_t r, g, b;

	// RGB 565
	r = (uint8_t)((in_color >> 19) & 0x1f);
	g = (uint8_t)((in_color >> 10) & 0x3f);
	b = (uint8_t)((in_color >> 3) & 0x1f);

	return (r << 11) | (g << 5) | b;
#else
#error Invalid color depth
#endif
}
