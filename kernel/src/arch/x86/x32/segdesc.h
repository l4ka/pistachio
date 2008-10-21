/*********************************************************************
 *                
 * Copyright (C) 2002, 2007-2008,  Karlsruhe University
 *                
 * File path:     arch/x86/x32/segdesc.h
 * Description:   IA32 Segment Descriptor
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
 * $Id: segdesc.h,v 1.5 2003/09/24 19:04:27 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __ARCH__X86__X32__SEGDESC_H__
#define __ARCH__X86__X32__SEGDESC_H__

class x86_segdesc_t 
{
public:
    enum segtype_e
    {
	code = 0xb,
	data = 0x3,
	tss  = 0x9
    };

    void set_seg(u32_t base, u32_t limit, int dpl, segtype_e type);
    void set_sys(u32_t base, u32_t limit, int dpl, segtype_e type);

private:
    union {
	u32_t raw[2];
	struct {
	    u32_t limit_low	: 16;
	    u32_t base_low	: 24 __attribute__((packed));
	    u32_t type		:  4;
	    u32_t s		:  1;
	    u32_t dpl		:  2;
	    u32_t p		:  1;
	    u32_t limit_high	:  4;
	    u32_t avl		:  2;
	    u32_t d		:  1;
	    u32_t g		:  1;
	    u32_t base_high	:  8;
	} d __attribute__((packed));
    } x;
    friend class kdb_t;
};

INLINE void x86_segdesc_t::set_seg(u32_t base, u32_t limit, 
				    int dpl, segtype_e type)
{
    if (limit > ( 1 << 20)) 
    {
	x.d.limit_low  = (limit >> 12) & 0xFFFF;
	x.d.limit_high = (limit >> 28) & 0xF;
	x.d.g = 1;	/* 4K granularity	*/
    }
    else
    {
	x.d.limit_low  = limit & 0xFFFF;
	x.d.limit_high = limit >> 16;
	x.d.g = 0;	/* 1B granularity	*/
    }

    x.d.base_low   = base & 0xFFFFFF;
    x.d.base_high  = (base >> 24) & 0xFF;
    x.d.type = type;
    x.d.dpl = dpl;
    
    /* default fields */
    x.d.p = 1;	/* present		*/
    x.d.d = 1;	/* 32-bit segment	*/
    x.d.s = 1;	/* non-system segment	*/
    
    /* unused fields */
    x.d.avl = 0;
}

INLINE void x86_segdesc_t::set_sys(u32_t base, u32_t limit, 
				    int dpl, segtype_e type)
{
    x.d.limit_low  = limit & 0xFFFF;
    x.d.limit_high = limit >> 16;
    x.d.base_low   = base        & 0xFFFFFF;
    x.d.base_high  = (base >> 24) &     0xFF;
    x.d.type = type;
    x.d.dpl = dpl;

    /* default fields */
    x.d.p = 1;	/* present		*/
    x.d.g = 0;	/* byte granularity	*/
    x.d.d = 0;	/* 32-bit segment	*/
    x.d.s = 0;	/* non-system segment	*/
    
    /* unused fields */
    x.d.avl = 0;
}


class x86_idtdesc_t 
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


/* x86_idtdesc_t::set
 * sets an descriptor entry
 * - address is the offset of the handler in X86_KCS
 * - type selects Interrupt Gate or Trap Gate respectively
 * - dpl sets the numerical maximum CPL of allowed calling code
 */

INLINE void x86_idtdesc_t::set(u16_t segsel, void (*address)(), 
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


#endif /* !__ARCH__X86__X32__SEGDESC_H__ */
