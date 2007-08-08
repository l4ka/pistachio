/*********************************************************************
 *                
 * Copyright (C) 2002-2007,  Karlsruhe University
 *                
 * File path:     glue/v4-amd64/init.cc
 * Description:   System initialization
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
 * $Id: init.cc,v 1.27 2007/02/21 07:06:14 stoess Exp $ 
 *                
 ********************************************************************/
#include <init.h>
#include <kmemory.h>
#include <mapping.h>
#include <ctors.h>


#include INC_API(smp.h)

#include INC_API(kernelinterface.h)
#include INC_API(types.h)
#include INC_API(processor.h)
#include INC_API(schedule.h)

#include INC_ARCH(cpuid.h)
#include INC_ARCH(descreg.h)
#include INC_ARCHX(x86,amdhwcr.h)
#include INC_ARCH(segdesc.h)
#include INC_ARCH(tss.h)
#include INC_ARCHX(x86,apic.h)
#include INC_ARCH(config.h)

#include INC_GLUE(intctrl.h)
#include INC_GLUE(timer.h)
#include INC_GLUE(idt.h)
#include INC_GLUEX(x86,memory.h)

#include INC_PLAT(rtc.h)
#include INC_PLAT(perfmon.h)


#if defined(CONFIG_AMD64_COMPATIBILITY_MODE)
#include INC_GLUE(ia32/kernelinterface.h)
#include INC_GLUE(ia32/init.h)
#endif /* defined(CONFIG_AMD64_COMPATIBILITY_MODE) */

amd64_cpu_features_t boot_cpu_ft UNIT("amd64.cpulocal") CTORPRIO(CTORPRIO_GLOBAL, 1);
amd64_tss_t tss UNIT("amd64.cpulocal") CTORPRIO(CTORPRIO_GLOBAL, 2);
bool tracebuffer_initialized UNIT("amd64.cpulocal");


struct gdt_struct {
    amd64_segdesc_t segdsc[GDT_SIZE - 2];	/* 6 entries a  8 byte */
    amd64_tssdesc_t tssdsc;			/* 1 entries a 16 byte */
} gdt UNIT("amd64.cpulocal");

u8_t amd64_cache_line_size;


/**********************************************************************
 *
 * SMP specific code and data
 *
 **********************************************************************/

#if defined(CONFIG_SMP)
extern "C" void _start_ap(void);
extern "C" void init_paging();
extern "C" void SECTION(SEC_INIT) startup_processor();
spinlock_t smp_boot_lock;

/* commence to sync TSC */
static void smp_bp_commence();
spinlock_t smp_commence_lock;


amd64_segdesc_t	smp_boot_gdt[3];
static void setup_smp_boot_gdt()
{
    /* segment descriptors in long mode and legacy mode are almost identical.
     *  However, in long mode, most of the fields are ignored, thus we can set
     *  up those segments although the APs are not yet in long mode when they
     *  are used.
     */
#   define gdt_idx(x) ((x) >> 3)
  smp_boot_gdt[gdt_idx(X86_KCS)].set_seg((u64_t)0, amd64_segdesc_t::code, 0, amd64_segdesc_t::m_comp);
  smp_boot_gdt[gdt_idx(X86_KDS)].set_seg((u64_t)0, amd64_segdesc_t::data, 0, amd64_segdesc_t::m_comp);  
#   undef gdt_idx
}


INLINE u8_t get_apic_id()
{
    local_apic_t<APIC_MAPPINGS> apic;
    return apic.id();
}
#endif

/**
 * Clear BSS
 * 
 */
static void SECTION(SEC_INIT) clear_bss()
{

    extern u8_t _start_bss[];
    extern u8_t _end_bss[];
    
    for (u8_t *p = _start_bss; p < _end_bss; p++)
	*p = 0;

}


/**
 * Check CPU features 
 * 
 */

