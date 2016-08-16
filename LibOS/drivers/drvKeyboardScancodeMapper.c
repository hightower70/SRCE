#include <drvKeyboardScancodeMapper.h>

static const uint16_t l_scan_code_to_virtual_key_table[];

//******************************************************************************
//  macros to identify special charaters(other than Digits and Alphabets)
//******************************************************************************
#define Symbol_Exclamation              (0x1E)
#define Symbol_AT                       (0x1F)
#define Symbol_Pound                    (0x20)
#define Symbol_Dollar                   (0x21)
#define Symbol_Percentage               (0x22)
#define Symbol_Cap                      (0x23)
#define Symbol_AND                      (0x24)
#define Symbol_Star                     (0x25)
#define Symbol_NormalBracketOpen        (0x26)
#define Symbol_NormalBracketClose       (0x27)

#define Symbol_Return                   (0x28)
#define Symbol_Escape                   (0x29)
#define Symbol_Backspace                (0x2A)
#define Symbol_Tab                      (0x2B)
#define Symbol_Space                    (0x2C)
#define Symbol_HyphenUnderscore         (0x2D)
#define Symbol_EqualAdd                 (0x2E)
#define Symbol_BracketOpen              (0x2F)
#define Symbol_BracketClose             (0x30)
#define Symbol_BackslashOR              (0x31)
#define Symbol_SemiColon                (0x33)
#define Symbol_InvertedComma            (0x34)
#define Symbol_Tilde                    (0x35)
#define Symbol_CommaLessThan            (0x36)
#define Symbol_PeriodGreaterThan        (0x37)
#define Symbol_FrontSlashQuestion       (0x38)


/****************************************************************************
  Function:
    BYTE App_HID2ASCII(BYTE a)
  Description:
    This function converts the HID code of the key pressed to coressponding
    ASCII value. For Key strokes like Esc, Enter, Tab etc it returns 0.

  Precondition:
    None

  Parameters:
    BYTE a          -   HID code for the key pressed

  Return Values:
    BYTE            -   ASCII code for the key pressed

  Remarks:
    None
***************************************************************************/
uint16_t drvScancodeToKeycode(uint8_t in_scan_code, uint8_t in_modifier_state, uint8_t in_lock_state)
{
	return l_scan_code_to_virtual_key_table[in_scan_code];
}
#if 0
	uint8_t key_code;
	bool shift_state = false;
	static uint8_t shifted_numbers[9] = { '\'', '\"', '+', '!', '%', '/', '=', '(', ')' };

	if((in_modifier_state & (sysVKC_MF_LEFT_SHIFT | sysVKC_MF_RIGHT_SHIFT)) != 0 )
  {
		shift_state = true;
  }

	// handle numbers
  if(in_scan_code >= 0x1E && in_scan_code < 0x27)
  {
		if(shift_state)
    {
			return shifted_numbers[in_scan_code - 0x1E];
    }
    else
    {
			return(in_scan_code + 0x13);
    }
  }

	// convert letters
  if( in_scan_code >= 0x04 && in_scan_code <= 0x1D)
  {
		if((((in_lock_state & sysVKC_LS_CAPS_LOCK) != 0)&&(((in_modifier_state & sysVKC_MF_LEFT_SHIFT) == 0)&& ((in_modifier_state & sysVKC_MF_RIGHT_SHIFT) == 0)))
     ||(((in_lock_state & sysVKC_LS_CAPS_LOCK == 0) && (((in_modifier_state & sysVKC_MF_LEFT_SHIFT) != 0)) || ((in_modifier_state & sysVKC_MF_RIGHT_SHIFT) != 0))))
           return(in_scan_code + 0x3d); // return capital
      else
           return(in_scan_code + 0x5d); // return small case
  }

	// convert numpad numbers
  if((in_scan_code>=0x59 && in_scan_code <= 0x61)&&((in_lock_state & sysVKC_LS_NUM_LOCK) != 0))
  {
		return(in_scan_code-0x28);
  }

	switch(in_scan_code)
	{
		case 0x2a:	// backspace
			return 0x08;

		case 0x2b:	// tab
			return 0x09;

		case 0x28:	// enter
			return 0x0d;

		case 0x29:	// ESC
			return 0x1b;

		case 0x2c:	// Space
			return ' ';

		case 0x35:	// 0
			if(shift_state)
				return 0xa7;
			else
				return '0';
	}

