/*****************************************************************************/
/*                                                                           */
/*    Domino Operation System Driver Module                                  */
/*    Mass Storage Driver                                                    */
/*                                                                           */
/*    Copyright (C) 2006 Laszlo Arvai                                        */
/*                                                                           */
/*****************************************************************************/

#ifndef __drvMassStorage_h
#define __drvMassStorage_h

///////////////////////////////////////////////////////////////////////////////
// Includes
#include <sysTypes.h>


#define fileIOFUNC_MEDIA_DETECTED 1
#define fileIOFUNC_DETECT_AND_INIT_MEDIA 2

///////////////////////////////////////////////////////////////////////////////
// Function prototypes
void drvMSInitialize(void);
void drvMSCleanUp(void);
bool drvMSReadSector( uint8_t* out_buffer, uint32_t in_lba );
bool drvMSWriteSector( uint8_t* in_buffer, uint32_t in_lba );
uint32_t drvMSIOControl(uint16_t in_function_code);

#endif