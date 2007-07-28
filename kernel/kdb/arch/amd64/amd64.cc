/*********************************************************************
 *                
 * Copyright (C) 2002-2007,  Karlsruhe University
 *                
 * File path:     kdb/arch/amd64/amd64.cc
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
#include INC_ARCHX(x86,amdhwcr.h)
#include INC_ARCH(trapgate.h)
#include INC_ARCH(cpuid.h)
#include INC_ARCHX(x86,ioport.h)
#include INC_ARCH(segdesc.h)
#include INC_PLAT(nmi.h)
#include INC_GLUE(idt.h)


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
	    "show AMD64 control registers");

CMD(cmd_show_ctrlregs, cg)
{
    u64_t cr0, cr2, cr3, cr4, cr8;
    __asm__ __volatile__ (
 	"mov    %%cr0, %0	\n"
	"mov	%%cr2, %1	\n"
	"mov 	%%cr3, %2	\n"
	"mov	%%cr4, %3	\n"
 	"mov    %%cr8, %0	\n"
	: "=q"(cr0), "=q"(cr2), "=q"(cr3), "=q"(cr4), "=q"(cr8));

    printf("CR0: %16x\n", cr0);
    printf("CR2: %16x\n", cr2);
    printf("CR3: %16x\n", cr3);
    printf("CR4: %16x\n", cr4);
    printf("CR8: %16x\n", cr4);
    return CMD_NOQUIT;
}

#if !defined(CONFIG_PLAT_SIMICS)
DECLARE_CMD (cmd_dump_msrs, arch, 'm', "dumpmsrs",
	     "dump model specific registers");

CMD (cmd_dump_msrs, cg)
{
	printf("LASTBRANCH_FROM_IP: %x\n", x86_rdmsr (AMD64_LASTBRANCHFROMIP));
	printf("LASTBRANCH_TO_IP:   %x\n", x86_rdmsr (AMD64_LASTBRANCHTOIP));
	printf("LASTINT_FROM_IP:    %x\n", x86_rdmsr (AMD64_LASTINTFROMIP));
	printf("LASTINT_TO_IP:      %x\n", x86_rdmsr (AMD64_LASTINTTOIP));
    return CMD_NOQUIT;
}
#endif

DECLARE_CMD(cmd_show_frame, root, ' ', "frame",
	    "show exception frame of thread");

static void SECTION(SEC_KDEBUG) dump_rflags(const u64_t rflags)
{
    printf("%c%c%c%c%c%c%c%c%c%c%c",
	   rflags & (1 <<  0) ? 'C' : 'c',
	   rflags & (1 <<  2) ? 'P' : 'p',
	   rflags & (1 <<  4) ? 'A' : 'a',
	   rflags & (1 <<  6) ? 'Z' : 'z',
	   rflags & (1 <<  7) ? 'S' : 's',
	   rflags & (1 << 11) ? 'O' : 'o',
	   rflags & (1 << 10) ? 'D' : 'd',
	   rflags & (1 <<  9) ? 'I' : 'i',
	   rflags & (1 <<  8) ? 'T' : 't',
	   rflags & (1 << 16) ? 'R' : 'r',
	   ((rflags >> 12) & 3) + '0'
	);
}

CMD(cmd_show_frame, cg)
{
    amd64_exceptionframe_t * frame = (amd64_exceptionframe_t*)kdb.kdb_param;
    printf("fault addr: %16x\tstack: %16x\terror code: %x frame: %p\n", 
	   frame->rip, frame->rsp, frame->error, frame);

    printf("rax: %16x\t r8: %16x\n", frame->rax, frame->r8);
    printf("rcx: %16x\t r9: %16x\n", frame->rcx, frame->r9);
    printf("rdx: %16x\tr10: %16x\n", frame->rdx, frame->r10);
    printf("rbx: %16x\tr11: %16x\n", frame->rbx, frame->r11);
    printf("rsp: %16x\tr12: %16x\n", frame->rsp, frame->r12);
    printf("rbp: %16x\tr13: %16x\n", frame->rbp, frame->r13);
    printf("rsi: %16x\tr14: %16x\n", frame->rsi, frame->r14);
    printf("rdi: %16x\tr15: %16x\n", frame->rdi, frame->r15);
    printf("rfl: %16x[", frame->rflags);
    dump_rflags(frame->rflags);printf("]\n");
    return CMD_NOQUIT;
}

/**
 * cmd_ports - read or write AMD64's I/O space
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
    for (word_t i = 0; i < sizeof(idt)/sizeof(amd64_idtdesc_t); i++)
    {
	amd64_idtdesc_t e = idt.get_descriptor(i);
	if (e.x.d.p)
	    printf("%2x -> %4x:%x, dpl=%d, %s (%16x:%16x)\n", i,
		   e.x.d.selector,
		   e.x.d.offset_low | (e.x.d.offset_high << 16),
		   e.x.d.dpl,
		   ((char*[]){0,0,0,0,0,0,0,0,0,0,0,0,0,0,"INT ","TRAP"})[e.x.d.type],
		   e.x.raw[0], e.x.raw[1]);
    };
    return CMD_NOQUIT;
}

/**
 * control NMI handling
 */
DECLARE_CMD (cmd_nmi, arch, 'n', "nmi", "control nmi handling");

CMD(cmd_nmi, cg)
{
    nmi_t nmi;
    switch (get_choice("NMI", "Enable/Disable", 'd')) {
    case 'd': nmi.mask(); break;
    case 'e': nmi.unmask(); break;
    }
    return CMD_NOQUIT;
}

/**
 * cmd_gdt - dump global segment descriptor table (GDT)
 */
DECLARE_CMD (cmd_gdt, arch, 'g', "gdt", "dump the GDT");

    

CMD(cmd_gdt, cg)
{
    extern amd64_segdesc_t gdt[];
    
    printf("\nGDT-dump: gdt at %x\n", gdt);
    for (int i = 0; i < GDT_SIZE - 2 ; i++)
    {
	
        amd64_segdesc_t *ent = gdt+i;
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
        else if ( ent->x.d.type == amd64_segdesc_t::inv)
            printf("inv        ");
	else
            printf("data E%c R%c ",
                    ent->x.d.type & 0x4 ? 'D' : 'U',
                    ent->x.d.type & 0x2 ? 'W' : 'O');
        printf("%cP %cA\n",
                ent->x.d.p ? ' ' : '!',
                ent->x.d.type & 0x1 ? ' ' : '!');
    }

    amd64_tssdesc_t *tss = (amd64_tssdesc_t *) &gdt[GDT_SIZE - 2];
    
    printf("GDT[%d] = %16x", GDT_SIZE-2, tss->x.raw);
    printf(" <%16x,%16x> ",
	   tss->x.d.base_low + (tss->x.d.base_med << 24) + (tss->x.d.base_high << 32),
	   tss->x.d.base_low + (tss->x.d.base_med << 24) + (tss->x.d.base_high << 32) + 
	   (tss->x.d.g ? 0xfff |
	    (tss->x.d.limit_low + (tss->x.d.limit_high << 16)) << 12 :
	    (tss->x.d.limit_low + (tss->x.d.limit_high << 16))));

    printf("dpl=%d 64-bit ", tss->x.d.dpl);
    printf("tss\n");

    printf("FS_MSR = %16x\nGS_MSR = %16x\n", x86_rdmsr(AMD64_FS_MSR), x86_rdmsr(AMD64_GS_MSR));

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
