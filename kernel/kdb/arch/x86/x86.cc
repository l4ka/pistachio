/*********************************************************************
 *                
 * Copyright (C) 2007,  Karlsruhe University
 *                
 * File path:     kdb/arch/x86/x86.cc
 * Description:   
 *                
 * @LICENSE@
 *                
 * $Id:$
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
	    "show X86 control registers");

CMD(cmd_show_ctrlregs, cg)
{
    word_t cr0, cr2, cr3, cr4;
    __asm__ __volatile__ (
	"mov	%%cr0, %0	\n"
	"mov	%%cr2, %1	\n"
	"mov	%%cr3, %2	\n"
	"mov	%%cr4, %3	\n"
	: "=r"(cr0), "=r"(cr2), "=r"(cr3), "=r"(cr4));
    
    printf("CR0: %wx\n", cr0);
    printf("CR2: %wx\n", cr2);
    printf("CR3: %wx\n", cr3);
    printf("CR4: %wx\n", cr4);
    
    return CMD_NOQUIT;
}

DECLARE_CMD (cmd_dump_msrs, arch, 'm', "dumpmsrs",
	     "dump model specific registers");

CMD (cmd_dump_msrs, cg)
{
#if defined(CONFIG_CPU_X86_I686)
	printf("LASTBRANCH_FROM_IP: %x\n", x86_rdmsr (X86_LASTBRANCHFROMIP_MSR));
	printf("LASTBRANCH_TO_IP:   %x\n", x86_rdmsr (X86_LASTBRANCHTOIP_MSR));
	printf("LASTINT_FROM_IP:    %x\n", x86_rdmsr (X86_LASTINTFROMIP_MSR));
	printf("LASTINT_TO_IP:      %x\n", x86_rdmsr (X86_LASTINTTOIP_MSR));
#endif

#if defined(CONFIG_CPU_X86_P4)
	for (int i = 0; i < 18; i++) {
	    u64_t pmc = x86_rdmsr (X86_COUNTER_BASE_MSR + i);
	    u64_t cccr = x86_rdmsr (X86_CCCR_BASE_MSR + i);
	    printf("PMC/CCCR %02u: 0x%08x%08x/0x%08x%08x\n",
		   i,
		   (u32_t)(pmc >> 32), (u32_t)pmc,
		   (u32_t)(cccr >> 32), (u32_t)cccr);
	}
#endif

    return CMD_NOQUIT;
}


DECLARE_CMD (cmd_dump_current_frame, root, ' ', "frame",
	     "show current exception frame");

CMD (cmd_dump_current_frame, cg)
{ 
    debug_param_t * param = (debug_param_t*)kdb.kdb_param;
    param->frame->dump();
    return CMD_NOQUIT;
}


/**
 * cmd_ports - read or write X86's I/O space
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
    word_t cpuid = get_dec("CPU id", 0, NULL);
    cpu_t* cpu = cpu_t::get(cpuid);
    local_apic_t<APIC_MAPPINGS_START> local_apic;
    // don't nmi ourselfs
    if (cpu->get_apic_id() == local_apic.id())
	return CMD_NOQUIT;
    local_apic.send_nmi(cpu->get_apic_id());
    return CMD_QUIT;
}
#endif

#if defined(CONFIG_IOAPIC)
DECLARE_CMD(cmd_show_lvt, arch, 'l', "lvt",
	    "show APIC local vector table");

CMD(cmd_show_lvt, cg)
{
    local_apic_t<APIC_MAPPINGS_START> local_apic;

    printf("  timer:   0x%8x\n", local_apic.read_vector (local_apic_t<APIC_MAPPINGS_START>::lvt_timer));
    printf("  lin0:    0x%8x\n", local_apic.read_vector (local_apic_t<APIC_MAPPINGS_START>::lvt_lint0));
    printf("  lin1:    0x%8x\n", local_apic.read_vector (local_apic_t<APIC_MAPPINGS_START>::lvt_lint1));
    printf("  error:   0x%8x\n", local_apic.read_vector (local_apic_t<APIC_MAPPINGS_START>::lvt_error));
    printf("  perf:    0x%8x\n", local_apic.read_vector (local_apic_t<APIC_MAPPINGS_START>::lvt_perfcount));
    printf("  thermal: 0x%8x\n", local_apic.read_vector (local_apic_t<APIC_MAPPINGS_START>::lvt_thermal_monitor));

    return CMD_NOQUIT;
}
#endif

