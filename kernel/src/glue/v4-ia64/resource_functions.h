/*********************************************************************
 *                
 * Copyright (C) 2002, 2003,  Karlsruhe University
 *                
 * File path:     glue/v4-ia64/resource_functions.h
 * Description:   Functions for handling the ia64 specific resources
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
 * $Id: resource_functions.h,v 1.6 2003/09/24 19:04:37 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __GLUE__V4_IA64__RESOURCE_FUNCTIONS_H__
#define __GLUE__V4_IA64__RESOURCE_FUNCTIONS_H__

#include INC_API(tcb.h)
#include INC_API(resources.h)
#include INC_ARCH(ia64.h)


/**
 * Enable the copy area resource by initializing the copy area region
 * with the region register of our IPC partner.
 *
 * @param tcb			current TCB
 * @param partner		partner TCB
 */
INLINE void thread_resources_t::enable_copy_area (tcb_t * tcb, tcb_t * partner)
{
    if (! tcb->resource_bits.have_resource (COPY_AREA))
    {
	// Associate the copy area region with the partner's space.
	rr_t rr (false, partner->space->get_region_id (), 12);
	rr.put (4);
	ia64_srlz_d ();

	tcb->resource_bits += COPY_AREA;
	partner_rid = partner->space->get_region_id ();
    }
}


/**
 * Disable the copy area resource by inserting an invalid region
 * register for the copy area.  We also flush the TLB entries in the
 * copy area since these are not necessarily flushed when when the TLB
 * entries in the main user area are flushed (depends on CPU
 * implementation).
 *
 * @param tcb			current TCB
 * @param disable_resource	disable copy-area resource or not
 */
INLINE void thread_resources_t::disable_copy_area (tcb_t * tcb,
						   bool disable_resource)
{
    if (tcb->resource_bits.have_resource (COPY_AREA))
    {
	// Disable access in the copy area reagion.
	rr_t rr (false, 0, 12);
	rr.put (4);
	ia64_srlz_d ();

	// Purge TLB entries for copy area.
	purge_tc (ia64_phys_to_rr (4, (addr_t) 0), 61, partner_rid);

	if (disable_resource)
	    tcb->resource_bits -= COPY_AREA;
    }
}


/**
 * Enable global breakpoint resource by modifying the psr.db bit of
 * the exception context and the switch context of the thread.  The
 * global breakpoint resource is handed off to the next thread during
 * a thread switch.
 *
 * @param tcb			tcb to enable breakpoints on
 */
INLINE void thread_resources_t::enable_global_breakpoint (tcb_t * tcb)
{
    ia64_exception_context_t * user_frame =
	(ia64_exception_context_t *) tcb->get_stack_top () - 1;
    ia64_switch_context_t * kernel_frame =
	(ia64_switch_context_t *) tcb->stack;

    user_frame->ipsr.db = kernel_frame->psr.db = 1;

    tcb->resource_bits += BREAKPOINT;
}


/**
 * Disable global breakpoint resource by modifying the psr.db bit of
 * the exception context and the switch context of the thread.
 *
 * @param tcb			tcb to enable breakpoints on
 */
INLINE void thread_resources_t::disable_global_breakpoint (tcb_t * tcb)
{
    // Disable breakpoints in current context.
    psr_t psr = get_psr ();
    psr.db = 0;
    set_psr_low (psr);
    ia64_srlz_d ();
    ia64_srlz_i ();

    ia64_exception_context_t * user_frame =
	(ia64_exception_context_t *) tcb->get_stack_top () - 1;
    ia64_switch_context_t * kernel_frame =
	(ia64_switch_context_t *) tcb->stack;

    user_frame->ipsr.db = kernel_frame->psr.db = 0;

    tcb->resource_bits -= BREAKPOINT;
}


/**
 * Enable global performance monitoring resource by modifying the
 * psr.db bit of the exception context and the switch context of the
 * thread.  The global performance monitoring resource is handed off
 * to the next thread during a thread switch.
 *
 * @param tcb			tcb to enable perfmon on
 */
INLINE void thread_resources_t::enable_global_perfmon (tcb_t * tcb)
{
    ia64_exception_context_t * user_frame =
	(ia64_exception_context_t *) tcb->get_stack_top () - 1;
    ia64_switch_context_t * kernel_frame =
	(ia64_switch_context_t *) tcb->stack;

    user_frame->ipsr.pp = user_frame->ipsr.up =
	kernel_frame->psr.pp = kernel_frame->psr.up = 1;

    tcb->resource_bits += PERFMON;
}


/**
 * Disable global performance monitoring resource by modifying the
 * psr.db bit of the exception context and the switch context of the
 * thread.
 *
 * @param tcb			tcb to enable perfmon on
 */
INLINE void thread_resources_t::disable_global_perfmon (tcb_t * tcb)
{
    // Disable performance counters in current context.
    psr_t psr = get_psr ();
    psr.pp = psr.up = 0;
    set_psr_low (psr);
    ia64_srlz_d ();
    ia64_srlz_i ();

    ia64_exception_context_t * user_frame =
	(ia64_exception_context_t *) tcb->get_stack_top () - 1;
    ia64_switch_context_t * kernel_frame =
	(ia64_switch_context_t *) tcb->stack;

    user_frame->ipsr.pp = user_frame->ipsr.up =
	kernel_frame->psr.pp = kernel_frame->psr.up = 0;

    tcb->resource_bits -= PERFMON;
}

#endif /* !__GLUE__V4_IA64__RESOURCE_FUNCTIONS_H__ */
