/*********************************************************************
 *                
 * Copyright (C) 2002-2005, 2007-2008,  Karlsruhe University
 *                
 * File path:     arch/x86/tss.h
 * Description:   AMD64 Task State Segment
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
 * $Id: tss.h,v 1.4 2006/10/19 22:57:34 ud3 Exp $ 
 *                
 ********************************************************************/
#ifndef __ARCH__X86__X64__TSS_H__
#define __ARCH__X86__X64__TSS_H__

#include <debug.h> 

#if defined(CONFIG_X86_IO_FLEXPAGES)
#define X86_IOPERMBITMAP_BITS          (1 << 16)
#define X86_IOPERMBITMAP_ALIGNMENT     __attribute__((aligned(4096)));
#else
#define X86_IOPERMBITMAP_BITS 0
#define X86_IOPERMBITMAP_ALIGNMENT
#endif

#define IOPERMBITMAP_SIZE		(X86_IOPERMBITMAP_BITS / 8)


class x86_tss_t 
{
public:
    void setup(u16_t ss0=0);
    void set_sp0(u64_t sp0);
    u64_t get_sp0();
    addr_t get_io_bitmap();

private:
    u32_t	reserved0;
    u64_t	sp[3] __attribute__((packed));	      
    u64_t	reserved1;
    u64_t	ist[7] __attribute__((packed));	      
    u64_t	reserved2;
    u16_t	reserved3;
    u16_t	iopbm_offset;    
    u8_t	io_bitmap[IOPERMBITMAP_SIZE] X86_IOPERMBITMAP_ALIGNMENT;
    u8_t	stopper;
} __attribute__((packed));

INLINE void x86_tss_t::setup(u16_t ss0)
{
    iopbm_offset = (u16_t)((u64_t)io_bitmap - (u64_t)this);
    stopper = 0xff;
}

INLINE void x86_tss_t::set_sp0(u64_t sp0)
{
    sp[0] = sp0;
}

INLINE u64_t x86_tss_t::get_sp0()
{
    return sp[0];
}


INLINE addr_t x86_tss_t::get_io_bitmap()
{
    return (addr_t) io_bitmap;
}


extern x86_tss_t tss;

#if defined(CONFIG_IS_64BIT)
typedef x86_tss_t x86_tss_t;
#endif

#endif /* !__ARCH__X86__X64__TSS_H__ */
