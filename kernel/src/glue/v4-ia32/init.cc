/*********************************************************************
 *                
 * Copyright (C) 2002-2007,  Karlsruhe University
 *                 
* File path:     glue/v4-ia32/init.cc
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
 * $Id: init.cc,v 1.60 2007/03/20 14:56:23 skoglund Exp $
 *                
 ********************************************************************/

#include <debug.h>
#include <kmemory.h>
#include <mapping.h>
#include <ctors.h>

#define SEC_INIT	".init"

/* cpu specific types */
#include INC_ARCH(cpu.h)

/* pagetable and mmu management */
#include INC_ARCHX(x86,mmu.h)
#include INC_ARCH(ptab.h)

/* idt, tss, gdt etc. */
#include INC_ARCH(segdesc.h)
#include INC_ARCH(sysdesc.h)
#include INC_ARCH(tss.h)

/* floating point unit */
#include INC_ARCHX(x86,fpu.h)

/* K8 flush filter support  */
#if defined(CONFIG_CPU_IA32_K8)
#include INC_ARCHX(x86,amdhwcr.h)
#endif 

#include INC_GLUE(config.h)
#include INC_GLUE(idt.h)
#include INC_GLUE(space.h)
#include INC_GLUE(intctrl.h)
#include INC_GLUE(memory.h)
#include INC_GLUE(timer.h)
#include INC_GLUE(syscalls.h)

#include INC_API(smp.h)
#include INC_API(kernelinterface.h)
#include INC_API(tcb.h)
#include INC_API(schedule.h)
#include INC_API(processor.h)

#include INC_PLAT(rtc.h)
#include INC_PLAT(perfmon.h)

static word_t init_pdir[1024] __attribute__((aligned(4096))) SECTION(".init.data");

#if defined (CONFIG_IA32_PSE)

#define MAX_KERNEL_MAPPINGS	64
#define PAGE_ATTRIB_INIT	(IA32_PAGE_VALID | IA32_PAGE_WRITABLE | IA32_PAGE_SUPER)

#else /* !CONFIG_IA32_PSE */

#define MAX_KERNEL_MAPPINGS	8
#define PAGE_ATTRIB_INIT	(IA32_PAGE_VALID | IA32_PAGE_WRITABLE)
// 2nd-level page tables for the initial page table
static word_t init_ptable[MAX_KERNEL_MAPPINGS][1024] __attribute__((aligned(4096))) SECTION(".init.data");

#endif /* !CONFIG_IA32_PSE */

/**********************************************************************
 *
 * SMP specific code and data
 *
 **********************************************************************/
#if defined(CONFIG_SMP)
extern "C" void _start_ap(void);
spinlock_t smp_boot_lock;

/* commence to sync TSC */
static void smp_bp_commence();
spinlock_t smp_commence_lock;


ia32_segdesc_t	smp_boot_gdt[3];
static void setup_smp_boot_gdt()
{
#   define gdt_idx(x) ((x) >> 3)
    smp_boot_gdt[gdt_idx(X86_KCS)].set_seg(0, ~0UL, 0, ia32_segdesc_t::code);
    smp_boot_gdt[gdt_idx(X86_KDS)].set_seg(0, ~0UL, 0, ia32_segdesc_t::data);
#   undef gdt_idx
}

INLINE u8_t get_apic_id()
{
    local_apic_t<APIC_MAPPINGS> apic;
    return apic.id();
}
#endif



/**********************************************************************
 *
 *  processor local initialization, performed by all IA32 CPUs
 *
 **********************************************************************/

/* processor local data */
ia32_segdesc_t	gdt[GDT_SIZE] UNIT("ia32.cpulocal");
ia32_tss_t	tss UNIT("ia32.cpulocal");

#if defined(CONFIG_TRACEBUFFER)
tracebuffer_t * tracebuffer;
EXTERN_KMEM_GROUP (kmem_misc);

void SECTION(SEC_INIT) setup_tracebuffer (void)
{
    tracebuffer = (tracebuffer_t *) kmem.alloc (kmem_misc, TRACEBUFFER_SIZE);
    ASSERT (TRACEBUFFER_SIZE == MB (4));
    get_kernel_space ()->add_mapping (tracebuffer,
				     virt_to_phys (tracebuffer),
				     pgent_t::size_4m,
				     true, false, true);
    tracebuffer->initialize ();
}
#endif /* CONFIG_TRACEBUFFER */

