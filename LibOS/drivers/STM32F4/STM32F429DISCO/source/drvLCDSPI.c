/*****************************************************************************/
/* Graphics LCD SPI interface functions for STM32F429 Discovery              */
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
#include <stm32f4xx_hal.h>
#include <drvHelpers.h>
#include "sysConfig.h"

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/
#define drvLCDSPI_TIMEOUT_MAX              ((uint32_t)0x1000)

/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/
static SPI_HandleTypeDef l_lcd_spi;

/*****************************************************************************/
/* Module local functions                                                    */
/*****************************************************************************/
static void drvLCDSPIErrorHandler(void);
static void drvSPILCDWrite(uint16_t in_value);

/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief SPI Bus initialization for graphics LCD
/// @note This function is not dealing with GPIO Clock. It must be enabled in the general init section.
void drvLCDSPIinitialize(void)
{
  GPIO_InitTypeDef   GPIO_InitStructure;

  if(HAL_SPI_GetState(&l_lcd_spi) == HAL_SPI_STATE_RESET)
  {
  	// Configure SPI GPIO pins
    GPIO_InitStructure.Pin				= (drvLCD_SPI_SCK_PIN | drvLCD_SPI_MOSI_PIN | drvLCD_SPI_MISO_PIN);
    GPIO_InitStructure.Mode  			= GPIO_MODE_AF_PP;
    GPIO_InitStructure.Pull  			= GPIO_PULLDOWN;
    GPIO_InitStructure.Speed  		= GPIO_SPEED_MEDIUM;
    GPIO_InitStructure.Alternate	= drvLCD_SPI_AF;
    HAL_GPIO_Init(drvLCD_SPI_PORT, &GPIO_InitStructure);

    // Configure NWR
    GPIO_InitStructure.Pin     = drvLCD_WRX_PIN;
    GPIO_InitStructure.Mode    = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pull    = GPIO_NOPULL;
    GPIO_InitStructure.Speed   = GPIO_SPEED_FAST;
    HAL_GPIO_Init(drvLCD_WRX_PORT, &GPIO_InitStructure);

    // Configure NRD
    GPIO_InitStructure.Pin     = drvLCD_RDX_PIN;
    GPIO_InitStructure.Mode    = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pull    = GPIO_NOPULL;
    GPIO_InitStructure.Speed   = GPIO_SPEED_FAST;
    HAL_GPIO_Init(drvLCD_RDX_PORT, &GPIO_InitStructure);

    // Configure NCS
    GPIO_InitStructure.Pin     = drvLCD_NCS_PIN;
    GPIO_InitStructure.Mode    = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pull    = GPIO_NOPULL;
    GPIO_InitStructure.Speed   = GPIO_SPEED_FAST;
    HAL_GPIO_Init(drvLCD_NCS_PORT, &GPIO_InitStructure);

    // Init pins
    drvSetPinHigh(drvLCD_NCS_PORT, drvLCD_NCS_PIN);
    drvSetPinHigh(drvLCD_WRX_PORT, drvLCD_WRX_PIN);
    drvSetPinHigh(drvLCD_RDX_PORT, drvLCD_RDX_PIN);

    // Enable SPI clock
  	drvLCD_SPI_GPIO_CLK_ENABLE();

  	// SPI Config
  	l_lcd_spi.Instance = drvLCD_SPI;
    /* SPI baudrate is set to 5.6 MHz (PCLK2/SPI_BaudRatePrescaler = 90/16 = 5.625 MHz)
       to verify these constraints:
       - ILI9341 LCD SPI interface max baudrate is 10MHz for write and 6.66MHz for read
       - l3gd20 SPI interface max baudrate is 10MHz for write/read
       - PCLK2 frequency is set to 90 MHz
    */
  	l_lcd_spi.Init.BaudRatePrescaler	= SPI_BAUDRATEPRESCALER_16;
  	l_lcd_spi.Init.Direction      		= SPI_DIRECTION_2LINES;
  	l_lcd_spi.Init.CLKPhase       		= SPI_PHASE_1EDGE;
  	l_lcd_spi.Init.CLKPolarity    		= SPI_POLARITY_LOW;
  	l_lcd_spi.Init.CRCCalculation 		=	SPI_CRCCALCULATION_DISABLED;
  	l_lcd_spi.Init.CRCPolynomial  		= 7;
  	l_lcd_spi.Init.DataSize       		= SPI_DATASIZE_8BIT;
  	l_lcd_spi.Init.FirstBit       		= SPI_FIRSTBIT_MSB;
  	l_lcd_spi.Init.NSS            		= SPI_NSS_SOFT;
  	l_lcd_spi.Init.TIMode         		= SPI_TIMODE_DISABLED;
  	l_lcd_spi.Init.Mode           		= SPI_MODE_MASTER;

    HAL_SPI_Init(&l_lcd_spi);
  }
}