#if 0


   if((a==0x62) &&(NUM_Lock_Pressed == 1)) return(0x30);





   if(a>=0x2D && a<=0x38)
    {
        switch(a)
        {
            case Symbol_HyphenUnderscore:
                if(!ShiftkeyStatus)
                    AsciiVal = 0x2D;
                else
                    AsciiVal = 0x5F;
                break;
            case Symbol_EqualAdd:
                if(!ShiftkeyStatus)
                    AsciiVal = 0x3D;
                else
                    AsciiVal = 0x2B;
                break;
            case Symbol_BracketOpen:
                if(!ShiftkeyStatus)
                    AsciiVal = 0x5B;
                else
                    AsciiVal = 0x7B;
                break;
            case Symbol_BracketClose:
                if(!ShiftkeyStatus)
                    AsciiVal = 0x5D;
                else
                    AsciiVal = 0x7D;
                break;
            case Symbol_BackslashOR:
                if(!ShiftkeyStatus)
                    AsciiVal = 0x5C;
                else
                    AsciiVal = 0x7C;
                break;
            case Symbol_SemiColon:
                if(!ShiftkeyStatus)
                    AsciiVal = 0x3B;
                else
                    AsciiVal = 0x3A;
                break;
            case Symbol_InvertedComma:
                if(!ShiftkeyStatus)
                    AsciiVal = 0x27;
                else
                    AsciiVal = 0x22;
                break;
            case Symbol_Tilde:
                if(!ShiftkeyStatus)
                    AsciiVal = 0x60;
                else
                    AsciiVal = 0x7E;
                break;
            case Symbol_CommaLessThan:
                if(!ShiftkeyStatus)
                    AsciiVal = 0x2C;
                else
                    AsciiVal = 0x3C;
                break;
            case Symbol_PeriodGreaterThan:
                if(!ShiftkeyStatus)
                    AsciiVal = 0x2E;
                else
                    AsciiVal = 0x3E;
                break;
            case Symbol_FrontSlashQuestion:
                if(!ShiftkeyStatus)
                    AsciiVal = 0x2F;
                else
                    AsciiVal = 0x3F;
                break;
            default:
                break;
        }
        return(AsciiVal);
    }
#endif
   return(0);
}
#endif

