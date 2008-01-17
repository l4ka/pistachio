/*********************************************************************
 *                
 * Copyright (C) 2002-2008,  Karlsruhe University
 *                
 * File path:     kdb/arch/x86/x64/x86.cc
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
 * $Id: amd64.cc,v 1.5 2006/06/13 14:42:04 stoess Exp $
 *                
 ********************************************************************/

#include <debug.h>
#include <kdb/kdb.h>
#include <kdb/input.h>
#include <linear_ptab.h>
#include INC_ARCH(cpu.h)
#include INC_ARCH(pgent.h)
#include INC_ARCH(amdhwcr.h)
#include INC_ARCH(cpu.h)
#include INC_ARCH(ioport.h)
#include INC_ARCH(segdesc.h)
#include INC_ARCH(trapgate.h)
#include INC_PLAT(nmi.h)
#include INC_GLUE(idt.h)

/**
 * cmd_idt - dump the IDT
 */
DECLARE_CMD (cmd_idt, arch, 'i', "idt", "dump the IDT");

CMD(cmd_idt, cg)
{
    printf("\nIDT-dump: idt at %x\n", &idt);
    for (word_t i = 0; i < sizeof(idt)/sizeof(x86_idtdesc_t); i++)
    {
	x86_idtdesc_t e = idt.get_descriptor(i);
	if (e.x.d.p)
	    printf("%2x -> %4x:%x, dpl=%d, %s (%16x:%16x)\n", i,
		   e.x.d.selector,
		   e.x.d.offset_low | (e.x.d.offset_high << 16),
		   e.x.d.dpl,
		   ((const char*[]){0,0,0,0,0,0,0,0,0,0,0,0,0,0,"INT ","TRAP"})[e.x.d.type],
		   e.x.raw[0], e.x.raw[1]);
    };
    return CMD_NOQUIT;
}

/**
 * cmd_gdt - dump global segment descriptor table (GDT)
 */
DECLARE_CMD (cmd_gdt, arch, 'g', "gdt", "dump the GDT");

    

CMD(cmd_gdt, cg)
{
    extern x86_segdesc_t gdt[];
    
    printf("\nGDT-dump: gdt at %x\n", gdt);
    for (int i = 0; i < GDT_SIZE - 2 ; i++)
    {
	
        x86_segdesc_t *ent = gdt+i;
        printf("GDT[%d] = %16x", i, ent->x.raw);
        if (ent->x.raw == 0 || (! ent->x.d.s) )
        {
            printf("\n");
            continue;
        }
	
	printf(" <%16x,%16x> ",
	       ent->x.d.base_low + (ent->x.d.base_high << 24),
	       ent->x.d.base_low + (ent->x.d.base_high << 24) +
	       (ent->x.d.g ? 0xfff |
		(ent->x.d.limit_low + (ent->x.d.limit_high << 16)) << 12 :
		(ent->x.d.limit_low + (ent->x.d.limit_high << 16))));
	
	printf("dpl=%d %d-bit ", ent->x.d.dpl, ent->x.d.l ? 64 : (ent->x.d.d ? 32 : 16));
	    
        if ( ent->x.d.type & 0x8 )
            printf("code %cC %cR ",
                    ent->x.d.type & 0x4 ? ' ' : '!',
                    ent->x.d.type & 0x2 ? ' ' : '!');
        else if ( ent->x.d.type == x86_segdesc_t::inv)
            printf("inv        ");
	else
            printf("data E%c R%c ",
                    ent->x.d.type & 0x4 ? 'D' : 'U',
                    ent->x.d.type & 0x2 ? 'W' : 'O');
        printf("%cP %cA\n",
                ent->x.d.p ? ' ' : '!',
                ent->x.d.type & 0x1 ? ' ' : '!');
    }

    x86_tssdesc_t *tss = (x86_tssdesc_t *) (addr_t) &gdt[GDT_SIZE - 2];
    
    printf("GDT[%d] = %16x", GDT_SIZE-2, tss->x.raw);
    printf(" <%16x,%16x> ",
	   tss->x.d.base_low + (tss->x.d.base_med << 24) + (tss->x.d.base_high << 32),
	   tss->x.d.base_low + (tss->x.d.base_med << 24) + (tss->x.d.base_high << 32) + 
	   (tss->x.d.g ? 0xfff |
	    (tss->x.d.limit_low + (tss->x.d.limit_high << 16)) << 12 :
	    (tss->x.d.limit_low + (tss->x.d.limit_high << 16))));

    printf("dpl=%d 64-bit ", tss->x.d.dpl);
    printf("tss\n");

    printf("FS_MSR = %16x\nGS_MSR = %16x\n", x86_rdmsr(X86_X64_MSR_FS), x86_rdmsr(X86_X64_MSR_GS));

    return CMD_NOQUIT;
}


/**
 * cmd_cpu - dump CPU feature description
 */
DECLARE_CMD (cmd_cpu, arch, 'C', "cpu", "dump CPU features");

CMD(cmd_cpu, cg)
{
    boot_cpu_ft.dump_features();
    return CMD_NOQUIT;
}

/**
 * cmd_hcwr - dump HWCR register contents
 */
DECLARE_CMD (cmd_amdhwcr, arch, 'h', "hwcr", "dump AMD's HWCR contents");

CMD(cmd_amdhwcr, cg)
{
    x86_amdhwcr_t::dump_hwcr();
    return CMD_NOQUIT;
}



/**
 * cmd_pgtcalc - calculate PGT indices
 */
DECLARE_CMD (cmd_pgtcalc, arch, 'P', "pgtcalc", "calculate page table indices");

CMD(cmd_pgtcalc, cg)
{
    addr_t addr = (addr_t) get_hex ("Virtual address", NULL);
    printf("%p -> %d, %d, %d, %d\n",
	   addr,
	   page_table_index(pgent_t::size_512g, addr),
	   page_table_index(pgent_t::size_1g, addr),
	   page_table_index(pgent_t::size_2m, addr),
	   page_table_index(pgent_t::size_4k, addr));
    
	   
    return CMD_NOQUIT;
}
