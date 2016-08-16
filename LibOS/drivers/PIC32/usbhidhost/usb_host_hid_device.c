/******************************************************************************

	usb_host_hid_device.c

	multiple HID device support

	mouse, keybord, joystick 

    2013/11/22    Suwa   http://www.suwa-koubou.jp

*******************************************************************************/

#include <string.h>
#include <stdlib.h>
#include "GenericTypeDefs.h"
#include "HardwareProfile.h"
#include "debug.h"
#include "usb_config.h"
#include "usb.h"
#include "usb_host_hid_parser.h"
#include "usb_host_hid.h"
#include "usb_host_hid_device.h"
#include "drvUSBHIDHost.h"
#include <sysUserInput.h>
#include <drvKeyboardScancodeMapper.h>

//#define	DEBUG_MODE


void ProcessKey(uint16_t in_key);

#define	putstr(x)	UART2PrintString(x)
#define	inkey()		UART2IsPressed()
#define	getch()		UART2GetChar()
#define	puthex(x)	UART2PutHex(x)
#define	putch(x)	UART2PutChar(x)
#define putdec(x)   UART2PutDec(x)
#define putcrlf()   UART2PrintString("\r\n")


// *** structures **************************************************************
typedef enum _APP_STATE
{
    DEVICE_NOT_CONNECTED = 0,
    DEVICE_CONNECTED,        /* Device Enumerated  - Report Descriptor Parsed */
    READY_TO_TX_RX_REPORT,
    GET_INPUT_REPORT,        /* perform operation on received report */
    INPUT_REPORT_PENDING,
    SEND_OUTPUT_REPORT,      /* Not needed in case of mouse */
    OUTPUT_REPORT_PENDING,
    ERROR_REPORTED 
} APP_STATE;


typedef struct _HID_REPORT_BUFFER	// mouse, keyboard, joystick etc.
{
    WORD  Report_ID;
    WORD  ReportSize;
    BYTE* ReportData;
    WORD  ReportPollRate;
}   HID_REPORT_BUFFER;


typedef struct _HID_LED_REPORT_BUFFER // keyboard only
{
    BYTE  NUM_LOCK      : 1;
    BYTE  CAPS_LOCK     : 1;
    BYTE  SCROLL_LOCK   : 1;
    BYTE  UNUSED        : 5;
}   HID_LED_REPORT_BUFFER;


// *** Macros ****************************************************************************
// item of report descripter
#define MINIMUM_POLL_INTERVAL           (0x0A)        // Minimum Polling rate for HID reports is 10ms
#define USAGE_PAGE_GEN_DESKTOP          (0x01)
#define USAGE_PAGE_BUTTONS              (0x09)
#define USAGE_PAGE_LEDS                 (0x08)
#define USAGE_PAGE_KEY_CODES            (0x07)
#define USAGE_MIN_MODIFIER_KEY          (0xE0)
#define USAGE_MAX_MODIFIER_KEY          (0xE7)
#define USAGE_MIN_NORMAL_KEY            (0x00)
#define USAGE_MAX_NORMAL_KEY            (0xFF)
#define HID_CAPS_LOCK_VAL               (0x39)
#define HID_NUM_LOCK_VAL                (0x53)
#define HID_SCROLL_LOCK_VAL             (0x47)

// Usage ID of Generic Desktop Page(0x01)
#define USAGE_POINTER                   (0x01)
#define USAGE_MOUSE                     (0x02)
#define USAGE_JOYSTICK                  (0x04)
#define USAGE_GAMEPAD                   (0x05)
#define	USAGE_KEYBOARD                  (0x06)
#define USAGE_KEYPAD                    (0x07)
#define USAGE_MULTI_AXIS                (0x08)
#define USAGE_TABLET                    (0x09)

#define MAX_DEVICES                     (USB_MAX_HID_DEVICES+2)   // deviceAddress= HUB PortNumber +1
#define MAX_ERROR_COUNTER               (10)

// HID Report Usages from HID Usage Tables 1.12 Section 4, Table 6
#define HID_USAGE_POINTER  0x01
#define HID_USAGE_MOUSE    0x02
#define HID_USAGE_JOYSTICK 0x04
#define HID_USAGE_KEYBOARD 0x06
#define HID_USAGE_X        0x30
#define HID_USAGE_Y        0x31
#define HID_USAGE_Z        0x32
#define HID_USAGE_RX       0x33
#define HID_USAGE_RY       0x34
#define HID_USAGE_RZ       0x35
#define HID_USAGE_WHEEL    0x38

#define JOY_MAX_AXES		6			// X, Y, Z, Rx, Ry, Rz
#define JOY_AXIS_X			0
#define JOY_AXIS_Y			1
#define JOY_AXIS_Z			2
#define JOY_AXIS_RX			3
#define JOY_AXIS_RY			4
#define JOY_AXIS_RZ			5

#ifndef USB_MALLOC
    #define USB_MALLOC(size) malloc(size)
#endif

#ifndef USB_FREE
    #define USB_FREE(ptr) free(ptr)
#endif

#ifndef USB_FREE_AND_CLEAR
    #define USB_FREE_AND_CLEAR(ptr) {USB_FREE(ptr); ptr = NULL;}
#endif

#define MODIFIER_LEFT_CONTROL           (0)
#define MODIFIER_LEFT_SHIFT             (1)
#define MODIFIER_LEFT_ALT               (2)
#define MODIFIER_LEFT_GUI               (3)
#define MODIFIER_RIGHT_CONTROL          (4)
#define MODIFIER_RIGHT_SHIFT            (5)
#define MODIFIER_RIGHT_ALT              (6)
#define MODIFIER_RIGHT_GUI              (7)

#define KEY_REPORT_DATA_LENGTH					(6)

#define MODIFIER_KEYS_COUNT							(8)

// *** Global Function Prototypes ********************************************************
BOOL USB_HIDDeviceEventHandler(BYTE address, USB_EVENT event, void *data, DWORD size);

// *** Private Function Prototypes *******************************************************
static int  searchDevice(BYTE device, int num);
static BOOL hid_device_read(BYTE deviceAddress);
static void setHIDDeviceType(BYTE deviceAddress, BYTE *rptDescriptor);

static void App_ProcessInputReport(BYTE deviceAddress);
static void App_MouseInputReport(BYTE deviceAddress, USB_MOUSE_DATA *data);
static void App_KeyboardInputReport(BYTE deviceAddress);
static void App_JoystickInputReport(BYTE deviceAddress);

static void App_PrepareOutputReport(BYTE deviceAddress);

static BOOL USB_HID_DataCollectionHandler(BYTE deviceAddress);
static BOOL USB_HID_MOUSE_DataCollectionHandler(BYTE deviceAddress);
static BOOL USB_HID_KEYBOARD_DataCollectionHandler(BYTE deviceAddress);
static BOOL USB_HID_JOYSTICK_DataCollectionHandler(BYTE deviceAddress);


// ******************************************************************************************
// ******************************************************************************************
// Section: Global Variables
// ******************************************************************************************
// ******************************************************************************************

// HID device name
static char *deviceName[HID_MAX_DEVICETYPE] = {
	"unknown", "mouse", "joystick",
    "gamepad", "keyboard", "keypad",
};

// number of connecting device
static int numOfDevice[HID_MAX_DEVICETYPE];

