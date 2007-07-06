/*********************************************************************
 *
 * Copyright (C) 2004,  National ICT Australia (NICTA)
 *
 * File path:     platform/innovator/irq.cc
 * Description:   TI Innovator interrupt demultiplexing 
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
 * $Id: irq.cc,v 1.3 2004/08/12 11:15:04 cvansch Exp $
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

extern "C" void
arm_irq (arm_irq_context_t * context)
{
    word_t irq_code, level, irq_base, irq;

    irq_base = io_to_virt (REG_ARM_IRQHDL1_BASE);
    irq_code = REG_IRQHDL_IRQ_CODE(irq_base);
    irq = irq_code;

    if (irq_code == 0)
    {
	irq_base = io_to_virt (REG_ARM_IRQHDL2_BASE);
	irq_code = REG_IRQHDL_IRQ_CODE(irq_base);
	irq = 32 + irq_code;
    }

    /* determine whether it is edge triggered or level triggered */
    level = *((volatile unsigned int *) (irq_base + ILR_BASE +
			    irq_code * 4)) & 0x2;

    void (*irq_handler) (int, arm_irq_context_t *) =
	(void (*)(int, arm_irq_context_t *)) interrupt_handlers[irq];

    get_interrupt_ctrl()->mask(irq);
    if (level)
    {
	/* XXX cvansch: I think this should happen before calling the handler? */
	/* need clear ITR. level trigger will not clear ITR when read IRQ_CODE */
	irq_handler (irq, context);
	if (irq >= 32)
	{
	    REG_IRQHDL2_CTL |= 1;
	}
	REG_IRQHDL_ITR(irq_base) &= ~1;
	REG_IRQHDL_CTL(irq_base) |= 1;
    }
    else /* edge.*/
    {
	irq_handler (irq, context);
	if (irq >= 32)
	{
	    REG_IRQHDL2_CTL |= 1;
	}
	REG_IRQHDL_ITR(io_to_virt(REG_ARM_IRQHDL1_BASE)) &= ~1;
	REG_IRQHDL_CTL(io_to_virt(REG_ARM_IRQHDL1_BASE)) |= 1;
    }

    get_interrupt_ctrl()->unmask(irq);
}

void intctrl_t::init_arch()
{
}
