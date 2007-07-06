/*********************************************************************
 *                
 * Copyright (C) 2003-2004, University of New South Wales
 *                
 * File path:    glue/v4-sparc64/syscalls.h
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
 * $Id: syscalls.h,v 1.4 2004/02/12 01:38:11 philipd Exp $
 *                
 ********************************************************************/

#ifndef __GLUE__V4_SPARC64__SYSCALLS_H__
#define __GLUE__V4_SPARC64__SYSCALLS_H__

//
// System call trap numbers
//

#define SYSCALL_kernel_interface	0x70
#define SYSCALL_ipc			0x71
#define SYSCALL_lipc			0x72
#define SYSCALL_exchange_registers	0x73
#define SYSCALL_thread_control		0x74
#define SYSCALL_system_clock		0x75
#define SYSCALL_thread_switch		0x76
#define SYSCALL_schedule		0x77
#define SYSCALL_unmap			0x78
#define SYSCALL_space_control		0x79
#define SYSCALL_processor_control	0x7a
#define SYSCALL_memory_control		0x7b

//
// System call function attributes.
//

#define SYSCALL_ATTR(sec_name)

#define SYS_IPC_RETURN_TYPE                     threadid_t
#define SYS_THREAD_CONTROL_RETURN_TYPE          word_t
#define SYS_EXCHANGE_REGISTERS_RETURN_TYPE      threadid_t
#define SYS_SPACE_CONTROL_RETURN_TYPE           word_t
#define SYS_SCHEDULE_RETURN_TYPE                word_t
#define SYS_MEMORY_CONTROL_RETURN_TYPE          word_t
#define SYS_USER_STATE_RETURN_TYPE		word_t

//
// Syscall declaration wrappers.
//

/**********************
* ExchangeRegisters() *
**********************/

#define SYS_EXCHANGE_REGISTERS(dest, control, usp, uip,				\
			       uflags, uhandle, pager, is_local)		\
  SYS_EXCHANGE_REGISTERS_RETURN_TYPE SYSCALL_ATTR ("exchange_registers")	\
  sys_exchange_registers (dest, control, usp, uip,				\
			  uhandle, pager, uflags, is_local)

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
#define return_exchange_registers(result, control, sp, ip, flags, pager, handle) 	\
do {											\
    register word_t o1 asm ("o1") = control;						\
    register word_t o2 asm ("o2") = sp;							\
    register word_t o3 asm ("o3") = ip;							\
    register threadid_t o4 asm ("o4") = pager;						\
    register word_t o5 asm ("o5") = handle;						\
    register word_t g4 asm ("g4") = flags;						\
    asm volatile ("" :: "r" (o1), "r" (o2), "r" (o3), "r" (o4), "r" (o5), "r" (g4));	\
    return result;									\
} while (0);

/******************
* ThreadControl() *
******************/

#define SYS_THREAD_CONTROL(dest, space, scheduler, pager,		\
			   utcb_location)      		  		\
  SYS_THREAD_CONTROL_RETURN_TYPE SYSCALL_ATTR ("thread_control")	\
  sys_thread_control (dest, space, scheduler, pager,            	\
                      utcb_location)

/**
 * Preload registers and return from sys_thread_control
 * @param result The RESULT value after the system call
 */
#define return_thread_control(result)	return (result)

/*****************
* ThreadSwitch() *
*****************/

#define SYS_THREAD_SWITCH(dest)					\
  void SYSCALL_ATTR ("thread_switch")				\
  sys_thread_switch (dest)


/**
 * Return from sys_thread_switch
 */
#define return_thread_switch() return

/*************
* Schedule() *
*************/

#define SYS_SCHEDULE(dest, time_control, processor_control,	\
		     prio, preemption_control)			\
  SYS_SCHEDULE_RETURN_TYPE SYSCALL_ATTR ("schedule")		\
  sys_schedule (dest, time_control, processor_control,		\
		prio, preemption_control)

/**
 * Preload registers and return from sys_schedule
 * @param result The RESULT value after the system call
 * @param time_control The TIME_CONTROL value after the system call
 */
#define return_schedule( result, time_control )						\
do {											\
    register word_t o1 asm ("o1") = time_control;					\
    asm volatile ("" :: "r" (o1));							\
    return result;									\
} while (0);

/********
* Ipc() *
********/

#define SYS_IPC(to, from, timeout)				\
  SYS_IPC_RETURN_TYPE SYSCALL_ATTR ("ipc") 			\
  sys_ipc (to, from, timeout)

/**
 * Preload registers and return from sys_ipc
 * @param from The FROM value after the system call
 */
#define return_ipc(from)	return (from)

/**********
* Unmap() *
**********/

#define SYS_UNMAP(control)					\
  void SYSCALL_ATTR ("unmap") sys_unmap (control)

/**
 * Return from sys_unmap
 */
#define return_unmap() return

/*****************
* SpaceControl() *
*****************/

#define SYS_SPACE_CONTROL(space, control, kip_area, utcb_area,	\
			  redirector)				\
  SYS_SPACE_CONTROL_RETURN_TYPE SYSCALL_ATTR ("space_control")	\
  sys_space_control (space, control, kip_area, utcb_area,	\
		     redirector)

/**
 * Preload registers and return from sys_thread_switch
 * @param result The RESULT value after the system call
 * @param control The CONTROL value after the system call
 */
#define return_space_control(result, control)						\
do {											\
    register word_t o1 asm ("o1") = control;						\
    asm volatile ("" :: "r" (o1));							\
    return result;									\
} while (0);

/*******************
* ProcessorControl *
*******************/

#define SYS_PROCESSOR_CONTROL(processor_no, internal_frequency,	\
			      external_frequency, voltage)	\
  void SYSCALL_ATTR ("processor_control")			\
  sys_processor_control (processor_no, internal_frequency,	\
			 external_frequency, voltage)

#define return_processor_control() return

/******************
* MemoryControl() *
******************/

#define SYS_MEMORY_CONTROL(control, attribute0, attribute1,	\
			   attribute2, attribute3)		\
  SYS_MEMORY_CONTROL_RETURN_TYPE SYSCALL_ATTR ("memory_control")\
  sys_memory_control (control, attribute0, attribute1,		\
		      attribute2, attribute3)


#define return_memory_control() return


#endif /* !__GLUE__V4_SPARC64__SYSCALLS_H__ */
