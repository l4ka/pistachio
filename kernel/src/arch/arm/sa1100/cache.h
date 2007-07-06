/*********************************************************************
 *
 * Copyright (C) 2003-2004,  National ICT Australia (NICTA)
 *
 * File path:     arm/sa1100/cache.h
 * Description:   Functions which manipulate the SA-1100 cache
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
 * $Id: cache.h,v 1.8 2005/01/12 02:52:40 cvansch Exp $
 *
 ********************************************************************/

#ifndef __PLATFORM__ARM__SA1100_CACHE_H_
#define __PLATFORM__ARM__SA1100_CACHE_H_

#include <debug.h>

#define ZERO_BANK_VADDR		(IO_AREA1_VADDR)

class arm_cache
{
public:

    static inline void cache_flush(void)
    {
	__asm__ __volatile__ (
	    "	mov	r0,	#0		\n"
	    "	mcr	p15, 0, r0, c7, c5, 0	\n" /* Invalidate I-cache */
	    "					\n" /* clean cache routine */
	    "	add	r1,	%0, #8192	\n" /* taken from sa1100   */
	    "1:					\n" /* reference manual    */
	    "	ldr	r2,	[%0], #32	\n" /* page 58 (sect 6.2.3)*/
	    "	teq	r1,	%0		\n"
	    "	bne	1b			\n"
	    "					\n"
	    /* Should probably also flush mini-D cache here if used */
	    "					\n"
	    "	mcr	p15, 0, r0, c7, c10, 4	\n" /* drain write buffer */
	    "					\n"
	    :: "r" (ZERO_BANK_VADDR)
	    : "r0", "r1", "r2", "memory");
    }

    static inline void cache_flushd(void)
    {
	__asm__ __volatile__ (
	    "					\n" /* clean cache routine */
	    "	add	r1,	%0, #8192	\n" /* taken from sa1100   */
	    "1:					\n" /* reference manual    */
	    "	ldr	r2,	[%0], #32	\n" /* page 58 (sect 6.2.3)*/
	    "	teq	r1,	%0		\n"
	    "	bne	1b			\n"
	    "	mov	r1,	#0		\n"
	    /* Should probably also flush mini-D cache here if used */
	    "					\n"
	    "	mcr	p15, 0, r1, c7, c10, 4	\n" /* drain write buffer */
	    "					\n"
	    :: "r" (ZERO_BANK_VADDR)
	    : "r1", "r2", "memory");
    }

    static inline void flush_icache_ent(addr_t vaddr, word_t log2size)
    {
	__asm__ __volatile__ (
	    "	mov	r0,	#0		\n"
	    "	mcr	p15, 0, r0, c7, c5, 0	\n" /* flush I cache */
	    "					\n"
	    ::
	    : "r0", "memory"); 
    }

    static inline void flush_dcache_ent(addr_t vaddr, word_t log2size)
    {
	if (log2size >= 15) {	/* for >32k, whole flush is better */
	    cache_flushd();
	    return;
	}
	word_t size = size = 1ul << log2size;

	__asm__ __volatile__ (
	    "					\n"
	    "	add	r0,	%0,	%1	\n"
	    "1:					\n"
	    "	mcr	p15, 0, %0, c7, c10, 1	\n" /* clean D cache line */
	    "	mcr	p15, 0, %0, c7, c6, 1	\n" /* flush D cache line */
	    "	add	%0,	%0,    #32	\n"
	    "	teq	r0,	%0		\n"
	    "	bne	1b			\n"
	    "	mov	r0,	#0		\n"
	    "					\n"
	    /* Should probably also flush mini-D cache here if used */
	    "					\n"
	    "	mcr	p15, 0, r0, c7, c10, 4	\n" /* drain write buffer */
	    "					\n"
	    : "+r" (vaddr)
	    : "r" (size)
	    : "r0", "memory");
    }

    static inline void flush_ent(addr_t vaddr, word_t log2size)
    {
	if (log2size >= 15) {	/* for >32k, whole flush is better */
	    cache_flush();
	    return;
	}
	word_t size = 1ul << log2size;

	__asm__ __volatile__ (
	    "	mov	r0,	#0		\n"
	    "	mcr	p15, 0, r0, c7, c5, 0	\n" /* flush I cache */
	    "	add	r0,	%0,	%1	\n"
	    "1:					\n"
	    "	mcr	p15, 0, %0, c7, c10, 1	\n" /* clean D cache line */
	    "	mcr	p15, 0, %0, c7, c6, 1	\n" /* flush D cache line */
	    "	add	%0,	%0,	#32	\n"
	    "	teq	r0,	%0		\n"
	    "	bne	1b			\n"
	    "	mov	r0,	#0		\n"
	    "					\n"
	    /* Should probably also flush mini-D cache here if used */
	    "					\n"
	    "    mcr     p15, 0, r0, c7, c10, 4	\n" /* drain write buffer */
	    "					\n"
	    : "+r" (vaddr)
	    : "r" (size)
	    : "r0", "memory");
    }

    static inline void cache_flush_debug(void)
    {
	printf("About to cache flush... ");
	cache_flush();
	printf("done.\n");
    }

    static inline void tlb_flush(void)
    {
	__asm__ __volatile__ (
	    "	mcr	p15, 0, r0, c8, c7, 0	\n"
	::); 
    }

    static inline void tlb_flush_ent(addr_t vaddr, word_t log2size)
    {
	word_t a = (word_t)vaddr;

        __asm__ __volatile__ (
	    "	mcr	p15, 0, r0, c8, c5, 0	\n"	/* Invalidate I TLB */
        ::); 
	for (word_t i=0; i < (1ul << log2size); i += ARM_PAGE_SIZE)
	{
	    __asm__ __volatile__ (
		"   mcr	    p15, 0, %0, c8, c6, 1   \n"	/* Invalidate D TLB entry */
	    :: "r" (a));
	    a += ARM_PAGE_SIZE;
	}
    }

    static inline void tlb_flush_debug(void)
    {
        printf("About to TLB flush... ");
        tlb_flush();
        printf("done.\n");
    }

    static inline void cache_invalidate(word_t target)
    {
        __asm__ __volatile__ (
	    "	mcr	p15, 0, %0, c7, c6, 1 \n"
        :: "r" (target));
    }

    static inline void cache_clean(word_t target)
    {
        __asm__ __volatile__ (
	    "	mcr	p15, 0, %0, c7, c10, 1 \n"
        :: "r" (target));
    }
};

#endif /* __PLATFORM__ARM__SA1100_CACHE_H_ */
