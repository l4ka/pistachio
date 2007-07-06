/*********************************************************************
 *                
 * Copyright (C) 2002-2004,  Karlsruhe University
 *                
 * File path:     l4/ia64/kdebug.h
 * Description:   IA64 kernel debugger interface
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
 * $Id: kdebug.h,v 1.9 2004/03/17 18:34:24 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __L4__IA64__KDEBUG_H__
#define __L4__IA64__KDEBUG_H__

#include <l4/types.h>

#define L4_KDB_Enter(s)						\
do {								\
    __asm__ __volatile__ (					\
	"{ .mlx					\n"		\
	"	break.m	0x3			\n"		\
	"	movl	r0 = 1f ;;		\n"		\
	"}					\n"		\
	"					\n"		\
	"	.rodata				\n"		\
	"1:	stringz " #s "			\n"		\
	"	.previous			\n"		\
	:							\
	:							\
	:							\
	"memory");						\
} while (0)


L4_INLINE void L4_KDB_ClearPage (void)
{
}

L4_INLINE void L4_KDB_PrintChar (char c)
{
    register char r14 asm ("r14") = c;

    asm volatile (
	"{ .mlx					\n"
	"	break.m	0x1			\n"
	"	movl	r0 = 0 ;;		\n"
	"}					\n"
	:
	:
	"r" (r14));
}

L4_INLINE void L4_KDB_PrinString (const char * s)
{
}

L4_INLINE char L4_KDB_ReadChar (void)
{
    register char r14 asm ("r14");

    asm volatile (
	"{ .mlx					\n"
	"	break.m	0x4			\n"
	"	movl	r0 = 0 ;;		\n"
	"}					\n"
	:							       
	"=r" (r14));

    return r14;
}

L4_INLINE char L4_KDB_ReadChar_Blocked (void)
{
    register char r14 asm ("r14");

    asm volatile (
	"{ .mlx					\n"
	"	break.m	0x2			\n"
	"	movl	r0 = 0 ;;		\n"
	"}					\n"
	:							       
	"=r" (r14));

    return r14;
}

#endif /* !__L4__IA64__KDEBUG_H__ */
