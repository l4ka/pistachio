/*********************************************************************
 *
 * Copyright (C) 2004,  National ICT Australia (NICTA)
 *
 * File path:     platform/pleb/intctrl.h
 * Description:   Functions which manipulate the SA-1100 interrupt controller
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
 * $Id: intctrl.h,v 1.5 2004/12/09 01:04:19 cvansch Exp $
 *
 ********************************************************************/

#ifndef __PLATFORM__PLEB__INTCTRL_H_
#define __PLATFORM__PLEB__INTCTRL_H_

#include <intctrl.h>
#include INC_GLUE(hwspace.h)
#include INC_ARCH(thread.h)
#include INC_API(space.h)
#include INC_PLAT(timer.h)

#define IRQS           32

#define SA1100_IRQ_OS_TIMER_0 26

#define SA1100_IBASE		(0x00050000UL + SA1100_OS_TIMER_BASE)
#define SA1100_ICIP		(*(volatile word_t *)(SA1100_IBASE + 0x00))
#define SA1100_ICMR		(*(volatile word_t *)(SA1100_IBASE + 0x04))
#define SA1100_ICLR		(*(volatile word_t *)(SA1100_IBASE + 0x08))
#define SA1100_ICFP		(*(volatile word_t *)(SA1100_IBASE + 0x10))
#define SA1100_ICPR		(*(volatile word_t *)(SA1100_IBASE + 0x20))
#define SA1100_ICCR		(*(volatile word_t *)(SA1100_IBASE + 0x0c))

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
        ASSERT(vector < IRQS);
        interrupt_handlers[vector] = (word_t) handler;
        TRACE_INIT("interrupt vector[%d] = %p\n", vector, 
                interrupt_handlers[vector]);
    }

    static inline void mask(word_t irq)
    {
        if (irq >= IRQS) printf("irq = %d\n", irq);
        ASSERT(irq < IRQS);
        SA1100_ICMR &= ~(1 << irq);
    }

    static inline bool unmask(word_t irq)
    {
        ASSERT(irq < IRQS);
        SA1100_ICMR |= (1 << irq);
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
	SA1100_ICLR = 0x0; /* No FIQs for now */
    }

    bool is_irq_available(int irq)
    { 
        return irq >= 0 && irq < IRQS && irq != SA1100_IRQ_OS_TIMER_0;  
    }

    void set_cpu(word_t irq, word_t cpu) {}


}; 

#endif /* __PLATFORM__PLEB__INTCTRL_H_ */
