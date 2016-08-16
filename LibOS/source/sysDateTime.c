/*****************************************************************************/
/* Date&Time management                                                      */
/*                                                                           */
/* Copyright (C) 2015 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
// Includes
#include <sysDateTime.h>
#include <sysString.h>

///////////////////////////////////////////////////////////////////////////////
// Constants
#define NUMBER_OF_DAYS 7                     // Number of days in a week
#define ORIGIN_YEAR    1970                  // the begin year

#define SECONDS_IN_A_DAY 86400
#define SECONDS_IN_AN_HOUR 3600
#define SECONDS_IN_A_MINUTE 60

///////////////////////////////////////////////////////////////////////////////
// Local variables
const sysString l_day_names[NUMBER_OF_DAYS] = { (sysString)"Sunday", (sysString)"Monday", (sysString)"Tuesday", (sysString)"Wednesday", (sysString)"Thursday", (sysString)"Friday", (sysString)"Saturday" };
#ifdef rtcENABLE_DAYLIGHT_SAVING
bool l_daylight_saving_time_changed = false;
#endif

///////////////////////////////////////////////////////////////////////////////
// Local prototypes
static sysStringLength WordToString( uint16_t in_word, sysString out_buffer, sysStringLength in_pos, uint8_t in_max_length );

#ifdef rtcENABLE_DAYLIGHT_SAVING
static void GetDaylightSavingStartDate( sysDateTime* inout_datetime );
static void GetDaylightSavingEndDate( sysDateTime* inout_datetime );
#endif

#ifdef rtcENABLE_CLOCK_ADJUST
static dosInt16 l_second_counter = 0;
static dosInt16 l_clock_adjust = 0;
#endif

///////////////////////////////////////////////////////////////////////////////
// Convert time to string
sysStringLength sysConvertTimeToString( sysString out_buffer, sysStringLength in_buffer_length, sysStringLength in_pos, sysDateTime* in_datetime )
{
	// check buffer length
  if( in_pos + 9 >= in_buffer_length )
    return in_pos;

  // convert hour
  in_pos = WordToString( in_datetime->Hour, out_buffer, in_pos, 2 );
  if( out_buffer[in_pos-2] == '0' )
    out_buffer[in_pos-2] = ' ';

  out_buffer[in_pos++] = ':';

  // convert minute
  in_pos = WordToString( in_datetime->Minute, out_buffer, in_pos, 2 );
  out_buffer[in_pos++] = ':';

  // convert seconds
  in_pos = WordToString( in_datetime->Second, out_buffer, in_pos, 2 );
  out_buffer[in_pos] = '\0';

	return in_pos;
}

///////////////////////////////////////////////////////////////////////////////
// Convert date to string
sysStringLength sysConvertDateToString( sysString out_buffer, sysStringLength in_buffer_length, sysStringLength in_pos, sysDateTime* in_date, sysChar in_separator )
{
  // check buffer length
  if( in_pos + 11 >= in_buffer_length )
    return in_pos;

  // convert year
  in_pos = WordToString( in_date->Year, out_buffer, in_pos, 4 );
  out_buffer[in_pos++] = in_separator;

  // convert month
  in_pos = WordToString( in_date->Month, out_buffer, in_pos, 2 );
  out_buffer[in_pos++] = in_separator;

  // convert day
  in_pos = WordToString( in_date->Day, out_buffer, in_pos,  2 );
  out_buffer[in_pos] = '\0';

	return in_pos;
}

///////////////////////////////////////////////////////////////////////////////
//  Convert day of week to string
void sysConvertDayOfWeekToString( sysDateTime* in_date, sysString out_buffer, uint8_t in_buffer_length )
{
  uint8_t i;

  // check parameter
  if( in_date->DayOfWeek >= NUMBER_OF_DAYS )
    return;

  i = 0;
  while( i < in_buffer_length - 1 && l_day_names[in_date->DayOfWeek][i] != '\0' )
  {
    out_buffer[i] = l_day_names[in_date->DayOfWeek][i];
    i++;
  }

  out_buffer[i] = '\0';
}

///////////////////////////////////////////////////////////////////////////////
//! Converts string to Date
//! \param Buffer conatining the string
//! \param Buffer length
//! \param Position within the buffer
//! \param Success flag
//! \param DateTime struct to store result date
//! \param Separator character
void sysStringToDate(sysString in_buffer, sysStringLength in_buffer_length, sysStringLength* inout_pos, bool* inout_success, sysDateTime* out_datetime, sysChar in_separator, sysDateTimeFormat in_format)
{
	uint16_t year;
	uint8_t month, day;

	//TODO: format handling
	strSkipWhitespaces(	in_buffer, in_buffer_length, inout_pos);
	strStringToWord(in_buffer, in_buffer_length, inout_pos, inout_success, &year);
	strCheckForSeparator(in_buffer, in_buffer_length, inout_pos, inout_success, in_separator);
	strStringToByte(in_buffer, in_buffer_length, inout_pos, inout_success, &month);
	strCheckForSeparator(in_buffer, in_buffer_length, inout_pos, inout_success, in_separator);
	strStringToByte(in_buffer, in_buffer_length, inout_pos, inout_success, &day);

	// validate
	if(*inout_success)
	{
		// validate
		if(month < 1 && month > 12)
			*inout_success = false;
		else
		{
			if(day < 1 || day > sysGetMonthLength(year, month))
				*inout_success = false;
		}
	}

	// update dateimte
	if(*inout_success)
	{
		out_datetime->Year = year;
		out_datetime->Month = month;
		out_datetime->Day = day;

		sysUpdateDayOfWeek(out_datetime);
	}
}

///////////////////////////////////////////////////////////////////////////////
//! Converts string to Time
//! \param Buffer conatining the string
//! \param Buffer length
//! \param Position within the buffer
//! \param Success flag
//! \param DateTime struct to store result time
//! \param Separator character
void sysStringToTime(sysString in_buffer, sysStringLength in_buffer_length, sysStringLength* inout_pos, bool* inout_success, sysDateTime* out_datetime, sysChar in_separator)
{
	uint8_t hour, minute, second;

	//TODO: format handling
	strSkipWhitespaces(	in_buffer, in_buffer_length, inout_pos);
	strStringToByte(in_buffer, in_buffer_length, inout_pos, inout_success, &hour);
	strCheckForSeparator(in_buffer, in_buffer_length, inout_pos, inout_success, in_separator);
	strStringToByte(in_buffer, in_buffer_length, inout_pos, inout_success, &minute);
	strCheckForSeparator(in_buffer, in_buffer_length, inout_pos, inout_success, in_separator);
	strStringToByte(in_buffer, in_buffer_length, inout_pos, inout_success, &second);

	// validate
	if(*inout_success)
	{
		// validate
		if(hour > 23 || minute > 59 || second > 59)
			*inout_success = false;
	}

	// update datetime
	if(*inout_success)
	{
		out_datetime->Hour = hour;
		out_datetime->Minute = minute;
		out_datetime->Second = second;
	}
}

///////////////////////////////////////////////////////////////////////////////
//! Convert date, time to seconds (since Jan 1, ORIGIN_YEAR)
//! \param Data, time to convert
uint32_t sysConvertDateTimeToSeconds( sysDateTime* in_datetime )
{
  uint16_t i;
  uint16_t days;

  // Calculate number of days spent so far from beginning of this year
  days = in_datetime->Day - 1;
  for (i = 1; i < in_datetime->Month; i++) 
  {
    days += sysGetMonthLength( in_datetime->Year, (uint8_t)i );
  }

  // calculate the number of days in the previous years
  for (i = ORIGIN_YEAR; i < in_datetime->Year; i++)
  {
    days += sysIsLeapYear(i) ? 366 : 365;
  }

  return ((uint32_t)days) * SECONDS_IN_A_DAY + ((uint32_t)in_datetime->Hour) * SECONDS_IN_AN_HOUR + in_datetime->Minute * SECONDS_IN_A_MINUTE + in_datetime->Second;
}

///////////////////////////////////////////////////////////////////////////////
//! Convert deconds (since Jan 1, ORIGIN_YEAR) to date and time 
//! \param Data, time to convert
void sysConvertSecondsToDateTime( sysDateTime* out_datetime, uint32_t in_seconds )
{
  uint16_t year;
  uint8_t month, day;
  uint32_t month_length;
  uint8_t hour, minute;

  // calculate year
  year = ORIGIN_YEAR;
  while( in_seconds > 365 * SECONDS_IN_A_DAY )
  {
    if( sysIsLeapYear(year) )
    {
      if( in_seconds > 366 * SECONDS_IN_A_DAY )
      {
        in_seconds -= 366 * SECONDS_IN_A_DAY;
        year += 1;
      }
    }
    else
    {
      in_seconds -= 365 * SECONDS_IN_A_DAY;
      year += 1;
    }
  }
  out_datetime->Year = year;

  // calculate month
  month = 1;
  while(true)
  {
    month_length = sysGetMonthLength( year, month ) * SECONDS_IN_A_DAY;

    if( in_seconds >= month_length )
    {
      in_seconds -= month_length;
      month++;
    }
    else
      break;
  } 
  out_datetime->Month = month;

  // store day
  day = 1;
  while( in_seconds >= SECONDS_IN_A_DAY )
  {
    day++;
    in_seconds -= SECONDS_IN_A_DAY;
  }
  out_datetime->Day = day;

  // calculate time
  hour = 0;
  while( in_seconds >= SECONDS_IN_AN_HOUR )
  {
     hour++;
     in_seconds -= SECONDS_IN_AN_HOUR;
  }
  out_datetime->Hour = hour;

  minute = 0;
  while( in_seconds >= SECONDS_IN_A_MINUTE )
  {
    minute++;
    in_seconds -= SECONDS_IN_A_MINUTE;
  }
  out_datetime->Minute = minute;
  out_datetime->Second = (uint8_t)in_seconds;

  // update day of week
  sysUpdateDayOfWeek( out_datetime );
}

///////////////////////////////////////////////////////////////////////////////
// Update Day of Week
void sysUpdateDayOfWeek( sysDateTime* inout_date )
{
	uint8_t month;
	uint16_t year;

	year = inout_date->Year;
	month = inout_date->Month;

	if( month < 3 )
	{
		month += 12;
		year--;
	}

	inout_date->DayOfWeek = (2 + inout_date->Day + (13*month-2) / 5 + year + year / 4 - year / 100 + year / 400) % 7;
}

///////////////////////////////////////////////////////////////////////////////
// Is Leap Year
bool sysIsLeapYear( uint16_t in_year )
{
	return ( (((in_year % 4) == 0) && ((in_year % 100) != 0)) || ((in_year % 400) == 0));
}

///////////////////////////////////////////////////////////////////////////////
// Get month length (in days)
uint8_t sysGetMonthLength( uint16_t in_year, uint8_t in_month )
{
	uint8_t month_length[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

	// check month
	if( in_month < 1 || in_month > 12 )
		return 0;

	if( in_month == 2 && sysIsLeapYear( in_year ) )
		return month_length[in_month-1]+1;
	else 
		return month_length[in_month-1];
}

#ifdef rtcENABLE_CLOCK_ADJUST

///////////////////////////////////////////////////////////////////////////////
// Set adjust
void sysSetClockAdjust( dosInt16 in_adjust )
{
	l_second_counter = 0;
	l_clock_adjust = in_adjust;
}

///////////////////////////////////////////////////////////////////////////////
// Get Adjust
dosInt16 sysGetClockAdjust(void)
{
	return l_clock_adjust;
}

#endif

///////////////////////////////////////////////////////////////////////////////
// Add one second
void sysAddOneSecond( sysDateTime* in_datetime )
{
#ifdef rtcENABLE_DAYLIGHT_SAVING
	sysDateTime datetime;
#endif

#ifdef rtcENABLE_CLOCK_ADJUST
	// increment second counter
	l_second_counter++;

	if( l_clock_adjust < 0 && l_second_counter == -l_clock_adjust )
	{
		l_second_counter = 0;
		return;
	}

	if( l_clock_adjust > 0 && l_second_counter == l_clock_adjust )
	{
		l_second_counter = 0;
		in_datetime->Second++;
	}
#endif

	// increment seconds
	in_datetime->Second++;

	// normalize date
	sysNormalizeDateTime( in_datetime );

	// handle daylight saving
#ifdef rtcENABLE_DAYLIGHT_SAVING
	// do this check only if second and minute is zero
	if( in_datetime->Second == 0 && in_datetime->Minute == 0 )
	{
		// daylight saving start
		datetime = *in_datetime;
		GetDaylightSavingStartDate( &datetime );

		if( sysIsEqualDateTime( in_datetime, &datetime ) )
			in_datetime->Hour++;

		// daylight saving end
		datetime = *in_datetime;
		GetDaylightSavingEndDate( &datetime );

		if( sysIsEqualDateTime( in_datetime, &datetime ) )
		{
			if( !l_daylight_saving_time_changed )
			{
				in_datetime->Hour--;
				l_daylight_saving_time_changed = true;
			}
			else
				l_daylight_saving_time_changed = false;
		}
	}
#endif
}

///////////////////////////////////////////////////////////////////////////////
// Gets and clears daylight saving time changed flag
bool sysGetAndClearDaylightSavingTimeChangedFlag(void)
{
#ifdef rtcENABLE_DAYLIGHT_SAVING
	bool retval = l_daylight_saving_time_changed;

	l_daylight_saving_time_changed = false;

	return retval;
#else
	return false;
#endif
}

///////////////////////////////////////////////////////////////////////////////
// Is equal datetime
bool sysIsEqualDateTime( sysDateTime* in_datetime1, sysDateTime* in_datetime2 )
{
	return in_datetime1->Second == in_datetime2->Second &&
					in_datetime1->Minute == in_datetime2->Minute &&
					in_datetime1->Hour == in_datetime2->Hour &&
					in_datetime1->Day == in_datetime2->Day &&
					in_datetime1->Month == in_datetime2->Month &&
					in_datetime1->Year == in_datetime2->Year;
}

///////////////////////////////////////////////////////////////////////////////
// Normalize Date
void sysNormalizeDateTime( sysDateTime* in_datetime )
{
	// check seconds overflow
	while( in_datetime->Second >= 60 )
	{
		in_datetime->Second -= 60;

		// increment minutes
		in_datetime->Minute++;
	}

	// check for minutes overflow
	while( in_datetime->Minute >= 60 )
	{
		in_datetime->Minute -= 60;

		// increment hour
		in_datetime->Hour++;
	}

	// check hour overflow
	while( in_datetime->Hour >= 24 )
	{
		in_datetime->Hour -= 24;

		// increment day
		in_datetime->Day++;
	}

	// check for month overflow
	while( in_datetime->Month > 12 )
	{
		in_datetime->Month -= 12;

		// increment year
		in_datetime->Year++;
	}	

	// check day overflow
	while( in_datetime->Day > sysGetMonthLength( in_datetime->Year, in_datetime->Month ) )
	{
		in_datetime->Day -= sysGetMonthLength( in_datetime->Year, in_datetime->Month );

		// increment month
		in_datetime->Month++;

		// check for month overflow
		if( in_datetime->Month > 12 )
		{
			in_datetime->Month -= 12;

			// increment year
			in_datetime->Year++;
		}	
	}

	// update day of week
	sysUpdateDayOfWeek( in_datetime );
}

///////////////////////////////////////////////////////////////////////////////
// Compare datetime
int sysCompareDateTime( sysDateTime* in_datetime1, sysDateTime* in_datetime2 )
{
	// compare years
	if( in_datetime1->Year > in_datetime2->Year )
		return 1;

	if( in_datetime1->Year < in_datetime2->Year )
		return -1;

	// compare months
	if( in_datetime1->Month > in_datetime2->Month )
		return 1;

	if( in_datetime1->Month < in_datetime2->Month )
		return -1;

	// compare days
	if( in_datetime1->Day > in_datetime2->Day )
		return 1;

	if( in_datetime1->Day < in_datetime2->Day )
		return -1;

 	// compare hours
	if( in_datetime1->Hour > in_datetime2->Hour )
		return 1;

	if( in_datetime1->Hour < in_datetime2->Hour )
		return -1;

	// compare minutes
	if( in_datetime1->Minute > in_datetime2->Minute )
		return 1;

	if( in_datetime1->Minute < in_datetime2->Minute )
		return -1;

	// compare seconds
	if( in_datetime1->Second > in_datetime2->Second )
		return 1;

	if( in_datetime1->Second < in_datetime2->Second )
		return -1;

	// time and dates are equal
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Compare datetime
int sysCompareTime( sysDateTime* in_datetime1, sysDateTime* in_datetime2 )
{
 	// compare hours
	if( in_datetime1->Hour > in_datetime2->Hour )
		return 1;

	if( in_datetime1->Hour < in_datetime2->Hour )
		return -1;

	// compare minutes
	if( in_datetime1->Minute > in_datetime2->Minute )
		return 1;

	if( in_datetime1->Minute < in_datetime2->Minute )
		return -1;

	// compare seconds
	if( in_datetime1->Second > in_datetime2->Second )
		return 1;

	if( in_datetime1->Second < in_datetime2->Second )
		return -1;

	// time and dates are equal
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Substract Time
void sysSubstractTime( sysDateTime* in_time, sysDateTime* in_time_to_substract )
{
	uint8_t carry = 0;

	// seconds
	if( in_time_to_substract->Second > in_time->Second )
	{
		in_time->Second = in_time->Second + 60 - in_time_to_substract->Second;
		carry = 1;
	}
	else
	{
		in_time->Second -= in_time_to_substract->Second;
		carry = 0;
	}

	// minutes
	if( in_time_to_substract->Minute + carry > in_time->Minute )
	{
		in_time->Minute = in_time->Minute + 60 - in_time_to_substract->Minute - carry;
		carry = 1;
	}
	else
	{
		in_time->Minute -= in_time_to_substract->Minute + carry;
		carry = 0;
	}

	// hours
	if( in_time_to_substract->Hour + carry > in_time->Hour )
	{
		in_time->Hour = in_time->Hour + 24 - in_time_to_substract->Hour - carry;
		carry = 1;
	}
	else
	{
		in_time->Hour -= in_time_to_substract->Hour + carry;
		carry = 0;
	}
}

///////////////////////////////////////////////////////////////////////////////
// Get daylight saving start date
#ifdef rtcENABLE_DAYLIGHT_SAVING
void 	GetDaylightSavingStartDate( sysDateTime* inout_datetime )
{
	// prepare datatime struct with the first day of the starting month of the daylight saving
	// year member must be valid
	inout_datetime->Month = 3; // daylight saving start at the last sunday of march
	inout_datetime->Day = 1; // first day of march
	inout_datetime->Hour = rtcDAYLIGHT_SAVING_START_HOUR;
	inout_datetime->Minute = 0;
	inout_datetime->Second = 0;

	sysUpdateDayOfWeek( inout_datetime );

	// last sunday
	inout_datetime->Day = 31 - (inout_datetime->DayOfWeek + 2) % 7;
}
#endif

#ifdef rtcENABLE_DAYLIGHT_SAVING
///////////////////////////////////////////////////////////////////////////////
// Get daylight saving end date
void GetDaylightSavingEndDate( sysDateTime* inout_datetime )
{
	// prepare datatime struct with the first day of the ending month of the daylight saving
	// year member must be valid
	inout_datetime->Month = 10; 	// daylight saving ends at last sunday of october
	inout_datetime->Day = 1;
	inout_datetime->Hour = rtcDAYLIGHT_SAVING_END_HOUR;
	inout_datetime->Minute = 0;
	inout_datetime->Second = 0;

	sysUpdateDayOfWeek( inout_datetime );

	// last sunday
	inout_datetime->Day = 31 - (inout_datetime->DayOfWeek + 2) % 7;
}

#endif

///////////////////////////////////////////////////////////////////////////////
// Local functions
static sysStringLength WordToString( uint16_t in_word, sysString out_buffer, sysStringLength in_pos, uint8_t in_max_length )
{
  uint16_t divisor[] = { 10000, 1000, 100, 10, 1 };
  sysChar ch;
	uint8_t i;

  for( i = 0; i < 5; i++ )
  {
    ch ='0';

    while( in_word >= divisor[i] )
    {
      ch++;

      in_word -= divisor[i];
    }

    if( i >= (5 - in_max_length) )
      out_buffer[in_pos + i - (5 - in_max_length)] = ch;
  }

	return in_pos + in_max_length;
}