static void SECTION(SEC_INIT) check_cpu_features()
{

#if 0
    boot_cpu_ft.dump_features();
#endif
    amd64_cache_line_size = boot_cpu_ft.get_l1_cache().d.dcache.l_size;
}

/**
 * Initialize boot memory 
 * 
 */

static void SECTION(SEC_INIT) init_bootmem()
{
    
    extern u8_t _start_bootmem[];
    extern u8_t _end_bootmem[];
    
    
    for (u8_t * p = _start_bootmem; p < _end_bootmem; p++){
	*p = 0;
    }

    kmem.init(start_bootmem, end_bootmem); 
}

static void add_more_kmem()
{

    /* 
     * Scan memory descriptors for a block of reserved physical memory
     * that is within the range of (to be) contiguously mapped
     * physical memory.  If there's one, it has been set up by the
     * boot loader for us. 
     */
    
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
	    TRACE_INIT("Using region %x size %x for kernel memory\n", md->low(), md->size());
            /* Map region kernel writable  */
            get_kernel_space()->remap_area(
                phys_to_virt(md->low()), md->low(),
                (KERNEL_PAGE_SIZE == AMD64_2MPAGE_SIZE)
                 ? pgent_t::size_2m
                 : pgent_t::size_4k,
                (md->size() + (KERNEL_PAGE_SIZE-1)) & ~(KERNEL_PAGE_SIZE-1),
                true, true, true);

            /* Add it to allocator */
            kmem.add(phys_to_virt(md->low()),
                     md->size());
	    found = true;
        }
    }
    
    if (!found)
	TRACE_INIT("Did not find any region usable for kernel memory\n");

}

static void SECTION(SEC_INIT) init_meminfo()
{

    extern word_t _memory_descriptors_size[];

    if (get_kip()->memory_info.get_num_descriptors() == (word_t) &_memory_descriptors_size){
	TRACE_INIT("\tBootloader did not patch memory info...\n");
	get_kip()->memory_info.n = 0;
    }
    
    /* 
     * reserve ourselves
     */
    
    get_kip()->memory_info.insert(memdesc_t::reserved, false,
				  start_text_phys, end_text_phys);

    get_kip()->memory_info.insert(memdesc_t::reserved, false,
				  start_bootmem_phys, end_bootmem_phys);

    get_kip()->memory_info.insert(memdesc_t::reserved, false,
				  start_syscalls_phys, end_syscalls_phys);

    /* 
     * add user area
     */

    get_kip()->memory_info.insert(memdesc_t::conventional, true,
				  (void *) USER_AREA_START, (void *) USER_AREA_END);
    

    /* 
     * dump reserved memory regions
     */
    
    //TRACE_INIT("Memory descriptors (p=%p/n=%d):\n",
    //       get_kip()->memory_info.get_memdesc(0),
    //     get_kip()->memory_info.get_num_descriptors());
    //for (word_t i=0; i < get_kip()->memory_info.get_num_descriptors(); i++){
	//memdesc_t *m = get_kip()->memory_info.get_memdesc(i);
	//TRACE_INIT("\t#%d, low=%x, high=%x, type=%x\n", i, 
	//   m->low(),  m->high(),  m->type());
    //}

}

/**********************************************************************
 *
 *  processor local initialization, performed by all CPUs
 *
 **********************************************************************/


#if defined(CONFIG_TRACEBUFFER)
tracebuffer_t * tracebuffer;
EXTERN_KMEM_GROUP (kmem_misc);

void SECTION(SEC_INIT) setup_tracebuffer (void)
{
    tracebuffer = (tracebuffer_t *) kmem.alloc (kmem_misc, TRACEBUFFER_SIZE);
    ASSERT (TRACEBUFFER_SIZE == MB (2));
    get_kernel_space ()->add_mapping (tracebuffer,
				     virt_to_phys (tracebuffer),
				     pgent_t::size_2m,
				     true, false, true);
    tracebuffer->initialize ();
}
#endif /* defined(CONFIG_TRACEBUFFER) */