static void setup_gdt(ia32_tss_t &tss, cpuid_t cpuid)
{
#   define gdt_idx(x) ((x) >> 3)

    gdt[gdt_idx(X86_KCS)].set_seg(0, ~0UL, 0, ia32_segdesc_t::code);
    gdt[gdt_idx(X86_KDS)].set_seg(0, ~0UL, 0, ia32_segdesc_t::data);
    gdt[gdt_idx(IA32_UCS)].set_seg(0, ~0UL, 3, ia32_segdesc_t::code);
    gdt[gdt_idx(IA32_UDS)].set_seg(0, ~0UL, 3, ia32_segdesc_t::data);

    /* MyUTCB pointer, 
     * we use a separate page for all processors allocated in space_t 
     * and have one UTCB entry per cache line in the SMP case */
    ASSERT(unsigned(cpuid * CACHE_LINE_SIZE) < IA32_PAGE_SIZE);
    gdt[gdt_idx(IA32_UTCB)].set_seg((u32_t)MYUTCB_MAPPING + 
				    (cpuid * CACHE_LINE_SIZE),
				    sizeof(threadid_t) - 1, 
				    3, ia32_segdesc_t::data);

    /* the TSS
     * The last byte in ia32_tss_t is a stopper for the IO permission bitmap.
     * That's why we set the limit in the GDT to one byte less than the actual
     * size of the structure. (IA32-RefMan, Part 1, Chapter Input/Output) */
#if defined(CONFIG_IO_FLEXPAGES)
    //TRACEF("gdt @ %d = %x (size %x)\n", gdt_idx(IA32_TSS), TSS_MAPPING, sizeof(ia32_tss_t)-1);
    gdt[gdt_idx(IA32_TSS)].set_sys((u32_t) TSS_MAPPING, sizeof(ia32_tss_t)-1, 
				   0, ia32_segdesc_t::tss);
#else
    //TRACEF("gdt @ %d = %x (size %x)\n", gdt_idx(IA32_TSS), &tss, sizeof(ia32_tss_t)-1);
    gdt[gdt_idx(IA32_TSS)].set_sys((u32_t) &tss, sizeof(ia32_tss_t)-1, 
				   0, ia32_segdesc_t::tss);
#endif
    
#ifdef CONFIG_TRACEBUFFER
    if (tracebuffer)
        gdt[gdt_idx(IA32_TBS)].set_seg((u32_t)tracebuffer, MB(4)-1, 3,
				       ia32_segdesc_t::data);
    else gdt[gdt_idx(IA32_TBS)].set_seg(0, ~0UL, 3, ia32_segdesc_t::data);
#endif
}

/**
 * activate_gdt: activates the previously set up GDT
 */
static void SECTION(".init.cpu") activate_gdt()
{
    TRACE_INIT("%s\n", __FUNCTION__);

    /* create a temporary GDT descriptor to load the GDTR from */
    ia32_sysdesc_t gdt_desc = {sizeof(gdt), (u32_t)gdt, 0} ;

    asm("lgdt %0		\n"     /* load descriptor table       	*/
	"ljmp	%1,$1f		\n"	/* refetch code segment	descr.	*/
	"1:			\n"	/*   by jumping across segments	*/
	:
	: "m"(gdt_desc), "i" (X86_KCS)
	);

    /* set the segment registers from the freshly installed GDT
       and load the Task Register with the TSS via the GDT */
    asm("mov  %0, %%ds		\n"	/* reload data segment		*/
	"mov  %0, %%es		\n"	/* need valid %es for movs/stos	*/
	"mov  %1, %%ss		\n"	/* reload stack segment		*/
	"mov  %2, %%gs		\n"	/* load UTCB segment		*/
#ifdef CONFIG_TRACEBUFFER
        "mov  %3, %%fs          \n"     /* tracebuffer segment          */
#else
	"mov  %0, %%fs		\n"	/* default without tracebuffer	*/
#endif
	"movl %4, %%eax		\n"
	"ltr  %%ax		\n"	/* load install TSS		*/
	:
	:
#if defined(CONFIG_IA32_SMALL_SPACES)
	"r"(X86_KDS),
#else
	"r"(IA32_UDS),
#endif
	"r"(X86_KDS), "r"(IA32_UTCB), "r"(IA32_TBS), "r"(IA32_TSS)
	: "eax");
}

