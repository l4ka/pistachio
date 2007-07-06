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
#include INC_ARCH(sysdesc.h)
#include INC_ARCH(segdesc.h)
/* K8 flush filter support  */
#if defined(CONFIG_CPU_IA32_K8)
#include INC_ARCH(hwcr_k8.h)
#endif 
#include INC_PLAT(nmi.h)
#include INC_GLUE(idt.h)
#if defined(CONFIG_IOAPIC)
# include INC_ARCH(apic.h)
#endif

DECLARE_CMD (cmd_reset, root, '6', "reset", "Reset system");

CMD(cmd_reset, cg)
{
    asm volatile (
	"	movb	$0xFE, %al	\n"
	"	outb	%al, $0x64	\n");

    /* NOTREACHED */
    return CMD_NOQUIT;
}

DECLARE_CMD(cmd_show_ctrlregs, arch, 'c', "ctrlregs",
	    "show IA32 control registers");

CMD(cmd_show_ctrlregs, cg)
{
    u32_t cr0, cr2, cr3, cr4;
    __asm__ __volatile__ (
	"movl	%%cr0, %0	\n"
	"movl	%%cr2, %1	\n"
	"movl	%%cr3, %2	\n"
	"movl	%%cr4, %3	\n"
	: "=r"(cr0), "=r"(cr2), "=r"(cr3), "=r"(cr4));
    printf("CR0: %8x\n", cr0);
    printf("CR2: %8x\n", cr2);
    printf("CR3: %8x\n", cr3);
    printf("CR4: %8x\n", cr4);
    return CMD_NOQUIT;
}

DECLARE_CMD (cmd_dump_msrs, arch, 'm', "dumpmsrs",
	     "dump model specific registers");

CMD (cmd_dump_msrs, cg)
{
#if defined(CONFIG_CPU_IA32_I686)
	printf("LASTBRANCH_FROM_IP: %x\n", ia32_rdmsr (IA32_LASTBRANCHFROMIP));
	printf("LASTBRANCH_TO_IP:   %x\n", ia32_rdmsr (IA32_LASTBRANCHTOIP));
	printf("LASTINT_FROM_IP:    %x\n", ia32_rdmsr (IA32_LASTINTFROMIP));
	printf("LASTINT_TO_IP:      %x\n", ia32_rdmsr (IA32_LASTINTTOIP));
#endif

#if defined(CONFIG_CPU_IA32_P4)
	for (int i = 0; i < 18; i++) {
	    u64_t pmc = ia32_rdmsr (IA32_COUNTER_BASE + i);
	    u64_t cccr = ia32_rdmsr (IA32_CCCR_BASE + i);
	    printf("PMC/CCCR %02u: 0x%08x%08x/0x%08x%08x\n",
		   i,
		   (u32_t)(pmc >> 32), (u32_t)pmc,
		   (u32_t)(cccr >> 32), (u32_t)cccr);
	}
#endif

    return CMD_NOQUIT;
}

static void SECTION(SEC_KDEBUG) dump_eflags(const u32_t eflags)
{
    printf("%c%c%c%c%c%c%c%c%c%c%c",
	   eflags & (1 <<  0) ? 'C' : 'c',
	   eflags & (1 <<  2) ? 'P' : 'p',
	   eflags & (1 <<  4) ? 'A' : 'a',
	   eflags & (1 <<  6) ? 'Z' : 'z',
	   eflags & (1 <<  7) ? 'S' : 's',
	   eflags & (1 << 11) ? 'O' : 'o',
	   eflags & (1 << 10) ? 'D' : 'd',
	   eflags & (1 <<  9) ? 'I' : 'i',
	   eflags & (1 <<  8) ? 'T' : 't',
	   eflags & (1 << 16) ? 'R' : 'r',
	   ((eflags >> 12) & 3) + '0'
	);
}

void ia32_dump_frame (ia32_exceptionframe_t * frame)
{
   printf("fault addr: %8x\tstack: %8x\terror code: %x frame: %p\n",
	   frame->eip, frame->esp, frame->error, frame);

    printf("eax: %8x\tebx: %8x\n", frame->eax, frame->ebx);
    printf("ecx: %8x\tedx: %8x\n", frame->ecx, frame->edx);
    printf("esi: %8x\tedi: %8x\n", frame->esi, frame->edi);
    printf("ebp: %8x\tefl: %8x [", frame->ebp, frame->eflags);
    dump_eflags(frame->eflags);printf("]\n");
    printf("cs:      %4x\tss:      %4x\n",
	   frame->cs & 0xffff, frame->ss & 0xffff);
    printf("ds:      %4x\tes:      %4x\n",
	   frame->ds & 0xffff, frame->es & 0xffff);
}

DECLARE_CMD (cmd_dump_current_frame, root, ' ', "frame",
	     "show current exception frame");

CMD (cmd_dump_current_frame, cg)
{
    ia32_dump_frame ((ia32_exceptionframe_t *) kdb.kdb_param);
    return CMD_NOQUIT;
}


/**
 * cmd_ports - read or write IA32's I/O space
 */
