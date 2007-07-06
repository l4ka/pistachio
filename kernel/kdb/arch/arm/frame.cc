/*********************************************************************
 *                
 * Copyright (C) 2003,  National ICT Australia (NICTA)
 *                
 * File path:     kdb/arch/arm/frame.cc
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
 * $Id: frame.cc,v 1.3 2004/12/09 01:24:41 cvansch Exp $
 *                
 ********************************************************************/
#include <debug.h>
#include <kdb/cmd.h>
#include <kdb/kdb.h>
#include <kdb/input.h>

#include INC_ARCH(thread.h)
#include INC_API(tcb.h)

extern tcb_t * kdb_get_tcb();

void SECTION(SEC_KDEBUG) arm_dump_frame(arm_irq_context_t *context)
{
    printf ("== Stack frame: %p == \n", context);
    printf ("cpsr = %8lx, pc = %8lx, sp  = %8lx, lr  = %8lx\n", context->cpsr, context->pc, context->sp, context->lr);
    printf ("r0  = %8lx, r1  = %8lx, r2  = %8lx, r3  = %8lx, r4  = %8lx\n", context->r0, context->r1, context->r2, context->r3, context->r4);
    printf ("r5  = %8lx, r6  = %8lx, r7  = %8lx, r8  = %8lx, r9  = %8lx\n", context->r5, context->r6, context->r7, context->r8, context->r9);
    printf ("r10 = %8lx, r11 = %8lx, r12 = %8lx\n", context->r10, context->r11, context->r12);
}

/**
 * cmd_dump_current_frame: show exception frame of current thread
 */
DECLARE_CMD (cmd_dump_current_frame, root, ' ', "frame",
	     "show current user exception frame");

CMD (cmd_dump_current_frame, cg)
{
    arm_irq_context_t *frame = (arm_irq_context_t *)(kdb.kdb_param);

    arm_dump_frame(frame);

    return CMD_NOQUIT;
}


/**
 * cmd_dump_frame: show exception frame
 */
DECLARE_CMD (cmd_dump_frame, root, 'F', "dumpframe",
	     "show exception frame");


arm_irq_context_t SECTION(SEC_KDEBUG) * get_frame()
{
    space_t * space = get_current_space();
    if (!space) space = get_kernel_space();
    word_t val = get_hex("tcb/tid/addr", (word_t)space->get_tcb(kdb.kdb_param), "current");
    arm_irq_context_t * frame;

    if (val == ABORT_MAGIC)
	return NULL;

    if (!space->is_tcb_area((addr_t)val) &&
	((val & (~0xffful)) != (word_t)get_idle_tcb()))
    {
	threadid_t tid;
	tid.set_raw(val);
	frame = (arm_irq_context_t *)((word_t)space->get_tcb(tid) + KTCB_SIZE);
	frame --;
    } else
    {
	frame = (arm_irq_context_t *)val;
	if (frame == (arm_irq_context_t *) addr_to_tcb ((addr_t) val))
	{
	    frame = (arm_irq_context_t *)((word_t)frame + KTCB_SIZE);
	    frame --;
	}
    }
    return frame;

}

CMD (cmd_dump_frame, cg)
{
    arm_irq_context_t *frame = get_frame();

    if (frame)
	arm_dump_frame(frame);

    return CMD_NOQUIT;
}


/**
 * cmd_find_frame: search for an exception frame
 */
DECLARE_CMD (cmd_find_frame, root, 's', "findframe",
	     "search for an exception frame");

CMD (cmd_find_frame, cg)
{
    arm_irq_context_t *frame, *search;
    word_t *addr;
    tcb_t * tcb = kdb_get_tcb();

    if (tcb)
    {
	tcb = (tcb_t *)((word_t)tcb + KTCB_SIZE);
	frame = (arm_irq_context_t *) (tcb) - 1;

	for (addr = (word_t *) frame; (word_t) addr >= (word_t) frame - 2048;
			addr --)
	{
	   search = (arm_irq_context_t *)addr;
	   if ((search->cpsr & 0x07ffffdf) == 0x00000010)
		arm_dump_frame(search);
	}
    }

    return CMD_NOQUIT;
}

