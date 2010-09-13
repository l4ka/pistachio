/*********************************************************************
 *                
 * Copyright (C) 1999-2010,  Karlsruhe University
 * Copyright (C) 2008-2009,  Volkmar Uhlig, IBM Corporation
 *                
 * File path:     glue/v4-powerpc/init.cc
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
 * $Id$
 *                
 ********************************************************************/

#include <debug.h>
#include <kmemory.h>
#include <mapping.h>
#include <generic/ctors.h>
#include <generic/init.h>

#include INC_ARCH(page.h)
#include INC_ARCH(phys.h)
#include INC_ARCH(bat.h)
#include INC_ARCH(string.h)
#include INC_ARCH(cache.h)
#include INC_ARCH(ppc750.h)
#include INC_ARCH(pvr.h)
#include INC_ARCH(swtlb.h)

#ifdef CONFIG_PLAT_OFPPC
#include INC_PLAT(1275tree.h)
#include INC_PLAT(ofppc.h)
#endif
#ifdef CONFIG_PLAT_PPC44X
#include INC_PLAT(fdt.h)
#endif

#include INC_PLAT(platform.h)

#include INC_API(tcb.h)
#include INC_API(space.h)
#include INC_API(kernelinterface.h)
#include INC_API(schedule.h)
#include INC_API(cpu.h)

#include INC_GLUE(pghash.h)
#include INC_GLUE(intctrl.h)
#include INC_GLUE(space.h)
#include INC_GLUE(bat.h)
#include INC_GLUE(memcfg.h)

EXTERN_KMEM_GROUP(kmem_misc);

word_t decrementer_interval = 0;
word_t cpu_count = 1;

#if defined(CONFIG_SMP)
static SECTION(".init.data") DEFINE_SPINLOCK(cpu_start_lock);
static SECTION(".init.data") volatile cpuid_t cpu_start_id;
#endif


// Debug consoles

__attribute__((unused)) 
static void fatal( char *msg ) 
{
    printf( "Fatal error: %s\n", msg );
    while( 1 ) ;
}

SECTION(SEC_INIT) void timer_init( word_t cpu_hz, word_t bus_hz, word_t cpu )
{
    word_t decrementer_hz;

    decrementer_hz = bus_hz / 1;
    decrementer_interval = TIMER_TICK_LENGTH * (decrementer_hz / 1000) / 1000; 
    TRACE_INIT( "\tDecrementer %d KHz, timer tick %d us\n"
	        "\tDecrementer ticks %d (CPU %d)\n", decrementer_hz/1000, 
	        TIMER_TICK_LENGTH, decrementer_interval, cpu );
}

/*****************************************************************************
 *
 *                            Platform init
 *
 *****************************************************************************/
static dtree_t *dtree = NULL;
static word_t dtree_size = 0;

dtree_t *get_dtree()
{
    if (!dtree)
    {
        kernel_interface_page_t *kip = get_kip();
        for( word_t i = 0; i < kip->memory_info.get_num_descriptors(); i++ ) 
        {
            memdesc_t *mdesc = kip->memory_info.get_memdesc( i );
            
            if( (mdesc->type() == memdesc_t::boot_specific) && 
                (mdesc->subtype() == 0xf) )
            {
                dtree = (dtree_t*) mdesc->low();
                dtree_size = mdesc->size();
                break;
            }
        }
    }
    return dtree;
}

