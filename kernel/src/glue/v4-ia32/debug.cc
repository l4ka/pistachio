/*********************************************************************
 *                
 * Copyright (C) 2002-2003, 2006-2007,  Karlsruhe University
 *                
 * File path:     glue/v4-ia32/debug.cc
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
 * $Id: debug.cc,v 1.12 2006/06/19 08:01:59 stoess Exp $
 *                
 ********************************************************************/

#include <debug.h>
#include INC_API(kernelinterface.h)
#include INC_API(tcb.h)
#include INC_API(smp.h)

#include INC_ARCH(traps.h)
#include INC_ARCH(trapgate.h)
#include INC_ARCHX(x86,apic.h)

#define KDB_STACK_SIZE	KTCB_SIZE
static char kdb_stack[KDB_STACK_SIZE] UNIT("cpulocal")
    __attribute__ ((aligned (KDB_STACK_SIZE) ));

#define do_enter_kdebug(frame)		\
    ((tcb_t *)kdb_stack)->set_space( get_current_space() ? : get_kernel_space() );	\
asm ("pushl %%ebp	\n"		\
     "mov %%esp, %%ebp	\n"		\
     "mov %0, %%esp	\n"		\
     "pushl %%ebp	\n"		\
     "pushl %1		\n"		\
     "call *%2		\n"		\
     "addl $4, %%esp	\n"		\
     "popl %%esp	\n"		\
     "popl %%ebp	\n"		\
     :					\
     : "r"(&kdb_stack[KDB_STACK_SIZE]),	\
       "r"(frame),			\
       "r"(get_kip()->kdebug_entry)	\
     : "memory");			

IA32_EXC_NO_ERRORCODE(exc_breakpoint, IA32_EXC_BREAKPOINT)
{
    do_enter_kdebug(frame);
}

IA32_EXC_NO_ERRORCODE(exc_debug, IA32_EXC_DEBUG)
{
    do_enter_kdebug(frame);
}

IA32_EXC_NO_ERRORCODE(exc_nmi, IA32_EXC_NMI)
{
#ifdef CONFIG_DEBUG 
#if defined CONFIG_SMP
   local_apic_t<APIC_MAPPINGS> local_apic;
    void ia32_dump_frame (ia32_exceptionframe_t * frame);
    local_apic.EOI();
    printf("Current Frame:\n");
    ia32_dump_frame(frame); 
    printf("Current TCB:\n");
    dump_tcb(get_current_tcb());
    printf("CPU Mailbox:\n");
    get_cpu_mailbox (get_current_cpu())->dump_mailbox();
#else
    do_enter_kdebug(frame);
#endif
    
#endif /* CONFIG_DEBUG */
}
 

#ifdef CONFIG_SMP
IA32_EXC_NO_ERRORCODE(exc_debug_ipi, 0)
{
    
}
#endif
