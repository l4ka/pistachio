/*********************************************************************
 *
 * Copyright (C) 2002-2004,  Karlsruhe University
 *
 * File path:     pistachio/profiling/kernel/kdb/arch/ia32/x86.cc
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
 * $Id: x86.cc,v 1.104 2006/06/20 06:47:38 stoess Exp $
 *
 ********************************************************************/

#include <debug.h>
#include <kdb/kdb.h>
#include <kdb/input.h>
#include INC_ARCH(cpu.h)
#include INC_ARCH(trapgate.h)
#include INC_ARCH(ioport.h)
#include INC_ARCH(segdesc.h)
/* K8 flush filter support  */
#if defined(CONFIG_CPU_X86_K8)
#include INC_ARCH(amdhwcr.h)
#endif 
#include INC_PLAT(nmi.h)
#include INC_GLUE(idt.h)
#if defined(CONFIG_IOAPIC)
# include INC_ARCH(apic.h)
#endif
#if defined(CONFIG_SMP)
#include INC_GLUE(cpu.h)
#endif


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
	    printf("%2x -> %4x:%x, dpl=%d, %s (%x:%x)\n", i,
		   e.x.d.sel,
		   e.x.d.offset_low | (e.x.d.offset_high << 16),
		   e.x.d.dpl,
		   ((const char*[]){0,0,0,0,0,0,"INT ","TRAP"})[e.x.d.type],
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
    for (int i = 0; i < GDT_SIZE; i++)
    {
	x86_segdesc_t *ent = gdt+i;
	printf("GDT[%d] = %p:%p", i, ent->x.raw[0], ent->x.raw[1]);
	if ( (ent->x.raw[0] == 0 && ent->x.raw[1] == 0) ||
	     (! ent->x.d.s) )
	{
	    printf("\n");
	    continue;
	}
	printf(" <%p,%p> ",
		ent->x.d.base_low + (ent->x.d.base_high << 24),
		ent->x.d.base_low + (ent->x.d.base_high << 24) +
		(ent->x.d.g ? 0xfff |
		    (ent->x.d.limit_low + (ent->x.d.limit_high << 16)) << 12 :
		    (ent->x.d.limit_low + (ent->x.d.limit_high << 16))));
	printf("dpl=%d %d-bit ", ent->x.d.dpl, ent->x.d.d ? 32 : 16);
	if ( ent->x.d.type & 0x8 )
	    printf("code %cC %cR ",
		    ent->x.d.type & 0x4 ? ' ' : '!',
		    ent->x.d.type & 0x2 ? ' ' : '!');
	else
	    printf("data E%c R%c ",
		    ent->x.d.type & 0x4 ? 'D' : 'U',
		    ent->x.d.type & 0x2 ? 'W' : 'O');
	printf("%cP %cA\n",
		ent->x.d.p ? ' ' : '!',
		ent->x.d.type & 0x1 ? ' ' : '!');
    }

    return CMD_NOQUIT;
}

/**
 * cmd_cpu - dump CPU feature description
 */
DECLARE_CMD (cmd_cpu, arch, 'C', "cpu", "dump CPU features");

CMD(cmd_cpu, cg)
{
    /* see: Intel IA32, CPUID instruction */
    const char* features[] = {
	"fpu",  "vme",    "de",   "pse",   "tsc",  "msr", "pae",  "mce",
	"cx8",  "apic",   "?",    "sep",   "mtrr", "pge", "mca",  "cmov",
	"pat",  "pse-36", "psn",  "cflsh", "?",    "ds",  "acpi", "mmx",
	"fxsr", "sse",    "sse2", "ss",    "ht",   "tm",  "ia64", "?" };

    /* from above document, table 7, page 17 */
    const char* cachecfg[16][16] =
    {
	{ /* 0x00 */
	    "",
	    "ITLB: 32*4K, 4w", "ITLB: 2*4M",
	    "DTLB: 64*4K, 4w", "DTLB: 8*4M, 4w", 0,
	    "ICache: 8K, 4w, 32", 0, "ICache: 16K, 4w, 32", 0,
	    "DCache: 8K, 2w, 32", 0, "DCache: 16K, 4w, 32" },
	{ /* 0x10 */ },
	{ /* 0x20 */
	    0, 0,
	    "3rd level: 512K, 4w, 64",
	    "3rd level: 1M, 8w, 64",
	},
	{ /* 0x30 */ },
	{ /* 0x40 */
	    "no L2 or L3",
	    "Cache: 128K, 4w, 32", "Cache: 256K, 4w, 32",
	    "Cache: 512K, 4w, 32", "Cache: 1M, 4w, 32",
	    "Cache: 2M, 4w, 32",
	},
	{ /* 0x50 */
	    "ITLB: 64*{4K,2M/4M}", "ITLB: 128*{4K,2M/4M}",
	    "ITLB: 256*{4K,2M/4M}", 0, 0, 0, 0, 0, 0, 0, 0,
	    "DTLB: 64*{4K,4M}", "DTLB: 128*{4K,4M}",
	    "DTLB: 256*{4K,4M}"
	},
	{ /* 0x60 */
	    0, 0, 0, 0, 0, 0,
	    "DCache: 8K, 4w, 64", "DCache: 16K, 4w, 64",
	    "DCache: 32K, 4w, 64"
	},
	{ /* 0x70 */
	    "TC: 12Kuop, 8w", "TC: 16Kuop, 8w", "TC: 32Kuop, 8w", 0,
	    0, 0, 0, 0, 0,
	    "Cache: 128K, 8w, 64", "Cache: 256K, 8w, 64",
	    "Cache: 512K, 8w, 64", "Cache: 1M, 8w, 64",
	},
	{ /* 0x80 */
	    0, 0,
	    "Cache: 256K, 8w, 32", "Cache: 512K, 8w, 32",
	    "Cache: 1M, 8w, 32", "Cache: 2M, 8w, 32",
	}
    };

    word_t id[4][4];
    word_t i;
    for (i = 0; i < 4; i++)
	x86_cpuid(i, &id[i][0], &id[i][1], &id[i][2], &id[i][3]);
    for (i = 0; i <= id[0][0]; i++)
	printf("cpuid(%d):%x:%x:%x:%x\n", i,
	       id[i][0], id[i][1], id[i][2], id[i][3]);
    printf("0: max=%d \"", id[0][0]);
    for (i = 0; i < 12; i++) printf("%c", (((char*) &id[0][1])[i ^ ((i >= 4) * 12)]));
    printf("\"\n1: fam=%d, mod=%d, step=%d\n1: ",
	   (id[1][0] >> 8) & 0xF,
	   (id[1][0] >> 4) & 0xF,
	   (id[1][0] >> 0) & 0xF);
    for (i = 0; i < 32; i++)
	if ((id[1][3] >> i) & 1) printf("%s ", features[i]);
    printf("\n");
    /* 2: eax[7:0] determines, how often 2 must be called - noimp */
    for (i = 1; i < 16; i++)
    {
	word_t j = ((unsigned char*)id[2])[i];
	if (((id[2][i/4] & 0x80000000U) == 0) && (j != 0))
	    printf("[%2x] %s\n", j, cachecfg[0][j]);
    }

    return CMD_NOQUIT;
}

#if defined(CONFIG_CPU_X86_K8)
/**
 * cmd_hcwr - dump HWCR register contents
 */
DECLARE_CMD (cmd_amdhwcr, arch, 'h', "hwcr", "dump AMD's HWCR contents");

CMD(cmd_amdhwcr, cg)
{
    x86_amdhwcr_t::dump_hwcr();
    return CMD_NOQUIT;
}
#endif
