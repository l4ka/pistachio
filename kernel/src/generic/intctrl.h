/*********************************************************************
 *                
 * Copyright (C) 2002-2003,  Karlsruhe University
 *                
 * File path:     intctrl.h
 * Description:   Interrupt controller
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
 * $Id: intctrl.h,v 1.9 2003/09/24 19:04:24 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __GENERIC__INTCTRL_H__
#define __GENERIC__INTCTRL_H__

#include <debug.h>	/* for UNIMPLEMENTED() */

/*
  This class is:
  - an interface description
  - a porting helper, the real intctrl_t can be derived from it
*/

class generic_intctrl_t {
 public:
    void mask(word_t irq)		 { UNIMPLEMENTED(); }
    // unmask returns true if an IRQ was already pending
    bool unmask(word_t irq)		 { UNIMPLEMENTED(); return false; }
    void mask_and_ack(word_t irq)	 { UNIMPLEMENTED(); }
    void ack(word_t irq)		 { UNIMPLEMENTED(); }
    void enable(word_t irq)		 { UNIMPLEMENTED(); }
    void disable(word_t irq)		 { UNIMPLEMENTED(); }

    /* set affinity/routing */
    void set_cpu(word_t irq, word_t cpu) { UNIMPLEMENTED(); }

    /* system-global initialization */
    void init_arch()			 { UNIMPLEMENTED(); }
    /* cpu-local initialization */
    void init_cpu()			 { UNIMPLEMENTED(); }
    
    word_t get_number_irqs()		 { UNIMPLEMENTED(); return 0; }
    bool is_irq_available(word_t irq)	 { UNIMPLEMENTED(); return false; }

    /* handler invoked on interrupt */
    void handle_irq(word_t irq)		 { UNIMPLEMENTED(); }
};

/* callback function */
void handle_interrupt(word_t irq);

#endif /* !__GENERIC__INTCTRL_H__ */
