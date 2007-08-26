/*********************************************************************
 *                
 * Copyright (C) 2002-2004, 2007,  Karlsruhe University
 *                
 * File path:     glue/v4-amd64/resources.cc
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
#include INC_ARCHX(x86,fpu.h)
#include INC_GLUE(resource_functions.h)
#include <kdb/tracepoints.h>

#define FPU_REENABLE
static tcb_t * fpu_owner UNIT("cpulocal");

DECLARE_KMEM_GROUP (kmem_resources);

#ifdef FPU_REENABLE
DECLARE_TRACEPOINT(AMD64_FPU_REENABLE);
#endif

void thread_resources_t::save(tcb_t * tcb)
{
    if (tcb->resource_bits.have_resource(FPU))
    {
	x86_fpu_t::disable();
#ifndef FPU_REENABLE
	tcb->resource_bits -= FPU;
#endif
    }

    if (tcb->resource_bits.have_resource(COPY_AREA))
	release_copy_area (tcb, false);
}

void thread_resources_t::load(tcb_t * tcb)
{
    if (tcb->resource_bits.have_resource(COPY_AREA))
    {
	//TRACEF("%p: load copy area resources\n", tcb);

 	for (word_t i = 0; i < COPY_AREA_COUNT; i++)
 	{
 	    tcb_t * partner = tcb->get_partner_tcb();
 	    if (copy_area_pml4_idx[i] != ~0UL)
	    {
		
		pgent_t *src_pdp = partner->get_space()->pgent(copy_area_pml4_idx[i])->
		    subtree(partner->get_space(), pgent_t::size_1g)->
		    next(partner->get_space(), pgent_t::size_1g, copy_area_pdp_idx[i]);
		tcb->space->populate_copy_area(i, src_pdp, tcb->get_cpu());
	    }
	}
    }
#ifdef FPU_REENABLE
    if (tcb->resource_bits.have_resource(FPU))
    {
	ASSERT (fpu_owner == tcb);
	ASSERT (fpu_state != NULL);
	TRACEPOINT(AMD64_FPU_REENABLE, 
		   printf("strictly reenabling FPU for %t\n", tcb));
	x86_fpu_t::enable();
    }
#endif
}

void thread_resources_t::purge(tcb_t * tcb)
{
    if (fpu_owner == tcb)
    {
	x86_fpu_t::enable();
	x86_fpu_t::save_state(this->fpu_state);
	fpu_owner = NULL;
	x86_fpu_t::disable();

#ifdef FPU_REENABLE
	tcb->resource_bits -= FPU;
#endif
    }

    if (tcb->resource_bits.have_resource(COPY_AREA))
	release_copy_area (tcb, false);
}


void thread_resources_t::init(tcb_t * tcb)
{
    tcb->resource_bits.init();
    fpu_state = NULL;

    last_copy_area = 0;
    pdp = NULL;
    for (word_t i = 0; i < COPY_AREA_COUNT; i++)
    {
	copy_area_pml4_idx[i] = ~0UL;
	copy_area_pdp_idx[i] = ~0UL;
    }
}


void thread_resources_t::free(tcb_t * tcb)
{
    ASSERT(tcb);
    if (fpu_state)
    {
	kmem.free(kmem_resources, fpu_state, x86_fpu_t::get_state_size());
	fpu_state = NULL;

	if (fpu_owner == tcb)
	{
	    fpu_owner = NULL;
	    x86_fpu_t::disable();
	}
    }

    if (tcb->resource_bits.have_resource(COPY_AREA))
	release_copy_area (tcb, false);
}

void thread_resources_t::x86_no_math_exception(tcb_t * tcb)
{
    ASSERT(&tcb->resources == this);
    x86_fpu_t::enable();

    // if the current thread owns the fpu already do a quick exit
    if (fpu_owner != tcb)
    {
	if (fpu_owner != NULL)
	{
	    x86_fpu_t::save_state(fpu_owner->resources.fpu_state);
#ifdef  FPU_REENABLE
	    fpu_owner->resource_bits -= FPU;
#endif
	}
	fpu_owner = tcb;

	if (fpu_state == NULL)
	{
	    fpu_state = kmem.alloc(kmem_resources, x86_fpu_t::get_state_size());
	    x86_fpu_t::init();
	}
	else
	    x86_fpu_t::load_state(fpu_state);
    }

    tcb->resource_bits += FPU;
}

