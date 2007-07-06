/*********************************************************************
 *                
 * Copyright (C) 2002-2003,   University of New South Wales
 *                
 * File path:     platform/tsunami/intctrl.h
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
 * $Id: intctrl.h,v 1.6 2004/03/08 05:26:37 benno Exp $
 *                
 ********************************************************************/

#ifndef __PLATFORM__INTCTRL_H_
#define __PLATFORM__INTCTRL_H_

#include INC_ARCH(intctrl.h)
#include INC_PLAT(82C59.h)

#define TSUNAMI_IRQ_MASK (AS_KSEG_START + 0x101a0000200UL) // DIM0 (assumes CPU0)
#define TSUNAMI_IRQ_REQ  (AS_KSEG_START + 0x101a0000300UL) // DIRR

#define	NUM_SPECIAL_IRQS  8
#define	NUM_ISA_IRQS  16
#define	NUM_PCI_IRQS  64
#define	NUM_IRQS  (NUM_SPECIAL_IRQS + NUM_ISA_IRQS + NUM_PCI_IRQS)

#define	BASE_SPECIAL_IRQ  0
#define	BASE_ISA_IRQ (NUM_SPECIAL_IRQS + BASE_SPECIAL_IRQ)
#define	BASE_PCI_IRQ (NUM_ISA_IRQS + BASE_ISA_IRQ)
	
#define	MCHECK_IRQ  2
#define PERF_IRQ    4

class intctrl_t : public alpha_intctrl_t {
 private:

    word_t cached_irq_mask;
    PIC82C59_t sio;

 public:
    word_t get_number_irqs(void) 
	{ return NUM_IRQS; }

    bool is_irq_available(int irq) 
	{ return irq < NUM_IRQS && (irq >= BASE_ISA_IRQ || irq == MCHECK_IRQ || irq == PERF_IRQ); }

    void set_cpu(word_t irq, word_t cpu) {}

    word_t decode_irq(word_t irq) {
	word_t ret = 0;

	if (irq >= 0x800) {
	    ret = NUM_SPECIAL_IRQS + ((irq - 0x800) >> 4);
	} else {
	    printf("Got a weird device interrupt (vector 0x%lx)\n", irq);
	    enter_kdebug("Weird int");
	}
	
	return ret;
    }

    void mask(word_t irq) {
	if (irq >= BASE_PCI_IRQ) {
	    /* PCI Interrupt */
	    cached_irq_mask &= ~(1UL << (irq - BASE_PCI_IRQ));
	    *(volatile word_t *)TSUNAMI_IRQ_MASK = cached_irq_mask;

	} else if (irq >= BASE_ISA_IRQ) {
	    /* ISA Interrupt */
	    sio.disable(irq - BASE_ISA_IRQ);
	    sio.ack(irq - BASE_ISA_IRQ);
	} else {
	    /* `Special' interrupt */
	}
    }

    bool unmask(word_t irq) {
	if (irq >= BASE_PCI_IRQ) {
	    /* PCI Interrupt */
	    cached_irq_mask |= (1UL << (irq - BASE_PCI_IRQ));
	    *(volatile word_t *)TSUNAMI_IRQ_MASK = cached_irq_mask;
	} else if (irq >= BASE_ISA_IRQ) {
	    /* ISA Interrupt */
	    sio.enable(irq - BASE_ISA_IRQ);
	    sio.ack(irq - BASE_ISA_IRQ);

	} else {
	    /* `Special' interrupt */
	}

	return false;
    }

    void enable(word_t irq) {
	unmask(irq);
    }

    bool disable(word_t irq) {
	mask(irq);
	return false;
    }

    void ack(word_t irq) {
	if (irq >= BASE_PCI_IRQ) {
	    /* PCI Interrupt */
	    /* Necessary? Linux just masks */

	} else if (irq >= BASE_ISA_IRQ) {
	    /* ISA Interrupt */
	    sio.ack(irq - BASE_ISA_IRQ);

	} else {
	    /* `Special' interrupt */
	}
    }

    void init_arch() {
	sio.init();
	*(volatile word_t *)TSUNAMI_IRQ_MASK = 0;
	cached_irq_mask = 0;
    }

    void init_cpu() {}

    void print_status(void) {
	sio.print_status(); 
	printf("TSUNAMI:\n");
	printf("\tMask:     0x%016lx\n", *(volatile word_t *) TSUNAMI_IRQ_MASK);
	printf("\tRequest:  0x%016lx\n", *(volatile word_t *) TSUNAMI_IRQ_REQ);
	printf("\tActual:   0x%016lx\n", *(volatile word_t *) TSUNAMI_IRQ_MASK & *(volatile word_t *) TSUNAMI_IRQ_REQ);
    }
};



#endif /* __PLATFORM__INTCTRL_H__ */
