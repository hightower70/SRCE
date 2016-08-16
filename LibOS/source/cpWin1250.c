/*****************************************************************************/
/* Win1250 Codepage handling functions                                       */
/*                                                                           */
/* Copyright (C) 2015 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <cpCodePages.h>
#include <sysVirtualKeyboardCodes.h>

/*****************************************************************************/
/* Types                                                                     */
/*****************************************************************************/

typedef struct
{
	uint8_t CharacterCode;
	uint8_t Shift;
	uint8_t AltGr;
} KeymapTableEntry;

/*****************************************************************************/
/* Tables                                                                    */
/*****************************************************************************/

static const KeymapTableEntry l_keymap_table[256] =
{
	{ 0x00,		0x00,		0x00 },	// 0x00
	{ 0x00,		0x00,		0x00 },	// 0x01
	{ 0x00,		0x00,		0x00 },	// 0x02
	{ 0x00,		0x00,		0x00 },	// 0x03
	{ 0x00,		0x00,		0x00 },	// 0x04
	{ 0x00,		0x00,		0x00 }, // 0x05
	{ 0x00,		0x00,		0x00 }, // 0x06
	{ 0x00,		0x00,		0x00 }, // 0x07
	{ 0x08,		0x00,		0x00 }, // 0x08 sysVKC_BACK
	{ 0x09,		0x00,		0x00 }, // 0x09 sysVKC_TAB
	{ 0x00,		0x00,		0x00 }, // 0x0A
	{ 0x00,		0x00,		0x00 }, // 0x0B
	{ 0x00,		0x00,		0x00 }, // 0x0C sysVKC_CLEAR
	{ 0x0D,		0x00,		0x00 }, // 0x0D sysVKC_RETURN
	{ 0x00,		0x00,		0x00 }, // 0x0E
	{ 0x00,		0x00,		0x00 }, // 0x0F

	{ 0x00,		0x00,		0x00 }, // 0x10 sysVKC_SHIFT
	{ 0x00,		0x00,		0x00 }, // 0x11 sysVKC_CONTROL
	{ 0x00,		0x00,		0x00 }, // 0x12 sysVKC_MENU
	{ 0x00,		0x00,		0x00 }, // 0x13 sysVKC_PAUSE
	{ 0x00,		0x00,		0x00 }, // 0x14 sysVKC_CAPITAL
	{ 0x00,		0x00,		0x00 }, // 0x15 sysVKC_KANA, sysVKC_HANGUL
	{ 0x00,		0x00,		0x00 }, // 0x16 
	{ 0x00,		0x00,		0x00 }, // 0x17 sysVKC_JUNJA
	{ 0x00,		0x00,		0x00 }, // 0x18 sysVKC_FINAL
	{ 0x00,		0x00,		0x00 }, // 0x19 sysVKC_HANJA, sysVKC_KANJI
	{ 0x00,		0x00,		0x00 }, // 0x1A
	{ 0x00,		0x00,		0x00 }, // 0x1B sysVKC_ESCAPE
	{ 0x00,		0x00,		0x00 }, // 0x1C sysVKC_CONVERT
	{ 0x00,		0x00,		0x00 }, // 0x1D sysVKC_NONCONVERT
	{ 0x00,		0x00,		0x00 }, // 0x1E sysVKC_ACCEPT
	{ 0x00,		0x00,		0x00 }, // 0x1F sysVKC_MODECHANGE

	{ ' ',		0x00,		0x00 }, // 0x20 sysVKC_SPACE
	{ 0x00,		0x00,		0x00 }, // 0x21 sysVKC_PRIOR
	{ 0x00,		0x00,		0x00 }, // 0x22 sysVKC_NEXT
	{ 0x00,		0x00,		0x00 }, // 0x23 sysVKC_END	
	{ 0x00,		0x00,		0x00 }, // 0x24 sysVKC_HOME
	{ 0x00,		0x00,		0x00 }, // 0x25 sysVKC_LEFT
	{ 0x00,		0x00,		0x00 },	// 0x26 sysVKC_UP
	{ 0x00,		0x00,		0x00 },	// 0x27 sysVKC_RIGHT
	{ 0x00,		0x00,		0x00 },	// 0x28 sysVKC_DOWN
	{ 0x00,		0x00,		0x00 },	// 0x29 sysVKC_SELECT
	{ 0x00,		0x00,		0x00 },	// 0x2A sysVKC_PRINT
	{ 0x00,		0x00,		0x00 },	// 0x2B sysVKC_EXECUTE
	{ 0x00,		0x00,		0x00 },	// 0x2C sysVKC_SNAPSHOT
	{ 0x00,		0x00,		0x00 },	// 0x2D sysVKC_INSERT
	{ 0x00,		0x00,		0x00 },	// 0x2E sysVKC_DELETE
	{ 0x00,		0x00,		0x00 },	// 0x2F sysVKC_HELP

	{  '0',	'\xA7',		0x00 },	// 0x30 sysVKC_0
	{  '1',		'\'',		 '~' },	// 0x31 sysVKC_1
	{  '2',		'\"',	'\xA1' },	// 0x32 sysVKC_2
	{  '3',		 '+',		 '^' },	// 0x33 sysVKC_3
	{  '4',		 '!',	'\xA2' },	// 0x34 sysVKC_4
	{  '5',		 '%',	'\xB0' },	// 0x35 sysVKC_5
	{  '6',		 '/',	'\xB2' },	// 0x36 sysVKC_6
	{  '7',		 '=',		 '`' },	// 0x37 sysVKC_7
	{  '8',		 '(',	'\xff' },	// 0x38 sysVKC_8
	{  '9',		 ')',	'\xB4' },	// 0x39 sysVKC_9
	{ 0x00,		0x00,		0x00 }, // 0x3A
	{ 0x00,		0x00,		0x00 }, // 0x3B
	{ 0x00,		0x00,		0x00 }, // 0x3C
	{ 0x00,		0x00,		0x00 }, // 0x3D
	{ 0x00,		0x00,		0x00 }, // 0x3E
	{ 0x00,		0x00,		0x00 }, // 0x3F

	{ 0x00,		0x00,		0x00 }, // 0x40
	{ 'a',		 'A',		0x00 },	// 0x41 sysVKC_A
	{ 'b',		 'B',		 '{' },	// 0x42 sysVKC_B
	{ 'c',		 'C',		 '&' },	// 0x43 sysVKC_C
	{ 'd',		 'D',	'\xD0' },	// 0x44 sysVKC_D
	{ 'e',		 'E',		0x00 },	// 0x45 sysVKC_E
	{ 'f',		 'F',		 '[' },	// 0x46 sysVKC_F
	{ 'g',		 'G',		 ']' },	// 0x47 sysVKC_G
	{ 'h',		 'H',		0x00 },	// 0x48 sysVKC_H
	{ 'i',		 'I',		0x00 },	// 0x49 sysVKC_I
	{ 'j',		 'J',		0x00 },	// 0x4A sysVKC_J
	{ 'k',		 'K',	'\xB3' },	// 0x4B sysVKC_K
	{ 'l',		 'L',	'\xA3' },	// 0x4C sysVKC_L
	{ 'm',		 'M',		0x00 },	// 0x4D sysVKC_M
	{ 'n',		 'N',		 '}' },	// 0x4E sysVKC_N
	{ 'o',		 'O',		0x00 },	// 0x4F sysVKC_O

	{ 'p',		 'P',		0x00 },	// 0x50 sysVKC_P
	{ 'q',		 'Q',		'\\' },	// 0x51 sysVKC_Q
	{ 'r',		 'R',		0x00 },	// 0x52 sysVKC_R
	{ 's',		 'S',	'\xF0' },	// 0x53 sysVKC_S
	{ 't',		 'T',		0x00 },	// 0x54 sysVKC_T
	{ 'u',		 'U',	'\x80' },	// 0x55 sysVKC_U
	{ 'v',		 'V',		 '@' },	// 0x56 sysVKC_V
	{ 'w',		 'W',		 '|' },	// 0x57 sysVKC_W
	{ 'x',		 'X',		 '#' },	// 0x58 sysVKC_X
	{ 'y',		 'Y',		 '>' },	// 0x59 sysVKC_Y
	{ 'z',		 'Z',		0x00 },	// 0x5A sysVKC_Z
	{ 0x00,		0x00,		0x00 },	// 0x5B sysVKC_LWIN
	{ 0x00,		0x00,		0x00 },	// 0x5C sysVKC_RWIN
	{ 0x00,		0x00,		0x00 },	// 0x5D sysVKC_APPS
	{ 0x00,		0x00,		0x00 },	// 0x5E
	{ 0x00,		0x00,		0x00 },	// 0x5F sysVKC_SLEEP

	{  '0',		0x00,		0x00 },	// 0x60 sysVKC_NUMPAD0
	{  '1',		0x00,		0x00 },	// 0x61 sysVKC_NUMPAD1
	{  '2',		0x00,		0x00 },	// 0x62 sysVKC_NUMPAD2
	{  '3',		0x00,		0x00 },	// 0x63 sysVKC_NUMPAD3
	{  '4',		0x00,		0x00 },	// 0x64 sysVKC_NUMPAD4
	{  '5',		0x00,		0x00 },	// 0x65 sysVKC_NUMPAD5
	{  '6',		0x00,		0x00 },	// 0x66 sysVKC_NUMPAD6
	{  '7',		0x00,		0x00 },	// 0x67 sysVKC_NUMPAD7
	{  '8',		0x00,		0x00 },	// 0x68 sysVKC_NUMPAD8
	{  '9',		0x00,		0x00 },	// 0x69 sysVKC_NUMPAD9
	{  '*',		0x00,		0x00 },	// 0x6A sysVKC_MULTIPLY
	{  '+',		0x00,		0x00 },	// 0x6B sysVKC_ADD
	{  ',',		0x00,		0x00 },	// 0x6C sysVKC_SEPARATOR
	{  '-',		0x00,		0x00 },	// 0x6D sysVKC_SUBTRACT
	{  '.',		0x00,		0x00 },	// 0x6E sysVKC_DECIMAL
	{  '/',		0x00,		0x00 },	// 0x6F sysVKC_DIVIDE

	{ 0x00,		0x00,		0x00 },	// 0x70 sysVKC_F1
	{ 0x00,		0x00,		0x00 },	// 0x71 sysVKC_F2
	{ 0x00,		0x00,		0x00 },	// 0x72 sysVKC_F3
	{ 0x00,		0x00,		0x00 },	// 0x73 sysVKC_F4
	{ 0x00,		0x00,		0x00 },	// 0x74 sysVKC_F5
	{ 0x00,		0x00,		0x00 },	// 0x75 sysVKC_F6
	{ 0x00,		0x00,		0x00 },	// 0x76 sysVKC_F7
	{ 0x00,		0x00,		0x00 },	// 0x77 sysVKC_F8
	{ 0x00,		0x00,		0x00 },	// 0x78 sysVKC_F9
	{ 0x00,		0x00,		0x00 },	// 0x79 sysVKC_F10
	{ 0x00,		0x00,		0x00 },	// 0x7A sysVKC_F11
	{ 0x00,		0x00,		0x00 },	// 0x7B sysVKC_F12
	{ 0x00,		0x00,		0x00 },	// 0x7C sysVKC_F13
	{ 0x00,		0x00,		0x00 },	// 0x7D sysVKC_F14
	{ 0x00,		0x00,		0x00 },	// 0x7E sysVKC_F15
	{ 0x00,		0x00,		0x00 },	// 0x7F sysVKC_F16


	{ 0x00,		0x00,		0x00 },	// 0x80 sysVKC_F17
	{ 0x00,		0x00,		0x00 },	// 0x81 sysVKC_F18
	{ 0x00,		0x00,		0x00 },	// 0x82 sysVKC_F19
	{ 0x00,		0x00,		0x00 },	// 0x83 sysVKC_F20
	{ 0x00,		0x00,		0x00 },	// 0x84 sysVKC_F21
	{ 0x00,		0x00,		0x00 },	// 0x85 sysVKC_F22
	{ 0x00,		0x00,		0x00 },	// 0x86 sysVKC_F23
	{ 0x00,		0x00,		0x00 },	// 0x87 sysVKC_F24
	{ 0x00,		0x00,		0x00 },	// 0x88 sysVKC_F24
	{ 0x00,		0x00,		0x00 },	// 0x89
	{ 0x00,		0x00,		0x00 },	// 0x8A
	{ 0x00,		0x00,		0x00 },	// 0x8B
	{ 0x00,		0x00,		0x00 },	// 0x8C
	{ 0x00,		0x00,		0x00 },	// 0x8D
	{ 0x00,		0x00,		0x00 },	// 0x8E
	{ 0x00,		0x00,		0x00 },	// 0x8F

	{ 0x00,		0x00,		0x00 },	// 0x90 sysVKC_NUMLOCK
	{ 0x00,		0x00,		0x00 },	// 0x91 sysVKC_SCROLL
	{ 0x00,		0x00,		0x00 },	// 0x92
	{ 0x00,		0x00,		0x00 },	// 0x93
	{ 0x00,		0x00,		0x00 },	// 0x94
	{ 0x00,		0x00,		0x00 },	// 0x95
	{ 0x00,		0x00,		0x00 },	// 0x96
	{ 0x00,		0x00,		0x00 },	// 0x97
	{ 0x00,		0x00,		0x00 },	// 0x98
	{ 0x00,		0x00,		0x00 },	// 0x99
	{ 0x00,		0x00,		0x00 },	// 0x9A
	{ 0x00,		0x00,		0x00 },	// 0x9B
	{ 0x00,		0x00,		0x00 },	// 0x9C
	{ 0x00,		0x00,		0x00 },	// 0x9D
	{ 0x00,		0x00,		0x00 },	// 0x9E
	{ 0x00,		0x00,		0x00 },	// 0x9F

	{ 0x00,		0x00,		0x00 },	// 0xA0 sysVKC_LSHIFT
	{ 0x00,		0x00,		0x00 },	// 0xA1 sysVKC_RSHIFT
	{ 0x00,		0x00,		0x00 },	// 0xA2 sysVKC_LCONTROL
	{ 0x00,		0x00,		0x00 },	// 0xA3 sysVKC_RCONTROL
	{ 0x00,		0x00,		0x00 },	// 0xA4 sysVKC_LMENU
	{ 0x00,		0x00,		0x00 },	// 0xA5 sysVKC_RMENU
	{ 0x00,		0x00,		0x00 },	// 0xA6 sysVKC_BROWSER_BACK
	{ 0x00,		0x00,		0x00 },	// 0xA7 sysVKC_BROWSER_FORWARD
	{ 0x00,		0x00,		0x00 },	// 0xA8 sysVKC_BROWSER_REFRESH
	{ 0x00,		0x00,		0x00 },	// 0xA9 sysVKC_BROWSER_STOP
	{ 0x00,		0x00,		0x00 },	// 0xAA sysVKC_BROWSER_SEARCH
	{ 0x00,		0x00,		0x00 },	// 0xAB sysVKC_BROWSER_FAVORITES
	{ 0x00,		0x00,		0x00 },	// 0xAC sysVKC_BROWSER_HOME
	{ 0x00,		0x00,		0x00 },	// 0xAD sysVKC_VOLUME_MUTE
	{ 0x00,		0x00,		0x00 },	// 0xAE sysVKC_VOLUME_DOWN
	{ 0x00,		0x00,		0x00 },	// 0xAF sysVKC_VOLUME_UP

	{ 0x00,		0x00,		0x00 },	// 0xB0 sysVKC_MEDIA_NEXT_TRACK
	{ 0x00,		0x00,		0x00 },	// 0xB1 sysVKC_MEDIA_PREV_TRACK
	{ 0x00,		0x00,		0x00 },	// 0xB2 sysVKC_MEDIA_STOP
	{ 0x00,		0x00,		0x00 },	// 0xB3 sysVKC_MEDIA_PLAY_PAUSE
	{ 0x00,		0x00,		0x00 },	// 0xB4 sysVKC_MEDIA_LAUNCH_MAIL
	{ 0x00,		0x00,		0x00 },	// 0xB5 sysVKC_MEDIA_LAUNCH_MEDIA_SELECT
	{ 0x00,		0x00,		0x00 },	// 0xB6 sysVKC_MEDIA_LAUNCH_APP1
	{ 0x00,		0x00,		0x00 },	// 0xB7 sysVKC_MEDIA_LAUNCH_APP2
	{ 0x00,		0x00,		0x00 },	// 0xB8
	{ 0x00,		0x00,		0x00 },	// 0xB9
	{ 0xE9,		0xC9,		 '$' },	// 0xBA sysVKC_OEM_1
	{ 0xF3,		0xD3,		0x00 },	// 0xBB sysVKC_OEM_PLUS
	{ ',',		 '?',		 ';' },	// 0xBC sysVKC_OEM_COMMA
	{  '-',		 '_',		 '*' },	// 0xBD sysVKC_OEM_MINUS
	{  '.',		 ':',		0x00 },	// 0xBE sysVKC_OEM_PERIOD
	{ 0xFC,		0xDC,		0x00 },	// 0xBF sysVKC_OEM_2

	{ 0xF6,		0xD6,		0x00 },	// 0xC0 sysVKC_OEM_3
	{ 0x00,		0x00,		0x00 },	// 0xC1
	{ 0x00,		0x00,		0x00 },	// 0xC2
	{ 0x00,		0x00,		0x00 },	// 0xC3
	{ 0x00,		0x00,		0x00 },	// 0xC4
	{ 0x00,		0x00,		0x00 },	// 0xC5
	{ 0x00,		0x00,		0x00 },	// 0xC6
	{ 0x00,		0x00,		0x00 },	// 0xC7
	{ 0x00,		0x00,		0x00 },	// 0xC8
	{ 0x00,		0x00,		0x00 },	// 0xC9
	{ 0x00,		0x00,		0x00 },	// 0xCA
	{ 0x00,		0x00,		0x00 },	// 0xCB
	{ 0x00,		0x00,		0x00 },	// 0xCC
	{ 0x00,		0x00,		0x00 },	// 0xCD
	{ 0x00,		0x00,		0x00 },	// 0xCE
	{ 0x00,		0x00,		0x00 },	// 0xCF

	{ 0x00,		0x00,		0x00 },	// 0xD0
	{ 0x00,		0x00,		0x00 },	// 0xD1
	{ 0x00,		0x00,		0x00 },	// 0xD2
	{ 0x00,		0x00,		0x00 },	// 0xD3
	{ 0x00,		0x00,		0x00 },	// 0xD4
	{ 0x00,		0x00,		0x00 },	// 0xD5
	{ 0x00,		0x00,		0x00 },	// 0xD6
	{ 0x00,		0x00,		0x00 },	// 0xD7
	{ 0x00,		0x00,		0x00 },	// 0xD8
	{ 0x00,		0x00,		0x00 },	// 0xD9
	{ 0x00,		0x00,		0x00 },	// 0xDA
	{ 0xF5,		0xD5,		0x00 },	// 0xDB sysVKC_OEM_4
	{ 0xFB,		0xDB,		0x00 },	// 0xDC sysVKC_OEM_5
	{ 0xFA,		0xDA,		0x00 },	// 0xDD sysVKC_OEM_6 
	{ 0xE1,		0xC1,		0x00 },	// 0xDE sysVKC_OEM_7
	{ 0x00,		0x00,		0x00 },	// 0xDF sysVKC_OEM_8

	{ 0x00,		0x00,		0x00 },	// 0xE0
	{ 0x00,		0x00,		0x00 },	// 0xE1
	{ 0xED,		0xCD,		 '<' },	// 0xE2 sysVKC_OEM_102
	{ 0x00,		0x00,		0x00 },	// 0xE3
	{ 0x00,		0x00,		0x00 },	// 0xE4
	{ 0x00,		0x00,		0x00 },	// 0xE5 sysVKC_PROCESSKEY
	{ 0x00,		0x00,		0x00 },	// 0xE6
	{ 0x00,		0x00,		0x00 },	// 0xE7 sysVKC_PACKET
	{ 0x00,		0x00,		0x00 },	// 0xE8
	{ 0x00,		0x00,		0x00 },	// 0xE9
	{ 0x00,		0x00,		0x00 },	// 0xEA
	{ 0x00,		0x00,		0x00 },	// 0xEB
	{ 0x00,		0x00,		0x00 },	// 0xEC
	{ 0x00,		0x00,		0x00 },	// 0xED
	{ 0x00,		0x00,		0x00 },	// 0xEE
	{ 0x00,		0x00,		0x00 },	// 0xEF

	{ 0x00,		0x00,		0x00 },	// 0xF0
	{ 0x00,		0x00,		0x00 },	// 0xF1
	{ 0x00,		0x00,		0x00 },	// 0xF2
	{ 0x00,		0x00,		0x00 },	// 0xF3
	{ 0x00,		0x00,		0x00 },	// 0xF4
	{ 0x00,		0x00,		0x00 },	// 0xF5
	{ 0x00,		0x00,		0x00 },	// 0xF6 sysVKC_ATTN
	{ 0x00,		0x00,		0x00 },	// 0xF7 sysVKC_CRSEL
	{ 0x00,		0x00,		0x00 },	// 0xF8 sysVKC_EXSEL
	{ 0x00,		0x00,		0x00 },	// 0xF9 sysVKC_EREOF
	{ 0x00,		0x00,		0x00 },	// 0xFA sysVKC_PLAY
	{ 0x00,		0x00,		0x00 },	// 0xFB sysVKC_ZOOM
	{ 0x00,		0x00,		0x00 },	// 0xFC sysVKC_NONAME
	{ 0x00,		0x00,		0x00 },	// 0xFD sysVKC_PA1
	{ 0x00,		0x00,		0x00 },	// 0xFE sysVKC_OEM_CLEAR
	{ 0x00,		0x00,		0x00 }	// 0xFF

};

