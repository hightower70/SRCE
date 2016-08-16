#ifndef __fbTypes_h
#define __fbTypes_h

#include <sysTypes.h>
#include <sysString.h>
#include <sysDateTime.h>

///////////////////////////////////////////////////////////////////////////////
// Types

// File flags
#define fbFF_FOLDER_FILE_BIT	0
#define fbFF_FILE		(0<<fbFF_FOLDER_FILE_BIT)
#define fbFF_FOLDER	(1<<fbFF_FOLDER_FILE_BIT)

#define fbFF_DEFAULT (1<<7)


typedef struct 
{
	uint8_t Flags;
	sysString FileName;
	uint32_t Size;
	sysDateTime DateTime;
}	fbFileInformation;



#endif