#include <sysUserInput.h>
#include <sysVirtualKeyboardCodes.h>
#include <fbFileBrowser.h>

void fbUserInputEventHandler(uint8_t in_device_number, sysUserInputEventCategory in_event_category, sysUserInputEventType in_event_type, uint32_t in_event_param)
{
	// if busy or inactive do nothing
	if (fbGetStatus() < fbs_Active)
		return;

	if(in_device_number == 1 && (in_event_category == sysUIEC_Pressed || in_event_category == sysUIEC_Repeated))
	{
		switch(in_event_type)
		{
			case sysUIET_JoyYAxis:
				if(in_event_param < 0x7f)
				{
					fbSelectionMove(-1);
				}
				else
				{
					fbSelectionMove(1);
				}
				break;

			case sysUIET_JoyXAxis:
				if (in_event_param > 0x7f)
				{
					if (fbSelectionEnter())
					{
						fbFileSelectionHandler();
					}
				}
				break;

			case sysUIET_Key:
				if(sysVKC_IS_SPECIAL_KEY(in_event_param))
				{
					switch (sysVKC_KEY_CODE(in_event_param))
					{
						case sysVKC_DOWN:
							fbSelectionMove(1);
							break;

						case sysVKC_UP:
							fbSelectionMove(-1);
							break;

						case sysVKC_PRIOR:
							fbSelectionMove(fbSELECTION_PAGE_UP);
							break;

						case sysVKC_NEXT:
							fbSelectionMove(fbSELECTION_PAGE_DOWN);
							break;

						case sysVKC_END:
							fbSelectionMove(fbSELECTION_LAST);
							break;

						case sysVKC_HOME:
							fbSelectionMove(fbSELECTION_FIRST);
							break;
					}
				}
				else
				{
					switch(in_event_param)
					{
						case sysVKC_RETURN:
							if (fbSelectionEnter())
							{
								fbFileSelectionHandler();
							}
							break;
					}
				}
				break;
		}
	}
}
