/*****************************************************************************/
/* Black and White (monochrome) graphics drive for PIC32                     */
/*                                                                           */
/* Copyright (C) 2015 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/
/*                                                                           */
/* Hardware used for video signal generator                                  */
/*                 330                                                       */
/* OC3  (sync)  --\/\/\/--                                                   */
/*                 180    |                                                  */
/* SDO1 (video) --\/\/\/-- -------o Video Out                                */
/*                        |                                                  */
/*                        \                                                  */
/*                        /                                                  */
/*                    120 \                                                  */
/*                        /                                                  */
/*                        |                                                  */
/*                        _                                                  */
/*                                                                           */
/* OC1 (screen) --                                                           */
/*                |                                                          */
/* SS1 -----------                                                           */
/*                                                                           */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
// Incudes
#include <sysTypes.h>
#include <plib.h>
#include "sysConfig.h"

///////////////////////////////////////////////////////////////////////////////
// Constants

//#define videoNTSC

#define ROUNDDIV(dividend,divisor) ((dividend+(divisor/2))/divisor)

#define videoHORIZONTAL_RESOLUTION guiSCREEN_WIDTH
#define videoVERTICAL_RESOLUTION guiSCREEN_HEIGHT
#define videoBORDER_WIDTH_IN_PIXEL 25

#define videoTIMER_FREQUENCY sysPBCLK_FREQUENCY // input clock frequency of the video timer
#define videoCLOCK_FREQUENCY sysPBCLK_FREQUENCY // input clock frequency of the video pixel clock generator


// Video timing constants
#ifdef videoNTSC

// Timing contants for NTSC video generation
#define videoTOTAL_NUMBER_OF_LINES 262  // number of lines in NTSC frame
#define videoHORIZONTAL_SYNC_FREQ 15734	// Tpb clock in a line (63.5us)

#define videoBACK_PORCH_WIDTH		4700	// minimum length of the back-porch signal in ns
#define videoFRONT_PORCH_WIDTH	1500	// minimum length og the pront-porch in ns
#define videoSYNC_WIDTH				4700	// sync signal width in ns
#else
// Timing constants for PAL video generation
#define videoTOTAL_NUMBER_OF_LINES 312	// number of lines in PAL frame
#define videoHORIZONTAL_SYNC_FREQ 15625 // horizontal sync pulse frequency in PAL system

#define videoBACK_PORCH_WIDTH		5700	// minimum length of the back-porch signal in ns
#define videoFRONT_PORCH_WIDTH	1650	// minimum length og the pront-porch in ns
#define videoSYNC_WIDTH					4700	// sync signal width in ns

#endif

// count the number of remaining black lines top+bottom
#define videoVSYNC_LINE_COUNT		3          // V sync lines
#define videoBLANK_LINE_COUNT		(videoTOTAL_NUMBER_OF_LINES-videoVERTICAL_RESOLUTION-videoVSYNC_LINE_COUNT)
#define videoPREEQ_LINE_COUNT		(videoBLANK_LINE_COUNT/4)         // preeq + bottom blank
#define videoPOSTEQ_LINE_COUNT	(videoBLANK_LINE_COUNT-videoPREEQ_LINE_COUNT)  // posteq + top blank

// Video State values
#define VS_PREEQ    0
#define VS_SYNC     1
#define VS_POSTEQ   2
#define VS_LINE     3

// range conversion constants
#define videoNANOSEC_DIVISOR 1000000000
#define videoMICROSEC_DIVISOR 1000000
#define videoNANOSEC_TO_MICROSEC_DIVISOR 1000

