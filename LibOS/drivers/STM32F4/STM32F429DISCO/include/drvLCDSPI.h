/*****************************************************************************/
/* Graphics LCD SPI interface                                                */
/*                                                                           */
/* Copyright (C) 2014 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

#ifndef __halLCDSPI_h
#define __halLCDSPI_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysTypes.h>

/*****************************************************************************/
/* Function prototypes                                                       */
/*****************************************************************************/
void drvLCDSPIinitialize(void);
void drvLCDSPIWriteData(uint16_t in_reg_value);
void drvLCDSPIWriteReg(uint8_t Reg);
void drvLCDSPIDelay(uint32_t in_delay);


#endif
