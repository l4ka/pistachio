/*********************************************************************
 *                
 * Copyright (C) 2002-2003,   University of New South Wales
 *                
 * File path:     glue/v4-mips64/init.cc
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
 * $Id: init.cc,v 1.23 2004/05/14 05:07:15 cvansch Exp $
 *                
 ********************************************************************/

#include <mapping.h>

#include INC_API(kernelinterface.h)
#include INC_API(schedule.h)
#include INC_API(space.h)

#include INC_GLUE(memory.h)
#include INC_GLUE(intctrl.h)
#include INC_GLUE(timer.h)
#include INC_ARCH(tlb.h)
#include INC_API(syscalls.h)
#include INC_GLUE(syscalls.h)
#include INC_PLAT(cache.h)
#include INC_API(processor.h)
#ifdef CONFIG_SMP
#include INC_GLUE(smp.h)
#endif

#define BOOTMEM_PAGES (CONFIG_BOOTMEM_PAGES)

#ifdef CONFIG_SMP
word_t plat_cpu_freq[CONFIG_SMP_MAX_CPUS], plat_bus_freq[CONFIG_SMP_MAX_CPUS];
#else
word_t plat_cpu_freq[1], plat_bus_freq[1];
#endif

static void SECTION (".init") dump_info(void)
{
    TRACE_INIT("Kernel configuration:\n");
    TRACE_INIT("\tKTCB area: 0x%lx -> 0x%lx (0x%lx)\n", KTCB_AREA_START, KTCB_AREA_END, KTCB_AREA_SIZE);
}

static void SECTION (".init") init_cpu_cache(void)
{
    cache_t::init_cpu();
}

/*
 * Initialize kernel debugger with initial boot memory, and register
 * kernel memory in the kernel info page.
 */
static void SECTION (".init") init_bootmem (void)
{
    word_t bootmem_size = BOOTMEM_PAGES<<MIPS64_PAGE_BITS;

    extern char _bootstack_top[];
    addr_t start_bootmem = (addr_t)(_bootstack_top);
    addr_t start_bootmem_phys = virt_to_phys(start_bootmem);

    if ((word_t)start_bootmem_phys < (word_t)get_kip()->sigma0.mem_region.high)
	start_bootmem_phys = get_kip()->sigma0.mem_region.high;
    if ((word_t)start_bootmem_phys < (word_t)get_kip()->sigma1.mem_region.high)
	start_bootmem_phys = get_kip()->sigma1.mem_region.high;
    if ((word_t)start_bootmem_phys < (word_t)get_kip()->root_server.mem_region.high)
	start_bootmem_phys = get_kip()->root_server.mem_region.high;

    start_bootmem = phys_to_virt(start_bootmem_phys);
#ifdef CONFIG_SMP
    // Define the area reserved for the exception vectors.
    start_bootmem = phys_to_virt((addr_t)0x1900000);	// XXX - fix this properly
#endif

    addr_t end_bootmem = (addr_t)((word_t)start_bootmem + bootmem_size);
    addr_t end_bootmem_phys = (addr_t)((word_t)start_bootmem_phys + bootmem_size);

    kmem.init (start_bootmem, end_bootmem);

    /* Register reservations in kernel info page. */
    /* feed the kernel memory allocator */

    // Define the user's virtual address space.
    get_kip ()->memory_info.insert( memdesc_t::conventional, true,
	    (addr_t)0, (addr_t)(1ULL<<CONFIG_MIPS64_ADDRESS_BITS));

    /* Register reservations in kernel interface page. */
    get_kip ()->memory_info.insert( memdesc_t::conventional, false, 
	    (addr_t)MIPS64_PAGE_SIZE, addr_align (get_kip()->main_mem.high, KB(4)));
    get_kip ()->memory_info.insert( memdesc_t::reserved, false, 
	    addr_align_up (get_kip()->main_mem.high, KB(4)), (addr_t)~0UL);

    get_kip ()->memory_info.insert (memdesc_t::reserved, false,
		    addr_align (start_text_phys, KB(4)),
		    addr_align_up (end_text_phys, KB (4)));

    get_kip ()->memory_info.insert (memdesc_t::reserved, false,
		    addr_align (start_bootmem_phys, KB(4)),
		    addr_align_up (end_bootmem_phys, KB (4)));

    // Define the area reserved for the exception vectors.
    get_kip ()->memory_info.insert( memdesc_t::reserved, false, 
	    (addr_t)0, (addr_t)MIPS64_PAGE_SIZE);

#ifdef CONFIG_PLAT_ERPCN01
    get_kip ()->dedicated_mem0.set ((addr_t)0x14000000, (addr_t)0x14001000);
    get_kip ()->memory_info.insert( memdesc_t::dedicated, false, 
	    (addr_t)0x14000000, (addr_t)0x14001000);
#elif CONFIG_PLAT_U4600
    get_kip ()->dedicated_mem0.set ((addr_t)0x10000000, (addr_t)0x14001000);
    get_kip ()->memory_info.insert( memdesc_t::dedicated, false, 
	    (addr_t)0x10000000, (addr_t)0x14001000);
    get_kip ()->dedicated_mem1.set ((addr_t)0x1c000000, (addr_t)0x1cc00000);
    get_kip ()->memory_info.insert( memdesc_t::dedicated, false, 
	    (addr_t)0x1c000000, (addr_t)0x1cc00000);
#elif CONFIG_PLAT_SB1
#endif

#ifdef CONFIG_SMP
    // Define the area reserved for the exception vectors.
    get_kip ()->memory_info.insert( memdesc_t::reserved, false, 
	    (addr_t)0x1800000, (addr_t)0x1840000);
#endif
}

