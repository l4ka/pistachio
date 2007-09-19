/*********************************************************************
 *                
 * Copyright (C) 2002-2003, 2006-2007,  Karlsruhe University
 *                
 * File path:     glue/v4-amd64/debug.cc
 * Description:   Debugging support
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
 * $Id: debug.cc,v 1.4 2006/10/19 22:57:39 ud3 Exp $ 
 *                
 ********************************************************************/

#include <debug.h>
#include INC_API(kernelinterface.h)
#include INC_API(tcb.h)

#include INC_ARCHX(x86,traps.h)
#include INC_ARCH(trapgate.h)

#define KDB_STACK_SIZE	1024
static char kdb_stack[KDB_STACK_SIZE] 
    __attribute__ ((aligned (KDB_STACK_SIZE) ));

#define do_enter_kdebug(frame)		\
    ((tcb_t *)kdb_stack)->set_space( get_current_space() ? : get_kernel_space() );	\
    asm ("pushq %%rbp		\n"		\
	 "movq %%rsp, %%rbp	\n"		\
	 "movq %0, %%rsp	\n"		\
	 "pushq %%rbp		\n"		\
	 "mov %1, %%rdi		\n"		\
	 "callq *%2		\n"		\
	 "popq %%rsp		\n"		\
	 "popq %%rbp		\n"		\
	 :					\
         : "r"(&kdb_stack[KDB_STACK_SIZE]),	\
           "r"(frame),			\
           "r"(get_kip()->kdebug_entry) \
         : "memory");			

X86_EXCNO_ERRORCODE(exc_breakpoint, X86_EXC_BREAKPOINT)
{
    do_enter_kdebug(frame);
}

X86_EXCNO_ERRORCODE(exc_debug, X86_EXC_DEBUG)
{
    do_enter_kdebug(frame);
}

X86_EXCNO_ERRORCODE(exc_nmi, X86_EXC_NMI)
{
    do_enter_kdebug(frame);
}

