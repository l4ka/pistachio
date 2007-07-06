/*********************************************************************
 *                
 * Copyright (C) 2006,  Karlsruhe University
 *                
 * File path:     arch/mips32/debug.h
 * Description:   Debug definitions for MIPS32
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
 * $Id: debug.h,v 1.1 2006/02/23 21:07:39 ud3 Exp $
 *                
 ********************************************************************/
#ifndef __ARCH__MIPS32__DEBUG_H__
#define __ARCH__MIPS32__DEBUG_H__

#include INC_GLUE(syscalls.h)

#define enter_kdebug(x) \
do { \
    __asm__ __volatile__ ( \
		".set	noat		\n\t" \
		"li	$1, %0			\n\t" \
		"la	$2, 1f			\n\t" \
		"break			    \n\t" \
		".set	at			\n\t" \
		"	.data		    \n\t" \
		"1:	.string " #x "  \n\t" \
		"	.previous	    \n\t" \
		: : "i" (L4_TRAP_KDEBUG) : "memory", "$1", "$2"	\
    ); \
} while (0)


INLINE int spin_forever(int pos = 0)  {
    while(1)
        ; // XXX
    return 0;
}

INLINE void spin(int pos = 0, int cpu = 0) {
    // XXX
}

#endif /* !__ARCH__MIPS32__DEBUG_H__ */
