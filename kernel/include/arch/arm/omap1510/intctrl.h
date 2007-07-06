/*********************************************************************
 *
 * Copyright (C) 2004,  National ICT Australia (NICTA)
 *
 * File path:     arch/omap1510/intctrl.h
 * Description:   Functions which manipulate the OMAP1510 interrupt controller
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
 * $Id: intctrl.h,v 1.3 2004/08/12 11:54:52 cvansch Exp $
 *
 ********************************************************************/

#ifndef __PLATFORM__INNOVATOR__INTCTRL_H_
#define __PLATFORM__INNOVATOR__INTCTRL_H_

#include <intctrl.h>
#include INC_GLUE(hwspace.h)
#include INC_ARCH(thread.h)
#include INC_API(space.h)
#include INC_CPU(io.h)

#define IRQS	    64

#define	REG_ARM_IRQHDL1_BASE	0xFFFECB00
#define	REG_ARM_IRQHDL2_BASE	0xFFFE0000
#define IRQHDL_ITR		0x0
#define	IRQHDL_MIR		0x4
#define IRQHDL_IRQ_CODE		0x10
#define IRQHDL_FIQ_CODE		0x14
#define IRQHDL_CTL_REG		0x18
#define	ILR_BASE		0x1C


#define REG_IRQHDL1_MIR *((volatile word_t*)((io_to_virt(REG_ARM_IRQHDL1_BASE)) + IRQHDL_MIR))
#define REG_IRQHDL2_MIR	*((volatile word_t*)((io_to_virt(REG_ARM_IRQHDL2_BASE)) + IRQHDL_MIR))
#define REG_IRQHDL2_CTL	*((volatile word_t*)((io_to_virt(REG_ARM_IRQHDL2_BASE)) + IRQHDL_CTL_REG))

#define REG_IRQHDL_IRQ_CODE(base)   *((volatile word_t*) ((base) + IRQHDL_IRQ_CODE))
#define REG_IRQHDL_ITR(base)	*((volatile word_t*) ((base) + IRQHDL_ITR))
#define REG_IRQHDL_CTL(base)	*((volatile word_t*) ((base) + IRQHDL_CTL_REG))

extern word_t arm_high_vector; 
extern word_t interrupt_handlers[IRQS];

class intctrl_t : public generic_intctrl_t {

public:
    void init_arch();
    void init_cpu();

    word_t get_number_irqs(void)
    {
        return IRQS;
    }

    void register_interrupt_handler (word_t vector, void (*handler)(word_t,
            arm_irq_context_t *))
    {
        ASSERT(vector >= 0 && vector < IRQS);
        interrupt_handlers[vector] = (word_t) handler;
        TRACE_INIT("interrupt vector[%d] = %p\n", vector, 
                interrupt_handlers[vector]);
    }

    static inline void mask(word_t irq)
    {
        ASSERT(irq >=0 && irq < IRQS);
	
	if (irq > 31)
	    REG_IRQHDL2_MIR |= (1 << (irq - 32));
	else
	    REG_IRQHDL1_MIR |= (1 << irq);

    }

    static inline bool unmask(word_t irq)
    {
        ASSERT(irq < IRQS);

	if (irq > 31)
	    REG_IRQHDL2_MIR &= ~(1<<(irq - 32));
	else
	    REG_IRQHDL1_MIR &= ~(1<<irq);

        return false;
    }

    static inline void disable(word_t irq)
    {  
        mask(irq);
    }

    static inline bool enable(word_t irq)
    {
        return unmask(irq);
    }

    void disable_fiq(void)
    {
      	/* No fiq control on interrupt handler,
	 * this should be done by setting CP15.
	 */
    }

    bool is_irq_available(int irq)
    {
	/* irq0 is used by indicating 2nd level irq. */
	return irq >= 0 && irq < IRQS;
    }

    void set_cpu(word_t irq, word_t cpu) {}
}; 

#endif /* __PLATFORM__INNOVATOR__INTCTRL_H_ */
