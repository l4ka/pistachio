/*********************************************************************
 *                
 * Copyright (C) 2003,  Karlsruhe University
 *                
 * File path:     kdb/arch/ia64/interrupts.cc
 * Description:   Interrupt information
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
 * $Id: interrupts.cc,v 1.4 2003/09/24 19:05:06 skoglund Exp $
 *                
 ********************************************************************/
#include <debug.h>
#include <kdb/kdb.h>
#include <kdb/cmd.h>

#include INC_ARCH(cr.h)
#include INC_GLUE(intctrl.h)
#include INC_GLUE(context.h)


/**
 * Dump pending/masked interrupts.
 */
DECLARE_CMD (cmd_ia64_pending_ints, arch, 'I', "pendingints",
	     "dump pending/masked interrupts");

CMD (cmd_ia64_pending_ints, cg)
{
    ia64_exception_context_t * frame =
	(ia64_exception_context_t *) kdb.kdb_param;
    cr_tpr_t tpr = cr_get_tpr ();

    printf ("Pending/masked interrupts (Global IRQ offset = %d):\n",
	    INTERRUPT_VECTOR_OFFSET);
    printf ("  psr.i=%d,  tpr.mmi=%d  %s\n",
	    frame->ipsr.i, tpr.mmi,
	    (frame->ipsr.i == 0 || tpr.mmi) ? "(All interrupts masked)" : "");

    word_t max_irqs = get_interrupt_ctrl ()->get_number_irqs ();

    for (word_t i = 0; i < 256;)
    {
	bool deliver_pending = false;
	bool ack_pending = false;
	bool masked = false;

	if (i >= INTERRUPT_VECTOR_OFFSET &&
	    i < INTERRUPT_VECTOR_OFFSET + max_irqs)
	{
	    // Interrupt delivered through I/O SAPIC.

	    iosapic_redir_t ent = get_interrupt_ctrl ()->
		get_redir (i - INTERRUPT_VECTOR_OFFSET);
	    deliver_pending = ent.delivery_status != 0;
	    ack_pending = ent.remote_irr != 0;
	    masked = ent.mask != 0;
	}

	if ((i & 15) == 0)
	    printf ("  %3d-%d%s  [", i, i+15, (i+15) > 99 ? "" : " ");

	printf ("%c%c%c%c", deliver_pending ? 'd' : '~',
		is_interrupt_pending (i) ? 'p' : '~',
		ack_pending ? 'a' : '~', masked ? 'm' : '~');
	i++;

	if ((i & 15) == 0)
	    printf ("] %c\n",
		    ((i-1) < 16 || ((i-1) >> 4) > tpr.mic) ? ' ' : 'M');
	else if ((i & 7) == 0)
	    printf ("] [");
	else
	    printf ("|");
    }

    return CMD_NOQUIT;
}
