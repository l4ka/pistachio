/*********************************************************************
 *                
 * Copyright (C) 2003-2004,  National ICT Australia (NICTA)
 *                
 * File path:     platform/csb337/intctrl.h
 * Description:   
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
 * $Id: intctrl.h,v 1.1 2004/08/12 10:58:53 cvansch Exp $
 *                
 ********************************************************************/
#ifndef __PLATFORM__CSB337__INTCTRL_H__
#define __PLATFORM__CSB337__INTCTRL_H__

#include	INC_PLAT(aic.h)

/* Atmel AT91RM9200 Advanced Interrupt Controller (AIC) */
#define IRQS	32

extern word_t arm_high_vector; 

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
	AIC(AIC_SVR(vector)) = (word_t)handler;
	TRACE_INIT("interrupt vector[%d] = %p\n", vector, 
		(word_t*)AIC(AIC_SVR(vector)));
    }

    static inline void mask(word_t irq)
    {
	ASSERT(irq < IRQS);
	AIC(AIC_IDCR) = (1ul << irq);
	AIC(AIC_EOICR) = 0;	/* Signal that we have handled this interrupt */
    }

    static inline bool unmask(word_t irq)
    {
	ASSERT(irq < IRQS);
	AIC(AIC_IECR) = (1ul << irq);
	return false;
//        return (AIC(AIC_IPR) & (1 << irq)) != 0;
    }

    static inline void disable(word_t irq)
    {  
	mask(irq);
    }

    static inline bool enable(word_t irq)
    {
	return unmask(irq);
    }

    static inline void ack(word_t irq)
    {
	ASSERT(irq < IRQS);
	AIC(AIC_EOICR) = 0;	/* Signal that we have handled this interrupt */
    }

    void disable_fiq(void)
    {
	AIC(AIC_FFDR) = ~(0ul);	/* Disable Fast Forwarding */
    }

    bool is_irq_available(int irq)
    { 
	/* IRQ0 - Fast Interrupt not available */
	return (irq >= 1) && (irq < IRQS);  
    }

    void set_cpu(word_t irq, word_t cpu) {}
}; 

#endif /*__PLATFORM__CSB337__INTCTRL_H__ */
