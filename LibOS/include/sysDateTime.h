/*****************************************************************************/
/*                                                                           */
/*    Domino Operation System Kernel Module                                  */
/*                                                                           */
/*    Copyright (C) 2005 Laszlo Arvai                                        */
/*                                                                           */
/*    ------------------------------------------------------------------     */
/*    sysdateTime - Date & Time Functions                                   */
/*****************************************************************************/

#ifndef __sysDateTime_h
#define __sysDateTime_h

///////////////////////////////////////////////////////////////////////////////
// Includes
#include <sysTypes.h>
#include <sysString.h>

///////////////////////////////////////////////////////////////////////////////
// Typedefs

// Date&Time
typedef struct
{
  uint8_t Hour;					// 0-23
  uint8_t Minute;				// 0-59
  uint8_t Second;				// 0-59

  uint16_t Year;
  uint8_t Month;				// 1-12
  uint8_t Day;					// 1-31

  uint8_t DayOfWeek;		// 0 - Sunday, 1 - Monday ... 6 - Saturday
} sysDateTime;

// DateTime format
typedef enum
{
	sysDateTimeFormat_YYYYMMDD,
} sysDateTimeFormat;

///////////////////////////////////////////////////////////////////////////////
// Function prototypes
void sysConvertDayOfWeekToString( sysDateTime* in_date, sysString out_buffer, uint8_t in_buffer_length );
uint32_t sysConvertDateTimeToSeconds( sysDateTime* in_date );
void sysConvertSecondsToDateTime( sysDateTime* out_datetime, uint32_t in_seconds );

sysStringLength sysConvertDateToString( sysString out_buffer, sysStringLength in_buffer_length, sysStringLength in_pos, sysDateTime* in_date, sysChar in_separator );
sysStringLength sysConvertTimeToString( sysString out_buffer, sysStringLength in_buffer_length, sysStringLength in_pos, sysDateTime* in_time );

void sysStringToDate(sysString in_buffer, sysStringLength in_buffer_length, sysStringLength* inout_pos, bool* inout_success, sysDateTime* out_datetime, sysChar in_separator, sysDateTimeFormat in_format);
void sysStringToTime(sysString in_buffer, sysStringLength in_buffer_length, sysStringLength* inout_pos, bool* inout_success, sysDateTime* out_datetime, sysChar in_separator);

void sysUpdateDayOfWeek( sysDateTime* inout_date );
bool sysIsLeapYear( uint16_t in_year );
uint8_t sysGetMonthLength( uint16_t in_year, uint8_t in_month );
void sysAddOneSecond( sysDateTime* in_datetime );
bool sysIsEqualDateTime( sysDateTime* in_datetime1, sysDateTime* in_datetime2 );
void sysNormalizeDateTime( sysDateTime* in_datetime );
int sysCompareDateTime( sysDateTime* in_datetime1, sysDateTime* in_datetime2 );
int sysCompareTime( sysDateTime* in_datetime1, sysDateTime* in_datetime2 );
void sysSubstractTime( sysDateTime* in_time, sysDateTime* in_time_to_substract );
void sysSetClockAdjust( int16_t in_adjust );
int16_t sysGetClockAdjust(void);

#endif
