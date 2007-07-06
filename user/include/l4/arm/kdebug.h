/*********************************************************************
 *
 * Copyright (C) 2003-2004,  National ICT Australia (NICTA)
 *                
 * File path:     l4/arm/kdebug.h
 * Description:   ARM kernel debugger interface
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
 * $Id: kdebug.h,v 1.10 2006/12/05 17:07:34 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __L4__ARM__KDEBUG_H__
#define __L4__ARM__KDEBUG_H__

#include <l4/types.h>
#include <l4/arm/syscalls.h>

/* FIXME - pass in str... */
#define L4_KDB_Enter(str...)			\
do {						\
	__asm__ __volatile__ (			\
		"mov	lr,	pc\n"		\
		"mov	pc,	%0\n"		\
		:				\
		: "r" (L4_TRAP_KDEBUG)		\
		: "lr");			\
} while (0)

#define __L4_KDB_Op_Arg(op, name, argtype)	\
L4_INLINE void L4_KDB_##name (argtype arg)	\
{						\
    __asm__ __volatile__ (			\
		"mov	r0,	%0\n"		\
		"mov	lr,	pc\n"		\
		"mov	pc,	%1\n"		\
		:				\
		: "r" (arg), "r" (op)		\
		: "r0", "lr", "memory" );	\
} 

#define __L4_KDB_Op_Ret(op, name, rettype)	\
L4_INLINE rettype L4_KDB_##name (void)		\
{						\
    register unsigned int ret;			\
    __asm__ __volatile (			\
		"mov	lr,	pc\n"		\
		"mov	pc,	%1\n"		\
		"mov	%0,	r0\n"		\
		: "=r" (ret)			\
		: "r" (op)			\
		: "r0", "lr", "memory");	\
    return ret;					\
}


__L4_KDB_Op_Ret(L4_TRAP_KGETC, ReadChar_Blocked, char );
__L4_KDB_Op_Ret(L4_TRAP_KGETC_NB, ReadChar, long );
__L4_KDB_Op_Arg(L4_TRAP_KPUTC, PrintChar, char );

#endif /* !__L4__ARM__KDEBUG_H__ */