/**
 * Setup global descriptor table 
 * 
 */

static void SECTION(SEC_INIT) init_gdt(amd64_tss_t &tss, cpuid_t cpuid)
{

    /* Initialize GDT */
    gdt.segdsc[GDT_IDX(AMD64_INVS)].set_seg((u64_t) 0, amd64_segdesc_t::inv, 0, amd64_segdesc_t::m_long);
    gdt.segdsc[GDT_IDX(X86_KCS)].set_seg((u64_t) 0, amd64_segdesc_t::code, 0, amd64_segdesc_t::m_long);
    gdt.segdsc[GDT_IDX(X86_KDS)].set_seg((u64_t) 0, amd64_segdesc_t::data, 0, amd64_segdesc_t::m_long);
    gdt.segdsc[GDT_IDX(AMD64_UCS)].set_seg((u64_t) 0, amd64_segdesc_t::code, 3, amd64_segdesc_t::m_long);
    gdt.segdsc[GDT_IDX(AMD64_UDS)].set_seg((u64_t) 0, amd64_segdesc_t::data, 3, amd64_segdesc_t::m_long);
    
#if defined(CONFIG_AMD64_COMPATIBILITY_MODE)
    gdt.segdsc[GDT_IDX(AMD64_UCS32)].set_seg((u64_t) 0, amd64_segdesc_t::code, 3, amd64_segdesc_t::m_comp);
#endif /* defined(CONFIG_AMD64_COMPATIBILITY_MODE) */

    /* TODO: Assertion correct ? */
    ASSERT(unsigned(cpuid * AMD64_CACHE_LINE_SIZE) < AMD64_2MPAGE_SIZE);
    
    /* Set TSS */
#if defined(CONFIG_IO_FLEXPAGES)
    gdt.tssdsc.set_seg((u64_t) TSS_MAPPING, sizeof(amd64_tss_t) - 1);
#else 
    gdt.tssdsc.set_seg((u64_t) &tss, sizeof(amd64_tss_t) - 1);
#endif    

    /* Load descriptor registers */
    amd64_descreg_t::setdescreg(amd64_descreg_t::gdtr, (u64_t) &gdt, sizeof(gdt));
    amd64_descreg_t::setdescreg(amd64_descreg_t::tr, AMD64_TSS);


    /*
     * As reloading fs/gs clobbers the upper 32bit of the segment descriptor 
     * registers, we have to set them twice:
     * - before loading the segment selectors (otherwise #GP because of invalid segment)
     * - after reloading the segment selectors  (otherwise upper 32 bits = 0)
     * registers 
     */ 
	
    gdt.segdsc[GDT_IDX(AMD64_UTCBS)].set_seg(UTCB_MAPPING + (cpuid * AMD64_CACHE_LINE_SIZE),
					     amd64_segdesc_t::data,
					     3, 
					     amd64_segdesc_t::m_long,
					     amd64_segdesc_t::msr_gs);
    
#if defined(CONFIG_TRACEBUFFER)
    gdt.segdsc[GDT_IDX(AMD64_TBS)].set_seg((u64_t) tracebuffer,
					    amd64_segdesc_t::data,
					    3, 
					    amd64_segdesc_t::m_long,
					    amd64_segdesc_t::msr_fs);
#endif



    /* Load segment registers */
    asm("mov  %0, %%ds		\n\t"		// load data  segment (DS)
	"mov  %0, %%es		\n\t"		// load extra segment (ES)
	"mov  %0, %%ss		\n\t"		// load stack segment (SS)
	"mov  %1, %%gs		\n\t"		// load UTCB segment  (GS)
#ifdef CONFIG_TRACEBUFFER       
	"mov  %2, %%fs		\n\t"		// tracebuffer segment (FS)
#else
	"mov  %0, %%fs		\n\t"	        // no tracebuffer
#endif  
 	"pushq  %3	      	\n\t"		// new CS
 	"pushq $1f		\n\t"		// new IP		
 	"lretq			\n\t"
 	"1:			\n\t"	
	: /* No Output */ : "r" (0), "r" (AMD64_UTCBS), "r" (AMD64_TBS), "r" ((u64_t) X86_KCS)
	);
    
    
    gdt.segdsc[GDT_IDX(AMD64_UTCBS)].set_seg(UTCB_MAPPING + (cpuid * AMD64_CACHE_LINE_SIZE),
					     amd64_segdesc_t::data,
					     3, 
					     amd64_segdesc_t::m_long,
					     amd64_segdesc_t::msr_gs);
    
#if defined(CONFIG_TRACEBUFFER)
    gdt.segdsc[GDT_IDX(AMD64_TBS)].set_seg((u64_t)tracebuffer,
					    amd64_segdesc_t::data,
					    3, 
					    amd64_segdesc_t::m_long,
					    amd64_segdesc_t::msr_fs);
#endif

}

