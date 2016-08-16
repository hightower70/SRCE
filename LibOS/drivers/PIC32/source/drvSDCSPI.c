#include <sysTypes.h>
#include <plib.h>
#include <sysConfig.h>

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/

/*****************************************************************************/
/* Macros                                                                    */
/*****************************************************************************/

// SPI index generator
#define __SD_SPI(spi_index)       SPI_CHANNEL ## spi_index
#define _SD_SPI(spi_index) __SD_SPI(spi_index)
#define SD_SPI _SD_SPI(SD_SPI_CHANNEL)

// SPI interrupt vector generator
#define __SD_SPI_INT(spi_index)       _SPI_ ## spi_index ## _VECTOR
#define _SD_SPI_INT(spi_index) __SD_SPI_INT(spi_index)

#define __SD_SPI_INT_VECTOR(spi_index)       INT_SPI_ ## spi_index ## _VECTOR
#define _SD_SPI_INT_VECTOR(spi_index) __SD_SPI_INT_VECTOR(spi_index)
#define SD_SPI_INT_VECTOR _SD_SPI_INT_VECTOR(GLCD_SPI_CHANNEL)

#define drvSPI1A_CONF		0x8120	// SPI on, 8-bit master,CKE=1,CKP=0
#define drvSPI1A_BAUD		SpiBrgVal(sysPBCLK_FREQUENCY, 400000)

void drvSDCCSSPIInit(void)
{
	drvSDCCSInit();

  SpiChnEnable(drvSDC_SPI_CHANNEL, 0);
  SpiChnConfigure(drvSDC_SPI_CHANNEL, SPI_CONFIG_MSTEN | SPI_CONFIG_CKE_REV | SPI_CONFIG_MODE8);
  SpiChnSetBrg(drvSDC_SPI_CHANNEL, SpiBrgVal(sysPBCLK_FREQUENCY, 400000));

	SpiChnEnable(drvSDC_SPI_CHANNEL, 1);
}

void drvSDCCSSPICleanUp()
{
	SpiChnClose(drvSDC_SPI_CHANNEL);
	drvSDCCSDeinit();
}

void drvSDCSPISetBaudRate(uint32_t in_baud_rate)
{
  SpiChnSetBrg(drvSDC_SPI_CHANNEL, SpiBrgVal(sysPBCLK_FREQUENCY, in_baud_rate));
}

uint8_t drvSDCSPISendAndReceiveByte(uint8_t in_data)
{
	SpiChnPutC(drvSDC_SPI_CHANNEL, in_data);
	return SpiChnGetC(drvSDC_SPI_CHANNEL);
}

void drvSDCSPISendAsynchronByte(uint8_t in_data)
{
	SpiChnWriteC(drvSDC_SPI_CHANNEL, in_data);
}

uint8_t drvSDCSPIReceiveAsynchronByte()
{
	return SpiChnGetC(drvSDC_SPI_CHANNEL);
}
