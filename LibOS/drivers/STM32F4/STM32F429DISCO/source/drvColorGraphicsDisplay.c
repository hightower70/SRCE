/*****************************************************************************/
/* ILI9341 LCD controller driver for VSYNC/HSYNC interface                   */
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
#include <drvLCDSPI.h>
#include <drvILI9341.h>

/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/
static bool l_initialized = false;

/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Initializes ILI9341 LCD driver chip
void drvGraphicsDisplayInitialize(void)
{
	if (!l_initialized)
	{
		// Initialize ILI9341 SPI bus
		drvLCDSPIinitialize();

		// Configure LCD
		drvLCDSPIWriteReg(0xCA);
		drvLCDSPIWriteData(0xC3);
		drvLCDSPIWriteData(0x08);
		drvLCDSPIWriteData(0x50);
		drvLCDSPIWriteReg(drvILI9341_POWERB);
		drvLCDSPIWriteData(0x00);
		drvLCDSPIWriteData(0xC1);
		drvLCDSPIWriteData(0x30);
		drvLCDSPIWriteReg(drvILI9341_POWER_SEQ);
		drvLCDSPIWriteData(0x64);
		drvLCDSPIWriteData(0x03);
		drvLCDSPIWriteData(0x12);
		drvLCDSPIWriteData(0x81);
		drvLCDSPIWriteReg(drvILI9341_DTCA);
		drvLCDSPIWriteData(0x85);
		drvLCDSPIWriteData(0x00);
		drvLCDSPIWriteData(0x78);
		drvLCDSPIWriteReg(drvILI9341_POWERA);
		drvLCDSPIWriteData(0x39);
		drvLCDSPIWriteData(0x2C);
		drvLCDSPIWriteData(0x00);
		drvLCDSPIWriteData(0x34);
		drvLCDSPIWriteData(0x02);
		drvLCDSPIWriteReg(drvILI9341_PRC);
		drvLCDSPIWriteData(0x20);
		drvLCDSPIWriteReg(drvILI9341_DTCB);
		drvLCDSPIWriteData(0x00);
		drvLCDSPIWriteData(0x00);
		drvLCDSPIWriteReg(drvILI9341_FRMCTR1);
		drvLCDSPIWriteData(0x00);
		drvLCDSPIWriteData(0x1B);
		drvLCDSPIWriteReg(drvILI9341_DFC);
		drvLCDSPIWriteData(0x0A);
		drvLCDSPIWriteData(0xA2);
		drvLCDSPIWriteReg(drvILI9341_POWER1);
		drvLCDSPIWriteData(0x10);
		drvLCDSPIWriteReg(drvILI9341_POWER2);
		drvLCDSPIWriteData(0x10);
		drvLCDSPIWriteReg(drvILI9341_VCOM1);
		drvLCDSPIWriteData(0x45);
		drvLCDSPIWriteData(0x15);
		drvLCDSPIWriteReg(drvILI9341_VCOM2);
		drvLCDSPIWriteData(0x90);
		drvLCDSPIWriteReg(drvILI9341_MAC);
		drvLCDSPIWriteData(0xC8);
		drvLCDSPIWriteReg(drvILI9341_3GAMMA_EN);
		drvLCDSPIWriteData(0x00);
		drvLCDSPIWriteReg(drvILI9341_RGB_INTERFACE);
		drvLCDSPIWriteData(0xC2);
		drvLCDSPIWriteReg(drvILI9341_DFC);
		drvLCDSPIWriteData(0x0A);
		drvLCDSPIWriteData(0xA7);
		drvLCDSPIWriteData(0x27);
		drvLCDSPIWriteData(0x04);

		// Column address set
		drvLCDSPIWriteReg(drvILI9341_COLUMN_ADDR);
		drvLCDSPIWriteData(0x00);
		drvLCDSPIWriteData(0x00);
		drvLCDSPIWriteData(0x00);
		drvLCDSPIWriteData(0xEF);

		// Page Address Set
		drvLCDSPIWriteReg(drvILI9341_PAGE_ADDR);
		drvLCDSPIWriteData(0x00);
		drvLCDSPIWriteData(0x00);
		drvLCDSPIWriteData(0x01);
		drvLCDSPIWriteData(0x3F);
		drvLCDSPIWriteReg(drvILI9341_INTERFACE);
		drvLCDSPIWriteData(0x01);
		drvLCDSPIWriteData(0x00);
		drvLCDSPIWriteData(0x06);

		drvLCDSPIWriteReg(drvILI9341_GRAM);
		drvLCDSPIDelay(200);

		drvLCDSPIWriteReg(drvILI9341_GAMMA);
		drvLCDSPIWriteData(0x01);

		drvLCDSPIWriteReg(drvILI9341_PGAMMA);
		drvLCDSPIWriteData(0x0F);
		drvLCDSPIWriteData(0x29);
		drvLCDSPIWriteData(0x24);
		drvLCDSPIWriteData(0x0C);
		drvLCDSPIWriteData(0x0E);
		drvLCDSPIWriteData(0x09);
		drvLCDSPIWriteData(0x4E);
		drvLCDSPIWriteData(0x78);
		drvLCDSPIWriteData(0x3C);
		drvLCDSPIWriteData(0x09);
		drvLCDSPIWriteData(0x13);
		drvLCDSPIWriteData(0x05);
		drvLCDSPIWriteData(0x17);
		drvLCDSPIWriteData(0x11);
		drvLCDSPIWriteData(0x00);
		drvLCDSPIWriteReg(drvILI9341_NGAMMA);
		drvLCDSPIWriteData(0x00);
		drvLCDSPIWriteData(0x16);
		drvLCDSPIWriteData(0x1B);
		drvLCDSPIWriteData(0x04);
		drvLCDSPIWriteData(0x11);
		drvLCDSPIWriteData(0x07);
		drvLCDSPIWriteData(0x31);
		drvLCDSPIWriteData(0x33);
		drvLCDSPIWriteData(0x42);
		drvLCDSPIWriteData(0x05);
		drvLCDSPIWriteData(0x0C);
		drvLCDSPIWriteData(0x0A);
		drvLCDSPIWriteData(0x28);
		drvLCDSPIWriteData(0x2F);
		drvLCDSPIWriteData(0x0F);

		drvLCDSPIWriteReg(drvILI9341_SLEEP_OUT);
		drvLCDSPIDelay(200);
		drvLCDSPIWriteReg(drvILI9341_DISPLAY_ON);

		// GRAM start writing
		drvLCDSPIWriteReg(drvILI9341_GRAM);

		l_initialized = true;
	}
}
