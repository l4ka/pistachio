/*********************************************************************
 *                
 * Copyright (C) 2003, 2007,  National ICT Australia (NICTA)
 *                
 * File path:     glue/v4-powerpc64/debug.h
 * Description:   Debug support
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
 * $Id: debug.h,v 1.3 2004/06/04 02:14:26 cvansch Exp $
 *                
 ********************************************************************/

#ifndef __ARCH__POWERPC64__DEBUG_H__
#define __ARCH__POWERPC64__DEBUG_H__

#include INC_GLUE(syscalls.h)

#define DEBUG_MAGIC_STR	"KD# "

#define SPIN_INTERRUPT  15 
#define SPIN_IDLE       30
#define SPIN_YIELD      45
#define SPIN_TIMER_INT  60
#define SPIN_IPC        75

#if defined(CONFIG_DEBUG)

INLINE void spin_forever(int pos = 0)
{
    while(1);
}

INLINE void spin(int pos = 0, int cpu = 0)
{
}

#define enter_kdebug(x)						\
    asm volatile (						\
	"mr	0, %0 ;"					\
	"trap ;"						\
	"b 1f ;"						\
	".string	\"" DEBUG_MAGIC_STR x"\";"		\
	".align 2 ;"						\
	"1:" 							\
	: /* outputs */						\
	: /* inputs */ "r" (L4_TRAP64_KDEBUG) 			\
	: /* clobbers */ "r0"					\
    )

#define HERE()               printf("Here %s:%d\n", __PRETTY_FUNCTION__, __LINE__)

#endif	/* CONFIG_DEBUG */

#endif /* __ARCH__POWERPC64__DEBUG_H__ */