// *** each device ******************* 
static APP_STATE             App_State[MAX_DEVICES]; // Initial value is DEVICE_NOT_CONNECTED.
static BYTE                  deviceType[MAX_DEVICES]; // Initial value is HID_UNKNOWN	
static HID_REPORT_BUFFER     Appl_raw_report_buffer[MAX_DEVICES];
static BYTE                  ErrorDriver[MAX_DEVICES];
static BYTE                  ErrorCounter[MAX_DEVICES];
static BYTE                  NumOfBytesRcvd[MAX_DEVICES];
static BOOL                  ReportBufferUpdated[MAX_DEVICES];

// *** mouse,joystick *******************  
static USB_JOYSTICK_DATA	l_last_joystick_data[MAX_DEVICES];
static HID_DATA_DETAILS      Appl_Axis_Details[MAX_DEVICES][JOY_MAX_AXES];
static HID_DATA_DETAILS      Appl_Buttons_Details[MAX_DEVICES];
static HID_DATA_DETAILS      Appl_Wheel_Axis_Details[MAX_DEVICES];
static HID_DATA_DETAILS      Appl_HatSwitch_Details[MAX_DEVICES];



static HID_USER_DATA_SIZE    Appl_Button_report_buffer[12];   // Joystick has max 12 buttons. 
static HID_USER_DATA_SIZE    Appl_XY_report_buffer[4];        // Jpystick has 4 stick as X,Y,Rz,Z.
static HID_USER_DATA_SIZE    Appl_Wheel_report_buffer[1];     // wheel
static HID_USER_DATA_SIZE    Appl_HatSwitch_report_buffer[1]; // Jpystick has 1 HAT Switch.

// *** keyboards *************** 
static HID_DATA_DETAILS      Appl_ModifierKeysDetails[MAX_DEVICES];
static HID_DATA_DETAILS      Appl_NormalKeysDetails[MAX_DEVICES];
static HID_DATA_DETAILS      Appl_LED_Indicator[MAX_DEVICES];
//static HID_USER_DATA_SIZE    Appl_BufferModifierKeys[8];
//static HID_USER_DATA_SIZE    Appl_BufferNormalKeys[6];
static HID_LED_REPORT_BUFFER Appl_led_report_buffer[MAX_DEVICES];
static BOOL                  LED_Key_Pressed[MAX_DEVICES];
static BYTE                  CAPS_Lock_Pressed[MAX_DEVICES];
static BYTE                  NUM_Lock_Pressed[MAX_DEVICES];
static BYTE                  SCROLL_Lock_Pressed[MAX_DEVICES];
static HID_USER_DATA_SIZE Appl_BufferModifierKeys[MODIFIER_KEYS_COUNT];
static HID_USER_DATA_SIZE Appl_BufferPreviousModifierKeys[MAX_DEVICES][MODIFIER_KEYS_COUNT];
static HID_USER_DATA_SIZE Appl_BufferNormalKeys[KEY_REPORT_DATA_LENGTH];
static HID_USER_DATA_SIZE Appl_BufferPreviousKeys[MAX_DEVICES][KEY_REPORT_DATA_LENGTH];

// ******************************************************************************************
// ******************************************************************************************
// Section: Application Callable Functions
// ******************************************************************************************
// ******************************************************************************************

/*============================================================================================
	int getHIDnumOfDevice(BYTE devicetype)
============================================================================================*/

int getHIDnumOfDevice(BYTE devicetype)
{
	if(devicetype >= HID_MAX_DEVICETYPE) return 0;
	return numOfDevice[devicetype];
}


/*============================================================================================
	int devicename_read(int num, USB_DEVICENAME_DATA *data)

	first argumnet 'num' is number of connecting mouse 
	return code = 1  read complete
                = 0  not ready, read pending
                =-1  device not connected
============================================================================================*/

int mouse_read(int num, USB_MOUSE_DATA *data)
{
	int adr;

	memset(data, 0, sizeof(USB_MOUSE_DATA));
	if((adr = searchDevice(HID_MOUSE, num)) != -1){
		if(hid_device_read(adr)) return 1;
		else return 0;
	}else{
		return -1;
	}
}


int keyboard_read(int num)
{
	int adr;

	if((adr = searchDevice(HID_KEYBOARD, num)) != -1){
		if(hid_device_read(adr)) return 1;
		else return 0;
	}else{
		return -1;
	}
}

int joystick_read(int num)
{
	int adr;

	if((adr = searchDevice(HID_JOYSTICK, num)) != -1){
		if(hid_device_read(adr)) return 1;
		else return 0;
	}else{
		return -1;
	}
}


// ******************************************************************************************
// ******************************************************************************************
// Section: HID Device Event Handler
// ******************************************************************************************
// ******************************************************************************************

BOOL USB_HIDDeviceEventHandler ( BYTE address, USB_EVENT event, void *data, DWORD size )
{

    // Handle specific events.
    switch (event){

    case EVENT_HID_ATTACH:
        DEBUG_PutString("\r\n"); DEBUG_PutString(deviceName[deviceType[address]]); DEBUG_PutString(" attached.\r\n");
        DEBUG_PutString(" device address="); DEBUG_PutDec(address); DEBUG_PutString("\r\n");

		// update numOfDevice
		numOfDevice[deviceType[address]]++;
        break;

    case EVENT_HID_DETACH:
        DEBUG_PutString("\r\n"); DEBUG_PutString(deviceName[deviceType[address]]); DEBUG_PutString(" detached.\r\n");
        DEBUG_PutString(" device address="); DEBUG_PutDec(address); DEBUG_PutString("\r\n");

		// update numOfDevice
		numOfDevice[deviceType[address]]--;
		if(Appl_raw_report_buffer[address].ReportData){
			USB_FREE_AND_CLEAR(Appl_raw_report_buffer[address].ReportData);
		}
        deviceType[address] = HID_UNKNOWN;
        break;

	case EVENT_HID_READ_DONE:
	case EVENT_HID_WRITE_DONE:
	case EVENT_HID_BAD_REPORT_DESCRIPTOR:
		break;

    case EVENT_HID_RPT_DESC_PARSED:
        DEBUG_PutString( "\r\nHID: device report descriptor parsed\r\n" );
        setHIDDeviceType(address, (BYTE *)data);
        return USB_HID_DataCollectionHandler(address);

    default:
       break;
   }

   return TRUE;

} // USB_HIDDeviceEventHandler


// ******************************************************************************************
// ******************************************************************************************
// Section: Private Functions
// ******************************************************************************************
// ******************************************************************************************

/*===========================================================================================
	searchDevice(BYTE device, int num)
===========================================================================================*/

static int searchDevice(BYTE device, int num)
{
	int i;

	// search device
	for(i = 0; i < MAX_DEVICES; i++){
		if(deviceType[i] == device){
			if(--num == 0){
				return i;	// return value is deviceAddress.
			}
		}
	}
	return -1;
}

/*============================================================================================
	hid_device_read() 

	result code TRUE   read complete
								FALSE  do not complete, unready or read pending

	void *data
					case mouse     USB_MOUSE_DATA    *data
					case keyboard  USB_KEYBOARD_DATA *data
					case other     USB_XXXX_DATA     *data
============================================================================================*/

