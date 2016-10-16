/*****************************************************************************/
/* Keyboard input repated HAL functions for Linux systems                    */
/*                                                                           */
/* Copyright (C) 2016 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <regex.h>
#include <dirent.h>
#include <fcntl.h>
#include <linux/input.h>
#include <sysTypes.h>
#include <sysUserInput.h>
#include <sysVirtualKeyboardCodes.h>

/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/
static int keyboardFd = -1;

static sysUserInputEventCategory l_event_category_lookup[3] = 
{
	sysUIEC_Released,
	sysUIEC_Pressed,
	sysUIEC_Repeated
};

static const uint16_t l_key_code_to_virtual_key_table[256] =
{ 
	sysVKC_INVALID,	// 0x00
	sysVKC_ESCAPE,	// 0x01
	sysVKC_1,				// 0x02
	sysVKC_2,				// 0x03
	sysVKC_3,				// 0x04
	sysVKC_4,				// 0x05
	sysVKC_5,				// 0x03
	sysVKC_6,				// 0x07
	sysVKC_7,				// 0x08
	sysVKC_8,				// 0x09
	sysVKC_9,				// 0x0a
	sysVKC_0,				// 0x0b
	sysVKC_INVALID,	// 0x0c
	sysVKC_INVALID,	// 0x0d
	sysVKC_INVALID,	// 0x0e
	sysVKC_TAB,	// 0x0f
	
	sysVKC_Q,	// 0x10
	sysVKC_W,	// 0x11
	sysVKC_E,	// 0x12
	sysVKC_R,	// 0x13
	sysVKC_T,	// 0x14
	sysVKC_Y,	// 0x15
	sysVKC_U,	// 0x16
	sysVKC_I,	// 0x17
	sysVKC_O,	// 0x18
	sysVKC_P,	// 0x19
	sysVKC_INVALID,	// 0x1a
	sysVKC_INVALID,	// 0x1b
	sysVKC_RETURN,	// 0x1c
	sysVKC_LCONTROL,	// 0x1d
	sysVKC_A,	// 0x1e
	sysVKC_S,	// 0x1f
	
	
	sysVKC_D,	// 0x20
	sysVKC_F,	// 0x21
	sysVKC_G,	// 0x22
	sysVKC_H,	// 0x23
	sysVKC_J,	// 0x24
	sysVKC_K,	// 0x25
	sysVKC_L,	// 0x26
	sysVKC_INVALID,	// 0x27
	sysVKC_INVALID,	// 0x28
	sysVKC_INVALID,	// 0x29
	sysVKC_INVALID,	// 0x2a
	sysVKC_INVALID,	// 0x2b
	sysVKC_Z,	// 0x2c
	sysVKC_X,	// 0x2d
	sysVKC_C,	// 0x2e
	sysVKC_V,	// 0x2f
		
	sysVKC_B,	// 0x30
	sysVKC_N,	// 0x31
	sysVKC_M,	// 0x32
	sysVKC_INVALID,	// 0x33
	sysVKC_INVALID,	// 0x34
	sysVKC_INVALID,	// 0x35
	sysVKC_INVALID,	// 0x36
	sysVKC_INVALID,	// 0x37
	sysVKC_INVALID,	// 0x38
	sysVKC_SPACE,	// 0x39
	sysVKC_INVALID,	// 0x3a
	sysVKC_F1,	// 0x3b
	sysVKC_F2,	// 0x3c
	sysVKC_F3,	// 0x3d
	sysVKC_F4,	// 0x3e
	sysVKC_F5,	// 0x3f
	
	sysVKC_F6,	// 0x40
	sysVKC_F7,	// 0x41
	sysVKC_F8,	// 0x42
	sysVKC_F9,	// 0x43
	sysVKC_F10,	// 0x44
	sysVKC_NUMLOCK,	// 0x45
	sysVKC_SCROLL,	// 0x46
	sysVKC_NUMPAD7,	// 0x47
	sysVKC_NUMPAD8,	// 0x48
	sysVKC_NUMPAD9,	// 0x49
	sysVKC_SUBTRACT,// 0x4a
	sysVKC_NUMPAD4,	// 0x4b
	sysVKC_NUMPAD5,	// 0x4c
	sysVKC_NUMPAD6,	// 0x4d
	sysVKC_ADD,			// 0x4e
	sysVKC_NUMPAD1,	// 0x4f
		
	sysVKC_NUMPAD2,	// 0x50
	sysVKC_NUMPAD3,	// 0x51
	sysVKC_NUMPAD0,	// 0x52
	sysVKC_DECIMAL,	// 0x53
	sysVKC_INVALID,	// 0x54
	sysVKC_INVALID,	// 0x55
	sysVKC_INVALID,	// 0x56
	sysVKC_F11,	// 0x57
	sysVKC_F12,	// 0x58
	sysVKC_INVALID,	// 0x59
	sysVKC_INVALID,	// 0x5a
	sysVKC_INVALID,	// 0x5b
	sysVKC_INVALID,	// 0x5c
	sysVKC_INVALID,	// 0x5d
	sysVKC_INVALID,	// 0x5e
	sysVKC_INVALID,	// 0x5f

	sysVKC_INVALID,	// 0x60
	sysVKC_INVALID,	// 0x61
	sysVKC_INVALID,	// 0x62
	sysVKC_INVALID,	// 0x63
	sysVKC_INVALID,	// 0x64
	sysVKC_INVALID,	// 0x65
	sysVKC_HOME,	// 0x66
	sysVKC_UP | sysVKC_SPECIAL_KEY_FLAG,	// 0x67
	sysVKC_PRIOR,	// 0x68
	sysVKC_LEFT |  sysVKC_SPECIAL_KEY_FLAG,	// 0x69
	sysVKC_RIGHT | sysVKC_SPECIAL_KEY_FLAG,	// 0x6a
	sysVKC_END |  sysVKC_SPECIAL_KEY_FLAG,	// 0x6b
	sysVKC_DOWN | sysVKC_SPECIAL_KEY_FLAG,	// 0x6c
	sysVKC_NEXT,	// 0x6d
	sysVKC_INSERT,	// 0x6e
	sysVKC_DELETE,	// 0x6f

	sysVKC_INVALID,	// 0x70
	sysVKC_INVALID,	// 0x71
	sysVKC_INVALID,	// 0x72
	sysVKC_INVALID,	// 0x73
	sysVKC_INVALID,	// 0x74
	sysVKC_INVALID,	// 0x75
	sysVKC_INVALID,	// 0x76
	sysVKC_INVALID,	// 0x77
	sysVKC_INVALID,	// 0x78
	sysVKC_INVALID,	// 0x79
	sysVKC_INVALID,	// 0x7a
	sysVKC_INVALID,	// 0x7b
	sysVKC_INVALID,	// 0x7c
	sysVKC_INVALID,	// 0x7d
	sysVKC_INVALID,	// 0x7e
	sysVKC_INVALID,	// 0x7f

	sysVKC_INVALID,	// 0x80
	sysVKC_INVALID,	// 0x81
	sysVKC_INVALID,	// 0x82
	sysVKC_INVALID,	// 0x83
	sysVKC_INVALID,	// 0x84
	sysVKC_INVALID,	// 0x85
	sysVKC_INVALID,	// 0x86
	sysVKC_INVALID,	// 0x87
	sysVKC_INVALID,	// 0x88
	sysVKC_INVALID,	// 0x89
	sysVKC_INVALID,	// 0x8a
	sysVKC_INVALID,	// 0x8b
	sysVKC_INVALID,	// 0x8c
	sysVKC_INVALID,	// 0x8d
	sysVKC_INVALID,	// 0x8e
	sysVKC_INVALID,	// 0x8f

	sysVKC_INVALID,	// 0x90
	sysVKC_INVALID,	// 0x91
	sysVKC_INVALID,	// 0x92
	sysVKC_INVALID,	// 0x93
	sysVKC_INVALID,	// 0x94
	sysVKC_INVALID,	// 0x95
	sysVKC_INVALID,	// 0x96
	sysVKC_INVALID,	// 0x97
	sysVKC_INVALID,	// 0x98
	sysVKC_INVALID,	// 0x99
	sysVKC_INVALID,	// 0x9a
	sysVKC_INVALID,	// 0x9b
	sysVKC_INVALID,	// 0x9c
	sysVKC_INVALID,	// 0x9d
	sysVKC_INVALID,	// 0x9e
	sysVKC_INVALID,	// 0x9f

	sysVKC_INVALID,	// 0xa0
	sysVKC_INVALID,	// 0xa1
	sysVKC_INVALID,	// 0xa2
	sysVKC_INVALID,	// 0xa3
	sysVKC_INVALID,	// 0xa4
	sysVKC_INVALID,	// 0xa5
	sysVKC_INVALID,	// 0xa6
	sysVKC_INVALID,	// 0xa7
	sysVKC_INVALID,	// 0xa8
	sysVKC_INVALID,	// 0xa9
	sysVKC_INVALID,	// 0xaa
	sysVKC_INVALID,	// 0xab
	sysVKC_INVALID,	// 0xac
	sysVKC_INVALID,	// 0xad
	sysVKC_INVALID,	// 0xae
	sysVKC_INVALID,	// 0xaf

	sysVKC_INVALID,	// 0xb0
	sysVKC_INVALID,	// 0xb1
	sysVKC_INVALID,	// 0xb2
	sysVKC_INVALID,	// 0xb3
	sysVKC_INVALID,	// 0xb4
	sysVKC_INVALID,	// 0xb5
	sysVKC_INVALID,	// 0xb6
	sysVKC_INVALID,	// 0xb7
	sysVKC_INVALID,	// 0xb8
	sysVKC_INVALID,	// 0xb9
	sysVKC_INVALID,	// 0xba
	sysVKC_INVALID,	// 0xbb
	sysVKC_INVALID,	// 0xbc
	sysVKC_INVALID,	// 0xbd
	sysVKC_INVALID,	// 0xbe
	sysVKC_INVALID,	// 0xbf

	sysVKC_INVALID,	// 0xc0
	sysVKC_INVALID,	// 0xc1
	sysVKC_INVALID,	// 0xc2
	sysVKC_INVALID,	// 0xc3
	sysVKC_INVALID,	// 0xc4
	sysVKC_INVALID,	// 0xc5
	sysVKC_INVALID,	// 0xc6
	sysVKC_INVALID,	// 0xc7
	sysVKC_INVALID,	// 0xc8
	sysVKC_INVALID,	// 0xc9
	sysVKC_INVALID,	// 0xca
	sysVKC_INVALID,	// 0xcb
	sysVKC_INVALID,	// 0xcc
	sysVKC_INVALID,	// 0xcd
	sysVKC_INVALID,	// 0xce
	sysVKC_INVALID,	// 0xcf

	sysVKC_INVALID,	// 0xd0
	sysVKC_INVALID,	// 0xd1
	sysVKC_INVALID,	// 0xd2
	sysVKC_INVALID,	// 0xd3
	sysVKC_INVALID,	// 0xd4
	sysVKC_INVALID,	// 0xd5
	sysVKC_INVALID,	// 0xd6
	sysVKC_INVALID,	// 0xd7
	sysVKC_INVALID,	// 0xd8
	sysVKC_INVALID,	// 0xd9
	sysVKC_INVALID,	// 0xda
	sysVKC_INVALID,	// 0xdb
	sysVKC_INVALID,	// 0xdc
	sysVKC_INVALID,	// 0xdd
	sysVKC_INVALID,	// 0xde
	sysVKC_INVALID,	// 0xdf

	sysVKC_INVALID,	// 0xe0
	sysVKC_INVALID,	// 0xe1
	sysVKC_INVALID,	// 0xe2
	sysVKC_INVALID,	// 0xe3
	sysVKC_INVALID,	// 0xe4
	sysVKC_INVALID,	// 0xe5
	sysVKC_INVALID,	// 0xe6
	sysVKC_INVALID,	// 0xe7
	sysVKC_INVALID,	// 0xe8
	sysVKC_INVALID,	// 0xe9
	sysVKC_INVALID,	// 0xea
	sysVKC_INVALID,	// 0xeb
	sysVKC_INVALID,	// 0xec
	sysVKC_INVALID,	// 0xed
	sysVKC_INVALID,	// 0xee
	sysVKC_INVALID,	// 0xef

	sysVKC_INVALID,	// 0xf0
	sysVKC_INVALID,	// 0xf1
	sysVKC_INVALID,	// 0xf2
	sysVKC_INVALID,	// 0xf3
	sysVKC_INVALID,	// 0xf4
	sysVKC_INVALID,	// 0xf5
	sysVKC_INVALID,	// 0xf6
	sysVKC_INVALID,	// 0xf7
	sysVKC_INVALID,	// 0xf8
	sysVKC_INVALID,	// 0xf9
	sysVKC_INVALID,	// 0xfa
	sysVKC_INVALID,	// 0xfb
	sysVKC_INVALID,	// 0xfc
	sysVKC_INVALID,	// 0xfd
	sysVKC_INVALID,	// 0xfe
	sysVKC_INVALID	// 0xff
};



/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Initializes keyboard HAL 
void halKeyboardInputInitialize(void)
{
	DIR *dirp;
	struct dirent *dp;
	regex_t kbd;

	char fullPath[1024];
	static char *dirName = "/dev/input/by-id";
	int result;
	
	regcomp(&kbd, "event-kbd", 0);

	if ((dirp = opendir(dirName)) == NULL) 
	{
		perror("Couldn't open '/dev/input/by-id'\n");
		return;
	}

	// Find any files that match the regex for keyboard
	keyboardFd = -1;
	do 
	{
		errno = 0;
		if ((dp = readdir(dirp)) != NULL) 
		{
			if (regexec(&kbd, dp->d_name, 0, NULL, 0) == 0)
			{
				sprintf(fullPath, "%s/%s", dirName, dp->d_name);
				keyboardFd = open(fullPath, O_RDONLY | O_NONBLOCK);
				if (keyboardFd == -1)
				{
					perror("Can't open keyboard for event reading\n");
					return;
				}
				else
				{
					if (ioctl(keyboardFd, EVIOCGRAB, 1) != 0)
					{
						perror("Can't get access for the keyboard\n");
						return;
					}
				}
			}
		}
	} while (dp != NULL && keyboardFd == -1);

	closedir(dirp);

	regfree(&kbd);
}

void halKeyboardInputCleanup(void)
{
	if (keyboardFd != -1)
		close(keyboardFd);
}

void halKeyboardDispatchEvent(void)
{
	int count, i;
	struct input_event *evp;
	int rd;
	struct input_event event[64];
	uint16_t virtual_key_code;

	rd = read(keyboardFd, event, sizeof(event));
	if (rd > 0)
	{
		count = rd / sizeof(struct input_event);
		i = 0;
		while (count--)
		{
			if (event[i].type == EV_KEY && event[i].value >= 0 && event[i].value < 3 && event[i].code < 255)
			{
				virtual_key_code = l_key_code_to_virtual_key_table[event[i].code];
				
				if(virtual_key_code != sysVKC_INVALID)
					sysUserInputEventHandler(1, l_event_category_lookup[event[i].value], sysUIET_Key, virtual_key_code);
			}
			i++;
		}
	}
}

	