// useful video line with in ns
#define videoDISPLAYED_MAX_LINE_WIDTH ((videoNANOSEC_DIVISOR / videoHORIZONTAL_SYNC_FREQ) - videoBACK_PORCH_WIDTH - videoFRONT_PORCH_WIDTH - videoSYNC_WIDTH)
// Pixel clock
#define videoPIXEL_CLOCK_MIN (videoNANOSEC_DIVISOR / videoDISPLAYED_MAX_LINE_WIDTH * (videoHORIZONTAL_RESOLUTION + 2 * videoBORDER_WIDTH_IN_PIXEL))
// Pixel Clock BRG
#define videoPIXEL_CLOCK_BRG (videoCLOCK_FREQUENCY / (2 * videoPIXEL_CLOCK_MIN) - 1)
// Pixel Clock in Hz
#define videoPIXEL_CLOCK (videoCLOCK_FREQUENCY / (2 * (videoPIXEL_CLOCK_BRG+1)))
// Displayed effective pixel area
#define videoDISPLAYED_LINE_WIDTH (videoNANOSEC_DIVISOR/videoPIXEL_CLOCK * (videoHORIZONTAL_RESOLUTION + 2 * videoBORDER_WIDTH_IN_PIXEL))

// divisor for horizontal sync generator
#define videoHORIZONTAL_SYNC_DIVISOR (videoPIXEL_CLOCK / videoHORIZONTAL_SYNC_FREQ * (videoPIXEL_CLOCK_BRG+1) * 2 )
// timing for composite video horizontal state machine
#define videoSYNC_CYCLE (videoTIMER_FREQUENCY / videoNANOSEC_TO_MICROSEC_DIVISOR * videoSYNC_WIDTH / videoMICROSEC_DIVISOR) // calculation is done in mucrosec to avoid overflow
// timing cycle for back-porch
#define videoBACK_PORCH_CYCLE ROUNDDIV(videoTIMER_FREQUENCY / 1000 * videoBACK_PORCH_WIDTH, videoMICROSEC_DIVISOR)

// pixel (display) start position in ns (from the falling edge of the sync)
#define videoPIXEL_START ((videoDISPLAYED_MAX_LINE_WIDTH - videoDISPLAYED_LINE_WIDTH)/2 + videoSYNC_WIDTH + videoBACK_PORCH_WIDTH)
// pixel start position in eight pixel units
#define videoPIXEL_START_BYTES (videoPIXEL_CLOCK / videoNANOSEC_TO_MICROSEC_DIVISOR * videoPIXEL_START / videoMICROSEC_DIVISOR / 8)

// pixel stop position in ns
#define videoPIXEL_STOP (videoPIXEL_START + ((videoHORIZONTAL_RESOLUTION + 10)* videoPIXEL_CLOCK_DIVISOR * 1000 / (videoCLOCK_FREQUENCY / videoMICROSEC_DIVISOR)))

// pixel start position in video timer cycle
#define videoPIXEL_START_CYCLE (ROUNDDIV(videoTIMER_FREQUENCY / videoMICROSEC_DIVISOR * videoPIXEL_START, 1000))
// pixel stop position in timer cycle
#define videoPIXEL_STOP_CYCLE (ROUNDDIV(videoTIMER_FREQUENCY / videoMICROSEC_DIVISOR * videoPIXEL_STOP, 1000)-10)


///////////////////////////////////////////////////////////////////////////////
// Configuration check
#if videoPIXEL_CLOCK < videoPIXEL_CLOCK_MIN
#error "Pixel Clock can't generated using the current settings. Try to change the horizontal resolution, border width or Fpb clock"
#endif

static uint8_t l_zero[videoPIXEL_START_BYTES]= {0};

///////////////////////////////////////////////////////////////////////////////
// Global variables
//uint8_t g_video_bitmap[ videoVERTICAL_RESOLUTION*(videoHORIZONTAL_RESOLUTION/8)]; // image buffer
extern uint8_t g_gui_frame_buffer[];

///////////////////////////////////////////////////////////////////////////////
// Module global variables
static uint16_t l_vertical_line_count;
static uint8_t l_video_state;
static bool l_callback_is_running;
volatile uint8_t* l_video_address;