static BOOL hid_device_read(BYTE deviceAddress)
{
	// call usb host layer
	USBTasks();

	// Was device detached?
	// Or, was report descriptor parsed with error.?
	if (!USBHostHIDDeviceDetect(deviceAddress))
	{
		App_State[deviceAddress] = DEVICE_NOT_CONNECTED;
	}

	switch (App_State[deviceAddress])
	{
		case DEVICE_NOT_CONNECTED:
			USBTasks();
			/* True if report descriptor is parsed with no error */
			if (USBHostHIDDeviceDetect(deviceAddress))
			{
				App_State[deviceAddress] = DEVICE_CONNECTED;
			}
			break;

		case DEVICE_CONNECTED:
			App_State[deviceAddress] = READY_TO_TX_RX_REPORT;
			break;

		case READY_TO_TX_RX_REPORT:
			if (USBHostHIDDeviceDetect(deviceAddress))
			{
				App_State[deviceAddress] = GET_INPUT_REPORT;
			}
			else
			{
				App_State[deviceAddress] = DEVICE_NOT_CONNECTED;
			}
			break;

		case GET_INPUT_REPORT:
			if (!USBHostHIDRead(deviceAddress, Appl_raw_report_buffer[deviceAddress].Report_ID, 0,
													Appl_raw_report_buffer[deviceAddress].ReportSize, Appl_raw_report_buffer[deviceAddress].ReportData))
			{
				App_State[deviceAddress] = INPUT_REPORT_PENDING;
			}
			break;

		case INPUT_REPORT_PENDING:
			if (USBHostHIDTransferIsComplete(deviceAddress, &ErrorDriver[deviceAddress], &NumOfBytesRcvd[deviceAddress]))
			{
				if (ErrorDriver[deviceAddress] || (NumOfBytesRcvd[deviceAddress] != Appl_raw_report_buffer[deviceAddress].ReportSize ))
				{
					ErrorCounter[deviceAddress]++ ;
					if (MAX_ERROR_COUNTER <= ErrorDriver[deviceAddress])
					{
						App_State[deviceAddress] = ERROR_REPORTED;
					}
					else
					{
						App_State[deviceAddress] = READY_TO_TX_RX_REPORT;
					}
				}
				else
				{
					ErrorCounter[deviceAddress] = 0;
					ReportBufferUpdated[deviceAddress] = TRUE;
					App_State[deviceAddress] = READY_TO_TX_RX_REPORT;

					App_ProcessInputReport(deviceAddress);
					App_PrepareOutputReport(deviceAddress);
					return 	TRUE;
				}
			}
			break;

		case SEND_OUTPUT_REPORT: /* Will be done while implementing Keyboard */
			if (!USBHostHIDWrite(deviceAddress, Appl_LED_Indicator[deviceAddress].reportID, Appl_LED_Indicator[deviceAddress].interfaceNum, Appl_LED_Indicator[deviceAddress].reportLength,
													 (BYTE*) & Appl_led_report_buffer[deviceAddress]))
			{
				App_State[deviceAddress] = OUTPUT_REPORT_PENDING;
			}
			break;

		case OUTPUT_REPORT_PENDING:
			if (USBHostHIDTransferIsComplete(deviceAddress, &ErrorDriver[deviceAddress], &NumOfBytesRcvd[deviceAddress]))
			{
				if (ErrorDriver[deviceAddress])
				{
					ErrorCounter[deviceAddress]++ ;
					if (MAX_ERROR_COUNTER <= ErrorDriver[deviceAddress])
					{
						App_State[deviceAddress] = ERROR_REPORTED;
					}
				}
				else
				{
					ErrorCounter[deviceAddress] = 0;
					App_State[deviceAddress] = READY_TO_TX_RX_REPORT;
				}
			}
			break;

		case ERROR_REPORTED:	// forever error loop
			break;

		default:
			break;
	}

	// ReportData isn't ready!
	return FALSE;
}


/*----------------------------------------------------------------------------------
    void App_ProcessInputReport()
----------------------------------------------------------------------------------*/

static void App_ProcessInputReport(BYTE deviceAddress)
{

	switch(deviceType[deviceAddress]){
	case HID_MOUSE:
		//App_MouseInputReport(deviceAddress, (USB_MOUSE_DATA *)data);
		break;

	case HID_KEYBOARD:
		App_KeyboardInputReport(deviceAddress);
		break;

	case HID_JOYSTICK:
		App_JoystickInputReport(deviceAddress);
		break;
	}
}


static void App_MouseInputReport(BYTE deviceAddress, USB_MOUSE_DATA *data)
{

   /* process input report received from device */
    USBHostHID_ApiImportData(Appl_raw_report_buffer[deviceAddress].ReportData, Appl_raw_report_buffer[deviceAddress].ReportSize
                          ,Appl_Button_report_buffer, &Appl_Buttons_Details[deviceAddress]);
 /*/   USBHostHID_ApiImportData(Appl_raw_report_buffer[deviceAddress].ReportData, Appl_raw_report_buffer[deviceAddress].ReportSize
                          ,Appl_XY_report_buffer, &Appl_XY_Axis_Details[deviceAddress]);*/

    USBHostHID_ApiImportData(Appl_raw_report_buffer[deviceAddress].ReportData, Appl_raw_report_buffer[deviceAddress].ReportSize
                          ,Appl_Wheel_report_buffer, &Appl_Wheel_Axis_Details[deviceAddress]);

	#ifdef DEBUG_MODE
	{
		int i;
		putstr( "HID: Raw Report \r\n");
		putstr("ReportSize="); putdec(Appl_raw_report_buffer[deviceAddress].ReportSize);
		for(i=0;i<(Appl_raw_report_buffer[deviceAddress].ReportSize);i++){
			puthex(Appl_raw_report_buffer[deviceAddress].ReportData[i]);
		}
    	putstr("\r\n");
		putstr("Left  Bt : "); putdec(Appl_Button_report_buffer[0]);putstr("\r\n");
		putstr("Right Bt : "); putdec(Appl_Button_report_buffer[1]);putstr("\r\n");
		putstr("CenterBt : "); putdec(Appl_Button_report_buffer[2]);putstr("\r\n");
    
		putstr("X-Axis : "); putdec((char)Appl_XY_report_buffer[0]);putstr("\r\n");
		putstr("Y-Axis : "); putdec((char)Appl_XY_report_buffer[1]);putstr("\r\n");
		putstr("Z-Axis : "); putdec((char)Appl_Wheel_report_buffer[0]);putstr("\r\n");
	}
	#endif

	data->button.left = Appl_Button_report_buffer[0] & 1;
	data->button.right = Appl_Button_report_buffer[1] & 1;
	data->button.wheel = Appl_Button_report_buffer[2] & 1;
	data->x_axis = (char)Appl_XY_report_buffer[0];
	data->y_axis = (char)Appl_XY_report_buffer[1];
	data->wheel = (char)Appl_Wheel_report_buffer[0];
}


