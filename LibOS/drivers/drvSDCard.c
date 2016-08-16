/*****************************************************************************/
/* SD Card driver using SPI communiation mode                                */
/*                                                                           */
/* Copyright (C) 2014-2015 Laszlo Arvai                                      */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
// Includes
#include <fileSDCard.h>
#include <fileTypes.h>
#include <sysTimer.h>
#include <drvMassStorage.h>
#include "sysConfig.h"

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/
// <editor-fold defaultstate="collapsed" desc="Constants">

#define SDCARD_BLOCK_SIZE            512u  //Should always be 512 for v1 and v2 devices.


// Description: This macro represents an SD card start single data block token (used for single block writes)
#define DATA_START_TOKEN            0xFE

// Description: This macro represents an SD card start multi-block data token (used for multi-block writes)
#define DATA_START_MULTI_BLOCK_TOKEN    0xFC

// Description: This macro represents an SD card stop transmission token.  This is used when finishing a multi block write sequence.
#define DATA_STOP_TRAN_TOKEN        0xFD

// Description: This macro represents an SD card data accepted token
#define DATA_ACCEPTED               0x05

// Description: This macro indicates that the SD card expects to transmit or receive more data
#define MOREDATA    !0

// Description: This macro indicates that the SD card does not expect to transmit or receive more data
#define NODATA      0

// Description: This macro represents a floating SPI bus condition
#define MMC_FLOATING_BUS    0xFF

// Description: This macro represents a bad SD card response byte
#define MMC_BAD_RESPONSE    MMC_FLOATING_BUS

// Description: This macro defines the command code to reset the SD card
#define     cmdGO_IDLE_STATE        0
// Description: This macro defines the command code to initialize the SD card
#define     cmdSEND_OP_COND         1
// Description: This macro defined the command code to check for sector addressing
#define     cmdSEND_IF_COND         8
// Description: This macro defines the command code to get the Card Specific Data
#define     cmdSEND_CSD             9
// Description: This macro defines the command code to get the Card Information
#define     cmdSEND_CID             10
// Description: This macro defines the command code to stop transmission during a multi-block read
#define     cmdSTOP_TRANSMISSION    12
// Description: This macro defines the command code to get the card status information
#define     cmdSEND_STATUS          13
// Description: This macro defines the command code to set the block length of the card
#define     cmdSET_BLOCKLEN         16
// Description: This macro defines the command code to read one block from the card
#define     cmdREAD_SINGLE_BLOCK    17
// Description: This macro defines the command code to read multiple blocks from the card
#define     cmdREAD_MULTI_BLOCK     18
// Description: This macro defines the command code to tell the media how many blocks to pre-erase (for faster multi-block writes to follow)
//Note: This is an "application specific" command.  This tells the media how many blocks to pre-erase for the subsequent WRITE_MULTI_BLOCK
#define     cmdSET_WR_BLK_ERASE_COUNT   23
// Description: This macro defines the command code to write one block to the card
#define     cmdWRITE_SINGLE_BLOCK   24
// Description: This macro defines the command code to write multiple blocks to the card
#define     cmdWRITE_MULTI_BLOCK    25
// Description: This macro defines the command code to set the address of the start of an erase operation
#define     cmdTAG_SECTOR_START     32
// Description: This macro defines the command code to set the address of the end of an erase operation
#define     cmdTAG_SECTOR_END       33
// Description: This macro defines the command code to erase all previously selected blocks
#define     cmdERASE                38
//Description: This macro defines the command code to intitialize an SD card and provide the CSD register value.
//Note: this is an "application specific" command (specific to SD cards) and must be preceded by cmdAPP_CMD.
#define     cmdSD_SEND_OP_COND      41
// Description: This macro defines the command code to begin application specific command inputs
#define     cmdAPP_CMD              55
// Description: This macro defines the command code to get the OCR register information from the card
#define     cmdREAD_OCR             58
// Description: This macro defines the command code to disable CRC checking
#define     cmdCRC_ON_OFF           59

// Desription: Media Response Delay Timeouts
#define NCR_TIMEOUT     (WORD)20        //Byte times before command response is expected (must be at least 8)
#define NAC_TIMEOUT     (DWORD)0x40000  //SPI byte times we should wait when performing read operations (should be at least 100ms for SD cards)
#define WRITE_TIMEOUT   (DWORD)0xA0000  //SPI byte times to wait before timing out when the media is performing a write operation (should be at least 250ms for SD cards).

#define drvSDCSPI_SLOW_BAUD_RATE 300000	// should be 400kHz but be on safe side

// </editor-fold>

/*****************************************************************************/
/* Types                                                                     */
/*****************************************************************************/
// <editor-fold defaultstate="collapsed" desc="Types">

/**
 * @brief An enumeration of SD commands
 *
 * This enumeration corresponds to the position of each command in the SD command array
 * These macros indicate to the SendSDCardCommand function which element of the sdmmc_cmdtable array
 * to retrieve command code information from.
 */
typedef enum
{
	GO_IDLE_STATE,
	SEND_OP_COND,
	SEND_IF_COND,
	SEND_CSD,
	SEND_CID,
	STOP_TRANSMISSION,
	SEND_STATUS,
	SET_BLOCKLEN,
	READ_SINGLE_BLOCK,
	READ_MULTI_BLOCK,
	WRITE_SINGLE_BLOCK,
	WRITE_MULTI_BLOCK,
	TAG_SECTOR_START,
	TAG_SECTOR_END,
	ERASE,
	APP_CMD,
	READ_OCR,
	CRC_ON_OFF,
	SD_SEND_OP_COND,
	SET_WR_BLK_ERASE_COUNT
}	SDCardCommandIndex;

/**
 * @brief Enumeration of different SD response types
 */
typedef enum
{
    R1,     // R1 type response
    R1b,    // R1b type response
    R2,     // R2 type response
    R3,     // R3 type response
    R7      // R7 type response
} SDCardCommandResponseType;

/**
 * @brief SD card command data structure
 *
 * This structure is used to create a command table of information needed for each relevant SD command
 */