/**
 * setup_msrs: initializes all model specific registers for CPU
 */
static void setup_msrs()
{
    
    /* sysret (63..48) / syscall (47..32)  CS/SS MSR */
    x86_wrmsr(AMD64_STAR_MSR, ((AMD64_SYSRETCS << 48) | (AMD64_SYSCALLCS << 32)));
    
    /* long mode syscalls MSR */
    x86_wrmsr(AMD64_LSTAR_MSR, (u64_t)(syscall_entry));

    /* compatibility mode syscalls MSR */
#if defined(CONFIG_AMD64_COMPATIBILITY_MODE)
#if defined(CONFIG_CPU_AMD64_EM64T)
    x86_wrmsr(X86_SYSENTER_CS_MSR, AMD64_SYSCALLCS);
    x86_wrmsr(X86_SYSENTER_EIP_MSR, (u64_t)(sysenter_entry_32));
#if defined(CONFIG_IO_FLEXPAGES)
    x86_wrmsr(X86_SYSENTER_ESP_MSR, (u64_t)(TSS_MAPPING) + 4);
#else
    x86_wrmsr(X86_SYSENTER_ESP_MSR, (u64_t)(&tss) + 4);
#endif
#else /* !defined(CONFIG_CPU_AMD64_EM64T) */
    x86_wrmsr(AMD64_CSTAR_MSR, (u64_t)(syscall_entry_32));
#endif /* !defined(CONFIG_CPU_AMD64_EM64T) */
#endif /* defined(CONFIG_AMD64_COMPATIBILITY_MODE) */

    /* long mode syscall RFLAGS MASK  */
    x86_wrmsr(AMD64_SFMASK_MSR, (u64_t)(AMD64_SYSCALL_FLAGMASK));

    /* enable syscall/sysret in EFER */
    word_t efer = x86_rdmsr(AMD64_EFER_MSR);
    efer |= AMD64_EFER_SCE;
    x86_wrmsr(AMD64_EFER_MSR, efer);
    
}

