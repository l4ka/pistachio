/*********************************************************************
 *
 * Copyright (C) 2002-2007,  Karlsruhe University
 *
 * File path:     glue/v4-x86/init.cc
 * Description:   ia32-specific initialization
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
 ********************************************************************/

#include <debug.h>
#include <kmemory.h>
#include <mapping.h>
#include <ctors.h>
#include <init.h>
#include <linear_ptab.h>
#include <kdb/tracepoints.h>

#include INC_GLUE(idt.h)
#include INC_GLUE(intctrl.h)
#include INC_GLUE(space.h)
#include INC_GLUE(timer.h)
#include INC_GLUE(memory.h)
#include INC_GLUE(cpu.h)

#include INC_ARCH(apic.h)
#include INC_ARCH(amdhwcr.h)

#include INC_API(smp.h)
#include INC_API(kernelinterface.h)
#include INC_API(processor.h)
#include INC_API(schedule.h)

#include INC_PLAT(perfmon.h)

#if defined(CONFIG_X86_COMPATIBILITY_MODE)
#include INC_GLUE_SA(x32comp/kernelinterface.h)
#include INC_GLUE_SA(x32comp/init.h)
#endif /* defined(CONFIG_X86_COMPATIBILITY_MODE) */


// from either glue/v4-ia32/ or glue/v4-x86/
void setup_msrs();
void SECTION(SEC_INIT) init_meminfo();
void SECTION(".init.cpu") check_cpu_features();
cpuid_t SECTION(".init.cpu") init_cpu();
void SECTION(SEC_INIT) setup_gdt(x86_tss_t &tss, cpuid_t cpuid);

#if defined(CONFIG_TRACEBUFFER)
tracebuffer_t * tracebuffer;
EXTERN_KMEM_GROUP (kmem_misc);

void SECTION(SEC_INIT) setup_tracebuffer (void)
{
    tracebuffer = (tracebuffer_t *) kmem.alloc (kmem_misc, TRACEBUFFER_SIZE);
    for (word_t p = 0; p < TRACEBUFFER_SIZE; p += page_size(TRACEBUFFER_PGENTSZ))
    {	
	//TRACEF("add tbuf mapping %t -> %t\n", addr_offset(tracebuffer, p),
	//     virt_to_phys(addr_offset(tracebuffer, p)));
	get_kernel_space()->add_mapping(addr_offset(tracebuffer, p),
					virt_to_phys(addr_offset(tracebuffer, p)),
					TRACEBUFFER_PGENTSZ,
					true, false, true);
    }
    tracebuffer->initialize ();
}
#endif /* CONFIG_TRACEBUFFER */


#if defined(CONFIG_SMP)
/**************************************************************************
 *
 * SMP functions.
 *
 *************************************************************************/
extern "C" void _start_ap(void);
extern void setup_smp_boot_gdt();

spinlock_t smp_boot_lock;
/* commence to sync TSC */
spinlock_t smp_commence_lock;


INLINE u8_t get_apic_id (void)
{
    local_apic_t<APIC_MAPPINGS_START> apic;
    return apic.id();
}


static void smp_ap_commence (void)
{
    smp_boot_lock.unlock();

    /* finally we sync the time-stamp counters */
    while( smp_commence_lock.is_locked() );

    x86_settsc(0);
}


static void smp_bp_commence (void)
{
    // wait for last processor to call in
    smp_boot_lock.lock();

    // now release all at once
    smp_commence_lock.unlock();

    x86_settsc(0);
}


/**
 * startup_processor
 */
extern "C" void SECTION(SEC_INIT) startup_processor (void)
{
#if defined(CONFIG_DEBUG)
    if (x86_reboot_scheduled)
	x86_reset();
#endif

    TRACE_INIT("\tAP processor is alive\n");
    x86_mmu_t::set_active_pagetable((word_t) get_kernel_space()->get_top_pdir_phys());
    TRACE_INIT("\tAP switched to kernel ptab\n");

    // first thing -- check CPU features
    check_cpu_features();

    /* perform processor local initialization */
    cpuid_t cpuid = init_cpu();

    get_current_scheduler()->init (false);
    get_idle_tcb()->notify (smp_ap_commence);
    get_current_scheduler()->start (cpuid);

    spin_forever(cpuid);
}
#endif /* defined(CONFIG_SMP) */



