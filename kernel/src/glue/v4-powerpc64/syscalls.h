/*********************************************************************
 *                
 * Copyright (C) 2003,  National ICT Australia (NICTA)
 *                
 * File path:     glue/v4-powerpc64/syscalls.h
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
 * $Id: syscalls.h,v 1.10 2004/06/04 02:52:57 cvansch Exp $
 *                
 ********************************************************************/
#ifndef __GLUE__V4_POWERPC64__SYSCALLS_H__
#define __GLUE__V4_POWERPC64__SYSCALLS_H__

#include INC_ARCH(frame.h)
#include INC_GLUE(abi.h)


#define L4_TRAP64_MAGIC	    (0x4c345f5050433634ul)  /* "L4_PPC64" */
#define L4_TRAP64_KDEBUG    (L4_TRAP64_MAGIC + 0)
#define L4_TRAP64_KPUTC	    (L4_TRAP64_MAGIC + 1)
#define L4_TRAP64_KGETC	    (L4_TRAP64_MAGIC + 2)
#define L4_TRAP64_KGETC_NB  (L4_TRAP64_MAGIC + 3)

#define SYSCALL_base			(-32000)
#define SYSCALL_ipc			(SYSCALL_base - 0)
#define SYSCALL_thread_switch		(SYSCALL_base - 1)
#define SYSCALL_thread_control		(SYSCALL_base - 2)
#define SYSCALL_exchange_registers	(SYSCALL_base - 3)
#define SYSCALL_schedule		(SYSCALL_base - 4)
#define SYSCALL_unmap			(SYSCALL_base - 5)
#define SYSCALL_space_control		(SYSCALL_base - 6)
#define SYSCALL_processor_control	(SYSCALL_base - 7)
#define SYSCALL_memory_control		(SYSCALL_base - 8)
#define SYSCALL_system_clock		(SYSCALL_base - 9)
#define SYSCALL_rtas_call		(SYSCALL_base - 20)
#define SYSCALL_last			(SYSCALL_base - 20)

#if !defined(ASSEMBLY)

/*
 * System call function attributes.
 */

#define SYSCALL_ATTR(sec_name)
//__attribute__ ((noreturn))

/*
 * Syscall declaration wrappers.
 */

#define SYS_IPC(to, from, timeout)				\
    word_t SYSCALL_ATTR ("ipc")					\
	sys_ipc (to, from, timeout)

#define SYS_THREAD_CONTROL(dest, space, scheduler, pager, utcb_location)    \
    word_t SYSCALL_ATTR ("thread_control")				    \
	sys_thread_control (dest, space, scheduler, pager, utcb_location)

#define SYS_SPACE_CONTROL(space, control, kip_area, utcb_area,		\
			  redirector)					\
    word_t SYSCALL_ATTR ("space_control")				\
	sys_space_control (space, control, kip_area, utcb_area,		\
			    redirector)

#define SYS_SCHEDULE(dest, time_control, processor_control,	\
		     prio, preemption_control)			\
    word_t SYSCALL_ATTR ("schedule")				\
	sys_schedule (dest, time_control, processor_control,	\
			prio, preemption_control)

#define SYS_EXCHANGE_REGISTERS(dest, control, usp, uip,			\
			       uflags, uhandle, pager, is_local)	\
    word_t SYSCALL_ATTR ("exchange_registers")				\
	sys_exchange_registers (dest, control, usp, uip,		\
				uflags, uhandle, pager, is_local)

#define SYS_THREAD_SWITCH(dest)					\
    void SYSCALL_ATTR ("thread_switch")				\
	sys_thread_switch (dest)

#define SYS_UNMAP(control)					\
    void SYSCALL_ATTR ("unmap") sys_unmap (control)

#define SYS_PROCESSOR_CONTROL(processor_no, internal_frequency,		\
			      external_frequency, voltage)		\
    void SYSCALL_ATTR ("processor_control")				\
	sys_processor_control (processor_no, internal_frequency,	\
				external_frequency, voltage)

#define SYS_MEMORY_CONTROL(control, attribute0, attribute1,	\
			   attribute2, attribute3)		\
    word_t SYSCALL_ATTR ("memory_control")			\
	sys_memory_control (control, attribute0, attribute1,	\
			    attribute2, attribute3)

#define SYS_RTAS_CALL(token, nargs, nret, ptr )			\
    word_t SYSCALL_ATTR ("rtas_call")				\
	sys_rtas_call (token, nargs, nret, ptr)
    
extern "C" SYS_RTAS_CALL( word_t token, word_t nargs, word_t nret, word_t ptr );


/* The instruction executed in user mode which requests the kernel interface
 * page.  It is an instruction illegal to use in user mode, and fires a
 * program exception.
 */
#define KIP_EXCEPT_INSTR	0x7c0002e4      // tlbia
#define KDEBUG_EXCEPT_INSTR	0x7fe00008      // trap

#define return_user_0param()						\
do {									\
    asm volatile (							\
	    "mtlr %0 ;"							\
	    "ld %%r1, 0 (%%r1);"					\
	    "blr ;"							\
	    : 								\
	    : "r" (__builtin_return_address(0))				\
	    );								\
    while(1);								\
} while(0)


#define return_thread_switch()		return_user_0param()
#define return_unmap()			return_user_0param()
#define return_processor_control()	return_user_0param()

#define return_user_1param(param1)					\
do {									\
    register word_t ret1 asm("r3") = param1;				\
    asm volatile (							\
	    "mtlr %0 ;"							\
	    "ld %%r1, 0 (%%r1);"					\
	    "blr ;"							\
	    : 								\
	    : "r" (__builtin_return_address(0)),			\
	      "r" (ret1)						\
	    );								\
    while(1);								\
} while(0)

#define return_thread_control(result)	return_user_1param(result)
#define return_ipc(from)		return_user_1param(from.get_raw())
#define return_memory_control(result)	return_user_1param (result)

#define return_user_2param(param1, param2)				\
do {									\
    register word_t ret1 asm("r3") = param1;				\
    register word_t ret2 asm("r4") = param2;				\
    asm volatile (							\
	    "mtlr %0 ;"							\
	    "ld %%r1, 0 (%%r1);"					\
	    "blr ;"							\
	    : 								\
	    : "r" (__builtin_return_address(0)),			\
	      "r" (ret1), "r" (ret2)					\
	    );								\
    while(1);								\
} while(0)

#define return_space_control( result, control )	return_user_2param( result, control )
#define return_schedule( result, time_control )	return_user_2param( result, time_control )

#define return_exchange_registers( result, control, sp, ip, flags, pager, handle )  \
{									\
    register threadid_t tid asm("r3") = result;				\
    register word_t ctrl asm("r4") = control;				\
    register word_t sp_r asm("r5") = sp;				\
    register word_t ip_r asm("r6") = ip;				\
    register word_t flg asm("r7") = flags;				\
    register threadid_t pgr asm("r8") = pager;				\
    register word_t hdl asm("r9") = handle;				\
    asm volatile (							\
	    "mtlr %0 ;"							\
	    "ld %%r1, 0 (%%r1);"					\
	    "blr ;"							\
	    : 								\
	    : "r" (__builtin_return_address(0)),			\
	      "r" (tid), "r" (ctrl), "r" (sp_r), "r" (ip_r),		\
	      "r" (flg), "r" (pgr), "r" (hdl)				\
	    );								\
    while(1);								\
}

#endif	/* !defined(ASSEMBLY) */

#endif /* __GLUE__V4_POWERPC64__SYSCALLS_H__ */
