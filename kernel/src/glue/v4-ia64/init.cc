/*********************************************************************
 *                
 * Copyright (C) 2002-2004,  Karlsruhe University
 *                
 * File path:     glue/v4-ia64/init.cc
 * Description:   V4 initialization for IA-64
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
 * $Id: init.cc,v 1.45 2004/03/30 17:33:02 skoglund Exp $
 *                
 ********************************************************************/
#include <debug.h>
#include <kmemory.h>
#include <tcb_layout.h>
#include <mapping.h>

#include INC_API(kernelinterface.h)
#include INC_API(schedule.h)
#include INC_API(tcb.h)
#include INC_API(processor.h)
#include INC_API(procdesc.h)

#include INC_ARCH(pal.h)
#include INC_ARCH(sal.h)
#include INC_ARCH(tlb.h)
#include INC_ARCH(trmap.h)
#include INC_ARCH(rr.h)
#include INC_ARCH(itc_timer.h)
#include INC_ARCH(ia64.h)

#include INC_GLUE(smp.h)
#include INC_GLUE(memory.h)
#include INC_GLUE(registers.h)
#include INC_GLUE(intctrl.h)
#include INC_GLUE(smp.h)

#include INC_PLAT(bootinfo.h)

EXTERN_KMEM_GROUP (kmem_misc);

// Number of virtual address bits implemented in the CPU
word_t ia64_num_vaddr_bits;


// Min-state save area for Machine Checks
addr_t ia64_mc_save_area;


/**
 * Tick rate of platform clock.
 */
word_t platform_freq;


void init_efi (addr_t, word_t, word_t, u32_t, addr_t);
void init_sal (void);
void init_pal (void);
void ipanic (const char * str);
void purge_complete_tc (void);


static void cmc_handler (word_t irq, ia64_exception_context_t * frame);


#if defined(CONFIG_SMP)

extern "C" void _start_ap (void);
word_t ap_wakeup_vector = 0xf0;


/**
 * Initialize application processors
 */
static bool SECTION (".init")
init_application_processors (void)
{
    TRACE_INIT ("Initializing application processors\n");

    init_xcpu_handling ();

    /* Set the rendezvous function */
    void (*rendez_func)(void) = _start_ap;
    ((word_t *) rendez_func)[0] = virt_to_phys (((word_t *) rendez_func)[0]);
    ((word_t *) rendez_func)[1] = virt_to_phys (((word_t *) rendez_func)[1]);
 
    sal_status_e stat;
    if ((stat = sal_set_boot_rendez (rendez_func, 256)) != SAL_OK)
    {
	TRACE_INIT ("Error: sal_set_boot_rendez() => %ld\n", (long) stat);
	return false;
    }

    for (cpuid_t cpu = 1; cpu <= CONFIG_SMP_MAX_CPUS; cpu++)
    {
	if (! smp_is_processor_available (cpu))
	    continue;

	smp_startup_processor (cpu, ap_wakeup_vector);
	if (! smp_wait_for_processor (cpu))
	    printf ("Failed to start processor %d\n", cpu);
    }

    return true;
}
#endif /* CONFIG_SMP */

/**
 * Perform processor local initialization.  Invoked once for each
 * processor in the system.
 */
static void SECTION (".init")
init_cpu (cpuid_t cpuid)
{
    get_kernel_space ()->init_cpu_mappings (cpuid);

    // Purge all translation caches

    purge_complete_tc ();

    // Mask Perf Mon, Machine Check, LINT0, and LINT1 signals

    cr_ivec_t ivec = 0;
    ivec.m = 1;

    cr_set_pmv (ivec);
    cr_set_lrr0 (ivec);
    cr_set_lrr1 (ivec);

    ivec.m = 0;
    ivec.vector = 2;
    cr_set_cmcv (ivec);
    get_interrupt_ctrl ()->register_handler (2, cmc_handler);

    ia64_srlz_d ();

    // Calculate ITC tick rate

    word_t proc, bus, itc, itc_freq, proc_freq;
    pal_status_e pstatus;

    if ((pstatus = pal_freq_ratios (&proc, &bus, &itc)) != PAL_OK)
	panic ("pal_freq_ratios() => %d\n", (long) pstatus);

    itc_freq = platform_freq * (itc >> 32) / (itc & 0xffffffff);
    proc_freq = platform_freq * (itc >> 32) / (itc & 0xffffffff);
    TRACE_INIT ("ITC freq: %dMHz\n", itc_freq / 1000000);

    procdesc_t * pdesc = get_kip ()->processor_info.get_procdesc (cpuid);
    pdesc->arch1 = itc_freq;

    // Enable periodic timer

    get_itc_ptimer ()->init_cpulocal (IVEC_TIMER);
    get_itc_ptimer ()->setup (itc_freq / (1000000 / get_timer_tick_length ()));

    cr_set_tpr (cr_tpr_t::some_enabled (14));

    // Register processor with KIP

    init_processor (cpuid, platform_freq / 1000, proc_freq / 1000);
}


