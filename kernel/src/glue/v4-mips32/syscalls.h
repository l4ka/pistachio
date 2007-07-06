/*********************************************************************
 *                
 * Copyright (C) 2006,  Karlsruhe University
 *                
 * File path:     glue/v4-mips32/syscalls.h
 * Description:   System call declarations for MIPS32
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
 * $Id: syscalls.h,v 1.1 2006/02/23 21:07:40 ud3 Exp $
 *                
 ********************************************************************/
#ifndef __GLUE__V4_MIPS32__SYSCALLS_H__
#define __GLUE__V4_MIPS32__SYSCALLS_H__

#if defined(ASSEMBLY)

#define L4_TRAP_KPUTC			(-100)
#define L4_TRAP_KGETC			(-101)
#define L4_TRAP_KDEBUG			(-102)
#define L4_TRAP_UNUSED			(-103)
#define L4_TRAP_KGETC_NB		(-104)
#define L4_TRAP_READ_PERF		(-110)
#define L4_TRAP_WRITE_PERF		(-111)

/* The syscall assembler depends on the values below */
#define SYSCALL_ipc			(-101)
#define SYSCALL_thread_switch		(-102)
#define SYSCALL_thread_control		(-103)
#define SYSCALL_exchange_registers	(-104)
#define SYSCALL_schedule		(-105)
#define SYSCALL_unmap			(-106)
#define SYSCALL_space_control		(-107)
#define SYSCALL_processor_control	(-108)
#define SYSCALL_memory_control		(-109)
#define SYSCALL_system_clock		(-110)
#define SYSCALL_user_state		(-111)

#else

#define L4_TRAP_KPUTC			(-100ul)
#define L4_TRAP_KGETC			(-101ul)
#define L4_TRAP_KDEBUG			(-102ul)
#define L4_TRAP_GETUTCB			(-103ul)
#define L4_TRAP_KGETC_NB		(-104ul)
#define L4_TRAP_READ_PERF		(-110ul)
#define L4_TRAP_WRITE_PERF		(-111ul)

/* The syscall assembler depends on the values below */
#define SYSCALL_ipc			(-101ul)
#define SYSCALL_thread_switch		(-102ul)
#define SYSCALL_thread_control		(-103ul)
#define SYSCALL_exchange_registers	(-104ul)
#define SYSCALL_schedule		(-105ul)
#define SYSCALL_unmap			(-106ul)
#define SYSCALL_space_control		(-107ul)
#define SYSCALL_processor_control	(-108ul)
#define SYSCALL_memory_control		(-109ul)
#define SYSCALL_system_clock		(-110ul)
#define SYSCALL_user_state		(-111ul)

//
// System call function attributes.
//

#define SYSCALL_ATTR(sec_name)

#define SYS_IPC_RETURN_TYPE                     word_t 

#define SYS_THREAD_CONTROL_RETURN_TYPE          word_t
#define SYS_EXCHANGE_REGISTERS_RETURN_TYPE      word_t 
#define SYS_SPACE_CONTROL_RETURN_TYPE           word_t
#define SYS_SCHEDULE_RETURN_TYPE                word_t
#define SYS_MEMORY_CONTROL_RETURN_TYPE          word_t
#define SYS_CLOCK_RETURN_TYPE			u64_t



//
// Syscall declaration wrappers.
//

#define SYS_IPC(to, from, timeout)						\
  SYS_IPC_RETURN_TYPE SYSCALL_ATTR ("ipc")					\
  sys_ipc (to, from, timeout)

#define SYS_THREAD_CONTROL(dest, space, scheduler, pager, utcb)			\
  SYS_THREAD_CONTROL_RETURN_TYPE SYSCALL_ATTR ("thread_control")		\
  sys_thread_control (dest, space, scheduler, pager, utcb)

#define SYS_SPACE_CONTROL(space, control, kip_area, utcb_area,			\
			  redirector)						\
  SYS_SPACE_CONTROL_RETURN_TYPE SYSCALL_ATTR ("space_control")			\
  sys_space_control (space, control, kip_area, utcb_area,			\
		     redirector)

