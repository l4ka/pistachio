/*********************************************************************
 *                
 * Copyright (C) 2002, 2003,  Karlsruhe University
 *                
 * File path:     arch/ia64/perf.h
 * Description:   Performance monitor functionality for IA64
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
 * $Id: perf.h,v 1.6 2004/02/24 23:56:25 cvansch Exp $
 *                
 ********************************************************************/
#ifndef __ARCH__IA64__PERF_H__
#define __ARCH__IA64__PERF_H__


/**
 * Generic performance monitor configuration register for ia64.
 */
class pmc_t
{
public:
    enum priv_e {
	none	= 0,
	user	= (1 << 0),
	kernel	= (1 << 3),
	both	= (1 << 0) | (1 << 3)
    };

    union {
	struct {
	    word_t plm		: 4;
	    word_t ev		: 1;
	    word_t oi		: 1;
	    word_t pm		: 1;
	    word_t __ig1	: 1;
	    word_t es		: 8;
	    word_t __impl	: 48;
	};
	word_t raw;
    };

    // Construction

    pmc_t (void) {}
    pmc_t (word_t w) { raw = w; }
    
    // Conversion

    operator word_t (void) { return raw; }
};



/**
 * Generic performance monitor data register for ia64
 */
class pmd_t
{
public:
    word_t raw;

    // Construction

    pmd_t (void) {}
    pmd_t (word_t w) { raw = w; }

    // Conversion

    operator word_t (void) { return raw; }
};

#define PMD_MASK ((word_t)0x00007fffffffffffull)

/**
 * Read performance monitor configuration register.
 * @param n		register number
 * @returns value of register
 */
INLINE pmc_t get_pmc (word_t n)
{
    pmc_t ret;
    __asm__ __volatile__ ("mov %0 = pmc[%1]" :"=r" (ret.raw) :"r" (n));
    return ret;
}

/**
 * Read performance monitor data register.
 * @param n		register number
 * @returns value of register
 */
INLINE pmd_t get_pmd (word_t n)
{
    pmd_t ret;
    __asm__ __volatile__ ("mov %0 = pmd[%1]" :"=r" (ret.raw) :"r" (n));
    return ret;
}

/**
 * Write performance monitor configuration register.
 * @param n		register number
 * @param value		value to be written
 */
INLINE void set_pmc (word_t n, pmc_t value)
{
    __asm__ __volatile__ (";;mov pmc[%0] = %1; ;;" ::"r" (n), "r" (value.raw));
}

/**
 * Write performance monitor data register.
 * @param n		register number
 * @param value		value to be written
 */
INLINE void set_pmd (word_t n, pmd_t value)
{
    __asm__ __volatile__ ("mov pmd[%0] = %1" ::"r" (n), "r" (value.raw));
}


/**
 * Wrapper class to access the performance monitor overflow status
 * registers.
 */
class pmc_overflow_t
{
public:
    bool is_overflowed (word_t n)
	{ return get_pmc (n / 64) & (1UL << (n & 63)); }

    void clr_overflow (word_t n)
	{ set_pmc (n / 64, get_pmc (n / 64) & ~(1UL << (n & 63))); }

    void clr_overflow (void)
	{
	    set_pmc (0, get_pmc (0) & 1);
	    set_pmc (1, 0); set_pmc (2, 0); set_pmc (3, 0);
	}

    bool is_frozen (void)
	{ return get_pmc (0) & 1; }

    void clr_freeze (void)
	{ set_pmc (0, get_pmc (0) & ~1UL); }

    void set_freeze (void)
	{ set_pmc (0, get_pmc (0) | 1); }
};


#endif /* !__ARCH__IA64__PERF_H__ */
