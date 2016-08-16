/*****************************************************************************/
/* Configuration for STM32F429-DISCO                                         */
/*                                                                           */
/* Copyright (C) 2016 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

#ifndef __sysConfig_h
#define __sysConfig_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysTypes.h>

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
// Diagnostics
#define emuDIAG_DISPLAY_STATISTICS 1

///////////////////////////////////////////////////////////////////////////////
// SDRAM config
#define drvSDRAM_START_ADDRESS     ((void*)0xD0000000)
#define drvSDRAM_MEMORY_SIZE       (uint32_t)0x800000

///////////////////////////////////////////////////////////////////////////////
// Graphics config
#define guiSCREEN_WIDTH 240
#define guiSCREEN_HEIGHT 320

#define guiCOLOR_DEPTH 16

// Graphics display RAM
#define  guiLCD_FRAME_BUFFER   drvSDRAM_START_ADDRESS    // Start of the SDRAM

///////////////////////////////////////////////////////////////////////////////
// Resource config
typedef int sysResourceAddress;

///////////////////////////////////////////////////////////////////////////////
// Wave config
#define drvWAVEPLAYER_SAMPLE_RATE 44100
#define drvWAVEPLAYER_SAMPLE_OFFSET 1024
#define drvWAVEPLAYER_SAMPLE_MULTIPLIER 2

/*****************************************************************************/
/* Pin definitions                                                           */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
// LCD pin definitions
#define drvLCD_RDX_PIN							GPIO_PIN_12
#define drvLCD_RDX_PORT							GPIOD
#define drvLCD_WRX_PIN              GPIO_PIN_13
#define drvLCD_WRX_PORT          		GPIOD
#define drvLCD_NCS_PIN              GPIO_PIN_2
#define drvLCD_NCS_PORT          		GPIOC

#define drvLCD_SPI                  SPI5
#define drvLCD_SPI_GPIO_CLK_ENABLE()	__SPI5_CLK_ENABLE()
#define drvLCD_SPI_AF								GPIO_AF5_SPI5
#define drvLCD_SPI_PORT							GPIOF

#define drvLCD_SPI_SCK_PIN					GPIO_PIN_7				// SCK
#define drvLCD_SPI_MISO_PIN         GPIO_PIN_8				// MISO
#define drvLCD_SPI_MOSI_PIN         GPIO_PIN_9				// MOSI

///////////////////////////////////////////////////////////////////////////////
// LED pins
#define drvLED_LD3_PIN							GPIO_PIN_13
#define drvLED_LD3_PORT							GPIOG
#define drvLED_LD4_PIN							GPIO_PIN_14
#define drvLED_LD4_PORT							GPIOG

///////////////////////////////////////////////////////////////////////////////
// SDRAM pins
#define drvSDRAM_A0_PIN GPIO_PIN_0
#define drvSDRAM_A0_GPIO_PORT GPIOF
#define drvSDRAM_A1_PIN GPIO_PIN_1
#define drvSDRAM_A1_GPIO_PORT GPIOF
#define drvSDRAM_A2_PIN GPIO_PIN_2
#define drvSDRAM_A2_GPIO_PORT GPIOF
#define drvSDRAM_A3_PIN GPIO_PIN_3
#define drvSDRAM_A3_GPIO_PORT GPIOF
#define drvSDRAM_A4_PIN GPIO_PIN_4
#define drvSDRAM_A4_GPIO_PORT GPIOF
#define drvSDRAM_A5_PIN GPIO_PIN_5
#define drvSDRAM_A5_GPIO_PORT GPIOF
#define drvSDRAM_A6_PIN GPIO_PIN_12
#define drvSDRAM_A6_GPIO_PORT GPIOF
#define drvSDRAM_A7_PIN GPIO_PIN_13
#define drvSDRAM_A7_GPIO_PORT GPIOF
#define drvSDRAM_A8_PIN GPIO_PIN_14
#define drvSDRAM_A8_GPIO_PORT GPIOF
#define drvSDRAM_A9_PIN GPIO_PIN_15
#define drvSDRAM_A9_GPIO_PORT GPIOF
#define drvSDRAM_A10_PIN GPIO_PIN_0
#define drvSDRAM_A10_GPIO_PORT GPIOG
#define drvSDRAM_A11_PIN GPIO_PIN_1
#define drvSDRAM_A11_GPIO_PORT GPIOG

