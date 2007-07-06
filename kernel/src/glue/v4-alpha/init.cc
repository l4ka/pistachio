/*********************************************************************
 *                
 * Copyright (C) 2002,   University of New South Wales
 *                
 * File path:     glue/v4-alpha/init.cc
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
 * $Id: init.cc,v 1.29 2004/08/25 07:15:37 awiggins Exp $
 *                
 ********************************************************************/

#include <stddef.h>
#include <debug.h>
#include <kmemory.h>
#include <mapping.h>

#include INC_API(kernelinterface.h)
#include INC_API(schedule.h)
#include INC_API(space.h)
#include INC_API(resources.h)

#include INC_ARCH(palcalls.h)
#include INC_ARCH(hwrpb.h)
#include INC_ARCH(console.h)

#include INC_GLUE(intctrl.h)
#include INC_GLUE(memory.h)
#include INC_GLUE(timer.h)

#include INC_ARCH(debug.h)

#define INIT_TEXT SECTION(".init.text")

extern char _start_pal_code;
extern char _end_pal_code;
extern char _start_pal_stub;
extern char _end_pal_stub;

extern "C" void __ent_interrupt(void);
extern "C" void __ent_arith(void);
extern "C" void __ent_mm(void);
extern "C" void __ent_if(void);
extern "C" void __ent_una(void);
extern "C" void __ent_sys(void);

static alpha_pcb_t alpha_init_pcb;
struct hwrpb *hwrpb = (struct hwrpb *) CONSOLE_AREA_START;

#define BOOTMEM_PAGES (CONFIG_BOOTMEM_PAGES)

static void INIT_TEXT dump_info(void)
{
    printf("Kernel configuration:\n");
    printf("\tVPT  area: 0x%lx -> 0x%lx (0x%lx)\n", VLPT_AREA_START, VLPT_AREA_END, VLPT_AREA_SIZE);
    printf("\tKTCB area: 0x%lx -> 0x%lx (0x%lx)\n", KTCB_AREA_START, KTCB_AREA_END, KTCB_AREA_SIZE);
    
    printf("System configuration:\n");
    printf("\tTimer frequency        : %dHz\n", hwrpb->rpb_intr_freq / 4096);
    printf("\tCycle counter frequency: %dHz\n", hwrpb->rpb_cc_freq);
}

/* Before this function, we are executing in the initial AS provided by the console.  Afterwards,
 * we are executing in our AS */
static void init_pageing(void)
{
    get_asid_cache()->init();
    
    /* The Alpha ARM says that an alpha processor should support > 13 ASNs */
    get_asid_cache()->set_valid(0, 12);

    /* Create and init kernel space */
    init_kernel_space();

    /* Update the console */
    remap_console();

    /* Setup initial pcb */
    alpha_init_pcb.ptbr = (word_t) virt_to_phys((addr_t) get_kernel_space()->pgent(0)) >> ALPHA_PAGE_BITS;
    alpha_init_pcb.asn = get_kernel_space()->get_asid()->get();
    alpha_init_pcb.usp = 0;
    alpha_init_pcb.flags = 1;

    /* Set up the kernel GP so kernel context is set up correctly */
    word_t gp;
    asm __volatile__ ("stq   $29, 0(%0)" : : "r" (&gp));
    /* sjw (05/08/2002): We may need to do this per-cpu? */
    PAL::wrkgp(gp);
    

    /* We do this early for the boot CPU so we can get early page faults */
    PAL::wrent((word_t) __ent_mm, PAL::entMM);

    /* Write VPTPTR */
    PAL::wrvptptr(VLPT_AREA_START);

    /* sjw (02/08/2002): Ensure that nothing gets between these two instructions that modifies sp! */
    asm __volatile__ ("stq   $30, 0(%0)\n\t" : : "r" (&alpha_init_pcb.ksp));
    PAL::swpctx((word_t) virt_to_phys((addr_t) &alpha_init_pcb));

    PAL::tbia();
    PAL::imb();

    /* If this works ;) */
    get_asid_cache()->set_valid(13, hwrpb->rpb_max_asn);
}

#define WRITE_SPINNER(a, b) do { static char __a[] = (a); 	video_print_string(__a, (b) - sizeof(__a) + 3); } while(0)

