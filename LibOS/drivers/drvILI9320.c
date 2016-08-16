#include <sysTypes.h>
#include <sysTimer.h>
#include <drvColorGraphics.h>
#include "sysConfig.h"


/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/
#define drvILI9320COMMAND_ADDRESS 0
#define drvILI9320DATA_ADDRESS 1

/*****************************************************************************/
/* Local function protoypes                                                  */
/*****************************************************************************/
static void drvILI9320WriteCommand(uint16_t in_command);
static void drvILI9320WriteData(uint16_t in_data);
static void drvILI9320WriteCommandAndData(uint16_t in_command, uint16_t in_data);
static void drvILI9320SetAddress(uint16_t in_x, uint16_t in_y);
static void drvILI9320SetWindow(uint16_t in_x1, uint16_t in_y1, uint16_t in_x2, uint16_t in_y2);
static uint16_t drvILI9320ColorConverter(guiColor in_color);
static void drvILI9320WaitForBusFree(void);

/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/
static uint16_t l_background_color;
static uint16_t l_foreground_color;

#ifdef guiSCREEN_LANDSCAPE
#define GET_X_COORDINATE(x, y) (y)
#define GET_Y_COORDINATE(x, y) (x)
#else
#define GET_X_COORDINATE(x, y) (x)
#define GET_Y_COORDINATE(x, y) (y)
#endif


