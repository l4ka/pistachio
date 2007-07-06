/****************************************************************************
 *
 * Copyright (C) 2002, Karlsruhe University
 *
 * File path:	glue/v4-powerpc/init.cc
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
 * $Id: init.cc,v 1.62 2003/12/11 13:09:41 joshua Exp $
 *
 ***************************************************************************/

#include <debug.h>
#include <kmemory.h>
#include <mapping.h>

#include INC_ARCH(page.h)
#include INC_ARCH(phys.h)
#include INC_ARCH(bat.h)
#include INC_ARCH(string.h)
#include INC_ARCH(cache.h)
#include INC_ARCH(ibm750.h)
#include INC_ARCH(pvr.h)

#include INC_PLAT(1275tree.h)
#include INC_PLAT(ofppc.h)

#include INC_API(tcb.h)
#include INC_API(space.h)
#include INC_API(kernelinterface.h)
#include INC_API(schedule.h)
#include INC_API(processor.h)

#include INC_GLUE(pghash.h)
#include INC_GLUE(intctrl.h)
#include INC_GLUE(space.h)
#include INC_GLUE(bat.h)


word_t decrementer_interval = 0;
word_t cpu_count = 1;

#if defined(CONFIG_SMP)
static SECTION(".init.data") DEFINE_SPINLOCK(cpu_start_lock);
static SECTION(".init.data") volatile cpuid_t cpu_start_id;
#endif


// Debug consoles
#if defined(CONFIG_KDB_CONS_OF1275)
extern void init_of1275_console( word_t entry );
#endif
#if defined(CONFIG_KDB_CONS_PSIM_COM)
extern void init_psim_com_console();
#endif

static void fatal( char *msg )
{
    printf( "Fatal error: %s\n", msg );
    while( 1 ) ;
}

SECTION(".init") void timer_init( word_t cpu_hz, word_t bus_hz )
{
    word_t decrementer_hz;

    decrementer_hz = bus_hz / 4;
    decrementer_interval = TIMER_TICK_LENGTH * decrementer_hz / 1000000; 
    TRACE_INIT( "Decrementer %d (KHz), timer tick %d (us), "
	        "decrementer ticks %d\n", decrementer_hz/1000, 
	        TIMER_TICK_LENGTH, decrementer_interval );
}

/*****************************************************************************
 *
 *                            Platform init
 *
 *****************************************************************************/

SECTION(".init") void of1275_map( addr_t low, addr_t high )
    /* Map the position-independent device tree, and install.
     */
{
    word_t size = (word_t)high - (word_t)low;

    addr_t vaddr = get_kernel_space()->map_device( low, size, true );

    get_of1275_tree()->init( (char *)vaddr );
}

SECTION(".init") void of1275_init( kernel_interface_page_t *kip )
    /* Finds and installs the position-independent copy of the
     * OpenFirmware device tree.
     */
{
    // Look for the position-independent copy of the OpenFirmware device tree
    // in the kip's memory descriptors.
    for( word_t i = 0; i < kip->memory_info.get_num_descriptors(); i++ ) 
    {
	memdesc_t *mdesc = kip->memory_info.get_memdesc( i );
	if( (mdesc->type() == OF1275_KIP_TYPE) && 
		(mdesc->subtype() == OF1275_KIP_SUBTYPE) )
	{
	    of1275_map( mdesc->low(), mdesc->high() );
	    return;
	}
    }

    // Not found.  Things won't work, but ...
    printf( "*** Error: the boot loader didn't supply a copy of the\n"
	    "*** Open Firmware device tree!\n" );
    get_of1275_tree()->init( NULL );
}


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

