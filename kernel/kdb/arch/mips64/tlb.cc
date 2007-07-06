/*********************************************************************
 *                
 * Copyright (C) 2002,  University of New South Wales
 *                
 * File path:     kdb/arch/mips64/tlb.cc
 * Description:   TLB management commands
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
 * $Id: tlb.cc,v 1.5 2003/11/17 05:42:14 cvansch Exp $
 *                
 ********************************************************************/
#include <debug.h>
#include <kdb/cmd.h>
#include <kdb/kdb.h>
#include INC_ARCH(tlb.h)
#include INC_PLAT(config.h)
#include INC_ARCH(mipsregs.h)
#include INC_GLUE(context.h)


DECLARE_CMD_GROUP (mips64_tlb);


/**
 * cmd_mips64_tlb: Mips64 TLB management.
 */
DECLARE_CMD (cmd_mips64_tlb, arch, 't', "tlb", "TLB management");

CMD(cmd_mips64_tlb, cg)
{
    return mips64_tlb.interact (cg, "tlb");
}

/**
 * cmd_mips64_dum: dump Mips64 TLB
 */
DECLARE_CMD (cmd_mips64_dum, mips64_tlb, 'd', "dump", "dump hardware TLB");

CMD(cmd_mips64_dum, cg)
{
    word_t i, save;
    static const char * pagemask[] = {
	"  4k", "  8k", " 16k", " 32k",
	" 64k", "128k", "256k", "  1M",
	"  2M", "  4M", "  8M", " 16M",
	" 32M", " 64M", "128M", "256M",
    };
    static const char * cache[] = {
	"Wthr ", "WthrA", "Off  ", "Wb   ",
	"CoEx ", "CoCOW", "CoCUW", "Accel"
    };

    __asm__ __volatile__ (
	"dmfc0   %0, "STR(CP0_ENTRYHI)"\n\t"
	: "=r" (save)
    );

    printf ("Index           EntryHi  EntryLo0 (cache v d)  EntryLo1 (cache v d) | Size  ASID  Global\n");

    for (i=0; i<CONFIG_MIPS64_TLB_SIZE; i++)
    {
	word_t hi, lo0, lo1, mask, size;
	__asm__ __volatile__ (
	    "mtc0 %4,"STR(CP0_INDEX)"\n\t"
	    "nop;nop;nop;tlbr;nop;nop;nop;nop\n\t"
	    "dmfc0 %0,"STR(CP0_ENTRYHI)"\n\t"
	    "dmfc0 %1,"STR(CP0_ENTRYLO0)"\n\t"
	    "dmfc0 %2,"STR(CP0_ENTRYLO1)"\n\t" 
	    "mfc0 %3,"STR(CP0_PAGEMASK)"\n\t" 
	    : "=r" (hi), "=r" (lo0), "=r" (lo1), "=r" (mask)
	    : "r" (i)
	);
	size = 0;
	mask >>= 13;
	while (mask&1)
	{
	    mask >>= 1;
	    size ++;
	}
	printf("%2d:    %p  %8lx (%s %d %d)  %8lx (%s %d %d) | %s   %3d     %s\n",
			i, hi,
			lo0, cache[(lo0&0x38)>>3], (lo0>>1)&1, (lo0>>2)&1,
			lo1, cache[(lo1&0x38)>>3], (lo1>>1)&1, (lo1>>2)&1,
			pagemask[size], hi&0xFF, lo0&1 ? "Yes" : " No");
    }

    __asm__ __volatile__ (
	"dmtc0   %0, "STR(CP0_ENTRYHI)"\n\t"
	: : "r" (save)
    );

    return CMD_NOQUIT;
}


/**
 * cmd_mips64_tran: translate Mips64 TLB
 */
DECLARE_CMD (cmd_mips64_trans, mips64_tlb, 't', "translate", "translate TLB");

CMD(cmd_mips64_trans, cg)
{
    word_t i, save;

    __asm__ __volatile__ (
	"dmfc0   %0, "STR(CP0_ENTRYHI)"\n\t"
	: "=r" (save)
    );

    printf ("Index       (virt)   Even Page  (phys)      |      (virt)   Odd Page   (phys)       ASID\n");

    for (i=0; i<CONFIG_MIPS64_TLB_SIZE; i++)
    {
	word_t hi, lo0, lo1, mask, size;
	__asm__ __volatile__ (
	    "mtc0 %4,"STR(CP0_INDEX)"\n\t"
	    "nop;nop;nop;tlbr;nop;nop;nop;nop\n\t"
	    "dmfc0 %0,"STR(CP0_ENTRYHI)"\n\t"
	    "dmfc0 %1,"STR(CP0_ENTRYLO0)"\n\t"
	    "dmfc0 %2,"STR(CP0_ENTRYLO1)"\n\t" 
	    "dmfc0 %3,"STR(CP0_PAGEMASK)"\n\t" 
	    : "=r" (hi), "=r" (lo0), "=r" (lo1), "=r" (mask)
	    : "r" (i)
	);
	size = 0;
	mask >>= 13;
	while (mask&1)
	{
	    mask >>= 1;
	    size ++;
	}
	printf("%2d:    ", i);

	if (lo0&2)
	    printf("%p -> %p", (hi&(~((1<<13)-1))), (lo0>>6)<<CONFIG_MIPS64_VPN_SHIFT);
	else
	    printf("------------------------------------");

	printf(" | ");

	if (lo1&2)
	    printf("%p -> %p", (hi&(~((1<<13)-1)))+(1<<(size+12)), (lo1>>6)<<CONFIG_MIPS64_VPN_SHIFT);
	else
	    printf("------------------------------------");
	if (lo0&1)
	    printf("   all\n");
	else
	    printf("   %3d\n", hi&0xFF);
    }

    __asm__ __volatile__ (
	"dmtc0   %0, "STR(CP0_ENTRYHI)"\n\t"
	: : "r" (save)
    );

    return CMD_NOQUIT;
}



/**
 * cmd_tlb_info: dump TLB information
 */
DECLARE_CMD (cmd_tlb_info, mips64_tlb, 'i', "info", "dump TLB information");

CMD(cmd_tlb_info, cg)
{
    printf("tlb info\n");
    return CMD_NOQUIT;
}