typedef struct
{
	uint8_t CmdCode;												// The command code
	uint8_t CRC;														// The CRC value for that command
  SDCardCommandResponseType	ResponseType;	// The response type
	uint8_t MoreDataExpected;								// Set to MOREDATA or NODATA, depending on whether more data is expected or not
} SDCardCommandEntry;

/**
 * @brief The format of an R1 type response
 *
 * This union represents different ways to access an SD card R1 type response packet.
 */
typedef union
{
	uint8_t _byte;                         // Byte-wise access

	// This structure allows bitwise access of the response
	struct
  {
		unsigned IN_IDLE_STATE:1;       // Card is in idle state
		unsigned ERASE_RESET:1;         // Erase reset flag
		unsigned ILLEGAL_CMD:1;         // Illegal command flag
		unsigned CRC_ERR:1;             // CRC error flag
		unsigned ERASE_SEQ_ERR:1;       // Erase sequence error flag
		unsigned ADDRESS_ERR:1;         // Address error flag
		unsigned PARAM_ERR:1;           // Parameter flag
		unsigned B7:1;                  // Unused bit 7
  };
 } RESPONSE_1;

/**
 * @brief The format of an R2 type response
 *
 * This union represents different ways to access an SD card R2 type response packet
 */
typedef union
{
	uint16_t _word;
  struct
  {
		uint8_t      _byte0;
		uint8_t      _byte1;
  };
  struct
  {
		unsigned IN_IDLE_STATE:1;
		unsigned ERASE_RESET:1;
		unsigned ILLEGAL_CMD:1;
		unsigned CRC_ERR:1;
		unsigned ERASE_SEQ_ERR:1;
		unsigned ADDRESS_ERR:1;
		unsigned PARAM_ERR:1;
		unsigned B7:1;
		unsigned CARD_IS_LOCKED:1;
		unsigned WP_ERASE_SKIP_LK_FAIL:1;
		unsigned ERROR:1;
		unsigned CC_ERROR:1;
		unsigned CARD_ECC_FAIL:1;
		unsigned WP_VIOLATION:1;
		unsigned ERASE_PARAM:1;
		unsigned OUTRANGE_CSD_OVERWRITE:1;
	};
} RESPONSE_2;

/**
 * @brief The format of an R7 or R3 type response
 *
 * This union represents different ways to access an SD card R7 type response packet.
 */
typedef union
{
	struct
  {
		uint8_t _byte;                         // Byte-wise access
		union
		{
			//Note: The SD card argument response field is 32-bit, big endian format.
			//However, the C compiler stores 32-bit values in little endian in RAM.
			//When writing to the _returnVal/argument bytes, make sure to byte
			//swap the order from which it arrived over the SPI from the SD card.
			uint32_t _returnVal;
			struct
			{
				uint8_t _byte0;
				uint8_t _byte1;
				uint8_t _byte2;
				uint8_t _byte3;
			};
		}argument;
  } bytewise;

	// This structure allows bitwise access of the response
  struct
  {
		struct
		{
			unsigned IN_IDLE_STATE:1;       // Card is in idle state
			unsigned ERASE_RESET:1;         // Erase reset flag
			unsigned ILLEGAL_CMD:1;         // Illegal command flag
			unsigned CRC_ERR:1;             // CRC error flag
			unsigned ERASE_SEQ_ERR:1;       // Erase sequence error flag
			unsigned ADDRESS_ERR:1;         // Address error flag
			unsigned PARAM_ERR:1;           // Parameter flag
			unsigned B7:1;                  // Unused bit 7
		}bits;
		uint32_t _returnVal;
  } bitwise;
} RESPONSE_7;

/**
 * @brief A union of responses from an SD card
 *
 * The SDCardCommandResponse union represents any of the possible responses that an SD card can return after
 * being issued a command.
 */
typedef union
{
	RESPONSE_1  r1;
	RESPONSE_2  r2;
	RESPONSE_7  r7;
} SDCardCommandResponse;

/**
 * @brief An SD command packet
 *
 * This union represents different ways to access an SD card command packet
 */
typedef struct
{
	// The CRC byte
	union
	{
		uint8_t crc;								// byte access
		struct
		{
			unsigned END_BIT:1;				// Packet end bit
			unsigned CRC7:7;					// CRC value
		};
	};

	// Address
	union
	{
		uint32_t address;
		struct
		{
			BYTE addr0;								// Address byte 0
			BYTE addr1;								// Address byte 1
			BYTE addr2;								// Address byte 2
			BYTE addr3;								// Address byte 3
		};
	};

	// command
	union
	{
		uint8_t cmd;               // Command code byte
		struct
		{
			unsigned CMD_INDEX:6;      // Command code
			unsigned TRANSMIT_BIT:1;   // Transmit bit
			unsigned START_BIT:1;      // Packet start bit
		};
	};
} SDCardCommand;

typedef enum
{
	CARD_MODE_NORMAL,
	CARD_MODE_HC
} SDCardType;

// </editor-fold>

/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/
// <editor-fold defaultstate="collapsed" desc="Module Global variables">

static fileMediaInfo l_media_info = {0};
static SDCardType l_card_type;
static uint16_t l_SDCardCRCTable[];