SECTION(".init") static void kip_mem_init( kernel_interface_page_t *kip, word_t bootmem_phys_high )
{
    // Define the user's virtual address space.
    kip->memory_info.insert( memdesc_t::conventional, true,
	    (addr_t)0, (addr_t)KERNEL_OFFSET );
    // Since the system calls are exposed to the users, define
    // a region of the uppger 1gig as accessible to the user.
    kip->memory_info.insert( memdesc_t::shared, true,
	    ofppc_syscall_start(), ofppc_syscall_end() );

    // Define the area reserved for the exception vectors.
    kip->memory_info.insert( memdesc_t::reserved, false, 
	    (addr_t)0, (addr_t)PHYS_START_AVAIL );

    // Define the area reserved for kernel code.
    kip->memory_info.insert( memdesc_t::reserved, false,
	    virt_to_phys(ofppc_start_code()), 
	    virt_to_phys(ofppc_end_code()) );

    // Define the area reserved for kernel data.
    kip->memory_info.insert( memdesc_t::reserved, false,
	    virt_to_phys(ofppc_start_data()), 
	    (addr_t)bootmem_phys_high );
}

SECTION(".init") static void kip_cpu_init( kernel_interface_page_t *kip )
    // Invoked for each processor.
{
    static word_t cpu_khz = 0, bus_khz = 0;

    if( cpu_khz == 0 ) 
    {
    	word_t cpu_hz, bus_hz;
	if( !ofppc_get_cpu_speed(&cpu_hz, &bus_hz) )
	{
	    printf( "Error: unable to obtain the cpu and bus speeds.\n" );
	    cpu_hz = bus_hz = 0;
	}
	if( cpu_hz == 0 ) cpu_hz = 1000000;	// Make a guess
	if( bus_hz == 0 ) bus_hz = 1000000;	// Make a guess

	cpu_khz = cpu_hz / 1000;
	bus_khz = bus_hz / 1000;

	TRACE_INIT( "PowerPC cpu speed: %d (KHz)\n", cpu_khz );
	TRACE_INIT( "Bus speed: %d (KHz)\n", bus_khz );

	timer_init( cpu_hz, bus_hz );
    }

    init_processor( get_current_cpu(), bus_khz, cpu_khz );
}

SECTION(".init") static void kip_sc_init( kernel_interface_page_t *kip )
{
    kip->schedule_syscall	= (syscall_t)_sc_schedule;
    kip->thread_switch_syscall	= (syscall_t)_sc_thread_switch;
    kip->system_clock_syscall	= (syscall_t)_sc_system_clock;
    kip->exchange_registers_syscall = (syscall_t)_sc_xchg_registers;
    kip->unmap_syscall		= (syscall_t)_sc_unmap;
    kip->lipc_syscall		= (syscall_t)_sc_ipc;
    kip->ipc_syscall		= (syscall_t)_sc_ipc;
    kip->memory_control_syscall	= (syscall_t)_sc_memory_ctrl;
    kip->processor_control_syscall = (syscall_t)_sc_processor_ctrl;
    kip->thread_control_syscall	= (syscall_t)_sc_thread_ctrl;
    kip->space_control_syscall	= (syscall_t)_sc_space_ctrl;
    kip->arch_syscall3		= (syscall_t)_sc_perf;
}

/*****************************************************************************
 *
 *                More init functions, run on the boot stack
 *
 *****************************************************************************/
INLINE word_t cpu_phys_area( cpuid_t cpu )
{
    word_t cpu_phys = (word_t)ofppc_start_cpu_phys();
    ASSERT( (cpu_phys & BAT_128K_PAGE_MASK) == cpu_phys );
    return cpu_phys + cpu*KB(128);
}

INLINE word_t cpu_area_size( void )
{
    return (word_t)ofppc_end_cpu_phys() - (word_t)ofppc_start_cpu_phys();
}

#if defined(CONFIG_SMP)
SECTION(".init") static void reclaim_cpu_kmem()
{
    word_t tot = 0;

    // Claim as boot mem the unused physical pages of the 128KB aligned
    // cpu data areas.
    for( cpuid_t cpu = 1; cpu < cpu_count; cpu++ ) {
	word_t start = cpu_phys_area(cpu) + cpu_area_size();
	word_t end = cpu_phys_area(cpu+1);
	word_t size = end - start;
	if( size )
	    kmem.add( phys_to_virt((addr_t)start), size );
	tot += size;
    }

    TRACE_INIT( "Claimed %d bytes for kernel memory.\n", tot );
}
#endif