/**
 * setup_msrs: initializes all model specific registers for CPU
 */
static void setup_msrs()
{
#ifdef CONFIG_IA32_SYSENTER
    /* here we also setup the model specific registers for the syscalls */
    x86_wrmsr(IA32_SYSENTER_CS_MSR, (u32_t)(X86_KCS));
    x86_wrmsr(IA32_SYSENTER_EIP_MSR, (u32_t)(exc_user_sysipc));
#if defined(CONFIG_IO_FLEXPAGES)
    x86_wrmsr(IA32_SYSENTER_ESP_MSR, (u32_t)(TSS_MAPPING) + 4);
#else
    x86_wrmsr(IA32_SYSENTER_ESP_MSR, (u32_t)(&tss) + 4);
#endif
#endif

#if defined(CONFIG_X86_FXSR)
    x86_fpu_t::enable_osfxsr();
#endif
    x86_fpu_t::disable();
    
#if defined(CONFIG_CPU_IA32_K8) 
#if defined(CONFIG_FLUSHFILTER)
    TRACE_INIT("Enabling K8 Flush Filter\n");
    x86_amdhwcr_t::enable_flushfilter();
#else
    TRACE_INIT("Disabling K8 Flush Filter\n");
    x86_amdhwcr_t::disable_flushfilter();
#endif
#endif

#if defined(CONFIG_IA32_PAT)
    // Initialize all PAT values to their default after-reset state,
    // except PAT4 which is set to write-combining (instead of
    // write-back), and PAT7 which is set to write-protected (instead
    // of uncacheable).  This setup allows for safe "fallback" to
    // regular page-based cache settings in the case the PAT bit in
    // the page table is ignored, and it allows for regular behaviour
    // for the PCD and PWT bits if the PAT bit is not set.

    u64_t pats =
	((u64_t) IA32_PAT_WB << (8*0)) | ((u64_t) IA32_PAT_WT << (8*1)) |
	((u64_t) IA32_PAT_UM << (8*2)) | ((u64_t) IA32_PAT_UC << (8*3)) |
	((u64_t) IA32_PAT_WC << (8*4)) | ((u64_t) IA32_PAT_WT << (8*5)) |
	((u64_t) IA32_PAT_UM << (8*6)) | ((u64_t) IA32_PAT_WP << (8*7));

    x86_wrmsr (IA32_CR_PAT_MSR, pats);
#endif
}

/**
 * checks the IA32 features (CPUID) to make sure the processor
 * has all necessary features */
static void SECTION(".init.cpu") check_cpu_features()
{
    u32_t req_features = IA32_FEAT_FPU;
#ifdef CONFIG_IA32_PSE
    req_features |= IA32_FEAT_PSE;
#endif
#ifdef CONFIG_IA32_PGE
    req_features |= IA32_FEAT_PGE;
#endif
#ifdef CONFIG_X86_FXSR
    req_features |= IA32_FEAT_FXSR;
#endif
#ifdef CONFIG_IA32_SYSENTER
    req_features |= IA32_FEAT_SEP;
#endif
#ifdef CONFIG_IOAPIC
    req_features |= IA32_FEAT_APIC;
#endif
    u32_t avail_features = ia32_get_cpu_features();

    if ((req_features & avail_features) != req_features)
    {
	printf("CPU does not support all features (%x) -- halting\n", req_features);
#if defined(CONFIG_VERBOSE_INIT)
	const char* ia32_features[] = {
	    "fpu",  "vme",    "de",   "pse",   "tsc",  "msr", "pae",  "mce",
	    "cx8",  "apic",   "?",    "sep",   "mtrr", "pge", "mca",  "cmov",
	    "pat",  "pse-36", "psn",  "cflsh", "?",    "ds",  "acpi", "mmx",
	    "fxsr", "sse",    "sse2", "ss",    "ht",   "tm",  "ia64", "pbe" };
	for (int i = 0; i < 32; i++)
	    if ((req_features & 1 << i) && (!(avail_features & 1 << i)))
		printf("%s ", ia32_features[i]);
	printf("missing\n");
#endif
	spin_forever();
    }
}