////////////////////////////////////////////////////////////////////////////////
// SD Card Command Table
const SDCardCommandEntry l_command_table[] =
{
	// cmd                      crc     response
	{cmdGO_IDLE_STATE,          0x95,   R1,     NODATA},
	{cmdSEND_OP_COND,           0xF9,   R1,     NODATA},
	{cmdSEND_IF_COND,						0x87,   R7,     NODATA},
	{cmdSEND_CSD,               0xAF,   R1,     MOREDATA},
	{cmdSEND_CID,               0x1B,   R1,     MOREDATA},
	{cmdSTOP_TRANSMISSION,      0xC3,   R1b,    NODATA},
	{cmdSEND_STATUS,            0xAF,   R2,     NODATA},
	{cmdSET_BLOCKLEN,           0xFF,   R1,     NODATA},
	{cmdREAD_SINGLE_BLOCK,      0xFF,   R1,     MOREDATA},
	{cmdREAD_MULTI_BLOCK,       0xFF,   R1,     MOREDATA},
	{cmdWRITE_SINGLE_BLOCK,     0xFF,   R1,     MOREDATA},
	{cmdWRITE_MULTI_BLOCK,      0xFF,   R1,     MOREDATA},
	{cmdTAG_SECTOR_START,       0xFF,   R1,     NODATA},
	{cmdTAG_SECTOR_END,         0xFF,   R1,     NODATA},
	{cmdERASE,                  0xDF,   R1b,    NODATA},
	{cmdAPP_CMD,                0x73,   R1,     NODATA},
	{cmdREAD_OCR,               0x25,   R7,     NODATA},
	{cmdCRC_ON_OFF,             0x25,   R1,     NODATA},
	{cmdSD_SEND_OP_COND,        0xFF,   R7,     NODATA}, //Actual response is R3, but has same number of bytes as R7.
	{cmdSET_WR_BLK_ERASE_COUNT, 0xFF,   R1,     NODATA}
};

// </editor-fold>

/*****************************************************************************/
/* Local function prototypes                                                 */
/*****************************************************************************/
//static bool drvSDCardDetectMedia(void);
//fileMediaInfo* drvSDCardInitMedia(void);

SDCardCommandResponse SDCardSendCommand(SDCardCommandIndex in_command, uint32_t in_address);
static uint16_t SDCardCRC16Get(uint8_t *in_data, uint16_t in_length);

/*****************************************************************************/
/* Public functions                                                          */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
//! Initialize SD Card access
void drvSDCardInitialize(void)
{
	l_media_info.Status = fileMS_UNKNOWN;
}

///////////////////////////////////////////////////////////////////////////////
//! Cleans up all hardware line for SD Card access. The card will no longer
//! be accessible.
void drvSDCardCleanUp(void)
{
	drvSDCCSSPICleanUp();
	l_media_info.Status = fileMS_UNKNOWN;
}

///////////////////////////////////////////////////////////////////////////////
//! Reads absolute sector from the SD Card
//! \param Buffer to read the sector content
//! \param LBA address of the sector
//! \return true if operation was success
bool drvSDCardReadSector( uint8_t* out_buffer, uint32_t in_lba )
{
	uint8_t data;
	SDCardCommandResponse response;
	uint32_t timeout;
	bool success;
	uint16_t byte_counter;
	uint16_t crc;
	uint8_t* buffer = out_buffer;
#ifdef drvSDC_CRC_ENABLED
	uint16_t crc_calc;
#endif

	//SDHC cards are addressed on a 512 byte block basis.  This is 1:1 equivalent
	//to LBA addressing.  For standard capacity media, the media is expecting
	//a complete byte address.  Therefore, to convert from the LBA address to the
	//byte address, we need to multiply by 512.
	if (l_card_type == CARD_MODE_NORMAL)
	{
		in_lba <<= 9; //Equivalent to multiply by 512
	}

	response = SDCardSendCommand(READ_SINGLE_BLOCK, in_lba);
  //Note: SDCardSendCommand() sends 8 SPI clock cycles after getting the
  //response.  This meets the NAC min timing paramemter, so we don't
  //need additional clocking here.
  // Make sure the command was accepted successfully
	if(response.r1._byte != 0x00)
	{
		//Perhaps the card isn't initialized or present.
		return false;
	}

	//We successfully sent the READ_SINGLE_BLOCK command to the media,
	//and we need to keep polling the media until it sends
	//us the data start token byte.  This could typically take a
	//couple/few milliseconds, up to a maximum of 100ms.
	timeout = NAC_TIMEOUT; //prepare timeout counter for next state
	success = true;
	while(timeout != 0 && success)
	{
		timeout--;
		data = drvSDCSPISendAndReceiveByte(0xFF);
		if(data == DATA_START_TOKEN)
		{
			//We got the start token.  Ready to receive the data
			//block now.
			break;
		}
		else
		{
			if(data != MMC_FLOATING_BUS)
			{
				//We got an unexpected non-0xFF, non-start token byte back?
				//Some kind of error must have occurred.
				success = false;
			}
		}
	}

	if(success)
	{
		//This operation directly dictates data thoroughput in the
		//application, therefore optimized code should be used for each
		//processor type.
		byte_counter = SDCARD_BLOCK_SIZE;

		buffer--;
		data = 0;
		crc_calc = 0;
		while(1)
		{
			drvSDCSPISendAsynchronByte(0xFF);
#ifdef drvSDC_CRC_ENABLED
			crc_calc = (crc_calc << 8) ^ (uint16_t)l_SDCardCRCTable[(crc_calc >> 8) ^ data];
#endif
			buffer++;
			if((--byte_counter) == 0)
			{
				break;
			}
			data = drvSDCSPIReceiveAsynchronByte();
      *buffer = data;
		}

		// last data byte
		data = drvSDCSPIReceiveAsynchronByte();
    *buffer = data;
#ifdef drvSDC_CRC_ENABLED
		crc_calc = (crc_calc << 8) ^ (uint16_t)l_SDCardCRCTable[(crc_calc >> 8) ^ data];
#endif

		//Read two bytes to receive the CRC-16 value on the data block.
    crc = drvSDCSPISendAndReceiveByte(0xFF);
    crc = (crc << 8) + drvSDCSPISendAndReceiveByte(0xFF);

		// check CRC
#ifdef drvSDC_CRC_ENABLED
		if(crc != crc_calc)
			success = false;
#endif
	}

	// deselect card
	drvSDCCS(PIN_HIGH);
  drvSDCSPISendAndReceiveByte(0xFF); // delay

	return success;
}

///////////////////////////////////////////////////////////////////////////////
//! Writes absolute sector to the SD Card
//! \param Buffer to write ti the card
//! \param LBA address of the sector
//! \return true if operation was success
bool drvSDCardWriteSector( uint8_t* in_buffer, uint32_t in_lba )
{

}

