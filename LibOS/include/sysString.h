/*****************************************************************************/
/* Safe String handling functions                                            */
/*                                                                           */
/* Copyright (C) 2014-2015 Laszlo Arvai                                      */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

#ifndef __sysSafeString_h
#define __sysSafeString_h

///////////////////////////////////////////////////////////////////////////////
// Includes
#include <sysTypes.h>

///////////////////////////////////////////////////////////////////////////////
// Constants

// string conversion options
#define TS_NO_ZERO_BLANKING (1<<1)
#define TS_RIGHT_ADJUSTMENT (1<<2)
#define TS_DISPLAY_MINUS_SIGN (1<<3)

// parser options
#define TS_ACCEPT_MINUS_SIGN (1<<1)

#define charIsDigit(x) ((x)>='0' && ((x))<='9')
#define charIsHexDigit(x) (((x)>='0' && ((x))<='9') || ((x)>='a' && ((x))<='f') || ((x)>='A' && ((x))<='F'))

// line end types
typedef enum
{
	sysString_LE_CR,
	sysString_LE_LF,
	sysString_LE_CRLF,
	sysString_LE_LFCR
}  sysStringLineEnd;

///////////////////////////////////////////////////////////////////////////////
// ASCII control codes
#define sysASCII_NUL	0
#define sysASCII_SOH	1
#define sysASCII_STX	2
#define sysASCII_ETX	3
#define sysASCII_EOT 4
#define sysASCII_ENQ	5
#define sysASCII_0K	6
#define sysASCII_BEL	7
#define sysASCII_BS	8
#define sysASCII_HT	9
#define sysASCII_LF	10
#define sysASCII_VT	11
#define sysASCII_FF	12
#define sysASCII_CR	13
#define sysASCII_SO	14
#define sysASCII_SI	15
#define sysASCII_DLE 16
#define sysASCII_DC1	17
#define sysASCII_DC2	18
#define sysASCII_DC3	19
#define sysASCII_DC4	20
#define sysASCII_NAK	21
#define sysASCII_SYN	22
#define sysASCII_ETB	23
#define sysASCII_CAN	24
#define sysASCII_EM	25
#define sysASCII_SUB	26
#define sysASCII_ESC	27
#define sysASCII_FS	28
#define sysASCII_GS	29
#define sysASCII_RS	30
#define sysASCII_US	31

///////////////////////////////////////////////////////////////////////////////
// Functions
sysStringLength strGetLength( sysString in_string );
//sysStringLength strGetConstLength( sysConstString in_string );
sysASCIIChar strUnicodeToASCIIChar( sysUnicodeChar in_unicode );
//
sysChar strCharToUpper( sysChar in_char );
sysChar strCharToLower( sysChar in_char );

// comparision
int strCompareConstString(sysString in_string1, sysConstString in_string2);
int strCompareConstStringNoCase(sysString in_string1, sysConstString in_string2);
int strCompareString(sysString in_string1, sysString in_string2);
int strCompareStringNoCase(sysString in_string1, sysString in_string2);


// String concatenation, copy, fill functions
sysStringLength strCopyString( sysString in_destination, sysStringLength in_destination_size, sysStringLength in_destination_pos, sysString in_source );
sysStringLength strCopyConstString( sysString in_destination, sysStringLength in_destination_size, sysStringLength in_destination_pos, sysConstString in_source );
//void strInt16ToString( sysString in_buffer, uint8_t in_buffer_length, dosInt16 in_value, uint8_t in_field_length, uint8_t in_precision, uint8_t in_options );
void strFillString( sysString in_destination, sysStringLength in_destination_size, sysChar in_char, sysStringLength in_start_pos, sysStringLength in_required_length );
sysStringLength strFindChar( sysString in_buffer, sysChar in_char );
sysStringLength strFindLastChar( sysString in_buffer, sysChar in_char );
//void strSetLength( sysString in_destination, sysStringLength in_destination_size, sysChar in_char, sysStringLength in_required_length );
//void strAppendInt16ToString( sysString in_buffer, uint8_t in_buffer_length, dosInt16 in_value, uint8_t in_field_length, uint8_t in_precision, uint8_t in_options );
sysStringLength strAppendConstString( sysString in_destination, sysStringLength in_destination_size, sysConstString in_source );
//void strAppendString( sysString in_destination, sysStringLength in_destination_size, sysString in_source );
//sysStringLength strCopyCharacter(sysString in_destination, sysStringLength in_destination_size, sysStringLength in_pos, sysChar in_char);

// hex conversion
//sysChar strNibbleToHexDigit( uint8_t in_nibble );
//sysStringLength strWordToHexStringPos( sysString in_buffer, sysStringLength in_buffer_length, sysStringLength in_pos, uint16_t in_value );
//void strWordToHexString( sysString in_buffer, sysStringLength in_buffer_length, uint16_t in_value );
//void strAppendWordToHexString( sysString in_buffer, sysStringLength in_buffer_length, uint16_t in_value );