DEFINE_SPINLOCK (initial_stack_lock);

static void SECTION (".init") finalize_cpu_init (word_t cpu_id)
{
    cpuid_t cpuid = cpu_id;
    initial_stack_lock.unlock ();

#if defined(CONFIG_SMP)
    // Mark CPU as being active
    smp_processor_online (cpuid);

    if (cpuid == 0 && ! init_application_processors ())
	printf ("Error while initializing MP system --- "
		"running with single CPU\n");
#endif

    printf ("Finalized CPU %d\n", cpuid);
}


/**
 * Initialize kernel debugger with initial boot memory, and register
 * kernel memory in the kernel info page.
 */
static void SECTION (".init")
init_bootmem (void)
{
    kmem.init (start_bootmem, end_bootmem);

    /* Register reservations in kernel interface page. */
    get_kip ()->reserved_mem0.set (addr_align (start_text_phys, KB (4)),
				   addr_align_up (end_text_phys, KB (4)));
    get_kip ()->reserved_mem1.set (addr_align (start_bootmem_phys, KB (4)),
				   addr_align_up (end_bootmem_phys, KB (4)));

    get_kip ()->memory_info.insert (memdesc_t::reserved, false,
				    addr_align (start_text_phys, KB (4)),
				    addr_align_up (end_text_phys, KB (4)));
    get_kip ()->memory_info.insert (memdesc_t::reserved, false,
				    addr_align (start_bootmem_phys, KB (4)),
				    addr_align_up (end_bootmem_phys, KB (4)));
}

static void cmc_handler (word_t irq, ia64_exception_context_t * frame)
{
    printf ("Corrected Machine Check @ %p\n", frame->iip);
    enter_kdebug ("corrected machine check");
}


/**
 * Register a min-state save area for Machine Checks.
 */
static void SECTION (".init")
init_mc (void)
{
    return;

    ia64_mc_save_area = kmem.alloc (kmem_misc, KB (4));

    addr_t addr = virt_to_phys (ia64_mc_save_area);
    TRACE_INIT ("Registering Machine Check min-state save area @ %p\n", addr);
    addr = (addr_t) ((word_t) addr | (1UL << 63));

    pal_status_e status;
    if ((status = pal_mc_register_mem (addr)) != PAL_OK)
	printf ("Error: PAL_MC_REGISTER_MEM => %d\n", status);
}	


/**
 * Set up a pinned translation for the system call stubs.
 */
static void SECTION (".init")
setup_syscall_stubs (void)
{
    extern word_t _syscall_stubs, _syscall_stubs_phys;

    translation_t tr (true, translation_t::write_back, true, true, 0,
		      translation_t::xp_rx, &_syscall_stubs_phys, true);

    itrmap.add_map (tr, &_syscall_stubs, 14, 0);

    // Also add a DTR mapping (for debugging reasons)
    tr.set (true, translation_t::write_back, true, true, 0,
	    translation_t::rwx, &_syscall_stubs_phys, true);
    dtrmap.add_map (tr, &_syscall_stubs, 14, 0);
}


/**
 * Set up region registers for physical memory access.
 */
static void SECTION (".init")
init_region_regs (void)
{
    rr_t rr;
    rr.set (false, RID_PHYS_MEM, 28);
    rr.put (7);
    ia64_srlz_d ();
    rr.set (false, RID_PHYS_MEM_UC, 28);
    rr.put (6);
    ia64_srlz_d ();

    if ((KTCB_AREA_START >> 61) != 5)
	ipanic ("TCB region != 5");

    // For the boot CPU the region regiser for TCB area is set in
    // init_kernel_space().
    extern space_t * kernel_space;

#if defined(__GNUC__) && __GNUC__ >= 3 && __GNUC_MINOR__ >= 3
    space_t * ks = kernel_space;
#else
    // We're not running in virtual mode yet, so use the physical
    // address when grabbing the kernel space.
    space_t * ks = *(virt_to_phys (&kernel_space));
#endif

    if (ks != NULL)
    {
	rr.set (false, ks->get_region_id (), KTCB_ALLOC_SIZE_SHIFT);
	rr.put (5);
	ia64_srlz_d ();
    }
}


