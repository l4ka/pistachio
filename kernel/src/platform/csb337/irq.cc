/*********************************************************************
 *
 * Copyright (C) 2003-2004,  National ICT Australia (NICTA)
 *
 * File path:     platform/csb337/irq.cc
 * Description:   Cogent-CSB337 interrupt demultiplexing 
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
 * $Id: irq.cc,v 1.1 2004/08/12 11:17:37 cvansch Exp $
 *
 ********************************************************************/

#include <debug.h>
#include <linear_ptab.h>
#include INC_API(tcb.h)
#include INC_API(syscalls.h)
#include INC_API(kernelinterface.h)
#include INC_ARCH(thread.h)
#include INC_PLAT(console.h)
#include INC_GLUE(syscalls.h)
#include INC_GLUE(intctrl.h)
#include INC_CPU(cpu.h)

extern "C" void spurious_interrupt_handler(word_t , arm_irq_context_t *);

extern "C" void arm_irq(arm_irq_context_t *context)
{
    void (*irq_handler)(int, arm_irq_context_t *) =
	    (void (*)(int, arm_irq_context_t *))AIC(AIC_IVR);

    irq_handler(AIC(AIC_ISR), context); 
}

void intctrl_t::init_arch()
{
    int i;

    /* Program interrupt number into AIC_SVR */
    for (i=0; i < 32; i++)
	AIC(AIC_SVR(i)) = (word_t)spurious_interrupt_handler;

    AIC(AIC_IDCR) = ~(0ul);	/* Disable all interrupts */
    AIC(AIC_FFDR) = ~(0ul);	/* Disable all fast forwarding */
    AIC(AIC_DCR) = 0;	/* Turn off protection / global mask */
}

