/*********************************************************************
 *                
 * Copyright (C) 2002, 2003,  Karlsruhe University
 *                
 * File path:     kdb/arch/ia64/frame.cc
 * Description:   Exception frame dumping
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
 * $Id: frame.cc,v 1.19 2003/09/24 19:05:06 skoglund Exp $
 *                
 ********************************************************************/
#include <debug.h>
#include <kdb/cmd.h>
#include <kdb/kdb.h>
#include <kdb/input.h>

#include INC_GLUE(context.h)
#include INC_API(tcb.h)


void ia64_dump_psr (psr_t psr);
void ia64_dump_frame (ia64_exception_context_t * frame);


/**
 * cmd_dump_current_frame: show exception frame of current thread
 */
DECLARE_CMD (cmd_dump_current_frame, root, ' ', "frame",
	     "show current exception frame");

CMD (cmd_dump_current_frame, cg)
{
    ia64_dump_frame ((ia64_exception_context_t *) kdb.kdb_param);
    return CMD_NOQUIT;
}


/**
 * cmd_dump_frame: show exception frame
 */
DECLARE_CMD (cmd_dump_frame, root, 'F', "dumpframe",
	     "show exception frame");

CMD (cmd_dump_frame, cg)
{
    tcb_t * tcb = addr_to_tcb (kdb.kdb_param);
    ia64_exception_context_t * f =
	(ia64_exception_context_t *) tcb->get_stack_top () - 1;

    word_t frame = get_hex ("Frame", (word_t) f, "current user frame");
    ia64_dump_frame ((ia64_exception_context_t *) frame);
    return CMD_NOQUIT;
}


word_t * next_nat_collection (word_t * addr)
{
    return (word_t *) (((word_t) (addr + 65) & ~0x1ff) - 8);
}