static void init_debugging(void)
{
    WRITE_SPINNER("Timer[ ]",  SPIN_TIMER_INT);
    WRITE_SPINNER("Int[ ]", SPIN_INTERRUPT);
    WRITE_SPINNER("Yield[ ]", SPIN_YIELD);
    WRITE_SPINNER("Idle[ ]", SPIN_IDLE);
    WRITE_SPINNER("Slowpath[ ]", SPIN_IPC);
}

/**
 * init_bootmem - Initialises the inital RAM pool
 * @start: The start of the reserved memory region
 * @end: The end of the reserved memory region
 *
 * Description: This function initialises the kernel memory
 * allocator.
 *
 **/
#define UPDATE_FREE(f, a) do { word_t _b = (word_t) addr_align_up((a), ALPHA_PAGE_SIZE) >> ALPHA_PAGE_BITS; if(_b > f) f = _b; } while(0)
    
extern word_t _end[];
extern word_t _start[];

static int INIT_TEXT init_bootmem()
{
    /* Initialise KIP memory info from HWRPB */
    struct mddt *mem_desc = (struct mddt *) ( ((word_t) INIT_HWRPB) + INIT_HWRPB->rpb_memdat_off);
    int nclusters = mem_desc->mddt_cluster_cnt;
    struct mddt::mddt_cluster *cluster = mem_desc->mddt_clusters;
    word_t free_start = 0;
    
    UPDATE_FREE(free_start, _end);
    UPDATE_FREE(free_start, get_kip()->sigma0.mem_region.high);
    UPDATE_FREE(free_start, get_kip()->sigma1.mem_region.high);
    UPDATE_FREE(free_start, get_kip()->root_server.mem_region.high);

    /* sjw (16/10/2002): Yuck! */
    get_kip()->memory_info.n = 0;

    /* sjw (14/10/2002): This should eventually be much nicer, but we make some assumptions here. */    
    for(int i = 0; i < nclusters; cluster++, i++) {

	memdesc_t::type_e type = (cluster->mddt_usage & 3) ? memdesc_t::reserved : memdesc_t::conventional;

/*	printf("Inserting 0x%lx - 0x%lx, %d\n", cluster->start_pfn  << ALPHA_PAGE_BITS, 
	       (cluster->start_pfn + cluster->numpages)  << ALPHA_PAGE_BITS, type);
*/
	get_kip()->memory_info.insert(type, false, (addr_t) (cluster->mddt_pfn << ALPHA_PAGE_BITS), 
				      (addr_t) ( (cluster->mddt_pfn + cluster->mddt_pg_cnt) << ALPHA_PAGE_BITS));
    }


#ifdef CONFIG_PLAT_TSUNAMI
    /* Values from Tsunami 21272 Chipset -- Function description */
#if CONFIG_ALPHA_ADDRESS_BITS == 48
    get_kip()->memory_info.insert(memdesc_t::dedicated, false,
				  (addr_t) (1UL << 43), 
				  (addr_t) ((1UL << 43) + 0x3FF000000));
#endif

#if CONFIG_ALPHA_ADDRESS_BITS == 43
    get_kip()->memory_info.insert(memdesc_t::dedicated, false,
				  (addr_t) (1UL << 40), 
				  (addr_t) ((1UL << 40) + 0x3FF000000));
#endif

#endif

#ifdef CONFIG_PLAT_MIATA
    /* Values from 21174 TRM */
    get_kip()->memory_info.insert(memdesc_t::dedicated, false,
				  (addr_t) 0x8000000000, 
				  (addr_t) 0x8700000000);
    get_kip()->memory_info.insert(memdesc_t::dedicated, false,
				  (addr_t) 0x8780000000, 
				  (addr_t) 0x87B0000000);

    /* These are here if Byte/Word mode is enabled */
    get_kip()->memory_info.insert(memdesc_t::dedicated, false,
				  (addr_t) 0x8800000000, 
				  (addr_t) 0xbc00000000);
    get_kip()->memory_info.insert(memdesc_t::dedicated, false,
				  (addr_t) 0xc7c0000000, 
				  (addr_t) 0xc800000000);
#endif

    /* Console area + kernel */
    /* sjw (22/10/2002): Make this more accurate! */
    get_kip()->memory_info.insert(memdesc_t::reserved, false,
				  0,
				  addr_align_up (_end, ALPHA_PAGE_SIZE));
    
    cluster = mem_desc->mddt_clusters;
    /* Find some bootmem */
    for(int i = 0; i < nclusters; cluster++, i++) {
	word_t start = cluster->mddt_pfn;
	int npages = cluster->mddt_pg_cnt;

	if((start + npages) < free_start)
	    continue;

	if(start < free_start) {
	    npages -= (free_start - start);
	    start = free_start;
	}

	if(npages < BOOTMEM_PAGES)
	    continue;

	start <<= ALPHA_PAGE_BITS;
	word_t end = start + (BOOTMEM_PAGES << ALPHA_PAGE_BITS);
	
	get_kip()->memory_info.insert(memdesc_t::reserved, false,
				      (addr_t) start, (addr_t) end);
		
	kmem.init(phys_to_virt((addr_t) start), phys_to_virt((addr_t) end));

	return 0;
    }

    printf("Couldn't find enough bootmem!");

    /* can't drop to debugger as we don't have enough state set up */
    PAL::halt();
    
    return 1;
}

