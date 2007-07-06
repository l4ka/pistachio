/*********************************************************************
 *                
 * Copyright (C) 2006,  Karlsruhe University
 *                
 * File path:     l4/mips32/kdebug.h
 * Description:   L4 Kdebug interface for MIPS32
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
 * $Id: kdebug.h,v 1.1 2006/02/23 21:07:57 ud3 Exp $
 *                
 ********************************************************************/
#ifndef __L4__MIPS32__KDEBUG_H__
#define __L4__MIPS32__KDEBUG_H__

#define __L4_TRAP_KPUTC			(-100ul)
#define __L4_TRAP_KGETC			(-101ul)
#define __L4_TRAP_KDEBUG		(-102ul)
#define __L4_TRAP_KGETC_NB		(-104ul)
#define __L4_TRAP_READ_PERF		(-110ul)
#define __L4_TRAP_WRITE_PERF	(-111ul)

 //#define L4_KDB_Enter(str...)

#define L4_KDB_Enter(str...)			\
do {						\
    __asm__ __volatile__ (			\
	".set	noat\n\t"			\
	"li	$1, %0\n\t"			\
	"la	$2, 1f		    \n\t"	\
	"break			    \n\t"	\
	".set	at\n\t"				\
	"	.data		    \n\t"	\
	"1: .string	" #str ";"		\
	"	.previous	    \n\t"	\
	: : "i" (__L4_TRAP_KDEBUG) : "memory", "$1", "$2"	\
    );						\
} while (0)

#define __L4_KDB_Op_Arg(op, name, argtype)	\
L4_INLINE void L4_KDB_##name (argtype arg)	\
{						\
    register char r_c asm("$4") = arg;		\
    __asm__ __volatile__ (			\
	    ".set noat\n\t"			\
	    "li     $1, %0\n\t"			\
	    "break\n\t"				\
	    ".set at\n\t"			\
	    : : "i" (op), "r" (r_c) : "$1"	\
	    );					\
}


 #define __L4_KDB_Op_Ret(op, name, rettype)	\
 L4_INLINE rettype L4_KDB_##name (void)		\
 {						\
     register char ret asm("$2");		\
     __asm__ __volatile__ (			\
 	".set noat	\n\t"			\
 	"li	$1, %1	\n\t"			\
 	"break		\n\t"			\
 	".set at	\n\t"			\
 	: "=r" (ret) : "i" (op) : "$1"		\
     );						\
     return ret;					\
 }
 
 __L4_KDB_Op_Ret( __L4_TRAP_KGETC, ReadChar_Blocked, char );
 __L4_KDB_Op_Ret( __L4_TRAP_KGETC_NB, ReadChar, long );
 __L4_KDB_Op_Arg( __L4_TRAP_KPUTC, PrintChar, char );


#endif /* !__L4__MIPS32__KDEBUG_H__ */