/**
 * init_cpu: initializes the processor
 *
 * this function is called once for each processor to initialize 
 * the processor specific data and registers
 */
static cpuid_t SECTION(".init.cpu") init_cpu()
{
    cpuid_t cpuid = 0;

    /* configure IRQ hardware - local part
     * this has to be done before reading the cpuid since it may change
     * when having one of those broken BIOSes like ServerWorks */
    get_interrupt_ctrl()->init_cpu();

#if defined(CONFIG_SMP)
    cpuid = get_apic_id();
#endif

    /* Allow performance counters for users */
#if defined(CONFIG_PERFMON)

#if defined(CONFIG_TBUF_PERFMON) 
    /* initialize tracebuffer */
    setup_perfmon_cpu(cpuid);
#endif
    
    /* Allow performance counters for users */
    x86_cr4_set(X86_CR4_PCE);  
#endif /* defined(CONFIG_PERFMON) */

    /* initialize the CPU specific mappings */
    get_kernel_space()->init_cpu_mappings(cpuid);

    // call cpu ctors
    call_cpu_ctors();

    tss.setup(X86_KDS);
    setup_gdt(tss, cpuid);
    activate_gdt();

    /* can take exceptions from now on, 
     * idt is initialized via a constructor */
    idt.activate();

    /* activate msrs */
    setup_msrs();

    /* initialize timer - local part */
    get_timer()->init_cpu();

    /* initialize V4 processor info */
    init_processor (cpuid, get_timer()->get_bus_freq(), 
		    get_timer()->get_proc_freq());
    
    return cpuid;
}



/***********************************************************************
 *
 *          system specific initialization and global data 
 *
 **********************************************************************/


/**
 * setup_idt: initializes the interrupt descriptor table
 */


static void __attribute__((unused)) dump_pdir() 
{
    for (int i = 0 ; i < 1024; i++)
	if (init_pdir[i])
	    printf("%d/%x @ %x: %x\n", i, i << 22, 
		   &init_pdir[i], init_pdir[i]);
}

/**
 * init_bootmem: initializes the boot memory
 *
 * At system startup, a fixed amount of kernel memory is allocated
 * to allow basic initialization of the system before sigma0 is up.
 */