static void App_KeyboardInputReport(BYTE deviceAddress)
{
	int i, j;
	BYTE modifier_keys;
	BYTE pressed_keys_count;
	BYTE lock_keys;
	BOOL pressed;
	WORD key_code;

  /* process input report received from device */
  USBHostHID_ApiImportData(Appl_raw_report_buffer[deviceAddress].ReportData, Appl_raw_report_buffer[deviceAddress].ReportSize
													,Appl_BufferModifierKeys, &Appl_ModifierKeysDetails[deviceAddress]);
  USBHostHID_ApiImportData(Appl_raw_report_buffer[deviceAddress].ReportData, Appl_raw_report_buffer[deviceAddress].ReportSize
													,Appl_BufferNormalKeys, &Appl_NormalKeysDetails[deviceAddress]);

	// process modifier keys
	modifier_keys = 0;
	if(Appl_BufferModifierKeys[MODIFIER_LEFT_CONTROL] != 0)
		modifier_keys |= sysVKC_MF_LEFT_CONTROL;

	if(Appl_BufferModifierKeys[MODIFIER_LEFT_SHIFT] != 0)
		modifier_keys |= sysVKC_MF_LEFT_SHIFT;

	if(Appl_BufferModifierKeys[MODIFIER_LEFT_ALT] != 0)
		modifier_keys |= sysVKC_MF_LEFT_ALT;

	if(Appl_BufferModifierKeys[MODIFIER_LEFT_GUI] != 0)
		modifier_keys |= sysVKC_MF_LEFT_GUI;

	if(Appl_BufferModifierKeys[MODIFIER_RIGHT_CONTROL] != 0)
		modifier_keys |= sysVKC_MF_RIGHT_CONTROL;

	if(Appl_BufferModifierKeys[MODIFIER_RIGHT_SHIFT] != 0)
		modifier_keys |= sysVKC_MF_RIGHT_SHIFT;

	if(Appl_BufferModifierKeys[MODIFIER_RIGHT_ALT] != 0)
		modifier_keys |= sysVKC_MF_RIGHT_ALT;

	if(Appl_BufferModifierKeys[MODIFIER_RIGHT_GUI] != 0)
		modifier_keys |= sysVKC_MF_RIGHT_GUI;


	#ifdef DEBUG_MODE
	{
		int i;
    	putstr( "HID: Raw Report \r\n");
		putstr("ReportSize="); putdec(Appl_raw_report_buffer[deviceAddress].ReportSize);putstr("\r\n");
    	for(i=0;i<(Appl_raw_report_buffer[deviceAddress].ReportSize);i++){
    		puthex( Appl_raw_report_buffer[deviceAddress].ReportData[i]);
    	}
    	putstr("\r\n");
	}
	#endif


	//TODO UpdateModifierKeys(modifier_keys);

	// update keys
	pressed_keys_count = 0;
	lock_keys = 0;
	for (i = 0; i < KEY_REPORT_DATA_LENGTH; i++)
	{
		// pressed keys
		if (Appl_BufferNormalKeys[i] != 0)
		{
			pressed_keys_count++;

			switch (Appl_BufferNormalKeys[i])
			{
					// CAPS lock
				case HID_CAPS_LOCK_VAL:
					CAPS_Lock_Pressed[deviceAddress] = !CAPS_Lock_Pressed[deviceAddress];
					LED_Key_Pressed[deviceAddress] = TRUE;
					Appl_led_report_buffer[deviceAddress].CAPS_LOCK = CAPS_Lock_Pressed[deviceAddress];
					if(CAPS_Lock_Pressed[deviceAddress])
						lock_keys |= sysVKC_LS_CAPS_LOCK;
					break;

					// NUM lock
				case HID_NUM_LOCK_VAL:
					NUM_Lock_Pressed[deviceAddress] = !NUM_Lock_Pressed[deviceAddress];
					LED_Key_Pressed[deviceAddress] = TRUE;
					Appl_led_report_buffer[deviceAddress].NUM_LOCK = NUM_Lock_Pressed[deviceAddress];
					if(NUM_Lock_Pressed[deviceAddress])
						lock_keys |= sysVKC_LS_NUM_LOCK;
					break;

				case HID_SCROLL_LOCK_VAL:
					SCROLL_Lock_Pressed[deviceAddress] = !SCROLL_Lock_Pressed[deviceAddress];
					LED_Key_Pressed[deviceAddress] = TRUE;
					Appl_led_report_buffer[deviceAddress].SCROLL_LOCK = SCROLL_Lock_Pressed[deviceAddress];
					if(SCROLL_Lock_Pressed[deviceAddress])
						lock_keys |= sysVKC_LS_SCROLL_LOCK;
					break;

					// any other key
				default:
					// check if it was pressed earlier
					pressed = TRUE;
					for (j = 0; j < KEY_REPORT_DATA_LENGTH && pressed; j++)
					{
						if (Appl_BufferNormalKeys[i] == Appl_BufferPreviousKeys[deviceAddress][j])
							pressed = FALSE;
					}

					// it appears first in the current report -> key was pressed
					if (pressed)
					{
						key_code = drvScancodeToKeycode(Appl_BufferNormalKeys[i], modifier_keys, lock_keys);
						if(key_code != sysVKC_INVALID)
							sysUserInputEventHandler(deviceAddress, sysUIEC_Pressed, sysUIET_Key, key_code );
					}
					break;
			}
		}
		else
		{
			// no more keys to process
			break;
		}
	}

	// process released keys
	for (i = 0; i < KEY_REPORT_DATA_LENGTH; i++)
	{
		if(Appl_BufferPreviousKeys[i] != 0)
		{
			pressed = FALSE;
			for(j=0;j<KEY_REPORT_DATA_LENGTH && !pressed;j++)
			{
				if(Appl_BufferPreviousKeys[deviceAddress][i] == Appl_BufferNormalKeys[j])
					pressed = TRUE;
			}

			if(!pressed)
			{
				// key was released
				key_code = drvScancodeToKeycode(Appl_BufferPreviousKeys[deviceAddress][i], modifier_keys, lock_keys);
			}
		}
		else
		{
			break;
		}
	}

	// just for sure clear matrix when no key is pressed
	//TODO if(modifier_keys == 0 && pressed_keys_count == 0)
	//TODO	ClearKeyboardMatrix();

	// copy report data to the previous buffer
	for (i = 0; i < KEY_REPORT_DATA_LENGTH; i++)
		Appl_BufferPreviousKeys[deviceAddress][i] = Appl_BufferNormalKeys[i];

	// clear report buffer
	for (i = 0; i < KEY_REPORT_DATA_LENGTH; i++)
		Appl_BufferNormalKeys[i] = 0;

	for (i = 0; i < Appl_raw_report_buffer[deviceAddress].ReportSize; i++)
		Appl_raw_report_buffer[deviceAddress].ReportData[i] = 0;

#if 0

	// modifier
	*((BYTE *)&(data->modifier)) = Appl_raw_report_buffer[deviceAddress].ReportData[0];
	// keycode
	memcpy(data->keycode, &(Appl_raw_report_buffer[deviceAddress].ReportData[2]), 3);

	// check CapsLock NumericLock
	for(i=2;i<(Appl_raw_report_buffer[deviceAddress].ReportSize);i++) 
	{
		if(Appl_raw_report_buffer[deviceAddress].ReportData[i] != 0)
		{
            if(Appl_raw_report_buffer[deviceAddress].ReportData[i] == HID_CAPS_LOCK_VAL)
            {
               CAPS_Lock_Pressed[deviceAddress] = !CAPS_Lock_Pressed[deviceAddress];
               LED_Key_Pressed[deviceAddress] = TRUE;
               Appl_led_report_buffer[deviceAddress].CAPS_LOCK = CAPS_Lock_Pressed[deviceAddress];

            }
            else if(Appl_raw_report_buffer[deviceAddress].ReportData[i] == HID_NUM_LOCK_VAL)
            {
                NUM_Lock_Pressed[deviceAddress] = !NUM_Lock_Pressed[deviceAddress];
                LED_Key_Pressed[deviceAddress] = TRUE;
                Appl_led_report_buffer[deviceAddress].NUM_LOCK = NUM_Lock_Pressed[deviceAddress];
            }
		}
	}

	if(CAPS_Lock_Pressed[deviceAddress]) data->modifier.capslock=1;
	else  data->modifier.capslock=0;

	if(NUM_Lock_Pressed[deviceAddress])  data->modifier.numlock=1;
	else  data->modifier.numlock=0;

	// clear receive buffer
    for(i=0;i<Appl_raw_report_buffer[deviceAddress].ReportSize;i++)
    {
        Appl_raw_report_buffer[deviceAddress].ReportData[i] = 0;
    }
#endif
}
static void App_JoystickInputReport(BYTE deviceAddress)
{
	int   n;
	WORD  btn;
	HID_USER_DATA_SIZE data_buffer;
	drvUSBHIDHostEventCategory event_category;

	/* process input report received from device */

	// process X axis
	USBHostHID_ApiImportData(Appl_raw_report_buffer[deviceAddress].ReportData, Appl_raw_report_buffer[deviceAddress].ReportSize
													 , &data_buffer, &Appl_Axis_Details[deviceAddress][JOY_AXIS_X]);


	if (l_last_joystick_data[deviceAddress].x_axis != data_buffer)
	{
		// generate event
		if ( data_buffer == 0x7f)
			event_category = drvUHHEC_Released;
		else
			event_category = drvUHHEC_Pressed;

		sysUserInputEventHandler(deviceAddress, event_category, drvUHHET_JoyXAxis, data_buffer);

		// update previous value
		l_last_joystick_data[deviceAddress].x_axis = data_buffer;
	}

	// process Y axis
	USBHostHID_ApiImportData(Appl_raw_report_buffer[deviceAddress].ReportData, Appl_raw_report_buffer[deviceAddress].ReportSize
													 , &data_buffer, &Appl_Axis_Details[deviceAddress][JOY_AXIS_Y]);

	if (l_last_joystick_data[deviceAddress].y_axis != data_buffer)
	{
		// generate event
		if ( data_buffer == 0x7f)
			event_category = drvUHHEC_Released;
		else
			event_category = drvUHHEC_Pressed;

		sysUserInputEventHandler(deviceAddress, event_category, drvUHHET_JoyYAxis, data_buffer);

		// update previous value
		l_last_joystick_data[deviceAddress].y_axis = data_buffer;
	}



	USBHostHID_ApiImportData(Appl_raw_report_buffer[deviceAddress].ReportData, Appl_raw_report_buffer[deviceAddress].ReportSize
													 , Appl_Button_report_buffer, &Appl_Buttons_Details[deviceAddress]);


	USBHostHID_ApiImportData(Appl_raw_report_buffer[deviceAddress].ReportData, Appl_raw_report_buffer[deviceAddress].ReportSize
													 , Appl_HatSwitch_report_buffer, &Appl_HatSwitch_Details[deviceAddress]);

#ifdef DEBUG_MODE
	{
		int i;
		putstr( "HID: Raw Report \r\n");
		putstr(" Size=");
		putdec(Appl_raw_report_buffer[deviceAddress].ReportSize);
		putstr("\r\n Data=");
		for (i = 0; i < (Appl_raw_report_buffer[deviceAddress].ReportSize); i++)
		{
			puthex(Appl_raw_report_buffer[deviceAddress].ReportData[i]);
		}
		putcrlf();
	}
#endif

	/***
data->button.btn1 = Appl_Button_report_buffer[0] & 1;
data->button.btn2 = Appl_Button_report_buffer[1] & 1;
data->button.btn3 = Appl_Button_report_buffer[2] & 1;
data->button.btn4 = Appl_Button_report_buffer[3] & 1;
data->button.btn5 = Appl_Button_report_buffer[4] & 1;
data->button.btn6 = Appl_Button_report_buffer[5] & 1;
data->button.btn7 = Appl_Button_report_buffer[6] & 1;
data->button.btn8 = Appl_Button_report_buffer[7] & 1;
data->button.btn9 = Appl_Button_report_buffer[8] & 1;
data->button.btn10 = Appl_Button_report_buffer[9] & 1;
data->button.btn11 = Appl_Button_report_buffer[10] & 1;
data->button.btn12 = Appl_Button_report_buffer[11] & 1;
	 ***/
	for (btn = n = 0; n < 12; n++)
	{
		btn |= ((Appl_Button_report_buffer[n] & 1) << n);
	}
	//*((WORD *)&(data->button)) = btn;

	//data->x_axis = Appl_XY_report_buffer[0];
	//data->y_axis = Appl_XY_report_buffer[1];
	//data->z_axis = Appl_XY_report_buffer[2];
	//data->rz_axis = Appl_XY_report_buffer[3];
	//data->hatswitch = Appl_HatSwitch_report_buffer[0];
}


