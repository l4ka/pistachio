/*********************************************************************
 *                
 * Copyright (C) 2002-2005,  Karlsruhe University
 *                
 * File path:     glue/v4-ia64/syscalls.h
 * Description:   Syscall specifi macros
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
 * $Id: syscalls.h,v 1.21 2005/10/19 16:21:22 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __GLUE__V4_IA64__SYSCALLS_H__
#define __GLUE__V4_IA64__SYSCALLS_H__


typedef struct _sysret2_t { word_t w[2]; } sysret2_t;
typedef struct _sysret3_t { word_t w[3]; } sysret3_t;
typedef struct _sysret4_t { word_t w[4]; } sysret4_t;


//
// System call function attributes
//
#define SYSCALL_ATTR(sec_name)



//
//	Ipc ()
//
#define SYS_IPC(to, from, timeout)				\
  sysret2_t SYSCALL_ATTR ("ipc") sys_ipc (to, from, timeout)

#define return_ipc(from)					\
do {								\
    sysret2_t ret;						\
    ret.w[0] = 0;						\
    ret.w[1] = (from).get_raw ();				\
    current->set_partner (from);				\
    return ret;							\
} while(0)


//
//	ThreadControl ()
//
#define SYS_THREAD_CONTROL(dest, space, scheduler, pager,	\
			   utcblocation) 			\
  word_t SYSCALL_ATTR ("thread_control")			\
  sys_thread_control (dest, space, scheduler, pager, utcblocation)

#define return_thread_control(result)				\
do {								\
    return result;						\
} while (0)


//
//	ExchangeRegisters ()
//
#define SYS_EXCHANGE_REGISTERS(dest, control, usp, uip,		\
			       uflags, uhandle, pager,		\
			       is_local)	 		\
  sysret4_t SYSCALL_ATTR ("exchange_registers")			\
  sys_exchange_registers (dest, control, usp, uip,		\
			  uflags, uhandle, pager, is_local)

#define return_exchange_registers(result,			\
	control, sp, ip, flags, pager, handle)			\
do {								\
    sysret4_t ret;						\
    ret.w[0] = (result).get_raw ();				\
    ret.w[1] = control;						\
    ret.w[2] = sp;						\
    ret.w[3] = ip;						\
    word_t * ptr = get_current_tcb ()->get_stack_top () - 4 -	\
	(sizeof (ia64_switch_context_t) / sizeof (word_t));	\
    *ptr++ = flags;						\
    *ptr++ = handle;						\
    *ptr = pager.get_raw ();					\
    return ret;							\
} while (0)


//
//	ThreadSwitch ()
//
#define SYS_THREAD_SWITCH(dest)					\
  void SYSCALL_ATTR ("thread_switch")				\
  sys_thread_switch (dest)

#define return_thread_switch()					\
	return


//
//	Schedule ()
//
#define SYS_SCHEDULE(dest, time_control, processor_control,	\
		     prio, preemption_control)			\
  sysret2_t SYSCALL_ATTR ("schedule")				\
  sys_schedule (dest, time_control, processor_control,		\
		prio, preemption_control)

#define return_schedule(result, time_control)			\
do {								\
    sysret2_t ret;						\
    ret.w[0] = result;						\
    ret.w[1] = time_control;					\
    return ret;							\
} while (0)


//
//	SpaceControl ()
//
#define SYS_SPACE_CONTROL(space, control, kip_area, utcb_area,	\
			  redirector)				\
  sysret2_t SYSCALL_ATTR ("space_control")			\
  sys_space_control (space, control, kip_area, utcb_area,	\
		     redirector)

#define return_space_control(result, control)			\
do {								\
    sysret2_t ret;						\
    ret.w[0] = result;						\
    ret.w[1] = control;						\
    return ret;							\
} while (0)


//
//	Unmap ()
//
#define SYS_UNMAP(control)					\
  void SYSCALL_ATTR ("unmap") sys_unmap (control)

#define return_unmap()						\
	return


//
//	ProcessorControl ()
//
#define SYS_PROCESSOR_CONTROL(processor_no, internal_frequency,	\
			      external_frequency, voltage)	\
  void SYSCALL_ATTR ("processor_control")			\
  sys_processor_control (processor_no, internal_frequency,	\
			 external_frequency, voltage)

#define return_processor_control()				\
	return


//
//	MemoryControl ()
//
#define SYS_MEMORY_CONTROL(control, attribute0, attribute1,	\
			   attribute2, attribute3)		\
  void SYSCALL_ATTR ("memory_control")				\
  sys_memory_control (control, attribute0, attribute1,		\
		      attribute2, attribute3)

#define return_memory_control(result)				\
	return result


//
//	SAL_Call ()
//
#define SYS_SAL_CALL(idx, a1, a2, a3, a4, a5, a6)		\
  extern "C" sal_return_t SYSCALL_ATTR ("sal_call") 		\
  sys_sal_call (idx, a1, a2, a3, a4, a5, a6)


#endif /* !__GLUE__V4_IA64__SYSCALLS_H__ */
