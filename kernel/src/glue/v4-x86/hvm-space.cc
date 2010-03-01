/*********************************************************************
 *
 * Copyright (C) 2007,  Karlsruhe University
 *
 * File path:     glue/v4-ia32/hvm/ctrl.cc
 * Description:   Full Virtualization Extensions
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
 * $Id$
 *
 ********************************************************************/

#include INC_API(tcb.h)
#include INC_API(space.h)
#include INC_API(queueing.h)
#include INC_GLUE(hvm-space.h)


INLINE void x86_hvm_space_t::set_hvm_mode (tcb_t *tcb, space_t *space)
{
    if (space->is_hvm_space())
    {
	if (!tcb->resource_bits.have_resource(HVM))
	{
	    if (tcb->get_arch()->enable_hvm())
		tcb->resource_bits += HVM;
	}
    }
    else
    {
	if (tcb->resource_bits.have_resource(HVM))
	{
	    tcb->resource_bits -= HVM;
	    tcb->get_arch()->disable_hvm();
	}
    }
}


bool x86_hvm_space_t::activate (space_t *space)
{
    if (active)
	return true;
#if defined(CONFIG_IO_FLEXPAGES)
    /*
     * If we use the IOPBM we allocate it early to avoid synchronization
     * of all VMCS on a lazy allocation of the IOPBM.
     */
    if (!create_io_bitmap (space))
	return false;
#endif
    active = true;

    /* Loop through existing TCBs pointing to space, and activate HVM */
    tcb_t *last_tcb = NULL;
    for (tcb_t *tcb = tcb_list; tcb != last_tcb; tcb = tcb->get_arch ()->space_list.prev)
    {
	ASSERT (tcb->exists ());
	ASSERT (tcb->get_space () == space);
	set_hvm_mode(tcb, tcb->get_space ());
	last_tcb = tcb;
    }
    return true;
}


void x86_hvm_space_t::handle_gphys_unmap (addr_t g_paddr, word_t log2size)
{
    if (!active)
	return;
    /*
     * Loop through all TCBs (VCPUs) using the space,
     * and remove the entries from their VTLBs.
     */
    tcb_t *last_tcb = NULL;
    for (tcb_t *tcb = tcb_list; tcb != last_tcb; tcb = tcb->get_arch ()->space_list.prev)
    {
	ASSERT (tcb->exists ());
        ASSERT (tcb->get_arch()->is_hvm_enabled());
        // Currently, just flush the VTLB completely.
	last_tcb = tcb;
    }
}


bool x86_hvm_space_t::lookup_gphys_addr (addr_t gvaddr, addr_t *gpaddr)
{
    if (!active)
	return false;
    
    tcb_t *tcb = tcb_list;
   
    if (!tcb || !tcb->exists() || !tcb->get_arch()->is_hvm_enabled()) 
	return false;
    
    return tcb->get_arch()->vtlb.lookup_gphys_addr(gvaddr, gpaddr);
}


void x86_hvm_space_t::enqueue_tcb (tcb_t *tcb, space_t *space)
{
    ENQUEUE_LIST_HEAD (tcb_list, tcb, get_arch ()->space_list);
    set_hvm_mode (tcb, space);
}

void x86_hvm_space_t::dequeue_tcb (tcb_t *tcb, space_t *space)
{
    DEQUEUE_LIST (tcb_list, tcb, get_arch ()->space_list);
}