void drvGraphicsInitialize( void)
{
	int x,y;

	// clear video memory
	for( y=0; y<videoVERTICAL_RESOLUTION; y++)
		for (x=0; x<videoHORIZONTAL_RESOLUTION/8; x++)
			g_gui_frame_buffer[y*videoHORIZONTAL_RESOLUTION/8 + x]= 0x55;

  // Init vertical sync state machine
  l_video_state = VS_LINE;
  l_vertical_line_count = 1;

	// Init the SPI1 for pixel shifting
	SpiChnOpen( 1, SPICON_ON | SPICON_MSTEN | SPICON_FRMEN | SPICON_FRMSYNC | SPICON_FRMPOL, (videoPIXEL_CLOCK_BRG + 1) * 2 );

	 // DMA configuration for back porch extension
  DmaChnOpen( 1, 1, DMA_OPEN_DEFAULT);
  DmaChnSetEventControl( 1, DMA_EV_START_IRQ_EN | DMA_EV_START_IRQ(_SPI1_TX_IRQ));
  DmaChnSetTxfer( 1, (void*)l_zero, (void *)&SPI1BUF, videoPIXEL_START_BYTES, 1, 1);

	// Init DMA0 for video data transfer
	DmaChnOpen( 0, DMA_CHN_PRI0, DMA_OPEN_DEFAULT);
	DmaChnSetEventControl( 0, DMA_EV_START_IRQ_EN | DMA_EV_START_IRQ(_SPI1_TX_IRQ));
	DmaChnSetTxfer( 0, (void*)g_gui_frame_buffer, (void*)&SPI1BUF, videoHORIZONTAL_RESOLUTION/8+1, 1, 1);

  // chain DMA0 to completion of DMA1 transfer
  DmaChnSetControl( 0, DMA_CTL_CHAIN_EN | DMA_CTL_CHAIN_DIR);

	// Open output compare for sync generation
	OpenOC3( OC_ON | OC_TIMER3_SRC | OC_CONTINUE_PULSE, 0, videoSYNC_CYCLE);

	// Open output compare for pixel start generation
  OpenOC1( OC_ON | OC_TIMER3_SRC | OC_CONTINUE_PULSE, 0, videoSYNC_CYCLE);


	// Initialize timer for video signal generation
	OpenTimer3(T3_ON | T3_PS_1_1 | T3_SOURCE_INT, videoHORIZONTAL_SYNC_DIVISOR-1);
	ConfigIntTimer3(T3_INT_ON | T3_INT_PRIOR_7 | T3_INT_SUB_PRIOR_0);
}

void guiGraphicsUpdateCanvas( uint16_t in_left, uint16_t in_top, uint16_t in_right, uint16_t in_bottom )
{

}


void __ISR(_TIMER_3_VECTOR, ipl7srs) T3Interrupt( void)
{
	// switch to next video state
	if(--l_vertical_line_count == 0)
	{
		switch(l_video_state)
		{
			case VS_PREEQ:  // 0
				// next video state
				l_vertical_line_count = videoVSYNC_LINE_COUNT;
				l_video_state = VS_SYNC;

				// vertical sync pulse
				OC3R = videoHORIZONTAL_SYNC_DIVISOR - videoSYNC_CYCLE - videoBACK_PORCH_CYCLE;
				OC1R = videoHORIZONTAL_SYNC_DIVISOR - videoSYNC_CYCLE - videoBACK_PORCH_CYCLE;
				break;

			case VS_SYNC:   // 1
				l_vertical_line_count = videoPOSTEQ_LINE_COUNT;
				l_video_state = VS_POSTEQ;

				// horizontal sync pulse
				OC3R = videoSYNC_CYCLE;
				OC1R = videoSYNC_CYCLE;
				break;

			case VS_POSTEQ: // 2
				l_vertical_line_count = videoVERTICAL_RESOLUTION;
				l_video_address = g_gui_frame_buffer;
				l_video_state = VS_LINE;
				break;

			case VS_LINE:   // 3
			default:
				l_video_state = VS_PREEQ;
				l_vertical_line_count = videoPREEQ_LINE_COUNT;
				break;
		}
	}

	// handle video state
	if (l_video_state == VS_LINE)
	{
		DCH0SSA = KVA_TO_PA((void*) l_video_address-1);
		l_video_address += videoHORIZONTAL_RESOLUTION/8;
		DmaChnEnable(1);
	}

	// clear the interrupt flag
	mT3ClearIntFlag();
}