SECTION(".init") static word_t do_kmem_init()
{
    word_t bootmem_low, bootmem_high;
    word_t tot, size;

    // Allocate some initial boot mem, using pages after the first
    // cpu data area.
    bootmem_low = phys_to_virt( cpu_phys_area(0) + cpu_area_size() );
#if defined(CONFIG_SMP)
    // Allocate the pages up to the next CPU data area.
    bootmem_high = phys_to_virt( cpu_phys_area(1) );
#else
    bootmem_high = bootmem_low + KB(512);
#endif
    kmem.init( (addr_t)bootmem_low, (addr_t)bootmem_high );
    tot = bootmem_high - bootmem_low;

    // Claim the memory used by the exception vector code.
    size = (word_t)ofppc_start_kernel() - phys_to_virt(PHYS_START_AVAIL);
    if( size )
	kmem.add( (addr_t)phys_to_virt(PHYS_START_AVAIL), size );
    tot += size;

    // Claim the memory between the end of the kernel data section and
    // the start of the cpu data page.
    size = cpu_phys_area(0) - (word_t)ofppc_end_data_phys();
    if( size )
	kmem.add( phys_to_virt(ofppc_end_data_phys()), size );
    tot += size;

    TRACE_INIT( "Kernel boot mem: %d bytes\n", tot );
    return virt_to_phys(bootmem_high);
}

/****************************************************************************
 *
 *                  Per-cpu init.
 *
 ****************************************************************************/

static SECTION(".init") void perfmon_init( void )
{
    if( powerpc_version_t::read().is_750() )
    {
	ppc750_mmcr0_t mmcr0;

	mmcr0.raw = 0;
	mmcr0.x.pmc1select = ppc750_mmcr0_t::cycle_cnt;
	mmcr0.x.pmc2select = ppc750_mmcr0_t::instr_complete_cnt;
	ppc_set_mmcr0( mmcr0.raw );

	ppc_set_pmc1( 0 );
	ppc_set_pmc2( 0 );
	ppc_set_pmc3( 0 );
	ppc_set_pmc4( 0 );
    }
}

static SECTION(".init") void timer_start( void )
{
    ppc_set_dec( decrementer_interval );
}

static SECTION(".init") void cpu_init( cpuid_t cpu )
{
#if defined(CONFIG_SMP)
# define CPU_SPILL_SIZE	(CACHE_LINE_SIZE * CONFIG_SMP_MAX_CPUS)
#else
# define CPU_SPILL_SIZE	(CACHE_LINE_SIZE)
#endif
#if (CACHE_LINE_SIZE < 32)
# error "Expecting a cache line size of 32-bytes or larger."
#endif

    /* Give the cpu some extra storage to spill state during an exception.
     * The storage must be safe to access at any time via a physical address.
     * Each cpu requires its own spill area, on non-conflicting cache lines.
     */
    static char spill_area[CPU_SPILL_SIZE] __attribute__ ((aligned(CACHE_LINE_SIZE)));

    char *cpu_spill = &spill_area[ cpu * CACHE_LINE_SIZE ];
    ppc_set_sprg( SPRG_CPU, virt_to_phys((word_t)cpu_spill) );

    // Initialize the time base to 0. */
    ppc_set_tbl( 0 );	// Make sure that tbu won't be upset by a carry.
    ppc_set_tbu( 0 );	// Clear the tbu.
    ppc_set_tbl( 0 );	// Clear the tbl.

    if( powerpc_version_t::read().is_750() )
	ppc750_configure();
    perfmon_init();
    set_fp_lazy_tcb( NULL );
}

#if defined(CONFIG_SMP)

SECTION(".init") static void bat_map_cpu_data( cpuid_t cpu )
{
     ppc_bat_t bat;
     bat.raw.upper = bat.raw.lower = 0;
     bat.x.bepi = KERNEL_CPU_OFFSET >> BAT_BEPI;
     bat.x.bl = BAT_BL_128K;
     bat.x.vs = 1;
     bat.x.brpn = cpu_phys_area(cpu) >> BAT_BRPN;
     bat.x.m = 0;	/* We don't need memory coherency. */
     bat.x.pp = BAT_PP_READ_WRITE;
     ppc_set_cpu_dbat( l, bat.raw.lower );
     ppc_set_cpu_dbat( u, bat.raw.upper );
     isync();
}

