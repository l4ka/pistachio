/*********************************************************************
 *                
 * Copyright (C) 2002-2003,  University of New South Wales
 *                
 * File path:     glue/v4-mips64/space.cc
 * Description:   VAS implementation 
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
 * $Id: tlb.cc,v 1.9 2003/11/17 05:30:19 cvansch Exp $
 *                
 ********************************************************************/

#include <debug.h>			/* for UNIMPLEMENTED	*/
#include INC_ARCH(tlb.h)
#include INC_PLAT(config.h)
#include INC_ARCH(mipsregs.h)
#include INC_GLUE(context.h)


void SECTION (".init") init_tlb(void)
{
    word_t i;
    TRACEF("Initialize TLB\n");

    __asm__ __volatile__ (
	"mtc0 %0,"STR(CP0_PAGEMASK)"\n\t"
	"mtc0 $0,"STR(CP0_WIRED)"\n\t"
	:
	: "r" (CONFIG_MIPS64_PAGEMASK_4K)
    );


    for (i=0; i<CONFIG_MIPS64_TLB_SIZE; i++)
	__asm__ __volatile__ (
	    "mtc0 %0,"STR(CP0_INDEX)"\n\t"
	    "dmtc0 %0,"STR(CP0_ENTRYHI)"\n\t"
	    "dmtc0 $0,"STR(CP0_ENTRYLO0)"\n\t"
	    "dmtc0 $0,"STR(CP0_ENTRYLO1)"\n\t"
	    "nop;nop;nop;\n\t"
	    "tlbwi\n\t"
	    :
	    : "r" (i)
	);

    __asm__ __volatile__ (
	"nop;nop;nop;\n\t"
	"mtc0 $0,"STR(CP0_ENTRYHI)"\n\t"
    );
}

void mips64_dump_tlb()
{
    word_t i, save;
    printf("-------------------------TLB DUMP-----------------------------\n");

    __asm__ __volatile__ (
	"dmfc0	%0, "STR(CP0_ENTRYHI)"\n\t"
	: "=r" (save)
    );

    for (i=0; i<CONFIG_MIPS64_TLB_SIZE; i++)
    {
	word_t hi, lo0, lo1;
	__asm__ __volatile__ (
	    "mtc0 %3,"STR(CP0_INDEX)"\n\t"
	    "nop;nop;nop;tlbr;nop;nop;nop;nop\n\t"
	    "dmfc0 %0,"STR(CP0_ENTRYHI)"\n\t"
	    "dmfc0 %1,"STR(CP0_ENTRYLO0)"\n\t"
	    "dmfc0 %2,"STR(CP0_ENTRYLO1)"\n\t"
	    : "=r" (hi), "=r" (lo0), "=r" (lo1)
	    : "r" (i)
	);
	printf("%2d: [%p] %08lx / %08lx\n", i, hi, lo0, lo1);
    }

    __asm__ __volatile__ (
	"dmtc0	%0, "STR(CP0_ENTRYHI)"\n\t"
	: : "r" (save)
    );

    printf("-------------------------TLB DUMP-----------------------------\n");
}

extern "C" void mips64_dump_memory(mips64_irq_context_t *context)
{
    unsigned int i;

    mips64_dump_tlb();

    printf("-------------------------EPC DUMP-----------------------------\n");
    printf("EPC = %16lx\n", context->epc);
    for (i=0; i<32; i+= 4)
    {
	printf("%c [%16lx] = 0x%08lx\n",i==24? '*' : ' ', context->epc-24+i, *(unsigned int*)(context->epc-24+i));
    }
}
