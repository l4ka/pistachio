/*********************************************************************
 *                
 * Copyright (C) 2002, 2003,  Karlsruhe University
 *                
 * File path:     glue/v4-ia64/intctrl.h
 * Description:   IA64 interrupt controller
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
 * $Id: intctrl.h,v 1.11 2003/09/24 19:04:37 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __GLUE__V4_IA64__INTCTRL_H__
#define __GLUE__V4_IA64__INTCTRL_H__

#include <intctrl.h>
#include INC_GLUE(context.h)
#include INC_ARCH(iosapic.h)

extern word_t interrupt_vector[256];


#define IVEC_IPI		(238)
#define IVEC_TIMER		(239)
#define IVEC_PERFMON		(240)


/**
 * Offset for first interrupt vector number available to user.
 */
#define INTERRUPT_VECTOR_OFFSET		(16)


#define INTCTRL_REDIR_ENTRIES		(CONFIG_MAX_IOAPICS * 64)

class intctrl_t : public generic_intctrl_t
{
    class iosapic_redir_entry_t {
    public:
	iosapic_redir_t		entry;
	iosapic_t		*iosapic;
	word_t			line;
	bool			pending;

	/**
	 * Initialize and invalidate I/O SAPIC redirection entry.
	 */
	void init (void)
	    { iosapic = NULL; line = ~0UL; pending = false; }

	/**
	 * Set I/O SAPIC redirection entry.
	 * @param apic		address of I/O SAPIC
	 * @param irq		IRQ line on I/O SAPIC
	 */
	void set (iosapic_t * apic, word_t irq)
	    { iosapic = apic; line = irq; }

	/**
	 * Determine whether IRQ is a valid interrupt line.
	 * @return true if valid, false otherwise
	 */
	bool is_valid (void)
	    { return line != ~0UL; }
    };

    iosapic_redir_entry_t redir[INTCTRL_REDIR_ENTRIES];
    word_t num_irqs;

public:

    // Generic API.

    void init_arch (void);
    void set_cpu (word_t irq, word_t cpu);
    void mask (word_t irq);
    bool unmask (word_t irq);
    void disable (word_t irq);
    void enable (word_t irq);
    
    void handle_irq (word_t irq);
    word_t get_number_irqs (void);
    bool is_irq_available (word_t irq);

    // Architecture dependent API.

    void init_iosapic (iosapic_t * iosapic, word_t irq_base);

    void register_handler (word_t vector,
			   void (*handler)(word_t, ia64_exception_context_t *))
	{ interrupt_vector[vector] = *(word_t *) handler; }

    void synch (word_t irq, bool low_only = false)
	{
	    redir[irq].iosapic->set_redir (redir[irq].line,
					   redir[irq].entry,
					   low_only);
	}

    void eoi (word_t irq)
	{ redir[irq].iosapic->eoi (irq + INTERRUPT_VECTOR_OFFSET); }

    iosapic_redir_t get_redir (word_t irq)
	{
	    iosapic_redir_t ret;
	    ret = redir[irq].iosapic->get_redir (redir[irq].line);
	    return ret;
	}
};


/**
 * @return pointer to interrupt controller
 */
INLINE intctrl_t * get_interrupt_ctrl (void) 
{
    extern intctrl_t intctrl;
    return &intctrl;
}

#endif /* !__GLUE__V4_IA64__INTCTRL_H__ */
