/*********************************************************************
 *                
 * Copyright (C) 2003-2004,  National ICT Australia (NICTA)
 *                
 * File path:     glue/v4-arm/syscalls.h
 * Description:   
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
 * $Id: syscalls.h,v 1.16 2006/10/27 18:01:19 reichelt Exp $
 *                
 ********************************************************************/
#ifndef __GLUE__V4_ARM__SYSCALLS_H__
#define __GLUE__V4_ARM__SYSCALLS_H__

#include INC_ARCH(asm.h)
#include INC_GLUE(config.h)

#define USER_UTCB_REF		(*(word_t *)USER_UTCB_REF_PAGE)

#define L4_TRAP_KPUTC		0x000000a0
#define L4_TRAP_KGETC		0x000000a4
#define L4_TRAP_KGETC_NB	0x000000a8
#define L4_TRAP_KDEBUG		0x000000ac
#define L4_TRAP_GETUTCB		0x000000b0
#define L4_TRAP_KIP		0x000000b4

#define SYSCALL_ipc		    0x0
#define SYSCALL_thread_switch	    0x4
#define SYSCALL_thread_control	    0x8
#define SYSCALL_exchange_registers  0xc
#define SYSCALL_schedule	    0x10
#define SYSCALL_unmap		    0x14
#define SYSCALL_space_control	    0x18
#define SYSCALL_processor_control   0x1c
#define SYSCALL_memory_control	    0x20
#define SYSCALL_system_clock	    0x24
#define SYSCALL_lipc		    0x28

/* Upper bound on syscall number */
#define SYSCALL_limit		SYSCALL_lipc

/*
 * attributes for system call functions
 */
#define SYSCALL_ATTR(x)

#define SYS_IPC_RETURN_TYPE			threadid_t
#define SYS_THREAD_CONTROL_RETURN_TYPE		word_t
#define SYS_EXCHANGE_REGISTERS_RETURN_TYPE	threadid_t
#define SYS_SPACE_CONTROL_RETURN_TYPE		word_t
#define SYS_SCHEDULE_RETURN_TYPE		word_t
#define SYS_MEMORY_CONTROL_RETURN_TYPE		word_t


/*
 * Syscall declaration wrappers.
 */

#define SYS_IPC(to, from, timeout)				\
  SYS_IPC_RETURN_TYPE SYSCALL_ATTR ("ipc")			\
  sys_ipc (to, from, timeout)

#define SYS_THREAD_CONTROL(dest, space, scheduler, pager, utcb) \
  SYS_THREAD_CONTROL_RETURN_TYPE SYSCALL_ATTR ("thread_control")\
  sys_thread_control (dest, space, scheduler, pager, utcb)

#define SYS_SPACE_CONTROL(space, control, kip_area, utcb_area,  \
                          redirector)                           \
  SYS_SPACE_CONTROL_RETURN_TYPE SYSCALL_ATTR ("space_control")  \
  sys_space_control (space, control, kip_area, utcb_area,       \
                     redirector)

#define SYS_SCHEDULE(dest, time_control, processor_control,     \
                     prio, preemption_control)                  \
  SYS_SCHEDULE_RETURN_TYPE SYSCALL_ATTR ("schedule")            \
  sys_schedule (dest, time_control, processor_control,          \
                prio, preemption_control)

#define SYS_EXCHANGE_REGISTERS(dest, control, usp, uip, uflags,	\
                        uhandle, pager, is_local)		\
  SYS_EXCHANGE_REGISTERS_RETURN_TYPE SYSCALL_ATTR ("exchange_registers")\
  sys_exchange_registers (dest, control, usp, uip,		\
                          uflags, uhandle, pager, is_local)

#define SYS_THREAD_SWITCH(dest)					\
  void SYSCALL_ATTR ("thread_switch")				\
  sys_thread_switch (dest)

#define SYS_UNMAP(control)					\
  void SYSCALL_ATTR ("unmap") sys_unmap (control)

