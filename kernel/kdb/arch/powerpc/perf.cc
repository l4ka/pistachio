/****************************************************************************
 *
 * Copyright (C) 2002, Karlsruhe University
 *
 * File path:	kdb/arch/powerpc/perf.cc
 * Description:	Performance analysis functions.
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
 * $Id: perf.cc,v 1.3 2003/09/24 19:05:08 skoglund Exp $
 *
 ***************************************************************************/

#include <debug.h>
#include <kdb/kdb.h>

#include INC_ARCH(ibm750.h)

DECLARE_CMD_GROUP( perf );
DECLARE_CMD( cmd_perf, arch, 'p', "perf", "Performance menu" );

DECLARE_CMD( cmd_addr_switch, perf, 'a', "addrswitch", "Address switch" );
DECLARE_CMD( cmd_multi_word, perf, 's', "multiword", "Multi-word store/load" );

CMD( cmd_perf, cg )
{
    return perf.interact( cg, "perf" );
}

CMD( cmd_addr_switch, cg )
{
    word_t dummy, start, stop;
    int i;

    for( i = 0; i < 10; i++ ) {
    	asm volatile (
    		"mfpmc1 %1;"
		"addi %0, %0, 1;"
		"addi %0, %0, 1;"
		"addi %0, %0, 1;"
		"addi %0, %0, 1;"
		"addi %0, %0, 1;"
		"addi %0, %0, 1;"
		"addi %0, %0, 1;"
		"addi %0, %0, 1;"
		"addi %0, %0, 1;"
		"addi %0, %0, 1;"
		"mfpmc1 %2;"
		: "=r" (dummy), "=r" (start), "=r" (stop)
		:
		: "r6", "r7", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
		);

	printf( "addi --> %d/10\n", (stop-start) );
    }

    return CMD_NOQUIT;
}

CMD( cmd_multi_word, cg )
{
    word_t regs[28];
    word_t count = 1000;
    word_t start, stop, start2, stop2;

    start = stop = 0;
    start2 = stop2 = 0;

    /* The 500 MHz 750 requires 31 cycles for the 28 register store, and 
     * 32 cycles for the 28 register load.  Doesn't look like the 750
     * cripples this instruction.
     */

    asm volatile( 
	    "mtctr %5 ;"		// load the loop count
	    "mfpmc1 %0 ;"		// grab the start time
	    "mr %%r3, %4 ;"		// ensure that the start index is
	    				// not included in the registers to save
	    "1: "
	    "stmw %%r4, 0(%%r3) ;"	// store the registers
	    "bdnz 1b ;"			// loop
	    "mfpmc1 %1 ;"		// grab the stop time
	    "stmw %%r4, 0(%%r3) ;"	// save the stop time

	    "mtctr %5 ;"		// load the loop count
	    "mfpmc1 %%r0 ;"		// grab the start time
	    "2:"
	    "lmw %%r4, 0(%%r3) ;"	// load the registers
	    "bdnz 2b ;"			// loop
	    "mfpmc1 %3 ;"		// grab the stop time
	    "mr %2, %%r0 ;"		// transfer the start time
	    : "+r" (start), "+r" (stop), "+r" (start2), "+r" (stop2)
	    : "r" (regs), "r" (count)
	    : "ctr", "r0", "r3"
	    );

    printf( "stmw for 28 registers: %d/%d cycles\n", stop-start, count );
    printf( "lmw for 28 registers: %d/%d cycles\n", stop2-start2, count );

    return CMD_NOQUIT;
}