#define drvSDRAM_D0_PIN GPIO_PIN_14
#define drvSDRAM_D0_GPIO_PORT GPIOD
#define drvSDRAM_D1_PIN GPIO_PIN_15
#define drvSDRAM_D1_GPIO_PORT GPIOD
#define drvSDRAM_D2_PIN GPIO_PIN_0
#define drvSDRAM_D2_GPIO_PORT GPIOD
#define drvSDRAM_D3_PIN GPIO_PIN_1
#define drvSDRAM_D3_GPIO_PORT GPIOD
#define drvSDRAM_D4_PIN GPIO_PIN_7
#define drvSDRAM_D4_GPIO_PORT GPIOE
#define drvSDRAM_D5_PIN GPIO_PIN_8
#define drvSDRAM_D5_GPIO_PORT GPIOE
#define drvSDRAM_D6_PIN GPIO_PIN_9
#define drvSDRAM_D6_GPIO_PORT GPIOE
#define drvSDRAM_D7_PIN GPIO_PIN_10
#define drvSDRAM_D7_GPIO_PORT GPIOE
#define drvSDRAM_D8_PIN GPIO_PIN_11
#define drvSDRAM_D8_GPIO_PORT GPIOE
#define drvSDRAM_D9_PIN GPIO_PIN_12
#define drvSDRAM_D9_GPIO_PORT GPIOE
#define drvSDRAM_D10_PIN GPIO_PIN_13
#define drvSDRAM_D10_GPIO_PORT GPIOE
#define drvSDRAM_D11_PIN GPIO_PIN_14
#define drvSDRAM_D11_GPIO_PORT GPIOE
#define drvSDRAM_D12_PIN GPIO_PIN_15
#define drvSDRAM_D12_GPIO_PORT GPIOE
#define drvSDRAM_D13_PIN GPIO_PIN_8
#define drvSDRAM_D13_GPIO_PORT GPIOD
#define drvSDRAM_D14_PIN GPIO_PIN_9
#define drvSDRAM_D14_GPIO_PORT GPIOD
#define drvSDRAM_D15_PIN GPIO_PIN_10
#define drvSDRAM_D15_GPIO_PORT GPIOD

#define drvSDRAM_SDNRAS_PIN GPIO_PIN_11
#define drvSDRAM_SDNRAS_GPIO_PORT GPIOF

#define drvSDRAM_SDNWE_PIN GPIO_PIN_0
#define drvSDRAM_SDNWE_GPIO_PORT GPIOC

#define drvSDRAM_SDNCAS_PIN GPIO_PIN_15
#define drvSDRAM_SDNCAS_GPIO_PORT GPIOG

#define drvSDRAM_SDCLK_PIN GPIO_PIN_8
#define drvSDRAM_SDCLK_GPIO_PORT GPIOG

#define drvSDRAM_SDCKE1_PIN GPIO_PIN_5
#define drvSDRAM_SDCKE1_GPIO_PORT GPIOB

#define drvSDRAM_SDNE1_PIN GPIO_PIN_6
#define drvSDRAM_SDNE1_GPIO_PORT GPIOB

#define drvSDRAM_BA0_PIN GPIO_PIN_4
#define drvSDRAM_BA0_GPIO_PORT GPIOG
#define drvSDRAM_BA1_PIN GPIO_PIN_5
#define drvSDRAM_BA1_GPIO_PORT GPIOG

#define drvSDRAM_NBL0_PIN GPIO_PIN_0
#define drvSDRAM_NBL0_GPIO_PORT GPIOE
#define drvSDRAM_NBL1_PIN GPIO_PIN_1
#define drvSDRAM_NBL1_GPIO_PORT GPIOE

