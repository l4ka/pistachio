/********************************************************************* *                
 *
 * Copyright (C) 2003-2004,  National ICT Australia (NICTA)
 *                
 * File path:     glue/v4-arm/init.cc
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
 * $Id: init.cc,v 1.19 2004/12/09 01:14:49 cvansch Exp $
 *                
 ********************************************************************/

#include <debug.h>
#include INC_API(kernelinterface.h)
#include INC_API(schedule.h)
#include INC_GLUE(space.h)
#include INC_GLUE(memory.h)
#include INC_GLUE(intctrl.h)
#include INC_GLUE(config.h)
#include INC_GLUE(hwspace.h)
#include INC_GLUE(timer.h)
#include INC_CPU(syscon.h)
#include INC_ARCH(bootdesc.h)

#include <mapping.h>

DECLARE_KMEM_GROUP(kmem_user_utcb_ref);

extern "C" void init_platform(void);
extern "C" void init_cpu_mappings(void);
extern "C" struct arm_bootdesc* init_platform_mem(void);

#define BOOTMEM_PAGES (CONFIG_BOOTMEM_PAGES)

extern "C" void SECTION(".init") init_kip()
{
    TRACE_INIT("Initialising KIP...\n");    

    /* set user virtual address space */
    get_kip()->memory_info.insert(memdesc_t::conventional, true,
            (addr_t)USER_AREA_START, (addr_t)USER_AREA_END);

    /* reserved regions in physical memory */
    get_kip()->memory_info.insert(memdesc_t::reserved, false,
            addr_align(virt_to_phys(start_text), KB(4)),
            addr_align_up(virt_to_phys(end_kip), KB(4)));
    get_kip()->memory_info.insert(memdesc_t::reserved, false,
            addr_align(virt_to_phys(start_data), KB(4)),
            addr_align_up(virt_to_phys(end_kernel), KB(4)));

    get_kip()->reserved_mem0.set(
            addr_align(virt_to_phys(start_text), KB(4)),
            addr_align_up(virt_to_phys(end_kip), KB(4)));
    get_kip()->reserved_mem1.set(
            addr_align(virt_to_phys(start_data), KB(4)),
            addr_align_up(virt_to_phys(end_kernel), KB(4)));

    get_kip()->init();
}

/**
 * Get page table index of virtual address for a 1m size
 *
 * @param vaddr         virtual address
 *
 * @return page table index for a table of the given page size
 */
INLINE word_t page_table_1m (addr_t vaddr)
{
    return ((word_t) vaddr >> 20) & (ARM_L1_SIZE - 1);
}

/*
 * Add 1MB mappings during init.
 * This function works with the mmu disabled
 */
extern "C" void SECTION(".init") add_mapping_init(space_t *space, addr_t vaddr,
		addr_t paddr, bool uncached )
{
    pgent_t *pg = &space->pt.pdir[ page_table_1m(vaddr) ];

    pg->set_entry_1m(space, paddr, true, true, true, uncached);
}

/* Put the addresses of physical memory directly into the bootinfo structure.
 * We do this here because we want to use some of the memory for a page table,
 * which needs to be around very early on, before we go into virtual mode.
*/
extern "C" void SECTION(".init")
init_ram_descriptors(kernel_interface_page_t *kip_phys)
{
    struct arm_bootdesc* plat_mem = init_platform_mem();
    memdesc_t *current;
    int i;
    
    /* Setup platform physical memory map */
    for (i=0; plat_mem[i].type; i++)
    {
    	current = virt_to_phys(
			kip_phys->memory_info.get_memdesc(
				kip_phys->memory_info.n++));
    	current->set((memdesc_t::type_e)(plat_mem[i].type & 0xf),
			(plat_mem[i].type>>4) & 0xf, false,
			(addr_t)plat_mem[i].start,
			(addr_t)(plat_mem[i].end-1));
    }
}

/*
 * Setup the kernel page table and map the kernel and data areas.
 *
 * This function is entered with the MMU disabled and the code
 * running relocated in physical addresses. We setup the page
 * table and bootstrap ourselves into virtual mode/addresses.
 */