/*---------------------------------------------------------------------------
    void App_PrepareOutputReport()
	    Now, only support for keyboard
---------------------------------------------------------------------------*/

static void App_PrepareOutputReport(BYTE deviceAddress)
{
	switch(deviceType[deviceAddress]){
	case HID_KEYBOARD:
	    if(ReportBufferUpdated[deviceAddress] == TRUE)
    	{
        	ReportBufferUpdated[deviceAddress] = FALSE;
        	if(LED_Key_Pressed[deviceAddress])
        	{
            	App_State[deviceAddress] = SEND_OUTPUT_REPORT;
            	LED_Key_Pressed[deviceAddress] = FALSE;
        	}
     	}
		break;
	}

}


/*----------------------------------------------------------------------------------
	setHIDDeviceType(BYTE deviceAddress);
----------------------------------------------------------------------------------*/

static void setHIDDeviceType(BYTE deviceAddress, BYTE *rptDescriptor)
{
	static BYTE typeTable[] = {
		HID_UNKNOWN,	// 0 
		HID_UNKNOWN,    // 1 USAGE_POINTER
		HID_MOUSE,      // 2 USAGE_MOUSE
		HID_UNKNOWN,    // 3
		HID_JOYSTICK,   // 4 USAGE_JOYSTICK
		HID_GAMEPAD,    // 5 USAGE_GAMEPAD
		HID_KEYBOARD,	// 6 USAGE_KEYBOARD
		HID_KEYPAD,     // 7 USAGE_KEYPAD
	};
	BYTE usagePage;
	BYTE usageId;

	usagePage = rptDescriptor[1];
	usageId = rptDescriptor[3];

	// already set
	if(deviceType[deviceAddress]!=HID_UNKNOWN) return;

	if( usagePage == USAGE_PAGE_GEN_DESKTOP && 
		(usageId >= USAGE_MOUSE && usageId <= USAGE_KEYPAD) ){
		deviceType[deviceAddress] = typeTable[usageId];
	}else{
		deviceType[deviceAddress] = HID_UNKNOWN;
	}
}