SECTION(SEC_INIT) void dtree_remap( kernel_interface_page_t *kip )
{

    dtree_t *dtreemapping;
    paddr_t pdtree = (paddr_t) dtree;
    //TRACEF("dtree %p %d\n", dtree, dtree_size);
    addr_t page = get_kernel_space()->map_device( pdtree, dtree_size, true, pgent_t::cache_standard );
    dtreemapping = (dtree_t*)addr_offset(page, pdtree & (KERNEL_PAGE_SIZE - 1));

    if (!dtreemapping->is_valid())
	panic("Invalid device tree (%p)--can't continue\n");

    TRACE_INIT("Remapping device tree from %p to %p (sz=%x, magic=%x)\n", (word_t) pdtree, 
	       dtreemapping, dtreemapping->size, dtreemapping->magic);
    

#if defined(CONFIG_PLAT_OFPPC)
    get_of1275_tree()->init( dtreemapping );
#endif

#if defined(CONFIG_PLAT_PPC44X)
    dtree = (dtree_t*)kmem.alloc(kmem_misc, dtree_size);
    memcpy(dtree, dtreemapping, dtree_size);
    // XXX: unmap FDT
#endif

}


/*****************************************************************************
 *
 *                            Kip init
 *
 *****************************************************************************/

SECTION(SEC_INIT) addr_t kip_get_phys_mem( kernel_interface_page_t *kip )
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

SECTION(SEC_INIT) static void kip_mem_init( kernel_interface_page_t *kip, word_t bootmem_phys_high )
{
    // Define the user's virtual address space.
    kip->memory_info.insert( memdesc_t::conventional, true,
	    (addr_t)0, (addr_t)KERNEL_OFFSET );
    // Since the system calls are exposed to the users, define
    // a region of the uppger 1gig as accessible to the user.
    kip->memory_info.insert( memdesc_t::shared, true,
	    memcfg_syscall_start(), memcfg_syscall_end() );

    // Define the area reserved for the exception vectors.
    kip->memory_info.insert( memdesc_t::reserved, false, 
	    (addr_t)0, (addr_t)PHYS_START_AVAIL );

    // Define the area reserved for kernel code.
    kip->memory_info.insert( memdesc_t::reserved, false,
	    virt_to_phys(memcfg_start_code()), 
	    virt_to_phys(memcfg_end_code()) );

    // Define the area reserved for kernel data.
    kip->memory_info.insert( memdesc_t::reserved, false,
	    virt_to_phys(memcfg_start_data()), 
	    (addr_t)bootmem_phys_high );
}

SECTION(SEC_INIT) static void kip_cpu_init( kernel_interface_page_t *kip )
    // Invoked for each processor.
{
    static word_t cpu_khz = 0, bus_khz = 0;
    word_t cpu = get_current_cpu();
    
    if( cpu_khz == 0 ) 
    {
    	word_t cpu_hz, bus_hz;
	if( !get_cpu_speed(get_current_cpu(), &cpu_hz, &bus_hz) )
	{
	    printf( "Error: unable to obtain the cpu and bus speeds.\n" );
	    cpu_hz = bus_hz = 0;
	}
	if( cpu_hz == 0 ) cpu_hz = 1000000;	// Make a guess
	if( bus_hz == 0 ) bus_hz = 1000000;	// Make a guess

	cpu_khz = cpu_hz / 1000;
	bus_khz = bus_hz / 1000;

	TRACE_INIT( "\tPowerPC CPU speed: %d KHz (CPU %d)\n", cpu_khz, cpu);
	TRACE_INIT( "\tBus speed: %d KHz (CPU %d)\n", bus_khz, cpu);

	timer_init( cpu_hz, bus_hz, cpu );
    }

    init_cpu( get_current_cpu(), bus_khz, cpu_khz );
}

SECTION(SEC_INIT) static void kip_sc_init( kernel_interface_page_t *kip )
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
#if defined(CONFIG_PPC_MMU_SEGMENTS)
INLINE word_t cpu_phys_area( cpuid_t cpu )
{
    word_t cpu_phys = (word_t)memcfg_start_cpu_phys();
    ASSERT( (cpu_phys & BAT_128K_PAGE_MASK) == cpu_phys );
    return cpu_phys + cpu*KB(128);
}

INLINE word_t cpu_area_size( void )
{
    return (word_t)memcfg_end_cpu_phys() - (word_t)memcfg_start_cpu_phys();
}

#if defined(CONFIG_SMP)
SECTION(SEC_INIT) static void reclaim_cpu_kmem()
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