/**
 * Perform architecture (IA-64) specific initialization.
 * Initialization is global to all processors in the system.
 */
static void SECTION (".init")
init_arch (void)
{
    init_sal ();
    init_pal ();

    init_bootmem ();
    init_mc ();

    // Initialize interrupt controller

    get_interrupt_ctrl ()->init_arch ();

    // Check if TCB area is inside implemented virtual address range

    pal_vm_summary_t info;
    pal_status_e pstatus;
    if ((pstatus = pal_vm_summary (&info)) != PAL_OK)
	panic ("pal_vm_summary() => %d", pstatus);

    ia64_num_vaddr_bits = info.impl_virtual_addtress_msb;
    get_kip ()->memory_info.insert (memdesc_t::conventional, true, NULL,
				    (addr_t) (1UL << ia64_num_vaddr_bits));

    if ((KTCB_AREA_END & ~(7UL << 61)) > 1UL << ia64_num_vaddr_bits)
	panic ("TCB area in unimplemented virtual address range "
	       "(max %d bits).\n", ia64_num_vaddr_bits);

    // Confirm that SP to RSE offset is valid

    if ((KTCB_SIZE - IA64_SP_TO_RSE_OFFSET <= OFS_TCB_KERNEL_STACK) ||
	(IA64_SP_TO_RSE_OFFSET & 0x1ff) != 0)
	panic ("Invalid IA64_SP_TO_RSE_OFFSET (0x%x)\n",
	       IA64_SP_TO_RSE_OFFSET);

    // Check for UNAT collision of sp in exception frame

    if (((offsetof (ia64_exception_context_t, r12) >> 3) & 63 >=
	 (offsetof (ia64_exception_context_t, r1)  >> 3) & 63) &&
	((offsetof (ia64_exception_context_t, r12) >> 3) & 63 <=
	 (offsetof (ia64_exception_context_t, r31) >> 3) & 63))
	panic ("UNAT collision for sp in exception frame (position=%d)\n",
	       offsetof (ia64_exception_context_t, r12) >> 3);

    // Check that FP registers in exception frame are properly aligned

    if (((sizeof (ia64_exception_context_t) -
	  offsetof (ia64_exception_context_t, f6)) & 0xf) != 0)
	panic ("FP registers in exception frame not properly aligned\n");

    // Check that exception frame is multiple of 16

    if ((sizeof (ia64_exception_context_t) & 0xf) != 0)
	panic ("Exception frame not multiple of 16\n");

    // Get platform tick rate

    word_t drift;
    sal_status_e sstatus;

    if ((sstatus = sal_freq_base (sal_freq_base_t::platform,
				  &platform_freq, &drift)))
	panic ("sal_freq_base() => %d\n", (long) sstatus);

    // Initialize kernel debugger

    if (get_kip ()->kdebug_init)
	get_kip ()->kdebug_init ();
}