#undef UPDATE_FREE

static int INIT_TEXT init_arch(void)
{
    /* Note that this is called twice --- once before init_pageing, and once after */
    init_console();

    /* initialize kernel interface page */
    get_kip()->init();

    /* start and end are link variables */
    init_bootmem();

    /* initialise page tables */
    init_pageing();

    /* Really init console ;) */
    init_console();

    /* initialize mapping database */
    init_mdb();

    /* initialize kernel debugger if any */
    if (get_kip()->kdebug_init)
	get_kip()->kdebug_init();

    /* Initialise global interrupt state */
    get_interrupt_ctrl()->init_arch();    

    /* Initialise timer state */
    get_timer()->init_global();    

    return 0;
}


/**
 * init_cpu - Initialises per-CPU information
 *
 * Description: Initialised the per-CPU state for this 
 * processor.  This will be the first (C) function called
 * by a new processor.  
 *
 * The boot processor is guaranteed to call this before any other
 * processor (before the other processors are started).
 *
 **/
extern "C" void INIT_TEXT init_cpu(void)
{
    /* Initialise PAL context (register for interrupts etc. */
    
    /* Ensure that we will not take any interrupts (e.g. the clock).  We
     * can't take any page faults until this is set up, also.
     */
    PAL::swpipl(PAL::IPL_highest);

    /* Now set up the trap vectors */
    /* sjw (23/07/2002): Should this go in intctr_t::init_cpu()? */
    PAL::wrent((word_t) __ent_interrupt, PAL::entInt);
    PAL::wrent((word_t) __ent_arith, PAL::entArith);
    PAL::wrent((word_t) __ent_mm, PAL::entMM);
    PAL::wrent((word_t) __ent_if, PAL::entIF);
    PAL::wrent((word_t) __ent_una, PAL::entUna);
    PAL::wrent((word_t) __ent_sys, PAL::entSys);

    /* Initialise local interrupt state */
    get_interrupt_ctrl()->init_cpu();    

    /* Initialise local timer state */
    get_timer()->init_cpu();

    /* Initilise local resources */
    get_resources()->init_cpu();
}

extern "C" 
void
memcpy(char *dest, char *src, int size)
{
	int i=0;
	for(i=0; i<size;i++)
		dest[i] = src[i];
}

#if defined(CONFIG_ALPHA_PAL_IPC_FASTPATH)
void setup_pal()
{
    char *pal_stub_addr = \
	    (char*) (0xfffffc0000000000ul + 0x1bc00);
    char *fast_path_addr = (char *) (0xfffffc0000000000ul + 0x22000);

    memcpy(pal_stub_addr, &_start_pal_stub, &_end_pal_stub - &_start_pal_stub);
    memcpy(fast_path_addr, &_start_pal_code, &_end_pal_code - &_start_pal_code);

    PAL::cflush(0x1bc00 >> ALPHA_PAGE_BITS);
    PAL::cflush(0x22000 >> ALPHA_PAGE_BITS);
}
#endif

extern "C" void INIT_TEXT startup_system()
{

    init_arch();
    init_cpu();

#if defined(CONFIG_ALPHA_PAL_IPC_FASTPATH)
    setup_pal();
#endif

    init_hello();

    dump_info();
    /* sjw (23/07/2002): Start up other CPUs here */

    init_debugging();
//    interrupt_test();

    /* initialize the scheduler */
    get_current_scheduler()->init();
    /* get the thing going - we should never return */
    get_current_scheduler()->start();
    
    /* make sure we don't fall off the edge */
    spin_forever(1);    
}
