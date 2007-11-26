/*********************************************************************
 *                
 * Copyright (C) 2007,  Karlsruhe University
 *                
 * File path:     glue/v4-x86/resource_functions.h
 * Description:   
 *                
 * @LICENSE@
 *                
 * $Id:$
 *                
 ********************************************************************/
#ifndef __GLUE__V4_X86__RESOURCE_FUNCTIONS_H__
#define __GLUE__V4_X86__RESOURCE_FUNCTIONS_H__

#include <linear_ptab.h>

/**
 * Calculate start address of copy area.
 *
 * @param n		copy area number
 *
 * @return start address of indicated copy area.
 */
INLINE addr_t thread_resources_t::copy_area_address (word_t n)
{
    ASSERT (n < COPY_AREA_COUNT);
    return (addr_t) (COPY_AREA_START + COPY_AREA_SIZE * n);
}



/**
 * Calculate real start address of copy area.
 *
 * @param n             copy area number
 *
 * @return real address of first byte in indicated copy area
 */
INLINE addr_t thread_resources_t::copy_area_real_address (word_t n)
{
    ASSERT (n < COPY_AREA_COUNT);
    
    word_t addr = 0;
    pgent_t::pgsize_e pgsize = pgent_t::size_max - COPY_AREA_PDIRS + 1;
    for (word_t i = 0; i < COPY_AREA_PDIRS; i++)
    {
	ASSERT (pdir_idx[n][i] != ~0UL);
	addr |= pdir_idx[n][i] << page_shift(pgsize++);
    }
    return (addr_t) (addr);
}                       

/**
 * Retrieve pdir idx of copy area
 *
 * @param n		copy area number
 *
 * @return start address of indicated copy area.
 */
INLINE word_t thread_resources_t::copy_area_pdir_idx (word_t n, word_t p)
{
    ASSERT (n < COPY_AREA_COUNT);
    ASSERT (p < COPY_AREA_PDIRS);
    return pdir_idx[n][p];
}

/**
 * Enable a copy area.  If all copy areas have been exhausted, re-use
 * and old copy area in a FIFO order.  Adjust source and destination
 * address according to copy area or small space area.
 *
 * @param tcb			TCB of current thread
 * @param saddr			address within source space
 * @param partner		TCB of partner thread
 * @param daddr			address within destination space
 *
 */
INLINE void thread_resources_t::enable_copy_area (tcb_t * tcb,
						  addr_t * saddr,
						  tcb_t * partner,
						  addr_t * daddr)
{
#if defined(CONFIG_X86_SMALL_SPACES)
    if (tcb->space->is_small ())
    {
	// Copy directly from small space area.

	*saddr = addr_offset (*saddr, tcb->space->smallspace_offset ());

	// Copy directly into target space without using any copy
	// area.  Make sure that the page table of the target address
	// space is used.

	u32_t new_pdir = (u32_t)
	    partner->get_space ()->get_top_pdir_phys (partner->get_cpu ());
	if (x86_mmu_t::get_active_pagetable () != new_pdir)
	    x86_mmu_t::set_active_pagetable (new_pdir);
	tcb->resource_bits += IPC_PAGE_TABLE;
	return;
    }
    else if (partner->space->is_small ())
    {
	// Copy directly into small space area within current address
	// space.

	*daddr = addr_offset (*daddr, tcb->space->smallspace_offset ());
	return;
    }
#endif

    word_t n = last_copy_area;
    ASSERT (n <= COPY_AREA_COUNT);
    last_copy_area++;
    if (last_copy_area >= COPY_AREA_COUNT)
	last_copy_area = 0;

    
    pgent_t::pgsize_e pgsize = pgent_t::size_max;
    
    for (word_t i = 0; i < COPY_AREA_PDIRS; i++)
	pdir_idx[n][i] = page_table_index(pgsize--, *daddr); 
    
    bool flush = (pdir_idx[n][COPY_AREA_PDIRS-1] != ~0UL);

    
    for (word_t i = 0; i < COPY_AREA_COUNT; i++)
	tcb->space->populate_copy_area (i, tcb, partner->space, tcb->get_cpu ());

    // If we are overwriting a copy area we need to flush the old TLB
    // entries.
    if (flush)
	x86_mmu_t::flush_tlb (IS_SPACE_GLOBAL (partner->get_space ()));
    tcb->resource_bits += COPY_AREA;

    *daddr = addr_offset (copy_area_address (n),
			  (word_t) *daddr & (page_size(pgsize+1)-1));
}


/**
 * Release all copy areas.
 *
 * @param tcb			TCB of current thread
 * @param disable_copyarea	should copy area resource be disabled or not
 */

INLINE void thread_resources_t::release_copy_area (tcb_t * tcb,
						   bool disable_copyarea)
{
    
#if defined(CONFIG_X86_SMALL_SPACES)
    if (tcb->resource_bits.have_resource (IPC_PAGE_TABLE))
    {
	if (disable_copyarea)
	    tcb->resource_bits -= IPC_PAGE_TABLE;
	return;
    }
#endif
    if (tcb->resource_bits.have_resource (COPY_AREA))
    {
	for (word_t i = 0; i < COPY_AREA_COUNT; i++)
	    tcb->space->delete_copy_area (i, tcb->get_cpu());

	// Flush TLB to get rid of copy area TLB entries.  This can be
	// optimized away if we know that we're going to switch to
	// another address space (i.e., implicitly flush the TLB).
	x86_mmu_t::flush_tlb
	    (IS_SPACE_GLOBAL (tcb->get_partner_tcb ()->get_space ()));

	if (disable_copyarea)
	{
	    tcb->resource_bits -= COPY_AREA;
	    for (word_t i = 0; i < COPY_AREA_COUNT; i++)
		for (word_t j = 0; j < COPY_AREA_PDIRS; j++)
		    pdir_idx[i][j] = ~0UL;
	    last_copy_area = 0;
	}
    }    
}




#ifdef CONFIG_SMP
/**
 * Toggle X-CPU pagetable allocation
 *
 * @param tcb		TCB of current thread
 * @param cpu		remote processor
 */
INLINE void thread_resources_t::smp_xcpu_pagetable (tcb_t * tcb, cpuid_t cpu)
{
    ASSERT(tcb);
    ASSERT(tcb->get_space());

    if ( !tcb->get_space()->has_cpu_top_pdir(cpu) ) 
    {
	tcb->pdir_cache = (word_t)get_kernel_space ()->get_top_pdir_phys (cpu);
	tcb->resource_bits += SMP_PAGE_TABLE;
    }
    else 
	tcb->pdir_cache = (word_t)tcb->get_space ()->get_top_pdir_phys (cpu);
}
#endif


#endif /* !__GLUE__V4_X86__RESOURCE_FUNCTIONS_H__ */