static cpuid_t SECTION(".init.cpu") init_cpu()
{
    cpuid_t cpuid = 0;

#if defined(CONFIG_SMP)
    cpuid = get_apic_id();
#endif
    /* set up task state segment */
    TRACE_INIT("Activating TSS (CPU %d)\n", cpuid);
    tss.init();

    /* set up global descriptor table */
    TRACE_INIT("Initializing GDT (CPU %d)\n", cpuid);
    init_gdt(tss, cpuid);

    /* activate idt */
    TRACE_INIT("Activating IDT (CPU %d)\n", cpuid);;
    idt.activate();

    /* configure IRQ hardware - local part */    
    get_interrupt_ctrl()->init_cpu();    
    
    
#if defined(CONFIG_PERFMON)
    
#if defined(CONFIG_TBUF_PERFMON) 
    /* initialize tracebuffer */
    TRACE_INIT("Initializing Tracebuffer PMCs (CPU %d)\n", cpuid);
    setup_perfmon_cpu(cpuid);
#endif
    
     /* Allow performance counters for users */
    TRACE_INIT("Enabling performance monitoring at user level (CPU %d)\n", cpuid);
    x86_cr4_set(X86_CR4_PCE);  
#endif /* defined(CONFIG_PERFMON) */

    TRACE_INIT("Enabling global pages (CPU %d)\n", cpuid);
    x86_mmu_t::enable_global_pages();    

#if defined(CONFIG_FLUSHFILTER)
    TRACE_INIT("Enabling flush filter (CPU %d)\n", cpuid);
    x86_amdhwcr_t::enable_flushfilter();
#endif
    
    /* activate msrs */
    TRACE_INIT("Activating MSRS (CPU %d)\n", cpuid);
    setup_msrs();
	
    /* initialize the kernel's timer source - per CPU part*/
    TRACE_INIT("Initializing Timer (CPU %d)\n", cpuid);
    get_timer()->init_cpu();

    /* initialize V4 processor info */
    TRACE_INIT("Initializing Processor (CPU %d)\n", cpuid);
    init_processor (cpuid, get_timer()->get_bus_freq(), 
		    get_timer()->get_proc_freq());

    /* CPU specific mappings */
    get_kernel_space()->init_cpu_mappings(cpuid);    

    return cpuid;
}     

/**
 * Entry Point into C Kernel
 * 
 * Precondition: 64bit-mode enabled
 *		 idempotent mapping and mapping at KERNEL_OFFSET
 *   
 */

extern "C" void SECTION(".init.init64") startup_system(u32_t is_ap)
{   

#if defined(CONFIG_SMP)
    /* check if we are running on the BSP or on an AP */
    if ( is_ap )
	startup_processor();    
#endif 

    /* clear bss */
    clear_bss();
    
    /* init console */
    init_console(); 

    
    /* call global constructors */
    call_global_ctors();

    /* call node constructors */
    call_node_ctors();

    /* say hello ... */ 
    init_hello();
   
    /* Get CPU Features */
    TRACE_INIT("Checking CPU features\n");
    check_cpu_features();

    /* feed the kernel memory allocator */
    TRACE_INIT("Initializing boot memory (%p - %p)\n",
		start_bootmem, end_bootmem);
    init_bootmem();


    /* initialize kernel space */
    TRACE_INIT("Initializing kernel space\n");
    init_kernel_space();

    {
	/* copied here to catch errors early */
	TRACE_INIT("Activating TSS (Preliminary)\n");
	tss.init();
	/* set up global descriptor table */
	TRACE_INIT("Initializing GDT (Preliminary)\n");
	init_gdt(tss, 0);
	
	/* activate idt */
	TRACE_INIT("Activating IDT (Preliminary)\n");
	idt.activate();
    }

    /* initialize kernel interface page */
    TRACE_INIT("Initializing kernel interface page (%p)\n",
           get_kip());
    get_kip()->init();

 
    /* add additional kernel memory */
    TRACE_INIT("Adding more kernel memory\n");
    add_more_kmem();  

    /* init memory info */
    TRACE_INIT("Initializing memory info\n");
    init_meminfo(); 

    
    /* initialize mapping database */
    TRACE_INIT("Initializing mapping database\n");
    init_mdb ();

#if defined(CONFIG_IO_FLEXPAGES)
    /* configure IRQ hardware - global part */
    TRACE_INIT("Initializing IO port space\n");
    init_io_space();
#endif
    
#if defined(CONFIG_TRACEBUFFER)
    /* allocate and setup tracebuffer */
    TRACE_INIT("Initializing Tracebuffer\n");
    setup_tracebuffer();
#endif

    /* initialize kernel debugger if any */
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
	TRACE_INIT("Starting application processors (%p->%p)\n", 
		   _start_ap, SMP_STARTUP_ADDRESS);
	
	/* aqcuire commence lock before starting any processor */
	smp_commence_lock.init (1);

	/* boot gdt */
	setup_smp_boot_gdt();

	/* IPI trap gates */
	init_xcpu_handling ();

	// copy startup code to startup page
	for (word_t i = 0; i < AMD64_4KPAGE_SIZE / sizeof(word_t); i++)
	    ((word_t*)SMP_STARTUP_ADDRESS)[i] = ((word_t*)_start_ap)[i];

	/* at this stage we still have our 1:1 mapping at 0 */

	/* this is the location of the warm-reset vector	 
	 * (40:67h in real mode addressing) which points to the
	 * cpu startup code
	 */
	*((volatile unsigned short *) 0x469) = (SMP_STARTUP_ADDRESS >> 4);  // real mode segment selector
	*((volatile unsigned short *) 0x467) = (SMP_STARTUP_ADDRESS) & 0xf; // offset

	local_apic_t<APIC_MAPPINGS> local_apic;

	// make sure we don't try to kick out more CPUs we can handle
	int smp_cpus = 1;

	u8_t apic_id = get_apic_id();

	for (word_t id = 0; id < sizeof(word_t) * 8; id++)
	{
	    if (id == apic_id)
		continue;

  	
	    // note the typecast (word_t)1
	    // This is important if we want to check for a maximum of
	    // 64 cpus .If we would'nt cast, (1 << id) would be a 32bit 
	    // integer and overflow if id == 0x20.
	    if ((get_interrupt_ctrl()->get_lapic_map() & ((word_t)1 << id)) != 0)
	    {
		if (++smp_cpus > CONFIG_SMP_MAX_CPUS)
		{
		    printf("found more CPUs than Pistachio supports\n");
		    spin_forever();
		}
		smp_boot_lock.lock(); // unlocked by AP
		
		TRACE_INIT("Sending startup IPI to APIC %d\n", id);
		
		local_apic.send_init_ipi(id, true);
		for (int i = 0; i < 100000; i++);
		local_apic.send_init_ipi(id, false);
		for (int i = 0; i < 100000; i++);
		local_apic.send_startup_ipi(id, (void(*)(void))SMP_STARTUP_ADDRESS);
		
#warning VU: time out on AP call in
	    }
	}
	
    }


    smp_bp_commence ();

