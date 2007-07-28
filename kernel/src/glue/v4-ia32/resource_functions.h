/*********************************************************************
 *                
 * Copyright (C) 2002-2004, 2007,  Karlsruhe University
 *                
 * File path:     glue/v4-ia32/resource_functions.h
 * Description:   Functions for handling the ia32 specific resources
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
 * $Id: resource_functions.h,v 1.9 2004/04/20 15:38:05 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __GLUE__V4_IA32__RESOURCE_FUNCTIONS_H__
#define __GLUE__V4_IA32__RESOURCE_FUNCTIONS_H__

#include INC_GLUE(resources.h)
#include INC_API(tcb.h)


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
 * @param n		copy area number
 *
 * @return real address of first byte in inidacted copy area
 */
INLINE addr_t thread_resources_t::copy_area_real_address (word_t n)
{
    ASSERT (n < COPY_AREA_COUNT);
    ASSERT (copy_area_pgdir_idx[n] != ~0UL);
    return (addr_t) (copy_area_pgdir_idx[n] << IA32_PAGEDIR_BITS);
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
#if defined(CONFIG_IA32_SMALL_SPACES)
    if (tcb->space->is_small ())
    {
	// Copy directly from small space area.

	*saddr = addr_offset (*saddr, tcb->space->smallspace_offset ());

	// Copy directly into target space without using any copy
	// area.  Make sure that the page table of the target address
	// space is used.

	u32_t new_pdir = (u32_t)
	    partner->get_space ()->get_pdir (partner->get_cpu ());
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

    ia32_pgent_t * pgdir = phys_to_virt (partner->space->get_pdir (0));

    word_t idx = pgdir->get_pdir_idx (*daddr);
    word_t n = last_copy_area;
    ASSERT (n <= COPY_AREA_COUNT);

    last_copy_area++;
    if (last_copy_area >= COPY_AREA_COUNT)
	last_copy_area = 0;

    bool flush = (copy_area_pgdir_idx[n] != ~0UL);
    copy_area_pgdir_idx[n] = idx;
    tcb->space->populate_copy_area (n, &pgdir[idx], tcb->get_cpu ());

    // If we are overwriting a copy area we need to flush the old TLB
    // entries.
    if (flush)
	x86_mmu_t::flush_tlb (IS_SPACE_GLOBAL (partner->get_space ()));
    tcb->resource_bits += COPY_AREA;

    *daddr = addr_offset (copy_area_address (n),
			  (word_t) *daddr & (IA32_PAGEDIR_SIZE-1));
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
#if defined(CONFIG_IA32_SMALL_SPACES)
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
		copy_area_pgdir_idx[i] = ~0UL;
	    last_copy_area = 0;
	}
    }    
}

#endif /* !__GLUE__V4_IA32__RESOURCE_FUNCTIONS_H__ */
