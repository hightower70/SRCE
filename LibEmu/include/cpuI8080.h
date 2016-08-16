/*****************************************************************************/
/* Intel 8080 CPU Emulator                                                   */
/*                                                                           */
/* Copyright (C) 2015 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/
#ifndef __cpuI8080_h
#define __cpuI8080_h

///////////////////////////////////////////////////////////////////////////////
// Includes
#include <stdint.h>

///////////////////////////////////////////////////////////////////////////////
// Constants

// Reset vectors
#define cpuI8080_RST0 0
#define cpuI8080_RST1 1
#define cpuI8080_RST2 2
#define cpuI8080_RST3 3
#define cpuI8080_RST4 4
#define cpuI8080_RST5 5
#define cpuI8080_RST6 6
#define cpuI8080_RST7 7

// Flag bits
#define cpuI8080_F_CARRY         0x01
#define cpuI8080_F_UN1           0x02
#define cpuI8080_F_PARITY        0x04
#define cpuI8080_F_UN3           0x08
#define cpuI8080_F_AUXCARRY      0x10
#define cpuI8080_F_UN5           0x20
#define cpuI8080_F_ZERO          0x40
#define cpuI8080_F_SIGN          0x80

typedef struct {

  union {
    struct {
      uint16_t pc;    /* programcounter */
      uint16_t sp;    /* stackpointer */
      uint16_t psw,bc,de,hl;  /* register pairs */
    };
    struct {
      uint8_t pc_low,pc_high;
      uint8_t sp_low,sp_high;
      uint8_t flags;    /* sz0a0p1c */
      uint8_t a,c,b,e,d,l,h;  /* regs */
    };
  } reg;

  int32_t cycles;
  uint16_t result;        /* temp result */
  uint8_t i;        /* interrupt bit */
  uint8_t ipend;        /* pending int */
	uint8_t ac;						/* auxilary carry */
	uint8_t halted;
} cpuI8080State;

void cpuI8080Reset(cpuI8080State* R);

void cpuI8080INT(cpuI8080State* R, uint16_t vector);
int cpuI8080Exec(cpuI8080State* R, int cycles);
void cpuI8080UpdateFlags(cpuI8080State* R);

void WrI8080(register uint16_t Addr,register uint8_t Value);
uint8_t RdI8080(register uint16_t Addr);

void OutI8080(register uint16_t Port,register uint8_t Value);
uint8_t InI8080(register uint16_t Port);

#endif
