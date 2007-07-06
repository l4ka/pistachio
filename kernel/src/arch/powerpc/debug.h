/*********************************************************************
 *                
 * Copyright (C) 2002, Karlsruhe University
 *                
 * File path:	arch/powerpc/debug.h
 * Description:	debugging
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
 * $Id: debug.h,v 1.10 2003/09/24 19:04:30 skoglund Exp $
 *
 ********************************************************************/

#ifndef __ARCH__POWERPC__DEBUG_H__
#define __ARCH__POWERPC__DEBUG_H__

#define DEBUG_MAGIC_STR	"KD# "
#define DEBUG_IS_MAGIC(instr)	((((instr) >> 26) & 0x3f) == 18)

#if defined(CONFIG_DEBUG)

#include INC_GLUE(syscalls.h)

INLINE void spin_forever( int pos=0 )
{
    while( 1 );
}

INLINE void spin( int pos, int cpu=0 )
{
}

#define enter_kdebug(arg...)					\
    asm volatile (						\
	"li %%r0, %0 ;"						\
	"trap ;"						\
	"b 1f ;"						\
	".string	\"" DEBUG_MAGIC_STR arg "\";"		\
	".align 2 ;"						\
	"1:" 							\
	: /* outputs */						\
	: /* inputs */ "i" (L4_TRAP_KDEBUG) 			\
	: /* clobbers */ "r0" )

#endif	/* CONFIG_DEBUG */
#endif	/* __ARCH__POWERPC__DEBUG_H__ */