/**
 * Continue initializing now that we have a tcb stack.
 */
SECTION(".init") static void finish_cpu_init( void )
{
    // Release the boot stack.
    cpu_start_lock.unlock();

    // Enable recoverable exceptions (for this cpu).
    ppc_set_msr( MSR_KERNEL );

    get_interrupt_ctrl()->bat_map();
    kip_cpu_init( get_kip() );

    // Wait for kernel initialization to quiesce, and then enter the idle
    // loop.  The idle loop will enable external interrupts, but until
    // initialization finishes, the extern int handler is starting CPUs
    // (so we don't want to enable external interrupts).
    while( cpu_start_id != (cpuid_t)-1 ) ;

    timer_start();
    TRACE_INIT( "Going to idle cpu %d.\n", get_current_cpu() );
}

extern "C" SECTION(".init") void l4_powerpc_cpu_start( void )
{
    /* NOTE: do not perform i/o until the page hash is activated!
     * NOTE: do not cause any exceptions that will expect a valid tcb stack!
     *       i/o must not cause exceptions.
     */
    cpuid_t cpu = cpu_start_id;

    bat_map_cpu_data( cpu );
    cpu_init( cpu );

    get_pghash()->get_htab()->bat_map();
    get_pghash()->get_htab()->activate( get_kernel_space()->get_segment_id() );
    /* i/o is now possible. */

    get_current_scheduler()->init( false );
    get_idle_tcb()->notify( finish_cpu_init );
    get_current_scheduler()->start( cpu );

    while( 1 );
}

#endif	/* CONFIG_SMP */

/****************************************************************************
 *
 *                  Init functions which run on the tcb stack
 *
 ****************************************************************************/

static SECTION(".init") void install_extern_int_handler( void )
{
    extern word_t _except_extern_int[], _except_extern_int_end[];
    word_t dst;

    // Copy the external interrupt handler into place, overwriting
    // the IPI init handler.
    dst = KERNEL_OFFSET + PHYS_EXCEPT_START + EXCEPT_OFFSET_EXTERNAL_INT;
    memcpy_cache_flush( (word_t *)dst, (word_t *)_except_extern_int,
	    (word_t)_except_extern_int_end - (word_t)_except_extern_int );
}

#if defined(CONFIG_SMP)
SECTION(".init") static void start_all_cpus( void )
{
    for( cpuid_t cpu = 1; cpu < cpu_count; cpu++ )
    {
	cpu_start_lock.lock();	// Unlocked by the target cpu in l4_powerpc_cpu_start().
	cpu_start_id = cpu;	// cpu_start_id must be protected by the lock.
	get_interrupt_ctrl()->start_new_cpu( cpu );
    }
    cpu_start_lock.lock();	// Wait for last cpu to init.
}
#endif

SECTION(".init") static void finish_api_init( void )
{
    // Enable recoverable exceptions (for this cpu).
    ppc_set_msr( MSR_KERNEL );

    // We now have a valid stack and can handle page faults.
    init_kernel_space();
    get_pghash()->get_htab()->activate( get_kernel_space()->get_segment_id() );

    of1275_init( get_kip() );
#if defined(CONFIG_KDB_CONS_PSIM_COM)
    init_psim_com_console();
#endif

    kip_cpu_init( get_kip() );
    kip_sc_init( get_kip() );

    get_interrupt_ctrl()->init_arch();

#if defined(CONFIG_SMP)
    cpu_count = ofppc_get_cpu_count();
    TRACE_INIT( "Detected %d processors\n", cpu_count );

    reclaim_cpu_kmem();
    start_all_cpus();
#endif

    timer_start();

    install_extern_int_handler();
    get_interrupt_ctrl()->init_cpu(0);
#if defined(CONFIG_SMP)
    for( cpuid_t cpu = 1; cpu < cpu_count; cpu++ )
	get_interrupt_ctrl()->init_cpu( cpu );

    // Initialization is finished.  Let the waiting cpu's enter their idle
    // threads.
    cpu_start_id = (cpuid_t)-1;
#endif
}

