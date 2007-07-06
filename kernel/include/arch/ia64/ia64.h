/*********************************************************************
 *                
 * Copyright (C) 2002, 2003,  Karlsruhe University
 *                
 * File path:     arch/ia64/ia64.h
 * Description:   Some IA-64 specific CPU functions/definitions
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
 * $Id: ia64.h,v 1.5 2003/09/24 19:04:28 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __ARCH__IA64__IA64_H__
#define __ARCH__IA64__IA64_H__

#if !defined(ASSEMBLY)

INLINE void ia64_srlz_d (void)
{
    __asm__ __volatile__ ("srlz.d;;");
}

INLINE void ia64_srlz_i (void)
{
    __asm__ __volatile__ ("srlz.i;;");
}

INLINE void ia64_mf (void)
{
    __asm__ __volatile__ ("mf;;");
}

INLINE void ia64_mf_a (void)
{
    __asm__ __volatile__ ("mf.a;;");
}

INLINE bool ia64_cmpxchg (word_t * ptr, word_t old_val, word_t new_val)
{
    word_t stored_val;

    __asm__ __volatile__ (
	"	mov	ar.ccv = %2				\n"
	"	;;						\n"
	"	cmpxchg8.acq %0 = [%1], %3, ar.ccv		\n"
	:
	"=r" (stored_val)
	:
	"r" (ptr), "r" (old_val), "r" (new_val));

    return stored_val == old_val;
}

#endif /* !ASSEMBLY */

#endif /* !__ARCH__IA64__IA64_H__ */
