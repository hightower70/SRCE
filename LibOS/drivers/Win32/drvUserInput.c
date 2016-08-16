#include <Windows.h>
#include <sysUserInput.h>
#include <sysVirtualKeyboardCodes.h>

static uint32_t l_modifier_keys = 0;

void drvUserInputProcessKeyboardEvents(sysUserInputEventCategory in_input_category, LPARAM in_lparam, WPARAM in_wparam)
{
	// process modifier keys
	if (in_input_category == sysUIEC_Pressed)
	{
		switch (in_wparam)
		{
			case sysVKC_SHIFT:
				l_modifier_keys |= sysMS_SHIFT;
				break;

			case sysVKC_CONTROL:
				l_modifier_keys |= sysMS_CTRL;
				break;

			case sysVKC_MENU:
				l_modifier_keys |= sysMS_ALT;
				break;
		}
	}

	if (in_input_category == sysUIEC_Released)
	{
		switch (in_wparam)
		{
			case sysVKC_SHIFT:
				l_modifier_keys &= ~sysMS_SHIFT;
				break;

			case sysVKC_CONTROL:
				l_modifier_keys &= ~sysMS_CTRL;
				break;

			case sysVKC_MENU:
				l_modifier_keys &= ~sysMS_ALT;
				break;
		}
	}

	// pass the event to the application
	sysUserInputEventHandler(0, in_input_category, sysUIET_Key, (uint32_t)((((in_lparam & (1 << 24)) != 0) ? sysVKC_SPECIAL_KEY_FLAG : 0) | in_wparam));
}

uint32_t sysKeyboardGetModifiersState(uint8_t in_device_number)
{
	return l_modifier_keys;
}
