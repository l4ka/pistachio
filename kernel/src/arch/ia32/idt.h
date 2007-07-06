/*********************************************************************
 *                
 * Copyright (C) 2002,  Karlsruhe University
 *                
 * File path:     include/arch/ia32/idt.h
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
 * $Id: idt.h,v 1.4 2003/09/24 19:04:27 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __ARCH__IA32__IDT_H__
#define __ARCH__IA32__IDT_H__

class ia32_idtdesc_t 
{
public:
    enum type_e 
    {
	interrupt = 6,
	trap = 7
    };

    void set(u16_t segsel, void (*address)(), type_e type, int dpl);
    
private:
    union {
	u32_t raw[2];
	
	struct {
	    u32_t offset_low	: 16;
	    u32_t sel		: 16;
	    u32_t res0		:  8;
	    u32_t type		:  3;
	    u32_t d		:  1;
	    u32_t res1		:  1;
	    u32_t dpl		:  2;
	    u32_t p		:  1;
	    u32_t offset_high	: 16;
	} d;
    } x;
    friend class kdb_t;
};


/* ia32_idtdesc_t::set
 * sets an descriptor entry
 * - address is the offset of the handler in X86_KCS
 * - type selects Interrupt Gate or Trap Gate respectively
 * - dpl sets the numerical maximum CPL of allowed calling code
 */

INLINE void ia32_idtdesc_t::set(u16_t segsel, void (*address)(), 
				type_e type, int dpl)
{
    x.d.offset_low  = ((u32_t) address      ) & 0xFFFF;
    x.d.offset_high = ((u32_t) address >> 16) & 0xFFFF;
    x.d.sel = segsel;
    x.d.dpl = dpl;
    x.d.type = type;
    
    /* set constant values */
    x.d.p = 1;	/* present	*/
    x.d.d = 1;	/* size is 32	*/

    /* clear reserved fields */
    x.d.res0 = x.d.res1 = 0;
};


#endif /* !__ARCH__IA32__IDT_H__ */
