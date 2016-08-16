#ifndef __fileSDCard_h
#define __fileSDCard_h

#include <sysTypes.h>
#include <fileTypes.h>

bool drvSDCardDetectMedia(void);
bool drvSDCardInitMedia(void);
bool drvSDCardReadSector( uint8_t* out_buffer, uint32_t in_lba );
void drvSDCardCleanUp(void);
uint32_t drvSDCardIOControl(uint16_t in_function_code);

#endif