static void SECTION(SEC_INIT) init_bootmem()
{
    TRACE_INIT("Initializing boot memory (%p - %p)\n", 
	       start_bootmem, end_bootmem);
    kmem.init(start_bootmem, end_bootmem);

    /* now do reservations */
#if 0
    get_kip()->reserved_mem0.set(start_text_phys, end_text_phys);
    get_kip()->reserved_mem1.set(start_bootmem_phys, end_bootmem_phys);
#else
#warning kernel memory hack

    // Mark the kernel code as reserved
    get_kip()->reserved_mem0.set(start_text_phys, end_bootmem_phys);

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

/**
 * init_meminfo: registers memory section with KIP
 */
static void SECTION(SEC_INIT) init_meminfo()
{
    /* register virtual memory */
    get_kip()->memory_info.insert(memdesc_t::conventional, 
				  true, // virtual memory
				  (void*)USER_AREA_START,
				  (void*)USER_AREA_END);

    // Register kernel code/data as reserved
    get_kip()->memory_info.insert(memdesc_t::reserved,
                                  false,        // not virtual
                                  start_text_phys, end_bootmem_phys);

    // Were we booted via RMGR?
    if (!get_kip()->reserved_mem1.is_empty())
    {
        // Register physical
        get_kip()->memory_info.insert(memdesc_t::conventional,
                                      false,
                                      get_kip()->main_mem.low,
                                      get_kip()->main_mem.high);
        // Register KMEM area as reserved
        if (!get_kip()->reserved_mem1.is_empty())
            get_kip()->memory_info.insert(memdesc_t::reserved,
                                          false,
                                          get_kip()->reserved_mem1.low,
                                          get_kip()->reserved_mem1.high);
        if (!get_kip()->dedicated_mem0.is_empty())
            get_kip()->memory_info.insert(memdesc_t::dedicated,
                                          false,
                                          get_kip()->dedicated_mem0.low,
                                          get_kip()->dedicated_mem0.high);
    }
}


/**
 * init_arch: architecture specific initialization
 *
 * Initializes the kernel debugger, boot memory, and the IDT.
 */
static void SECTION(SEC_INIT) __attribute__((unused)) init_arch() 
{
}

static void SECTION(SEC_INIT) add_more_kmem()
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
            // Map region kernel writable 
            get_kernel_space()->remap_area(
                phys_to_virt(md->low()), md->low(),
                (KERNEL_PAGE_SIZE == IA32_SUPERPAGE_SIZE)
                 ? pgent_t::size_4m
                 : pgent_t::size_4k,
                (md->size() + (KERNEL_PAGE_SIZE-1)) & ~(KERNEL_PAGE_SIZE-1),
                true, true, true);
            // Add it to allocator
            kmem.add(phys_to_virt(md->low()),
                     md->size());
            // We found at least one usable descriptor
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


static void clear_bss()
{
    extern u8_t _bss_start[];
    extern u8_t _bss_end[];
    for (u8_t* p = _bss_start; p < _bss_end; p++)
	*p = 0;
}

/**
 * startup_system: starts up the system
 *
 * precondition: paging is initialized with init_paging
 *
 * The startup happens in two steps
 *   1) all global initializations are performed
 *      this includes initializing necessary devices and
 *      the kernel debugger. The kernel memory allocator
 *      is set up (see init_arch).
 *
 *   2) the boot processor itself is initialized
 */
extern "C" void SECTION(SEC_INIT) startup_system()
{
    clear_bss();

    // allow printing in ctors
    init_console ();

    call_global_ctors();
    call_node_ctors();

    /* system initialization - boot CPU's job */
    {
	/* mention ourself */
	init_hello ();

	/* first thing -- check CPU features */
	check_cpu_features();
	
	/* feed the kernel memory allocator */
	init_bootmem();

	/* initialize the kernel pagetable */
	init_kernel_space();
	
	
	{ /* copied here to catch errors early */
	    tss.setup(X86_KDS);
	    setup_gdt(tss, 0);
	    activate_gdt();
	    idt.activate();
	}

	/* initialize kernel interface page */
	get_kip()->init();

        add_more_kmem();
        
	init_meminfo ();

	/* configure IRQ hardware - global part */
	get_interrupt_ctrl()->init_arch();
	/* initialize timer - global part */
	get_timer()->init_global();

	/* initialize mapping database */
	init_mdb ();

#if defined(CONFIG_IO_FLEXPAGES)
	/* initialize IO address space */
	TRACE_INIT("Initializing IO space\n");
	init_io_space();
#endif

#if defined(CONFIG_TRACEBUFFER)
	/* allocate tracebuffer */
	setup_tracebuffer();
#endif

	/* initialize kernel debugger if any */
	if (get_kip()->kdebug_init)
	    get_kip()->kdebug_init();
    }

#if defined(CONFIG_SMP)
    /* start APs on an SMP + rendezvous */
    {
	TRACE_INIT("Starting application processors (%p->%p)\n", 
		   _start_ap, SMP_STARTUP_ADDRESS);
	
	// aqcuire commence lock before starting any processor
	smp_commence_lock.init (1);

	// boot gdt
	setup_smp_boot_gdt();

	// IPI trap gates
	init_xcpu_handling ();

	// copy startup code to startup page
	for (word_t i = 0; i < IA32_PAGE_SIZE / sizeof(word_t); i++)
	    ((word_t*)SMP_STARTUP_ADDRESS)[i] = ((word_t*)_start_ap)[i];

	/* at this stage we still have our 1:1 mapping at 0 */
	*((volatile unsigned short *) 0x469) = (SMP_STARTUP_ADDRESS >> 4);
	*((volatile unsigned short *) 0x467) = (SMP_STARTUP_ADDRESS) & 0xf;

	local_apic_t<APIC_MAPPINGS> local_apic;

	// make sure we don't try to kick out more CPUs we can handle
	int smp_cpus = 1;

	u8_t apic_id = get_apic_id();
	for (word_t id = 0; id < sizeof(word_t) * 8; id++)
	{
	    if (id == apic_id)
		continue;

	    if ((get_interrupt_ctrl()->get_lapic_map() & (1 << id)) != 0)
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
		local_apic.send_startup_ipi(id, (void(*)(void))SMP_STARTUP_ADDRESS);
#warning VU: time out on AP call in
	    }
	}

    }
#endif

    /* local initialization - all are equal */
    cpuid_t cpuid = init_cpu ();

    TRACE_INIT("%s done\n", __FUNCTION__);

#ifdef CONFIG_SMP
    smp_bp_commence ();
#endif

    get_current_scheduler()->init (true);
    get_current_scheduler()->start (cpuid);

    /* does not return */
    spin_forever(cpuid);
}

