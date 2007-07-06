/*********************************************************************
 *                
 * Copyright (C) 2002,   University of New South Wales
 *                
 * File path:     kdb/arch/mips64/frame.cc
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
 * $Id: frame.cc,v 1.9 2004/01/06 01:12:36 cvansch Exp $
 *                
 ********************************************************************/
#include <debug.h>
#include <kdb/cmd.h>
#include <kdb/kdb.h>
#include <kdb/input.h>

#include INC_GLUE(context.h)
#include INC_API(tcb.h)

extern tcb_t * kdb_get_tcb();

void SECTION(SEC_KDEBUG) mips64_dump_frame(mips64_irq_context_t *context)
{
    printf ("== Stack frame: %p == \n", context);
    printf ("== STATUS: %8x == CAUSE: %16lx == EPC: %16lx\n", context->status, context->cause, context->epc);
    printf ("at = %16lx, v0 = %16lx, v1 = %16lx, sp = %16lx\n", context->at, context->v0, context->v1, context->sp);
    printf ("a0 = %16lx, a1 = %16lx, a2 = %16lx, a3 = %16lx\n", context->a0, context->a1, context->a2, context->a3);
    printf ("t0 = %16lx, t1 = %16lx, t2 = %16lx, t3 = %16lx\n", context->t0, context->t1, context->t2, context->t3);
    printf ("t4 = %16lx, t5 = %16lx, t6 = %16lx, t7 = %16lx\n", context->t4, context->t5, context->t6, context->t7);
    printf ("s0 = %16lx, s1 = %16lx, s2 = %16lx, s3 = %16lx\n", context->s0, context->s1, context->s2, context->s3);
    printf ("s4 = %16lx, s5 = %16lx, s6 = %16lx, s7 = %16lx\n", context->s4, context->s5, context->s6, context->s7);
    printf ("t8 = %16lx, t9 = %16lx, s8 = %16lx, gp = %16lx\n", context->t8, context->t9, context->s8, context->gp);
    printf ("ra = %16lx, hi = %16lx, lo = %16lx\n", context->ra, context->hi, context->lo);
}

void SECTION(SEC_KDEBUG) dump_fprs( tcb_t *tcb )
{
    u64_t * fprs = (u64_t *)&tcb->resources;
    for (int i = 0; i < 32; i++)
	printf ("f%d\t= %16lx\n", i, fprs[i]);
    printf("FPCSR\t= %16lx\n", fprs[32]);
}


/**
 * cmd_dump_current_frame: show exception frame of current thread
 */
DECLARE_CMD (cmd_dump_current_frame, root, ' ', "frame",
	     "show current user exception frame");

CMD (cmd_dump_current_frame, cg)
{
    mips64_irq_context_t *frame = (mips64_irq_context_t *)(kdb.kdb_param);

    mips64_dump_frame(frame);

//    printf("tcb = %p\n", addr_to_tcb((addr_t)kdb.kdb_param));
//    printf("stack = %p\n",addr_to_tcb((addr_t)kdb.kdb_param)->stack);

    return CMD_NOQUIT;
}


/**
 * cmd_dump_frame: show exception frame
 */
DECLARE_CMD (cmd_dump_frame, root, 'F', "dumpframe",
	     "show exception frame");


mips64_irq_context_t SECTION(SEC_KDEBUG) * get_frame()
{
    space_t * space = get_current_space();
    if (!space) space = get_kernel_space();
    word_t val = get_hex("tcb/tid/addr", (word_t)space->get_tcb(kdb.kdb_param), "current");
    mips64_irq_context_t * frame;

    if (val == ABORT_MAGIC)
	return NULL;

    if (!space->is_tcb_area((addr_t)val) &&
	((val & (~0xffful)) != (word_t)get_idle_tcb()))
    {
	threadid_t tid;
	tid.set_raw(val);
	frame = (mips64_irq_context_t *)((word_t)space->get_tcb(tid) + MIPS64_PAGE_SIZE);
	frame --;
    } else
    {
	frame = (mips64_irq_context_t *)val;
	if (frame == (mips64_irq_context_t *) addr_to_tcb ((addr_t) val))
	{
	    frame = (mips64_irq_context_t *)((word_t)frame + MIPS64_PAGE_SIZE);
	    frame --;
	}
    }
    return frame;

}

CMD (cmd_dump_frame, cg)
{
    mips64_irq_context_t *frame = get_frame();

    if (frame)
	mips64_dump_frame(frame);

    return CMD_NOQUIT;
}


/**
 * cmd_find_frame: search for an exception frame
 */
DECLARE_CMD (cmd_find_frame, root, 's', "findframe",
	     "search for an exception frame");

CMD (cmd_find_frame, cg)
{
    mips64_irq_context_t *frame, *search;
    space_t * space = get_current_space();
    word_t val = get_hex("tcb/tid/addr", (word_t)space->get_tcb(kdb.kdb_param), "current");
    word_t *addr;

    if (val == ABORT_MAGIC)
	return CMD_NOQUIT;

    if (!space->is_tcb_area((addr_t)val) &&
	((val & (~(MIPS64_PAGE_SIZE-1))) != (word_t)get_idle_tcb()))
    {
	threadid_t tid;
	tid.set_raw(val);
	frame = (mips64_irq_context_t *)((word_t)space->get_tcb(tid) + MIPS64_PAGE_SIZE);
	frame --;
    } else
    {
	frame = (mips64_irq_context_t *) ((val & (~(MIPS64_PAGE_SIZE-1))) + MIPS64_PAGE_SIZE);
	frame --;
    }

    for (addr = (word_t *) frame; (word_t) addr >= (word_t) frame - 2048; addr --) {
    	search = (mips64_irq_context_t *)addr;
    	if ((search->status & ~0x3ff1f) == 0x40000e0)
    	    mips64_dump_frame(search);
    }

    return CMD_NOQUIT;
}

/**
 * cmd_dump_fprs: dump floating point registers
 */
DECLARE_CMD (cmd_dump_fprs, root, 'f', "fpr",
	     "show floating point registers");


CMD (cmd_dump_fprs, cg)
{
    tcb_t * tcb = kdb_get_tcb();

    if (tcb) {
	tcb->resources.mips64_fpu_spill( tcb );
	dump_fprs( tcb );
    }

    return CMD_NOQUIT;
}