uint32_t drvSDCardIOControl(uint16_t in_function_code)
{
	switch(in_function_code)
	{
		case fileIOFUNC_MEDIA_DETECTED:
			return l_media_info.Status == fileMS_OK;

		case fileIOFUNC_DETECT_AND_INIT_MEDIA:
			return drvSDCardInitMedia();

	}

}

/*****************************************************************************/
/* Local functions                                                           */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Detect media avaiability (card is plugged in)
/// @return True if media was detected
bool drvSDCardDetectMedia(void)
{
#ifdef drvSDCCD
	return (drvSDCCD() == PIN_LOW);
#else
	SDCardCommandResponse    response;
	uint8_t timeout;

	//First check if SPI module is enabled or not.
	if (l_media_info.Status == fileMS_UNKNOWN)
	{
		//If the SPI module is not enabled, then the media has evidently not
		//been initialized.  Try to send CMD0 and CMD13 to reset the device and
		//get it into SPI mode (if present), and then request the status of
		//the media.  If this times out, then the card is presumably not physically
		//present.
		drvSDCCSSPIInit();
		drvSDCCS(PIN_HIGH);
		drvSDCCSInit();

		// start with a slow SPI clock speed <=400kHz
		drvSDCSPISetBaudRate(drvSDCSPI_SLOW_BAUD_RATE);

    //Send CMD0 to reset the media
	  //If the card is physically present, then we should get a valid response.
    timeout = 4;
    do
    {
			//Toggle chip select, to make media abandon whatever it may have been doing
			//before.  This ensures the CMD0 is sent freshly after CS is asserted low,
			//minimizing risk of SPI clock pulse master/slave syncronization problems,
			//due to possible application noise on the SCK line.
			drvSDCCS(PIN_HIGH);
			drvSDCSPISendAndReceiveByte(0xFF);	//Send some "extraneous" clock pulses.  If a previous
																					//command was terminated before it completed normally,
																					//the card might not have received the required clocking
																					//following the transfer.
			drvSDCCS(PIN_LOW);
			timeout--;

      //Send CMD0 to software reset the device
      response = SDCardSendCommand(GO_IDLE_STATE, 0x0);
		} while((response.r1._byte != 0x01) && (timeout != 0));

		//Check if response was invalid (R1 response byte should be = 0x01 after GO_IDLE_STATE)
		if(response.r1._byte != 0x01)
		{
			drvSDCardCleanUp();
			return false;
		}
		else
		{
			//Card is presumably present.  The SDI pin should have a pull up resistor on
			//it, so the odds of SDI "floating" to 0x01 after sending CMD0 is very
			//remote, unless the media is genuinely present.  Therefore, we should
			//try to perform a full card initialization sequence now.
			drvSDCardInitMedia();    //Can block and take a long time to execute.
			if(l_media_info.Status == fileMS_OK)
			{
				/* if the card was initialized correctly, it means it is present */
				return true;
			}
			else
			{
				drvSDCardCleanUp();
				return false;
			}
		}
	}
	else
	{
		//The SPI module was already enabled.  This most likely means the media is
		//present and has already been initialized.  However, it is possible that
		//the user could have unplugged the media, in which case it is no longer
		//present.  We should send it a command, to check the status.
		drvSDCSPISetBaudRate(drvSDCSPI_SLOW_BAUD_RATE);
		response = SDCardSendCommand(SEND_STATUS,0x0);
		if((response.r2._word & 0xEC0C) != 0x0000)
		{
			//The card didn't respond with the expected result.  This probably
			//means it is no longer present.  We can try to re-initialized it,
			//just to be doubly sure.
			drvSDCardCleanUp();
			drvSDCardInitMedia();    //Can block and take a long time to execute.
			if(l_media_info.Status == fileMS_OK)
    	{
				/* if the card was initialized correctly, it means it is present */
    		return true;
			}
			else
    	{
				drvSDCardCleanUp();
				return false;
			}
		}
    else
    {
			//The CMD13 response to SEND_STATUS was valid.  This presumably
      //means the card is present and working normally.
      return true;
		}
	}
#endif
}

/** @brief Initializes the SD card.
 *
 *   Psuedo code flow for the media initialization process is as follows:
 *
 * -------------------------------------------------------------------------------------------
 * SD Card SPI Initialization Sequence (for physical layer v1.x or v2.0 device) is as follows:
 * -------------------------------------------------------------------------------------------
 * 0.  Power up tasks
 *     a.  Initialize microcontroller SPI module to no more than 400kbps rate so as to support MMC devices.
 *     b.  Add delay for SD card power up, prior to sending it any commands.  It wants the
 *         longer of: 1ms, the Vdd ramp time (time from 2.7V to Vdd stable), and 74+ clock pulses.
 * 1.  Send CMD0 (GO_IDLE_STATE) with CS = 0.  This puts the media in SPI mode and software resets the SD/MMC card.
 * 2.  Send CMD8 (SEND_IF_COND).  This requests what voltage the card wants to run at.
 *     Note: Some cards will not support this command.
 *     a.  If illegal command response is received, this implies either a v1.x physical spec device, or not an SD card (ex: MMC).
 *     b.  If normal response is received, then it must be a v2.0 or later SD memory card.
 *
 * If v1.x device:
 * -----------------
 * 3.  Send CMD1 repeatedly, until initialization complete (indicated by R1 response byte/idle bit == 0)
 * 4.  Basic initialization is complete.  May now switch to higher SPI frequencies.
 * 5.  Send CMD9 to read the CSD structure.  This will tell us the total flash size and other info which will be useful later.
 * 6.  Parse CSD structure bits (based on v1.x structure format) and extract useful information about the media.
 * 7.  The card is now ready to perform application data transfers.
 *
 * If v2.0+ device:
 * -----------------
 * 3.  Verify the voltage range is feasible.  If not, unusable card, should notify user that the card is incompatible with this host.
 * 4.  Send CMD58 (Read OCR).
 * 5.  Send CMD55, then ACMD41 (SD_SEND_OP_COND, with HCS = 1).
 *     a.  Loop CMD55/ACMD41 until R1 response byte == 0x00 (indicating the card is no longer busy/no longer in idle state).
 * 6.  Send CMD58 (Get CCS).
 *     a.  If CCS = 1 --> SDHC card.
 *     b.  If CCS = 0 --> Standard capacity SD card (which is v2.0+).
 * 7.  Basic initialization is complete.  May now switch to higher SPI frequencies.
 * 8.  Send CMD9 to read the CSD structure.  This will tell us the total flash size and other info which will be useful later.
 * 9.  Parse CSD structure bits (based on v2.0 structure format) and extract useful information about the media.
 * 10. The card is now ready to perform application data transfers.
 * --------------------------------------------------------------------------------
 *
 *  @return Returns true when media was found and initialized
 */