void ia64_dump_frame (ia64_exception_context_t * frame)
{
    word_t *rptr, i;

    printf ("Exception %d, frame @ %p:\n", frame->exception_num, frame);

    printf (" cr.iip:  %p  ar.ec:   %p  ar.lc:   %p\n"
	    " cr.ifa:  %p  cr.iim:  %p  cr.iha:  %p\n"
	    " ar.bsp:  %p  ar.unat: %p  ar.rnat: %p\n"
	    " ar.ccv:  %p  ar.rsc:  %p  cr.iipa: %p\n",
	    frame->iip, frame->ec, frame->lc,
	    frame->ifa, frame->iim, frame->iha,
	    frame->bspstore, frame->unat, frame->rnat,
	    frame->ccv, frame->rsc, frame->iipa);
    printf (" ar.fpsr: %p  [traps=%c%c%c%c%c%c  ",
	    frame->fpsr.raw,
	    frame->fpsr.traps_vd ? 'v' : '~',
	    frame->fpsr.traps_dd ? 'd' : '~',
	    frame->fpsr.traps_zd ? 'z' : '~',
	    frame->fpsr.traps_od ? 'o' : '~',
	    frame->fpsr.traps_ud ? 'u' : '~',
	    frame->fpsr.traps_id ? 'i' : '~');
    printf ("sf=(%03x %03x %03x %03x)]\n",
	    frame->fpsr.sf0, frame->fpsr.sf1,
	    frame->fpsr.sf2, frame->fpsr.sf3);
    printf (" cr.ipsr: %p  ", frame->ipsr);
    ia64_dump_psr (frame->ipsr);
    printf ("\n");
    printf (" cr.isr:  %p  [code: %4x, vec: %2x, slot: %d, acc: %c%c%c "
	    "%s%s%s%s%s%s%s\b]\n",
	    frame->isr.raw, frame->isr.code, frame->isr.vector,
	    frame->isr.instruction_slot,
	    frame->isr.rwx & 4 ? 'r' : '~', frame->isr.rwx & 2 ? 'w' : '~',
	    frame->isr.rwx & 1 ? 'x' : '~',
	    frame->isr.non_access ? "na " : "",
	    frame->isr.speculative_load ? "sp " : "",
	    frame->isr.register_stack ? "rs " : "",
	    frame->isr.incomplete_reg_frame ? "ir " : "",
	    frame->isr.nested_interruption ? "ni " : "",
	    frame->isr.supervisor_override ? "so " : "",
	    frame->isr.exception_deferral ? "ed " : "");
    printf (" ar.pfs:  %p  [size: %d (%d+%d), rot: %d, "
	    "rrb (gr: %d, fr: %d, pr: %d)]\n",
	    frame->pfs.raw, frame->pfs.framesize (),
	    frame->pfs.locals (), frame->pfs.outputs (), frame->pfs.sor,
	    frame->pfs.rrb_gr, frame->pfs.rrb_fr, frame->pfs.rrb_pr);
    printf (" cr.ifs:  %p  [size: %d (%d+%d), rot: %d, "
	    "rrb (gr: %d, fr: %d, pr: %d)]\n",
	    frame->ifs.raw, frame->ifs.framesize (),
	    frame->ifs.locals (), frame->ifs.outputs (), frame->ifs.sor,
	    frame->ifs.rrb_gr, frame->ifs.rrb_fr, frame->ifs.rrb_pr);
    printf (" cr.tpr:  %p  [mic: %d, mask %s]\n",
	    frame->tpr.raw, frame->tpr.mic, frame->tpr.mmi ? "all" : "mic");

    printf ("\nPredicate registers:\n p0-p63:  ");
    for (int i = 0; i <= 63; i++)
    {
	printf ("%d", frame->pr & (1UL << i) ? 1 : 0);
	if ((i & 7) == 7)
	    printf (" ");
    }

    printf ("\n\nBranch registers:\n");
    rptr = (word_t *) &frame->b0;
    for (i = 0; i <= 7; i++)
    {
	printf (" br%d: %p  ", i, *rptr++);
	if ((i & 3) == 3)
	    printf ("\n");
    }

    printf ("\nGeneral registers (num dirty = %d):\n", frame->num_dirty/8);
    rptr = (word_t *) &frame->r1;
    for (i = 0; i <= 31; i++)
    {
	word_t nat = frame->unat_kern & (1UL << (((word_t) rptr >> 3) & 0x3f));
	word_t reg = (i == 0) ? 0 : (i == 12) ? (word_t) frame->r12 : *rptr++;

	printf (" r%d%s %p%c ", i, i <= 9 ? ": " : ":", reg, nat ? '*' : ' ');
	if ((i & 3) == 3)
	    printf ("\n");
    }

    // Determine whether registers were dumped on empty kernel stack
    // (i.e., user exception) or the existing kernel stack (kernel
    // exceptions)
    tcb_t * tcb = addr_to_tcb ((addr_t) frame);
    if (frame->ipsr.cpl == 3 ||
	get_current_space ()->is_user_area (frame->bspstore))
	rptr = (word_t *) tcb->get_reg_stack_bottom ();
    else
	rptr = (word_t *) frame->bspstore;

    // Skip dirty registers which are not part of stack frame
    if (frame->num_dirty/8 > frame->ifs.framesize ())
    {
	word_t * tmp = rptr;
	rptr += frame->num_dirty/8 - frame->ifs.framesize ();

	// Adjust for NaT collection in dirty partition
	if (next_nat_collection (tmp) < rptr + frame->ifs.framesize () &&
	    next_nat_collection (tmp) >= rptr)
	    rptr--;
    }

    // Determine NaT collection for lower registers
    word_t rnat;
    if (next_nat_collection (rptr) < frame->bspstore_kern)
	rnat = *next_nat_collection (rptr);
    else
	rnat = frame->rnat_kern;

    for (i = 0; i < frame->ifs.framesize (); i++, rptr++)
    {
	if (((word_t) rptr & 0x1f8) == 0x1f8)
	{
	    // Change NaT collection
	    i--;
	    rnat = frame->rnat_kern;
	    continue;
	}

	word_t reg;
	bool mapped = true;

	if (get_current_space ()->is_user_area (rptr))
	    mapped = get_current_space ()->readmem (rptr, &reg);
	else
	    reg = *rptr;

	word_t nat = rnat & (1UL << (((word_t) rptr >> 3) & 0x3f));

	if (mapped)
	    printf (" r%d: %p%c ", i+32, reg, nat ? '*' : ' ');
	else
	    printf (" r%d: ################? ", i+32);

	if ((i & 3) == 3)
	    printf ("\n");
    }
    if ((--i & 3) != 3)
	printf ("\n");
}

