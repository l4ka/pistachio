/*********************************************************************
 *                
 * Copyright (C) 2002-2004, 2007,  Karlsruhe University
 *                
 * File path:     glue/v4-x86/x32/resources.cc
 * Description:   thread resource management
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
 * $Id: resources.cc,v 1.16 2004/06/02 08:41:44 sgoetz Exp $
 *                
 ********************************************************************/
#include INC_API(tcb.h)
#include INC_ARCH(fpu.h)
#include INC_GLUE_SA(resource_functions.h)
#include <kdb/tracepoints.h>

void thread_resources_t::load(tcb_t * tcb)
{
    if (tcb->resource_bits.have_resource (COPY_AREA))
    {
	// If we had a nested pagefault, the saved partner will be our
	// real communication partner.  For other types of IPC copy
	// interruptions, the saved_partner will be nil.
	threadid_t ptid = tcb->get_saved_partner ().is_nilthread () ?
	    tcb->get_partner () : tcb->get_saved_partner ();
	tcb_t * partner = tcb->get_space ()->get_tcb (ptid);

	for (word_t i = 0; i < COPY_AREA_COUNT; i++)
	{
	    if (copy_area_pdir_idx[i][0] != ~0UL)
	    {
		ia32_pgent_t * pgdir = phys_to_virt
		    (partner->space->get_pagetable (0) + copy_area_pdir_idx[i][0]);
		tcb->space->populate_copy_area (i, pgdir, tcb->get_cpu());
	    }
	}
    }
#if defined(CONFIG_X86_SMALL_SPACES)
    if (tcb->resource_bits.have_resource (IPC_PAGE_TABLE))
    {
	// We may need to reload the page table for our IPC copy
	// communication partner.
	threadid_t ptid = tcb->get_saved_partner ().is_nilthread () ?
	    tcb->get_partner () : tcb->get_saved_partner ();
	tcb_t * partner = tcb->get_space ()->get_tcb (ptid);
	u32_t new_pdir = (u32_t)
	    partner->get_space ()->get_pagetable (partner->get_cpu ());

	if (x86_mmu_t::get_active_pagetable () != new_pdir)
	    x86_mmu_t::set_active_pagetable (new_pdir);
    }
#endif

#ifdef FPU_REENABLE
    if (tcb->resource_bits.have_resource (FPU))
    {
	ASSERT (fpu_owner == tcb);
	ASSERT (fpu_state != NULL);
	TRACEPOINT(IA32_FPU_REENABLE, 
		   printf("strictly reenabling FPU for %t\n", tcb));
	x86_fpu_t::enable();
    }
#endif
}