/****************************************************************************
 *
 *                  Basic architecture init.
 *
 ****************************************************************************/

static SECTION(".init") void validate_kernel_bats( void )
{
    ppc_bat_t bat;
    word_t size;
    word_t code_bat_start, code_bat_end;
    word_t data_bat_start, data_bat_end;

    /* Retrieve and validate the size of the kernel's code bat.
     */
    bat.raw.lower = ppc_get_kernel_ibat( l );
    bat.raw.upper = ppc_get_kernel_ibat( u );
    
    code_bat_start = bat.x.bepi << BAT_BEPI;
    size = BAT_SMALL_PAGE_SIZE << (32 - count_leading_zeros(bat.x.bl));
    code_bat_end = code_bat_start + size;

    size = (word_t)ofppc_end_code() - (word_t)ofppc_start_kernel();
    if( size > (code_bat_end - code_bat_start) )
	fatal( "The kernel code size exceeds the code BAT size." );

    /* Retrieve and validate the size of the kernel's data bat.
     */
    bat.raw.lower = ppc_get_kernel_dbat( l );
    bat.raw.upper = ppc_get_kernel_dbat( u );

    data_bat_start = bat.x.bepi << BAT_BEPI;
    size = BAT_SMALL_PAGE_SIZE << (32 - count_leading_zeros(bat.x.bl));
    data_bat_end = data_bat_start + size;

    size = (word_t)ofppc_end_data() - (word_t)ofppc_start_data();
    if( size > (data_bat_end - data_bat_start) )
	fatal( "The kernel data size exceeds the data BAT size." );
}

static SECTION(".init") void install_exception_handlers( void )
{
    /* Deactivate machine check exceptions while we install the exception
     * vectors.  We need valid exception vectors to handle machine check
     * exceptions.
     */
    word_t msr = ppc_get_msr();
    msr = MSR_CLR( msr, MSR_ME );
    ppc_set_msr( msr );
    isync();	// Enable the msr change.

    // Copy the exceptions.
    memcpy_cache_flush( (word_t *)(KERNEL_OFFSET + PHYS_EXCEPT_START),
	    (word_t *)ofppc_start_except(),
	    (word_t)ofppc_end_except() - (word_t)ofppc_start_except() );

    /* Reenable machine check exceptions.
     */
    msr = MSR_SET( msr, MSR_ME );
    ppc_set_msr( msr );
    isync();
}

/****************************************************************************
 *
 *                  The kernel's C entry point.
 *
 ****************************************************************************/

extern "C" SECTION(".init") void l4_powerpc_init( word_t r3, word_t r4, word_t r5 )
{
#if defined(CONFIG_KDB_CONS_OF1275)
    init_of1275_console( r5 );
#endif

    /* Primary cpu init.
     */
    validate_kernel_bats();
    install_exception_handlers();
    cpu_init( 0 );

    /* Init kdb.  It should be completely independent of the kernel.
     */
    if( get_kip()->kdebug_init )
	get_kip()->kdebug_init();

    ASSERT( sizeof(utcb_t) == 512 );

    /* Init all of our memory related stuff.
     */
    word_t bootmem_phys_high = do_kmem_init();
    kip_mem_init( get_kip(), bootmem_phys_high );
    if( !get_pghash()->init((word_t)kip_get_phys_mem(get_kip())) )
	fatal( "unable to find a suitable location for the page hash." );
    init_mdb();

    /* Initialize the idle tcb, and push notify frames for starting
     * the idle thread. */
    get_current_scheduler()->init();
    /* Push a notify frame for the second stage of initialization, which
     * executes in the context of the idle thread.  This must execute
     * before the scheduler's notify frames. */
    get_idle_tcb()->notify( finish_api_init );
    get_current_scheduler()->start(); /* Does not return. */
}