/*----------------------------------------------------------------------------------
	USB_HID_DataCollectionHandler()

	macro in usb_config.h
	#define APPL_COLLECT_PARSED_DATA USB_HID_DataCollectionHandler

	This function was called in USB_ApplicationEventHandler function.
----------------------------------------------------------------------------------*/

static BOOL USB_HID_DataCollectionHandler(BYTE deviceAddress)
{
	switch(deviceType[deviceAddress]){
	case HID_MOUSE:
		#ifdef DEBUG_MODE
		putstr("Mouse DataCollection.\r\n");
		#endif
		return USB_HID_MOUSE_DataCollectionHandler(deviceAddress);

	case HID_KEYBOARD:
		#ifdef DEBUG_MODE
		putstr("Keyboard DataCollection.\r\n");
		#endif
		return USB_HID_KEYBOARD_DataCollectionHandler(deviceAddress);

	case HID_JOYSTICK:
		#ifdef DEBUG_MODE
		putstr("Joystick DataCollection.\r\n");
		#endif
		return USB_HID_JOYSTICK_DataCollectionHandler(deviceAddress);

	default: // other
		#ifdef DEBUG_MODE
		putstr("Unsupported device.\r\n");
		#endif
		return FALSE;
	}
}


static BOOL USB_HID_MOUSE_DataCollectionHandler(BYTE deviceAddress)
{
  BYTE                      NumOfReportItem;
  BYTE                      i;
  USB_HID_ITEM_LIST        *pitemListPtrs;
  USB_HID_DEVICE_RPT_INFO  *pDeviceRptinfo;
  HID_REPORTITEM           *reportItem;
  HID_USAGEITEM            *hidUsageItem;
  BYTE                      usageIndex;
  BYTE                      reportIndex;
  BYTE	                    xy_flg, wheel_flg;
  BOOL                      status;

  pDeviceRptinfo = USBHostHID_GetCurrentReportInfo(); // Get current Report Info pointer
  pitemListPtrs = USBHostHID_GetItemListPointers();   // Get pointer to list of item pointers

  xy_flg = wheel_flg = 0;
  reportIndex = 0;

  status = FALSE;
  NumOfReportItem = pDeviceRptinfo->reportItems;
#if 0
  for(i=0;i<NumOfReportItem;i++)
  {
    reportItem = &pitemListPtrs->reportItemList[i];
    if((reportItem->reportType==hidReportInput) && (reportItem->dataModes == (HIDData_Variable|HIDData_Relative))&&
           (reportItem->globals.usagePage==USAGE_PAGE_GEN_DESKTOP))
    {
      if(xy_flg == 0)
      {

        usageIndex = reportItem->firstUsageItem;
        hidUsageItem = &pitemListPtrs->usageItemList[usageIndex];

        reportIndex = reportItem->globals.reportIndex;
        Appl_XY_Axis_Details[deviceAddress].reportLength = (pitemListPtrs->reportList[reportIndex].inputBits + 7)/8;
        Appl_XY_Axis_Details[deviceAddress].reportID = (BYTE)reportItem->globals.reportID;
        Appl_XY_Axis_Details[deviceAddress].bitOffset = (BYTE)reportItem->startBit;
        Appl_XY_Axis_Details[deviceAddress].bitLength = (BYTE)reportItem->globals.reportsize;
        Appl_XY_Axis_Details[deviceAddress].count=(BYTE)reportItem->globals.reportCount;
        Appl_XY_Axis_Details[deviceAddress].interfaceNum= USBHostHID_ApiGetCurrentInterfaceNum();

		xy_flg = 1;	// SUWA

        if(Appl_XY_Axis_Details[deviceAddress].count==3)	// include wheel
        {
	      Appl_XY_Axis_Details[deviceAddress].count = 2;
	      Appl_Wheel_Axis_Details[deviceAddress].reportLength = Appl_XY_Axis_Details[deviceAddress].reportLength;
	      Appl_Wheel_Axis_Details[deviceAddress].reportID = Appl_XY_Axis_Details[deviceAddress].reportID;
	      Appl_Wheel_Axis_Details[deviceAddress].bitOffset = Appl_XY_Axis_Details[deviceAddress].bitOffset + Appl_XY_Axis_Details[deviceAddress].bitLength * Appl_XY_Axis_Details[deviceAddress].count;
          Appl_Wheel_Axis_Details[deviceAddress].bitLength = Appl_XY_Axis_Details[deviceAddress].bitLength;
          Appl_Wheel_Axis_Details[deviceAddress].count=1;
          Appl_Wheel_Axis_Details[deviceAddress].interfaceNum= Appl_XY_Axis_Details[deviceAddress].interfaceNum;
          wheel_flg = 1;
        }

        continue;

      } // if(xy_flg)

      if(wheel_flg == 0)
      {
        usageIndex = reportItem->firstUsageItem;
        hidUsageItem = &pitemListPtrs->usageItemList[usageIndex];

        reportIndex = reportItem->globals.reportIndex;
        Appl_Wheel_Axis_Details[deviceAddress].reportLength = (pitemListPtrs->reportList[reportIndex].inputBits + 7)/8;
        Appl_Wheel_Axis_Details[deviceAddress].reportID = (BYTE)reportItem->globals.reportID;
        Appl_Wheel_Axis_Details[deviceAddress].bitOffset = (BYTE)reportItem->startBit;
        Appl_Wheel_Axis_Details[deviceAddress].bitLength = (BYTE)reportItem->globals.reportsize;
        Appl_Wheel_Axis_Details[deviceAddress].count= 1;
        Appl_Wheel_Axis_Details[deviceAddress].interfaceNum= USBHostHID_ApiGetCurrentInterfaceNum();
        wheel_flg = 1;
      }

    }
    else if((reportItem->reportType==hidReportInput) && (reportItem->dataModes == HIDData_Variable)&&
           (reportItem->globals.usagePage==USAGE_PAGE_BUTTONS))
    {
      usageIndex = reportItem->firstUsageItem;
      hidUsageItem = &pitemListPtrs->usageItemList[usageIndex];

      reportIndex = reportItem->globals.reportIndex;
      Appl_Buttons_Details[deviceAddress].reportLength = (pitemListPtrs->reportList[reportIndex].inputBits + 7)/8;
      Appl_Buttons_Details[deviceAddress].reportID = (BYTE)reportItem->globals.reportID;
      Appl_Buttons_Details[deviceAddress].bitOffset = (BYTE)reportItem->startBit;
      Appl_Buttons_Details[deviceAddress].bitLength = (BYTE)reportItem->globals.reportsize;
      Appl_Buttons_Details[deviceAddress].count=(BYTE)reportItem->globals.reportCount;
      Appl_Buttons_Details[deviceAddress].interfaceNum= USBHostHID_ApiGetCurrentInterfaceNum();
    }
  }

  if(pDeviceRptinfo->reports == 1)
  {
    Appl_raw_report_buffer[deviceAddress].Report_ID = 0;
    Appl_raw_report_buffer[deviceAddress].ReportSize = (pitemListPtrs->reportList[reportIndex].inputBits + 7)/8;
    Appl_raw_report_buffer[deviceAddress].ReportData = (BYTE*)USB_MALLOC(Appl_raw_report_buffer[deviceAddress].ReportSize);
    Appl_raw_report_buffer[deviceAddress].ReportPollRate = pDeviceRptinfo->reportPollingRate;
    status = TRUE;
  }
#endif
  return(status);
}