void SECTION(SEC_INIT) clear_bss (void)
{
    extern u8_t _start_bss[];
    extern u8_t _end_bss[];
    for (u8_t* p = _start_bss; p < _end_bss; p++)
	*p = 0;
}

/**
 * init_bootmem: initializes the boot memory
 *
 * At system startup, a fixed amount of kernel memory is allocated
 * to allow basic initialization of the system before sigma0 is up.
 */

void SECTION(SEC_INIT) init_bootmem (void)
{
    for (addr_t p = start_bootmem; p < end_bootmem; p = addr_offset(p, sizeof(word_t)))
	* (word_t *)p = 0;

    kmem.init(start_bootmem, end_bootmem);
    /* now do reservations */

    // Mark the kernel code as reserved
    get_kip()->reserved_mem0.set(start_text_phys, end_bootmem_phys);

#if defined(CONFIG_SUBARCH_X32)
    // Were we booted via RMGR?
    if (!get_kip()->main_mem.is_empty())
    {
        word_t end = (word_t)get_kip()->main_mem.high;

        /* Allocate from end of physical memory or from end of kernel
         *accessible physical memory, whatever is lower  */
	if (end < virt_to_phys (KERNEL_AREA_END))
	    end = virt_to_phys(KERNEL_AREA_END);
        
        get_kip()->reserved_mem1.set((addr_t) (end - ADDITIONAL_KMEM_SIZE),
                                     (addr_t) end);
    }
#endif

}

void SECTION(SEC_INIT) add_more_kmem (void)
{
    /* Scan memory descriptors for a block of reserved physical memory
     * that is within the range of (to be) contiguously mapped
     * physical memory.  If there's one, it has been set up by the
     * boot loader for us. */
    bool found = false;
    for (word_t i = 0;
         i < get_kip()->memory_info.get_num_descriptors();
         i++)
    {
        memdesc_t* md = get_kip()->memory_info.get_memdesc(i);
        if (!md->is_virtual() &&
            (md->type() == memdesc_t::reserved) &&
            (word_t) md->high() <= KERNEL_AREA_END)
	{	    
	    TRACE_INIT("\tfound  %dM kmem (%x-%x) -> (%x-%x)\n", 
		       md->size() / (1024*1024), md->low(), md->high(), 
		       phys_to_virt(md->low()), phys_to_virt(md->high()));
	    
	    // Align to kernel page size
	    mem_region_t alloc = { addr_align_up(md->low(), KERNEL_PAGE_SIZE),
				   addr_align(addr_offset(md->low(),md->size()), KERNEL_PAGE_SIZE) };

	    // If KMEM is larger than 32 MB, allocate it chunkwise
	    const word_t chunksize = 32 * 1024 * 1024;
	    word_t allocsize = 0;
	    
	    while (alloc.get_size()) 
	    {
		if (alloc.get_size() >= chunksize )
		    allocsize = chunksize - (word_t) addr_mask(alloc.low, (chunksize-1));
		else 
		    allocsize = alloc.get_size();
		

		// Map region kernel writable 
		//TRACEF("add %x %x\n", alloc.low, allocsize);
		get_kernel_space()->remap_area(
		    phys_to_virt(alloc.low), alloc.low,
		    PGSIZE_KERNEL,
		    allocsize, true, true, true);

		// Add it to allocator
		kmem.add(phys_to_virt(alloc.low), allocsize);
		
		alloc.low = addr_offset(alloc.low, allocsize);
	    }
            found = true;
        }
    }
    
    /* Fall back to ol'style if no memory descriptors can be
       found */
    if (!found)
    {
        if (!get_kip()->reserved_mem1.is_empty())
        {
            kmem.add(phys_to_virt(get_kip()->reserved_mem1.low), 
                     get_kip()->reserved_mem1.get_size());
        }
    }
}


/**
 * init_cpu: initializes the processor
 *
 * this function is called once for each processor to initialize
 * the processor specific data and registers
 */