SECTION(SEC_INIT) static word_t init_bootmem()
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
    size = (word_t)memcfg_start_kernel() - phys_to_virt(PHYS_START_AVAIL);
    if( size )
	kmem.add( (addr_t)phys_to_virt(PHYS_START_AVAIL), size );
    tot += size;

    // Claim the memory between the end of the kernel data section and
    // the start of the cpu data page.
    size = cpu_phys_area(0) - (word_t)memcfg_end_data_phys();
    if( size )
	kmem.add( phys_to_virt(memcfg_end_data_phys()), size );
    tot += size;

    return virt_to_phys(bootmem_high);
}

#elif defined(CONFIG_PPC_MMU_TLB)
#if defined(CONFIG_SMP)
SECTION(SEC_INIT) static void reclaim_cpu_kmem()
{
}
#endif
SECTION(SEC_INIT) static word_t init_bootmem()
{
    addr_t bootmem_low, bootmem_high;
    word_t tot;

    // Allocate some initial boot mem, using pages after the first
    // cpu data area.
    bootmem_low = phys_to_virt( memcfg_end_cpu_phys() );
    bootmem_high = addr_offset(bootmem_low, KB(3584));

    kmem.init( bootmem_low, bootmem_high );
    tot = (word_t)bootmem_high - (word_t)bootmem_low;

    return virt_to_phys((word_t)bootmem_high);
}
#endif


/****************************************************************************
 *
 *                  Per-cpu init.
 *
 ****************************************************************************/

static SECTION(SEC_INIT) void perfmon_init( void )
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

static SECTION(SEC_INIT) void timer_start( void )
{
#ifdef CONFIG_PPC_BOOKE
    ppc_tcr_t tcr;
    tcr.auto_reload = 1;
    tcr.dec_irq_enable = 1;
    tcr.write();
    ppc_set_decar( decrementer_interval );
#endif

    ppc_set_dec( decrementer_interval );
}

static SECTION(SEC_INIT) void cpu_init( cpuid_t cpu )
{
#if defined(CONFIG_SMP)
# define CPU_SPILL_SIZE	(CACHE_LINE_SIZE * CONFIG_SMP_MAX_CPUS)
#else
# define CPU_SPILL_SIZE	(CACHE_LINE_SIZE)
#endif
#if (CACHE_LINE_SIZE < 32)
# error "Expecting a cache line size of 32-bytes or larger."
#endif
    install_exception_handlers(cpu);

    get_kernel_space()->init_cpu_mappings(cpu);

    call_cpu_ctors();

    /* Give the cpu some extra storage to spill state during an exception.
     * The storage must be safe to access at any time via a physical address.
     * Each cpu requires its own spill area, on non-conflicting cache lines.
     */
    static char spill_area[CPU_SPILL_SIZE] __attribute__ ((aligned(CACHE_LINE_SIZE)));

    char *cpu_spill = &spill_area[ cpu * CACHE_LINE_SIZE ];
    ppc_set_sprg( SPRG_CPU, (word_t)virt_to_phys(cpu_spill) );

#if 0
    // Initialize the time base to 0. */
    ppc_set_tbl( 0 );	// Make sure that tbu won't be upset by a carry.
    ppc_set_tbu( 0 );	// Clear the tbu.
    ppc_set_tbl( 0 );	// Clear the tbl.
#else
    /* don't reset time base but rather synchronize the cores */
    ON_CONFIG_SMP(printf("Unsynchronized time base for CPU %d\n", cpu));
#endif

    if( powerpc_version_t::read().is_750() )
	ppc750_configure();
    perfmon_init();
    set_fp_lazy_tcb( NULL );
}

#if defined(CONFIG_SMP)
/**
 * Continue initializing now that we have a tcb stack.
 */
