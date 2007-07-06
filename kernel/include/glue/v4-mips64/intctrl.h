/*********************************************************************
 *                
 * Copyright (C) 2002-2003,  University of New South Wales
 *                
 * File path:     glue/v4-mips64/intctrl.h
 * Description:   MIPS64 interrupt controller
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
 * $Id: intctrl.h,v 1.13 2004/06/04 02:32:31 cvansch Exp $
 *                
 ********************************************************************/
#ifndef __GLUE__V4_MIPS64__INTCTRL_H__
#define __GLUE__V4_MIPS64__INTCTRL_H__

#include <intctrl.h>
#include INC_GLUE(context.h)
#include INC_ARCH(mips_cpu.h)

extern word_t exception_handlers[32];
extern word_t interrupt_handlers[8];

class intctrl_t : public generic_intctrl_t {
 public:
	void init_arch();
	void init_cpu();

	void register_exception_handler (word_t vector, void *handler)
	{
	    ASSERT(vector < 32);
	    exception_handlers[vector] = (word_t)handler;
	    TRACE_INIT("exception vector[%d] = %p\n", vector, exception_handlers[vector]);
	}

	void register_interrupt_handler (word_t vector, void (*handler)(word_t, mips64_irq_context_t *))
	{
	    ASSERT(vector < 8);
	    interrupt_handlers[vector] = (word_t) handler;
	    TRACE_INIT("interrupt vector[%d] = %p\n", vector, interrupt_handlers[vector]);
	}

	static inline void mask(word_t irq)
	{
	    ASSERT(irq<8);
	    get_idle_tcb()->arch.int_mask &= ~(1<<irq);
	    mips_cpu::clear_cp0_status((1<<8)<<irq);
	}
	static inline bool unmask(word_t irq)
	{
	    ASSERT(irq<8);
	    get_idle_tcb()->arch.int_mask |= 1<<irq;
	    mips_cpu::set_cp0_status((1<<8)<<irq);
	    return false;
	}
	static inline void disable(word_t irq)
	{
	    ASSERT(irq<8);
	    get_idle_tcb()->arch.int_mask &= ~(1<<irq);
	    mips_cpu::clear_cp0_status((1<<8)<<irq);
	}
	static inline bool enable(word_t irq)
	{
	    ASSERT(irq<8);
	    get_idle_tcb()->arch.int_mask |= 1<<irq;
	    mips_cpu::set_cp0_status((1<<8)<<irq);
	    return false;
	}
	static inline void mask_and_ack(word_t irq)        { UNIMPLEMENTED(); }
	static inline void ack(word_t irq)                 { UNIMPLEMENTED(); }

	word_t get_number_irqs(void) 
	{ return 7; }

	bool is_irq_available(int irq) 
	{ return (irq < 7) && (irq >= 2); }

	void set_cpu(word_t irq, word_t cpu) {}
};


/**
 * @return pointer to interrupt controller
 */
INLINE intctrl_t * get_interrupt_ctrl() 
{
    extern intctrl_t intctrl;
    return &intctrl;
}

#endif /* !__GLUE__V4_MIPS64__INTCTRL_H__ */