void drvGraphicsInitialize(void)
{
	drvILI9320CSInitialize();
	drvILI9320ResetInitialize();
	drvILI9320PMP8Initialize();

	// reset display
	drvILI9320CS(PIN_HIGH);
	drvILI9320Reset(PIN_HIGH);
	sysTimerDelay(2);
	drvILI9320Reset(PIN_LOW);
	sysTimerDelay(2);
	drvILI9320Reset(PIN_HIGH);
	sysTimerDelay(50);

	drvILI9320CS(PIN_LOW);
#if 0
	drvILI9320WriteCommandAndData(0xe5, 0x8000);	// Set the Vcore voltage and this setting is must.
	drvILI9320WriteCommandAndData(0x00, 0x0001);	// Start internal OSC.
	drvILI9320WriteCommandAndData(0x01, 0x0100);	// set SS and SM bit
	drvILI9320WriteCommandAndData(0x02, 0x0700);	// set 1 line inversion
	drvILI9320WriteCommandAndData(0x03, 0x1038);	// set GRAM write direction and BGR=1.
	drvILI9320WriteCommandAndData(0x04, 0x0000);	// Resize register

	drvILI9320WriteCommandAndData(0x08, 0x0202);	// set the back porch and front porch
	drvILI9320WriteCommandAndData(0x09, 0x0000);	// set non-display area refresh cycle ISC[3:0]
	drvILI9320WriteCommandAndData(0x0A, 0x0000);	// FMARK function
	drvILI9320WriteCommandAndData(0x0C, 0x0000);	// RGB interface setting
	drvILI9320WriteCommandAndData(0x0D, 0x0000);	// Frame marker Position
	drvILI9320WriteCommandAndData(0x0F, 0x0000);	// RGB interface polarity

	//-----Power On sequence-----------------------
	drvILI9320WriteCommandAndData(0x10, 0x0000);	// SAP, BT[3:0], AP, DSTB, SLP, STB
	drvILI9320WriteCommandAndData(0x11, 0x0007);	// DC1[2:0], DC0[2:0], VC[2:0]
	drvILI9320WriteCommandAndData(0x12, 0x0000);	// VREG1OUT voltage
	drvILI9320WriteCommandAndData(0x13, 0x0000);	// VDV[4:0] for VCOM amplitude
	sysTimerDelay(200);

	drvILI9320WriteCommandAndData(0x10, 0x17B0);	// SAP, BT[3:0], AP, DSTB, SLP, STB
	drvILI9320WriteCommandAndData(0x11, 0x0037);	// DC1[2:0], DC0[2:0], VC[2:0]
	sysTimerDelay(50);
	drvILI9320WriteCommandAndData(0x12, 0x013A);	// VREG1OUT voltage
	sysTimerDelay(50);
	drvILI9320WriteCommandAndData(0x13, 0x1600);	// VREG1OUT voltage
	drvILI9320WriteCommandAndData(0x29, 0x000c);	// VCM[4:0] for VCOMH
	sysTimerDelay(50);

	//-----Gamma control-----------------------
	drvILI9320WriteCommandAndData(0x30, 0x0504);
	drvILI9320WriteCommandAndData(0x31, 0x0703);
	drvILI9320WriteCommandAndData(0x32, 0x0702);
	drvILI9320WriteCommandAndData(0x35, 0x0101);
	drvILI9320WriteCommandAndData(0x36, 0x0a1f);
	drvILI9320WriteCommandAndData(0x37, 0x0504);
	drvILI9320WriteCommandAndData(0x38, 0x0003);
	drvILI9320WriteCommandAndData(0x39, 0x0706);
	drvILI9320WriteCommandAndData(0x3C, 0x0707);
	drvILI9320WriteCommandAndData(0x3D, 0x091f);

	//-----Set RAM area-----------------------
	drvILI9320WriteCommandAndData(0x20, 0x0000);	// GRAM horizontal Address
	drvILI9320WriteCommandAndData(0x21, 0x0000);	// GRAM Vertical Address
	drvILI9320WriteCommandAndData(0x50, 0x0000);	// Horizontal GRAM Start Address
	drvILI9320WriteCommandAndData(0x51, 0x00EF);	// Horizontal GRAM End Address
	drvILI9320WriteCommandAndData(0x52, 0x0000);	// Vertical GRAM Start Address
	drvILI9320WriteCommandAndData(0x53, 0x013F);	// Vertical GRAM Start Address

	drvILI9320WriteCommandAndData(0x60, 0x2700);	// Gate Scan Line
	drvILI9320WriteCommandAndData(0x61, 0x0001);	// NDL,VLE, REV
	drvILI9320WriteCommandAndData(0x6A, 0x0000);	// set scrolling line

	//-----Partial Display Control------------
	drvILI9320WriteCommandAndData(0x80, 0x0000);
	drvILI9320WriteCommandAndData(0x81, 0x0000);
	drvILI9320WriteCommandAndData(0x82, 0x0000);
	drvILI9320WriteCommandAndData(0x83, 0x0000);
	drvILI9320WriteCommandAndData(0x84, 0x0000);
	drvILI9320WriteCommandAndData(0x85, 0x0000);

	//-----Panel Control----------------------
	drvILI9320WriteCommandAndData(0x90, 0x0010);
	drvILI9320WriteCommandAndData(0x92, 0x0000);
	drvILI9320WriteCommandAndData(0x93, 0x0003);
	drvILI9320WriteCommandAndData(0x95, 0x0110);
	drvILI9320WriteCommandAndData(0x97, 0x0000);
	drvILI9320WriteCommandAndData(0x98, 0x0000);

	//-----Display on-----------------------
	drvILI9320WriteCommandAndData(0x07, 0x0173);	// 262K color and display ON 
#endif
#if 1
	drvILI9320WriteCommandAndData(0xE5, 0x78F0); // set SRAM internal timing
	//drvILI9320WriteCommandAndData(0x01, 0x0100); // set Driver Output Control
	drvILI9320WriteCommandAndData(0x02, 0x0700); // set 1 line inversion
#ifdef guiSCREEN_LANDSCAPE
	drvILI9320WriteCommandAndData(0x01, 0x0000); // set Driver Output Control
	drvILI9320WriteCommandAndData(0x03, 0x1038); // set GRAM write direction and BGR=1.
#else
	drvILI9320WriteCommandAndData(0x01, 0x0100); // set Driver Output Control
	drvILI9320WriteCommandAndData(0x03, 0x1030); // set GRAM write direction and BGR=1.
#endif
	drvILI9320WriteCommandAndData(0x04, 0x0000); // Resize register
	drvILI9320WriteCommandAndData(0x08, 0x0207); // set the back porch and front porch
	drvILI9320WriteCommandAndData(0x09, 0x0000); // set non-display area refresh cycle ISC[3:0]
	drvILI9320WriteCommandAndData(0x0A, 0x0000); // FMARK function
	drvILI9320WriteCommandAndData(0x0C, 0x0000); // RGB interface setting
	drvILI9320WriteCommandAndData(0x0D, 0x0000); // Frame marker Position
	drvILI9320WriteCommandAndData(0x0F, 0x0000); // RGB interface polarity

	//*************Power On sequence ****************//
	drvILI9320WriteCommandAndData(0x10, 0x0000); // SAP, BT[3:0], AP, DSTB, SLP, STB
	drvILI9320WriteCommandAndData(0x11, 0x0007); // DC1[2:0], DC0[2:0], VC[2:0]
	drvILI9320WriteCommandAndData(0x12, 0x0000); // VREG1OUT voltage
	drvILI9320WriteCommandAndData(0x13, 0x0000); // VDV[4:0] for VCOM amplitude
	drvILI9320WriteCommandAndData(0x07, 0x0001);
	sysTimerDelay(200); // Dis-charge capacitor power voltage
	drvILI9320WriteCommandAndData(0x10, 0x1090); // SAP, BT[3:0], AP, DSTB, SLP, STB
	drvILI9320WriteCommandAndData(0x11, 0x0227); // Set DC1[2:0], DC0[2:0], VC[2:0]
	sysTimerDelay(50); // Delay 50ms
	drvILI9320WriteCommandAndData(0x12, 0x001F); // 0012
	sysTimerDelay(50); // Delay 50ms
	drvILI9320WriteCommandAndData(0x13, 0x1500); // VDV[4:0] for VCOM amplitude
	drvILI9320WriteCommandAndData(0x29, 0x0027); // 04  VCM[5:0] for VCOMH
	drvILI9320WriteCommandAndData(0x2B, 0x000D); // Set Frame Rate
	sysTimerDelay(50); // Delay 50ms
	drvILI9320WriteCommandAndData(0x20, 0x0000); // GRAM horizontal Address
	drvILI9320WriteCommandAndData(0x21, 0x0000); // GRAM Vertical Address
	// ----------- Adjust the Gamma Curve ----------//
	drvILI9320WriteCommandAndData(0x30, 0x0000);
	drvILI9320WriteCommandAndData(0x31, 0x0707);
	drvILI9320WriteCommandAndData(0x32, 0x0307);
	drvILI9320WriteCommandAndData(0x35, 0x0200);
	drvILI9320WriteCommandAndData(0x36, 0x0008);
	drvILI9320WriteCommandAndData(0x37, 0x0004);
	drvILI9320WriteCommandAndData(0x38, 0x0000);
	drvILI9320WriteCommandAndData(0x39, 0x0707);
	drvILI9320WriteCommandAndData(0x3C, 0x0002);
	drvILI9320WriteCommandAndData(0x3D, 0x1D04);
	//------------------ Set GRAM area ---------------//
	drvILI9320WriteCommandAndData(0x50, 0x0000); // Horizontal GRAM Start Address
	drvILI9320WriteCommandAndData(0x51, 0x00EF); // Horizontal GRAM End Address
	drvILI9320WriteCommandAndData(0x52, 0x0000); // Vertical GRAM Start Address
	drvILI9320WriteCommandAndData(0x53, 0x013F); // Vertical GRAM Start Address
	drvILI9320WriteCommandAndData(0x60, 0xA700); // Gate Scan Line
	drvILI9320WriteCommandAndData(0x61, 0x0001); // NDL,VLE, REV
	drvILI9320WriteCommandAndData(0x6A, 0x0000); // set scrolling line
	//-------------- Partial Display Control ---------//
	drvILI9320WriteCommandAndData(0x80, 0x0000);
	drvILI9320WriteCommandAndData(0x81, 0x0000);
	drvILI9320WriteCommandAndData(0x82, 0x0000);
	drvILI9320WriteCommandAndData(0x83, 0x0000);
	drvILI9320WriteCommandAndData(0x84, 0x0000);
	drvILI9320WriteCommandAndData(0x85, 0x0000);
	//-------------- Panel Control -------------------//
	drvILI9320WriteCommandAndData(0x90, 0x0010);
	drvILI9320WriteCommandAndData(0x92, 0x0600);
	drvILI9320WriteCommandAndData(0x07, 0x0133); // 262K color and display ON
#endif
	// remove CS
	drvPMP8WaitBusy();
	drvILI9320CS(PIN_HIGH);

	sysTimerDelay(50);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief refreshes full screen content (used only in frame buffer mode)
void drvColorGraphicsRefreshScreen(void)
{

}

///////////////////////////////////////////////////////////////////////////////
/// @brief Sets foreground color for drawing operations
/// @param in_color Color for foreground
void guiSetForegroundColor(guiColor in_color)
{
	l_foreground_color = drvILI9320ColorConverter(in_color);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Sets background color for drawing operations
/// @param in_color Color for background
void guiSetBackgroundColor(guiColor in_color)
{
	l_background_color = drvILI9320ColorConverter(in_color);
}

void drvColorGraphicsFillArea(guiCoordinate in_x1, guiCoordinate in_y1, guiCoordinate in_x2, guiCoordinate in_y2)
{
	uint16_t x1, y1, x2, y2;

	x1 = GET_X_COORDINATE(in_x1, in_y1);
	y1 = GET_Y_COORDINATE(in_x1, in_y1);

	x2 = GET_X_COORDINATE(in_x2, in_y2);
	y2 = GET_Y_COORDINATE(in_x2, in_y2);

	// check coordinates
	if(in_x1 > guiSCREEN_WIDTH)
		in_x1 = guiSCREEN_WIDTH - 1;

	if(in_y1 > guiSCREEN_HEIGHT)
		in_y1 = guiSCREEN_HEIGHT- 1;

	// check size
	if(in_x2 >= guiSCREEN_WIDTH)
		in_x2 = guiSCREEN_WIDTH - 1;

	if(in_y2 >= guiSCREEN_HEIGHT)
		in_y2 = guiSCREEN_HEIGHT - 1;

	// wait for communication bus will be free
	drvILI9320WaitForBusFree();

	drvILI9320CS(PIN_LOW);

	drvILI9320SetWindow(x1, y1, x2, y2);

	drvPMP8WriteDoubleByteBlock(drvILI9320DATA_ADDRESS, HIGH(l_foreground_color), LOW(l_foreground_color), (x2 - x1 + 1) *(y2 - y1 + 1));

	drvILI9320CS(PIN_HIGH);
}

void drvColorGraphicsBitBltFromResource(guiCoordinate in_destination_x, guiCoordinate in_destination_y,
																				guiCoordinate in_destination_width, guiCoordinate in_destination_height,
																				guiCoordinate in_source_x, guiCoordinate in_source_y,
																				guiCoordinate in_source_width, guiCoordinate in_source_height,
																				sysResourceAddress in_source_bitmap, uint8_t in_source_bit_per_pixel)
{
	uint16_t bitmap_row_bytes;
	uint8_t bitmap_data;
	uint16_t bitmap_x;
	uint16_t bitmap_y;
	sysResourceAddress source_pixel;
	uint16_t x1, y1, width, height;

	// reorder coordinates for the LCD
	x1 = GET_X_COORDINATE(in_destination_x + in_source_x, in_destination_y + in_source_y);
	y1 = GET_Y_COORDINATE(in_destination_x + in_source_x, in_destination_y + in_source_y);

	width = GET_X_COORDINATE(in_destination_width, in_destination_height);
	height = GET_Y_COORDINATE(in_destination_width, in_destination_height);

	// wait for communication bus will be free
	drvILI9320WaitForBusFree();

	// start communication
	drvILI9320CS(PIN_LOW);

	drvILI9320SetWindow(x1, y1, x1 + width - 1, y1 + height - 1);

	switch(in_source_bit_per_pixel)
	{
		case 1:
			bitmap_row_bytes = (in_source_width + 7) / 8;
			for(bitmap_y = in_source_y; bitmap_y < in_source_y + in_destination_height; bitmap_y++)
			{
				source_pixel = in_source_bitmap + bitmap_row_bytes * bitmap_y + in_source_x / 8;
				bitmap_data = drvResourceReadByte(source_pixel);
				bitmap_data <<= (in_source_x % 8);

				for(bitmap_x = in_source_x; bitmap_x < in_source_x + in_destination_width; bitmap_x++)
				{
					if((bitmap_x % 8) == 0)
						bitmap_data = drvResourceReadByte(source_pixel++);

					if((bitmap_data & 0x80) == 0)
					{
						drvILI9320WriteData(l_background_color);
					}
					else
					{
						drvILI9320WriteData(l_foreground_color);
					}

					bitmap_data <<= 1;
				}

				// remove CS
				drvILI9320WaitForBusFree();
				drvILI9320CS(PIN_LOW);
			}
			break;
		
		case 16:
			// TODO: clipping
			drvPMP8WriteBlock(drvILI9320DATA_ADDRESS, drvGetResourcePhysicalAddress(in_source_bitmap), in_source_width * in_source_height * sizeof(uint16_t));
			break;
	}
}

void drvColorGraphicsBitBlt(guiCoordinate in_destination_x, guiCoordinate in_destination_y,
														guiCoordinate in_destination_width, guiCoordinate in_destination_height,
														guiCoordinate in_source_x, guiCoordinate in_source_y,
														guiCoordinate in_source_width, guiCoordinate in_source_height,
														void* in_source_bitmap, uint8_t in_source_bit_per_pixel)
{
	uint16_t bitmap_row_bytes;
	uint8_t bitmap_data;
	uint16_t bitmap_x;
	uint16_t bitmap_y;
	uint8_t* source_pixel;
	uint16_t x1, y1, width, height;

	// reorder coordinates for the LCD
	x1 = GET_X_COORDINATE(in_destination_x + in_source_x, in_destination_y + in_source_y);
	y1 = GET_Y_COORDINATE(in_destination_x + in_source_x, in_destination_y + in_source_y);

	width = GET_X_COORDINATE(in_destination_width, in_destination_height);
	height = GET_Y_COORDINATE(in_destination_width, in_destination_height);


	// wait for communication bus will be free
	drvILI9320WaitForBusFree();

	// start communication
	drvILI9320CS(PIN_LOW);

	drvILI9320SetWindow(x1, y1, x1 + width - 1, y1 + height - 1);

	switch(in_source_bit_per_pixel)
	{
		case 1:
			bitmap_row_bytes = (in_source_width + 7) / 8;
			for(bitmap_y = in_source_y; bitmap_y < in_source_y + in_destination_height; bitmap_y++)
			{
				source_pixel = (uint8_t*)in_source_bitmap + bitmap_row_bytes * (bitmap_y + in_source_y) + in_source_x / 8;
				bitmap_data = *source_pixel;
				bitmap_data <<= (in_source_x % 8);

				for(bitmap_x = in_source_x; bitmap_x < in_source_x + in_destination_width; bitmap_x++)
				{
					if(((bitmap_x + in_source_x) % 8) == 0)
						bitmap_data = *source_pixel++;

					if((bitmap_data & 0x80) == 0)
					{
						drvILI9320WriteData(l_background_color);
					}
					else
					{
						drvILI9320WriteData(l_foreground_color);
					}

					bitmap_data <<= 1;
				}

				// remove CS
				drvILI9320WaitForBusFree();
				drvILI9320CS(PIN_LOW);
			}
			break;

		case 16:
			// TODO: clipping
			drvPMP8WriteBlock(drvILI9320DATA_ADDRESS, in_source_bitmap, in_source_width * in_source_height * sizeof(uint16_t));
			break;
	}
}


static void drvILI9320WaitForBusFree(void)
{
	drvPMP8WaitBusy();
	drvILI9320CS(PIN_HIGH);
	sysNOP();
	sysNOP();
	sysNOP();
	sysNOP();

	sysNOP();
	sysNOP();
	sysNOP();
	sysNOP();
}

static void drvILI9320SetAddress(uint16_t in_x, uint16_t in_y)
{
	drvILI9320WriteCommandAndData(0x20, in_y);
	drvILI9320WriteCommandAndData(0x21, in_x);
	drvILI9320WriteCommand(0x22);
}

static void drvILI9320SetWindow(uint16_t in_x1, uint16_t in_y1, uint16_t in_x2, uint16_t in_y2)
{
  drvILI9320WriteCommandAndData(0x20, in_x1);
  drvILI9320WriteCommandAndData(0x21, in_y1);
	drvILI9320WriteCommandAndData(0x50, in_x1);
	drvILI9320WriteCommandAndData(0x52, in_y1);
	drvILI9320WriteCommandAndData(0x51, in_x2);
	drvILI9320WriteCommandAndData(0x53, in_y2);
	drvILI9320WriteCommand(0x22);
}

static void drvILI9320WriteCommand(uint16_t in_command)
{
	drvILI9320PMP8WriteByte(drvILI9320COMMAND_ADDRESS, HIGH(in_command));
	drvILI9320PMP8WriteByte(drvILI9320COMMAND_ADDRESS, LOW(in_command));
}

static void drvILI9320WriteData(uint16_t in_data)
{
	drvILI9320PMP8WriteByte(drvILI9320DATA_ADDRESS, HIGH(in_data));
	drvILI9320PMP8WriteByte(drvILI9320DATA_ADDRESS, LOW(in_data));
}

static void drvILI9320WriteCommandAndData(uint16_t in_command, uint16_t in_data)
{
	// write command
	drvILI9320PMP8WriteByte(drvILI9320COMMAND_ADDRESS, HIGH(in_command));
	drvILI9320PMP8WriteByte(drvILI9320COMMAND_ADDRESS, LOW(in_command));

	// write data
	drvILI9320PMP8WriteByte(drvILI9320DATA_ADDRESS, HIGH(in_data));
	drvILI9320PMP8WriteByte(drvILI9320DATA_ADDRESS, LOW(in_data));
}


static uint16_t drvILI9320ColorConverter(guiColor in_color)
{
	uint8_t r,g,b;

	// RGB 565
	r = (uint8_t)((in_color >> 19) & 0x1f);
	g = (uint8_t)((in_color >> 10) & 0x3f);
	b = (uint8_t)((in_color >> 3) & 0x1f);

	return (r << 11) | (g << 5) | b;
}