/*****************************************************************************/
/* Intel 8080 CPU Emulator                                                   */
/*                                                                           */
/* Copyright (C) 2015 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <cpuI8080.h>

/*****************************************************************************/
/* Tables                                                                    */
/*****************************************************************************/

//#define DEBUG

#ifdef DEBUG
#include <stdio.h>
#endif

/* i8080 CPU */
/* opcode cycles */
static uint8_t l_cpu_instruction_cycles[256]={
/*       x0  x1  x2	 x3	 x4  x5  x6  x7	 x8	 x9	 xA  xB	 xC	 xD	 xE	 xF	*/
/* 0x */	4, 10,  7,  5,  5,  5,  7,  4,  0, 10,  7,  5,  5,  5,  7,  4,
/* 1x */	0, 10,  7,  5,  5,  5,  7,  4,  0, 10,  7,  5,  5,  5,  7,  4,
/* 2x */	0, 10, 16,  5,  5,  5,  7,  4,  0, 10, 16,  5,  5,  5,  7,  4,
/* 3x */	0, 10, 13,  5, 10, 10, 10,  4,  0, 10, 13,  5,  5,  5,  7,  4,
/* 4x */	5,  5,  5,  5,  5,  5,  7,  5,  5,  5,  5,  5,  5,  5,  7,  5,
/* 5x */	5,  5,  5,  5,  5,  5,  7,  5,  5,  5,  5,  5,  5,  5,  7,  5,
/* 6x */	5,  5,  5,  5,  5,  5,  7,  5,  5,  5,  5,  5,  5,  5,  7,  5,
/* 7x */	7,  7,  7,  7,  7,  7,  7,  7,  5,  5,  5,  5,  5,  5,  7,  5,
/* 8x */	4,  4,  4,  4,  4,  4,  7,  4,  4,  4,  4,  4,  4,  4,  7,  4,
/* 9x */	4,  4,  4,  4,  4,  4,  7,  4,  4,  4,  4,  4,  4,  4,  7,  4,
/* Ax */	4,  4,  4,  4,  4,  4,  7,  4,  4,  4,  4,  4,  4,  4,  7,  4,
/* Bx */	4,  4,  4,  4,  4,  4,  7,  4,  4,  4,  4,  4,  4,  4,  7,  4,
/* Cx */	5, 10, 10, 10, 11, 11,  7, 11,  5, 10, 10,  0, 11, 17,  7, 11,
/* Dx */	5, 10, 10, 10, 11, 11,  7, 11,  5,  0, 10, 10, 11,  0,  7, 11,
/* Ex */	5, 10, 10, 18, 11, 11,  7, 11,  5,  5, 10,  4, 11,  0,  7, 11,
/* Fx */	5, 10, 10,  4, 11, 11,  7, 11,  5,  5, 10,  4, 11,  0,  7, 11
};

// Sign, Zero, Parity flags look-up table
static uint8_t l_cpu_szp_flags[256] = 
{
 /* 0x */   cpuI8080_F_ZERO|cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_UN1, cpuI8080_F_UN1, cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_UN1, cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_UN1, cpuI8080_F_UN1, cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_UN1, cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_UN1, cpuI8080_F_UN1, cpuI8080_F_PARITY|cpuI8080_F_UN1, 
 /* 1x */   cpuI8080_F_UN1, cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_UN1, cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_UN1, cpuI8080_F_UN1, cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_UN1, cpuI8080_F_UN1, cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_UN1, cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_UN1, 
 /* 2x */   cpuI8080_F_UN1, cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_UN1, cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_UN1, cpuI8080_F_UN1, cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_UN1, cpuI8080_F_UN1, cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_UN1, cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_UN1, 
 /* 3x */   cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_UN1, cpuI8080_F_UN1, cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_UN1, cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_UN1, cpuI8080_F_UN1, cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_UN1, cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_UN1, cpuI8080_F_UN1, cpuI8080_F_PARITY|cpuI8080_F_UN1, 
 /* 4x */   cpuI8080_F_UN1, cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_UN1, cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_UN1, cpuI8080_F_UN1, cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_UN1, cpuI8080_F_UN1, cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_UN1, cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_UN1, 
 /* 5x */   cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_UN1, cpuI8080_F_UN1, cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_UN1, cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_UN1, cpuI8080_F_UN1, cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_UN1, cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_UN1, cpuI8080_F_UN1, cpuI8080_F_PARITY|cpuI8080_F_UN1, 
 /* 6x */   cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_UN1, cpuI8080_F_UN1, cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_UN1, cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_UN1, cpuI8080_F_UN1, cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_UN1, cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_UN1, cpuI8080_F_UN1, cpuI8080_F_PARITY|cpuI8080_F_UN1, 
 /* 7x */   cpuI8080_F_UN1, cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_UN1, cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_UN1, cpuI8080_F_UN1, cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_UN1, cpuI8080_F_UN1, cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_UN1, cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_UN1, 
 /* 8x */   cpuI8080_F_SIGN|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_UN1, 
 /* 9x */   cpuI8080_F_SIGN|cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_PARITY|cpuI8080_F_UN1, 
 /* Ax */   cpuI8080_F_SIGN|cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_PARITY|cpuI8080_F_UN1, 
 /* Bx */   cpuI8080_F_SIGN|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_UN1, 
 /* Cx */   cpuI8080_F_SIGN|cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_PARITY|cpuI8080_F_UN1, 
 /* Dx */   cpuI8080_F_SIGN|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_UN1, 
 /* Ex */   cpuI8080_F_SIGN|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_UN1, 
 /* Fx */   cpuI8080_F_SIGN|cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_PARITY|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_UN1, cpuI8080_F_SIGN|cpuI8080_F_PARITY
};