#if defined(CONFIG_SMP)
void SECTION (".init") init_processors()
{
    word_t smp_cpu;
    smp_cpu = 1;

    while (mips64_is_processor_available(smp_cpu))
    {
	if (smp_cpu > CONFIG_SMP_MAX_CPUS)
	{
	    printf("found more CPUs than Pistachio supports\n");
	    spin_forever();
	}
	cache_t::flush_cache_all();
	mips64_start_processor(smp_cpu);
	if (! mips64_wait_for_processor (smp_cpu))
	    printf ("Failed to start processor %d\n", smp_cpu);
	smp_cpu ++;
    }
}
#endif

static void SECTION (".init") finalize_cpu_init (word_t cpu_id)
{
    cpuid_t cpuid = cpu_id;

#if defined(CONFIG_SMP)
    // Mark CPU as being active
    mips64_processor_online(cpuid);

    if (cpuid == 0)
	init_processors ();
#endif
    TRACE_INIT("Finalized CPU %d\n", cpuid);
}

/*
 * Setup MIPS CPU
 */
extern "C" void init_cpu(void);

/*
 * Setup the Page tables and ASIDs
 */
extern "C" void SECTION(".init") init_pageing(void)
{
    /* Create and init kernel space */
    init_kernel_space();
}

extern word_t _start_cpu_local;

#if defined(CONFIG_SMP)
void SECTION (".init") init_cpulocal(int cpuid)
{
    int wired = 1, index = 0, i;
    word_t pagemask = (0xf<<12);
    word_t entryhi, entrylo0, entrylo1;

    TRACE_INIT("Initialize CPU Local (%d)\n", cpuid);

    // we make a 16kb cpu local area
    __asm__ __volatile__ (
	"mtc0 %0,"STR(CP0_WIRED)"\n\t"
	:: "r" (wired)
    );

    __asm__ __volatile__ (
	"mtc0 %0,"STR(CP0_PAGEMASK)"\n\t"
	:: "r" (pagemask)
    );

    entryhi = (3ul<<62) | (((word_t)0xffffffffc0000000ul>>13)<<13);
    entrylo0 = ((((0x1800000+0x40000*cpuid)&0xfffffff)>>12)<<6) | (3<<3) | 7; // XXX
    entrylo1 = 1;

    __asm__ __volatile__ (
	"mtc0  %0,"STR(CP0_INDEX)"\n\t"
	"dmtc0 %1,"STR(CP0_ENTRYHI)"\n\t"
	"dmtc0 %2,"STR(CP0_ENTRYLO0)"\n\t"
	"dmtc0 %3,"STR(CP0_ENTRYLO1)"\n\t"
	"nop;nop;nop;\n\t"
	"tlbwi\n\t"
	:: "r" (index), "r" (entryhi), "r" (entrylo0), "r" (entrylo1)
    );

    __asm__ __volatile__ (
	"nop;nop;nop;\n\t"
	"mtc0 $0,"STR(CP0_ENTRYHI)"\n\t"
	"mtc0 %0,"STR(CP0_PAGEMASK)"\n\t"
	:
	: "r" (CONFIG_MIPS64_PAGEMASK_4K)
    );

    /* Zero out region */
    for (i = 0; i < 0x4000; i++)
	*(char*)(0xffffffffc0000000+i) = 0;
}
#endif


