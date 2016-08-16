/*****************************************************************************/
/* DAC audio driver for STM32F429-DISCO                                      */
/*  Using DAC Cannel 2 (PA5 for output)                                      */
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
#include <stm32f4xx_hal.h>
#include <stm32f4xx_hal_dma_ex.h>
#include <drvWavePlayer.h>
#include <drvHelpers.h>

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/
#if drvWAVEPLAYER_BUFFER_COUNT != 2
#error "Only double buffer is supported."
#endif

/*****************************************************************************/
/* Local function prototypes                                                 */
/*****************************************************************************/
void drvWaveBuffer1FreeCallback(DMA_HandleTypeDef *in_dma_handle);
void drvWaveBuffer2FreeCallback(DMA_HandleTypeDef *in_dma_handle);

/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/
static drvWavePlayerBufferType l_wave_buffer1[drvWAVEPLAYER_BUFFER_LENGTH];
static volatile drvWavePlayerBufferStatus l_wave_buffer1_status = drvWP_BS_Empty;
static drvWavePlayerBufferType l_wave_buffer2[drvWAVEPLAYER_BUFFER_LENGTH];
volatile drvWavePlayerBufferStatus l_wave_buffer2_status = drvWP_BS_Empty;

static DMA_HandleTypeDef l_dac_dma_handle;
static DAC_HandleTypeDef l_dac_handle;
static TIM_HandleTypeDef l_timer_handle;

/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Initializes double buffered wave player
void drvWavePlayerInitialize(void)
{
  DAC_ChannelConfTypeDef sConfig;
	GPIO_InitTypeDef GPIO_InitStruct;
  TIM_MasterConfigTypeDef sMasterConfig;

	// enable DAC clock
  __DAC_CLK_ENABLE();

  // DAC GPIO Configuration  PA5     ------> DAC_OUT2
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  // Peripheral DMA init
  l_dac_dma_handle.Instance = DMA1_Stream6;
  l_dac_dma_handle.Init.Channel = DMA_CHANNEL_7;
  l_dac_dma_handle.Init.Direction = DMA_MEMORY_TO_PERIPH;
  l_dac_dma_handle.Init.PeriphInc = DMA_PINC_DISABLE;
  l_dac_dma_handle.Init.MemInc = DMA_MINC_ENABLE;
  l_dac_dma_handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
  l_dac_dma_handle.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
  l_dac_dma_handle.Init.Mode = DMA_CIRCULAR;
  l_dac_dma_handle.Init.Priority = DMA_PRIORITY_VERY_HIGH;
  l_dac_dma_handle.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
  l_dac_dma_handle.XferCpltCallback = drvWaveBuffer1FreeCallback;
  l_dac_dma_handle.XferM1CpltCallback = drvWaveBuffer2FreeCallback;
  HAL_DMA_Init(&l_dac_dma_handle);

  __HAL_LINKDMA(&l_dac_handle,DMA_Handle2,l_dac_dma_handle);

   // DAC Initialization
  l_dac_handle.Instance = DAC;
  HAL_DAC_Init(&l_dac_handle);

  // DAC channel OUT2 config
  sConfig.DAC_Trigger = DAC_TRIGGER_T6_TRGO;
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
  HAL_DAC_ConfigChannel(&l_dac_handle, &sConfig, DAC_CHANNEL_2);

  // DMA interrupt init
  HAL_NVIC_SetPriority(DMA1_Stream6_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream6_IRQn);

  // setup timer
  __TIM6_CLK_ENABLE();

  l_timer_handle.Instance = TIM6;
  l_timer_handle.Init.Prescaler = 0;
  l_timer_handle.Init.CounterMode = TIM_COUNTERMODE_UP;
  l_timer_handle.Init.Period = drvTimerGetSourceFrequency(6) / drvWAVEPLAYER_SAMPLE_RATE - 1;
  HAL_TIM_Base_Init(&l_timer_handle);

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  HAL_TIMEx_MasterConfigSynchronization(&l_timer_handle, &sMasterConfig);

  // Start DMA
  HAL_DMAEx_MultiBufferStart_IT(&l_dac_dma_handle, (uint32_t)l_wave_buffer1, (uint32_t)&l_dac_handle.Instance->DHR12R2, (uint32_t)l_wave_buffer2, drvWAVEPLAYER_BUFFER_LENGTH);

  HAL_DAC_Start(&l_dac_handle, DAC_CHANNEL_2);
  l_dac_handle.Instance->CR |= DAC_CR_DMAEN2;
  HAL_TIM_Base_Start(&l_timer_handle);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Cleans up audio driver
void drvWavePlayerCleanUp(void)
{
  // Peripheral clock disable
  __DAC_CLK_DISABLE();

  // DAC GPIO Configuration  PA5     ------> DAC_OUT2
  HAL_GPIO_DeInit(GPIOA, GPIO_PIN_5);

  // Peripheral DMA DeInit
  HAL_DMA_DeInit(&l_dac_dma_handle);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Starts buffer playing
/// @param in_buffer_index Buffer index to start to play
void drvWavePlayerPlayBuffer(uint8_t in_buffer_index)
{
	switch(in_buffer_index)
	{
		case 0:
			l_wave_buffer1_status = drvWP_BS_Playing;
			break;

		case 1:
			l_wave_buffer2_status = drvWP_BS_Playing;
			break;
	}
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Gets next free buffer index
uint8_t drvWavePlayerGetFreeBufferIndex(void)
{
	if(l_wave_buffer1_status == drvWP_BS_Empty)
		return 0;
	else
	{
		if(l_wave_buffer2_status == drvWP_BS_Empty)
			return 1;
		else
			return drvWAVEPLAYER_INVALID_BUFFER_INDEX;
	}
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Gets pointer to the wave data section of the given buffer
/// @param in_buffer_index Index of the wave buffer
/// @return Wave data pointer or null if index is invalid
drvWavePlayerBufferType* drvWaveGetBuffer(uint8_t in_buffer_index)
{
	switch(in_buffer_index)
	{
		case 0:
			return l_wave_buffer1;

		case 1:
			return l_wave_buffer2;

		default:
			return sysNULL;
	}
}

/*****************************************************************************/
/* Module local function implementation                                      */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Callback when buffer 1 is empty
void drvWaveBuffer1FreeCallback(DMA_HandleTypeDef *in_dma_handle)
{
	sysUNUSED(in_dma_handle);

	l_wave_buffer1_status = drvWP_BS_Empty;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Callback when buffer 2 is empty
void drvWaveBuffer2FreeCallback(DMA_HandleTypeDef *in_dma_handle)
{
	sysUNUSED(in_dma_handle);

	l_wave_buffer2_status = drvWP_BS_Empty;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief This function handles DMA1 stream6 global interrupt.
void DMA1_Stream6_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&l_dac_dma_handle);
}
