/*****************************************************************************/
/* Safe String handling functions                                            */
/*                                                                           */
/* Copyright (C) 2014-2015 Laszlo Arvai                                      */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
// Includes
#include <ctype.h>
#include <sysString.h>

/////////////////////////////////////////////////////////////////////////////////
//// Local functions
static void strConversionAdjustBuffer( sysString in_buffer, sysStringLength in_buffer_length, sysStringLength in_pos, sysStringLength in_field_length, uint8_t in_options, sysStringLength in_buffer_index );


///////////////////////////////////////////////////////////////////////////////
// Get string length (in character)
sysStringLength strGetLength( sysString in_string )
{
	sysString string = in_string;

	while(*string != '\0')
		string++;

	return (sysStringLength)(string - in_string);
}

/////////////////////////////////////////////////////////////////////////////////
//// Get const string length (in character)
//sysStringLength strGetConstLength( sysConstString in_string )
//{
//	return (sysStringLength)strlen( (char*)in_string );
//}
//
/////////////////////////////////////////////////////////////////////////////////
//// Copy character to ram buffer
//sysStringLength strCopyCharacter(sysString in_destination, sysStringLength in_destination_size, sysStringLength in_pos, sysChar in_char)
//{
//	if(in_pos < in_destination_size)
//		in_destination[in_pos++] = in_char;
//
//	if(in_pos < in_destination_size)
//		in_destination[in_pos] = '\0';
//
//	return in_pos;
//}

