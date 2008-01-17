/*********************************************************************
 *                
 * Copyright (C) 2007-2008,  Karlsruhe University
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
#include INC_API(tcb.h)
#include INC_ARCH(cpu.h)
#include INC_ARCH(trapgate.h)
#include INC_ARCH(ioport.h)
#include INC_ARCH(segdesc.h)
#include INC_GLUE(config.h)
#include INC_GLUE(schedule.h)

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


#if defined(CONFIG_SUBARCH_X32)
#define __BITS_WORD	32	
#else
#define __BITS_WORD	64
#endif

bool x86_reboot_scheduled;

extern void x86_reset();
void x86_reset_wrapper()
{
    asm volatile (
	".global x86_reset				\n\t"			
	".type x86_reset,@function			\n\t"			
	".section .init					\n\t"			
  	"x86_reset:					\n\t"
	"movb	$0xFE, %al	        		\n\t"
	"outb	%al, $0x64	        		\n\t"
	".previous					\n\t"			
	);	
}
CMD(cmd_reset, cg)
{  
#if defined(CONFIG_IOAPIC)
    local_apic_t<APIC_MAPPINGS_START> local_apic;
    local_apic.disable();
#endif
    x86_reboot_scheduled = true;
    x86_reset();
    /* NOTREACHED */
    ASSERT(false);
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
    printf("LASTBRANCH_FROM_IP: %x\n", x86_rdmsr (X86_MSR_LASTBRANCHFROMIP));
    printf("LASTBRANCH_TO_IP:   %x\n", x86_rdmsr (X86_MSR_LASTBRANCHTOIP));
    printf("LASTINT_FROM_IP:    %x\n", x86_rdmsr (X86_MSR_LASTINTFROMIP));
    printf("LASTINT_TO_IP:      %x\n", x86_rdmsr (X86_MSR_LASTINTTOIP));
#endif

#if defined(CONFIG_CPU_X86_P4)
    for (int i = 0; i < 18; i++) {
	u64_t pmc = x86_rdmsr (X86_MSR_COUNTER_BASE + i);
	u64_t cccr = x86_rdmsr (X86_MSR_CCCR_BASE + i);
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
DECLARE_CMD (cmd_send_nmi, arch, 'N', "send_nmi", "send NMI to CPU");

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

DECLARE_CMD (cmd_switch_cpus, arch, 'S', "switch_cpu", "switch CPU");

extern atomic_t kdb_current_cpu;
extern void kdb_wait_for_cpu();

CMD(cmd_switch_cpus, cg)
{
    cpuid_t cpu = get_current_cpu();
    word_t dst_cpu = get_dec("CPU id", 0, NULL);
    if (dst_cpu >= CONFIG_SMP_MAX_CPUS ||
	!cpu_t::get(dst_cpu)->is_valid())
	return CMD_NOQUIT;

    kdb_current_cpu = dst_cpu;
    local_apic_t<APIC_MAPPINGS_START> local_apic;
    local_apic.send_nmi(dst_cpu);
    /* Execute a dummy iret to receive NMIs again, then sleep */
    x86_iret_self();
    x86_sleep_uninterruptible();
    
    if (kdb_current_cpu == cpu)
    {
	printf("--- Switched to CPU %d ---\n", cpu);
	return CMD_ABORT;
    }
    
    return CMD_QUIT;
    
}


#endif /* defined(CONFIG_SMP) */

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

