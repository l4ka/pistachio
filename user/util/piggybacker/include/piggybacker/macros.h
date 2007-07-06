/****************************************************************************
 *
 * Copyright (C) 2002-2003, Karlsruhe University
 *
 * File path:	include/piggybacker/macros.h
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
 * $Id: macros.h,v 1.3 2005/01/19 14:04:06 cvansch Exp $
 *
 ***************************************************************************/
#ifndef __PIGGYBACKER__INCLUDE__MACROS_H__
#define __PIGGYBACKER__INCLUDE__MACROS_H__

#include <l4/types.h>

#define PIGGYBACK_ARCH(file) <piggybacker/__L4_ARCH__/file>

L4_INLINE L4_Word_t wrap_up( L4_Word_t val, L4_Word_t size )
{
    if( val % size )
	val = (val + size) & ~(size-1);
    return val;
}

L4_INLINE L4_Word_t get_msr()
{
    L4_Word_t msr;
    __asm__ __volatile__ (
	"   mfmsr	%0	\n"
	: "=r" (msr)
    );
    return msr;
}

#define MSR_IR	    0x20
#define MSR_DR	    0x10

#endif	/* __PIGGYBACKER__INCLUDE__MACROS_H__ */
