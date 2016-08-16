#include <usb.h>
#include <usb_host_hid_device.h>
#include <usb_host_hid.h>
#include <sysVirtualKeyboardCodes.h>
#include <drvUSBHIDHost.h>
#include <sysTimer.h>

//static void ProcessKeyboardData(USB_KEYBOARD_DATA* in_keyboard_data);

static sysTimeStamp l_keyboard_poll_timestamp;
static sysTimeStamp l_joystick_poll_timestamp;

void drvUSBHIDHOSTTask(void)
{
	USBTasks();

	// poll keyboard
	if(getHIDnumOfDevice(HID_KEYBOARD)!=0)
	{
		if(sysTimerGetTimeSince(l_keyboard_poll_timestamp) > 10)
		{
			l_keyboard_poll_timestamp = sysTimerGetTimestamp();
			
			keyboard_read(1);
		}
	}
	else
	{
		l_keyboard_poll_timestamp = sysTimerGetTimestamp();
	}

	// poll joystick
	if(getHIDnumOfDevice(HID_JOYSTICK)!=0)
	{
		if(sysTimerGetTimeSince(l_joystick_poll_timestamp) > 10)
		{
			l_joystick_poll_timestamp = sysTimerGetTimestamp();

			joystick_read(1);
		}
	}
	else
	{
		l_joystick_poll_timestamp = sysTimerGetTimestamp();
	}
}





/*============================================================================================
	BOOL USB_ApplicationEventHandler ( BYTE address, USB_EVENT event, void *data, DWORD size )
============================================================================================*/

BOOL USB_ApplicationEventHandler ( BYTE address, USB_EVENT event, void *data, DWORD size )
{

    // Handle specific events.
    switch (event){

    case EVENT_OVERRIDE_CLIENT_DRIVER_SELECTION:
        return FALSE;
 
    case EVENT_VBUS_RELEASE_POWER:
    case EVENT_VBUS_REQUEST_POWER:
    case EVENT_UNSUPPORTED_DEVICE:
    case EVENT_CANNOT_ENUMERATE:
    case EVENT_CLIENT_INIT_ERROR:
    case EVENT_OUT_OF_MEMORY:
    case EVENT_UNSPECIFIED_ERROR:
        break;

    default:
       break;
   }

   return TRUE;

} // USB_ApplicationEventHandler


/*-------------------------------------------------------------------------------
	BOOL USB_ApplicationDataEventHandler ( BYTE address, USB_EVENT event, void *data, DWORD size )
-------------------------------------------------------------------------------*/

BOOL USB_ApplicationDataEventHandler ( BYTE address, USB_EVENT event, void *data, DWORD size )
{
	return TRUE;
}
