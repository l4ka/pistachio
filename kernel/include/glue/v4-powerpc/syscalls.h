/*********************************************************************
 *                
 * Copyright (C) 2002, 2003,  Karlsruhe University
 *                
 * File path:     glue/v4-powerpc/syscalls.h
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
 ********************************************************************/
#ifndef __GLUE__V4_POWERPC__SYSCALLS_H__
#define __GLUE__V4_POWERPC__SYSCALLS_H__

#include INC_ARCH(frame.h)
#include INC_GLUE(abi.h)

#define L4_TRAP_KDEBUG  (0x5afe)
#define L4_TRAP_KPUTC   (L4_TRAP_KDEBUG + 1)
#define L4_TRAP_KGETC   (L4_TRAP_KDEBUG + 2)

//
// System call function attributes.
//

#define SYSCALL_ATTR(sec_name) \
  __attribute__ ((noreturn, section(".sys_" sec_name)))


//
// Syscall declaration wrappers.
//

#define SYS_IPC(to, from, timeout)				\
  void SYSCALL_ATTR ("ipc") 					\
  sys_ipc (to, from, timeout)

#define SYS_THREAD_CONTROL(dest, space, scheduler, pager, utcb_location)\
  void SYSCALL_ATTR ("thread_control")				\
  sys_thread_control (dest, space, scheduler, pager, utcb_location)

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



/* The instruction executed in user mode which requests the kernel interface
 * page.  It is an instruction illegal to use in user mode, and fires a
 * program exception.
 */
#define KIP_EXCEPT_INSTR        0x7c0002e4      // tlbia

#define TCB_STATE(r)	\
	(TOTAL_TCB_SIZE-sizeof(syscall_regs_t)+offsetof(syscall_regs_t,r))

#define return_user_0param()						\
do {									\
    asm volatile (							\
    	    "clrrwi %%r1, %%r1, %0;"  /* find the start of the tcb */	\
	    "lwz %%r13, %3 (%%r1) ;"  /* load srr0 */			\
	    "lwz %%r2,  %6 (%%r1) ;"  /* restore r2 */			\
    	    "lwz %%r31, %1 (%%r1) ;"  /* restore r31 */			\
    	    "lwz %%r30, %2 (%%r1) ;"  /* restore r30 */			\
    	    "lwz %%r12, %5 (%%r1) ;"  /* load srr1 */			\
    	    "lwz %%r1, %4 (%%r1) ;"   /* restore the user stack */	\
	    "mtsrr0 %%r13 ;"          /* restore srr0 */		\
    	    "mtsrr1 %%r12 ;"          /* restore srr1 */		\
    	    "rfi ;"							\
	    :								\
	    : "i" (KTCB_BITS), 				/* 0 */		\
	      "i" (TCB_STATE(r31)),			/* 1 */		\
	      "i" (TCB_STATE(r30)), 			/* 2 */		\
	      "i" (TCB_STATE(srr0_ip)),			/* 3 */		\
	      "i" (TCB_STATE(r1_stack)), 		/* 4 */		\
	      "i" (TCB_STATE(srr1_flags)),		/* 5 */		\
	      "i" (TCB_STATE(r2_local_id))		/* 6 */		\
	);								\
    while(1);								\
} while(0)

#define return_thread_switch() return_user_0param()
#define return_processor_control() return_user_0param()

#define return_kernel_ipc(ret_val)					\
do {									\
    asm volatile (							\
	    "mr " MKSTR(IPC_ABI_FROM_TID) ", %0 ;"			\
	    "mtlr %1 ;"							\
	    "mr %%r1, %2 ;"						\
	    "blr ;"							\
	    : 								\
	    : "r" (ret_val), "r" (__builtin_return_address(0)),		\
	      "r" (__builtin_frame_address(1))				\
	    );								\
    while(1);								\
} while(0)

/* For return_ipc() and return_ipc_error(), support returning to in-kernel
 * callers.  These macros will only work from the top-level ipc() function,
 * since they access frame information.
 */
#define return_ipc(from) return_kernel_ipc(from.get_raw())
#define return_ipc_error() return_kernel_ipc(threadid_t::nilthread())

#define return_user_ipc(ret_val)					\
do {									\
    extern word_t _sc_ipc_return[];					\
    asm volatile (							\
	    "mr " MKSTR(IPC_ABI_FROM_TID) ", %0 ;"			\
	    "mtlr %1 ;"							\
	    "blr ;"							\
	    :								\
	    : "r" (ret_val), "r" (_sc_ipc_return)			\
	    );								\
    while(1);								\
} while(0)

/* For return_ipc_abort(), we need not return to an in-kernel caller.  Nor
 * can we, since we have no access to the call-chain when we want to 
 * invoke return_ipc_abort().
 */
#define return_ipc_abort() return_user_ipc(threadid_t::nilthread())
 