/*
 * Setup MIPS Architecture
 */
extern "C" void SECTION(".init") init_arch(void)
{
    init_tlb();

    /* configure IRQ hardware - global part */
    get_interrupt_ctrl()->init_arch();

#if defined(CONFIG_SMP)
    init_cpulocal(0);
#endif

    /* configure IRQ hardware - local part */
    get_interrupt_ctrl()->init_cpu();

    get_asid_cache()->init();
    get_asid_cache()->set_valid(0, CONFIG_MAX_NUM_ASIDS-1);

    init_bootmem();
    
    /* initialize kernel interface page */
    get_kip()->init();

    /* initialise page tables */
    init_pageing();

    /* initialize mapping database */
    init_mdb ();
    
    /* initialize kernel debugger if any */
    if (get_kip()->kdebug_init)
	get_kip()->kdebug_init();

    /* initialize the kernel's timer source */
    get_timer()->init_global();
    get_timer()->init_cpu();

#if defined(CONFIG_SMP)
    init_xcpu_handling (0);
#endif

    init_processor( 0, plat_bus_freq[0]/1000, plat_cpu_freq[0]/1000);
}

extern void init_platform(word_t arg);

/*
 * Entry point from ASM into C kernel
 * Precondition: paging is initialized with init_paging
 */

extern "C" void SECTION(".init") startup_system(word_t a0, word_t a1, word_t a2, word_t a3)
{
    init_cpu();
    init_cpu_cache();
    
    init_platform(a0);

    init_console();

    init_hello();

    init_arch();

    dump_info();

    /* initialize the scheduler */
    get_current_scheduler()->init(true);

    get_idle_tcb ()->notify (finalize_cpu_init, 0);

    /* get the thing going - we should never return */
    get_current_scheduler()->start();
    
    printf("\nShould never get here!\nKernel Halted\n");
    /* make sure we don't fall off the edge */
    spin_forever(1);
}

#if defined(CONFIG_SMP)

extern "C" void SECTION (".init") startup_cpu (cpuid_t cpuid)
{
    init_cpu();
    init_cpu_cache();

    TRACE_INIT("CPU %d startup\n", cpuid);

    init_tlb();
    init_cpulocal(cpuid);
    // Define the area reserved for the exception vectors.
    get_kip ()->memory_info.insert( memdesc_t::reserved, false, 
	    (addr_t)(0x1800000ul+(0x40000*cpuid)), (addr_t)(0x1840000ul+(0x40000*cpuid)));

    get_asid_cache()->init();
    get_asid_cache()->set_valid(0, CONFIG_MAX_NUM_ASIDS-1);

    get_interrupt_ctrl()->init_cpu();
    get_timer()->init_cpu();
    init_xcpu_handling (cpuid);

    init_processor( cpuid, plat_bus_freq[cpuid]/1000, plat_cpu_freq[cpuid]/1000);

    get_current_scheduler()->init (false);

    get_idle_tcb ()->notify (finalize_cpu_init, cpuid);

    get_current_scheduler()->start (cpuid);

    spin_forever(1);
}

#endif
