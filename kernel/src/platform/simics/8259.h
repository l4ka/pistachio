/*********************************************************************
 *                
 * Copyright (C) 2002-2004, 2007,  Karlsruhe University
 *                
 * File path:     platform/simics/8259.h
 * Description:   Driver for i8259 Programmable Interrupt Controller
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
 * $Id: 8259.h,v 1.5 2006/10/19 22:57:36 ud3 Exp $
 *                
 ********************************************************************/
#ifndef __PLATFORM__PC99__8259_H__
#define __PLATFORM__PC99__8259_H__

#include INC_ARCH(ioport.h)	/* for in_u8/out_u8	*/

/**
 * Driver for i8259 PIC
 * @param base	the base address of the registers
 *
 * The template parameter BASE enables compile-time resolution of the
 * PIC's control register addresses.
 *
 * Note:
 *   Depending on whether CONFIG_X86_INKERNEL_PIC is defined or not
 *   objects will cache the mask register or not. Thus it is not wise
 *   to blindly instanciate them all over the place because the cached
 *   state would not be shared. Making the cached state static
 *   wouldn't work either because there are two PICs in a
 *   PC99. Intended use is a single object per PIC.
 *
 * Assumptions:
 * - BASE can be passed as port to in_u8/out_u8
 * - The PIC's A0=0 register is located at BASE
 * - The PIC's A0=1 register is located at BASE+1
 * - PICs in unbuffered cascade mode
 *
 * Uses:
 * - out_u8, in_u8
 */

template<u16_t base> class i8259_pic_t {
 private:
    
    u8_t mask_cache;
 public:

    /**
     *	Unmask interrupt
     *	@param irq	interrupt line to unmask
     */
    void unmask(word_t irq)
	{
	    u8_t mask_cache = in_u8(base+1);
	    mask_cache &= ~(1 << (irq)); 
	    out_u8(base+1, mask_cache);   
	}

    /**
     *	Mask interrupt
     *	@param irq	interrupt line to mask
     */
    void mask(word_t irq)
	{
	    u8_t mask_cache = in_u8(base+1);
	    mask_cache |= (1 << (irq)); 
	    out_u8(base+1, mask_cache);   
	}

    /**
     *	Send specific EOI
     *	@param irq	interrupt line to ack
     */
    void ack(word_t irq)
	{
	    out_u8(base, 0x60 + irq);   
	}

    /**
     *	Send specific EOI
     *	@param irq	interrupt line to ack
     */
    bool is_masked(word_t irq)
	{
	    return (mask_cache & (1 << irq));
	}
    

    /**
     *	initialize PIC
     *	@param vector_base	8086-style vector number base
     *	@param slave_info	slave mask for master or slave id for slave
     *
     *	Initializes the PIC in 8086-mode:
     *  - not special-fully-nested mode
     *	- reporting vectors VECTOR_BASE...VECTOR_BASE+7
     *	- all inputs masked
     */
    void init(u8_t vector_base, u8_t slave_info)
	{
	    mask_cache = 0xFF;
	    /*
	      ICW1:
	        0x10 | NEED_ICW4 | CASCADE_MODE | EDGE_TRIGGERED
	    */
	    out_u8(base, 0x11);

	    /*
	      ICW2:
	      - 8086 mode irq vector base
	        PIN0->IRQ(base), ..., PIN7->IRQ(base+7)
	    */
	    out_u8(base+1, vector_base);

	    /*
	      ICW3:
	       - master: slave list
	         Set bits mark input PIN as connected to a slave
	       - slave: slave id
	         This PIC is connected to the master's pin SLAVE_ID

	       Note: The caller knows whether its a master or not -
	             the handling is the same.
	    */
	    out_u8(base+1, slave_info);

	    /*
	      ICW4:
	        8086_MODE | NORMAL_EOI | NONBUFFERED_MODE | NOT_SFN_MODE
	     */
	    out_u8(base+1, 0x01); /* mode - *NOT* fully nested */

	    /*
	      OCW1:
	       - set initial mask
	    */
	    out_u8(base+1, mask_cache);
	    
	    //out_u8(base, 0x20);

	}
};

#endif /* !__PLATFORM__PC99__8259_H__ */
