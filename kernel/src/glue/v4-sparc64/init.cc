/*********************************************************************
 *                
 * Copyright (C) 2003-2004, University of New South Wales
 *                
 * File path:    glue/v4-sparc64/init.cc
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
 * $Id: init.cc,v 1.5 2004/05/21 02:34:56 philipd Exp $
 *                
 ********************************************************************/

#include <mapping.h>

#include INC_API(schedule.h)
#include INC_API(processor.h)
#include INC_API(kernelinterface.h)
#include INC_CPU(mmu.h)
#include INC_GLUE_API_ARCH(space.h)
#include INC_GLUE_API_ARCH(timer.h)

extern bool SECTION(".init") init_bootmem(void);

/**
 *  trap_kdebug()
 *  used by unused and unimplemented trap handlers to entry KDB.
 */
extern "C" void
trap_kdebug(char * reason, word_t pc)
{
    tl_t     tl_old;
    tl_t     tl_new;
    tpc_t    tpc;
    tnpc_t   tnpc;
    tt_t     tt;
    tstate_t tstate;
    ver_t    ver;

    printf("trap_kdebug: Unimplemented trap 0x%lx\n", pc);

    tl_old.get();                                 /* Save the current TL.   */
    ver.get();                                    /* We need rev.maxtl.     */
    for(u8_t i = 1; i <= ver.ver.maxtl; i++) {    /* For every TL.          */
	tl_new.tl = i;                            /* Set TL we want.        */
	tl_new.set();
	tpc.get();
	tnpc.get();
	tt.get();
	tstate.get();                             /* Get the trap state.    */
	tl_new.print(), printf(" "), tpc.print(), /* Print the results.     */
	printf("\t"), tnpc.print(), printf("\t"),
	tt.print(), printf(" "), tstate.print();
    }
    tl_old.set();                                 /* Restore original TL.   */

    enter_kdebug(reason);
}

/**
 *  init_rest_bootmem()
 *  Sets up kernel allocated memory past the pinned bootpage.
 */
void SECTION(".init")
init_rest_bootmem(void)
{
    extern word_t _start_text[];
    word_t bootmem_size = CONFIG_BOOTMEM_PAGES << SPARC64_PAGE_BITS;

    /**
     *  Feed additional memory to kernel memory allocator.
     */

    addr_t rest_bootmem_start = (addr_t)((word_t)_start_text + BOOTPAGE_SIZE);
    word_t rest_bootmem_size = bootmem_size - BOOTPAGE_SIZE;
    //kmem.add(rest_bootmem_start, rest_bootmem_size); // need to map it.

    /**
     *  Add kernel reserved memory descriptors.
     */

    tlb_t tlb_entry;
    tlb_entry.get(TLB_KERNEL_LOCKED, tlb_t::d_tlb); // Get kernel TLB entry.
    addr_t bootmem_paddr_start = tlb_entry.get_pa();
    addr_t bootmem_paddr_end = addr_offset(bootmem_paddr_start,  bootmem_size);

    get_kip()->memory_info.insert(memdesc_t::reserved, false,
				  bootmem_paddr_start, bootmem_paddr_end);

} // init_rest_bootmem()

/**
 *  init_paging()
 *  ...
 */
void SECTION(".init")
init_paging(void)
{
    /**
     *  Set up memory descriptors for virtual memory avaliable to user.
     */

#if (SPARC64_VIRTUAL_ADDRESS_BITS < 64)

    get_kip()->memory_info.insert(memdesc_t::conventional, true,
				  (addr_t)USER_AREA_LOWER_START,
				  (addr_t)(USER_AREA_LOWER_LIMIT + 1));
    get_kip()->memory_info.insert(memdesc_t::conventional, true,
				  (addr_t)USER_AREA_UPPER_START,
				  (addr_t)(USER_AREA_UPPER_LIMIT + 1));

#else /* SPARC64_VIRTUAL_ADDRESS_BITS == 64 */

    get_kip()->memory_info.insert(memdesc_t::conventional, true,
				  (addr_t)USER_AREA_START,
				  (addr_t)(USER_AREA_LIMIT + 1));

#endif /* (SPARC64_VIRTUAL_ADDRESS_BITS < 64) */

    space_t::init_kernel_space();

} // init_paging()

/**
 *  init_arch()
 *  Initialises architecture dependent parts of the system.
 */
void SECTION(".init")
init_arch(void)
{
    bool more_bootmem;

    more_bootmem = init_bootmem();

    /* Initialise kernel interface page */
    get_kip()->init();

    /* Initialise kernel debugger if any */
    if (get_kip()->kdebug_init)
	get_kip()->kdebug_init();

    /* Initialise page tables */
    init_paging();

    if(more_bootmem) {
	init_rest_bootmem();
    }

    /* Initialise mapping database */
    init_mdb();

    /* Initialise the kernel's timer source */
    get_timer()->init_global();
    get_timer()->init_cpu();

    /* Initialise main cpu. */
    extern word_t plat_bus_freq;
    extern word_t plat_cpu_freq;

    init_processor(0, plat_bus_freq/1000, plat_cpu_freq/1000);

} // init_arch()

/**
 * Entry point from ASM into C kernel
 * Precondition: BOOTPAGE (supperpage) is mapped in.
 */
extern "C" void SECTION(".init")
startup_system()
{
    init_console();

    init_hello();

    init_arch();

    /* initialize the scheduler */
    get_current_scheduler()->init();

    /* start the scheduler - this should never return */
    get_current_scheduler()->start();

    /* make sure we don't fall off the edge */
    spin_forever(1);
}
