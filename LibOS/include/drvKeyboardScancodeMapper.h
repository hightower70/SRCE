#ifndef __drvKeyboardScancodeMapper_h
#define __drvKeyboardScancodeMapper_h

#include <sysTypes.h>
#include <sysVirtualKeyboardCodes.h>

uint16_t drvScancodeToKeycode(uint8_t in_scan_code, uint8_t in_modifier_state, uint8_t in_lock_state);

#endif
