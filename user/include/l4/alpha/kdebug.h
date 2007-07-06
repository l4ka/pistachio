/*********************************************************************
 *                
 * Copyright (C) 2002,   University of New South Wales
 *                
 * File path:     l4/alpha/kdebug.h
 * Description:   Alpha kernel debugger interface
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
 * $Id: kdebug.h,v 1.3 2003/09/24 19:06:21 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __L4__ALPHA__KDEBUG_H__
#define __L4__ALPHA__KDEBUG_H__

#include <l4/types.h>
#include <l4/alpha/pal.h>

#define  __L4_TRAP_KPUTC  0
#define  __L4_TRAP_KGETC  1
#define  __L4_TRAP_ENTER  2

#define L4_KDB_Enter(str...)					\
do {								\
    register L4_Word_t r16 asm("$16") = __L4_TRAP_ENTER;	\
    __asm__ __volatile__ (					\
	"lda        $17, 1f\n\t"				\
	"call_pal %0       \n\t"				\
	".data		    \n\t"				\
	"1:	.string \"KD# " str "\"	    \n\t"		\
	".previous	    \n\t"				\
	: : "i" (PAL_gentrap), "r" (r16) : "memory");		\
} while (0)

#define __L4_KDB_Op_Arg(op, name, argtype)		\
L4_INLINE void L4_KDB_##name (argtype arg)		\
{							\
       register L4_Word_t r16 asm("$16") = op;		\
       register char r17 asm("$17") = arg;		\
       __asm__ __volatile__ (				\
	"call_pal %0       \n\t"			\
	: : "i" (PAL_gentrap), "r" (r16), "r" (r17)	\
       );						\
}

#define __L4_KDB_Op_Ret(op, name, rettype)		\
L4_INLINE rettype L4_KDB_##name (void)			\
{							\
    register L4_Word_t r16 asm("$16") = op;		\
    register char ret asm("$0");			\
    __asm__ __volatile__ (				\
 	"call_pal %1       \n\t"			\
	: "=r" (ret) : "i" (PAL_gentrap), "r" (r16)	\
							\
    );							\
    return ret;						\
}

__L4_KDB_Op_Ret( __L4_TRAP_KGETC, ReadChar_Blocked, char );
__L4_KDB_Op_Arg( __L4_TRAP_KPUTC, PrintChar, char );

#endif /* !__L4__ALPHA__KDEBUG_H__ */