#define SYS_PROCESSOR_CONTROL(processor_no, internal_frequency,	\
                              external_frequency, voltage)	\
  void SYSCALL_ATTR ("processor_control")                       \
  sys_processor_control (processor_no, internal_frequency,	\
                         external_frequency, voltage)

#define SYS_MEMORY_CONTROL(control, attribute0, attribute1,	\
                           attribute2, attribute3)		\
  SYS_MEMORY_CONTROL_RETURN_TYPE SYSCALL_ATTR ("memory_control")\
  sys_memory_control (control, attribute0, attribute1,		\
                      attribute2, attribute3) 

/**
 * Preload registers and return from sys_ipc
 * @param from The FROM value after the system call
 */
#define return_ipc(from) return(from)

/**
 * Preload registers and return from sys_thread_control
 * @param result The RESULT value after the system call
 */
#define return_thread_control(result)  return (result)

/**
 * Preload registers and return from sys_exchange_registers
 * @param result The RESULT value after the system call
 * @param control The CONTROL value after the system call
 * @param sp The SP value after the system call
 * @param ip The IP value after the system call
 * @param flags The FLAGS value after the system call
 * @param pager The PAGER value after the system call
 * @param handle The USERDEFINEDHANDLE value after the system call
 */
#define return_exchange_registers(result, control, sp, ip, flags, pager, handle)\
{									\
    register word_t rslt    asm("r0") = (result).get_raw();		\
    register word_t ctrl    asm("r1") = control;			\
    register word_t sp_r    asm("r2") = sp;				\
    register word_t ip_r    asm("r3") = ip;				\
    register word_t flg	    asm("r4") = flags;				\
    register word_t hdl	    asm("r5") = handle;				\
    register word_t pgr	    asm("r6") = (pager).get_raw();		\
    char * context = (char *) get_current_tcb()->get_stack_top () -	\
		ARM_IPC_STACK_SIZE - (4*4);				\
									\
    __asm__ __volatile__ (						\
	CHECK_ARG("r0", "%2")						\
	CHECK_ARG("r1", "%3")						\
	CHECK_ARG("r2", "%4")						\
	CHECK_ARG("r3", "%5")						\
	CHECK_ARG("r4", "%6")						\
	CHECK_ARG("r5", "%8")						\
	CHECK_ARG("r6", "%7")						\
	"mov	sp,	%1	\n"					\
	"mov	pc,	%0	\n"					\
	:: "r"	(__builtin_return_address(0)),				\
	   "r"	(context),						\
	   "r" (rslt), "r" (ctrl), "r" (sp_r),				\
	   "r" (ip_r), "r" (flg), "r" (pgr), "r" (hdl)			\
    );									\
    while (1);								\
}

/**
 * Return from sys_thread_switch
 */
#define return_thread_switch() return


/**
 * Return from sys_unmap
 */
#define return_unmap()                  return


/**
 * Preload registers and return from sys_thread_switch
 * @param result The RESULT value after the system call
 * @param control The CONTROL value after the system call
 */
#define return_space_control(result, control) {                         \
    __asm__ __volatile__ (                                              \
        " /* return_unmap */                                    \n"     \
        "       mov     r0, %0                                  \n"     \
        "       mov     r1, %1                                  \n"     \
        :                                                               \
        : "r" (result), "r" (control)                                   \
        : "r0", "r1");                                                  \
    return (result);                                                    \
}

/**
 * Preload registers and return from sys_schedule
 * @param result The RESULT value after the system call
 * @param time_control The TIME_CONTROL value after the system call
 */
#define return_schedule(result, time_control) {                         \
    __asm__ __volatile__ (                                              \
        " /* return_schedule */                                 \n"     \
        "       mov     r0, %0                                  \n"     \
        "       mov     r1, %1                                  \n"     \
        :                                                               \
        : "r" (result), "r" (time_control)                              \
        : "r0", "r1");                                                  \
    return (result);                                                    \
}

#define return_processor_control()

/**
 * Return from sys_memory_control
 */
#define return_memory_control(from)     return (from)

#endif /* __GLUE__V4_ARM__SYSCALLS_H__ */