static BOOL USB_HID_KEYBOARD_DataCollectionHandler(BYTE deviceAddress)
{
  BYTE                     NumOfReportItem;
  BYTE                     i;
  USB_HID_ITEM_LIST       *pitemListPtrs;
  USB_HID_DEVICE_RPT_INFO *pDeviceRptinfo;
  HID_REPORTITEM          *reportItem;
  HID_USAGEITEM           *hidUsageItem;
  BYTE                     usageIndex;
  BYTE                     reportIndex;
  BOOL                     foundLEDIndicator = FALSE;
  BOOL                     foundModifierKey = FALSE;
  BOOL                     foundNormalKey = FALSE;
  BOOL                     status;

  pDeviceRptinfo = USBHostHID_GetCurrentReportInfo(); // Get current Report Info pointer
  pitemListPtrs = USBHostHID_GetItemListPointers();   // Get pointer to list of item pointers

  foundLEDIndicator = foundModifierKey = foundNormalKey = FALSE;
  status = FALSE;
  reportIndex = 0;
  NumOfReportItem = pDeviceRptinfo->reportItems;

  for(i=0;i<NumOfReportItem;i++)
  {
    reportItem = &pitemListPtrs->reportItemList[i];
    if((reportItem->reportType==hidReportInput) && (reportItem->dataModes == HIDData_Variable)&&
           (reportItem->globals.usagePage==USAGE_PAGE_KEY_CODES))
    {
      usageIndex = reportItem->firstUsageItem;
      hidUsageItem = &pitemListPtrs->usageItemList[usageIndex];
      if((hidUsageItem->usageMinimum == USAGE_MIN_MODIFIER_KEY)
                &&(hidUsageItem->usageMaximum == USAGE_MAX_MODIFIER_KEY)) //else application cannot suuport
      {
        reportIndex = reportItem->globals.reportIndex;
        Appl_ModifierKeysDetails[deviceAddress].reportLength = (pitemListPtrs->reportList[reportIndex].inputBits + 7)/8;
        Appl_ModifierKeysDetails[deviceAddress].reportID = (BYTE)reportItem->globals.reportID;
        Appl_ModifierKeysDetails[deviceAddress].bitOffset = (BYTE)reportItem->startBit;
        Appl_ModifierKeysDetails[deviceAddress].bitLength = (BYTE)reportItem->globals.reportsize;
        Appl_ModifierKeysDetails[deviceAddress].count=(BYTE)reportItem->globals.reportCount;
        Appl_ModifierKeysDetails[deviceAddress].interfaceNum= USBHostHID_ApiGetCurrentInterfaceNum();
        foundModifierKey = TRUE;
      }

    }
    else if((reportItem->reportType==hidReportInput) && (reportItem->dataModes == HIDData_Array)&&
           (reportItem->globals.usagePage==USAGE_PAGE_KEY_CODES))
    {
      usageIndex = reportItem->firstUsageItem;
      hidUsageItem = &pitemListPtrs->usageItemList[usageIndex];

      if((hidUsageItem->usageMinimum == USAGE_MIN_NORMAL_KEY)
                &&(hidUsageItem->usageMaximum <= USAGE_MAX_NORMAL_KEY)) //else application cannot suuport
      {
        reportIndex = reportItem->globals.reportIndex;
        Appl_NormalKeysDetails[deviceAddress].reportLength = (pitemListPtrs->reportList[reportIndex].inputBits + 7)/8;
        Appl_NormalKeysDetails[deviceAddress].reportID = (BYTE)reportItem->globals.reportID;
        Appl_NormalKeysDetails[deviceAddress].bitOffset = (BYTE)reportItem->startBit;
        Appl_NormalKeysDetails[deviceAddress].bitLength = (BYTE)reportItem->globals.reportsize;
        Appl_NormalKeysDetails[deviceAddress].count=(BYTE)reportItem->globals.reportCount;
        Appl_NormalKeysDetails[deviceAddress].interfaceNum= USBHostHID_ApiGetCurrentInterfaceNum();
        foundNormalKey = TRUE;
      }
    }
    else if((reportItem->reportType==hidReportOutput) &&
                (reportItem->globals.usagePage==USAGE_PAGE_LEDS))
    {
      usageIndex = reportItem->firstUsageItem;
      hidUsageItem = &pitemListPtrs->usageItemList[usageIndex];

      reportIndex = reportItem->globals.reportIndex;
      Appl_LED_Indicator[deviceAddress].reportLength = (pitemListPtrs->reportList[reportIndex].outputBits + 7)/8;
      Appl_LED_Indicator[deviceAddress].reportID = (BYTE)reportItem->globals.reportID;
      Appl_LED_Indicator[deviceAddress].bitOffset = (BYTE)reportItem->startBit;
      Appl_LED_Indicator[deviceAddress].bitLength = (BYTE)reportItem->globals.reportsize;
      Appl_LED_Indicator[deviceAddress].count=(BYTE)reportItem->globals.reportCount;
      Appl_LED_Indicator[deviceAddress].interfaceNum= USBHostHID_ApiGetCurrentInterfaceNum();
      foundLEDIndicator = TRUE;
    }
  }


  if(pDeviceRptinfo->reports == 1)
  {
    Appl_raw_report_buffer[deviceAddress].Report_ID = 0;
    Appl_raw_report_buffer[deviceAddress].ReportSize = (pitemListPtrs->reportList[reportIndex].inputBits + 7)/8;
    Appl_raw_report_buffer[deviceAddress].ReportData = (BYTE*)USB_MALLOC(Appl_raw_report_buffer[deviceAddress].ReportSize);
    Appl_raw_report_buffer[deviceAddress].ReportPollRate = pDeviceRptinfo->reportPollingRate;
    if((foundNormalKey == TRUE)&&(foundModifierKey == TRUE))
      status = TRUE;
  }

  return(status);
}

