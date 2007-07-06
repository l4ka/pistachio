/*********************************************************************
 *                
 * Copyright (C) 2003,  University of New South Wales
 *                
 * File path:     platform/miata/82C59.h
 * Description:   implementation for the 2 cascaded 82C59 PICs
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
 * $Id: 82C59.h,v 1.5 2003/10/19 06:15:22 sjw Exp $
 *                
 ********************************************************************/
#ifndef __PLATFORM__MIATA__82C59_H__
#define __PLATFORM__MIATA__82C59_H__

#include INC_ARCH(devspace.h)

class PIC82C59_t {
 private:
    /* one for primary, one for secondary */
    u8_t mask[2];
    u8_t level[2];

 public:
    enum trigger_e {
	IST_EDGE = 0,
	IST_LEVEL = 1
    };

    enum addrs_e {
	ICU_LEN = 16,
	IO_ICU0 = 0x020,
	IO_ICU1 = 0x0A0,
	IO_ICU_SIZE = 2,
	IO_ELCR = 0x4D0,
	IO_ELCR_SIZE = 2
    };

    void init() {
	/* All are initially edge */
	level[0] = 0;
	level[1] = 0;

	/* This may not always be necessary, but it can't hurt:
	 * Initialise primary controller.
	 */
	
	/* Write an initialise command (bit 4 for ICW select, bit 0 for ICW4 Write Required), */
	devspace_t::outb(IO_ICU0, 1 << 4 | 1);
	
	/* Write ICW2: 0 (not sure what Interrupt Vector Base Address should be) */
	devspace_t::outb(IO_ICU0 + 1, 0);

	/* Write ICW3: set cascade mode (bit 2) */
	devspace_t::outb(IO_ICU0 + 1, 1 << 2);

	/* Write ICW4: set bit 0, Microprocessor Mode (???) */
	devspace_t::outb(IO_ICU0 + 1, 1);
	
	/* Initialise the secondary controller */
	
	/* Write an initialise command (bit 4 for ICW select, bit 0 for ICW4 Write Required), */
	devspace_t::outb(IO_ICU1, 1 << 4 | 1);
	
	/* Write ICW2: 0 (not sure what Interrupt Vector Base Address should be) */
	devspace_t::outb(IO_ICU1 + 1, 0);

	/* Write ICW3: set slave id code (??) */
	devspace_t::outb(IO_ICU1 + 1, 1 << 1);

	/* Write ICW4: set bit 0, Microprocessor Mode (???) */
	devspace_t::outb(IO_ICU1 + 1, 1);

	mask[0] = (u8_t) ~0;
	mask[1] = (u8_t) ~0;

	printf("Mask is 0x%x 0x%x\n", mask[0], mask[1]);

	/* Mask all interrupts */
	devspace_t::outb(IO_ICU0 + 1, mask[0]);
	devspace_t::outb(IO_ICU1 + 1, mask[1]);
	
	/* unmask the cascade */
	enable(2);
    }

    void set_trigger(int irq, enum trigger_e trigger) {
	int ctrl = irq / 8;
	int bit = irq % 8;

	if(trigger == IST_LEVEL)
	    level[irq] |= (1 << bit);
	else
	    level[irq] &= ~(1 << bit);
	
	devspace_t::outb(IO_ELCR + ctrl, level[irq]);
    }

    /* I prefer this terminology to mask and unmask --- they can be ambiguous */
    void enable(int irq) {
	if(irq < 8) {
	    mask[0] &= ~(1 << irq);
	    devspace_t::outb(IO_ICU0 + 1, mask[0]);
	} else {
	    mask[1] &= ~(1 << (irq - 8));
	    devspace_t::outb(IO_ICU1 + 1, mask[1]);
	}
    }

    void disable(int irq) {
	if(irq < 8) {
	    mask[0] |= (1 << irq);
	    devspace_t::outb(IO_ICU0 + 1, mask[0]);
	} else {
	    mask[1] |= (1 << (irq - 8));
	    devspace_t::outb(IO_ICU1 + 1, mask[1]);
	}
    }

    void ack(int irq) {
	if(irq > 7) 
	    devspace_t::outb(IO_ICU1, (0x3 << 5) | irq - 8);

	/* ack int or cascade int */
	devspace_t::outb(IO_ICU0, (0x3 << 5) | (irq > 7 ? 2 : irq));
    }

    void print_status(void) {
	word_t mask = 0;
	word_t req = 0;

	mask = devspace_t::inb(IO_ICU0 + 1) | (devspace_t::inb(IO_ICU1 + 1) << 8);
	req = devspace_t::inb(IO_ICU0) | (devspace_t::inb(IO_ICU1) << 8);
	
	printf("82C59:\n");
	printf("\tMask:    0x%x\n", mask);
	printf("\tRequest: 0x%x\n", req);
    }
};

#endif /* !__PLATFORM__MIATA__82C59_H__ */
