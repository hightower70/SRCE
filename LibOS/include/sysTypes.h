/*****************************************************************************/
/* System Type Definitions                                                   */
/*                                                                           */
/* Copyright (C) 2014 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

#ifndef __sysTypes_h
#define __sysTypes_h

///////////////////////////////////////////////////////////////////////////////
// Type definitions
#include <stdint.h>

#ifdef _WIN32
#pragma once

#define false   0
#define true    1

#define bool int
#else
#include <stdbool.h>
#endif

/* string types */
typedef uint16_t sysStringLength;

#define sysSTRING_MAX_LENGTH 0xfffe
#define sysSTRING_INVALID_POS 0xffff

typedef char sysChar;
typedef char sysASCIIChar;
typedef uint16_t sysUnicodeChar;
typedef sysChar*	sysString;
typedef const sysChar* sysConstString;

///////////////////////////////////////////////////////////////////////////////
// Macros
#ifndef LOW
#define LOW(x) ((x)&0xff)
#endif

#ifndef HIGH
#define HIGH(x) ((x)>>8)
#endif

#ifndef BV
#define BV(x) (1<<(x))
#endif

#define STRINGIZE(x) ___STRINGIZE(x)
#define ___STRINGIZE(x) #x

#define sysNULL 0

#define sysUNUSED(x) (void)(x)


#endif