static BOOL USB_HID_JOYSTICK_DataCollectionHandler(BYTE deviceAddress)
{
  BYTE                     NumOfReportItem;
  BYTE                     i;
  USB_HID_ITEM_LIST       *pitemListPtrs;
  USB_HID_DEVICE_RPT_INFO *pDeviceRptinfo;
  HID_REPORTITEM          *reportItem;
  HID_USAGEITEM           *hidUsageItem;
  BYTE                     usageIndex;
  BYTE                     reportIndex;
  BOOL                     status;
	BYTE										 bitLen;
	BYTE										 startBit;
	BYTE										 reportID;
	BYTE										 reportLen;

  pDeviceRptinfo = USBHostHID_GetCurrentReportInfo(); // Get current Report Info pointer
  pitemListPtrs = USBHostHID_GetItemListPointers();   // Get pointer to list of item pointers

  status = FALSE;


	// X axis
	if (USBHostHID_ApiFindValue(USAGE_PAGE_GEN_DESKTOP, HID_USAGE_X, hidReportInput, &reportID, &reportLen, &startBit, &bitLen ))
	{
		Appl_Axis_Details[deviceAddress][JOY_AXIS_X].reportLength = reportLen;  //pass reportLen as ApiFindValue and ApiFindBit returns it
		Appl_Axis_Details[deviceAddress][JOY_AXIS_X].reportID = reportID;
		Appl_Axis_Details[deviceAddress][JOY_AXIS_X].bitOffset = startBit;
		Appl_Axis_Details[deviceAddress][JOY_AXIS_X].bitLength = bitLen;
		Appl_Axis_Details[deviceAddress][JOY_AXIS_X].count = 1;
		Appl_Axis_Details[deviceAddress][JOY_AXIS_X].interfaceNum = USBHostHID_ApiGetCurrentInterfaceNum();
	}
	else
	{
		Appl_Axis_Details[deviceAddress][JOY_AXIS_X].reportLength = 0;
	}

	// Y axis
	if (USBHostHID_ApiFindValue(USAGE_PAGE_GEN_DESKTOP, HID_USAGE_Y, hidReportInput, &reportID, &reportLen, &startBit, &bitLen ))
	{
		Appl_Axis_Details[deviceAddress][JOY_AXIS_Y].reportLength = reportLen;  //pass reportLen as ApiFindValue and ApiFindBit returns it
		Appl_Axis_Details[deviceAddress][JOY_AXIS_Y].reportID = reportID;
		Appl_Axis_Details[deviceAddress][JOY_AXIS_Y].bitOffset = startBit;
		Appl_Axis_Details[deviceAddress][JOY_AXIS_Y].bitLength = bitLen;
		Appl_Axis_Details[deviceAddress][JOY_AXIS_Y].count = 1;
		Appl_Axis_Details[deviceAddress][JOY_AXIS_Y].interfaceNum = USBHostHID_ApiGetCurrentInterfaceNum();
	}
	else
	{
		Appl_Axis_Details[deviceAddress][JOY_AXIS_Y].reportLength = 0;
	}

	// Z axis
	if (USBHostHID_ApiFindValue(USAGE_PAGE_GEN_DESKTOP, HID_USAGE_Z, hidReportInput, &reportID, &reportLen, &startBit, &bitLen ))
	{
		Appl_Axis_Details[deviceAddress][JOY_AXIS_Z].reportLength = reportLen;  //pass reportLen as ApiFindValue and ApiFindBit returns it
		Appl_Axis_Details[deviceAddress][JOY_AXIS_Z].reportID = reportID;
		Appl_Axis_Details[deviceAddress][JOY_AXIS_Z].bitOffset = startBit;
		Appl_Axis_Details[deviceAddress][JOY_AXIS_Z].bitLength = bitLen;
		Appl_Axis_Details[deviceAddress][JOY_AXIS_Z].count = 1;
		Appl_Axis_Details[deviceAddress][JOY_AXIS_Z].interfaceNum = USBHostHID_ApiGetCurrentInterfaceNum();
	}
	else
	{
		Appl_Axis_Details[deviceAddress][JOY_AXIS_Z].reportLength = 0;
	}

	// RZ axis
	if (USBHostHID_ApiFindValue(USAGE_PAGE_GEN_DESKTOP, HID_USAGE_RZ, hidReportInput, &reportID, &reportLen, &startBit, &bitLen ))
	{
		Appl_Axis_Details[deviceAddress][JOY_AXIS_RZ].reportLength = reportLen;  //pass reportLen as ApiFindValue and ApiFindBit returns it
		Appl_Axis_Details[deviceAddress][JOY_AXIS_RZ].reportID = reportID;
		Appl_Axis_Details[deviceAddress][JOY_AXIS_RZ].bitOffset = startBit;
		Appl_Axis_Details[deviceAddress][JOY_AXIS_RZ].bitLength = bitLen;
		Appl_Axis_Details[deviceAddress][JOY_AXIS_RZ].count = 1;
		Appl_Axis_Details[deviceAddress][JOY_AXIS_RZ].interfaceNum = USBHostHID_ApiGetCurrentInterfaceNum();
	}
	else
	{
		Appl_Axis_Details[deviceAddress][JOY_AXIS_RZ].reportLength = 0;
	}
#if 0

  reportIndex = 0;
  NumOfReportItem = pDeviceRptinfo->reportItems;

  for(i=0;i<NumOfReportItem;i++)
  {
    reportItem = &pitemListPtrs->reportItemList[i];

    if((reportItem->reportType==hidReportInput) && (reportItem->dataModes == HIDData_Variable) &&
           (reportItem->globals.usagePage==USAGE_PAGE_GEN_DESKTOP))
    {
      usageIndex = reportItem->firstUsageItem;
      hidUsageItem = &pitemListPtrs->usageItemList[usageIndex];

      reportIndex = reportItem->globals.reportIndex;


    }
    else if((reportItem->reportType==hidReportInput) && (reportItem->dataModes == (HIDData_NullState | HIDData_Variable)) &&
           (reportItem->globals.usagePage==USAGE_PAGE_GEN_DESKTOP))
    {
      usageIndex = reportItem->firstUsageItem;
      hidUsageItem = &pitemListPtrs->usageItemList[usageIndex];

      reportIndex = reportItem->globals.reportIndex;
      Appl_HatSwitch_Details[deviceAddress].reportLength = (pitemListPtrs->reportList[reportIndex].inputBits + 7)/8;
      Appl_HatSwitch_Details[deviceAddress].reportID = (BYTE)reportItem->globals.reportID;
      Appl_HatSwitch_Details[deviceAddress].bitOffset = (BYTE)reportItem->startBit;
      Appl_HatSwitch_Details[deviceAddress].bitLength = (BYTE)reportItem->globals.reportsize;
      Appl_HatSwitch_Details[deviceAddress].count=(BYTE)reportItem->globals.reportCount;
      Appl_HatSwitch_Details[deviceAddress].interfaceNum= USBHostHID_ApiGetCurrentInterfaceNum();

    }

    else if((reportItem->reportType==hidReportInput) && (reportItem->dataModes == HIDData_Variable)&&
           (reportItem->globals.usagePage==USAGE_PAGE_BUTTONS))
    {
      usageIndex = reportItem->firstUsageItem;
      hidUsageItem = &pitemListPtrs->usageItemList[usageIndex];

      reportIndex = reportItem->globals.reportIndex;
      Appl_Buttons_Details[deviceAddress].reportLength = (pitemListPtrs->reportList[reportIndex].inputBits + 7)/8;
      Appl_Buttons_Details[deviceAddress].reportID = (BYTE)reportItem->globals.reportID;
      Appl_Buttons_Details[deviceAddress].bitOffset = (BYTE)reportItem->startBit;
      Appl_Buttons_Details[deviceAddress].bitLength = (BYTE)reportItem->globals.reportsize;
      Appl_Buttons_Details[deviceAddress].count=(BYTE)reportItem->globals.reportCount;
      Appl_Buttons_Details[deviceAddress].interfaceNum= USBHostHID_ApiGetCurrentInterfaceNum();
    }
  }
#endif
  if(pDeviceRptinfo->reports == 1)
  {
      Appl_raw_report_buffer[deviceAddress].Report_ID = 0;
      Appl_raw_report_buffer[deviceAddress].ReportSize = (pitemListPtrs->reportList[reportIndex].inputBits + 7)/8;
      Appl_raw_report_buffer[deviceAddress].ReportData = (BYTE*)USB_MALLOC(Appl_raw_report_buffer[deviceAddress].ReportSize);
      Appl_raw_report_buffer[deviceAddress].ReportPollRate = pDeviceRptinfo->reportPollingRate;
      status = TRUE;
  }

  return(status);
}


/*** end of usb_host_hid_device.c **********************************************************************/


