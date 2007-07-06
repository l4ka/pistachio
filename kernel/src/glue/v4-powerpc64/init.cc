/****************************************************************************
 *
 * Copyright (C) 2003-2004,  National ICT Australia (NICTA)
 *
 * File path:	glue/v4-powerpc64/init.cc
 * Description:	Kernel second stage initialization.
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
 * $Id: init.cc,v 1.13 2006/11/17 17:04:18 skoglund Exp $
 *
 ***************************************************************************/

#include <debug.h>
#include <kmemory.h>
#include <mapping.h>


// Debug consoles
#if defined(CONFIG_KDB_CONS_RTAS)
extern void init_rtas_console();
#endif
extern void init_serial_console();

#include INC_PLAT(prom.h)

#include INC_ARCH(msr.h)
#include INC_ARCH(string.h)
#include INC_ARCH(cache.h)
#include INC_ARCH(ppc64_registers.h)

#include INC_API(tcb.h)
#include INC_API(kernelinterface.h)
#include INC_API(schedule.h)
#include INC_API(processor.h)

#include INC_GLUE(intctrl.h)
#include INC_GLUE(timer.h)
#include INC_ARCH(segment.h)

DECLARE_KMEM_GROUP(kmem_cpu);

/*****************************************************************************
 *
 *                            Kip init
 *
 *****************************************************************************/

SECTION(".init") addr_t kip_get_phys_mem( kernel_interface_page_t *kip )
    /* Search through the kip's memory descriptors for the size
     * of physical memory.  We assume that physical memory always starts at 0.
     */
{
    addr_t max = 0;

    max = kip->main_mem.high;

    for( word_t i = 0; i < kip->memory_info.get_num_descriptors(); i++ ) 
    {
	memdesc_t *mdesc = kip->memory_info.get_memdesc( i );
	if( (mdesc->type() == memdesc_t::conventional)
		&& !mdesc->is_virtual()
		&& (mdesc->high() > max) )
	{
	    max = mdesc->high();
	}
    }

    return max;
}

SECTION(".init") static void kip_mem_init( kernel_interface_page_t *kip )
{
    extern char _start_kernel_phys[];
    extern char _end_kernel_phys[];

    // Define the user's virtual address space.
    kip->memory_info.insert( memdesc_t::conventional, true,
	    (addr_t)0, (addr_t)USER_AREA_END );

    // Define the area reserved for the exception vectors.
    kip->memory_info.insert( memdesc_t::reserved, false, 
	    (addr_t)0, (addr_t)KERNEL_PHYS_START );

    // Define the area reserved for kernel code.
    kip->memory_info.insert( memdesc_t::reserved, false,
	    _start_kernel_phys, _end_kernel_phys );

    // Reserve all other physical memory
    kip->memory_info.insert( memdesc_t::reserved, false, 
	    addr_align_up (kip->main_mem.high, KB(4)), (addr_t)~0ul);

    TRACEF( "Inserted kernel regions\n" );
}

/*****************************************************************************
 *
 *                More init functions, run on the boot stack
 *
 *****************************************************************************/

SECTION(".init")
word_t find_memory_area( word_t size )
{
    word_t phys_start = 0;
    bool busy = true;

    for (; busy; phys_start += POWERPC64_PAGE_SIZE)
    {
	kernel_interface_page_t *kip = get_kip();
    
	word_t phys_end = phys_start + size;

	if ((word_t)get_kip()->sigma0.mem_region.high > phys_start)
	    continue;
	if ((word_t)get_kip()->sigma1.mem_region.high > phys_start)
	    continue;
	if ((word_t)get_kip()->root_server.mem_region.high > phys_start)
	    continue;

	busy = false;
	// Walk through the KIP's memory descriptors and search for any
	// reserved memory regions that collide with our intended memory
	// allocation.
	for( word_t i = 0; i < kip->memory_info.get_num_descriptors(); i++ )
	{
	    memdesc_t *mdesc = kip->memory_info.get_memdesc( i );
	    if( (mdesc->type() == memdesc_t::conventional) || mdesc->is_virtual() )
		continue;

	    word_t low = (word_t)mdesc->low();
	    word_t high = (word_t)mdesc->high();

	    if( (phys_start < low) && (phys_end > high) )
		{ busy = true; break; }
	    if( (phys_start >= low) && (phys_start < high) )
		{ busy = true; break; }
	    if( (phys_end > low) && (phys_end <= high) )
		{ busy = true; break; }
	}
    }
    return phys_start;
}


SECTION(".init") static void do_kmem_init()
{
    word_t bootmem_size = CONFIG_BOOTMEM_PAGES << POWERPC64_PAGE_BITS;

    word_t bootmem_start = phys_to_virt(find_memory_area( bootmem_size ));
    word_t bootmem_end = bootmem_start + bootmem_size;

    kmem.init( (addr_t)bootmem_start, (addr_t)bootmem_end );

    // Define the area reserved for the exception vectors.
    get_kip()->memory_info.insert( memdesc_t::reserved, false, 
	    (addr_t)virt_to_phys( bootmem_start ),
	    (addr_t)virt_to_phys( bootmem_end ) );
}

#if defined(CONFIG_SMP)
# define CPU_SPILL_SIZE	(POWERPC64_CACHE_LINE_SIZE * CONFIG_SMP_MAX_CPUS)
#else
# define CPU_SPILL_SIZE	(POWERPC64_CACHE_LINE_SIZE)
#endif
#if (POWERPC64_CACHE_LINE_SIZE < 64)
# error "Expecting a cache line size of 64-bytes or larger."
#endif
static char spill_area[CPU_SPILL_SIZE] __attribute__ ((aligned(POWERPC64_CACHE_LINE_SIZE)));

