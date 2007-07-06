/*********************************************************************
 *                
 * Copyright (C) 2002-2004,  Karlsruhe University
 *                
 * File path:     platform/generic/intctrl-pic.h
 * Description:   PIC cascade in standard PCs
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
 * $Id: intctrl-pic.h,v 1.2 2006/10/19 22:57:36 ud3 Exp $
 *                
 ********************************************************************/
#ifndef __PLATFORM__GENERIC__INTCTRL_PIC_H__
#define __PLATFORM__GENERIC__INTCTRL_PIC_H__

#ifndef __GENERIC__INTCTRL_H__
#error not for standalone inclusion
#endif

#include INC_PLAT(8259.h)

class intctrl_t : public generic_intctrl_t {
 private:
    i8259_pic_t<0x20> master;
    i8259_pic_t<0xa0> slave;

 public:
    void init_arch();
    void init_cpu() { /* dummy */ };

    /* forward mask to the appropriate PIC */
    void mask(word_t irq) {
	(irq < 8) ? master.mask(irq) : slave.mask(irq-8);
    };

    /* forward mask to the appropriate PIC */
    bool unmask(word_t irq) {
	(irq < 8) ? master.unmask(irq) : slave.unmask(irq-8);
	return false;
    };

    /* check if interrupt is masked on  PIC */
    bool is_masked(word_t irq) {
	return (irq >= 8) ? slave.is_masked(irq-8) : master.is_masked(irq);
    }	

    void enable(word_t irq)	{ unmask(irq); }
    void disable(word_t irq)	{ mask(irq); }
    bool is_enabled(word_t irq) { return !is_masked(irq); }

    void ack(word_t irq) {
	if (irq >= 8)
	{
	    slave.ack(irq-8);
	    master.ack(2);
	}
	else
	    master.ack(irq);
    };
	
    void mask_and_ack(word_t irq) {
	mask(irq);
	ack(irq);
    };

    word_t get_number_irqs() { return 16; }

    bool is_irq_available(word_t irq);

    void set_cpu(word_t irq, word_t cpu) { /* dummy */ };

private:
    void handle_irq(word_t irq) __asm__("intctrl_t_handle_irq");
  
	
};

#endif /* !__PLATFORM__GENERIC__INTCTRL_PIC_H__ */