SECTION(SEC_INIT) static void finish_cpu_init( void )
{
    TRACE_INIT("CPU %d initialized--enter wait loop\n", get_current_cpu());

    // Release the boot stack.
    cpu_start_lock.unlock();

    // Enable recoverable exceptions (for this cpu).
    ppc_set_msr( MSR_KERNEL );

    //get_interrupt_ctrl()->map();
    kip_cpu_init( get_kip() );

    // Wait for kernel initialization to quiesce, and then enter the idle
    // loop.  The idle loop will enable external interrupts, but until
    // initialization finishes, the extern int handler is starting CPUs
    // (so we don't want to enable external interrupts).
    while( cpu_start_id != (cpuid_t)-1 ) ;

    timer_start();
    TRACE_INIT( "Going to idle cpu %d.\n", get_current_cpu() );
}

void dump_tlb();
extern "C" void SECTION(SEC_INIT) NORETURN startup_cpu ( cpuid_t cpu )
{
#ifdef CONFIG_PPC_MMU_SEGMENTS
    /* NOTE: do not perform i/o until the page hash is activated!
     * NOTE: do not cause any exceptions that will expect a valid tcb stack!
     *       i/o must not cause exceptions.
     * NOTE: cpu parameter is invalid for non-TLB based systems, fetch
     *	     from cpu_start_id 
     */
    cpu = cpu_start_id;
#endif

    cpu_init( cpu );

#ifdef CONFIG_PPC_MMU_SEGMENTS
    get_pghash()->get_htab()->bat_map();
    get_pghash()->get_htab()->activate( get_kernel_space()->get_segment_id() );
    /* i/o is now possible. */
#elif defined(CONFIG_PPC_MMU_TLB)
    /* kick startup mappings */
    setup_kernel_mappings();
#endif

    get_current_scheduler()->init( false );
    get_idle_tcb()->notify( finish_cpu_init );
    get_current_scheduler()->start( cpu );

    /* not reached */
    while( 1 );
}
#endif	/* CONFIG_SMP */

/****************************************************************************
 *
 *                  Init functions which run on the tcb stack
 *
 ****************************************************************************/

static SECTION(SEC_INIT) void install_extern_int_handler( void )
{
#ifndef CONFIG_PPC_BOOKE
    extern word_t _except_extern_int[], _except_extern_int_end[];
    word_t dst;

    // Copy the external interrupt handler into place, overwriting
    // the IPI init handler.
    dst = KERNEL_OFFSET + PHYS_EXCEPT_START + EXCEPT_OFFSET_EXTERNAL_INT;
    memcpy_cache_flush( (word_t *)dst, (word_t *)_except_extern_int,
	    (word_t)_except_extern_int_end - (word_t)_except_extern_int );
#endif
}

#if defined(CONFIG_SMP)
SECTION(SEC_INIT) static void start_all_cpus( void )
{
    for( cpuid_t cpu = 1; cpu < cpu_count; cpu++ )
    {
	cpu_start_lock.lock();	// Unlocked by the target cpu in startup_cpu
	printf("CPU0: starting CPU %d\n", cpu);
	cpu_start_id = cpu;	// cpu_start_id must be protected by the lock.
	get_interrupt_ctrl()->start_new_cpu( cpu );
        cpu_t::add_cpu(cpu);
    }
    cpu_start_lock.lock();	// Wait for last cpu to init.
}
#endif