extern "C" void SECTION(".init") init_memory(void) 
{
    extern char _end_init_memory[];
    extern char kernel_space_object[];

    word_t i;
    addr_t start, end;
    memdesc_t *current;
    space_t * kspace_phys = (space_t *)virt_to_phys( kernel_space_object ); 
    kernel_interface_page_t *kip_phys =
	    (kernel_interface_page_t *)virt_to_phys( get_kip() );

    /* Zero out the level 1 translation table */
    for (i = 0; i < sizeof(space_t)/sizeof(word_t); ++i) 
	((word_t*)kspace_phys)[i] = 0;

    /* Calculate the area for the kernel to use as boot memory */
    word_t bootmem_size = BOOTMEM_PAGES << PAGE_BITS_4K;
    addr_t start_bootmem;
    addr_t start_bootmem_phys = 0;

    init_ram_descriptors(kip_phys);

    /* Find a suitable spot for the bootmem */
    for (i = 0; i < kip_phys->memory_info.get_num_descriptors(); i++) {
	current = virt_to_phys(kip_phys->memory_info.get_memdesc(i));
	if (current->type() == memdesc_t::conventional && current->size() >= bootmem_size) {
	    /* Reserve the start of this spot */
	    start_bootmem_phys = (void *)((word_t)current->high() - bootmem_size);
	    break;
	}
    }

    if (start_bootmem_phys == 0) {
	while(1) ;
    }

    start_bootmem_phys = addr_align_up(start_bootmem_phys, ARM_PAGE_SIZE);

    /* Get the bootmem addresses */
    start_bootmem = phys_to_virt(start_bootmem_phys);
    addr_t end_bootmem =	(addr_t)((word_t)start_bootmem + bootmem_size);
    addr_t end_bootmem_phys =	(addr_t)virt_to_phys(end_bootmem);

    /* The kernel can only map 64MB for kernel memory */
    if (((word_t)end_bootmem - KERNEL_AREA_START) > 64*PAGE_SIZE_1M)
	while(1);
    if (((word_t)end_kernel - KERNEL_AREA_START) > 64*PAGE_SIZE_1M)
	while(1);

    /* Map the kernel area to its virtual address */
    start = addr_align(virt_to_phys(start_text), PAGE_SIZE_1M);
    end = virt_to_phys(end_kernel);

    for (i = (word_t)start; i < (word_t)end; i += PAGE_SIZE_1M)
    {
	add_mapping_init( kspace_phys, (addr_t)phys_to_virt(i), (addr_t)i, false );
    }

    /* Map the kernel bootmem area if not covered by the kernel mapping */
    if ((word_t)addr_align_up(end_bootmem_phys, PAGE_SIZE_1M) >
		    (word_t)addr_align_up(end, PAGE_SIZE_1M))
    {
	start = addr_align(start_bootmem_phys, PAGE_SIZE_1M);

	if ((word_t)start < (word_t)addr_align_up(end, PAGE_SIZE_1M))
	    start = addr_align_up(end, PAGE_SIZE_1M);

	end = end_bootmem_phys;

	/* Map the kernel bootmem */
	for (i = (word_t)start; i < (word_t)end; i += PAGE_SIZE_1M)
	{
	    add_mapping_init( kspace_phys, (addr_t)phys_to_virt(i), (addr_t)i, false );
	}
    }

    /* Enable kernel domain (0) */
    write_cp15_register(C15_domain, C15_CRm_default, C15_OP2_default, 0x0001);
    /* Set the translation table base to use the kspace_phys */
    write_cp15_register(C15_ttbase, C15_CRm_default, C15_OP2_default, kspace_phys);

    /* Map the current code area 1:1 */
    start = addr_align(virt_to_phys((addr_t)init_memory), PAGE_SIZE_1M);
    end = virt_to_phys(_end_init_memory);
    for (i = (word_t)start; i < (word_t)end; i += PAGE_SIZE_1M)
    {
	add_mapping_init( kspace_phys, (addr_t)i, (addr_t)i, false );
    }

    /* Map the kernel area to its uncached virtual address for the page table */
    start = addr_align(virt_to_phys(start_text), PAGE_SIZE_1M);
    end = virt_to_phys(end_kernel);
    for (i = (word_t)start; i < (word_t)end; i += PAGE_SIZE_1M)
    {
	add_mapping_init( kspace_phys, (addr_t)phys_to_page_table(i), (addr_t)i, true );
    }

    /* Enable virtual memory, caching etc */
    write_cp15_register(C15_control, C15_CRm_default,
		    C15_OP2_default, C15_CONTROL_KERNEL);
    CPWAIT;

    /* Switch to virtual memory code and stack */
    __asm__ __volatile__ (
	"add	sp, sp,	%0		\n"
	"mov	pc, %1			\n"
	: : "r" (KERNEL_OFFSET), "r" (_end_init_memory)
	: "r0", "memory"
    );

    __asm__ __volatile__ (
	".global    _end_init_memory;	    \n"
	"_end_init_memory:		    \n"
    );

    /* Initialise the kernel memory */
    kmem.init(start_bootmem, end_bootmem);

    /* Map the kernel bootmem area uncached if not covered by the kernel mapping */
    if ((word_t)addr_align_up(end_bootmem_phys, PAGE_SIZE_1M) >
		    (word_t)addr_align_up(end, PAGE_SIZE_1M))
    {
	start = addr_align(start_bootmem_phys, PAGE_SIZE_1M);

	if ((word_t)start < (word_t)addr_align_up(end, PAGE_SIZE_1M))
	    start = addr_align_up(end, PAGE_SIZE_1M);

	end = end_bootmem_phys;

	/* Map the kernel bootmem */
	for (i = (word_t)start; i < (word_t)end; i += PAGE_SIZE_1M)
	{
	    get_kernel_space()->add_mapping( (addr_t)phys_to_page_table(i), (addr_t)i,
			    pgent_t::size_1m, true, true, true );
	}
    }

    /* Initialize CPU specific mappings */
    init_cpu_mappings();

    /* Allocate and map the UTCB reference page */
    addr_t user_utcb_ref_phy = virt_to_phys(kmem.alloc(kmem_user_utcb_ref, 
            PAGE_SIZE_4K));
    arm_cache::cache_flush();

    get_kernel_space()->add_mapping((addr_t)USER_UTCB_REF_PAGE, 
            user_utcb_ref_phy, pgent_t::size_4k, false, false);

    /* Reserve bootmem */
    get_kip()->memory_info.insert(memdesc_t::reserved, false,
		    start_bootmem_phys, end_bootmem_phys);

    start = addr_align(virt_to_phys((addr_t)init_memory), PAGE_SIZE_1M);
    end = virt_to_phys(_end_init_memory);

    /* Remove 1:1 mapping */
    for (i = (word_t)start; i < (word_t)end; i += PAGE_SIZE_1M)
    {
	get_kernel_space()->remove_mapping( (addr_t)i,
			pgent_t::size_1m, true);
    }

    do {
	__asm__ __volatile__ (
	    "add	%0, %0, %1	\n"
	    "mov	pc, %0		\n"
	    :: "r" (__builtin_return_address(0)), "r" (KERNEL_OFFSET)
	);
    } while (1);
}


