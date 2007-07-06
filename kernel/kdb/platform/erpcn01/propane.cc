/*********************************************************************
 *                
 * Copyright (C) 2002,   University of New South Wales
 *                
 * File path:     platform/erpcn01/propane.cc
 * Description:   
 *                
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *                
 * $Id: propane.cc,v 1.3 2003/09/24 19:05:18 skoglund Exp $
 *                
 ********************************************************************/

#include INC_PLAT(gt64115.h)

volatile unsigned int *propane;
volatile unsigned int *propane_uart;

/* Setup / Check for propane interface */
int propane_init()
{
	int i;
	volatile unsigned int *reg;

	/* Auto detect FPGA Address for Propane */
	propane = (unsigned int *)0x9000000000000000UL;   // Uncached

	reg = (unsigned int *)(0x9000000000000000UL | 0x14000000 | GT_CS_2_0_LOW_DECODE_ADDRESS);
	propane = (unsigned int *)((unsigned long)propane | (((unsigned int)(*reg)&0x7FF)<<21));

	reg = (unsigned int *)(0x9000000000000000UL | 0x14000000 | GT_CS_1_LOW_DECODE_ADDRESS);
	propane = (unsigned int *)((unsigned long)propane | (((unsigned int)(*reg)&0xFF)<<20));

	if ((propane[0] & 0xF0000000) != 0xE0000000)
		return -1;

	if (((propane[0] & 0x0F000000) >> 24) == 0x1)
	{
		propane[2] = 0x0;		/* Disable interrupts */
	}
	else
		return -1;

	for (i=0; i<8; i++) {
		if ((propane[0]&0xF) & (1<<i)) {
			if ((propane[i<<18] & 0xF0000000) == 0xA0000000) {
				propane_uart = &propane[i<<18];
				break;
			}
		}
	}

	return 0;
}
