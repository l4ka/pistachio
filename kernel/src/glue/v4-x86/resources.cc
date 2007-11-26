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
#include INC_GLUE(resource_functions.h)
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


void thread_resources_t::load(tcb_t * tcb)
{
    if (tcb->resource_bits.have_resource (COPY_AREA))
    {
	// If we had a nested pagefault, the saved partner will be our
	// real communication partner.  For other types of IPC copy
	// interruptions, the saved_partner will be nil.
	threadid_t ptid = tcb->get_saved_partner ().is_nilthread () ?
	    tcb->get_partner () : tcb->get_saved_partner ();
	tcb_t *partner = tcb->get_space()->get_tcb(ptid);
	
	for (word_t i = 0; i < COPY_AREA_COUNT; i++)
	    tcb->space->populate_copy_area (i, tcb, partner->get_space(), tcb->get_cpu());
    }

#if defined(CONFIG_SMP)    
    if (tcb->resource_bits.have_resource (SMP_PAGE_TABLE)) 
    {
	/* Thread migrated to a processor where no first level ptab is
	 * allocated yet.  Using a resource flag ensures that the
	 * memory comes from the processor local pool (NUMA).  We
	 * already run in the thread context and therefore need a
	 * valid ptab.  We use the kernel space when migrating
	 * (space_t::move_tcb) and fix it up later. */
	if (!tcb->get_space()->has_cpu_top_pdir()) 
	    tcb->get_space()->alloc_cpu_top_pdir();
	
	ASSERT(tcb->get_space()->get_top_pdir_phys());
	tcb->pdir_cache = (word_t)tcb->get_space()->get_top_pdir_phys();
	x86_mmu_t::set_active_pagetable (tcb->pdir_cache);
	tcb->resource_bits -= SMP_PAGE_TABLE;
    }
#endif
		
#if defined(CONFIG_X86_SMALL_SPACES)
    if (tcb->resource_bits.have_resource (IPC_PAGE_TABLE))
    {
	// We may need to reload the page table for our IPC copy
	// communication partner.
	threadid_t ptid = tcb->get_saved_partner ().is_nilthread () ?
	    tcb->get_partner () : tcb->get_saved_partner ();
	tcb_t * partner = tcb->get_space ()->get_tcb (ptid);
	u32_t new_pdir = (u32_t)
	    partner->get_space ()->get_top_pdir_phys (partner->get_cpu ());

	if (x86_mmu_t::get_active_pagetable () != new_pdir)
	    x86_mmu_t::set_active_pagetable (new_pdir);
    }
#endif

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
	    pdir_idx[i][j] = ~0UL;
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


