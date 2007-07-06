/*********************************************************************
 *                
 * Copyright (C) 2002, 2003,  Karlsruhe University
 *                
 * File path:     glue/v4-tmplarch/syscalls.h
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
 * $Id: syscalls.h,v 1.6 2003/09/24 19:04:53 skoglund Exp $
 *                
 ********************************************************************/

//
// System call function attributes.
//

#define SYSCALL_ATTR(sec_name)


//
// Syscall declaration wrappers.
//

#define SYS_IPC(to, from, timeout)				\
  void SYSCALL_ATTR ("ipc") 					\
  sys_ipc (to, from, timeout)

#define SYS_THREAD_CONTROL(dest, space, scheduler, pager)	\
  void SYSCALL_ATTR ("thread_control")				\
  sys_thread_control (dest, space, scheduler, pager)

#define SYS_SPACE_CONTROL(space, control, kip_area, utcb_area,	\
			  redirector)				\
  void SYSCALL_ATTR ("space_control")				\
  sys_space_control (space, control, kip_area, utcb_area,	\
		     redirector)

#define SYS_SCHEDULE(dest, time_control, processor_control,	\
		     prio, preemption_control)			\
  void SYSCALL_ATTR ("schedule")				\
  sys_schedule (dest, time_control, processor_control,		\
		prio, preemption_control)

#define SYS_EXCHANGE_REGISTERS(dest, control, usp, uip,		\
			       uflags, uhandle, pager, is_local)\
  void SYSCALL_ATTR ("exchange_registers")			\
  sys_exchange_registers (dest, control, usp, uip,		\
			  uflags, uhandle, pager, is_local)

#define SYS_THREAD_SWITCH(dest)					\
  void SYSCALL_ATTR ("thread_switch")				\
  sys_thread_switch (dest)

#define SYS_UNMAP(control)					\
  void SYSCALL_ATTR ("unmap") sys_unmap (control)

#define SYS_PROCESSOR_CONTROL(processor_no, internal_frequency,	\
			      external_frequency, voltage)	\
  void SYSCALL_ATTR ("processor_control")			\
  sys_processor_control (processor_no, internal_frequency,	\
			 external_frequency, voltage)

#define SYS_MEMORY_CONTROL(control, attribute0, attribute1,	\
			   attribute2, attribute3)		\
  void SYSCALL_ATTR ("memory_control")				\
  sys_memory_control (control, attribute0, attribute1,		\
		      attribute2, attribute3)



#warning PORTME
/**
 * Preload registers and return from sys_ipc
 * @param from The FROM value after the system call
 */
#define return_ipc(from)	return


#warning PORTME
/**
 * Preload registers and return from sys_thread_control
 * @param result The RESULT value after the system call
 */
#define return_thread_control(result)	return


#warning PORTME
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
#define return_exchange_registers(result, control, sp, ip, flags, pager, handle)	return


#warning PORTME
/**
 * Return from sys_thread_switch
 */
#define return_thread_switch() return


#warning PORTME
/**
 * Return from sys_unmap
 */
#define return_unmap() return


#warning PORTME
/**
 * Preload registers and return from sys_thread_switch
 * @param result The RESULT value after the system call
 * @param control The CONTROL value after the system call
 */
#define return_space_control(result, control)	return


#warning PORTME
/**
 * Preload registers and return from sys_schedule
 * @param result The RESULT value after the system call
 * @param time_control The TIME_CONTROL value after the system call
 */
#define return_schedule( result, time_control )
