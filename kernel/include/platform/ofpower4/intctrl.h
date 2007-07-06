/****************************************************************************
 *
 * Copyright (C) 2003,  National ICT Australia (NICTA)
 *
 * File path:	platform/ofpower4/intctrl.h
 * Description:	ofpower4 interrupt controller declarations (XICS).
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
 * $Id: intctrl.h,v 1.3 2004/06/04 02:11:06 cvansch Exp $
 *
 ***************************************************************************/

#ifndef __PLATFORM__OFPOWER4__INTCTRL_H__
#define __PLATFORM__OFPOWER4__INTCTRL_H__

class intctrl_t : public generic_intctrl_t {
 public:
	void init_arch();
	void init_cpu();

#if 0
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
#endif

	static inline void mask(word_t irq)
	{
	    UNIMPLEMENTED();
	}
	static inline bool unmask(word_t irq)
	{
	    UNIMPLEMENTED();
	    return false;
	}
	static inline void disable(word_t irq)
	{
	    UNIMPLEMENTED();
	}
	static inline bool enable(word_t irq)
	{
	    UNIMPLEMENTED();
	    return false;
	}
	static inline void mask_and_ack(word_t irq)        { UNIMPLEMENTED(); }
	static inline void ack(word_t irq)                 { UNIMPLEMENTED(); }

	/* For now, we only export 1 interrupt */
	word_t get_number_irqs(void) 
	{ return 1; }

	bool is_irq_available(int irq) 
	{ return (irq == 0); }

	void set_cpu(word_t irq, word_t cpu) { UNIMPLEMENTED(); }
};

#endif /* __PLATFORM__OFPOWER4__INTCTRL_H__ */
