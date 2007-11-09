/*********************************************************************
 *                
 * Copyright (C) 2002-2004, 2007,  Karlsruhe University
 *                
 * File path:     glue/v4-x86/x64/resources.cc
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
 * $Id: resources.cc,v 1.5 2006/10/19 22:57:39 ud3 Exp $ 
 *                
 ********************************************************************/
#include INC_API(tcb.h)
#include INC_ARCH(fpu.h)
#include INC_GLUE_SA(resource_functions.h)
#include <kdb/tracepoints.h>

void thread_resources_t::load(tcb_t * tcb)
{
    if (tcb->resource_bits.have_resource(COPY_AREA))
    {
	//TRACEF("%p: load copy area resources\n", tcb);

 	for (word_t i = 0; i < COPY_AREA_COUNT; i++)
 	{
 	    tcb_t * partner = tcb->get_partner_tcb();
 	    if (copy_area_pdir_idx[i][0] != ~0UL)
	    {
		
		pgent_t *src_pdp = partner->get_space()->pgent(copy_area_pdir_idx[i][0])->
		    subtree(partner->get_space(), pgent_t::size_1g)->
		    next(partner->get_space(), pgent_t::size_1g, copy_area_pdir_idx[i][1]);
		tcb->space->populate_copy_area(i, src_pdp, tcb->get_cpu());
	    }
	}
    }
#ifdef FPU_REENABLE
    if (tcb->resource_bits.have_resource(FPU))
    {
	ASSERT (fpu_owner == tcb);
	ASSERT (fpu_state != NULL);
	TRACEPOINT(AMD64_FPU_REENABLE, "strictly reenabling FPU for %t\n", tcb);
	x86_fpu_t::enable();
    }
#endif
}
