/*********************************************************************
 *                
 * Copyright (C) 2003,  University of New South Wales
 *                
 * File path:     l4/sparc64/kdebug.h
 * Description:   SPARC v9 kernel debugger interface
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
 * $Id: kdebug.h,v 1.4 2004/02/15 23:05:04 philipd Exp $
 *                
 ********************************************************************/

#ifndef __L4__SPARC64__KDEBUG_H__
#define __L4__SPARC64__KDEBUG_H__

#define __L4_TRAP_KDB_PUTC		0x7c
#define __L4_TRAP_KDB_GETC		0x7d
#define __L4_TRAP_KDB_GETC_NB		0x7e
#define __L4_TRAP_KDB_ENTER		0x7f

#define L4_KDB_Enter(str...)					\
do {                         					\
    asm volatile (						\
	"setx	1f, %%g1, %%o0\n\t"				\
	"ldub	[ %%o0 ], %%g0\n\t"				\
	"ta	%0\n\t"						\
	".data\n\t"						\
	"1:	.string " #str "\n\t 2:"			\
	".previous"						\
	:: "i" (__L4_TRAP_KDB_ENTER) : "memory", "g1",		\
		"o0", "o1", "o2", "o3", "o4", "o5"		\
    );								\
} while(0)

#define __L4_KDB_Op_Arg(op, name, argtype) 			\
L4_INLINE void L4_KDB_##name(argtype arg)      			\
{                                          			\
    register char r_c asm("o0") = arg;				\
    __asm__ __volatile__ (					\
	"ta	%0"						\
	:: "r" (op), "r" (r_c) : "g1", "g4", "g5",		\
		"o1", "o2", "o3", "o4", "o5"			\
    );								\
} // L4_KDB_##name()

#define __L4_KDB_Op_Ret(op, name, rettype) 			\
L4_INLINE rettype L4_KDB_##name(void)				\
{								\
    register char ret asm("o0");				\
    __asm__ __volatile__ (					\
	"ta	%1"						\
	: "=r" (ret) : "r" (op) : "g1", "g4", "g5",		\
		"o1", "o2", "o3", "o4", "o5"			\
    );								\
    return ret;							\
} // L4_KDB_##name()


__L4_KDB_Op_Ret(__L4_TRAP_KDB_GETC, ReadChar_Blocked, char);
__L4_KDB_Op_Ret(__L4_TRAP_KDB_GETC_NB, ReadChar, char);
__L4_KDB_Op_Arg(__L4_TRAP_KDB_PUTC, PrintChar, char);


#endif /* !__L4__SPARC64__KDEBUG_H__ */