static SECTION(".init") void cpu_init( cpuid_t cpu )
{
    /* Give the cpu some extra storage to spill state during an exception.
     * The storage must be safe to access at any time via a physical address.
     * Each cpu requires its own spill area, on non-conflicting cache lines.
     */
    char *cpu_spill = &spill_area[ cpu * POWERPC64_CACHE_LINE_SIZE ];

    for (int i=0; i<CPU_SPILL_SIZE; i++)
	spill_area[i] = 0;

    ppc64_set_sprg( SPRG_LOCAL, (word_t)cpu_spill );
    ppc64_set_sprg( SPRG_TCB, (word_t)get_idle_tcb());
}

static SECTION(".init") void cpulocal_init( cpuid_t cpu )
{
    extern char _start_cpu_[];
    extern char _end_cpu_[];
    addr_t cpu_area = kmem.alloc( kmem_cpu, (word_t)
		    addr_align_up( (addr_t)(_end_cpu_ - _start_cpu_), POWERPC64_PAGE_SIZE ) );

    pgent_t pg;

    /* We assume cpu local area is less than 256 MB */
    /* XXX - this must change for SMP - assembler handled */
    segment_t::insert_entry( get_kernel_space(),
		    get_kernel_space()->get_vsid( (addr_t)KERNEL_CPU_OFFSET ),
		    ESID( (word_t)KERNEL_CPU_OFFSET ), false );

    for ( word_t i=0; i < (word_t)addr_align_up(
		(addr_t)( _end_cpu_ - _start_cpu_), POWERPC64_PAGE_SIZE );
		i+= POWERPC64_PAGE_SIZE )
    {
	/* Create a dummy page table entry */
	pg.set_entry( get_kernel_space(), pgent_t::size_4k,
			virt_to_phys((addr_t)((word_t)cpu_area + i)),
		      6, (pgent_t::l4default), true );
	/* Insert the kernel mapping, bolted */
	get_pghash()->insert_mapping( get_kernel_space(),
			(addr_t)(KERNEL_CPU_OFFSET + i),
			&pg, pgent_t::size_4k, true );
    }

    TRACEF( "Allocated cpu(%d) area %p\n", cpu, cpu_area );
}


SECTION(".init") static void finish_api_init( void )
{
    get_timer()->init_global();

#ifdef CONFIG_SMP
    init_processor( boot_cpu, boot_buskhz, boot_cpukhz );
#else
    init_processor( 0, boot_buskhz, boot_cpukhz );
#endif

    get_interrupt_ctrl()->init_arch();

    get_timer()->init_cpu();
}

static SECTION(".init") void install_exception_handlers( void )
{
    /* Deactivate machine check exceptions while we install the exception
     * vectors.  We need valid exception vectors to handle machine check
     * exceptions.
     */
    word_t msr = ppc64_get_msr();
    msr = msr & (~MSR_ME);
    ppc64_set_msr( msr );
    isync();	// Enable the msr change.

    extern char _except_start_[];
    extern char _except_end_[];

    memcpy_cache_flush( (word_t *)(KERNEL_OFFSET),
	    (word_t *)_except_start_,
	    (word_t)_except_end_ - (word_t)_except_start_ );
 
    /* Reenable machine check exceptions.
     */
    msr = MSR_KERNEL_MODE;	/* Now we can be in real kernel mode */
    ppc64_set_msr( msr );
    isync();
}

extern void init_plat (word_t);

extern void early_kernel_map ();

/****************************************************************************
 *
 *                  The kernel's C entry point.
 *
 ****************************************************************************/

extern "C" SECTION(".init") void start_kernel( word_t r3, word_t r4, word_t ofentry )
{
    /* We are called either real or virtual mode :(
     * First thing is to initialise the platform and map kernel data
     * to enable relocated operation
     */
    init_plat( ofentry );

    kernel_interface_page_t *kip = PTRRELOC(get_kip());

    /* Setup the Hash Page Table */
    if( !PTRRELOC(get_pghash())->init((word_t)kip_get_phys_mem(kip)) )
	prom_exit( "unable to find a suitable location for the page hash." );

    prom_puts( "Inserting kernel bolted hash table entry\n\r" );
    early_kernel_map();

#if defined(CONFIG_KDB_CONS_RTAS)
    init_rtas_console();
#endif

    /* Install the Hash Page Table and jump to virtmode_call() */
    PTRRELOC(get_pghash())->get_htab()->activate();

    /* We should never get here */
    while (1);
}


void switch_console( const char *name );
extern "C" SECTION(".init") void virtmode_call(void)
{
    /* --- Running in MAPPED VIRTUAL MODE from here! --- */

    cpu_init( 0 );

    install_exception_handlers();

    /* Init kdb.  It should be completely independent of the kernel.
     */
    if( get_kip()->kdebug_init )
	get_kip()->kdebug_init();

    do_kmem_init();
    kip_mem_init( get_kip() );

    cpulocal_init( 0 );

#if defined(CONFIG_KDB_CONS_RTAS)
    printf( "L4 - Pistachio  \n" );
    printf( "PPC64 %dMHz   ", boot_cpukhz/1000 );
#endif

#ifdef CONFIG_DEBUG
    init_serial_console();
#endif

    init_hello();

    /* initialize kernel interface page syscalls */
    get_kip()->init();

    init_mdb();
    init_kernel_space ();

    /* Initialize the idle tcb, and push notify frames for starting
     * the idle thread. */
    get_current_scheduler()->init();

    /* Push a notify frame for the second stage of initialization, which
     * executes in the context of the idle thread.  This must execute
     * before the scheduler's notify frames. */
    get_idle_tcb()->notify( finish_api_init );
    get_current_scheduler()->start(); /* Does not return. */

    /* we should never get here! */
    while (1);
}