DECLARE_CMD (cmd_ports, arch, 'p', "ports", "IO port access");

CMD(cmd_ports, cg)
{
    char dir  = get_choice ("Access mode", "In/Out", 'i');
    char width = get_choice ("Access width", "Byte/Word/Dword", 'b');
    u16_t port = get_hex ("Port", 0x80, NULL);

    u32_t val = 0;

    switch (dir) {
    case 'i':
	switch (width) {
	case 'b': val = in_u8(port); break;
	case 'w': val = in_u16(port); break;
	case 'd': val = in_u32(port); break;
	};
	printf("Value = %x\n", val);
	break;
    case 'o':
	val = get_hex ("Value", 0, NULL);
	switch (width) {
	case 'b': out_u8(port, val); break;
	case 'w': out_u16(port, val); break;
	case 'd': out_u32(port, val); break;
	}; break;
    };
    return CMD_NOQUIT;
}

/**
 * cmd_idt - dump the IDT
 */
DECLARE_CMD (cmd_idt, arch, 'i', "idt", "dump the IDT");

CMD(cmd_idt, cg)
{
    printf("\nIDT-dump: idt at %x\n", &idt);
    for (word_t i = 0; i < sizeof(idt)/sizeof(ia32_idtdesc_t); i++)
    {
	ia32_idtdesc_t e = idt.get_descriptor(i);
	if (e.x.d.p)
	    printf("%2x -> %4x:%x, dpl=%d, %s (%x:%x)\n", i,
		   e.x.d.sel,
		   e.x.d.offset_low | (e.x.d.offset_high << 16),
		   e.x.d.dpl,
		   ((char*[]){0,0,0,0,0,0,"INT ","TRAP"})[e.x.d.type],
		   e.x.raw[0], e.x.raw[1]);
    };
    return CMD_NOQUIT;
}

/**
 * enable/disable NMI handling
 */
DECLARE_CMD (cmd_enable_nmi, arch, 'n', "enable_nmi", "enable/disable NMI in chipset");

CMD(cmd_enable_nmi, cg)  
{  
    nmi_t nmi;  
    switch (get_choice("NMI", "Enable/Disable", 'd')) 
    {
    case 'd': 
	nmi.mask(); 
	break;      
    case 'e': 
	nmi.unmask(); 
	break;    
    }
    return CMD_NOQUIT; 
} 

#if defined(CONFIG_SMP)

/**
 * send NMI via IPI to (remote) processors
 */
DECLARE_CMD (cmd_send_nmi, arch, 'N', "send_nmi", "send nmi via IPI");

CMD(cmd_send_nmi, cg)
{
#if defined(CONFIG_IOAPIC)
    word_t apic = get_dec("send NMI to APIC id", 0, NULL);
    local_apic_t<APIC_MAPPINGS> local_apic;
    local_apic.send_nmi(apic);
#endif
    return CMD_NOQUIT;
}
#endif


/**
 * cmd_gdt - dump global segment descriptor table (GDT)
 */
DECLARE_CMD (cmd_gdt, arch, 'g', "gdt", "dump the GDT");

CMD(cmd_gdt, cg)
{
    extern ia32_segdesc_t gdt[];

    printf("\nGDT-dump: gdt at %x\n", gdt);
    for (int i = 0; i < GDT_SIZE; i++)
    {
	ia32_segdesc_t *ent = gdt+i;
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
	ia32_cpuid(i, &id[i][0], &id[i][1], &id[i][2], &id[i][3]);
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

#if defined(CONFIG_IOAPIC)
DECLARE_CMD(cmd_show_lvt, arch, 'l', "lvt",
	    "show APIC local vector table");

CMD(cmd_show_lvt, cg)
{
    local_apic_t<APIC_MAPPINGS> local_apic;

    printf("  timer:   0x%8x\n", local_apic.read_vector (local_apic_t<APIC_MAPPINGS>::lvt_timer));
    printf("  lin0:    0x%8x\n", local_apic.read_vector (local_apic_t<APIC_MAPPINGS>::lvt_lint0));
    printf("  lin1:    0x%8x\n", local_apic.read_vector (local_apic_t<APIC_MAPPINGS>::lvt_lint1));
    printf("  error:   0x%8x\n", local_apic.read_vector (local_apic_t<APIC_MAPPINGS>::lvt_error));
    printf("  perf:    0x%8x\n", local_apic.read_vector (local_apic_t<APIC_MAPPINGS>::lvt_perfcount));
    printf("  thermal: 0x%8x\n", local_apic.read_vector (local_apic_t<APIC_MAPPINGS>::lvt_thermal_monitor));

    return CMD_NOQUIT;
}
#endif

#if defined(CONFIG_CPU_IA32_K8)
/**
 * cmd_hcwr - dump HWCR register contents
 */
DECLARE_CMD (cmd_hwcr, arch, 'h', "hwcr", "dump HWCR contents");

CMD(cmd_hwcr, cg)
{
    ia32_hwcr_t::dump_hwcr();
    return CMD_NOQUIT;
}
#endif