///////////////////////////////////////////////////////////////////////////////
//@brief  Write LCD controller register value
void drvLCDSPIWriteData(uint16_t in_reg_value)
{
  // Set WRX to send data
	drvSetPinHigh(drvLCD_RDX_PORT, drvLCD_WRX_PIN);

  // Active /CS and Send data
	drvSetPinLow(drvLCD_NCS_PORT, drvLCD_NCS_PIN);
	drvSPILCDWrite(in_reg_value);

  // Deactivate /CS
	drvSetPinHigh(drvLCD_NCS_PORT, drvLCD_NCS_PIN);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Write LCD controller register address.
void drvLCDSPIWriteReg(uint8_t in_register)
{
  // Reset WRX to send data
	drvSetPinLow(drvLCD_RDX_PORT, drvLCD_WRX_PIN);

  // Active /CS and Send command
	drvSetPinLow(drvLCD_NCS_PORT, drvLCD_NCS_PIN);
	drvSPILCDWrite(in_register);

  // Deactivate /CS
	drvSetPinHigh(drvLCD_NCS_PORT, drvLCD_NCS_PIN);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief OS independent delay (busy wait)
void drvLCDSPIDelay(uint32_t in_delay)
{
	HAL_Delay(in_delay);
}

/*****************************************************************************/
/* Module local function implementation                                      */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief SPI Read 4 bytes from device
/// @param  in_size Number of bytes to read (max 4 bytes)
/// @retval Value read on the SPI
uint32_t drvLCDSPIRead(uint8_t in_size)
{
  HAL_StatusTypeDef status = HAL_OK;
  uint32_t read_value;

  status = HAL_SPI_Receive(&l_lcd_spi, (uint8_t*) &read_value, in_size, drvLCDSPI_TIMEOUT_MAX);

  /* Check the communication status */
  if(status != HAL_OK)
  {
    // Reset SPI bus
  	drvLCDSPIErrorHandler();
  }

  return read_value;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief SPI Write a byte to device
/// @param Value: value to be written
/// @retval None
static void drvSPILCDWrite(uint16_t in_value)
{
  HAL_StatusTypeDef status = HAL_OK;

  status = HAL_SPI_Transmit(&l_lcd_spi, (uint8_t*) &in_value, 1, drvLCDSPI_TIMEOUT_MAX);

  // Check the communication status
  if(status != HAL_OK)
  {
    // Reset SPI bus
  	drvLCDSPIErrorHandler();
  }
}

///////////////////////////////////////////////////////////////////////////////
/// @brief  Sends a Byte through the SPI interface and return the Byte received
///         from the SPI bus.
/// @param  in_byte_to_send Byte to send.
/// @retval The received byte value
uint8_t drvLCDSPIWriteRead(uint8_t in_byte_to_send)
{
  uint8_t received_byte = 0;

  // Send a Byte through the SPI peripheral and read byte from the SPI bus
  if(HAL_SPI_TransmitReceive(&l_lcd_spi, (uint8_t*)&in_byte_to_send, (uint8_t*) &received_byte, 1, drvLCDSPI_TIMEOUT_MAX) != HAL_OK)
  {
    // Reset SPI bus
  	drvLCDSPIErrorHandler();
  }

  return received_byte;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief SPI error handler function
static void drvLCDSPIErrorHandler(void)
{
  // release SPI bus
  HAL_SPI_DeInit(&l_lcd_spi);

  // Reinitialize SPI bus
  drvLCDSPIinitialize();
}


