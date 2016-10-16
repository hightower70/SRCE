/*****************************************************************************/
/* Color graphicsdriver for Linux framebuffer                                */
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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
# include <string.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <linux/kd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <guiTypes.h>
#include <guiColorGraphics.h>
#include "sysConfig.h"

/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/


/*****************************************************************************/
/* Global variables                                                          */
/*****************************************************************************/
void*	g_gui_screen_pixels;       // Pointer to the (device independent) bitmap data
int   g_gui_screen_line_size;    // Size in bytes of a bitmap scanline

/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/
static struct fb_var_screeninfo l_orig_var_info;
static int l_fbfd = 0; // framebuffer filedescriptor
static int l_kbfd = 0; // keyboard file descriptior
static long int l_gui_screen_memory_size = 0; // size of the framebuffer memory


/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Initialize color graphics system
void drvColorGraphicsInitialize(void)
{
	struct fb_var_screeninfo var_info;
	struct fb_fix_screeninfo fix_info;

  // Open the framebuffer device file for reading and writing
	l_fbfd  = open("/dev/fb0", O_RDWR);
	if (!l_fbfd) 
	{
		printf("Error: cannot open framebuffer device.\n");
		exit(1);
	}
	
  // Get original variable screen information
	if (ioctl(l_fbfd, FBIOGET_VSCREENINFO, &var_info)) 
	{
		printf("Error reading variable screen info.\n");
		exit(1);
	}

	// hide cursor
	ioctl(STDIN_FILENO, KDSETMODE, KD_GRAPHICS);
	//puts("\033[?25l");

	// Store for resetting before exit
	memcpy(&l_orig_var_info, &var_info, sizeof(struct fb_var_screeninfo));

	// Set variable info
	var_info.xres = guiSCREEN_WIDTH;
	var_info.yres = guiSCREEN_HEIGHT;
	var_info.xres_virtual = var_info.xres;
	var_info.yres_virtual = var_info.yres;
	var_info.bits_per_pixel = guiCOLOR_DEPTH;
	var_info.xoffset = 0;
	var_info.yoffset = 0;
	if (ioctl(l_fbfd, FBIOPUT_VSCREENINFO, &var_info)) 
	{
		printf("Error setting variable screen info.\n");
		exit(1);
	}

	// Get fixed screen information
	if (ioctl(l_fbfd, FBIOGET_FSCREENINFO, &fix_info)) 
	{
		printf("Error reading fixed screen info.\n");
		exit(1);
	}
	
	
	/*
	// hide cursor
	char *kbfds = "/dev/tty";
	l_kbfd = open(kbfds, O_WRONLY);
	if (l_kbfd >= 0) 
	{
		ioctl(l_kbfd, KDSETMODE, KD_GRAPHICS);
	}
	else 
	{
		printf("Could not open %s.\n", kbfds);
		exit(1);
	}
*/
	//ioctl(0, KDSETMODE, KD_GRAPHICS);

	//fputs("\e[?25l", stdout); /* hide the cursor */


	// map fb to user mem 
	l_gui_screen_memory_size = fix_info.smem_len;
	g_gui_screen_pixels = (char*)mmap(0, l_gui_screen_memory_size, PROT_READ | PROT_WRITE, MAP_SHARED, l_fbfd, 0);
	if ((int)g_gui_screen_pixels == -1)
	{
		printf("Failed to mmap.\n");
		exit(1);
	}
	
	g_gui_screen_line_size = var_info.xres * var_info.bits_per_pixel / 8;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Initialize color graphics display
void drvGraphicsDisplayInitialize(void)
{
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Refreshes screen content
void drvColorGraphicsRefreshScreen(void)
{

}

///////////////////////////////////////////////////////////////////////////////
/// @brief Cleans-up color graphics system
void drvColorGraphicsCleanup(void)
{
  // unmap fb file from memory
	munmap(g_gui_screen_pixels, l_gui_screen_memory_size);

	
	// reset the display mode
	if (ioctl(l_fbfd, FBIOPUT_VSCREENINFO, &l_orig_var_info)) 
	{
		printf("Error re-setting variable screen info.\n");
	}

	// reset cursor
	ioctl(STDIN_FILENO, KDSETMODE, KD_TEXT);

	// close fb file  
	close(l_fbfd);
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