extern "C" void init_cpu(void);

/**
 * Entry point from ASM into C kernel
 * Precondition: MMU and page tables are disabled
 * Warning: Do not use local variables in startup_system()
 */
extern "C" void SECTION(".init") startup_system()
{
    /* Processor specific initialisation */
    init_cpu();

    /* Map the L4 kernel, enable virtual memory and setup bootmem */
    init_memory();

    /* Initialise the platform, map the console and other memory */
    init_platform();

    /* Initialise the L4 console */
    init_console();
    init_hello();

    /* Setup the kernel address space */
    init_kernel_space();

    /* Prepare KIP */
    init_kip();

    /* Set up mapping database */
    init_mdb ();

    TRACE_INIT("Initialising kernel debugger...\n");
    
    /* initialize kernel debugger if any */
    if (get_kip()->kdebug_init)
	get_kip()->kdebug_init();
    else
	TRACE_INIT("No kernel debugger!\n\r");

    /* Cconfigure IRQ hardware */
    TRACE_INIT("Initialising interrupts...\n");
    get_interrupt_ctrl()->init_arch();
    get_interrupt_ctrl()->init_cpu();

    TRACE_INIT("Initialising timer...\n");
    /* initialize the kernel's timer source */
    get_timer()->init_cpu();
 
    TRACE_INIT("Initialising scheduler...\n");
    /* initialise the scheduler */
    get_current_scheduler()->init();

    /* get the thing going - we should never return */
    get_current_scheduler()->start();

    ASSERT(!"Scheduler start fell through!!!\n");
    
    /* make sure we don't fall off the edge */
    spin_forever(1);
}
