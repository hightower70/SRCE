#include <sysUserInput.h>
#include <emuInvaders.h>

void sysMainTask(void)
{
	emuInvadersTask();
}

void drvUserInputEventHandler(uint8_t in_device_number, sysUserInputEventCategory in_event_category, sysUserInputEventType in_event_type, uint32_t in_event_param)
{
	emuUserInputEventHandler(in_device_number, in_event_category, in_event_type, in_event_param);
}