extern "C" void SECTION (".init")
startup_system (efi_bootinfo_t * bootinfo)
{
    if (! bootinfo || ! bootinfo->is_valid ())
	ipanic ("Invalid bootinfo parameter\n");

    // We are running on valid kernel stacks
    asm volatile ("mov " MKSTR (r_KERNEL_STACK_COUNTER) "=2;;");

    // Create a physical "TCB location" for the init stack.  Needed in
    // order to save exception context in the right memory location.
    extern word_t _initial_stack;
    asm volatile ("mov " MKSTR (r_PHYS_TCB_ADDR) "=%0"
		  :
		  :"r" (virt_to_phys (addr_to_tcb (&_initial_stack))));

    init_region_regs ();

    // Map first 256 MB for instruction and data
    translation_t tr (1, translation_t::write_back, 1, 1,
		      0, translation_t::rwx, (addr_t) 0, 0);

    tr.put_itr (0, phys_to_virt ((addr_t) 0), HUGE_PGSIZE, 0);
    tr.put_dtr (0, phys_to_virt ((addr_t) 0), HUGE_PGSIZE, 0);

    // We are now able to run in virtual mode
    ia64_switch_to_virt ();
    
    // Initialize translation mappings
    itrmap.init ();
    dtrmap.init ();

    // We already added the mappings.  Just put them into the TR maps.
    itrmap.add_map (tr, phys_to_virt ((addr_t) 0), HUGE_PGSIZE, 0, false);
    dtrmap.add_map (tr, phys_to_virt ((addr_t) 0), HUGE_PGSIZE, 0, false);
	
    // Map next 256 MB for data only
    tr.set (1, translation_t::write_back, 1, 1,
	    0, translation_t::rwx, (addr_t) MB (256), 0);
    dtrmap.add_map (tr, phys_to_virt ((addr_t) MB (256)), HUGE_PGSIZE, 0);

    // Map first 256 MB of memory as unchachable data
    tr.set (1, translation_t::uncacheable, 1, 1,
	    0, translation_t::rwx, (addr_t) 0, 0);
    dtrmap.add_map (tr, phys_to_virt_uc ((addr_t) 0), HUGE_PGSIZE, 0);

    // Create a local copy of the bootinfo structure
    static efi_bootinfo_t bootinfo_copy;

    // We might have to create a temporary mapping for accessing the
    // original bootinfo structure
    bootinfo = phys_to_virt (bootinfo);
    if (! dtrmap.is_mapped (bootinfo))
    {
	tr.set (1, translation_t::write_back, 1, 1,
		0, translation_t::rwx, virt_to_phys (bootinfo), 0);
	dtrmap.add_map (tr, bootinfo, HUGE_PGSIZE, 0);

	bootinfo_copy = *bootinfo;

	dtrmap.free_map (bootinfo);
    }
    else
	bootinfo_copy = *bootinfo;

    bootinfo = &bootinfo_copy;

    if (bootinfo->memmap)
	bootinfo->memmap = phys_to_virt (bootinfo->memmap);
    if (bootinfo->systab)
	bootinfo->systab = phys_to_virt (bootinfo->systab);

    init_efi ((addr_t) bootinfo->memmap, (word_t) bootinfo->memmap_size,
	      (word_t) bootinfo->memdesc_size, bootinfo->memdesc_version,
	      (addr_t) bootinfo->systab);

    init_console ();
    init_hello ();

    /* PRINTING IS NOW ALLOWED. */

    init_arch ();
    setup_syscall_stubs ();
    init_cpu (0);

    get_itc_ptimer ()->init_global (IVEC_TIMER);

    get_kip ()->init ();

    init_mdb ();
    init_kernel_space ();

    get_current_scheduler()->init (true);
    get_idle_tcb ()->notify (finalize_cpu_init, 0);
    get_current_scheduler()->start ();

    /* NOTREACHED */
    for (;;)
	enter_kdebug ("spin");
}


/**
 * Initialize all translation registers.
 */
extern "C" int sprintf(char* obuf, const char* format, ...);
void SECTION (".init") init_translations (void)
{
    // Copy instruction translations
    for (word_t n = 1; n < itrmap.num_entries (); n++)
    {
	if (itrmap.is_free[n])
	    continue;
	itrmap.put (n, 0);
    }

    // Copy data translations
    for (word_t n = 1; n < dtrmap.num_entries (); n++)
    {
	if (dtrmap.is_free[n])
	    continue;
	dtrmap.put (n, 0);
    }
}

#if defined(CONFIG_SMP)

extern "C" void SECTION (".init") startup_ap (cpuid_t cpuid)
{
    // We are running on valid kernel stacks
    asm volatile ("mov " MKSTR (r_KERNEL_STACK_COUNTER) "=2;;");

    init_region_regs ();

    // Map first 256 MB for instruction and data
    translation_t tr (1, translation_t::write_back, 1, 1,
		      0, translation_t::rwx, (addr_t) 0, 0);
    
    tr.put_itr (0, phys_to_virt ((addr_t) 0), HUGE_PGSIZE, 0);
    tr.put_dtr (0, phys_to_virt ((addr_t) 0), HUGE_PGSIZE, 0);

    // We are now able to run in virtual mode
    ia64_switch_to_virt ();

    init_translations ();
    init_cpu (cpuid);

    get_current_scheduler()->init (false);
    get_idle_tcb ()->notify (finalize_cpu_init, cpuid);
    get_current_scheduler()->start (cpuid);

    for (;;);
}

#endif
