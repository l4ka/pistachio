/*********************************************************************
 *
 * Copyright (C) 2004,  National ICT Australia (NICTA)
 *
 * File path:     arm/omap1510/cache.h
 * Description:   Functions which manipulate the OMAP1510 cache
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
 * $Id: cache.h,v 1.2 2004/06/04 02:14:23 cvansch Exp $
 *
 ********************************************************************/

#ifndef __PLATFORM__ARM__OMAP1510__CACHE_H_
#define __PLATFORM__ARM__OMAP1510__CACHE_H_

#include <debug.h>


#define CACHE_SIZE	    8192
#define CACHE_LINE_SIZE	    16

class arm_cache
{
public:
    /* OMAP1510 dcache is 8KB write back, no lock-down, Harvard cache,
     * with 2-way associativity, 4 words per cache line. */
    static inline void cache_flush(void)
    {

    	word_t index;
	for (index = 0; index < CACHE_SIZE; index += CACHE_LINE_SIZE)
	{
	    /* clean and invalidate D cache */
	    __asm__ __volatile (
		"   mcr	    p15, 0, %0, c7, c14, 2	\n"
		:: "r" (index)
	    );
	}

	__asm__ __volatile__ (
	    "	mov	r0,	#0		\n"
	    "	mcr	p15, 0, r0, c7, c5, 0	\n" /* invalidate I caches */
	    "	mcr	p15, 0, r0, c7, c10, 4	\n" /* drain write buffer */
	    ::: "r0"
	);
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
	    "	 mov	r0,	#0		\n"
            "    mcr     p15, 0, r0, c8, c7, 0	\n"
	    ::: "r0"
	); 
    }

    static inline void tlb_flush_ent(addr_t vaddr, word_t log2size)
    {
	word_t a = (word_t)vaddr;

        __asm__ __volatile__ (
	    "	mov	r0,	#0		\n"
	    "	mcr     p15, 0, r0, c8, c5, 0	\n"	/* Invalidate I TLB */
	    ::: "r0"
	); 
	for (word_t i=0; i < (1ul << log2size); i += ARM_PAGE_SIZE)
	{
	    __asm__ __volatile__ (
		"   mcr	p15, 0, %0, c8, c6, 1    \n"	/* Invalidate D TLB entry */
		:: "r" (a)
	    );
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
            "mcr p15, 0, %0, c7, c6, 1 \n"
	    :: "r" (target)
	);
    }

    static inline void cache_clean(word_t target)
    {
        __asm__ __volatile__ (
            "mcr p15, 0, %0, c7, c10, 1 \n"
	    :: "r" (target)
	);
    }
};

#endif /* __PLATFORM__ARM__OMAP1510__CACHE_H_ */
