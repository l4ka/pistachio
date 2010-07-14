/*********************************************************************
 *                
 * Copyright (C) 1999-2010,  Karlsruhe University
 * Copyright (C) 2008-2009,  Volkmar Uhlig, IBM Corporation
 *                
 * File path:     arch/powerpc/types.h
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
 * $Id$
 *                
 ********************************************************************/

#ifndef __ARCH__POWERPC__TYPES_H__
#define __ARCH__POWERPC__TYPES_H__


#define L4_32BIT
#undef  L4_64BIT


typedef unsigned int __attribute__((__mode__(__DI__))) u64_t;
typedef unsigned int		u32_t;
typedef unsigned short		u16_t;
typedef unsigned char		u8_t;

typedef signed int __attribute__((__mode__(__DI__))) s64_t;
typedef signed int		s32_t;
typedef signed short		s16_t;
typedef signed char		s8_t;

/**
 *	word_t - machine word wide unsigned int
 */
typedef u32_t			word_t;

#ifdef CONFIG_PLAT_PPC44X
/* 440 supports 36 bit addressing */
typedef u64_t			paddr_t;
#else
typedef u32_t			paddr_t;
#endif

INLINE paddr_t addr_offset(paddr_t addr, word_t off)
{
    return (paddr_t)(addr + off);
}


INLINE paddr_t addr_mask (paddr_t addr, word_t mask)
{
    return (paddr_t) (addr & mask);
}


/**
 * Counts the number of zeros starting at the msb (bit 31).
 * It returns 0 to 32 inclusive.
 */
INLINE word_t count_leading_zeros( word_t num )
{
    word_t val;
    asm volatile ("cntlzw %0, %1" : "=r" (val) : "r" (num) );
    return val;
}

INLINE word_t equivalent( word_t arg1, word_t arg2 )
{
    word_t result;
    asm volatile ("eqv %0, %1, %2" : "=r" (result) : "r" (arg1), "r" (arg2) );
    return result;
}

INLINE void sync( void )
{
    asm volatile ("sync");
}

INLINE void isync( void )
{
    asm volatile ("isync");
}

#ifdef CONFIG_PPC_BOOKE
INLINE void mbar( void )
{
    asm volatile ("mbar");
}

INLINE void msync( void )
{
    asm volatile ("msync");
}

INLINE void eieio( void )
{
    asm volatile ("eieio");
}
#endif

INLINE int lsb(word_t mask)
{
    return (sizeof(word_t)*8) - 1 - count_leading_zeros(mask & -mask);
}

#endif /* !__ARCH__POWERPC__TYPES_H__ */
