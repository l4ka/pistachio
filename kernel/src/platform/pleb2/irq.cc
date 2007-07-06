/*********************************************************************
 *
 * Copyright (C) 2004,  National ICT Australia (NICTA)
 *
 * File path:     platform/pleb2/irq.cc
 * Description:   PLEB2 platform demultiplexing 
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
 * $Id: irq.cc,v 1.2 2004/12/02 22:40:43 cvansch Exp $
 *
 ********************************************************************/

#include <debug.h>
#include <linear_ptab.h>
#include INC_API(tcb.h)
#include INC_GLUE(intctrl.h)
#include INC_CPU(cpu.h)

extern "C" void arm_irq(arm_irq_context_t *context)
{
    word_t status = XSCALE_INT_ICIP;

    /* Handle timer first */
    if (status & (1ul << XSCALE_IRQ_OS_TIMER))
    {
	void (*irq_handler)(int, arm_irq_context_t *) =
		(void (*)(int, arm_irq_context_t *))interrupt_handlers[XSCALE_IRQ_OS_TIMER];
	irq_handler(XSCALE_IRQ_OS_TIMER, context);
	return;
    }

    /* XXX Use the clz instruction */
    for (int i = 7; i < IRQS; ++i)  /* 0..6 are reserved */
    {
	if (status & (1ul << i)) {
	    void (*irq_handler)(int, arm_irq_context_t *) =
		    (void (*)(int, arm_irq_context_t *))interrupt_handlers[i];

	    irq_handler(i, context); 
	    return;
	}
    }

    ASSERT(!"die");
}

void intctrl_t::init_arch()
{
}