///////////////////////////////////////////////////////////////////////////////
/// @brief Converts keyboard character events to Win1250 encoded character
/// @param in_device_number Number of the device which generated the event
/// @param in_event_category Category of the event. Only 'press' and 'release' events will be processed
/// @param in_event_type Type of the event. Only keyboard events will be processed.
/// @param in_event_param Scan code of the key
/// @return Win1250 encoded character or sysVKC_NULL if the pressed key is not a valid Win1250 character
sysChar cpConvertWin1250(uint8_t in_device_number, sysUserInputEventCategory in_event_category, sysUserInputEventType in_event_type, uint32_t in_event_param)
{
	uint32_t modifier_state = sysUserInputGetModifiersState(in_device_number);

	if (in_event_type == sysUIET_Key && (in_event_category == sysUIEC_Pressed || in_event_category == sysUIEC_Released))
	{
		if (in_event_param < 256)
		{
			if((modifier_state & (sysMS_ALT | sysMS_CTRL)) == (sysMS_ALT | sysMS_CTRL))
				return l_keymap_table[in_event_param].AltGr;
			else if ((modifier_state & sysMS_SHIFT) == sysMS_SHIFT)
				return l_keymap_table[in_event_param].Shift;
			else
				return l_keymap_table[in_event_param].CharacterCode;
		}
	}

	return sysVKC_NULL;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Converts win1250 encoded characters to uppercase character
/// @param in_char Character to convert
/// @return Converted character
sysChar cpToUpperWin1250(sysChar in_char)
{
	if (in_char >= 'a' && in_char <= 'z')
		in_char -= 'a' - 'A';

	switch ((uint8_t)in_char)
	{
		case 0xE9:		// e'
			in_char = 0xC9;
			break;

		case 0xE1:		// a'
			in_char = 0xC1;
			break;

		case 0xFC:		// u:
			in_char = 0xDC;
			break;

		case 0xF6:		// o:
			in_char = 0xD6;
			break;

		case 0xF3:		// o'
			in_char = 0xD3;
			break;

		case 0xF5:		// o"
			in_char = 0xD5;
			break;

		case 0xFA:		// u'
			in_char = 0xDA;
			break;

		case 0xED:		// i'
			in_char = 0xCD;
			break;
	}

	return in_char;
}