#if defined(CONFIG_SMP)
static void smp_ap_commence()
{
    smp_boot_lock.unlock();

    /* finally we sync the time-stamp counters */
    while( smp_commence_lock.is_locked() );

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
extern "C" void SECTION(SEC_INIT) startup_processor()
{
    TRACE_INIT("AP processor is alive\n");
    x86_mmu_t::set_active_pagetable((u32_t)get_kernel_space()->get_pdir());
    TRACE_INIT("AP switched to kernel ptab\n");

    // first thing -- check CPU features
    check_cpu_features();

    /* perform processor local initialization */
    cpuid_t cpuid = init_cpu();
    
    get_current_scheduler()->init (false);
    get_idle_tcb()->notify (smp_ap_commence);
    get_current_scheduler()->start (cpuid);

    spin_forever(cpuid);
}
#endif
/**
 * init_paging:  initializes the startup pagetable
 *               
 * The startup-pagetable contains two regions, a physical 1:1 mapping
 * at virtual address 0 and upwards and a virtual mapping at the 
 * virtual kernel address range.  These mappings are initialized as 
 * 4MB mappings which reduces the needed memory to the page directory only.  
 * After initializing the kernel memory allocator a new kernel pagetable 
 * is set up which can be populated using kmem_alloc etc. 
 */
extern "C" void SECTION(SEC_INIT) init_paging()
{
    /* zero pagetable */
    for (int i = 0; i < 1024; i++)
	init_pdir[i] = 0;

    /* Setup the initial mappings.  The first MAX_KERNEL_MAPPINGS*4MB
       are mapped 1:1.  The same region is also visible at
       KERNEL_OFFSET */
#warning "use virt_to_phys for page mappings"
#if defined(CONFIG_IA32_PSE)
    for (int i = 0; i < MAX_KERNEL_MAPPINGS; i++)
	init_pdir[i] = 
	    init_pdir[(KERNEL_OFFSET >> IA32_PAGEDIR_BITS) + i] = 
	    (i << IA32_PAGEDIR_BITS) | PAGE_ATTRIB_INIT;
#else
    for (int i = 0; i < MAX_KERNEL_MAPPINGS; i++)
    {
        // Fill 2nd-level page table
	for (int j = 0; j<1024; j++) 
	    init_ptable[i][j] = ((i << IA32_PAGEDIR_BITS) |
                                 (j << IA32_PAGE_BITS) |
                                 PAGE_ATTRIB_INIT);
        // Install page table in page directory
	init_pdir[i] = 
	    init_pdir[(KERNEL_OFFSET >> IA32_PAGEDIR_BITS) + i] = 
	    (word_t)(init_ptable[i]) | PAGE_ATTRIB_INIT;
    }
#endif /* CONFIG_IA32_PSE */

    /* now activate the startup pagetable */
#if defined(CONFIG_IA32_PSE)
    x86_mmu_t::enable_super_pages();
#endif
#ifdef CONFIG_IA32_PGE
    x86_mmu_t::enable_global_pages();
#endif
    x86_mmu_t::set_active_pagetable((u32_t)init_pdir);
    x86_mmu_t::enable_paging();
}