static const uint16_t l_scan_code_to_virtual_key_table[256] =
{
	sysVKC_INVALID,			// 0X00 Reserved (no event indicated)
	sysVKC_INVALID,			// 0x01	Keyboard ErrorRollOver
	sysVKC_INVALID,			// 0x02	Keyboard POSTFail
	sysVKC_INVALID,			// 0x03	Keyboard ErrorUndefined
	sysVKC_A,						// 0x04	Keyboard a and A
	sysVKC_B,						// 0x05	Keyboard b and B
	sysVKC_C,						// 0x06	Keyboard c and C
	sysVKC_D,						// 0x07	Keyboard d and D
	sysVKC_E,						// 0x08	Keyboard e and E
	sysVKC_F,						// 0x09	Keyboard f and F
	sysVKC_G,						// 0x0A	Keyboard g and G
	sysVKC_H,						// 0x0B	Keyboard h and H
	sysVKC_I,						// 0x0C	Keyboard i and I
	sysVKC_J,						// 0x0D	Keyboard j and J
	sysVKC_K,						// 0x0E	Keyboard k and K
	sysVKC_L,						// 0x0F	Keyboard l and L
	sysVKC_M,						// 0x10	Keyboard m and M
	sysVKC_N,						// 0x11	Keyboard n and N
	sysVKC_O,						// 0x12	Keyboard o and O
	sysVKC_P,						// 0x13	Keyboard p and P
	sysVKC_Q,						// 0x14	Keyboard q and Q
	sysVKC_R,						// 0x15	Keyboard r and R
	sysVKC_S,						// 0x16	Keyboard s and S
	sysVKC_T,						// 0x17	Keyboard t and T
	sysVKC_U,						// 0x18	Keyboard u and U
	sysVKC_V,						// 0x19	Keyboard v and V
	sysVKC_W,						// 0x1A	Keyboard w and W
	sysVKC_X,						// 0x1B	Keyboard x and X
	sysVKC_Y,						// 0x1C	Keyboard y and Y
	sysVKC_Z,						// 0x1D	Keyboard z and Z
	sysVKC_1,						// 0x1E	Keyboard 1 and !
	sysVKC_2,						// 0x1F	Keyboard 2 and @
	sysVKC_3,						// 0x20	Keyboard 3 and #
	sysVKC_4,						// 0x21	Keyboard 4 and $
	sysVKC_5,						// 0x22	Keyboard 5 and %
	sysVKC_6,						// 0x23	Keyboard 6 and ^
	sysVKC_7,						// 0x24	Keyboard 7 and &
	sysVKC_8,						// 0x25	Keyboard 8 and *
	sysVKC_9,						// 0x26	Keyboard 9 and (
	sysVKC_0,						// 0x27	Keyboard 0 and )
	sysVKC_RETURN,			// 0x28	Keyboard Return (ENTER)
	sysVKC_ESCAPE,			// 0x29	Keyboard ESCAPE
	sysVKC_BACK,				// 0x2A	Keyboard DELETE (Backspace)
	sysVKC_TAB,					// 0x2B	Keyboard Tab
	sysVKC_SPACE,				// 0x2C	Keyboard Spacebar
	sysVKC_OEM_MINUS,		// 0x2D	Keyboard - and (underscore)
	sysVKC_OEM_PLUS,		// 0x2E	Keyboard = and +
	sysVKC_OEM_4,				// 0x2F	Keyboard [ and {
	sysVKC_OEM_6,				// 0x30	Keyboard ] and }
	sysVKC_OEM_5,				// 0x31	Keyboard \ and |
	sysVKC_OEM_3,				// 0x32	Keyboard Non-US # and ~
	sysVKC_OEM_1,				// 0x33	Keyboard ; and :
	sysVKC_OEM_7,				// 0x34	Keyboard ' and "
	sysVKC_OEM_3,				// 0x35	Keyboard Grave Accent and Tilde
	sysVKC_OEM_COMMA,		// 0x36	Keyboard, and <
	sysVKC_OEM_PERIOD,	// 0x37	Keyboard . and >
	sysVKC_OEM_2,				// 0x38	Keyboard / and ?
	sysVKC_CAPITAL,			// 0x39	Keyboard Caps Lock
	sysVKC_F1,					// 0x3A	Keyboard F1
	sysVKC_F2,					// 0x3B	Keyboard F2
	sysVKC_F3,					// 0x3C	Keyboard F3
	sysVKC_F4,					// 0x3D	Keyboard F4
	sysVKC_F5,					// 0x3E	Keyboard F5
	sysVKC_F6,					// 0x3F	Keyboard F6
	sysVKC_F7,					// 0x40	Keyboard F7
	sysVKC_F8,					// 0x41	Keyboard F8
	sysVKC_F9,					// 0x42	Keyboard F9
	sysVKC_F10,					// 0x43	Keyboard F10
	sysVKC_F11,					// 0x44	Keyboard F11
	sysVKC_F12,					// 0x45	Keyboard F12
	sysVKC_PRINT,				// 0x46	Keyboard PrintScreen
	sysVKC_SCROLL,			// 0x47	Keyboard Scroll Lock
	sysVKC_PAUSE,				// 0x48	Keyboard Pause
	sysVKC_INSERT | sysVKC_SPECIAL_KEY_FLAG,			// 0x49	Keyboard Insert
	sysVKC_HOME | sysVKC_SPECIAL_KEY_FLAG,				// 0x4A	Keyboard Home
	sysVKC_PRIOR | sysVKC_SPECIAL_KEY_FLAG,				// 0x4B	Keyboard PageUp
	sysVKC_DELETE | sysVKC_SPECIAL_KEY_FLAG,			// 0x4C	Keyboard Delete Forward
	sysVKC_END | sysVKC_SPECIAL_KEY_FLAG,					// 0x4D	Keyboard End
	sysVKC_NEXT | sysVKC_SPECIAL_KEY_FLAG,				// 0x4E	Keyboard PageDown
	sysVKC_RIGHT | sysVKC_SPECIAL_KEY_FLAG,				// 0x4F	Keyboard RightArrow
	sysVKC_LEFT | sysVKC_SPECIAL_KEY_FLAG,				// 0x50	Keyboard LeftArrow
	sysVKC_DOWN | sysVKC_SPECIAL_KEY_FLAG,				// 0x51	Keyboard DownArrow
	sysVKC_UP | sysVKC_SPECIAL_KEY_FLAG,					// 0x52	Keyboard UpArrow
	sysVKC_NUMLOCK | sysVKC_SPECIAL_KEY_FLAG,			// 0x53	Keypad Num Lock and Clear
	sysVKC_DIVIDE | sysVKC_SPECIAL_KEY_FLAG,			// 0x54	Keypad /
	sysVKC_MULTIPLY,															// 0x55	Keypad *
	sysVKC_SUBTRACT,															// 0x56	Keypad -
	sysVKC_ADD,																		// 0x57	Keypad +
	sysVKC_RETURN | sysVKC_SPECIAL_KEY_FLAG,			// 0x58	Keypad ENTER
	sysVKC_NUMPAD1,																// 0x59	Keypad 1 and End
	sysVKC_NUMPAD2,																// 0x5A	Keypad 2 and Down Arrow
	sysVKC_NUMPAD3,																// 0x5B	Keypad 3 and PageDn
	sysVKC_NUMPAD4,																// 0x5C	Keypad 4 and Left Arrow
	sysVKC_NUMPAD5,																// 0x5D	Keypad 5
	sysVKC_NUMPAD6,																// 0x5E	Keypad 6 and Right Arrow
	sysVKC_NUMPAD7,																// 0x5F	Keypad 7 and Home
	sysVKC_NUMPAD8,																// 0x60	Keypad 8 and Up Arrow
	sysVKC_NUMPAD9,																// 0x61	Keypad 9 and PageUp
	sysVKC_NUMPAD0,																// 0x62	Keypad 0 and Insert
	sysVKC_DECIMAL,																// 0x63	Keypad . and Delete
	sysVKC_INVALID,																// 0x64	Keyboard Non-US \ and |
	sysVKC_APPS,																	// 0x65	Keyboard Application
	sysVKC_INVALID,																// 0x66	Keyboard Power
	sysVKC_INVALID,																// 0x67	Keypad =
	sysVKC_F13,																		// 0x68	Keyboard F13
	sysVKC_F14,																		// 0x69	Keyboard F14
	sysVKC_F15,																		// 0x6A	Keyboard F15
	sysVKC_F16,																		// 0x6B	Keyboard F16
	sysVKC_F17,																		// 0x6C	Keyboard F17
	sysVKC_F18,																		// 0x6D	Keyboard F18
	sysVKC_F19,																		// 0x6E	Keyboard F19
	sysVKC_F20,																		// 0x6F	Keyboard F20
	sysVKC_F21,																		// 0x70	Keyboard F21
	sysVKC_F22,																		// 0x71	Keyboard F22
	sysVKC_F23,																		// 0x72	Keyboard F23
	sysVKC_F24,																		// 0x73	Keyboard F24
	sysVKC_EXECUTE,																// 0x74	Keyboard Execute
	sysVKC_HELP,																	// 0x75	Keyboard Help
	sysVKC_MENU,																	// 0x76	Keyboard Menu
	sysVKC_SELECT,																// 0x77	Keyboard Select
	sysVKC_INVALID,																// 0x78	Keyboard Stop
	sysVKC_INVALID,																// 0x79	Keyboard Again
	sysVKC_INVALID,																// 0x7A	Keyboard Undo
	sysVKC_INVALID,																// 0x7B	Keyboard Cut
	sysVKC_INVALID,																// 0x7C	Keyboard Copy
	sysVKC_INVALID,																// 0x7D	Keyboard Paste
	sysVKC_INVALID,																// 0x7E	Keyboard Find
	sysVKC_VOLUME_MUTE,														// 0x7F	Keyboard Mute
	sysVKC_VOLUME_UP,															// 0x80	Keyboard Volume Up
	sysVKC_VOLUME_DOWN,														// 0x81	Keyboard Volume Down
	sysVKC_CAPITAL,																// 0x82	Keyboard Locking Caps Lock
	sysVKC_NUMLOCK,																// 0x83	Keyboard Locking Num Lock
	sysVKC_SCROLL,																// 0x84	Keyboard Locking Scroll Lock
	sysVKC_DECIMAL,																// 0x85	Keypad Comma
	sysVKC_INVALID,																// 0x86	Keypad Equal Sign
	sysVKC_INVALID,																// 0x87	Keyboard International1
	sysVKC_INVALID,																// 0x88	Keyboard International2
	sysVKC_INVALID,																// 0x89	Keyboard International3
	sysVKC_INVALID,																// 0x8A	Keyboard International4
	sysVKC_INVALID,																// 0x8B	Keyboard International5
	sysVKC_INVALID,																// 0x8C	Keyboard International6
	sysVKC_INVALID,																// 0x8D	Keyboard International7
	sysVKC_INVALID,																// 0x8E	Keyboard International8
	sysVKC_INVALID,																// 0x8F	Keyboard International9
	sysVKC_INVALID,																// 0x90	Keyboard LANG1
	sysVKC_INVALID,																// 0x91	Keyboard LANG2
	sysVKC_INVALID,																// 0x92	Keyboard LANG3
	sysVKC_INVALID,																// 0x93	Keyboard LANG4
	sysVKC_INVALID,																// 0x94	Keyboard LANG5
	sysVKC_INVALID,																// 0x95	Keyboard LANG6
	sysVKC_INVALID,																// 0x96	Keyboard LANG7
	sysVKC_INVALID,																// 0x97	Keyboard LANG8
	sysVKC_INVALID,																// 0x98	Keyboard LANG9
	sysVKC_INVALID,																// 0x99	Keyboard Alternate Erase
	sysVKC_ATTN,																	// 0x9A	Keyboard SysReq/Attention
	sysVKC_INVALID,																// 0x9B	Keyboard Cancel
	sysVKC_INVALID,																// 0x9C	Keyboard Clear
	sysVKC_INVALID,																// 0x9D	Keyboard Prior
	sysVKC_INVALID,																// 0x9E	Keyboard Return
	sysVKC_INVALID,																// 0x9F	Keyboard Separator
	sysVKC_INVALID,																// 0xA0	Keyboard Out
	sysVKC_INVALID,																// 0xA1	Keyboard Oper
	sysVKC_INVALID,																// 0xA2	Keyboard Clear/Again
	sysVKC_INVALID,																// 0xA3	Keyboard CrSel/Props
	sysVKC_INVALID,																// 0xA4	Keyboard ExSel
	sysVKC_LCONTROL,															// 0xE0	Keyboard LeftControl
	sysVKC_LSHIFT,																// 0xE1	Keyboard LeftShift
	sysVKC_LMENU,																	// 0xE2	Keyboard LeftAlt
	sysVKC_LWIN,																	// 0xE3	Keyboard Left GUI
	sysVKC_RCONTROL,															// 0xE4	Keyboard RightControl
	sysVKC_RSHIFT,																// 0xE5	Keyboard RightShift
	sysVKC_RMENU,																	// 0xE6	Keyboard RightAlt
	sysVKC_RWIN,																	// 0xE7	Keyboard Right GUI
	sysVKC_INVALID,																// 0xE8  Unused
	sysVKC_INVALID,																// 0xE9  Unused
	sysVKC_INVALID,																// 0xEA  Unused
	sysVKC_INVALID,																// 0xEB  Unused
	sysVKC_INVALID,																// 0xEC  Unused
	sysVKC_INVALID,																// 0xED  Unused
	sysVKC_INVALID,																// 0xEE  Unused
	sysVKC_INVALID,																// 0xEF  Unused
	sysVKC_INVALID,																// 0xF0  Unused
	sysVKC_INVALID,																// 0xF1  Unused
	sysVKC_INVALID,																// 0xF2  Unused
	sysVKC_INVALID,																// 0xF3  Unused
	sysVKC_INVALID,																// 0xF4  Unused
	sysVKC_INVALID,																// 0xF5  Unused
	sysVKC_INVALID,																// 0xF6  Unused
	sysVKC_INVALID,																// 0xF7  Unused
	sysVKC_INVALID,																// 0xF8  Unused
	sysVKC_INVALID,																// 0xF9  Unused
	sysVKC_INVALID,																// 0xFA  Unused
	sysVKC_INVALID,																// 0xFB  Unused
	sysVKC_INVALID,																// 0xFC  Unused
	sysVKC_INVALID,																// 0xFD  Unused
	sysVKC_INVALID,																// 0xFE  Unused
	sysVKC_INVALID																// 0xFF  Unused
};