// other pin definitions
#if 0
#define PC14_OSC32_IN_PIN GPIO_PIN_14
#define PC14_OSC32_IN_GPIO_PORT GPIOC
#define PC15_OSC32_OUT_PIN GPIO_PIN_15
#define PC15_OSC32_OUT_GPIO_PORT GPIOC
#define ENABLE_PIN GPIO_PIN_10
#define ENABLE_GPIO_PORT GPIOF
#define PH0_OSC_IN_PIN GPIO_PIN_0
#define PH0_OSC_IN_GPIO_PORT GPIOH
#define PH1_OSC_OUT_PIN GPIO_PIN_1
#define PH1_OSC_OUT_GPIO_PORT GPIOH
#define NCS_MEMS_SPI_PIN GPIO_PIN_1
#define NCS_MEMS_SPI_GPIO_PORT GPIOC
#define CSX_PIN GPIO_PIN_2
#define CSX_GPIO_PORT GPIOC
#define B1_PIN GPIO_PIN_0
#define B1_GPIO_PORT GPIOA
#define MEMS_INT1_PIN GPIO_PIN_1
#define MEMS_INT1_GPIO_PORT GPIOA
#define MEMS_INT2_PIN GPIO_PIN_2
#define MEMS_INT2_GPIO_PORT GPIOA
#define B5_PIN GPIO_PIN_3
#define B5_GPIO_PORT GPIOA
#define VSYNC_PIN GPIO_PIN_4
#define VSYNC_GPIO_PORT GPIOA
#define G2_PIN GPIO_PIN_6
#define G2_GPIO_PORT GPIOA
#define ACP_RST_PIN GPIO_PIN_7
#define ACP_RST_GPIO_PORT GPIOA
#define OTG_FS_PSO_PIN GPIO_PIN_4
#define OTG_FS_PSO_GPIO_PORT GPIOC
#define OTG_FS_OC_PIN GPIO_PIN_5
#define OTG_FS_OC_GPIO_PORT GPIOC
#define R3_PIN GPIO_PIN_0
#define R3_GPIO_PORT GPIOB
#define R6_PIN GPIO_PIN_1
#define R6_GPIO_PORT GPIOB
#define BOOT1_PIN GPIO_PIN_2
#define BOOT1_GPIO_PORT GPIOB
#define G4_PIN GPIO_PIN_10
#define G4_GPIO_PORT GPIOB
#define G5_PIN GPIO_PIN_11
#define G5_GPIO_PORT GPIOB
#define OTG_FS_ID_PIN GPIO_PIN_12
#define OTG_FS_ID_GPIO_PORT GPIOB
#define VBUS_FS_PIN GPIO_PIN_13
#define VBUS_FS_GPIO_PORT GPIOB
#define OTG_FS_DM_PIN GPIO_PIN_14
#define OTG_FS_DM_GPIO_PORT GPIOB
#define OTG_FS_DP_PIN GPIO_PIN_15
#define OTG_FS_DP_GPIO_PORT GPIOB
#define TE_PIN GPIO_PIN_11
#define TE_GPIO_PORT GPIOD
#define RDX_PIN GPIO_PIN_12
#define RDX_GPIO_PORT GPIOD
#define WRX_DCX_PIN GPIO_PIN_13
#define WRX_DCX_GPIO_PORT GPIOD
#define R7_PIN GPIO_PIN_6
#define R7_GPIO_PORT GPIOG
#define DOTCLK_PIN GPIO_PIN_7
#define DOTCLK_GPIO_PORT GPIOG
#define HSYNC_PIN GPIO_PIN_6
#define HSYNC_GPIO_PORT GPIOC
#define G6_PIN GPIO_PIN_7
#define G6_GPIO_PORT GPIOC
#define R4_PIN GPIO_PIN_11
#define R4_GPIO_PORT GPIOA
#define R5_PIN GPIO_PIN_12
#define R5_GPIO_PORT GPIOA
#define SWDIO_PIN GPIO_PIN_13
#define SWDIO_GPIO_PORT GPIOA
#define SWCLK_PIN GPIO_PIN_14
#define SWCLK_GPIO_PORT GPIOA
#define TP_INT1_PIN GPIO_PIN_15
#define TP_INT1_GPIO_PORT GPIOA
#define R2_PIN GPIO_PIN_10
#define R2_GPIO_PORT GPIOC
#define G7_PIN GPIO_PIN_3
#define G7_GPIO_PORT GPIOD
#define B2_PIN GPIO_PIN_6
#define B2_GPIO_PORT GPIOD
#define G3_PIN GPIO_PIN_10
#define G3_GPIO_PORT GPIOG
#define B3_PIN GPIO_PIN_11
#define B3_GPIO_PORT GPIOG
#define B4_PIN GPIO_PIN_12
#define B4_GPIO_PORT GPIOG
#define B6_PIN GPIO_PIN_8
#define B6_GPIO_PORT GPIOB
#define B7_PIN GPIO_PIN_9
#define B7_GPIO_PORT GPIOB
#endif

#endif