///////////////////////////////////////////////////////////////////////////////
/// @brief Copies string to ram buffer
/// @param in_destination Destination buffer
/// @param in_destination_size Size of the destination buffer
/// @param in_destination_pos Position of the string to copy (source string) in the destination buffer
/// @param in_source String to copy
/// @return Position of the end of the string in the destination buffer
sysStringLength strCopyString(sysString in_destination, sysStringLength in_destination_size, sysStringLength in_destination_pos, sysString in_source )
{
	while(in_destination_pos < in_destination_size - 1 && *in_source != '\0')
	{
		in_destination[in_destination_pos++] = *in_source++;
	}

	in_destination[in_destination_pos] = '\0';

	return in_destination_pos;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Copies constant string to ram buffer
/// @param in_destination Destination buffer
/// @param in_destination_size Size of the destination buffer
/// @param in_destination_pos Position of the string to copy (source string) in the destination buffer
/// @param in_source Constants String to copy
/// @return Position of the end of the string in the destination buffer
sysStringLength strCopyConstString( sysString in_destination, sysStringLength in_destination_size, sysStringLength in_destination_pos, sysConstString in_source )
{
	// set position
	in_destination += in_destination_pos;
	
	while( *in_source != 0 && in_destination_pos < in_destination_size - 1)
	{
		*in_destination++ = *in_source++;
		in_destination_pos++;
	}
	
	*in_destination = '\0';

	return in_destination_pos;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Concat constant string to ram buffer (appends to the end of the destination string)
/// @param in_destination Destination buffer
/// @param in_destination_size Size of the destination buffer
/// @param in_source Constants String to copy
/// @return Position of the end of the string in the destination buffer
sysStringLength strAppendConstString( sysString in_destination, sysStringLength in_destination_size, sysConstString in_source )
{
	return strCopyConstString( in_destination, in_destination_size, strGetLength( in_destination ), in_source );
}

/////////////////////////////////////////////////////////////////////////////////
//// Concat two string
//void strAppendString( sysString in_destination, sysStringLength in_destination_size, sysString in_source )
//{
//	sysStringLength dest_pos = 0;
//	sysStringLength source_pos = 0;
//
//	// find end of the destination string
//	while(in_destination[dest_pos] != '\0')
//		dest_pos++;
//
//	// copy string
//	while(dest_pos < in_destination_size && in_source[source_pos] != '\0')
//	{
//		in_destination[dest_pos++] = in_source[source_pos++];
//	}
//
//	// close string
//	if(dest_pos < in_destination_size)
//		in_destination[dest_pos] = '\0';
//	else
//	{
//		if(in_destination_size > 0 )
//			in_destination[in_destination_size-1]  = '\0';
//	}
//}
//

///////////////////////////////////////////////////////////////////////////////
/// @brief Finds character occurence in the string
/// @param in_buffer String buffer to search in
/// @param in_char Character to search
/// @return Index fo the first occurence of the given character or sysSTRING_INVALID_POS if no character has been found
sysStringLength strFindChar( sysString in_buffer, sysChar in_char )
{
	sysStringLength index = 0;
	
	while( *in_buffer != '\0' )
	{
		if( *in_buffer == in_char )
			return index;
			
		in_buffer++;
		index++;
	}
	
	return sysSTRING_INVALID_POS;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Finds the last occurence of the given character in the string
/// @param in_buffer String buffer to search in
/// @param in_char Character to search
/// @return Index fo the last occurence of the given character or sysSTRING_INVALID_POS if no character has been found
sysStringLength strFindLastChar( sysString in_buffer, sysChar in_char )
{
	sysStringLength index = 0;
	sysStringLength index_to_return = sysSTRING_INVALID_POS;
	
	while( *in_buffer != '\0' )
	{
		if( *in_buffer == in_char )
			index_to_return = index;
			
		in_buffer++;
		index++;
	}
	
	return index_to_return;
}


/////////////////////////////////////////////////////////////////////////////////
//// Set Length
//void strSetLength( sysString in_destination, sysStringLength in_destination_size, sysChar in_char, sysStringLength in_required_length )
//{
//	sysStringLength length = strGetLength( in_destination );
//
//	in_destination += length;	
//	
//	while( length < in_required_length && length < in_destination_size - 1 )
//	{
//		*in_destination++ = in_char;
//		length++;
//	}
//	
//	*in_destination = '\0';
//}
//	

///////////////////////////////////////////////////////////////////////////////
/// @brief Fill string with a specified character from the given position using the specified length
/// @param in_destination String buffer to fill in
/// @param in_destination_size Buffer size
/// @param in_char Character fill with
/// @param in_start_pos First character index to fill 
/// @param in_required_length New length of the filled string
void strFillString( sysString in_destination, sysStringLength in_destination_size, sysChar in_char, sysStringLength in_start_pos, sysStringLength in_required_length )
{
	sysStringLength length;
	
	length = in_start_pos;
	in_destination += in_start_pos;
	
	while( length < in_required_length && length < in_destination_size - 1 )
	{
		*in_destination++ = in_char;
		length++;
	}
	
	*in_destination = '\0';
}


/////////////////////////////////////////////////////////////////////////////////
/// @brief Converts from Unicode character to ASCII character (all character which can't be converted will be represented by ASCII 255 character)
/// @param in_unicode Unicode character code to convert
/// @return ASCII character value
sysASCIIChar strUnicodeToASCIIChar( sysUnicodeChar in_unicode )
{
	if( in_unicode <= 0xff )
		return (sysChar)in_unicode;
	else
		return (sysChar)0xff;
}


///////////////////////////////////////////////////////////////////////////////
// Char To Upper
sysChar strCharToUpper( sysChar in_char )
{
	return (sysChar)toupper((int)in_char);
}

///////////////////////////////////////////////////////////////////////////////
// Char To Lower
sysChar strCharToLower( sysChar in_char )
{
	return (sysChar)tolower((int)in_char);
}

/////////////////////////////////////////////////////////////////////////////////
//// Int16 to string
//void strAppendInt16ToString( sysString in_buffer, uint8_t in_buffer_length, dosInt16 in_value, uint8_t in_field_length, uint8_t in_precision, uint8_t in_options )
//{
//	uint16_t value;
//	sysStringLength pos;
//	
//	if( in_value < 0 )
//	{
//		value = (uint16_t)(-in_value);
//		in_options |= TS_DISPLAY_MINUS_SIGN;
//	}
//	else
//	{
//		value = (uint16_t)in_value;
//	}
//	
//	pos = strGetLength( in_buffer );
//		
//	strWordToStringPos( in_buffer, in_buffer_length, pos, value, in_field_length, in_precision, in_options );
//}	
//
/////////////////////////////////////////////////////////////////////////////////
//// Int16 to string
//void strInt16ToString( sysString in_buffer, uint8_t in_buffer_length, dosInt16 in_value, uint8_t in_field_length, uint8_t in_precision, uint8_t in_options )
//{
//	uint16_t value;
//	
//	if( in_value < 0 )
//	{
//		value = (uint16_t)(-in_value);
//		in_options |= TS_DISPLAY_MINUS_SIGN;
//	}
//	else
//	{
//		value = (uint16_t)in_value;
//	}
//		
//	strWordToStringPos( in_buffer, in_buffer_length, 0, value, in_field_length, in_precision, in_options );
//}
//

///////////////////////////////////////////////////////////////////////////////
// Word to string
sysStringLength strWordToString( sysString in_buffer, sysStringLength in_buffer_length, uint16_t in_value, uint8_t in_field_length, uint8_t in_precision, uint8_t in_options )
{
	return strWordToStringPos( in_buffer, in_buffer_length, 0, in_value, in_field_length, in_precision, in_options );
}	

/////////////////////////////////////////////////////////////////////////////////
//// Append word to string
sysStringLength strAppendWordToString( sysString in_buffer, sysStringLength in_buffer_length, uint16_t in_value, uint8_t in_field_length, uint8_t in_precision, uint8_t in_options )
{
	sysStringLength pos;

	pos = strGetLength( in_buffer );

	return strWordToStringPos( in_buffer, in_buffer_length, pos, in_value, in_field_length, in_precision, in_options );
}

///////////////////////////////////////////////////////////////////////////////
// Word to string
sysStringLength strWordToStringPos( sysString in_buffer, sysStringLength in_buffer_length, sysStringLength in_pos, uint16_t in_value, uint8_t in_field_length, uint8_t in_precision, uint8_t in_options )
{
	bool zero_blank = true;
	int divisor = 10000;
	uint8_t digit_index = 5;
	uint8_t digit;
	sysStringLength buffer_index = in_pos;
	sysStringLength field_length = in_buffer_length - in_pos - 1;
	
	// adjust length
	if( in_field_length != 0 && in_field_length < field_length )
		field_length = in_field_length;
	
	// parameter checking
	if( in_buffer_length - buffer_index <= 1 || in_buffer == sysNULL || field_length < 1 )
		return in_pos;
		
	if( (in_options & TS_DISPLAY_MINUS_SIGN) != 0 )
			in_buffer[buffer_index++] = '-';

	// send digits
	while( divisor > 0 && (buffer_index - in_pos) < field_length )
	{
		// store dot
		if( digit_index == in_precision )
		{
			if( zero_blank )
				in_buffer[buffer_index++] = '0';
				
			if( (buffer_index - in_pos) < field_length )
				in_buffer[buffer_index++] = '.';
			
			zero_blank = false;
		}

		// calculate digit
		digit = (uint8_t)(in_value / divisor);
		
		// store digit
		if( (digit != 0 || !zero_blank || (divisor == 1) ||
				((in_options & TS_NO_ZERO_BLANKING) != 0 && digit_index <= field_length)) && ((buffer_index - in_pos) < field_length) )
		{
			in_buffer[buffer_index++] = digit + '0';
			zero_blank = false;
		}
	
		// calculate remaining
		in_value -= digit * divisor;
	
		// next digit
		digit_index--;
		divisor /= 10;
	}
	
	// if conversion was unsuccessfull
	if( divisor > 0 || buffer_index >= in_buffer_length - 1 )
	{
		strFillString( in_buffer, in_buffer_length, '#', in_pos, in_pos + field_length );
		buffer_index = field_length;
	}
	else
	{
		// adjust buffer
		if( in_field_length != 0 )
		{
			strConversionAdjustBuffer( in_buffer, in_buffer_length, in_pos, field_length, in_options, buffer_index );

			buffer_index = field_length + in_pos;
		}

		// terminate buffer
		in_buffer[buffer_index] = '\0';
	}

	return buffer_index;
}

/////////////////////////////////////////////////////////////////////////////////
//// Int32 to string
//void strInt32ToString( sysString in_buffer, sysStringLength in_buffer_length, dosInt32 in_value, uint8_t in_field_length, uint8_t in_precision, uint8_t in_options )
//{
//	strInt32ToStringPos( in_buffer, in_buffer_length, 0, in_value, in_field_length, in_precision, in_options );
//}	
//
/////////////////////////////////////////////////////////////////////////////////
//// Int32 to string
//void strAppendInt32ToString( sysString in_buffer, sysStringLength in_buffer_length, dosInt32 in_value, uint8_t in_field_length, uint8_t in_precision, uint8_t in_options )
//{
//	sysStringLength pos;
//	
//	pos = strGetLength( in_buffer );
//
//	strInt32ToStringPos( in_buffer, in_buffer_length, pos, in_value, in_field_length, in_precision, in_options );
//}	
//		
//
/////////////////////////////////////////////////////////////////////////////////
//// Int32 to string
//sysStringLength strInt32ToStringPos( sysString in_buffer, sysStringLength in_buffer_length, sysStringLength in_pos, dosInt32 in_value, uint8_t in_field_length, uint8_t in_precision, uint8_t in_options )
//{
//	uint32_t value;
//	
//	if( in_value < 0 )
//	{
//		value = (uint32_t)(-in_value);
//		in_options |= TS_DISPLAY_MINUS_SIGN;
//	}
//	else
//	{
//		value = (uint32_t)in_value;
//	}
//		
//	return strDWordToStringPos( in_buffer, in_buffer_length, in_pos, value, in_field_length, in_precision, in_options );
//}


///////////////////////////////////////////////////////////////////////////////
/// @brief Converts DWord value to string representation
/// @param in_buffer Buffer to place the string
/// @param in_buffer_length Length of the buffer
/// @param in_value DWord value to convert
/// @param in_field_length Desired length of the result string (0 - ignored)
/// @param in_precision Number of fractional digits
/// @param in_optiones Option flags
/// @return position of the end of the covnerted string
sysStringLength strDWordToString( sysString in_buffer, sysStringLength in_buffer_length, uint32_t in_value, uint8_t in_field_length, uint8_t in_precision, uint8_t in_options )
{
	return strDWordToStringPos( in_buffer, in_buffer_length, 0, in_value, in_field_length, in_precision, in_options );
}	

///////////////////////////////////////////////////////////////////////////////
/// @brief Converts DWord value to string representation
/// @param in_buffer Buffer to place the string
/// @param in_buffer_length Length of the buffer
/// @param in_pos Position where the converted string will start in the buffer
/// @param in_value DWord value to convert
/// @param in_field_length Desired length of the result string (0 - ignored)
/// @param in_precision Number of fractional digits
/// @param in_optiones Option flags
sysStringLength strDWordToStringPos( sysString in_buffer, sysStringLength in_buffer_length, sysStringLength in_pos, uint32_t in_value, uint8_t in_field_length, uint8_t in_precision, uint8_t in_options )
{
	bool zero_blank = true;
	uint32_t divisor = 1000000000ul;
	uint8_t digit_index = 10;
	uint8_t digit;
	sysStringLength buffer_index = in_pos;
	sysStringLength field_length = in_buffer_length - in_pos - 1;
	
	// adjust length
	if( in_field_length != 0 && in_field_length < field_length )
		field_length = in_field_length;
	
	// parameter checking
	if( in_buffer_length - buffer_index <= 1 || in_buffer == sysNULL || field_length < 1 )
		return in_pos;

	if( (in_options & TS_DISPLAY_MINUS_SIGN) != 0 )
			in_buffer[buffer_index++] = '-';

	// send digits
	while( divisor > 0 && (buffer_index - in_pos) < field_length )
	{
		// store dot
		if( digit_index == in_precision )
		{
			if( zero_blank )
				in_buffer[buffer_index++] = '0';
				
			if( (buffer_index - in_pos) < field_length )
				in_buffer[buffer_index++] = '.';
			
			zero_blank = false;
		}

		// calculate digit
		digit = (uint8_t)(in_value / divisor);
		
		// store digit
		if( (digit != 0 || !zero_blank || (divisor == 1) ||
				((in_options & TS_NO_ZERO_BLANKING) != 0 && digit_index <= field_length)) && ((buffer_index - in_pos) < field_length) )
		{
			in_buffer[buffer_index++] = digit + '0';
			zero_blank = false;
		}
	
		// calculate remaining
		in_value -= digit * divisor;
	
		// next digit
		digit_index--;
		divisor /= 10;
	}
	
	// if conversion was unsuccessfull
	if( divisor > 0 || buffer_index >= in_buffer_length - 1 )
	{
		strFillString( in_buffer, in_buffer_length, '#', in_pos, in_pos + field_length );
	}
	else
	{
		// adjust buffer
		if( in_field_length != 0 )
		{
			strConversionAdjustBuffer( in_buffer, in_buffer_length, in_pos, field_length, in_options, buffer_index );

			buffer_index = field_length + in_pos;
		}


		// terminate buffer
		in_buffer[buffer_index] = '\0';
	}

	return buffer_index;
}

/////////////////////////////////////////////////////////////////////////////////
///// @brief  Converts Nibble to hex digit (uppercase)
///// @param Nibble to convert (0-15)
///// @return ASCII code of the converted HEX character ('0'-'F')
//sysChar strNibbleToHexDigit( uint8_t in_nibble )
//{
//	if( in_nibble > 9 )
//		return in_nibble - 10 + 'A';
//	else
//		return in_nibble + '0';
//}	
//
/////////////////////////////////////////////////////////////////////////////////
//// Word to hex string pos
//sysStringLength strWordToHexStringPos( sysString in_buffer, sysStringLength in_buffer_length, sysStringLength in_pos, uint16_t in_value )
//{
//	if( in_pos + 5 < in_buffer_length )
//	{
//		in_buffer[in_pos++] = strNibbleToHexDigit( (in_value >> 12) & 0x0f );
//		in_buffer[in_pos++] = strNibbleToHexDigit( (in_value >> 8) & 0x0f );
//		in_buffer[in_pos++] = strNibbleToHexDigit( (in_value >> 4) & 0x0f );
//		in_buffer[in_pos++] = strNibbleToHexDigit( in_value & 0x0f );
//		in_buffer[in_pos] = '\0';
//	}	
//
//	return in_pos;
//}	
//
/////////////////////////////////////////////////////////////////////////////////
///// @brief  Converts DWord (32bit unsigned int) value to hex string and stores it at the given position
///// @param Buffer to store conversion result
///// @param Length of the buffer
///// @param Start position of the stored string
///// @param Value to convert
//sysStringLength strDWordToHexStringPos( sysString in_buffer, sysStringLength in_buffer_length, sysStringLength in_pos, uint32_t in_value )
//{
//	sysStringLength pos = in_pos;
//
//	pos = strWordToHexStringPos(in_buffer, in_buffer_length, pos, (uint16_t)(in_value >> 16));
//	pos = strWordToHexStringPos(in_buffer, in_buffer_length, pos, (uint16_t)(in_value & 0xffff));
//
//	return pos;
//}	
//
/////////////////////////////////////////////////////////////////////////////////
//// Word to hex string
//void strWordToHexString( sysString in_buffer, sysStringLength in_buffer_length, uint16_t in_value )
//{
//	strWordToHexStringPos( in_buffer, in_buffer_length, 0, in_value );
//}	
//
/////////////////////////////////////////////////////////////////////////////////
//// Append Word to hex string
//void strAppendWordToHexString( sysString in_buffer, sysStringLength in_buffer_length, uint16_t in_value )
//{
//	sysStringLength pos;
//	
//	pos = strGetLength( in_buffer );
//
//	strWordToHexStringPos( in_buffer, in_buffer_length, pos, in_value );
//}	
// 
/////////////////////////////////////////////////////////////////////////////////
///// @brief  Converts byte value to hex string and stores it at the given position
///// @param Buffer to store conversion result
///// @param Length of the buffer
///// @param Start position of the stored string
///// @param Value to convert
//void strByteToHexStringPos( sysString in_buffer, sysStringLength in_buffer_length, sysStringLength in_pos, uint16_t in_value )
//{
//	if( in_pos + 3 < in_buffer_length )
//	{
//		in_buffer[in_pos++] = strNibbleToHexDigit( (in_value >> 4) & 0x0f );
//		in_buffer[in_pos++] = strNibbleToHexDigit( in_value & 0x0f );
//		in_buffer[in_pos] = '\0';
//	}	
//}	
//
/////////////////////////////////////////////////////////////////////////////////
///// @brief  Converts byte value to hex string
///// @param Buffer to store conversion result
///// @param Length of the buffer
///// @param Value to convert
//void strByteToHexString( sysString in_buffer, sysStringLength in_buffer_length, uint16_t in_value )
//{
//	strByteToHexStringPos( in_buffer, in_buffer_length, 0, in_value );
//}	
//
/////////////////////////////////////////////////////////////////////////////////
///// @brief  Converts byte value to hex string	and appends the result string to the end
///// @brief  of the given string buffer
///// @param Buffer to store conversion result
///// @param Length of the buffer
///// @param Value to convert
//void strAppendByteToHexString( sysString in_buffer, sysStringLength in_buffer_length, uint16_t in_value )
//{
//	sysStringLength pos;
//	
//	pos = strGetLength( in_buffer );
//
//	strByteToHexStringPos( in_buffer, in_buffer_length, pos, in_value );
//}	
//

/*****************************************************************************/
/* Comparision routines                                                      */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Compares string with constant string
/// @param String to comapare
/// @param Constant string to compare
/// @return -1, 0, 1 depending of the result
int strCompareConstString(sysString in_string1, sysConstString in_string2)
{
  for( ;  *in_string1 == *in_string2; in_string1++, in_string2++)
	{
		if(*in_string1 == '\0')
			return 0;
	}
  return ((*(unsigned char *)in_string1 < *(unsigned char *)in_string2) ? -1 : +1);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief  Compares string with constant string without case senitivity
/// @param String to comapare
/// @param Constant string to compare
/// @return -1, 0, 1 depending of the result
int strCompareConstStringNoCase(sysString in_string1, sysConstString in_string2)
{
	for( ;  strCharToUpper(*in_string1) == strCharToUpper(*in_string2); in_string1++, in_string2++)
	{
		if(*in_string1 == '\0')
			return 0;
	}
	return ((strCharToUpper(*in_string1) < strCharToUpper(*in_string2)) ? -1 : +1);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief  Compares two strings
/// @param String1 to comapare
/// @param String2 to compare
/// @return -1, 0, 1 depending of the result
int strCompareString(sysString in_string1, sysString in_string2)
{
  for( ; *in_string1 == *in_string2; in_string1++, in_string2++)
	{
		if(*in_string1 == '\0')
			return 0;
	}
  return ((*(unsigned char *)in_string1 < *(unsigned char *)in_string2) ? -1 : +1);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief  Compares two strings without case sensitivity
/// @param String1 to comapare
/// @param String2 to compare
/// @return -1, 0, 1 depending of the result
int strCompareStringNoCase(sysString in_string1, sysString in_string2)
{
  for( ; strCharToUpper(*in_string1) == strCharToUpper(*in_string2); in_string1++, in_string2++)
	{
		if(*in_string1 == '\0')
			return 0;
	}
  return (strCharToUpper(*in_string1) < strCharToUpper(*in_string2) ? -1 : +1);
}


/*****************************************************************************/
/* Parser routines                                                           */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Skips whitespace characters (space, tab)
/// @param Buffer containing the string
/// @param Length of the buffer
/// @param Pointer to the character position variable (it wil be updated to point to the first non-whitespce character)
void strSkipWhitespaces(sysString in_string, sysStringLength in_buffer_length, sysStringLength* in_index)
{
	while( *in_index < in_buffer_length && (in_string[*in_index] == ' ' || in_string[*in_index] == sysASCII_HT) )
		(*in_index)++;
}

/////////////////////////////////////////////////////////////////////////////////
///// @brief  Skips to the end of the line
///// @param Buffer containing the string
///// @param Length of the buffer
///// @param Pointer to the character position variable (it wil be updated to point to the first character on the next line)
///// @param Line end type
//void strSkipLineEnd(sysString in_string, sysStringLength in_buffer_length, sysStringLength* in_index, sysStringLineEnd in_line_end)
//{
//	sysStringLength pos = *in_index;
//
//	switch (in_line_end)
//	{
//		// CR line end
//		case sysString_LE_CR:
//			while(pos < in_buffer_length && in_string[pos] != '\0' )
//			{
//				if( in_string[pos] == ASCII_CR)
//				{
//					pos++;
//					break;
//				}
//				else
//				{
//					pos++;
//				}
//			}
//			break;
//
//		// LF line end
//		case sysString_LE_LF:
//			while(pos < in_buffer_length && in_string[pos] != '\0' )
//			{
//				if( in_string[pos] == ASCII_LF)
//				{
//					pos++;
//					break;
//				}
//				else
//				{
//					pos++;
//				}
//			}
//			break;
//
//		// CRLF line end
//		case sysString_LE_CRLF:
//			while(pos < in_buffer_length - 1 && in_string[pos] != '\0' )
//			{
//				if( in_string[pos] == ASCII_CR && in_string[pos+1] == ASCII_LF )
//				{
//					pos += 2;
//					break;
//				}
//				else
//				{
//					pos++;
//				}
//			}
//			break;
//
//		// LFCR line end
//		case sysString_LE_LFCR:
//			while(pos < in_buffer_length - 1 && in_string[pos] != '\0' )
//			{
//				if( in_string[pos] == ASCII_LF && in_string[pos+1] == ASCII_CR )
//				{
//					pos += 2;
//					break;
//				}
//				else
//				{
//					pos++;
//				}
//			}
//			break;
//	}
//
//	*in_index = pos;
//}
//
/////////////////////////////////////////////////////////////////////////////////
///// @brief  Checks for token
///// @param Buffer containing the string
///// @param Length of the buffer
///// @param Pointer to the character position variable (it wil be updated to point to the first character on the next line)
///// @param Pointer to the result status code variable (true - success, false - failed)
///// @param List of the expected tokens (terminated with null pointer)
///// @return Token index if token found or -1 if token was not found
//int strCheckForTokenNoCase(sysString in_buffer, sysStringLength in_buffer_length, sysStringLength* inout_index, bool* in_success, sysConstString in_expected_tokens[])
//{
//	int token_index;
//	sysStringLength token_pos;
//	sysStringLength index;
//
//	// sanity check
//	if(!(*in_success) || *inout_index >= in_buffer_length || in_expected_tokens[0] == DOS_NULL)
//		return -1;
//
//	// search for token
//	token_index = 0;
//	while(token_index >= 0)
//	{
//		index = *inout_index;
//		token_pos = 0;
//		while( index < in_buffer_length && in_buffer[index] != '\0' && in_expected_tokens[token_index][token_pos] != '\0')
//		{
//			if( strCharToUpper( in_buffer[index++] ) != strCharToUpper( in_expected_tokens[token_index][token_pos++]) )
//				break;
//		}
//
//		if(in_expected_tokens[token_index][token_pos] == '\0')
//		{
//			// found
//			*inout_index = index;
//			break;
//		}
//		else
//		{
//			// not found
//			token_index++;
//			if(in_expected_tokens[token_index] == DOS_NULL)
//			{
//				*in_success = false;
//				token_index = -1;
//			}
//		}
//	}
//
//	return token_index;
//}

///////////////////////////////////////////////////////////////////////////////
/// @brief Check for const string in a string (ignore case)
/// @param in_buffer Buffer containing the string to search in
/// @param in_buffer_length Length of the buffer
/// @param inout_index Index of the position in the buffer where comparison starts
/// @param in_success Success flag (if false at the calling of the function then it returns immediately) . When it returns it contains a flag showing if the search string was found at the given position (true value)
/// @param in_string String to search (const string)
void strCheckForConstStringNoCase(sysString in_buffer, sysStringLength in_buffer_length, sysStringLength* inout_index, bool* in_success, sysConstString in_string )
{
	sysStringLength index = *inout_index;
	
	// return if flag is not success
	if(!(*in_success))
		return;

	// compare strings
	while( index < in_buffer_length && in_buffer[index] != '\0' && *in_string != '\0' && *in_success )
	{
		if( strCharToUpper( in_buffer[index] ) != strCharToUpper( *in_string ) )
			*in_success = false;
			
		index++;
		in_string++;
	}
	
	if( index == *inout_index || *in_string != '\0' )
		*in_success = false;

	// update pointer when string was found
	if( in_success )
		*inout_index = index;
}

/////////////////////////////////////////////////////////////////////////////////
//// Check for const string
//void strCheckForConstString(sysString in_buffer, sysStringLength in_buffer_length, sysStringLength* inout_index, bool* in_success, sysConstString in_string )
//{
//	bool success = true;
//	sysStringLength index = *inout_index;
//
//	if(!(*in_success))
//		return;
//
//	while( index < in_buffer_length && in_buffer[index] != '\0' && *in_string != '\0' && *in_success )
//	{
//		if( in_buffer[index] != *in_string )
//			*in_success = false;
//
//		index++;
//		in_string++;
//	}
//
//	if( index == *inout_index || *in_string != '\0' )
//		*in_success = false;
//
//	if( success )
//		*inout_index = index;
//}

///////////////////////////////////////////////////////////////////////////////
/// @brief  Checks for const char (e.g. separator)
/// @param Buffer containing the string
/// @param Length of the buffer
/// @param Pointer to the character position variable (it wil be updated to point to the first character on the next line)
/// @param Pointer to the result status code variable (true - success, false - failed)
/// @param Expected character
void strCheckForSeparator( sysString in_buffer, sysStringLength in_buffer_length, sysStringLength* in_index, bool* in_success, sysChar in_char )
{
	if( *in_index < in_buffer_length && *in_success && in_buffer[*in_index] == in_char )
	{
		(*in_index)++;
	}
	else
		*in_success = false;	
}


///////////////////////////////////////////////////////////////////////////////
/// @brief Converts string to byte
/// @param in_buffer Buffer containing the string to convert
/// @param in_buffer_length Total length of the buffer
/// @param in_index Index of the first character to parse (it will be upated to point to the last parsed character)
/// @param in_success Must be true for normal operation (if it is false the function will return immediatelly). Set to false if operation is failed.
/// @param out_number Parsed value (valid only when success is true.
void strStringToByte( sysString in_buffer, sysStringLength in_buffer_length, sysStringLength* in_index, bool* in_success, uint8_t* out_number )
{
	uint16_t number;
	
	// convert to word
	strStringToWord( in_buffer, in_buffer_length, in_index, in_success, &number );
	
	// check for overflow
	if( number > UINT8_MAX)
		*in_success = false;
		
	// store result
	*out_number = (uint8_t)number;
}	

/////////////////////////////////////////////////////////////////////////////////
//// String to dosInt16 conversion
//void strStringToInt16( sysString in_buffer, sysStringLength in_buffer_length, sysStringLength* in_index, bool* in_success, dosInt16* out_number )
//{
//	bool neg = false;
//	uint16_t number;
//
//	// return if success flag is false
//	if( !(*in_success) )
//		return;
//
//	// check for '-' sign
//	if( in_buffer[*in_index] == '-' )
//	{
//		(*in_index)++;
//		neg = true;
//	}
//
//	// convert
//	strStringToWord( in_buffer, in_buffer_length, in_index, in_success, &number );
//
//	// check for overflow
//	if( neg )
//	{
//		if( number > -DOSINT16_MIN )
//			*in_success = false;
//	}
//	else
//	{
//		if( number > DOSINT16_MAX )
//			*in_success = false;
//	}	
//	
//	if( *in_success )
//		*out_number = number;
//	
//	// add sign
//	if( *in_success && neg )
//		*out_number = -(*out_number);
//}	
//
///////////////////////////////////////////////////////////////////////////////
/// @brief Coverts string to 16 bit unsiged int
/// @param in_buffer Buffer containing the string to convert
/// @param in_buffer_length Total length of the buffer
/// @param in_index Index of the first character to parse
/// @param in_success Must be true for normal operation (if it is false the function will return immediatelly). Set to false if operation is failed.
/// @param out_number Parsed value (valid only when success is true.
void strStringToWord( sysString in_buffer, sysStringLength in_buffer_length, sysStringLength* in_index, bool* in_success, uint16_t* out_number )
{
	uint8_t digits = 0;
	uint8_t dig;

	// initialize
	*out_number = 0;
	
	if( !(*in_success) || in_buffer == sysNULL || in_buffer_length == 0)
		return;

	// convert char to int	
	while( in_buffer[*in_index] >= '0' && in_buffer[*in_index] <= '9' && *in_success)
	{
		// get digit
		dig = in_buffer[*in_index] - '0';
		
		// check for overflow and add digit to number
		if(*out_number < 6553 || (*out_number == 6553 && digits <= 5 ) )
			*out_number = *out_number * 10 + dig;
		else
			*in_success = false; // overflow error

		(*in_index)++;

		digits++;
	}

	if( digits == 0 )
		*in_success = false;
}

/////////////////////////////////////////////////////////////////////////////////
//// String to int (16bit) general conversion
//void strStringToFixedInt16( sysString in_buffer, sysStringLength* in_index, bool* in_success, dosInt16* out_number, uint8_t in_fixed_point, uint16_t in_divisor )
//{
//	bool neg = false;
//	uint16_t number;
//	
//	// initialize
//	*out_number = 0;
//	number = 0;
//	
//	if( !(*in_success) )
//		return;
//		
//	// check for '-' sign
//	if( in_buffer[*in_index] == '-' )
//	{
//		(*in_index)++;
//		neg = true;
//	}
//
//	// convert to word	
//	strStringToFixedWord( in_buffer, in_index, in_success, &number, in_fixed_point, in_divisor );
//
//	// check for overflow
//	if( *in_success )
//	{
//		if( neg )
//		{
//			if( number > -DOSINT16_MIN )
//				*in_success = false;
//		}
//		else
//		{
//			if( number > DOSINT16_MAX )
//				*in_success = false;
//		}	
//	}
//		
//	// convert to fixed point
//	if( *in_success )
//	{
//		// add sign
//		if( neg )
//			*out_number = -number;
//		else
//			*out_number = number;
//	}
//}
//
/////////////////////////////////////////////////////////////////////////////////
//// String to int (16bit) general conversion
//void strStringToFixedWord( sysString in_buffer, sysStringLength* in_index, bool* in_success, uint16_t* out_number, uint8_t in_fixed_point, uint16_t in_divisor )
//{
//	uint8_t digits;
//	uint8_t dig;
//	uint8_t dot_pos = 0xff;
//	uint16_t number;
//	uint16_t power_of_fixed;
//	
//	// initialize
//	*out_number = 0;
//	number = 0;
//	
//	if( !(*in_success) )
//		return;
//		
//	// calculate 10^in_fixed_point
//	power_of_fixed = 1;
//	
//	for( digits = 0; digits < in_fixed_point; digits++ )
//		power_of_fixed *= 10;
//		
//	// convert char to int
//	digits = 0;	
//	while( (in_buffer[*in_index] == '.' || (in_buffer[*in_index] >= '0' && in_buffer[*in_index] <= '9')) && *in_success)
//	{
//		// check for dot
//		if( in_buffer[*in_index] == '.' )
//		{
//			// if this is the first dot store position
//			if( dot_pos == 0xff )
//			{
//				dot_pos = digits;
//			}
//			else
//			{
//				// second dot -> error
//				*in_success = false;
//			}	
//		}
//		else
//		{
//			if( (dot_pos == 0xff) || (digits - dot_pos < in_fixed_point) )
//			{
//				// get digit
//				dig = in_buffer[*in_index] - '0';
//				
//				// check for overflow and add digit to number
//				if(number < 6553 || (number == 6553 && dig <= 5 ) )
//					number = number * 10 + dig;
//				else
//					*in_success = false; // overflow error
//		
//				digits++;
//			}
//		}
//		
//		// next character
//		(*in_index)++;
//	}
//
//	if( digits == 0 )
//		*in_success = false;
//
//	// convert to fixed point
//	if( *in_success )
//	{
//		// if there was no dot
//		if( dot_pos == 0xff )
//			dot_pos = digits;
//			
//		// adjust missing fractions
//		while( digits - dot_pos < in_fixed_point )
//		{
//			number *= 10;
//			digits++;
//		}	
//
//		*out_number = (uint16_t)(((uint32_t)number * in_divisor + (power_of_fixed/2-1)) / power_of_fixed);
//	}
//}
//
/////////////////////////////////////////////////////////////////////////////////
//// String to uint16_t conversion
//void strStringToDWord( sysString in_buffer, sysStringLength in_buffer_length, sysStringLength* in_index, bool* in_success, uint32_t* out_number )
//{
//	uint8_t digits = 0;
//	uint8_t dig;
//
//	// sanity check
//	if( !(*in_success) || in_buffer == DOS_NULL || in_buffer_length == 0)
//	{
//		*in_success = false;
//		return;
//	}
//
//	// initialize
//	*out_number = 0;
//
//	// convert char to DWord	
//	while( in_buffer[*in_index] >= '0' && in_buffer[*in_index] <= '9' && *in_success)
//	{
//		// get digit
//		dig = in_buffer[*in_index] - '0';
//		
//		// check for overflow and add digit to number
//		if(*out_number < 429496729 || (*out_number == 429496729 && dig <= 6 ) )
//			*out_number = *out_number * 10 + dig;
//		else
//			*in_success = false; // overflow error
//
//		(*in_index)++;
//
//		digits++;
//	}
//
//	if( digits == 0 )
//		*in_success = false;
//}
//
/////////////////////////////////////////////////////////////////////////////////
//// String to int (32bit) general conversion
//void strStringToFixedDWord( sysString in_buffer, sysStringLength* in_index, bool* in_success, uint32_t* out_number, uint8_t in_fixed_point, uint32_t in_divisor )
//{
//	uint8_t digits;
//	uint8_t dig;
//	uint8_t dot_pos = 0xff;
//	uint32_t number;
//	uint32_t power_of_fixed;
//	
//	// initialize
//	*out_number = 0;
//	number = 0;
//	
//	if( !(*in_success) )
//		return;
//		
//	// calculate 10^in_fixed_point
//	power_of_fixed = 1;
//	
//	for( digits = 0; digits < in_fixed_point; digits++ )
//		power_of_fixed *= 10;
//		
//	// convert char to int
//	digits = 0;	
//	while( (in_buffer[*in_index] == '.' || (in_buffer[*in_index] >= '0' && in_buffer[*in_index] <= '9')) && *in_success)
//	{
//		// check for dot
//		if( in_buffer[*in_index] == '.' )
//		{
//			// if this is the first dot store position
//			if( dot_pos == 0xff )
//			{
//				dot_pos = digits;
//			}
//			else
//			{
//				// second dot -> error
//				*in_success = false;
//			}	
//		}
//		else
//		{
//			if( (dot_pos == 0xff) || (digits - dot_pos < in_fixed_point) )
//			{
//				// get digit
//				dig = in_buffer[*in_index] - '0';
//				
//				// check for overflow and add digit to number
//				if(number < 429496729 || (number == 429496729 && dig <= 6 ) )
//					number = number * 10 + dig;
//				else
//					*in_success = false; // overflow error
//		
//				digits++;
//			}
//		}
//		
//		// next character
//		(*in_index)++;
//	}
//
//	if( digits == 0 )
//		*in_success = false;
//
//	// convert to fixed point
//	if( *in_success )
//	{
//		// if there was no dot
//		if( dot_pos == 0xff )
//			dot_pos = digits;
//			
//		// adjust missing fractions
//		while( digits - dot_pos < in_fixed_point )
//		{
//			number *= 10;
//			digits++;
//		}	
//
//		*out_number = (uint32_t)(((dosUInt64)number * in_divisor + (power_of_fixed/2-1)) / power_of_fixed);
//	}
//}
//
/////////////////////////////////////////////////////////////////////////////////
//// Hex digit to nibble
//uint8_t strHexDigitToNibble( sysChar in_digit, bool* in_success )
//{
//	if( *in_success )
//	{
//		if( charIsDigit(in_digit) )
//			return in_digit - '0';
//		else
//		{
//			if( in_digit >= 'A' && in_digit <= 'F' )
//				return in_digit - 'A' + 10;
//			else
//			{
//				if( in_digit >= 'a' && in_digit <= 'f' )
//					return in_digit - 'a' + 10;
//				else
//					*in_success = false;
//			}	
//		}	
//	}	
//	
//	return 0;
//}	
//
/////////////////////////////////////////////////////////////////////////////////
//// Hex string to word
//void strHexStringToWord( sysString in_buffer, sysStringLength* in_index, bool* in_success, uint16_t* out_number )
//{
//	uint8_t digits;
//
//	// init
//	*out_number = 0;
//
//	// convert characters
//	digits = 0;	
//	while( charIsHexDigit(in_buffer[*in_index]) && in_success && digits < 4)
//	{
//		*out_number = ((*out_number) << 4) + strHexDigitToNibble( in_buffer[(*in_index)++], in_success );
//		digits++;
//	}
//}	
//
/////////////////////////////////////////////////////////////////////////////////
///// @brief  Converts hex string to DWord (32-bit unsigned)
///// @param Buffer containing the string
///// @param Length of the buffer
///// @param Pointer to the character position of the hex value
///// @param Pointer to the result status code variable (true - success, false - failed)
///// @param Output value
//void strHexStringToDWord( sysString in_buffer, sysStringLength in_buffer_length, sysStringLength* in_index, bool* in_success, uint32_t* out_number )
//{
//	uint8_t digits;
//
//	// sanity check
//	if( !(*in_success) || in_buffer == DOS_NULL || in_buffer_length == 0)
//	{
//		*in_success = false;
//		return;
//	}
//
//	// init
//	*out_number = 0;
//
//	// convert characters
//	digits = 0;	
//	while( charIsHexDigit(in_buffer[*in_index]) && in_success && digits < 8)
//	{
//		*out_number = ((*out_number) << 4) + strHexDigitToNibble( in_buffer[(*in_index)++], in_success );
//		digits++;
//	}
//}

/*****************************************************************************/
/* Local functions                                                           */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
// Adjust buffer
static void strConversionAdjustBuffer( sysString in_buffer, sysStringLength in_buffer_length, sysStringLength in_pos, sysStringLength in_field_length, uint8_t in_options, sysStringLength in_buffer_index )
{
	sysStringLength digit_index;
	
	// right adjustment
	if( (in_options & TS_RIGHT_ADJUSTMENT) != 0 )
	{
		// adjust buffer to the right
		for( digit_index = 1; digit_index <= in_buffer_index - in_pos; digit_index++ )
			in_buffer[in_pos + in_field_length - digit_index] = in_buffer[in_buffer_index - digit_index];
			
		// fill begining of buffer
		for( digit_index = in_pos; digit_index < (in_pos + in_field_length) - in_buffer_index + in_pos; digit_index++ )
			in_buffer[digit_index] = ' ';
	}
	else
	{
		while( in_buffer_index < (in_field_length + in_pos) )
			in_buffer[in_buffer_index++] = ' ';
	}
}

