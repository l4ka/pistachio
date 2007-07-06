/*********************************************************************
 *                
 * Copyright (C) 2002-2004,  Karlsruhe University
 *                
 * File path:     glue/v4-ia64/resources.cc
 * Description:   Thread resource management
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
 * $Id: resources.cc,v 1.11 2004/06/01 14:43:30 skoglund Exp $
 *                
 ********************************************************************/
#include INC_API(tcb.h)
#include INC_ARCH(rr.h)
#include INC_GLUE(resources.h)

#include <kdb/tracepoints.h>


/**
 * Current owner of high floating-point registers.
 * 
 * High floating-point registers are not handled as a regular resource
 * (i.e., there is no resouce bit associated with owning the FP
 * registers).  Instead, the kernel enables/disables the high FP
 * registers by directly modifying the processor status register in
 * the user-contexts.  The invariant is that only the user-context of
 * FPHIGH_OWNER will have the high FP registers enabled.
 *
 * Not using a resource bit for the high FP registers enables most
 * thread switches to be performed without invoking a resource
 * save/load.  Enabling/disabling floating-point registers is as such
 * kept off the critical path.
 */
static tcb_t * fphigh_owner UNIT ("cpulocal");


DECLARE_TRACEPOINT (RESOURCES);
DECLARE_TRACEPOINT (DISABLED_FP);

DECLARE_KMEM_GROUP (kmem_resources);


void thread_resources_t::dump (tcb_t * tcb)
{
    if (tcb->resource_bits.have_resource (COPY_AREA))
	printf ("<copy-area, rid=%07x> ", partner_rid);
    if (tcb->resource_bits.have_resource (BREAKPOINT))
	printf ("<breakpoint> ");
    if (tcb->resource_bits.have_resource (PERFMON))
	printf ("<perfmon> ");

    if (tcb->resource_bits.have_resources ())
	printf ("\b");
}

void thread_resources_t::save (tcb_t * tcb, tcb_t * dest)
{
    TRACEPOINT (RESOURCES,
		printf ("Resources save: tcb=%p  rsc=%p [",
			tcb, (word_t) tcb->resource_bits);
		dump (tcb);
		printf ("]\n"));

    if (tcb->resource_bits.have_resource (COPY_AREA))
	disable_copy_area (tcb, false);

    if (tcb->resource_bits.have_resource (BREAKPOINT))
    {
	disable_global_breakpoint (tcb);
	enable_global_breakpoint (dest);
    }

    if (tcb->resource_bits.have_resource (PERFMON))
    {
	disable_global_perfmon (tcb);
	enable_global_perfmon (dest);
    }
}

void thread_resources_t::load (tcb_t * tcb)
{
    TRACEPOINT (RESOURCES,
		printf ("Resources load: tcb=%p  rsc=%p [",
			tcb, (word_t) tcb->resource_bits);
		dump (tcb);
		printf ("]\n"));

    if (tcb->resource_bits.have_resource (COPY_AREA))
    {
	// Associate the copy area region with the partner's space.
	rr_t rr (false, partner_rid, 12);
	rr.put (4);
	ia64_srlz_d ();
    }
}

void thread_resources_t::purge (tcb_t * tcb)
{
    TRACEPOINT (RESOURCES,
		printf ("Resources purge: tcb=%p  rsc=%p [",
			tcb, (word_t) tcb->resource_bits);
		dump (tcb);
		printf ("]\n"));

    if (tcb->resource_bits.have_resource (COPY_AREA))
	disable_copy_area (tcb, false);

    if (fphigh_owner == tcb)
    {
	// Spill registers to TCB and deassociate ownership
	ia64_enable_fphigh ();
	high_fp->save ();
	ia64_disable_fphigh ();
	fphigh_owner = NULL;
    }
}

void thread_resources_t::free (tcb_t * tcb)
{
    TRACEPOINT (RESOURCES,
		printf ("Resources free: tcb=%p  rsc=%p [",
			tcb, (word_t) tcb->resource_bits);
		dump (tcb);
		printf ("]\n"));

    if (tcb->resource_bits.have_resource (COPY_AREA))
	disable_copy_area (tcb, false);

    if (high_fp != NULL)
    {
	kmem.free (kmem_resources, high_fp, KB (2));
	high_fp = NULL;

	if (fphigh_owner == tcb)
	    // Deassociate ownership
	    fphigh_owner = NULL;
    }
}

void thread_resources_t::init (tcb_t * tcb)
{
    tcb->resource_bits.init ();
    if (tcb->is_interrupt_thread())
	    tcb->resource_bits += INTERRUPT_THREAD;
    partner_rid = 0;
    high_fp = NULL;
}



/**
 * Handle access to disabled floating-point registers.  The kernel
 * (potentially) stores the current FP context to the current owner's
 * TCB, disables FP register access for current owner, loads FP
 * context for new owner, and enables FP register access for new
 * owner.
 *
 * @param tcb		current thread
 * @param frame		exception frame
 */
void thread_resources_t::handle_disabled_fp (tcb_t * tcb,
					     ia64_exception_context_t * frame)
{
    TRACEPOINT (DISABLED_FP,
		printf ("FP disabled fault (%s%s):  cur=%p  owner=%p\n",
			frame->isr.code & 0x1 ? "low" : "",
			frame->isr.code & 0x2 ? "high" : "",
			tcb, fphigh_owner));

    ASSERT (fphigh_owner != tcb);	// Should always be different thread
    ASSERT (~frame->isr.code & 0x1);	// Should not fault on lower FP regs

    ia64_enable_fphigh ();
    if (fphigh_owner != NULL)
    {
	// Save FP context into current owner's TCB.
	fphigh_owner->resources.save_fp ();

	// Disable high FP for current owner.
	ia64_exception_context_t * ctx =
	    ((ia64_exception_context_t *) fphigh_owner->get_stack_top ()) - 1;
	ctx->ipsr.dfh = 1;
    }

    if (high_fp == NULL)
	high_fp = (high_fp_t *) kmem.alloc (kmem_resources, KB (2));

    // Restore FP context into registers.  Or, if thread is using high
    // FP for the first time, reset FP context with nil-values.
    high_fp->restore ();

    // Enable high FP for new owner.
    frame->ipsr.dfh = 0;
    fphigh_owner = tcb;
}


extern "C" void handle_disabled_fp (ia64_exception_context_t * frame)
{
    tcb_t * current = get_current_tcb ();
    current->resources.handle_disabled_fp (current, frame);
}
