/*********************************************************************
 *                
 * Copyright (C) 2002-2007,  Karlsruhe University
 *                
 * File path:     glue/v4-x86/x64/syscalls.h
 * Description:   syscall macros
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
 * $Id: syscalls.h,v 1.11 2006/10/21 02:02:51 reichelt Exp $
 * 
 ********************************************************************/
#ifndef __GLUE_V4_X86__X64__SYSCALLS_H__
#define __GLUE_V4_X86__X64__SYSCALLS_H__

#include INC_ARCH(trapgate.h)

#if defined(CONFIG_X86_COMPATIBILITY_MODE)
#include INC_GLUE_SA(x32comp/thread.h)
#endif /* defined(CONFIG_X86_COMPATIBILITY_MODE) */


typedef struct {
    word_t rax;
    word_t rdx;
} x86_x64_sysret_t;


//
//	Ipc ()
//
//      GCC <  4.2.3  doesn't put to, from in registers
//	GCC >= 4.2.3 apparently does
//
//	the assembler stub in trap.S calls sys_ipc with from, to on the stack, and with
//	   rdi (arg1) = timeout
//	   rsi (arg2) = to
//	   rdx (arg3) = from
//

#define SYS_IPC(to, from, timeout)		\
    x86_x64_sysret_t SYSCALL_ATTR ("ipc")		\
	sys_ipc (timeout, to, from)		


#define return_ipc(from)			\
{						\
	x86_x64_sysret_t x86_x64_ret;		\
	x86_x64_ret.rax = (from).get_raw();	\
	x86_x64_ret.rdx = current->get_tag().raw;	\
	current->set_partner(from);		\
	return x86_x64_ret;			\
} 


//
//	ThreadControl ()
//
#define SYS_THREAD_CONTROL(dest, space, scheduler, pager, utcb_location)	\
  x86_x64_sysret_t SYSCALL_ATTR ("thread_control")				\
  sys_thread_control (dest, space, scheduler, pager, utcb_location)

#define return_thread_control(result)		\
{						\
	x86_x64_sysret_t x86_x64_ret;		\
	x86_x64_ret.rax = result;			\
	x86_x64_ret.rdx = 0;			\
	return x86_x64_ret;			\
}


//
//	SpaceControl ()
//
#define SYS_SPACE_CONTROL(space, control, kip_area, utcb_area,	\
			  redirector)				\
  x86_x64_sysret_t SYSCALL_ATTR ("space_control")			\
  sys_space_control (space, control, kip_area, utcb_area,	\
		     redirector)

#define return_space_control(result, control)	\
{						\
	x86_x64_sysret_t x86_x64_ret;		\
	x86_x64_ret.rax = result;			\
	x86_x64_ret.rdx = control;		\
	return x86_x64_ret;			\
} 


//
//	Schedule ()
//
#define SYS_SCHEDULE(dest, time_control, processor_control,	\
		     prio, preemption_control)			\
  x86_x64_sysret_t SYSCALL_ATTR ("schedule")			\
  sys_schedule (dest, time_control, processor_control,		\
		prio, preemption_control)

#define return_schedule(result, time_control)	\
{						\
	x86_x64_sysret_t x86_x64_ret;		\
	x86_x64_ret.rax = result;			\
	x86_x64_ret.rdx = time_control;		\
	return x86_x64_ret;			\
} 


//
//	ExchangeRegisters ()
//
//
#define SYS_EXCHANGE_REGISTERS(dest, control, usp, uip,         	\
                               uflags, uhandle, pager, is_local)	\
  void SYSCALL_ATTR ("exchange_registers")				\
  sys_exchange_registers (dest, control, usp, uip,			\
			  uflags, uhandle, pager, is_local)


