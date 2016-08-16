#ifndef __fbBuffer_h
#define __fbBuffer_h

///////////////////////////////////////////////////////////////////////////////
// Includes
#include <sysTypes.h>
#include <fbTypes.h>

///////////////////////////////////////////////////////////////////////////////
// Constants

//  sort mode
#define fbSM_ASCENDING 0
#define fbSM_DESCENDING 0x80
#define fbSM_DIRECTION_MASK 0x80
#define fbSM_NAME 0
#define fbSM_SIZE 1
#define fbSM_DATE 2
#define fbSM_TYPE_MASK 0x7f

#define fbINVALID_INDEX 65535

///////////////////////////////////////////////////////////////////////////////
// Function prototypes
void fbBufferSet(uint8_t* in_buffer, uint32_t in_buffer_size);
void fbBufferClear(void);
bool fbBufferStore(fbFileInformation* in_file_info);
void fbBufferSortStart(void);
bool fbBufferSort(void);
uint16_t fbBufferGetFileCount(void);
uint16_t fbGetDefaultFileIndex(void);
fbFileInformation* fbBufferGetFileInfo(uint16_t in_index);



#endif