#define SYS_SCHEDULE(dest, time_control, processor_control,			\
		     prio, preemption_control)					\
  SYS_SCHEDULE_RETURN_TYPE SYSCALL_ATTR ("schedule")				\
  sys_schedule (dest, time_control, processor_control,				\
		prio, preemption_control)

#define SYS_EXCHANGE_REGISTERS(dest, control, usp, uip,uflags,			\
			uhandle, pager, is_local)				\
  SYS_EXCHANGE_REGISTERS_RETURN_TYPE SYSCALL_ATTR ("exchange_registers")	\
  sys_exchange_registers (dest, control, usp, uip,				\
			  uflags, uhandle, pager, is_local)

#define SYS_THREAD_SWITCH(dest)							\
  void SYSCALL_ATTR ("thread_switch")						\
  sys_thread_switch (dest)

#define SYS_UNMAP(control)							\
  void SYSCALL_ATTR ("unmap") sys_unmap (control)

#define SYS_PROCESSOR_CONTROL(processor_no, internal_frequency,			\
			      external_frequency, voltage)			\
  void SYSCALL_ATTR ("processor_control")					\
  sys_processor_control (processor_no, internal_frequency,			\
			 external_frequency, voltage)

#define SYS_MEMORY_CONTROL(control, attribute0, attribute1,			\
			   attribute2, attribute3)				\
  SYS_MEMORY_CONTROL_RETURN_TYPE SYSCALL_ATTR ("memory_control")		\
  sys_memory_control (control, attribute0, attribute1,				\
		      attribute2, attribute3)


#define SYS_CLOCK()								\
  SYS_CLOCK_RETURN_TYPE SYSCALL_ATTR ("clock")					\
  sys_clock()




#define return_clock(ticks)							\
{										\
	return( ticks );							\
}

/**
 * Preload registers and return from sys_ipc
 * @param from The FROM value after the system call
 */
#define return_ipc(from)	return((from).get_raw())


/**
 * Preload registers and return from sys_thread_control
 * @param result The RESULT value after the system call
 */
#define return_thread_control(result)	return(result);


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
//#warning PORTME 
//#define return_exchange_registers(result, control, sp, ip, flags, pager, handle)	return

#define return_exchange_registers(result, control, sp, ip, flags, pager, handle)	\
{											\
    register word_t ctrl asm("$4") = control;	/* a0 */				\
    register word_t sp_r asm("$5") = sp;	/* a1 */				\
    register word_t ip_r asm("$6") = ip;	/* a2 */				\
    register word_t flg asm("$7") = flags;	/* a3 */				\
    register word_t pgr asm("$8") = pager.get_raw();	/* t0 */			\
    register word_t hdl asm("$9") = handle;	/* t1 */				\
											\
    __asm__ __volatile__ (								\
	"" : : "r" (ctrl), "r" (sp_r), "r" (ip_r),					\
	"r" (flg), "r" (pgr), "r" (hdl)							\
    );											\
    return ((result).get_raw());							\
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
//#define return_space_control(result, control)	return
#define return_space_control(result, control)				\
{									\
    register word_t c asm("$4") = control; /* a0 */			\
    __asm__ __volatile__ (						\
	"" : : "r" (c)							\
    );									\
    return (result);							\
}



/**
 * Preload registers and return from sys_schedule
 * @param result The RESULT value after the system call
 * @param time_control The TIME_CONTROL value after the system call
 */
//#define return_schedule( result, time_control )
#define return_schedule(result, time_control)				\
{									\
    register word_t c asm("$4") = time_control;	/* a0 */    		\
    __asm__ __volatile__ (						\
	"" : : "r" (c)							\
    );									\
    return (result);							\
}

/**
 * Return from sys_memory_control
 */
#define return_memory_control(result)	return (result)


/**
 * Return from sys_processor_control
 */
#define return_processor_control() return


#endif

#endif /* !__GLUE__V4_MIPS32__SYSCALLS_H__ */