#define return_user_1param(ret_val)					\
do {									\
   asm volatile (							\
	"mr %%r3, %0 ;"		/* the function return value */		\
	"clrrwi %%r1, %%r1, %1;"  /* find the start of the tcb */	\
	"lwz %%r13, %4 (%%r1) ; " /* load srr0 */			\
	"lwz %%r12, %6 (%%r1) ; " /* load srr1 */			\
	"lwz %%r2,  %7 (%%r1) ;"  /* restore r2 */			\
	"lwz %%r31, %2 (%%r1) ;"  /* restore r31 */			\
	"mtsrr1 %%r12 ;"	  /* restore srr1 */			\
	"mtsrr0 %%r13 ;"	  /* restore srr0 */			\
	"lwz %%r30, %3 (%%r1) ;"  /* restore r30 */			\
	"lwz %%r1, %5 (%%r1) ;"	  /* restore the user stack */		\
	"rfi ;"								\
	: 								\
	: "r" (ret_val), 				/* 0 */		\
	  "i" (KTCB_BITS),  				/* 1 */		\
	  "i" (TCB_STATE(r31)),				/* 2 */		\
	  "i" (TCB_STATE(r30)), 			/* 3 */		\
	  "i" (TCB_STATE(srr0_ip)), 			/* 4 */		\
	  "i" (TCB_STATE(r1_stack)), 			/* 5 */		\
	  "i" (TCB_STATE(srr1_flags)),			/* 6 */		\
	  "i" (TCB_STATE(r2_local_id))			/* 7 */		\
	: "r3", "r12"							\
	);								\
    while(1);								\
} while(0)

#define return_thread_control(result) return_user_1param(result);

#define return_user_2params( param1, param2 ) 				\
do {									\
    asm volatile (							\
	    "mr %%r3, %0 ;"	/* set return result */			\
	    "mr %%r4, %7 ;"	/* set return control result */		\
    	    "clrrwi %%r1, %%r1, %1;"  /* find the start of the tcb */	\
	    "lwz %%r13, %4 (%%r1) ;"  /* load srr0 */			\
    	    "lwz %%r2,  %8 (%%r1) ;"  /* restore r31 */			\
    	    "lwz %%r31, %2 (%%r1) ;"  /* restore r31 */			\
    	    "lwz %%r30, %3 (%%r1) ;"  /* restore r30 */			\
    	    "lwz %%r12, %6 (%%r1) ;"  /* load srr1 */			\
    	    "lwz %%r1, %5 (%%r1) ;"   /* restore the user stack */	\
    	    "mtsrr1 %%r12 ;"	  /* restore srr1 */			\
	    "mtsrr0 %%r13 ;"      /* restore srr0 */			\
    	    "rfi ;"							\
	    :								\
	    : "r" (param1),				/* 0 */		\
	      "i" (KTCB_BITS),  			/* 1 */		\
	      "i" (TCB_STATE(r31)),			/* 2 */		\
	      "i" (TCB_STATE(r30)),	 		/* 3 */		\
	      "i" (TCB_STATE(srr0_ip)), 		/* 4 */		\
	      "i" (TCB_STATE(r1_stack)), 		/* 5 */		\
	      "i" (TCB_STATE(srr1_flags)),		/* 6 */		\
	      "r" (param2),				/* 7 */		\
	      "i" (TCB_STATE(r2_local_id))		/* 8 */		\
	    : "r3", "r4", "r12"						\
	);								\
    while(1);								\
} while(0)

#define return_space_control( result, control )		\
	return_user_2params( result, control )
#define return_schedule( result, time_control )		\
	return_user_2params( result, time_control )

#define return_user_7params( param1, param2, param3, param4, param5, param6, param7 )									\
do {									\
    asm volatile (							\
	    "mr %%r3, %0 ;"	/* set return result  */		\
	    "mr %%r4, %4 ;"	/* set return param 2 */		\
	    "mr %%r5, %5 ;"	/* set return param 3 */		\
	    "mr %%r6, %6 ;"	/* set return param 4 */		\
	    "mr %%r7, %7 ;"	/* set return param 5 */		\
	    "mr %%r8, %8 ;"	/* set return param 6 */		\
	    "mr %%r9, %9 ;"	/* set return param 7 */		\
    	    "clrrwi %%r1, %%r1, %1;"  /* find the start of the tcb */	\
	    "lwz %%r13, %3 (%%r1) ;"       /* load srr0 */		\
	    "lwz %%r2,  (%2-4*4)(%%r1) ;"  /* restore r2 */		\
    	    "lwz %%r31, (%2-6*4)(%%r1) ;"  /* restore r31 */		\
    	    "lwz %%r30, (%2-7*4)(%%r1) ;"  /* restore r30 */		\
    	    "lwz %%r12, (%2-1*4)(%%r1) ;"  /* load srr1 */		\
    	    "lwz %%r1,  (%2-5*4)(%%r1) ;"  /* restore the user stack */	\
    	    "mtsrr1 %%r12 ;"	  /* restore srr1 */			\
	    "mtsrr0 %%r13 ;"      /* restore srr0 */			\
    	    "rfi ;"							\
	    :								\
	    : "r" (param1),				/* 0 */		\
	      "i" (KTCB_BITS),  			/* 1 */		\
	      "i" (TOTAL_TCB_SIZE),			/* 2 */		\
	      "i" (TCB_STATE(srr0_ip)), 		/* 3 */		\
	      "r" (param2),				/* 4 */		\
	      "r" (param3),				/* 5 */		\
	      "r" (param4),				/* 6 */		\
	      "r" (param5),				/* 7 */		\
	      "r" (param6),				/* 8 */		\
	      "r" (param7)				/* 9 */		\
	    : "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r12"		\
	);								\
    while(1);								\
} while(0)

