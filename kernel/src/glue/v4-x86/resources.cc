/*********************************************************************
 *                
 * Copyright (C) 2007,  Karlsruhe University
 *                
 * File path:     glue/v4-x86/resources.cc
 * Description:   
 *                
 * @LICENSE@
 *                
 * $Id:$
 *                
 ********************************************************************/
#include INC_API(tcb.h)
#include INC_ARCH(fpu.h)
#include INC_GLUE_SA(resource_functions.h)
#include <kdb/tracepoints.h>

//#define FPU_REENABLE
static tcb_t * fpu_owner UNIT("cpulocal");


DECLARE_KMEM_GROUP (kmem_resources);

#ifdef FPU_REENABLE
DECLARE_TRACEPOINT(X86_FPU_REENABLE);
#endif

void thread_resources_t::save(tcb_t * tcb)
{
    if (tcb->resource_bits.have_resource (FPU))
    {
	x86_fpu_t::disable();
#ifndef FPU_REENABLE
	tcb->resource_bits -= FPU;
#endif
    }

    if (tcb->resource_bits.have_resource (COPY_AREA))
	release_copy_area (tcb, false);
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

    if (tcb->resource_bits.have_resource (COPY_AREA))
	release_copy_area (tcb, false);
}


void thread_resources_t::init(tcb_t * tcb)
{
    tcb->resource_bits.init ();
    fpu_state = NULL;

    last_copy_area = 0;
    for (word_t i = 0; i < COPY_AREA_PDIRS; i++)
	for (word_t j = 0;  j < COPY_AREA_COUNT; j++)
	    copy_area_pdir_idx[i][j] = ~0UL;
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

    if (tcb->resource_bits.have_resource (COPY_AREA))
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