bool drvSDCardInitMedia(void)
{
	uint8_t i;
	SDCardCommandResponse response;
	uint16_t timeout;
	uint8_t count, index;
	uint8_t CSDResponse[20];
	uint32_t c_size;
	uint8_t c_size_mult;
	uint8_t block_len;

	// delay until Vdd stabilized
  sysTimerDelay(30);

	// Init media card interface
	drvSDCCSSPIInit();
	drvSDCCS(PIN_HIGH);
	drvSDCCSInit();

  // start with a slow SPI clock speed <=400kHz
	drvSDCSPISetBaudRate(drvSDCSPI_SLOW_BAUD_RATE);    
		
  //Generate 80 clock pulses.
  for(i = 0; i < 10; i++)
		drvSDCSPISendAndReceiveByte(0xFF);

  // Send CMD0 (with CS = 0) to reset the media and put SD cards into SPI mode.
  timeout = 100;
  do
  {
		//Toggle chip select, to make media abandon whatever it may have been doing
		//before.  This ensures the CMD0 is sent freshly after CS is asserted low,
    //minimizing risk of SPI clock pulse master/slave syncronization problems, 
    //due to possible application noise on the SCK line.
		drvSDCCS(PIN_HIGH);
    drvSDCSPISendAndReceiveByte(0xFF);  //Send some "extraneous" clock pulses.  If a previous
																				//command was terminated before it completed normally,
																				//the card might not have received the required clocking
																				//following the transfer.
    drvSDCCS(PIN_LOW);
    timeout--;

    //Send CMD0 to software reset the device
    response = SDCardSendCommand(GO_IDLE_STATE, 0x0);

  } while((response.r1._byte != 0x01) && (timeout != 0));

	if(timeout == 0)
	{
    drvSDCCS(PIN_HIGH);

		drvSDCSPISendAndReceiveByte(0xFF);	//Send some "extraneous" clock pulses.  If a previous
																				//command was terminated before it completed normally,
																				//the card might not have received the required clocking
																				//following the transfer.
		drvSDCCS(PIN_LOW);

		//Send CMD12, to stop any read/write transaction that may have been in progress
		response = SDCardSendCommand(STOP_TRANSMISSION, 0x0);    //Blocks until SD card signals non-busy

		//Now retry to send send CMD0 to perform software reset on the media
		response = SDCardSendCommand(GO_IDLE_STATE, 0x0);
		if(response.r1._byte != 0x01) //Check if card in idle state now.
		{
			//Card failed to process CMD0 yet again.  At this point, the proper thing
			//to do would be to power cycle the card and retry, if the host
			//circuitry supports disconnecting the SD card power.  Since the
			//SD/MMC PICtail+ doesn't support software controlled power removal
			//of the SD card, there is nothing that can be done with this hardware.
			//Therefore, we just give up now.  The user needs to physically
			//power cycle the media and/or the whole board.
			l_media_info.Status = fileMS_ERROR;
			return false;
		}
	}//if(timeout == 0) [for the CMD0 transmit loop]

	//Send CMD8 (SEND_IF_COND) to specify/request the SD card interface condition (ex: indicate what voltage the host runs at).
	//0x000001AA --> VHS = 0001b = 2.7V to 3.6V.  The 0xAA LSB is the check pattern, and is arbitrary, but 0xAA is recommended (good blend of 0's and '1's).
	//The SD card has to echo back the check pattern correctly however, in the R7 response.
	//If the SD card doesn't support the operating voltage range of the host, then it may not respond.
	//If it does support the range, it will respond with a type R7 reponse packet (6 bytes/48 bits).
	//Additionally, if the SD card is MMC or SD card v1.x spec device, then it may respond with
	//invalid command.  If it is a v2.0 spec SD card, then it is mandatory that the card respond
	//to CMD8.
	response = SDCardSendCommand(SEND_IF_COND, 0x1AA);   //Note: If changing "0x1AA", CRC value in table must also change.
	if(((response.r7.bytewise.argument._returnVal & 0xFFF) == 0x1AA) && (!response.r7.bitwise.bits.ILLEGAL_CMD))
  {
		//If we get to here, the device supported the CMD8 command and didn't complain about our host
		//voltage range.
		//Most likely this means it is either a v2.0 spec standard or high capacity SD card (SDHC)

		//Send CMD58 (Read OCR [operating conditions register]).  Reponse type is R3, which has 5 bytes.
		//Byte 4 = normal R1 response byte, Bytes 3-0 are = OCR register value.
		response = SDCardSendCommand(READ_OCR, 0x0);

		//Now that we have the OCR register value in the reponse packet, we could parse
		//the register contents and learn what voltage the SD card wants to run at.
		//If our host circuitry has variable power supply capability, it could
		//theoretically adjust the SD card Vdd to the minimum of the OCR to save power.

		//Now send CMD55/ACMD41 in a loop, until the card is finished with its internal initialization.
		//Note: SD card specs recommend >= 1 second timeout while waiting for ACMD41 to signal non-busy.
		for(timeout = 0; timeout < 0xFFFF; timeout++)
		{
			//Send CMD55 (lets SD card know that the next command is application specific (going to be ACMD41)).
			SDCardSendCommand(APP_CMD, 0x00000000);

			//Send ACMD41.  This is to check if the SD card is finished booting up/ready for full frequency and all
			//further commands.  Response is R3 type (6 bytes/48 bits, middle four bytes contain potentially useful data).
      //Note: When sending ACMD41, the HCS bit is bit 30, and must be = 1 to tell SD card the host supports SDHC
			response = SDCardSendCommand(SD_SEND_OP_COND,0x40000000); //bit 30 set

			//The R1 response should be = 0x00, meaning the card is now in the "standby" state, instead of
			//the "idle" state (which is the default initialization state after CMD0 reset is issued).  Once
			//in the "standby" state, the SD card is finished with basic intitialization and is ready
			//for read/write and other commands.
			if(response.r1._byte == 0)
			{
				break;  //Break out of for() loop.  Card is finished initializing.
			}
		}
		if(timeout >= 0xFFFF)
		{
    		l_media_info.Status = fileMS_ERROR;
		}

		//Now send CMD58 (Read OCR register).  The OCR register contains important
		//info we will want to know about the card (ex: standard capacity vs. SDHC).
    response = SDCardSendCommand(READ_OCR, 0x0);

		//Now check the CCS bit (OCR bit 30) in the OCR register, which is in our response packet.
		//This will tell us if it is a SD high capacity (SDHC) or standard capacity device.
		if(response.r7.bytewise.argument._returnVal & 0x40000000)    //Note the HCS bit is only valid when the busy bit is also set (indicating device ready).
		{
			l_card_type = CARD_MODE_HC;
    }
    else
    {
			l_card_type = CARD_MODE_NORMAL;
    }

		//SD Card should now be finished with initialization sequence.  Device should be ready
		//for read/write commands.

	}//if(((response.r7.bytewise._returnVal & 0xFFF) == 0x1AA) && (!response.r7.bitwise.bits.ILLEGAL_CMD))
  else
	{
		//The CMD8 wasn't supported.  This means the card is not a v2.0 card.
    //Presumably the card is v1.x device, standard capacity (not SDHC).

		drvSDCCS(PIN_HIGH);                         // deselect the devices
    sysTimerDelay(1);
    drvSDCCS(PIN_LOW);                          // select the device

		//The CMD8 wasn't supported.  This means the card is definitely not a v2.0 SDHC card.
    l_card_type = CARD_MODE_NORMAL;

    // According to the spec CMD1 must be repeated until the card is fully initialized
		timeout = 0x1FFF;
    do
    {
			//Send CMD1 to initialize the media.
      response = SDCardSendCommand(SEND_OP_COND, 0x00000000);    //When argument is 0x00000000, this queries MMC cards for operating voltage range
			timeout--;
		}while((response.r1._byte != 0x00) && (timeout != 0));

    // see if it failed
    if(timeout == 0)
    {
			l_media_info.Status = fileMS_ERROR;
			drvSDCCS(PIN_HIGH);                         // deselect the devices
    }
    else
    {
			//Set read/write block length to 512 bytes.  Note: commented out since
			//this theoretically isn't necessary, since all cards v1 and v2 are
			//required to support 512 byte block size, and this is supposed to be
			//the default size selected on cards that support other sizes as well.
      //response = SDCardSendCommand(SET_BLOCKLEN, 0x00000200);    //Set read/write block length to 512 bytes
    }
	}

  //Temporarily deselect device
  drvSDCCS(PIN_HIGH);

	//Basic initialization of media is now complete.  The card will now use push/pull
	//outputs with fast drivers.  Therefore, we can now increase SPI speed to
	//either the maximum of the microcontroller or maximum of media, whichever
	//is slower.  MMC media is typically good for at least 20Mbps SPI speeds.
	//SD cards would typically operate at up to 25Mbps or higher SPI speeds.
	drvSDCSPISetBaudRate(drvSDCSPI_BAUD_RATE);

  drvSDCCS(PIN_LOW);

	/* Send the CMD9 to read the CSD register */
  timeout = NCR_TIMEOUT;
  do
  {
		//Send CMD9: Read CSD data structure.
		response = SDCardSendCommand(SEND_CSD, 0x00);
		timeout--;
	}while((response.r1._byte != 0x00) && (timeout != 0));

  if(timeout == 0x00)
  {
		//Media failed to respond to the read CSD register operation.
		l_media_info.Status = fileMS_ERROR;
		drvSDCCS(PIN_HIGH);

    return false;
  }

	/* According to the simplified spec, section 7.2.6, the card will respond
	with a standard response token, followed by a data block of 16 bytes
	suffixed with a 16-bit CRC.*/
	index = 0;
	for (count = 0; count < 20u; count ++)
	{
		CSDResponse[index] = drvSDCSPISendAndReceiveByte(0xFF);
		index++;
		/* Hopefully the first byte is the datatoken, however, some cards do
		not send the response token before the CSD register.*/
		if((count == 0) && (CSDResponse[0] == DATA_START_TOKEN))
		{
			/* As the first byte was the datatoken, we can drop it. */
			index = 0;
		}
	}

	//Extract some fields from the response for computing the card capacity.
	//Note: The structure format depends on if it is a CSD V1 or V2 device.
	//Therefore, need to first determine version of the specs that the card
	//is designed for, before interpreting the individual fields.

	//Calculate the l_media_info.LastLBA (see SD card physical layer simplified spec 2.0, section 5.3.2).
	//In USB mass storage applications, we will need this information to
	//correctly respond to SCSI get capacity requests.  Note: method of computing
	//l_media_info.LastLBA depends on CSD structure spec version (either v1 or v2).
	if(CSDResponse[0] & 0xC0)	//Check CSD_STRUCTURE field for v2+ struct device
	{
		//Must be a v2 device (or a reserved higher version, that doesn't currently exist)

		//Extract the C_SIZE field from the response.  It is a 22-bit number in bit position 69:48.  This is different from v1.
		//It spans bytes 7, 8, and 9 of the response.
		c_size = (((DWORD)CSDResponse[7] & 0x3F) << 16) | ((WORD)CSDResponse[8] << 8) | CSDResponse[9];

		l_media_info.LastLBA = ((DWORD)(c_size + 1) * (WORD)(1024u)) - 1; //-1 on end is correction factor, since LBA = 0 is valid.
	}
	else //if(CSDResponse[0] & 0xC0)	//Check CSD_STRUCTURE field for v1 struct device
	{
		//Must be a v1 device.
		//Extract the C_SIZE field from the response.  It is a 12-bit number in bit position 73:62.
		//Although it is only a 12-bit number, it spans bytes 6, 7, and 8, since it isn't byte aligned.
		c_size = ((DWORD)CSDResponse[6] << 16) | ((WORD)CSDResponse[7] << 8) | CSDResponse[8];	//Get the bytes in the correct positions
		c_size &= 0x0003FFC0;	//Clear all bits that aren't part of the C_SIZE
		c_size = c_size >> 6;	//Shift value down, so the 12-bit C_SIZE is properly right justified in the DWORD.

		//Extract the C_SIZE_MULT field from the response.  It is a 3-bit number in bit position 49:47.
		c_size_mult = ((WORD)((CSDResponse[9] & 0x03) << 1)) | ((WORD)((CSDResponse[10] & 0x80) >> 7));

		//Extract the BLOCK_LEN field from the response. It is a 4-bit number in bit position 83:80.
		block_len = CSDResponse[5] & 0x0F;

		block_len = 1 << (block_len - 9); //-9 because we report the size in sectors of 512 bytes each

		//Calculate the MDD_SDSPI_finalLBA (see SD card physical layer simplified spec 2.0, section 5.3.2).
		//In USB mass storage applications, we will need this information to
		//correctly respond to SCSI get capacity requests (which will cause MDD_SDSPI_ReadCapacity() to get called).
		l_media_info.LastLBA = ((DWORD)(c_size + 1) * (WORD)((WORD)1 << (c_size_mult + 2)) * block_len) - 1;	//-1 on end is correction factor, since LBA = 0 is valid.
	}

	//Turn off CRC7 if we can, might be an invalid cmd on some cards (CMD59)
	//Note: POR default for the media is normally with CRC checking off in SPI
	//mode anyway, so this is typically redundant.
	SDCardSendCommand(CRC_ON_OFF,0x0);

	//Now set the block length to media sector size. It should be already set to this.
	SDCardSendCommand(SET_BLOCKLEN, SDCARD_BLOCK_SIZE);

  //Deselect media while not actively accessing the card.
  drvSDCCS(PIN_HIGH);

	l_media_info.Status = fileMS_OK;
	return true;
}