#define return_exchange_registers( result, control, sp, ip, flags, pager, handle ) return_user_7params( result, control, sp, ip, flags, pager, handle )

#define return_user_with_MRs()						\
do {									\
    utcb_t *utcb = get_current_tcb()->get_utcb();			\
    asm volatile (							\
	    "mr  %%r25, %7;"          /* our utcb location */		\
    	    "clrrwi %%r1, %%r1, %0;"  /* find the start of the tcb */	\
	    "lwz %%r27, %3 (%%r1) ;"  /* load srr0 */			\
	    "lwz %%r2,  %6 (%%r1) ;"  /* restore r2 */			\
    	    "lwz %%r31, %1 (%%r1) ;"  /* restore r31 */			\
    	    "lwz %%r30, %2 (%%r1) ;"  /* restore r30 */			\
    	    "lwz %%r26, %5 (%%r1) ;"  /* load srr1 */			\
    	    "lwz %%r1, %4 (%%r1) ;"   /* restore the user stack */	\
	    "lwz " MKSTR(IPC_ABI_MR0) ", (%8 +  0)(%%r25) ;"		\
	    "lwz " MKSTR(IPC_ABI_MR1) ", (%8 +  4)(%%r25) ;"		\
	    "lwz " MKSTR(IPC_ABI_MR2) ", (%8 +  8)(%%r25) ;"		\
	    "lwz " MKSTR(IPC_ABI_MR3) ", (%8 + 12)(%%r25) ;"		\
	    "lwz " MKSTR(IPC_ABI_MR4) ", (%8 + 16)(%%r25) ;"		\
	    "lwz " MKSTR(IPC_ABI_MR5) ", (%8 + 20)(%%r25) ;"		\
	    "lwz " MKSTR(IPC_ABI_MR6) ", (%8 + 24)(%%r25) ;"		\
	    "lwz " MKSTR(IPC_ABI_MR7) ", (%8 + 28)(%%r25) ;"		\
	    "lwz " MKSTR(IPC_ABI_MR8) ", (%8 + 32)(%%r25) ;"		\
	    "lwz " MKSTR(IPC_ABI_MR9) ", (%8 + 36)(%%r25) ;"		\
	    "mtsrr0 %%r27 ;"          /* restore srr0 */		\
    	    "mtsrr1 %%r26 ;"          /* restore srr1 */		\
    	    "rfi ;"							\
	    :								\
	    : "i" (KTCB_BITS), 				/* 0 */		\
	      "i" (TCB_STATE(r31)),			/* 1 */		\
	      "i" (TCB_STATE(r30)), 			/* 2 */		\
	      "i" (TCB_STATE(srr0_ip)),			/* 3 */		\
	      "i" (TCB_STATE(r1_stack)), 		/* 4 */		\
	      "i" (TCB_STATE(srr1_flags)),		/* 5 */		\
	      "i" (TCB_STATE(r2_local_id)),		/* 6 */		\
	      "r" (utcb),				/* 7 */		\
	      /* MR0 offset from UTCB start. */				\
	      "i" ((word_t)&utcb->mr[0] - (word_t)utcb)	/* 8 */		\
	);								\
    while(1);								\
} while(0)

#define return_unmap() return_user_with_MRs()

#if !defined(ASSEMBLY)
extern "C" {
extern void _sc_schedule( void );
extern void _sc_thread_switch( void );
extern void _sc_system_clock( void );
extern void _sc_xchg_registers( void );
extern void _sc_unmap( void );
extern void _sc_ipc( void );
extern void _sc_lipc( void );
extern void _sc_memory_ctrl( void );
extern void _sc_processor_ctrl( void );
extern void _sc_thread_ctrl( void );
extern void _sc_space_ctrl( void );
extern void _sc_perf( void );
}
#endif

#endif /* __GLUE__V4_POWERPC__SYSCALLS_H__ */