cpuid_t SECTION(".init.cpu") init_cpu (void)
{
    cpuid_t cpuid = 0;

    /* configure IRQ hardware - local part
     * this has to be done before reading the cpuid since it may change
     * when having one of those broken BIOSes like ServerWorks */
    get_interrupt_ctrl()->init_cpu();
    
#if defined(CONFIG_SMP)
    word_t apicid = get_apic_id();
    for (cpuid = 0; cpuid < cpu_t::count; cpuid++)
	if (cpu_t::get(cpuid)->apicid == apicid)
	    break;
    if (cpuid > CONFIG_SMP_MAX_CPUS)
	panic("unconfigured CPU started (LAPIC id %d)\n", apicid);
#endif
    
    TRACE_INIT("\tActivating TSS (CPU %d)se\n", cpuid);
    tss.setup(X86_KDS);

    TRACE_INIT("\tInitializing GDT (CPU %d)\n", cpuid);
    setup_gdt(tss, cpuid);

    /* can take exceptions from now on,
     * idt is initialized via a constructor */
    TRACE_INIT("\tActivating IDT (CPU %d)\n", cpuid);;
    idt.activate();

#if defined(CONFIG_PERFMON)

#if defined(CONFIG_TBUF_PERFMON)
    /* initialize tracebuffer */
    TRACE_INIT("\tInitializing Tracebuffer PMCs (CPU %d)\n", cpuid);
    setup_perfmon_cpu(cpuid);
#endif

     /* Allow performance counters for users */
    TRACE_INIT("\tEnabling performance monitoring at user level (CPU %d)\n", cpuid);
    x86_cr4_set(X86_CR4_PCE);
#endif /* defined(CONFIG_PERFMON) */

#if defined(CONFIG_X86_PGE)
    TRACE_INIT("\tEnabling global pages (CPU %d)\n", cpuid);
    x86_mmu_t::enable_global_pages();
#endif

#if defined(CONFIG_CPU_X86_K8) 
#if defined(CONFIG_K8_FLUSHFILTER)
    TRACE_INIT("\tEnabling K8 Flush Filter\n");
    x86_amdhwcr_t::enable_flushfilter();
#else
    TRACE_INIT("\tDisabling K8 Flush Filter\n");
    x86_amdhwcr_t::disable_flushfilter();
#endif
#endif

    TRACE_INIT("\tActivating MSRS (CPU %d)\n", cpuid);
    setup_msrs();

    TRACE_INIT("\tInitializing Timer (CPU %d)\n", cpuid);
    get_timer()->init_cpu(cpuid);

    /* initialize V4 processor info */
    TRACE_INIT("\tInitializing Processor (CPU %d)\n", cpuid);
    init_processor (cpuid, get_timer()->get_bus_freq(),
		    get_timer()->get_proc_freq());

    /* CPU specific mappings */
    get_kernel_space()->init_cpu_mappings(cpuid);

    call_cpu_ctors();

    return cpuid;
}



/**
 * Entry Point into C Kernel
 *
 * Precondition:
 *   - paging is initialized via init_paging
 *   - idempotent mapping at KERNEL_OFFSET
 *
 * The startup happens in two steps
 *   1) all global initializations are performed
 *      this includes initializing necessary devices and
 *      the kernel debugger. The kernel memory allocator
 *      is set up (see init_arch).
 *
 *   2) the boot processor itself is initialized
 */

#if defined(CONFIG_IS_64BIT)
extern "C" void SECTION(".init.init64") startup_system(u32_t is_ap)
#else
    extern "C" void SECTION(SEC_INIT) startup_system (void)
