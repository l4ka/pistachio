/*********************************************************************
 *                
 * Copyright (C) 2002-2004,  Karlsruhe University
 *                
 * File path:     arch/ia64/debug.h
 * Description:   IA-64 specific debug functionality
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
 * $Id: debug.h,v 1.11 2006/10/19 22:57:35 ud3 Exp $
 *                
 ********************************************************************/
#ifndef __ARCH__IA64__DEBUG_H__
#define __ARCH__IA64__DEBUG_H__

#include INC_ARCH(rr.h)
#include INC_GLUE(hwspace.h)

#if !defined(CONFIG_CPU_IA64_SKI)
INLINE u16_t * __ia64_screen_addr (int pos)
{
    return (u16_t *) phys_to_virt_uc ((word_t) (0xb8000 + pos*2));
}

INLINE void spin (int pos, int cpu = 0)
{
#if defined(CONFIG_SPIN_WHEELS)
    volatile u8_t * addr = (u8_t *) __ia64_screen_addr (pos + cpu * 80);
    switch (addr[0])
    {
    default:
    case '-':  addr[0] = '\\'; break;
    case '\\': addr[0] = '|';  break;
    case '|':  addr[0] = '/';  break;
    case '/':  addr[0] = '-';  break;
    }
    addr[1] = 7;
#endif /* defined(CONFIG_SPIN_WHEELS) */
}

INLINE void spin_forever (int pos = 0, int cpu = 0)
{
    for (;;)
	spin (pos, cpu);
}

#else
INLINE void spin_forever (int pos = 0)
{
    for (;;);
}
INLINE void spin (int pos, int cpu = 0)
{
}
#endif

#if 1
#define enter_kdebug(x)						\
do {								\
    __asm__ __volatile__ (					\
	"// Put into separate bundle		\n"		\
	"{ .mlx					\n"		\
	"	break.m	0x3			\n"		\
	"	movl	r0 = 1f ;;		\n"		\
	"}					\n"		\
	"					\n"		\
	"	.rodata				\n"		\
	"1:	stringz " #x "			\n"		\
	"	.previous			\n"		\
	:							\
	:							\
	:							\
	"memory");						\
} while (0)
#else
#define enter_kdebug(x) _enter_kdebug(x)
#endif


#endif /* !__ARCH__IA64__DEBUG_H__ */
