/*********************************************************************
 *                
 * Copyright (C) 2003-2004,  National ICT Australia (NICTA)
 *                
 * File path:     glue/v4-arm/intctrl.cc
 * Description:   Implementation of interrupt control functionality 
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
 * $Id: intctrl.cc,v 1.9 2004/08/12 11:36:36 cvansch Exp $
 *                
 ********************************************************************/

#include INC_GLUE(intctrl.h)

intctrl_t intctrl;
word_t interrupt_handlers[IRQS];

extern "C" void spurious_interrupt_handler(word_t irq,
        arm_irq_context_t * frame)
{
    printf("L4 ARM: Spurious interrupt %d\n", irq);
    enter_kdebug("Spurious interrupt");
}

extern "C" void intctrl_t_handle_irq(word_t irq, arm_irq_context_t * frame)
{   
    get_interrupt_ctrl()->mask(irq);
    handle_interrupt(irq);
}

void intctrl_t::init_cpu()
{
    get_kernel_space()->add_mapping((addr_t)ARM_HIGH_VECTOR_VADDR,
		(addr_t)virt_to_phys(&arm_high_vector),
			pgent_t::size_4k, true, true);

    disable_fiq();

    for (int i = 0; i < IRQS; ++i)
    {
	mask(i);
	register_interrupt_handler(i, intctrl_t_handle_irq);
    }
}
