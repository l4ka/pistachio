/*********************************************************************
 *                
 * Copyright (C) 2002-2004, 2007,  Karlsruhe University
 *                
 * File path:     glue/v4-amd64/resource_functions.h
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
 * $Id: resource_functions.h,v 1.4 2004/10/08 07:46:02 ud3 Exp $
 *                
 ********************************************************************/
#ifndef __GLUE__V4_IA32__RESOURCE_FUNCTIONS_H__
#define __GLUE__V4_IA32__RESOURCE_FUNCTIONS_H__

#include INC_GLUE(resources.h)
#include INC_API(tcb.h)
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
    return (addr_t) (COPY_AREA_START + (n * COPY_AREA_SIZE));
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
    ASSERT (copy_area_pml4_idx[n] != ~0UL);
    ASSERT (copy_area_pdp_idx[n] != ~0UL);
    return (addr_t) ((copy_area_pml4_idx[n] << AMD64_PML4_BITS) || (copy_area_pdp_idx[n] << AMD64_PDP_BITS));
}                       

/**
 * Enable a copy area.  If all copy areas have been exhausted, re-use
 * and old copy area in a FIFO order.
 *
 * @param tcb			TCB of current thread
 * @param saddr			address within source space
 * @param partner		TCB of partner thread
 * @param daddr			address within destination space
 *
 * @return an address into the copy area where kernel should perform
 * the IPC copy.
 */
INLINE void thread_resources_t::enable_copy_area (tcb_t * tcb,
						    addr_t * saddr,
						    tcb_t * partner,
						    addr_t * daddr)
{
    //TRACEF("tcb = %p, saddr = %p, partner = %p, daddr = %x\n", tcb, *saddr, partner, *daddr);
    pgent_t *src_pdp = partner->get_space()->
	pgent(page_table_index(pgent_t::size_512g, *daddr))->
	subtree(partner->get_space(), pgent_t::size_1g)->
	next(partner->get_space(), pgent_t::size_1g, page_table_index(pgent_t::size_1g, *daddr));
    
    word_t n = last_copy_area; 
    ASSERT (n <= COPY_AREA_COUNT); 
    
    last_copy_area++; 
    if (last_copy_area >= COPY_AREA_COUNT) 
 	last_copy_area = 0; 

    bool flush = (copy_area_pml4_idx[n] != ~0UL);
    copy_area_pml4_idx[n] = page_table_index(pgent_t::size_512g, *daddr); 
    copy_area_pdp_idx[n] = page_table_index(pgent_t::size_1g, *daddr); 
    
    tcb->space->populate_copy_area (n, src_pdp, tcb->get_cpu()); 

     // If we are overwriting a copy area we need to flush the old TLB 
     // entries. 
     if (flush) 
	 x86_mmu_t::flush_tlb (false); 

     tcb->resource_bits += COPY_AREA;
     
     *daddr = addr_offset (copy_area_address (n), (word_t) *daddr & (AMD64_PDP_SIZE-1));
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
    //TRACEF("tcb = %p, disable_copyarea = %s\n", tcb, (disable_copyarea ? "true" : "false"));
    
    if (tcb->resource_bits.have_resource(COPY_AREA)) 
     { 
	 for (word_t i = 0; i < COPY_AREA_COUNT; i++) 
	     tcb->space->delete_copy_area (i, tcb->get_cpu()); 

	// Flush TLB to get rid of copy area TLB entries.  This can be
	// optimized away if we know that we're going to switch to
	// another address space (i.e., implicitly flush the TLB).
	x86_mmu_t::flush_tlb (false);

 	if (disable_copyarea) {
	    tcb->resource_bits -= COPY_AREA;
	    last_copy_area = 0;
	    for (word_t i = 0; i < COPY_AREA_COUNT; i++){
		copy_area_pml4_idx[i] = ~0UL;
		copy_area_pdp_idx[i] = ~0UL;
	    }
	}
     }     
}

#endif /* !__GLUE__V4_IA32__RESOURCE_FUNCTIONS_H__ */