/** @brief Sends a command packet to the SD card.
 *
 *  Prepares a command packet and sends it out over the SPI interface.
 *  Response data of type 'R1' (as indicated by the SD/MMC product manual) is returned.
 *
 *  @param Command index to send
 *  @param Address to send
 *  @return The response from the card
 *           - Bit 0 - Idle state
 *           - Bit 1 - Erase Reset
 *           - Bit 2 - Illegal Command
 *           - Bit 3 - Command CRC Error
 *           - Bit 4 - Erase Sequence Error
 *           - Bit 5 - Address Error
 *           - Bit 6 - Parameter Error
 *           - Bit 7 - Unused. Always 0.
 */
SDCardCommandResponse SDCardSendCommand(SDCardCommandIndex in_command, uint32_t in_address)
{
	SDCardCommandResponse response;
  SDCardCommand  command_packet;
  uint16_t timeout;
  uint32_t long_timeout;

	//Select card
	drvSDCCS(PIN_LOW);

  // Copy over data
  command_packet.cmd        = l_command_table[in_command].CmdCode;
	command_packet.address    = in_address;
  command_packet.crc        = l_command_table[in_command].CRC;       // Calc CRC here

	command_packet.TRANSMIT_BIT = 1;             //Set Tranmission bit

	drvSDCSPISendAndReceiveByte(command_packet.cmd);                //Send Command
  drvSDCSPISendAndReceiveByte(command_packet.addr3);              //Most Significant Byte
  drvSDCSPISendAndReceiveByte(command_packet.addr2);
  drvSDCSPISendAndReceiveByte(command_packet.addr1);
  drvSDCSPISendAndReceiveByte(command_packet.addr0);              //Least Significant Byte
  drvSDCSPISendAndReceiveByte(command_packet.crc);                //Send CRC
  
  //Special handling for CMD12 (STOP_TRANSMISSION).  The very first byte after
  //sending the command packet may contain bogus non-0xFF data.  This
  //"residual data" byte should not be interpreted as the R1 response byte.
  if(in_command == STOP_TRANSMISSION)
  {
		drvSDCSPISendAndReceiveByte(0xff); //Perform dummy read to fetch the residual non R1 byte
  }
  
  //Loop until we get a response from the media.  Delay (NCR) could be up
  //to 8 SPI byte times.  First byte of response is always the equivalent of
  //the R1 byte, even for R1b, R2, R3, R7 responses.
  timeout = NCR_TIMEOUT;
  do
  {
		response.r1._byte = drvSDCSPISendAndReceiveByte(0xff);
		timeout--;
  }while((response.r1._byte == MMC_FLOATING_BUS) && (timeout != 0));
  
	//Check if we should read more bytes, depending upon the response type expected.
  if(l_command_table[in_command].ResponseType == R2)
  {
		response.r2._byte1 = response.r1._byte; //We already received the first byte, just make sure it is in the correct location in the struct.
		response.r2._byte0 = drvSDCSPISendAndReceiveByte(0xff); //Fetch the second byte of the response.
  }
  else if(l_command_table[in_command].ResponseType == R1b)
  {
		//Keep trying to read from the media, until it signals it is no longer
		//busy.  It will continuously send 0x00 bytes until it is not busy.
		//A non-zero value means it is ready for the next command.
		//The R1b response is received after a CMD12 STOP_TRANSMISSION
		//command, where the media card may be busy writing its internal buffer
		//to the flash memory.  This can typically take a few milliseconds,
		//with a recommended maximum timeout of 250ms or longer for SD cards.
		long_timeout = WRITE_TIMEOUT;
		do
		{
			response.r1._byte = drvSDCSPISendAndReceiveByte(0xff);
			long_timeout--;
		} while((response.r1._byte == 0x00) && (long_timeout != 0));
  
		response.r1._byte = 0x00;
  }
  else if (l_command_table[in_command].ResponseType == R7) //also used for response R3 type
  {
		//Fetch the other four bytes of the R3 or R7 response.
		//Note: The SD card argument response field is 32-bit, big endian format.
		//However, the C compiler stores 32-bit values in little endian in RAM.
		//When writing to the _returnVal/argument bytes, make sure the order it
		//gets stored in is correct.
		response.r7.bytewise.argument._byte3 = drvSDCSPISendAndReceiveByte(0xff);
		response.r7.bytewise.argument._byte2 = drvSDCSPISendAndReceiveByte(0xff);
		response.r7.bytewise.argument._byte1 = drvSDCSPISendAndReceiveByte(0xff);
		response.r7.bytewise.argument._byte0 = drvSDCSPISendAndReceiveByte(0xff);
  }

  drvSDCSPISendAndReceiveByte(0xff);  //Device requires at least 8 clock pulses after
																			//the response has been sent, before if can process
																			//the next command.  CS may be high or low.
  
	// see if we are expecting more data or not
  if(!(l_command_table[in_command].MoreDataExpected))
		drvSDCCS(PIN_HIGH);

	return response;
}

