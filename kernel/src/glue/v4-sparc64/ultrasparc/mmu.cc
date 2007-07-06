/*********************************************************************
 *                
 * Copyright (C) 2004, University of New South Wales
 *                
 * File path:    glue/v4-sparc64/ultrasparc/mmu.cc
 * Description:  C handlers for MMU faults
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
 * $Id: mmu.cc,v 1.2 2004/02/22 23:49:53 philipd Exp $
 *                
 ********************************************************************/

#include INC_API(tcb.h)
#include INC_GLUE(space.h)

extern "C" void itsb_miss_handler(void)
{
    tpc_t tpc;
    tpc.get();

    //TRACEF("tpc = %p\n", tpc.tpc);
    
    space_t *space = get_current_tcb()->get_space();
    if (EXPECT_FALSE(space == NULL)) {
	space = get_kernel_space();
    }

    /* Try to add the mapping to the TSB. */
    if(space->handle_tsb_miss((addr_t)tpc.tpc, tlb_t::i_tlb)) {
	return;
    }

    /* No mapping found. Generate a page fault. */
    tstate_t tstate;
    tstate.get();

    space->handle_pagefault((addr_t)tpc.tpc, (addr_t)tpc.tpc, space_t::execute,
	    		    tstate.get_pstate().pstate.priv);

}

extern "C" void dtsb_miss_handler(void)
{
    word_t access_reg = tlb_t::get_tag_access(tlb_t::d_tlb);
    hw_asid_t context = access_reg & ((1ULL << SPARC64_PAGE_BITS) - 1);
    addr_t vaddr = (addr_t)(access_reg & ~((1ULL << SPARC64_PAGE_BITS) - 1));

    //TRACEF("context = %x vaddr = %p\n", context, vaddr);

    space_t *space = space_t::lookup_space(context);
    if (EXPECT_FALSE(space == NULL)) {
	space = get_kernel_space();
    }

    /* Try to add the mapping to the TSB. */
    if(space->handle_tsb_miss(vaddr, tlb_t::d_tlb)) {
	return;
    }

    /* No mapping found. Generate a page fault. */
    tpc_t tpc;
    sfsr_t sfsr;
    tstate_t tstate;

    tpc.get();
    tstate.get();
    sfsr.get(sfsr_t::data);

    space->handle_pagefault(vaddr, (addr_t)tpc.tpc,
	    		    sfsr.sfsr.w ? space_t::write : space_t::read,
	    		    tstate.get_pstate().pstate.priv);
}

extern "C" void data_fault_handler(void)
{
    word_t access_reg = tlb_t::get_tag_access(tlb_t::d_tlb);
    hw_asid_t context = access_reg & ((1ULL << SPARC64_PAGE_BITS) - 1);
    addr_t vaddr = (addr_t)(access_reg & ~((1ULL << SPARC64_PAGE_BITS) - 1));

    //TRACEF("context = %x vaddr = %p\n", context, vaddr);
    
    /* Look up the faulting page */
    space_t *space = space_t::lookup_space(context);
    if (EXPECT_FALSE(space == NULL)) {
	space = get_kernel_space();
    }

    /* determine the faulting address and handle the fault */
    tpc_t tpc;
    sfsr_t sfsr;
    tstate_t tstate;

    tpc.get();
    tstate.get();
    sfsr.get(sfsr_t::data);

    space->handle_pagefault(vaddr, (addr_t)tpc.tpc,
	    		    sfsr.sfsr.w ? space_t::write : space_t::read,
	    		    tstate.get_pstate().pstate.priv);
}
