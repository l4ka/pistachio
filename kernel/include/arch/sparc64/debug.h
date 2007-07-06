/*********************************************************************
 *                
 * Copyright (C) 2003-2004, University of New South Wales
 *                
 * File path:    arch/sparc64/debug.h
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
 * $Id: debug.h,v 1.5 2004/02/06 05:47:33 philipd Exp $
 *                
 ********************************************************************/

#ifndef __ARCH__SPARC64__DEBUG_H__
#define __ARCH__SPARC64__DEBUG_H__

#include <debug.h>

#define enter_kdebug(x)							\
do {									\
    word_t tl;								\
									\
    __asm__ __volatile__("rdpr\t%%tl, %0\t!Read trap level\n"		\
			 : "=r" (tl) /* %0 */ :);			\
									\
    printf("enter_kdebug: %s called from trap level 0x%lx\n", (char *)x, tl); \
									\
    __asm__ __volatile__("illtrap\t0x0\n"				\
			 : /* no outputs */ :);				\
									\
} while (0)

#warning awiggins (01-09-03): Might change this later, reboots so firmware gets control.
INLINE int spin_forever(int pos = 0)
{
    asm("sir\t0\t! Trap to firmware\n");

    return 0;
}

INLINE void spin(int pos = 0, int cpu = 0)
{
}


#endif /* !__ARCH__SPARC64__DEBUG_H__ */