#ifdef drvSDC_CRC_ENABLED
//
// CRC Table : X16 + X12 + X5 + 1
//
static uint16_t l_SDCardCRCTable[256] =
{
    0x0000,0x1021,0x2042,0x3063,0x4084,0x50a5,0x60c6,0x70e7,
    0x8108,0x9129,0xa14a,0xb16b,0xc18c,0xd1ad,0xe1ce,0xf1ef,
    0x1231,0x0210,0x3273,0x2252,0x52b5,0x4294,0x72f7,0x62d6,
    0x9339,0x8318,0xb37b,0xa35a,0xd3bd,0xc39c,0xf3ff,0xe3de,
    0x2462,0x3443,0x0420,0x1401,0x64e6,0x74c7,0x44a4,0x5485,
    0xa56a,0xb54b,0x8528,0x9509,0xe5ee,0xf5cf,0xc5ac,0xd58d,
    0x3653,0x2672,0x1611,0x0630,0x76d7,0x66f6,0x5695,0x46b4,
    0xb75b,0xa77a,0x9719,0x8738,0xf7df,0xe7fe,0xd79d,0xc7bc,
    0x48c4,0x58e5,0x6886,0x78a7,0x0840,0x1861,0x2802,0x3823,
    0xc9cc,0xd9ed,0xe98e,0xf9af,0x8948,0x9969,0xa90a,0xb92b,
    0x5af5,0x4ad4,0x7ab7,0x6a96,0x1a71,0x0a50,0x3a33,0x2a12,
    0xdbfd,0xcbdc,0xfbbf,0xeb9e,0x9b79,0x8b58,0xbb3b,0xab1a,
    0x6ca6,0x7c87,0x4ce4,0x5cc5,0x2c22,0x3c03,0x0c60,0x1c41,
    0xedae,0xfd8f,0xcdec,0xddcd,0xad2a,0xbd0b,0x8d68,0x9d49,
    0x7e97,0x6eb6,0x5ed5,0x4ef4,0x3e13,0x2e32,0x1e51,0x0e70,
    0xff9f,0xefbe,0xdfdd,0xcffc,0xbf1b,0xaf3a,0x9f59,0x8f78,
    0x9188,0x81a9,0xb1ca,0xa1eb,0xd10c,0xc12d,0xf14e,0xe16f,
    0x1080,0x00a1,0x30c2,0x20e3,0x5004,0x4025,0x7046,0x6067,
    0x83b9,0x9398,0xa3fb,0xb3da,0xc33d,0xd31c,0xe37f,0xf35e,
    0x02b1,0x1290,0x22f3,0x32d2,0x4235,0x5214,0x6277,0x7256,
    0xb5ea,0xa5cb,0x95a8,0x8589,0xf56e,0xe54f,0xd52c,0xc50d,
    0x34e2,0x24c3,0x14a0,0x0481,0x7466,0x6447,0x5424,0x4405,
    0xa7db,0xb7fa,0x8799,0x97b8,0xe75f,0xf77e,0xc71d,0xd73c,
    0x26d3,0x36f2,0x0691,0x16b0,0x6657,0x7676,0x4615,0x5634,
    0xd94c,0xc96d,0xf90e,0xe92f,0x99c8,0x89e9,0xb98a,0xa9ab,
    0x5844,0x4865,0x7806,0x6827,0x18c0,0x08e1,0x3882,0x28a3,
    0xcb7d,0xdb5c,0xeb3f,0xfb1e,0x8bf9,0x9bd8,0xabbb,0xbb9a,
    0x4a75,0x5a54,0x6a37,0x7a16,0x0af1,0x1ad0,0x2ab3,0x3a92,
    0xfd2e,0xed0f,0xdd6c,0xcd4d,0xbdaa,0xad8b,0x9de8,0x8dc9,
    0x7c26,0x6c07,0x5c64,0x4c45,0x3ca2,0x2c83,0x1ce0,0x0cc1,
    0xef1f,0xff3e,0xcf5d,0xdf7c,0xaf9b,0xbfba,0x8fd9,0x9ff8,
    0x6e17,0x7e36,0x4e55,0x5e74,0x2e93,0x3eb2,0x0ed1,0x1ef0
};

//*****************************************************************************
//
//! \brief Get the direction and mode of a pin.
//!
//! \param pucSource is the data to ge CRC16.
//! \param usLen is the data length.
//!
//! \return Returns the CRC16 code.
//
//*****************************************************************************
static uint16_t SDCardCRC16Get(uint8_t *in_data, uint16_t in_length)
{
	uint16_t i;
	uint16_t result = 0;

	for (i = 0; i < in_length; i++)
	{
		result = (result << 8) ^ (uint16_t)l_SDCardCRCTable[(result >> 8) ^ in_data[i]];
	}

	return result;
}

#endif