// byte conversion
//void strByteToHexStringPos( sysString in_buffer, sysStringLength in_buffer_length, sysStringLength in_pos, uint16_t in_value );
//void strByteToHexString( sysString in_buffer, sysStringLength in_buffer_length, uint16_t in_value );
//void strAppendByteToHexString( sysString in_buffer, sysStringLength in_buffer_length, uint16_t in_value );

// word conversion
sysStringLength strWordToStringPos( sysString in_buffer, sysStringLength in_buffer_length, sysStringLength in_pos, uint16_t in_value, uint8_t in_field_length, uint8_t in_precision, uint8_t in_options );
sysStringLength strWordToString( sysString in_buffer, sysStringLength in_buffer_length, uint16_t in_value, uint8_t in_field_length, uint8_t in_precision, uint8_t in_options );
sysStringLength strAppendWordToString( sysString in_buffer, sysStringLength in_buffer_length, uint16_t in_value, uint8_t in_field_length, uint8_t in_precision, uint8_t in_options );

// dword conversion
sysStringLength strDWordToString( sysString in_buffer, sysStringLength in_buffer_length, uint32_t in_value, uint8_t in_field_length, uint8_t in_precision, uint8_t in_options );
sysStringLength strDWordToHexStringPos( sysString in_buffer, sysStringLength in_buffer_length, sysStringLength in_pos, uint32_t in_value );
sysStringLength strDWordToStringPos( sysString in_buffer, sysStringLength in_buffer_length, sysStringLength in_pos, uint32_t in_value, uint8_t in_field_length, uint8_t in_precision, uint8_t in_options );

// int32 conversion
//void strInt32ToString( sysString in_buffer, sysStringLength in_buffer_length, dosInt32 in_value, uint8_t in_field_length, uint8_t in_precision, uint8_t in_options );
//void strAppendInt32ToString( sysString in_buffer, sysStringLength in_buffer_length, dosInt32 in_value, uint8_t in_field_length, uint8_t in_precision, uint8_t in_options );
//sysStringLength strInt32ToStringPos( sysString in_buffer, sysStringLength in_buffer_length, sysStringLength in_pos, dosInt32 in_value, uint8_t in_field_length, uint8_t in_precision, uint8_t in_options );

// parser routines
void strSkipWhitespaces(sysString in_string, sysStringLength in_buffer_length, sysStringLength* in_index);
//void strSkipLineEnd(sysString in_string, sysStringLength in_buffer_length, sysStringLength* in_index, sysStringLineEnd in_line_end);
//int strCheckForTokenNoCase(sysString in_buffer, sysStringLength in_buffer_length, sysStringLength* inout_index, dosBool* in_success, sysConstString in_expected_tokens[]);
//void strCheckForConstStringNoCase(sysString in_buffer, sysStringLength in_buffer_length, sysStringLength* inout_index, dosBool* in_success, sysConstString in_string );
//void strCheckForConstString(sysString in_buffer, sysStringLength in_buffer_length, sysStringLength* inout_index, dosBool* in_success, sysConstString in_string );
void strCheckForSeparator( sysString in_buffer, sysStringLength in_buffer_length, sysStringLength* in_index, bool* in_success, sysChar in_char );
//void strStringToInt16( sysString in_buffer, sysStringLength in_buffer_length, sysStringLength* in_index, dosBool* in_success, dosInt16* out_number );
void strStringToWord( sysString in_buffer, sysStringLength in_buffer_length, sysStringLength* in_index, bool* in_success, uint16_t* out_number );
//void strStringToDWord( sysString in_buffer, sysStringLength in_buffer_length, sysStringLength* in_index, dosBool* in_success, uint32_t* out_number );
void strStringToByte( sysString in_buffer, sysStringLength in_buffer_length, sysStringLength* in_index, bool* in_success, uint8_t* out_number );
//void strStringToFixedInt16( sysString in_buffer, sysStringLength* in_index, dosBool* in_success, dosInt16* out_number, uint8_t in_fixed_point, uint16_t in_divisor );
//void strStringToFixedWord( sysString in_buffer, sysStringLength* in_index, dosBool* in_success, uint16_t* out_number, uint8_t in_fixed_point, uint16_t in_divisor );
//void strStringToFixedDWord( sysString in_buffer, sysStringLength* in_index, dosBool* in_success, uint32_t* out_number, uint8_t in_fixed_point, uint32_t in_divisor );
//
//uint8_t strHexDigitToNibble( sysChar in_digit, dosBool* in_success );
//void strHexStringToWord( sysString in_buffer, sysStringLength* in_index, dosBool* in_success, uint16_t* out_number );
//void strHexStringToDWord( sysString in_buffer, sysStringLength in_buffer_length, sysStringLength* in_index, dosBool* in_success, uint32_t* out_number );

#endif