SECTION(SEC_INIT) static void finish_api_init( void )
{
    // Enable recoverable exceptions (for this cpu).
    ppc_set_msr( MSR_KERNEL );

    // We now have a valid stack and can handle page faults.
#ifdef CONFIG_PPC_MMU_SEGMENTS
    get_pghash()->get_htab()->activate( get_kernel_space()->get_segment_id() );
#endif

    dtree_remap( get_kip() );

    kip_cpu_init( get_kip() );
    kip_sc_init( get_kip() );

    get_interrupt_ctrl()->init_arch();

    cpu_count = get_cpu_count();
    TRACE_INIT( "Detected %d processors\n", cpu_count );
    cpu_t::add_cpu(0);

#if defined(CONFIG_SMP)
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

#ifdef CONFIG_PPC_MMU_SEGMENTS

/*
 * function validates bat mappings
 */
static SECTION(SEC_INIT) void setup_kernel_mappings( void )
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

    size = (word_t)memcfg_end_code() - (word_t)memcfg_start_kernel();
    if( size > (code_bat_end - code_bat_start) )
	fatal( "The kernel code size exceeds the code BAT size." );

    /* Retrieve and validate the size of the kernel's data bat.
     */
    bat.raw.lower = ppc_get_kernel_dbat( l );
    bat.raw.upper = ppc_get_kernel_dbat( u );

    data_bat_start = bat.x.bepi << BAT_BEPI;
    size = BAT_SMALL_PAGE_SIZE << (32 - count_leading_zeros(bat.x.bl));
    data_bat_end = data_bat_start + size;

    size = (word_t)memcfg_end_data() - (word_t)memcfg_start_data();
    if( size > (data_bat_end - data_bat_start) )
	fatal( "The kernel data size exceeds the data BAT size." );
}

static SECTION(SEC_INIT) void install_exception_handlers( cpuid_t cpu )
{
    if (cpu != 0)
	return;

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
	    (word_t *)memcfg_start_except(),
	    (word_t)memcfg_end_except() - (word_t)memcfg_start_except() );

    /* Reenable machine check exceptions.
     */
    msr = MSR_SET( msr, MSR_ME );
    ppc_set_msr( msr );
    isync();
}
#endif /* CONFIG_PPC_MMU_SEGMENTS */



/****************************************************************************
 *
 *                  The kernel's C entry point.
 *
 ****************************************************************************/
extern "C" void SECTION(SEC_INIT) startup_system ( word_t r3, word_t r4, word_t r5 )
{
    init_console();

    call_global_ctors();
    call_node_ctors();

    /* Primary cpu init. */
    init_hello();

    TRACE_INIT("virtual memory layout:\n"
	       "\tuser area     %wx - %wx\n"
	       "\tcopy area     %wx - %wx\n"
	       "\tkernel area   %wx - %wx\n",
	       USER_AREA_START, USER_AREA_END,
	       COPY_AREA_START, COPY_AREA_END,
	       KERNEL_AREA_START, KERNEL_AREA_END);

    setup_kernel_mappings();
    install_exception_handlers(0);
    
    /* Init all of our memory related stuff.
     */
    word_t bootmem_phys_high = init_bootmem();
    kip_mem_init( get_kip(), bootmem_phys_high );
    
    TRACE_INIT("Initializing kernel space\n");
    space_t::init_kernel_space();

    TRACE_INIT("Initializing TCBs\n");
    tcb_t::init_tcbs();

    TRACE_INIT("Initializing boot CPU\n");
    cpu_init( 0 );

#if defined(CONFIG_TRACEBUFFER)
    /* allocate and setup tracebuffer */
    TRACE_INIT("Initializing Tracebuffer (%dM)\n", TRACEBUFFER_SIZE / (1024 * 1024));
    setup_tracebuffer();
#endif

    TRACE_INIT("Initializing kernel debugger\n");
    if( get_kip()->kdebug_init )
	get_kip()->kdebug_init();

    ASSERT( sizeof(utcb_t) == 512 );

#ifdef CONFIG_PPC_MMU_SEGMENTS
    if( !get_pghash()->init((word_t)kip_get_phys_mem(get_kip())) )
	fatal( "unable to find a suitable location for the page hash." );
#endif
    TRACE_INIT("Initializing mapping database\n");
    init_mdb();


    /* Initialize the idle tcb, and push notify frames for starting
     * the idle thread. */
    get_current_scheduler()->init( true );

    /* Push a notify frame for the second stage of initialization, which
     * executes in the context of the idle thread.  This must execute
     * before the scheduler's notify frames. */
    get_idle_tcb()->notify( finish_api_init );
    get_current_scheduler()->start( 0 ); /* Does not return. */
}
