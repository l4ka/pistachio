/*********************************************************************
 *                
 * Copyright (C) 2002, 2003,  University of New South Wales
 *                
 * File path:     glue/v4-alpha/syscalls.h
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
 * $Id: syscalls.h,v 1.11 2004/06/04 02:27:51 cvansch Exp $
 *                
 ********************************************************************/

#ifndef __GLUE__V4_ALPHA__SYSCALLS_H__
#define __GLUE__V4_ALPHA__SYSCALLS_H__

#define SYSCALL_ipc                0
#define SYSCALL_thread_switch      1
#define SYSCALL_thread_control     2
#define SYSCALL_exchange_registers 3
#define SYSCALL_schedule           4
#define SYSCALL_unmap              5
#define SYSCALL_space_control      6
#define SYSCALL_processor_control  7
#define SYSCALL_memory_control     8
#define SYSCALL_system_clock       9

/* Alpha specific */
#define SYSCALL_wrperfmon          100
#define SYSCALL_null               101
#define SYSCALL_halt               102
#define SYSCALL_read_idle          103

/* The application gets the kernel info page by doing some privileged PAL call, with 
   a0 ($16) == {'L', '4', 'u', 'K', 'K', 'I', 'P', '4'} == 0x4c34754b4b495034
*/
#define MAGIC_KIP_REQUEST          (0x4c34754b4b495034)

/*
  System call function attributes.
*/

#define SYSCALL_ATTR(sec_name)


#define SYS_IPC_RETURN_TYPE                     threadid_t
#define SYS_THREAD_CONTROL_RETURN_TYPE          word_t
#define SYS_EXCHANGE_REGISTERS_RETURN_TYPE      threadid_t
#define SYS_SPACE_CONTROL_RETURN_TYPE           word_t
#define SYS_SCHEDULE_RETURN_TYPE                word_t
#define SYS_MEMORY_CONTROL_RETURN_TYPE          word_t

#define SYS_IPC(to, from, timeout)                              \
  SYS_IPC_RETURN_TYPE SYSCALL_ATTR ("ipc")                      \
  sys_ipc (to, from, timeout)

#define SYS_THREAD_CONTROL(dest, space, scheduler, pager, utcb)		\
  SYS_THREAD_CONTROL_RETURN_TYPE SYSCALL_ATTR ("thread_control")	\
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

#define SYS_EXCHANGE_REGISTERS(dest, control, usp, uip,				\
                               uflags, uhandle, pager, is_local)		\
  SYS_EXCHANGE_REGISTERS_RETURN_TYPE SYSCALL_ATTR ("exchange_registers")	\
  sys_exchange_registers (dest, control, usp, uip,				\
                          uflags, uhandle, pager, is_local)

#define SYS_THREAD_SWITCH(dest)                                 \
  void SYSCALL_ATTR ("thread_switch")                           \
  sys_thread_switch (dest)

#define SYS_UNMAP(control)                                      \
  void SYSCALL_ATTR ("unmap") sys_unmap (control)

#define SYS_PROCESSOR_CONTROL(processor_no, internal_frequency, \
                              external_frequency, voltage)      \
  void SYSCALL_ATTR ("processor_control")                       \
  sys_processor_control (processor_no, internal_frequency,      \
                         external_frequency, voltage)

#define SYS_MEMORY_CONTROL(control, attribute0, attribute1,		\
                           attribute2, attribute3)			\
  SYS_MEMORY_CONTROL_RETURN_TYPE SYSCALL_ATTR ("memory_control")	\
  sys_memory_control (control, attribute0, attribute1,			\
                      attribute2, attribute3)

/**
 * Preload registers and return from sys_ipc
 * @param from The FROM value after the system call
 */


#define return_ipc(from)   return (from)


/**
 * Preload registers and return from sys_thread_control
 * @param result The RESULT value after the system call
 */

#define return_thread_control(result)  return result

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


#define return_exchange_registers(result, control, sp, ip, flags, pager, handle)				\
{														\
    register word_t r16 asm("$16") = control;									\
    register word_t r17 asm("$17") = sp;									\
    register word_t r18 asm("$18") = ip;									\
    register word_t r19 asm("$19") = flags;									\
    register word_t r20 asm("$20") = pager.get_raw();								\
    register word_t r21 asm("$21") = handle;									\
    asm __volatile__ ("" : : "r" (r16), "r" (r17), "r" (r18), "r" (r19), "r" (r20), "r" (r21)); 	        \
    return result;												\
}


/**
 * Return from sys_thread_switch
 */
#define return_thread_switch() return


/**
 * Return from sys_unmap
 */
#define return_unmap() return


/**
 * Preload registers and return from sys_thread_switch
 * @param result The RESULT value after the system call
 * @param control The CONTROL value after the system call
 */
#define return_space_control(result, control)	\
{						\
    register word_t r16 asm("$16") = control;	\
    asm __volatile__ ("" : : "r" (r16));	\
    return result;				\
}


/**
 * Preload registers and return from sys_schedule
 * @param result The RESULT value after the system call
 * @param time_control The TIME_CONTROL value after the system call
 */
#define return_schedule( result, time_control )		\
{							\
    register word_t r16 asm("$16") = time_control;	\
    asm __volatile__ ("" : : "r" (r16));		\
    return result;					\
}

#define return_processor_control() return

#endif /* __GLUE__V4_ALPHA__SYSCALLS_H__ */