#endif
{
#if defined(CONFIG_IS_64BIT) && defined(CONFIG_SMP)
    /* check if we are running on the BSP or on an AP */
    if ( is_ap )
	startup_processor();
#endif

    clear_bss();

    init_console();

    call_global_ctors();
    call_node_ctors();

    init_hello();

    TRACE_INIT("Checking CPU features\n");
    check_cpu_features();

    TRACE_INIT("virtual memory layout:\n"
	       "\tuser area     %wx - %wx\n"
	       "\tcopy area     %wx - %wx\n"
	       "\tktcb area     %wx - %wx\n"
	       "\tkernel area   %wx - %wx\n"
	       "\tcpulocal data %wx - %wx\n"
#if defined(CONFIG_IOAPIC)
	       "\tapic area     %wx - %wx\n"
#endif
	       "\tutcb pgarea   %wx\n"
	       "\tspace link    %wx\n",
	       USER_AREA_START, USER_AREA_END,
	       COPY_AREA_START, COPY_AREA_END,
	       KTCB_AREA_START, KTCB_AREA_END,
	       KERNEL_AREA_START, KERNEL_AREA_END,
	       start_cpu_local, end_cpu_local,
#if defined(CONFIG_IOAPIC)
	       APIC_MAPPINGS_START, APIC_MAPPINGS_END,
#endif
	       UTCB_MAPPING,  SPACE_BACKLINK
	);
	       
	       
    /* feed the kernel memory allocator */
    init_bootmem();

    TRACE_INIT("Initializing kernel space\n");
    space_t::init_kernel_space();

    TRACE_INIT("Activating TSS (Preliminary)\n");
    tss.setup(X86_KDS);

    TRACE_INIT("Initializing GDT (Preliminary)\n");
    setup_gdt(tss, 0);

    TRACE_INIT("Activating IDT (Preliminary)\n");
    idt.activate();

    TRACE_INIT("Initializing kernel interface page (%p)\n", get_kip());
    get_kip()->init();

    TRACE_INIT("Adding more kernel memory\n");
    add_more_kmem();

    TRACE_INIT("Initializing memory info\n");
    init_meminfo();

    TRACE_INIT("Initializing mapping database\n");
    init_mdb();

#if defined(CONFIG_X86_IO_FLEXPAGES)
    TRACE_INIT("Initializing IO port space\n");
    init_io_space();
#endif

#if defined(CONFIG_TRACEBUFFER)
    /* allocate and setup tracebuffer */
    TRACE_INIT("Initializing Tracebuffer\n");
    setup_tracebuffer();
#endif

    TRACE_INIT("Initializing kernel debugger\n");
    if (get_kip()->kdebug_init)
	get_kip()->kdebug_init();

    /* configure IRQ hardware - global part */
    TRACE_INIT("Initializing IRQ hardware\n");
    get_interrupt_ctrl()->init_arch();

    /* initialize the kernel's timer source */
    TRACE_INIT("Initializing Timer\n");
    get_timer()->init_global();

#if defined(CONFIG_SMP)
    /* start APs on an SMP + rendezvous */
    {
	TRACE_INIT("Starting %d application processors (%p->%p)\n",
		   cpu_t::count, _start_ap, SMP_STARTUP_ADDRESS);
	
	// aqcuire commence lock before starting any processor
	smp_commence_lock.init (1);

	// boot gdt
	setup_smp_boot_gdt();

	// IPI trap gates
	init_xcpu_handling ();

	// copy startup code to startup page
	for (word_t i = 0; i < X86_PAGE_SIZE / sizeof(word_t); i++)
	    ((word_t*)SMP_STARTUP_ADDRESS)[i] = ((word_t*)_start_ap)[i];

	/* at this stage we still have our 1:1 mapping at 0 */
	*((volatile unsigned short *) 0x469) = (SMP_STARTUP_ADDRESS >> 4);
	*((volatile unsigned short *) 0x467) = (SMP_STARTUP_ADDRESS) & 0xf;

	local_apic_t<APIC_MAPPINGS_START> local_apic;

	word_t apic_id = get_apic_id();

	for (cpuid_t cpuid = 0; cpuid < cpu_t::count; cpuid++) {
	    cpu_t* cpu = cpu_t::get(cpuid);

	    // don't start ourselfs
	    if (cpu->get_apic_id() == apic_id)
		continue;

	    smp_boot_lock.lock(); // unlocked by AP
	    TRACE_INIT("Sending startup IPI to CPU#%d APIC %d\n", 
		       cpuid, cpu->get_apic_id());
	    local_apic.send_init_ipi(cpu->get_apic_id(), true);
	    for (int i = 0; i < 200000; i++);
	    local_apic.send_init_ipi(cpu->get_apic_id(), false);
	    local_apic.send_startup_ipi(cpu->get_apic_id(), (void(*)(void))SMP_STARTUP_ADDRESS);
#warning VU: time out on AP call in
	}
    }
    
#endif /* CONFIG_SMP */

    /* Initialize CPU */
    cpuid_t cpuid = init_cpu();
    
#ifdef CONFIG_SMP
    smp_bp_commence ();
#else
    cpu_t::add_cpu(0);
#endif


#if defined(CONFIG_X86_COMPATIBILITY_MODE)
    TRACE_INIT("Initializing 32-bit kernel interface page (%p)\n",
	       x32::get_kip());
    x32::get_kip()->init();
    init_kip_32();
#endif /* defined(CONFIG_X86_COMPATIBILITY_MODE) */

    /* initialize the scheduler */
    get_current_scheduler()->init(true);
    /* get the thing going - we should never return */
    get_current_scheduler()->start(cpuid);

    /* make sure we don't fall off the edge */
    spin_forever(cpuid);
}