#if defined(CONFIG_X86_COMPATIBILITY_MODE)
#define return_exchange_registers(result,					\
    cntrl, sp, ip, flags, pager, handle)					\
{										\
    word_t x86_x64_dummy, x86_x64_uip;						\
    tcb_t *x86_x64_current = get_current_tcb();					\
    utcb_t *utcb = x86_x64_current->get_utcb();					\
    struct {									\
	word_t       rdi;							\
	word_t       r8;							\
	word_t       r9;							\
	word_t       r10;							\
	word_t       r11;							\
	word_t       rsp;							\
    } x86_x64_ret;								\
    x86_x64_ret.rdi = pager.get_raw();						\
    x86_x64_ret.r8 = ip;								\
    x86_x64_ret.r9 = flags;							\
    x86_x64_ret.r10 = handle;							\
    x86_x64_ret.r11 = (word_t) x86_x64_current->get_user_flags();			\
    x86_x64_ret.rsp = (word_t) x86_x64_current->get_user_sp();			\
    x86_x64_uip = (word_t) x86_x64_current->get_user_ip();				\
    if (utcb->is_compatibility_mode())						\
    {										\
	utcb->exreg32.control = cntrl;						\
	__asm__ __volatile__("movq   (%[ret]), %%rbp	\n"			\
			     "movq 16(%[ret]), %%rdi	\n"			\
			     "movq 24(%[ret]), %%rbx	\n"			\
			     "movq 32(%[ret]), %%r11	\n"			\
			     "movq 40(%[ret]), %%rsp	\n"			\
			     "movq  8(%[ret]), %%rsi	\n"			\
			     "sysretl"						\
			     : /* outputs */					\
			     "=a" (x86_x64_dummy),	/* %0 RAX */		\
			     "=S" (x86_x64_dummy),	/* %1 RSI */		\
			     "=c" (x86_x64_dummy),	/* %2 RCX */		\
			     "=d" (x86_x64_dummy)		/* %3 RDX */		\
			     : /* inputs */					\
				    "0" (threadid_32(result)), /* %4 RAX */	\
			     [ret]  "1" (&x86_x64_ret),	/* %5 RSI */		\
				    "2" (x86_x64_uip),	/* %6 RCX */		\
				    "3" (sp)		/* %7 RDX */		\
			     /* no clobbers */					\
	);									\
    }										\
    __asm__ __volatile__("movq   (%[ret]), %%rdi	\n"			\
			 "movq  8(%[ret]), %%r8		\n"			\
			 "movq 16(%[ret]), %%r9		\n"			\
			 "movq 24(%[ret]), %%r10	\n"			\
			 "movq 32(%[ret]), %%r11	\n"			\
			 "movq 40(%[ret]), %%rsp	\n"			\
			 "sysretq"						\
			 : /* outputs */					\
			 "=a" (x86_x64_dummy),		/* %0 RAX */		\
			 "=b" (x86_x64_dummy),		/* %1 RBX */		\
			 "=c" (x86_x64_dummy),		/* %2 RCX */		\
			 "=d" (x86_x64_dummy),		/* %3 RDX */		\
			 "=S" (x86_x64_dummy)		/* %4 RSI */		\
			 : /* inputs */						\
				"0" (result),		/* %5 RAX */		\
			 [ret]	"1" (&x86_x64_ret),	/* %6 RBX */		\
				"2" (x86_x64_uip),	/* %7 RCX */		\
				"3" (sp),		/* %8 RDX */		\
				"4" (cntrl)		/* %9 RSI */		\
			 /* no clobbers */					\
	);									\
    while(1);									\
}
#else /* !defined(CONFIG_X86_COMPATIBILITY_MODE) */
#define return_exchange_registers(result, cntrl, sp, ip,			\
				  flags, pager, handle)				\
{										\
    word_t x86_x64_dummy, x86_x64_uip;						\
    tcb_t *x86_x64_current = get_current_tcb();					\
    struct {									\
	word_t       rdi;							\
	word_t       r8;							\
	word_t       r9;							\
	word_t       r10;							\
	word_t       r11;							\
	word_t       rsp;							\
    } x86_x64_ret;								\
    x86_x64_ret.rdi = pager.get_raw();						\
    x86_x64_ret.r8 = ip;								\
    x86_x64_ret.r9 = flags;							\
    x86_x64_ret.r10 = handle;							\
    x86_x64_ret.r11 = (word_t) x86_x64_current->get_user_flags();			\
    x86_x64_ret.rsp = (word_t) x86_x64_current->get_user_sp();			\
    x86_x64_uip = (word_t) x86_x64_current->get_user_ip();				\
    __asm__ __volatile__("movq   (%[ret]), %%rdi	\n"			\
			 "movq  8(%[ret]), %%r8		\n"			\
			 "movq 16(%[ret]), %%r9		\n"			\
			 "movq 24(%[ret]), %%r10	\n"			\
			 "movq 32(%[ret]), %%r11	\n"			\
			 "movq 40(%[ret]), %%rsp	\n"			\
			 "sysretq"						\
			 : /* outputs */					\
			 "=a" (x86_x64_dummy),		/* %0 RAX */		\
			 "=b" (x86_x64_dummy),		/* %1 RBX */		\
			 "=c" (x86_x64_dummy),		/* %2 RCX */		\
			 "=d" (x86_x64_dummy),		/* %3 RDX */		\
			 "=S" (x86_x64_dummy)		/* %4 RSI */		\
			 : /* inputs */						\
				"0" (result),		/* %5 RAX */		\
			 [ret]	"1" (&x86_x64_ret),	/* %6 RBX */		\
				"2" (x86_x64_uip),	/* %7 RCX */		\
				"3" (sp),		/* %8 RDX */		\
				"4" (cntrl)		/* %9 RSI */		\
			 /* no clobbers */					\
	);									\
    while(1);									\
}
#endif /* !defined(CONFIG_X86_COMPATIBILITY_MODE) */


//
//	ThreadSwitch ()
//
#define SYS_THREAD_SWITCH(dest)					\
  void SYSCALL_ATTR ("thread_switch")				\
  sys_thread_switch (dest)

#define return_thread_switch() return


//
//	Unmap ()
//
#define SYS_UNMAP(control)					\
  void SYSCALL_ATTR ("unmap")					\
  sys_unmap (control)

#define return_unmap() return


//
//	ProcessorControl ()
//
#define SYS_PROCESSOR_CONTROL(processor_no, internal_frequency,	\
			      external_frequency, voltage)	\
  void SYSCALL_ATTR ("processor_control")			\
  sys_processor_control (processor_no, internal_frequency,	\
			 external_frequency, voltage)

#define return_processor_control() return


//
//	MemoryControl ()
//
#define SYS_MEMORY_CONTROL(control, attribute0, attribute1,	\
			   attribute2, attribute3)		\
  void SYSCALL_ATTR ("memory_control")				\
  sys_memory_control (control, attribute0, attribute1,		\
		      attribute2, attribute3)

#define return_memory_control()	return



/* entry functions for dispatching syscalls */
extern "C" void syscall_entry();
#if defined(CONFIG_X86_COMPATIBILITY_MODE)
extern "C" void syscall_entry_32();
extern "C" void sysenter_entry_32();
#endif /* defined(CONFIG_X86_COMPATIBILITY_MODE) */

#endif /* !__GLUE_V4_X86__X64__SYSCALLS_H__ */
