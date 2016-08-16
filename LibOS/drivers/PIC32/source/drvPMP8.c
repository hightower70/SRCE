/*****************************************************************************/
/* Parallel Master port (8-bit) driver                                       */
/*                                                                           */
/* Copyright (C) 2014-2015 Laszlo Arvai                                      */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

#include <sysTypes.h>
#include "sysConfig.h"

/*****************************************************************************/
/* Macros
/*****************************************************************************/

// DMA vector macro
#define __PMP8_DMA_INT_VECTOR(dma_index)       _DMA ## dma_index ## _VECTOR
#define _PMP8_DMA_INT_VECTOR(dma_index) __PMP8_DMA_INT_VECTOR(dma_index)
#define PMP8_DMA_INT_VECTOR _PMP8_DMA_INT_VECTOR(drvPMP8_DMA_CHANNEL)


/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/
#ifdef drvPMP8_DMA_CHANNEL
static volatile bool l_dma_busy = false;				// transfer done flag
static volatile uint8_t* l_dma_source_address;	// current DMA source buffer pointer
static volatile uint32_t l_dma_transfer_size;		// total DMA transfer size
#endif

///////////////////////////////////////////////////////////////////////////////
/// @brief Initializes PMP driver
void drvPMP8Init(void)
{
	mPMPOpen(drvPMP8_CONTROL,
           drvPMP8_MODE,
					 PMP_PEN_0,
					 PMP_INT_OFF);

#ifdef drvPMP8_DMA_CHANNEL
	// Open DMA channel
	DmaChnOpen(drvPMP8_DMA_CHANNEL, 0, DMA_OPEN_DEFAULT);
  DmaChnSetEventControl(drvPMP8_DMA_CHANNEL, DMA_EV_START_IRQ(_PMP_IRQ));

	// set the DMA events to generate an interrupt, the interrupt priority and sub-priority. Enable the DMA interrupt.
	DmaChnSetEvEnableFlags(drvPMP8_DMA_CHANNEL, DMA_EV_BLOCK_DONE);
	DmaChnSetIntPriority(drvPMP8_DMA_CHANNEL, 5, 3);
	DmaChnIntEnable(drvPMP8_DMA_CHANNEL);
#endif

}

///////////////////////////////////////////////////////////////////////////////
/// @brief Waits while PMP is busy
void drvPMP8WaitBusy(void)
{
	while(mIsPMPBusy() || l_dma_busy)
	{
	}
}

///////////////////////////////////////////////////////////////////////////////
/// @brief
/// @param in_address Address of PMP bus
/// @parma in_data Data to write
void drvPMP8WriteByte(uint8_t in_address, uint8_t in_data)
{
	PMPSetAddress(in_address);
	PMPMasterWrite(in_data);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Repeates writting two bytes until block length byte was transferred
/// @param in_first_byte First byte to write
/// @param in_second byte Second byte to transfer
/// @param in_block_length Number of byte to write (wil be rounded up to even number of odd is specified)
void drvPMP8WriteDoubleByteBlock(uint8_t in_address, uint8_t in_first_byte, uint8_t in_second_byte, uint32_t in_block_length)
{
	PMPSetAddress(in_address);

	while(in_block_length > 0)
	{
		PMPMasterWrite(in_first_byte);
		PMPMasterWrite(in_second_byte);

		in_block_length--;
	}

	while(mIsPMPBusy());
}


void drvPMP8WriteBlock(uint8_t in_address, uint8_t* in_block_address, uint32_t in_block_length)
{
#ifdef drvPMP8_DMA_CHANNEL

	// address
	PMPSetAddress(in_address);

	// init the variables that the ISR needs
	l_dma_source_address = in_block_address;
	l_dma_transfer_size = in_block_length;
	l_dma_busy = true;

	// start DMA transfer
	DmaChnSetTxfer(drvPMP8_DMA_CHANNEL, (void*)l_dma_source_address, (void *) &PMDIN, l_dma_transfer_size > DmaGetMaxTxferSize() ? DmaGetMaxTxferSize() : l_dma_transfer_size, 1, 1);
  DmaChnEnable(drvPMP8_DMA_CHANNEL);
	DmaChnStartTxfer(drvPMP8_DMA_CHANNEL, DMA_WAIT_BLOCK, 0);


#else
	PMPSetAddress(in_address);

	while(in_block_length > 0)
	{
		while(mIsPMPBusy());
		PMPMasterWrite(*in_block_address);

		in_block_address++;
		in_block_length--;
	}
	while(mIsPMPBusy());
#endif
}

#ifdef drvPMP8_DMA_CHANNEL
void __ISR(PMP8_DMA_INT_VECTOR, ipl5) PMP8DmaIntHandler(void)
{
	uint32_t max_transfer_size = DmaGetMaxTxferSize();

	// clear event and interupt flag
	DmaChnClrEvFlags(drvPMP8_DMA_CHANNEL, DMA_EV_BLOCK_DONE);
	DmaChnClrIntFlag(drvPMP8_DMA_CHANNEL);

	// another block is done
	if(l_dma_transfer_size <= max_transfer_size)
	{	// we're done, last block was transferred
		l_dma_busy = false;
	}
	else
	{	// we'll have to start a new transfer
		l_dma_transfer_size -= max_transfer_size;
		l_dma_source_address += max_transfer_size;

		DmaChnSetTxfer(drvPMP8_DMA_CHANNEL, (void*)l_dma_source_address, (void*)&PMDIN, l_dma_transfer_size > max_transfer_size ? max_transfer_size:l_dma_transfer_size, 1, 1);

		DmaChnEnable(drvPMP8_DMA_CHANNEL);
		//DmaChnForceTxfer(drvPMP8_DMA_CHANNEL);
		DmaChnStartTxfer(drvPMP8_DMA_CHANNEL, DMA_WAIT_BLOCK, 0);
	}


}
#endif