// Look-up tables for auxilary carry calculation 
static const uint8_t l_cpu_inr_aux[16]=	{	cpuI8080_F_AUXCARRY, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
static const uint8_t l_cpu_dcr_aux[16]= {	cpuI8080_F_AUXCARRY, cpuI8080_F_AUXCARRY, cpuI8080_F_AUXCARRY, cpuI8080_F_AUXCARRY, cpuI8080_F_AUXCARRY, cpuI8080_F_AUXCARRY, cpuI8080_F_AUXCARRY, cpuI8080_F_AUXCARRY, cpuI8080_F_AUXCARRY, cpuI8080_F_AUXCARRY, cpuI8080_F_AUXCARRY, cpuI8080_F_AUXCARRY, cpuI8080_F_AUXCARRY, cpuI8080_F_AUXCARRY, cpuI8080_F_AUXCARRY, 0 };
static const uint8_t l_cpu_sub_aux[8] = { cpuI8080_F_AUXCARRY, 0, 0, 0, cpuI8080_F_AUXCARRY, cpuI8080_F_AUXCARRY, cpuI8080_F_AUXCARRY, 0 };

#if PROFILE
static unsigned int lut_profiler[0x100]; /* occurance of opcodes */
#endif

/* opcode mnemonics (debug) */
//#if defined(CPU_DEBUG) | defined(PROFILE)
/*static*/ const char* lut_mnemonic[0x100]={
	"nop",     "lxi b,#", "stax b",  "inx b",   "inr b",   "dcr b",   "mvi b,#", "rlc",     "ill",     "dad b",   "ldax b",  "dcx b",   "inr c",   "dcr c",   "mvi c,#", "rrc",
	"ill",     "lxi d,#", "stax d",  "inx d",   "inr d",   "dcr d",   "mvi d,#", "ral",     "ill",     "dad d",   "ldax d",  "dcx d",   "inr e",   "dcr e",   "mvi e,#", "rar",
	"ill",     "lxi h,#", "shld",    "inx h",   "inr h",   "dcr h",   "mvi h,#", "daa",     "ill",     "dad h",   "lhld",    "dcx h",   "inr l",   "dcr l",   "mvi l,#", "cma",
	"ill",     "lxi sp,#","sta $",   "inx sp",  "inr M",   "dcr M",   "mvi M,#", "stc",     "ill",     "dad sp",  "lda $",   "dcx sp",  "inr a",   "dcr a",   "mvi a,#", "cmc",
	"mov b,b", "mov b,c", "mov b,d", "mov b,e", "mov b,h", "mov b,l", "mov b,M", "mov b,a", "mov c,b", "mov c,c", "mov c,d", "mov c,e", "mov c,h", "mov c,l", "mov c,M", "mov c,a",
	"mov d,b", "mov d,c", "mov d,d", "mov d,e", "mov d,h", "mov d,l", "mov d,M", "mov d,a", "mov e,b", "mov e,c", "mov e,d", "mov e,e", "mov e,h", "mov e,l", "mov e,M", "mov e,a",
	"mov h,b", "mov h,c", "mov h,d", "mov h,e", "mov h,h", "mov h,l", "mov h,M", "mov h,a", "mov l,b", "mov l,c", "mov l,d", "mov l,e", "mov l,h", "mov l,l", "mov l,M", "mov l,a",
	"mov M,b", "mov M,c", "mov M,d", "mov M,e", "mov M,h", "mov M,l", "hlt",     "mov M,a", "mov a,b", "mov a,c", "mov a,d", "mov a,e", "mov a,h", "mov a,l", "mov a,M", "mov a,a",
	"add b",   "add c",   "add d",   "add e",   "add h",   "add l",   "add M",   "add a",   "adc b",   "adc c",   "adc d",   "adc e",   "adc h",   "adc l",   "adc M",   "adc a",
	"sub b",   "sub c",   "sub d",   "sub e",   "sub h",   "sub l",   "sub M",   "sub a",   "sbb b",   "sbb c",   "sbb d",   "sbb e",   "sbb h",   "sbb l",   "sbb M",   "sbb a",
	"ana b",   "ana c",   "ana d",   "ana e",   "ana h",   "ana l",   "ana M",   "ana a",   "xra b",   "xra c",   "xra d",   "xra e",   "xra h",   "xra l",   "xra M",   "xra a",
	"ora b",   "ora c",   "ora d",   "ora e",   "ora h",   "ora l",   "ora M",   "ora a",   "cmp b",   "cmp c",   "cmp d",   "cmp e",   "cmp h",   "cmp l",   "cmp M",   "cmp a",
	"rnz",     "pop b",   "jnz $",   "jmp $",   "cnz $",   "push b",  "adi #",   "rst 0",   "rz",      "ret",     "jz $",    "ill",     "cz $",    "call $",  "aci #",   "rst 1",
	"rnc",     "pop d",   "jnc $",   "out p",   "cnc $",   "push d",  "sui #",   "rst 2",   "rc",      "ill",     "jc $",    "in p",    "cc $",    "ill",     "sbi #",   "rst 3",
	"rpo",     "pop h",   "jpo $",   "xthl",    "cpo $",   "push h",  "ani #",   "rst 4",   "rpe",     "pchl",    "jpe $",   "xchg",    "cpe $",   "ill",     "xri #",   "rst 5",
	"rp",      "pop psw", "jp $",    "di",      "cp $",    "push psw","ori #",   "rst 6",   "rm",      "sphl",    "jm $",    "ei",      "cm $",    "ill",     "cpi #",   "rst 7"
};
//#endif

/*****************************************************************************/
/* Register access maros                                                     */
/*****************************************************************************/

#define A(R)   R->reg.a
#define F(R)   R->reg.flags
#define B(R)   R->reg.b
#define C(R)   R->reg.c
#define D(R)   R->reg.d
#define E(R)   R->reg.e
#define H(R)   R->reg.h
#define L(R)   R->reg.l
#define PC(R)  R->reg.pc
#define PCL(R) R->reg.pc_low
#define PCH(R) R->reg.pc_high
#define SP(R)  R->reg.sp
#define SPL(R) R->reg.sp_low
#define SPH(R) R->reg.sp_high
#define PSW(R) R->reg.psw
#define BC(R)  R->reg.bc
#define DE(R)  R->reg.de
#define HL(R)  R->reg.hl
#define RES(R) R->result
#define INT(R) R->i
#define AUX(R) R->ac
#define IPEND(R) R->ipend
#define CYCLES(R) R->cycles
#define HALTED(R) R->halted

/*****************************************************************************/
/* Condition macros                                                          */
/*****************************************************************************/

#define ISNOTZERO(R)		((F(R)&cpuI8080_F_ZERO)==0)
#define ISZERO(R)				((F(R)&cpuI8080_F_ZERO)!=0)
#define ISNOTCARRY(R)		((RES(R)&0x100)==0)
#define ISCARRY(R)			((RES(R)&0x100)!=0)
#define ISPODD(R)			  ((F(R)&cpuI8080_F_PARITY)==0)
#define ISPEVEN(R)			((F(R)&cpuI8080_F_PARITY)!=0)
#define ISPLUS(R)				((F(R)&cpuI8080_F_SIGN)==0)
#define ISMIN(R)				((F(R)&cpuI8080_F_SIGN)!=0)

/*****************************************************************************/
/* Instruction helper macros                                                 */
/*****************************************************************************/

#define SKIP16(R) PC(R) += 2
#define JUMP(R)   PC(R) = Read16(PC(R))
#define CALL(R)   Push16(R, PC(R)+2); JUMP(R)
#define CCON(R)   CYCLES(R)-=6; CALL(R)
#define RET(R)    PC(R) = Pop16(R)
#define RCON(R)   CYCLES(R)-=6; RET(R)
#define RST(R, x) Push16(R, PC(R)); PC(R) = (x)<<3
#define CHGSZP(R,r) F(R)=l_cpu_szp_flags[r]
#define INR(R, r) r++; AUX(R)=l_cpu_inr_aux[(r)&0x0f]; CHGSZP(R, r)
#define DCR(R, r) r--; AUX(R)=l_cpu_dcr_aux[(r)&0x0f]; CHGSZP(R, r)
#define ADDAUX(R,r) ((A(R) ^ r ^ RES(R)) & 0x10)
#define SUBAUX(R,r)	l_cpu_sub_aux[(((A(R)&0x08)>>1)|(((r)&0x08)>>2)|((RES(R)&0x08)>>3))]
#define ADD(R,r)  RES(R) = A(R) + r; AUX(R)=ADDAUX(R,r); A(R)=RES(R)&0xff; CHGSZP(R, A(R))
#define ADC(R,r)  RES(R) =(RES(R)>>8&1)+A(R)+r; AUX(R)=ADDAUX(R,r); A(R)=RES(R)&0xff; CHGSZP(R, A(R))
#define DAD(R,r)  temp_dword = (uint32_t)r+HL(R); RES(R)=(RES(R)&0xff)|((temp_dword>>8)&0x100); HL(R) = temp_dword&0xffff
#define CMP(R,r)  RES(R)=A(R)-r; AUX(R)=SUBAUX(R,r); CHGSZP(R, RES(R)&0xff)
#define SUB(R,r)  RES(R)=A(R)-r; AUX(R)=SUBAUX(R,r); A(R)=RES(R)&0xff; CHGSZP(R, A(R))
#define SBB(R,r)  RES(R)=(A(R)-r)-(RES(R)>>8&1); AUX(R)=SUBAUX(R,r); A(R)=RES(R)&0xff; CHGSZP(R, A(R))
#define ANA(R,r)  AUX(R)=(((A(R) | r) & 0x08)<<1); RES(R)=A(R)=A(R)&r; CHGSZP(R, A(R))
#define XRA(R,r)  RES(R)=A(R)=A(R)^r;AUX(R)=0; CHGSZP(R, A(R))
#define ORA(R,r)  RES(R)=A(R)=A(R)|r;AUX(R)=0; CHGSZP(R, A(R))

/*****************************************************************************/
/* Local functions                                                           */
/*****************************************************************************/

static void Push16(cpuI8080State* R, uint16_t Value);
static uint16_t Pop16(cpuI8080State* R);
static uint16_t Read16(uint16_t Address);

///////////////////////////////////////////////////////////////////////////////
/// @brief Resets emulated I8080 CPU
/// @param R CPU registers and status information
void cpuI8080Reset(cpuI8080State *R)
{
	// init registers
	PC(R) = 0;
	SP(R) = 0;
	A(R) = 0;
	BC(R) = 0;
	DE(R) = 0;
	HL(R) = 0;
	CYCLES(R) = 0;
	RES(R) = 0;        /* temp result */
	INT(R) = 0;        /* interrupt bit */
	IPEND(R) = 0;      /* pending int */
	AUX(R) = 0;        /* aux carry bit */
	HALTED(R) = 0;
	F(R)=cpuI8080_F_UN1;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Generates interrupt request
/// @param R CPU registers and status information
/// @param vector Interrupt vector to request
void cpuI8080INT(cpuI8080State* R, uint16_t vector)
{
	if (INT(R))
	{
		if(HALTED(R))
		{
			HALTED(R) = 0;
			PC(R)++;
		}

		INT(R) = IPEND(R) = 0;
		CYCLES(R) -= 11;

		RST(R, vector);
	}
	else
	{
		IPEND(R) = 0x80 | vector;
	}
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Actializes flag (F) register content based on the actual processor state
/// @param R CPU registers and status information
void cpuI8080UpdateFlags(cpuI8080State* R)
{
	F(R) = (F(R) & (cpuI8080_F_SIGN|cpuI8080_F_ZERO|cpuI8080_F_PARITY)) | (RES(R)>>8&1) | AUX(R) | cpuI8080_F_UN1;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Executes instructions (runs processor)
/// @param R CPU registers and status information
/// @param cycles Minimum number of CPU cycles to execute
/// @return Difference of the executed cycles compared to the requested cycles
int cpuI8080Exec(cpuI8080State* R, int cycles)
{
	uint8_t opcode;
	uint8_t temp_byte;
	uint16_t temp_word;
	uint32_t temp_dword;

	CYCLES(R) += cycles;

	while (CYCLES(R)>0)
	{
		opcode = RdI8080(PC(R));
		PC(R)++;

		CYCLES(R) -= l_cpu_instruction_cycles[opcode];

		switch (opcode)
		{
			/* MOVE, LOAD, AND STORE */
		case 0x40: break;                         // mov b,b
		case 0x41: B(R) = C(R); break;            // mov b,c
		case 0x42: B(R) = D(R); break;            // mov b,d
		case 0x43: B(R) = E(R); break;            // mov b,e
		case 0x44: B(R) = H(R); break;            // mov b,h
		case 0x45: B(R) = L(R); break;            // mov b,l
		case 0x46: B(R) = RdI8080(HL(R)); break;  // mov b,M
		case 0x47: B(R) = A(R); break;            // mov b,a

		case 0x48: C(R) = B(R); break;            // mov c,b
		case 0x49: break;                         // mov c,c
		case 0x4a: C(R) = D(R); break;            // mov c,d
		case 0x4b: C(R) = E(R); break;            // mov c,e
		case 0x4c: C(R) = H(R); break;            // mov c,h
		case 0x4d: C(R) = L(R); break;            // mov c,l
		case 0x4e: C(R) = RdI8080(HL(R)); break;  // mov c,M
		case 0x4f: C(R) = A(R); break;            // mov c,a

		case 0x50: D(R) = B(R); break;            // mov d,b
		case 0x51: D(R) = C(R); break;            // mov d,c
		case 0x52: break;                         // mov d,d
		case 0x53: D(R) = E(R); break;            // mov d,e
		case 0x54: D(R) = H(R); break;            // mov d,h
		case 0x55: D(R) = L(R); break;            // mov d,l
		case 0x56: D(R) = RdI8080(HL(R)); break;  // mov d,M
		case 0x57: D(R) = A(R); break;            // mov d,a

		case 0x58: E(R) = B(R); break;            // mov e,b
		case 0x59: E(R) = C(R); break;            // mov e,c
		case 0x5a: E(R) = D(R); break;            // mov e,d
		case 0x5b: break;                         // mov e,e
		case 0x5c: E(R) = H(R); break;						// mov e,h
		case 0x5d: E(R) = L(R); break;            // mov e,l
		case 0x5e: E(R) = RdI8080(HL(R)); break;  // mov e,M
		case 0x5f: E(R) = A(R); break;            // mov e,a

		case 0x60: H(R) = B(R); break;						// mov h,b
		case 0x61: H(R) = C(R); break;						// mov h,c
		case 0x62: H(R) = D(R); break;						// mov h,d
		case 0x63: H(R) = E(R); break;						// mov h,e
		case 0x64: break;													// mov h,h
		case 0x65: H(R) = L(R); break;						// mov h,l
		case 0x66: H(R) = RdI8080(HL(R)); break;  // mov h,M
		case 0x67: H(R) = A(R); break;						// mov h,a

		case 0x68: L(R) = B(R); break;						// mov l,b
		case 0x69: L(R) = C(R); break;						// mov l,c
		case 0x6a: L(R) = D(R); break;						// mov l,d
		case 0x6b: L(R) = E(R); break;						// mov l,e
		case 0x6c: L(R) = H(R); break;						// mov l,h
		case 0x6d: break;													// mov l,l
		case 0x6e: L(R) = RdI8080(HL(R)); break;  // mov l,M
		case 0x6f: L(R) = A(R); break;						// mov l,a

		case 0x70: WrI8080(HL(R), B(R)); break;   // mov M,b
		case 0x71: WrI8080(HL(R), C(R)); break;   // mov M,c
		case 0x72: WrI8080(HL(R), D(R)); break;   // mov M,d
		case 0x73: WrI8080(HL(R), E(R)); break;   // mov M,e
		case 0x74: WrI8080(HL(R), H(R)); break;   // mov M,h
		case 0x75: WrI8080(HL(R), L(R)); break;   // mov M,l
																							// HLT
		case 0x77: WrI8080(HL(R), A(R)); break;   // mov M,a

		case 0x78: A(R) = B(R); break;						// mov a,b
		case 0x79: A(R) = C(R); break;						// mov a,c
		case 0x7a: A(R) = D(R); break;						// mov a,d
		case 0x7b: A(R) = E(R); break;						// mov a,e
		case 0x7c: A(R) = H(R); break;						// mov a,h
		case 0x7d: A(R) = L(R); break;						// mov a,l
		case 0x7e: A(R) = RdI8080(HL(R)); break;  // mov a,M
		case 0x7f: break;													// mov a,a

			/* MVI */
		case 0x06: B(R) = RdI8080(PC(R)); PC(R)++; break;							// mvi b,#
		case 0x0e: C(R) = RdI8080(PC(R)); PC(R)++; break;							// mvi c,#
		case 0x16: D(R) = RdI8080(PC(R)); PC(R)++; break;							// mvi d,#
		case 0x1e: E(R) = RdI8080(PC(R)); PC(R)++; break;							// mvi e,#
		case 0x26: H(R) = RdI8080(PC(R)); PC(R)++; break;							// mvi h,#
		case 0x2e: L(R) = RdI8080(PC(R)); PC(R)++; break;							// mvi l,#
		case 0x36: WrI8080(HL(R), RdI8080(PC(R))); PC(R)++; break;    // mvi M,#
		case 0x3e: A(R) = RdI8080(PC(R)); PC(R)++; break;							// mvi a,#

		case 0x01: BC(R) = Read16(PC(R)); PC(R)+=2; break;						// lxi b,#
		case 0x11: DE(R) = Read16(PC(R)); PC(R)+=2; break;						// lxi d,#
		case 0x21: HL(R) = Read16(PC(R)); PC(R)+=2; break;						// lxi h,#

		case 0x02: WrI8080(BC(R), A(R)); break;												// stax b
		case 0x12: WrI8080(DE(R), A(R)); break;												// stax d
		case 0x0a: A(R) = RdI8080(BC(R)); break;											// ldax b
		case 0x1a: A(R) = RdI8080(DE(R)); break;											// ldax d
		case 0x22: temp_word = Read16(PC(R)); WrI8080(temp_word, L(R)); WrI8080(temp_word+1, H(R)); SKIP16(R); break;		// shld
		case 0x2a: temp_word = Read16(PC(R)); L(R) = RdI8080(temp_word); H(R) = RdI8080(temp_word+1); SKIP16(R); break;	// lhld
		case 0x32: WrI8080(Read16(PC(R)), A(R)); SKIP16(R); break;		// sta $
		case 0x3a: A(R) = RdI8080(Read16(PC(R))); SKIP16(R); break;   // lda $

		case 0xeb: temp_word=DE(R); DE(R)=HL(R); HL(R)=temp_word; break; // xchg


			/* STACK OPS */
		case 0xc5: Push16(R, BC(R)); break;     // push b
		case 0xd5: Push16(R, DE(R)); break;     // push d
		case 0xe5: Push16(R, HL(R)); break;     // push h
		case 0xf5: cpuI8080UpdateFlags(R); Push16(R, PSW(R)); break;  // push psw

		case 0xc1: BC(R) = Pop16(R); break;     // pop b
		case 0xd1: DE(R) = Pop16(R); break;     // pop d
		case 0xe1: HL(R) = Pop16(R); break;     // pop h
		case 0xf1: PSW(R) = Pop16(R); RES(R) = (F(R)<<8&0x100); AUX(R) = F(R)&cpuI8080_F_AUXCARRY; break;  // pop psw

		case 0xe3: temp_word = RdI8080(SP(R)); temp_word |= RdI8080(SP(R)+1) << 8; WrI8080(SP(R), L(R)); WrI8080(SP(R)+1, H(R)); HL(R)=temp_word; break; // xthl

		case 0xf9: SP(R) = HL(R); break;    // sphl

		case 0x31: SP(R) = Read16(PC(R)); SKIP16(R); break;    // lxi sp,#

		case 0x33: SP(R)++; break;					// inx sp
		case 0x3b: SP(R)--; break;					// dcx sp


			/* JUMP */
		case 0xc3: JUMP(R); break;					// jmp $

		case 0xc2: if (ISNOTZERO(R)) { JUMP(R); } else { SKIP16(R); } break;  // jnz $
		case 0xca: if (ISZERO(R)) { JUMP(R); } else { SKIP16(R); } break;			// jz $
		case 0xd2: if (ISNOTCARRY(R)) { JUMP(R); } else { SKIP16(R); } break; // jnc $
		case 0xda: if (ISCARRY(R)) { JUMP(R); } else { SKIP16(R); } break;		// jc $
		case 0xe2: if (ISPODD(R)) { JUMP(R); } else { SKIP16(R); } break;			// jpo $
		case 0xea: if (ISPEVEN(R)) { JUMP(R); } else { SKIP16(R); } break;		// jpe $
		case 0xf2: if (ISPLUS(R)) { JUMP(R); } else { SKIP16(R); } break;			// jp $
		case 0xfa: if (ISMIN(R)) { JUMP(R); } else { SKIP16(R); } break;			// jm $

		case 0xe9: PC(R) = HL(R); break;    // pchl

			/* CALL */
		case 0xcd: CALL(R); break;					// call $

		case 0xc4: if (ISNOTZERO(R)) { CCON(R); } else { SKIP16(R); } break;  // cnz $
		case 0xcc: if (ISZERO(R)) { CCON(R); } else { SKIP16(R); } break;			// cz $
		case 0xd4: if (ISNOTCARRY(R)) { CCON(R); } else { SKIP16(R); } break; // cnc $
		case 0xdc: if (ISCARRY(R)) { CCON(R); } else { SKIP16(R); } break;		// cc $
		case 0xe4: if (ISPODD(R)) { CCON(R); } else { SKIP16(R); } break;			// cpo $
		case 0xec: if (ISPEVEN(R)) { CCON(R); } else { SKIP16(R); } break;		// cpe $
		case 0xf4: if (ISPLUS(R)) { CCON(R); } else { SKIP16(R); } break;			// cp $
		case 0xfc: if (ISMIN(R)) { CCON(R); } else { SKIP16(R); } break;			// cm $


			/* RETURN */
		case 0xc9: RET(R); break;														// ret 

		case 0xc0: if (ISNOTZERO(R)) { RCON(R); } break;		// rnz
		case 0xc8: if (ISZERO(R)) { RCON(R); } break;				// rz
		case 0xd0: if (ISNOTCARRY(R)) { RCON(R); } break;		// rnc
		case 0xd8: if (ISCARRY(R)) { RCON(R); } break;			// rc
		case 0xe0: if (ISPODD(R)) { RCON(R); } break;				// rpo
		case 0xe8: if (ISPEVEN(R)) { RCON(R); } break;			// rpe
		case 0xf0: if (ISPLUS(R)) { RCON(R); } break;				// rp
		case 0xf8: if (ISMIN(R)) { RCON(R); } break;				// rm


			/* RESTART */
		case 0xc7: 
		case 0xcf: 
		case 0xd7: 
		case 0xdf: 
		case 0xe7: 
		case 0xef: 
		case 0xf7: 
		case 0xff:
			RST(R, opcode>>3&7); break;    // rst x


			/* INCREMENT AND DECREMENT */
		case 0x04: INR(R, B(R)); break;     // inr b
		case 0x0c: INR(R, C(R)); break;     // inr c
		case 0x14: INR(R, D(R)); break;     // inr d
		case 0x1c: INR(R, E(R)); break;     // inr e
		case 0x24: INR(R, H(R)); break;     // inr h
		case 0x2c: INR(R, L(R)); break;     // inr l
		case 0x34: temp_byte = RdI8080(HL(R)) + 1; WrI8080(HL(R),temp_byte); AUX(R)=l_cpu_inr_aux[temp_byte&0x0f]; CHGSZP(R, temp_byte); break; // inr M
		case 0x3c: INR(R, A(R)); break;     // inr a

		case 0x05: DCR(R, B(R)); break;     // dcr b
		case 0x0d: DCR(R, C(R)); break;     // dcr c
		case 0x15: DCR(R, D(R)); break;     // dcr d
		case 0x1d: DCR(R, E(R)); break;     // dcr e
		case 0x25: DCR(R, H(R)); break;     // dcr h
		case 0x2d: DCR(R, L(R)); break;     // dcr l
		case 0x35: temp_byte = RdI8080(HL(R)) - 1; WrI8080(HL(R),temp_byte); AUX(R)=l_cpu_dcr_aux[temp_byte&0x0f]; CHGSZP(R, temp_byte); break; // dcr M
		case 0x3d: DCR(R, A(R)); break;     // dcr a

		case 0x03: BC(R)++; break;       // inx b
		case 0x13: DE(R)++; break;       // inx d
		case 0x23: HL(R)++; break;       // inx h

		case 0x0b: BC(R)--; break;       // dcx b
		case 0x1b: DE(R)--; break;       // dcx d
		case 0x2b: HL(R)--; break;       // dcx h


			/* ADD */
		case 0x80: ADD(R, B(R)); break;     // add b
		case 0x81: ADD(R, C(R)); break;     // add c
		case 0x82: ADD(R, D(R)); break;     // add d
		case 0x83: ADD(R, E(R)); break;     // add e
		case 0x84: ADD(R, H(R)); break;     // add h
		case 0x85: ADD(R, L(R)); break;     // add l
		case 0x86: temp_byte = RdI8080(HL(R)); ADD(R, temp_byte); break;      // add M
		case 0x87: ADD(R, A(R)); break;     // add a

		case 0x88: ADC(R, B(R)); break;     // adc b
		case 0x89: ADC(R, C(R)); break;     // adc c
		case 0x8a: ADC(R, D(R)); break;     // adc d
		case 0x8b: ADC(R, E(R)); break;     // adc e
		case 0x8c: ADC(R, H(R)); break;     // adc h
		case 0x8d: ADC(R, L(R)); break;     // adc l
		case 0x8e: temp_byte = RdI8080(HL(R)); ADC(R, temp_byte); break;      // adc M
		case 0x8f: ADC(R, A(R)); break;     // adc a

		case 0xc6: temp_byte = RdI8080(PC(R)); ADD(R, temp_byte); PC(R)++; break;    // adi #
		case 0xce: temp_byte = RdI8080(PC(R)); ADC(R, temp_byte); PC(R)++; break;    // aci #

		case 0x09: DAD(R, BC(R)); break;      // dad b
		case 0x19: DAD(R, DE(R)); break;      // dad d
		case 0x29: DAD(R, HL(R)); break;      // dad h
		case 0x39: DAD(R, SP(R)); break;      // dad sp


			/* SUBTRACT */
		case 0x90: SUB(R, B(R)); break;     // sub b
		case 0x91: SUB(R, C(R)); break;     // sub c
		case 0x92: SUB(R, D(R)); break;     // sub d
		case 0x93: SUB(R, E(R)); break;     // sub e
		case 0x94: SUB(R, H(R)); break;     // sub h
		case 0x95: SUB(R, L(R)); break;     // sub l
		case 0x96: temp_byte = RdI8080(HL(R)); SUB(R, temp_byte); break;      // sub M
		case 0x97: SUB(R, A(R)); break;     // sub a

		case 0x98: SBB(R, B(R)); break;     // sbb b
		case 0x99: SBB(R, C(R)); break;     // sbb c
		case 0x9a: SBB(R, D(R)); break;     // sbb d
		case 0x9b: SBB(R, E(R)); break;     // sbb e
		case 0x9c: SBB(R, H(R)); break;     // sbb h
		case 0x9d: SBB(R, L(R)); break;     // sbb l
		case 0x9e: temp_byte = RdI8080(HL(R)); SBB(R, temp_byte); break;      // sbb M
		case 0x9f: SBB(R, A(R)); break;     // sbb a

		case 0xd6: temp_byte = RdI8080(PC(R)); SUB(R, temp_byte); PC(R)++; break;    // sui #
		case 0xde: temp_byte = RdI8080(PC(R)); SBB(R, temp_byte); PC(R)++; break;    // sbi #


			/* LOGICAL */
		case 0xa0: ANA(R, B(R)); break;     // ana b
		case 0xa1: ANA(R, C(R)); break;     // ana c
		case 0xa2: ANA(R, D(R)); break;     // ana d
		case 0xa3: ANA(R, E(R)); break;     // ana e
		case 0xa4: ANA(R, H(R)); break;     // ana h
		case 0xa5: ANA(R, L(R)); break;     // ana l
		case 0xa6: temp_byte = RdI8080(HL(R)); ANA(R, temp_byte); break;      // ana M
		case 0xa7: ANA(R, A(R)); break;     // ana a

		case 0xe6: temp_byte = RdI8080(PC(R)); ANA(R, temp_byte); PC(R)++; break;    // ani #

		case 0xa8: XRA(R, B(R)); break;     // xra b
		case 0xa9: XRA(R, C(R)); break;     // xra c
		case 0xaa: XRA(R, D(R)); break;     // xra d
		case 0xab: XRA(R, E(R)); break;     // xra e
		case 0xac: XRA(R, H(R)); break;     // xra h
		case 0xad: XRA(R, L(R)); break;     // xra l
		case 0xae: temp_byte = RdI8080(HL(R)); XRA(R, temp_byte); break;      // xra M
		case 0xaf: XRA(R, A(R)); break;     // xra a

		case 0xee: temp_byte = RdI8080(PC(R)); XRA(R, temp_byte); PC(R)++; break;    // xri #

		case 0xb0: ORA(R, B(R)); break;     // ora b
		case 0xb1: ORA(R, C(R)); break;     // ora c
		case 0xb2: ORA(R, D(R)); break;     // ora d
		case 0xb3: ORA(R, E(R)); break;     // ora e
		case 0xb4: ORA(R, H(R)); break;     // ora h
		case 0xb5: ORA(R, L(R)); break;     // ora l
		case 0xb6: temp_byte = RdI8080(HL(R)); ORA(R, temp_byte); break;      // ora M
		case 0xb7: ORA(R, A(R)); break;     // ora a

		case 0xf6: temp_byte = RdI8080(PC(R)); ORA(R, temp_byte); PC(R)++; break;    // ori #

		case 0xb8: CMP(R, B(R)); break;     // cmp b
		case 0xb9: CMP(R, C(R)); break;     // cmp c
		case 0xba: CMP(R, D(R)); break;     // cmp d
		case 0xbb: CMP(R, E(R)); break;     // cmp e
		case 0xbc: CMP(R, H(R)); break;     // cmp h
		case 0xbd: CMP(R, L(R)); break;     // cmp l
		case 0xbe: temp_byte = RdI8080(HL(R)); CMP(R, temp_byte); break;      // cmp M
		case 0xbf: CMP(R, A(R)); break;     // cmp a

		case 0xfe: temp_byte = RdI8080(PC(R)); CMP(R, temp_byte); PC(R)++; break;    // cpi #


			/* ROTATE */
		case 0x07: RES(R)=(RES(R)&0xff)|(A(R)<<1&0x100); A(R) = (A(R)<<1&0xfe)|(RES(R)>>8&1); break;													// rlc
		case 0x0f: RES(R)=(RES(R)&0xff)|(A(R)<<8&0x100); A(R) = (A(R)>>1&0x7f)|(RES(R)>>1&0x80); break;												// rrc
		case 0x17: temp_word=((uint16_t)A(R))<<1&0x100; A(R)=A(R)<<1|(RES(R)>>8&1); RES(R)=(RES(R)&0xff)|temp_word; break;		// ral
		case 0x1f: temp_word=((uint16_t)A(R))<<8&0x100; A(R)=A(R)>>1|(RES(R)>>1&0x80); RES(R)=(RES(R)&0xff)|temp_word; break; // rar


			/* SPECIALS */
		case 0x2f: A(R) = ~A(R); break;				// cma
		case 0x37: RES(R) |= 0x100; break;    // stc
		case 0x3f: RES(R) ^= 0x100; break;    // cmc

		case 0x27:                            // daa
			{
				int c=RES(R);
				uint8_t value_to_add = 0;
				if (AUX(R)||((A(R)&0xf)>9))
				{
					value_to_add = 6; 
				}
				if ((c&0x100) || ((A(R)&0xf0)>0x90) || (((A(R)&0xf0)>=0x90) && (A(R) & 0x0f) > 9)) 
				{
					value_to_add |= 0x60;
          c |= 0x100;
        }
				RES(R) = A(R) + value_to_add; 
				AUX(R)=ADDAUX(R,value_to_add); // set aux carry
				A(R)=RES(R)&0xff;								// store result in A
				RES(R)=c;												// store carry
				CHGSZP(R, A(R)); // set other flag bits (PZS)
				break;
			}


			/* INPUT/OUTPUT */
		case 0xd3: OutI8080(RdI8080(PC(R)), A(R)); PC(R)++; break;	// out p
		case 0xdb: A(R)=InI8080(RdI8080(PC(R))); PC(R)++; break;		// in p


			/* CONTROL */
		case 0xf3: INT(R) = 0; break;																												// di
		case 0xfb: INT(R) = 1; if (IPEND(R) & 0x80) cpuI8080INT(R, IPEND(R) & 0x7f); break; // ei 
		case 0x00: break;       // nop
		case 0x76: HALTED(R) = 1; PC(R)--; break;	// hlt (mov M,M)

		default: break;
		}

#if defined(CPU_DEBUG)
		printf("%04x:%10s a%02X f%02X b%02X c%02X d%02X e%02X h%02X l%02X sp%04X\n",PC(R),lut_mnemonic[opcode],A(R),F(R),B(R),C(R),D(R),E(R),H(R),L(R),SP(R));
#endif

#if PROFILE
		lut_profiler[opcode]++;
#endif

	}

	return CYCLES(R);
}

/*****************************************************************************/
/* Local functions                                                           */
/*****************************************************************************/
static void Push16(cpuI8080State* R, uint16_t Value)
{
	SP(R) -= 2;
	WrI8080(SP(R), Value & 0xff);
	WrI8080(SP(R)+1, (Value >> 8) & 0xff );
}

static uint16_t Pop16(cpuI8080State* R)
{
	uint16_t value = ((RdI8080(SP(R)+1)<<8) | RdI8080(SP(R)));
	SP(R) += 2;

	return value;
}

static uint16_t Read16(uint16_t Address)
{
	return ((uint16_t)RdI8080(Address+1) << 8) + RdI8080(Address);
}

