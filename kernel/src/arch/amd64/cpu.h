/*********************************************************************
 *                
 * Copyright (C) 2003, 2006,  Karlsruhe University
 *                
 * File path:     arch/amd64/cpu.h
 * Description:   X86-64 CPU Specific functions
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
 * $Id: cpu.h,v 1.4 2006/09/25 13:30:29 stoess Exp $
 *                
 ********************************************************************/
#ifndef __ARCH__AMD64__CPU_H__
#define __ARCH__AMD64__CPU_H__

#include INC_ARCH(amd64.h)

#ifndef ASSEMBLY 
INLINE u64_t amd64_rdpmc(const int ctrsel)
{
    u32_t __eax, __edx;
    
    __asm__ __volatile__ (
	"rdpmc"
	: "=a"(__eax), "=d"(__edx)
	: "c"(ctrsel));
    
    return ( (((u64_t) __edx) << 32) | ( (u64_t) __eax));	     
}

INLINE u64_t amd64_rdtsc(void)
{
    u32_t __eax, __edx;

    __asm__ __volatile__ (
	"rdtsc"
	: "=a"(__eax), "=d"(__edx));

    return ( (((u64_t) __edx) << 32) | ( (u64_t) __eax));	     
}

INLINE u64_t amd64_rdmsr(const u32_t reg)
{
    u32_t __eax, __edx;

    __asm__ __volatile__ (
	"rdmsr"
	: "=a"(__eax), "=d"(__edx)
	: "c"(reg)
	);

    return ( (((u64_t) __edx) << 32) | ( (u64_t) __eax));	     
}

INLINE void amd64_wrmsr(const u32_t reg, const u64_t val)
{
    __asm__ __volatile__ (
	"wrmsr"
	:
	: "a"( (u32_t) val), "d" ( (u32_t) (val >> 32)), "c" (reg));
}

/*
 * The AMD manual tells us that setting the TSC isn't a good idea. We still do
 * it for SMP synchronization; offsetting in software would be the alternative.
 */ 

INLINE void amd64_settsc(const u64_t val)
{
    amd64_wrmsr(0x10, val);
}

    
INLINE void amd64_wbinvd()
{
    __asm__ ("wbinvd\n" : : : "memory");
}

INLINE int amd64_lsb (word_t w) __attribute__ ((const));
INLINE int amd64_lsb (word_t w)
{
    int bitnum;
    __asm__ ("bsf %1, %0" : "=r" (bitnum) : "rm" (w));
    return bitnum;
}

INLINE int amd64_msb (word_t w) __attribute__ ((const));
INLINE int amd64_msb (word_t w)
{
    int bitnum;
    __asm__ ("bsr %1, %0" : "=r" (bitnum) : "rm" (w));
    return bitnum;
}

INLINE void amd64_cr0_set(const word_t val)
{
    word_t tmp;
    __asm__ __volatile__ ("mov	%%cr0, %0	\n"
			  "or	%1, %0		\n"
			  "mov	%0, %%cr0	\n"
			  : "=r"(tmp)
			  : "ri"(val));
}

INLINE void amd64_cr0_mask(const word_t val)
{
    word_t tmp;
    __asm__ __volatile__ ("mov	%%cr0, %0	\n"
			  "and	%1, %0		\n"
			  "mov	%0, %%cr0	\n"
			  : "=r"(tmp)
			  : "ri"(~val));
}

INLINE void amd64_cr4_set(const word_t val)
{
    word_t tmp;
    __asm__ __volatile__ ("mov	%%cr4, %0	\n"
			  "or	%1, %0		\n"
			  "mov	%0, %%cr4	\n"
			  : "=r"(tmp)
			  : "ri"(val));
}

INLINE void amd64_cr4_mask(const word_t val)
{
    word_t tmp;
    __asm__ __volatile__ ("mov	%%cr4, %0	\n"
			  "and	%1, %0		\n"
			  "mov	%0, %%cr4	\n"
			  : "=r"(tmp)
			  : "ri"(~val));
}

#endif /* ASSEMBLY */


#endif /* !__ARCH__AMD64__CPU_H__ */