#endif /* CONFIG_SMP */


    /* Initialize CPU */
    TRACE_INIT("Initializing CPU 0\n");
    cpuid_t cpuid = init_cpu();

#if defined(CONFIG_AMD64_COMPATIBILITY_MODE)
    TRACE_INIT("Initializing 32-bit kernel interface page (%p)\n",
           ia32::get_kip());
    ia32::get_kip()->init();
    init_kip_32();
#endif /* defined(CONFIG_AMD64_COMPATIBILITY_MODE) */

    /* initialize the scheduler */
    get_current_scheduler()->init(true);
    /* get the thing going - we should never return */
    get_current_scheduler()->start(cpuid);
    
    
    /* make sure we don't fall off the edge */
    spin_forever(1);
}


#if defined(CONFIG_SMP)
static void smp_ap_commence()
{
    smp_boot_lock.unlock();

    /* finally we sync the time-stamp counters */
    while( smp_commence_lock.is_locked() );

    /* TSC should not be written, but we do it anyway ;-) */
    x86_settsc(0);
}

static void smp_bp_commence()
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

void startup_processor()
{
    TRACE_INIT("AP processor is alive\n");    
    x86_mmu_t::set_active_pagetable((word_t)get_kernel_space()->get_pml4());
    TRACE_INIT("AP switched to kernel ptab\n");

    /* first thing -- check CPU features */
    check_cpu_features();

    /* perform processor local initialization */
    cpuid_t cpuid = init_cpu();
    
    get_current_scheduler()->init (false);
    get_idle_tcb()->notify (smp_ap_commence);
    get_current_scheduler()->start (cpuid);

    spin_forever(cpuid);
}
#endif
