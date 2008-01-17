/*********************************************************************
 *                
 * Copyright (C) 2003, 2006-2008,  Karlsruhe University
 *                
 * File path:     arch/x86/x64/segdesc.h
 * Description:   paste ia32/segdesc.h, s/ia32/amd64
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
 * $Id: segdesc.h,v 1.5 2006/10/20 17:17:58 reichelt Exp $
 *                
 ********************************************************************/
#ifndef __ARCH__X86__X64__SEGDESC_H__
#define __ARCH__X86__X64__SEGDESC_H__

#include INC_ARCH(cpu.h)

/**
 * Code or Data Segment Descriptor for long/compatibility mode
 */
class x86_segdesc_t 
{
public:
    enum segtype_e
    {
	inv  = 0x0,
	code = 0xb,
	data = 0x3
    };
    
    enum mode_e
    {
	m_long = 1,
	m_comp = 0
    };

    enum msr_e
    {
	msr_none = 0,
	msr_fs = 1,
	msr_gs = 2
    };
    
    void set_seg(u64_t base, segtype_e type, int dpl, mode_e mode=m_long, msr_e msr=msr_none)	
	{
	    /* If we set FS or GS, we have to set MSR's for a 64bit base */ 
	    if (msr != msr_none && (base >> 32))
	    {
		u32_t reg = (msr == msr_fs) ? X86_X64_MSR_FS : X86_X64_MSR_GS;
		x86_wrmsr(reg, base);
	    }
    
	    x.d.base_low   = base & 0xFFFFFF;
	    x.d.base_high  = (base >> 24) & 0xFF;
    
	    x.d.limit_low  = 0xFFFF;
	    x.d.limit_high = 0xF;
    
	    x.d.g = 1;	
    
	    x.d.type = type;
	    x.d.l    = mode;
	    x.d.dpl  = dpl;
    
	    if (mode == m_long && type == code)
		x.d.d = 0;	/* code with L=1, D=0 => long mode */
	    else
		x.d.d = 1;	/* code with L=0, D=1 => compatibility mode
				   D=1 needed for data in compatibility mode */
    
	    /* default fields */
	    x.d.p = 1;		/* present		*/
	    x.d.s = 1;		/* non-system segment	*/
	    x.d.avl = 0;
	}

    void set_seg(u32_t base, segtype_e type, int dpl, mode_e mode=m_long)
	{
	    x.d.base_low   = base & 0xFFFFFF;
	    x.d.base_high  = (base >> 24) & 0xFF;
    
	    x.d.type = type;
	    x.d.l    = mode;
	    x.d.dpl = dpl;
	    x.d.g = 1;	
    
	    if (mode == m_long)
		x.d.d = 0;	/* L=1, D=0 =>long mode	*/
	    else
		x.d.d = 1;	/* L=0, D=1 => 32bit segment */
    
	    /* default fields */
	    x.d.p = 1;		/* present		*/
	    x.d.s = 1;		/* non-system segment	*/
	    x.d.avl = 0;
	}


private:
    union {
	u64_t raw;
	struct {
	    u64_t limit_low	: 16;
	    u64_t base_low	: 24;
	    u64_t type		:  4;
	    u64_t s		:  1;
	    u64_t dpl		:  2;
	    u64_t p		:  1;
	    u64_t limit_high	:  4;
	    u64_t avl		:  1;
	    u64_t l		:  1;
	    u64_t d		:  1;
	    u64_t g		:  1;
	    u64_t base_high	:  8;
	} d;
    } x;
    friend class kdb_t;
};

/* 
 * Limits are ignored for code/data segments in 64bit mode, 
 * addresses are ignored unless segment is selected by FS or GS
 */

#if !defined(X64_32BIT_CODE)

/**
 * TSS Descriptor for long/compatibility mode
 */
class x86_tssdesc_t 
{
public:
    void set_seg(u64_t base, u32_t limit);

private:
    union {
	u64_t raw[2];
	struct {
	    u64_t limit_low	: 16;
	    u64_t base_low	: 24;
	    u64_t type		:  4;
	    u64_t s		:  1;
	    u64_t dpl		:  2;
	    u64_t p		:  1;
	    u64_t limit_high	:  4;
	    u64_t avl		:  1;
	    u64_t res0		:  2;
	    u64_t g		:  1;
	    u64_t base_med	:  8;
	    u64_t base_high     : 32;
	    u64_t res1		:  8;
	    u64_t mbz		:  5;
	    u64_t res2		: 19;
	    
	} d;
    } x;
    friend class kdb_t;
};

/* 
 * Limits are checked 64bit mode, 
 * Addresses are ignored unless segment is for selected 
 * by FS or GS
 */   
INLINE void x86_tssdesc_t::set_seg(u64_t base, u32_t limit)
{
    x.d.base_low  = base & 0xFFFFFF;
    x.d.base_med  = (base >> 24) & 0xFF;
    x.d.base_high = (base >> 32) & 0xFFFFFFFF;
    
    if (limit > (1 << 20))
    {
	x.d.limit_low  = (limit >> 12) & 0xFFFF;
	x.d.limit_high = (limit >> 28) & 0xF;
	x.d.g = 1;      /* 4K granularity       */
    }
    else
    {
	x.d.limit_low  =  limit        & 0xFFFF;
	x.d.limit_high = (limit >> 16) & 0xFF;
	x.d.g = 0;      /* 1B granularity       */
    }
    
    x.d.type = 0x9;	/* 64bit TSS type	*/
    x.d.s = 0;		/* system segment	*/
    x.d.dpl =  0;	/* Privilege Level 0	*/
    x.d.p = 1;		/* present		*/
    x.d.avl = 0;
    x.d.mbz = 0;
    x.d.res0 = 0;
}


/**
 * IDT Descriptor for long mode
 * Note: Would look different for compatibility mode
 */

class x86_idtdesc_t 
{
   
public:
    enum segtype_e
    {
	interrupt = 0xe,
	trap      = 0xf
    };

    void set(u16_t selector, void (*address)(), segtype_e type, int dpl, int ist=0)
	{
	    x.d.offset_low = ( (u64_t) address & 0xFFFF );
	    x.d.offset_high = ( (u64_t) address >> 16);
	    x.d.selector   = selector;
	    x.d.ist = ist;
	    x.d.type = type;
	    x.d.dpl = dpl;
    
	    x.d.p = 1;		/* present */
	    x.d.s = 0;		/* system segment */
    
	    x.d.res0 = x.d.res1 = 0;
	}

private:
    union {
	u64_t raw[2];
	struct {
	    u64_t offset_low	: 16;
	    u64_t selector	: 16; 
	    u64_t ist		:  3;
	    u64_t res0		:  5;
	    u64_t type		:  4;
	    u64_t s		:  1;
	    u64_t dpl		:  2;
	    u64_t p		:  1;
	    u64_t offset_high	: 48 __attribute__((packed)); 
	    u64_t res1		: 32;
	} d;
    } x;
    friend class kdb_t;
};


#endif /* !X64_32BIT_CODE */

#endif /* !__ARCH__X86__X64__SEGDESC_H__ */
