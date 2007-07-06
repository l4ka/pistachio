/*********************************************************************
 *
 * Copyright (C) 2003-2004,  National ICT Australia (NICTA)
 *
 * File path:      glue/v4-arm/traps.S
 * Description:    Exception vectors
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
 * $Id: traps.S,v 1.28 2004/12/01 23:58:27 cvansch Exp $
 *
 ********************************************************************/

#include INC_ARCH(thread.h)
#include INC_GLUE(syscalls.h)
#include INC_ARCH(asm.h)
#include INC_ARCH(fass.h)

#include <tcb_layout.h>
#include <asmsyms.h>

.balign 4096

/* Relative branches, loads and stores to locations outside this 4K page are
 * broken, as this is remapped to the high interrupt vector 0xFFFF0000
 */
BEGIN_PROC(arm_high_vector)
	b	arm_reset_exception
	b	arm_undefined_inst_exception
	b	arm_swi_exception 
	b	arm_l4_syscall
	b	arm_data_abort_exception
	nop
	b	arm_irq_exception
	b	arm_fiq_exception
END_PROC(arm_high_vector)

arm_common_return:
	SET_USER_DACR
	RESTORE_ALL
	movs	pc,	lr

arm_reset_exception:
	/* Save R13_abt, SPSR_abt */
	sub	lr,	lr,	#8
	str	lr,	tmp_r14_abt
	mrs	lr,	spsr
	str	lr,	tmp_spsr_abt

	/* Enter supervisor mode, IRQ/FIQ disabled */
	msr	cpsr_c,	#0x000000d3

	/* since SAVE_ALL_INT only does user's banked lr */
	str	lr,	[sp, #(-PT_SIZE + PT_KLR)]

	ldr	lr,	tmp_r14_abt
	SAVE_ALL_INT
	SAVE_CPSR_MRS 
	SET_KERNEL_DACR

	/* Call C function reset_exception(arm_irq_context_t *) */
	mov	r0,	sp
	ldr	r1,	=reset_exception
	adr	lr,	arm_common_return
	mov	pc,	r1

arm_undefined_inst_exception:
	/* Save R13_abt, SPSR_abt */
	sub	lr,	lr,	#8
	str	lr,	tmp_r14_abt
	mrs	lr,	spsr
	str	lr,	tmp_spsr_abt

	/* Enter supervisor mode, IRQ/FIQ disabled */
	msr	cpsr_c,	#0x000000d3

	/* since SAVE_ALL_INT only does user's banked lr */
	str	lr,	[sp, #(-PT_SIZE + PT_KLR)]

	ldr	lr,	tmp_r14_abt
	SAVE_ALL_INT
	SAVE_CPSR_MRS
	SET_KERNEL_DACR

	/* Call C function undefined_exception(arm_irq_context_t *) */
	mov	r0,	sp
	ldr	r1,	=undefined_exception
	adr	lr,	arm_common_return
	mov	pc,	r1

arm_fiq_exception:
	/* Save R13_abt, SPSR_abt */
	sub	lr,	lr,	#8
	str	lr,	tmp_r14_abt
	mrs	lr,	spsr
	str	lr,	tmp_spsr_abt

	/* Enter supervisor mode, IRQ/FIQ disabled */
	msr	cpsr_c,	#0x000000d3

	/* since SAVE_ALL_INT only does user's banked lr */
	str	lr,	[sp, #(-PT_SIZE + PT_KLR)]

	ldr	lr,	tmp_r14_abt
	SAVE_ALL_INT
	SAVE_CPSR_MRS
	SET_KERNEL_DACR

	/* Call C function fiq_exception(arm_irq_context_t *) */
	mov	r0,	sp
	ldr	r1,	=fiq_exception
	adr	lr,	arm_common_return
	mov	pc,	r1

arm_swi_exception:
	SAVE_ALL_INT
	SAVE_CPSR_MRS
	SET_KERNEL_DACR

	/* Call C function syscall_exception(arm_irq_context_t *) */
	mov	r0,	sp
	ldr	r1,	=syscall_exception
	adr	lr,	arm_common_return
	mov	pc,	r1
 
arm_l4_syscall:
	/* Save R14_abt, SPSR_abt */
	sub	lr,	lr,	#4
	str	lr,	tmp_r14_abt
	mrs	lr,	spsr
	str	lr,	tmp_spsr_abt

	/* Enter supervisor mode, IRQ/FIQ disabled */
	msr	cpsr_c,	#0x000000d3

	ldr	lr,	tmp_r14_abt

	/* Test to see if it is a syscall */
	cmp	lr,	#0xffffff00
#ifdef CONFIG_IPC_FASTPATH
	bne	arm_non_ipc_syscall

#define	to_tid		r0
#define	from_tid	r1
#define	timeouts	r2
#define	mr0		r3
#define	mr1		r4
#define	mr2		r5
#define	mr3		r6
#define	mr4		r7

#define	to_tcb		r8
#define	current		r9
#define	tmp1		r10
#define	tmp2		r11
#define	tmp3		r12
#define	tmp4		r2	    /* only use after continue_ipc! */
#define	tmp5		r1	    /* only use after last from_tid use! */
#define	tmp6		lr

#if (KTCB_BITS != 12)
#error KTCB_BITS changed
#endif
#if ((KTCB_AREA_START) != 0xe0000000)
#error KTCB_AREA moved
#endif
#if ((USER_UTCB_REF_PAGE) != 0xff000000)
#error KTCB_AREA moved
#endif

	/***** Fast path IPC *****/

	/* It is a syscall, so save the user's banked SP and LR as well as
	 * CPSR (SETUP)
	 */
	ldr	r12,	tmp_spsr_abt				/* SETUP */

	/* Look for a nil to-tid. */				/* TEST0 */
	cmp	to_tid, #0					/* TEST0 */

	/* Check for typed messages and flags	bits 15..6 = 0	   TEST1 */
	movne	tmp1,	mr0, LSL #16				/* TEST1 */

	stmdb	sp,	{r12, sp, lr}^				/* SETUP */
	nop							/* SETUP */
	sub	sp,	sp,	#ARM_IPC_STACK_SIZE		/* SETUP */

	beq	arm_slowpath					/* TEST0 */

	movs	tmp1,	tmp1, LSR #22				/* TEST1 */
//	bne	arm_slowpath					/* TEST1 */

	/* Calculate to_tcb (destination tcb)			   CALC1 */
	moveq	to_tcb,	to_tid, LSL #(32 - VALID_THREADNO_BITS - L4_GLOBAL_VERSION_BITS)	/* CALC1 */
	moveq	to_tcb, to_tcb, LSR #(32 - VALID_THREADNO_BITS)	/* CALC1 */

	/* Calculate current tcb				   CALC2 */
	moveq	current, sp, LSR #12				/* CALC2 */

	moveq	to_tcb, to_tcb, LSL #12				/* CALC1 */
	addeq	to_tcb,	to_tcb, #0xe0000000			/* CALC1 */

	ldreq	tmp2,	[to_tcb, #OFS_TCB_MYSELF_GLOBAL]	/* TEST4 */
	ldreq	tmp3,	[to_tcb, #OFS_TCB_RESOURCE_BITS]	/* TEST9 */

	moveq	current, current, LSL #12			/* CALC2 */
	ldreq	tmp6,	[current, #OFS_TCB_RESOURCE_BITS]	/* TEST10 */

	/* Check that the receive timeout is infinite		   TEST3 | (lower 16 timeout bits == 0) */
	moveqs	tmp1,	timeouts, LSL #16			/* TEST3 */
//	bne	arm_slowpath					/* TEST3 */

	/* Check to_tcb->get_global_id == to_tid		   TEST4 */
	cmpeq	tmp2,	to_tid					/* TEST4 */
//	bne	arm_slowpath					/* TEST4 */

	ldreq	tmp1,	[to_tcb, #OFS_TCB_THREAD_STATE]		/* TEST5 */

	/* Check if any resource bits are set			   TEST9 | TEST10 */
	orreqs	tmp3,	tmp3, tmp6				/* TEST9 | TEST10 */
//	bne	arm_slowpath					/* TEST9 | TEST10 */

	ldreq	tmp6,	[to_tcb, #OFS_TCB_SPACE]		/* TEST11 */
	/* Check partner (to_tcb) is waiting			   TEST5 */
	cmpeq	tmp1,	#-1					/* TEST5 */
	bne	arm_slowpath					/* TEST5 */

	ldr	tmp1,	[to_tcb, #OFS_TCB_PARTNER]		/* TEST6 */
	/* Check if to_tcb->space == NULL			   TEST11 */
	cmp	tmp6,	#0					/* TEST11 */
	beq	arm_slowpath					/* TEST11 */

	ldr	tmp3,	[current, #OFS_TCB_MYSELF_GLOBAL]	/* TEST6 */
	/* tcb->get_partner().is_anythread()			   TEST6 */
	cmp	tmp1,	#-1					/* TEST6 */
	beq	1f						/* TEST6 */
	/* tcb->get_partner() == current->get_global_id()	   TEST6 */
	cmp	tmp1,	tmp3					/* TEST6 */
	bne	arm_slowpath					/* TEST6 */
1:
	/* Check if IPC is a Call				   TEST12 */
	cmp	to_tid,	from_tid				/* TEST12 */
	beq	2f						/* TEST12 */
	
	/* Require send_head to be empty			   TEST8 */
	ldr	tmp1,	[current, #OFS_TCB_SEND_HEAD]		/* TEST8 */
	cmp	tmp1,	#0					/* TEST8 */
	bne	arm_slowpath					/* TEST8 */

	/* XXX reorder ? */
	/* Check that receive phase blocks			   TEST7 */
	cmp	from_tid,   #-1					/* TEST7 */
	bne	check_other_tcb					/* TEST7 */

2:

continue_ipc:
#ifdef	CONFIG_ENABLE_FASS
	/* Macro uses register ip (r12 / tmp3) */
	SET_KERNEL_DACR

	/* Touch the current thread's UTCB to fault it in if needed
	 * This is done before SET_KERNEL_DACR! */
	ldr	tmp1,	[current, #OFS_TCB_MYSELF_LOCAL]
	ldr	tmp1,	[tmp1, #84]

	add	tmp1,	tmp6,	#OFS_SPACE_DOMAIN
	ldr	tmp6,	[tmp1]
	mov	tmp6,	tmp6,	LSR #2
	cmp	tmp6,	#INVALID_DOMAIN
	beq	arm_slowpath

#else
	/* Check for address space switch */
	ldr	tmp3,	[current, #OFS_TCB_SPACE]
	cmp	tmp6,	tmp3
	bne	arm_slowpath
#endif

	mov	tmp2,	#-1
	/* Set thread state to waiting				   STORE1 */
	str	tmp2,	[current, #OFS_TCB_THREAD_STATE]	/* STORE1  tmp2 = -1 */
	/* Set partner						   STORE2/3 */
	str	from_tid,	[current, #OFS_TCB_PARTNER]	/* STORE2 */
#if 0
	ldr	tmp3,	[current, #OFS_TCB_MYSELF_GLOBAL]	/* STORE3 */
	str	tmp3,	[to_tcb, #OFS_TCB_PARTNER]		/* STORE3 */
#endif

	/* Use copy loop if more than 5 message registers	   TEST2 */
	and	tmp1,	mr0,	#0x3f				/* TEST2 */
	subs	tmp1,	tmp1,	#5				/* TEST2 */
	bmi	fast_path_switch_to				/* TEST2 */

	/* current utcb */
	ldr	tmp2,	[current, #OFS_TCB_MYSELF_LOCAL]

	/* destination utcb */
#ifdef	CONFIG_ENABLE_FASS

	/* cpd_set[domain][CPD_BITFIELD_POS(utcb_section)] &
	            (1 << CPD_BITFIELD_OFFSET(utcb_section)) */

	ldr	tmp5,	[to_tcb, #OFS_TCB_MYSELF_LOCAL]
	ldr	tmp4,	arm_fass_ptr
	mov	tmp3,	tmp6,	LSL #(12 - 5)
	add	tmp3,	tmp3,	tmp5,	LSR #(20 + 5)
	ldr	tmp3,	[tmp4, tmp3, LSL #2]

	mov	tmp4,	tmp5,	LSR #20
	and	tmp4,	tmp4,	#31
	mov	tmp5,	#1
	ands	tmp3,	tmp3,	tmp5,	LSL tmp4
	ldrne	tmp3,	[to_tcb, #OFS_TCB_MYSELF_LOCAL]
	bne	4f

	/* SET_BIT_WORD */
	ldr	tmp3,	arm_utcb_dirty_ptr
	ldr	tmp4,	[tmp3]
	orr	tmp4,	tmp4,	tmp5,	LSL tmp6
	str	tmp4,	[tmp3]

	ldr	tmp3,	[to_tcb, #OFS_TCB_UTCB]
4:
#else	/* No FASS */
	/* Non-fass only gets here for Intra address space IPC */
	ldr	tmp3,	[to_tcb, #OFS_TCB_MYSELF_LOCAL]
#endif
	/* tmp1 = num to copy - 1
	 * tmp2 = from utcb
	 * tmp3 = to utcb	    */
	add	tmp2,	tmp2,	#84
	add	tmp3,	tmp3,	#84

copy_loop:
	ldr	tmp4,	[tmp2],	#4
	subs	tmp1,	tmp1,	#1
	str	tmp4,	[tmp3],	#4
	bpl	copy_loop

fast_path_switch_to:
#ifdef CONFIG_ENABLE_FASS
	/* Check for address space switch */
	ldr	tmp2,	[to_tcb, #OFS_TCB_SPACE]
	ldr	tmp3,	[current, #OFS_TCB_SPACE]
	cmp	tmp2,	tmp3
	beq	3f

	/* ACTIVATE NEW DOMAIN */
	/* current_domain = target */
	ldr	tmp4,	arm_current_ptr
	mov	tmp1,	#1
	str	tmp6,	[tmp4]

	/* SET_BIT_WORD(domain_dirty, target) */
	mov	tmp1,	tmp1,	LSR tmp6
	ldr	tmp4,	arm_domain_ptr
	ldr	tmp3,	[tmp4]
	orr	tmp3,	tmp3,	tmp1
	str	tmp3,	[tmp4]

	/* current_pid = get_pid */
	ldr	tmp1,	[to_tcb, #OFS_TCB_SPACE]
	ldr	tmp4,	arm_pid_ptr
	add	tmp1,	tmp1,	#OFS_SPACE_DOMAIN
	ldr	tmp3,	[tmp1,	#(OFS_SPACE_PID - OFS_SPACE_DOMAIN)]
	str	tmp3,	[tmp4]
3:
#endif

	/* Set new UTCB XXX - if we fault after this, (before switch) is this bad? */
	ldr	tmp3,	[to_tcb, #OFS_TCB_MYSELF_LOCAL]
	mov	tmp1,	#0xff000000
	str	tmp3,	[tmp1]

	/* Set fast path return address */
	adr	tmp1,	fast_path_recover

	/* Create switch stack */
	sub	sp,	sp,	#(4*4)
	str	tmp1,	[sp, #12]	/* Save return address */

	/* Save stack */
	str	sp,	[current, #OFS_TCB_STACK]

	/* Set destination thread to running */
	mov	tmp1,	#TSTATE_RUNNING
	str	tmp1,	[to_tcb, #OFS_TCB_THREAD_STATE]

	/* Load new stack */
	add	sp,	to_tcb,	#KTCB_SIZE

	/* Clean up mr0 (clear receive flags) */
	and	mr0,	mr0,	#(~(0xe << 12))

	/* Load result (should be cached from before XXX) */
	ldr	r0,	[current, #OFS_TCB_MYSELF_GLOBAL]

	/* trashes ip/r12 and lr */
	/* XXX - optimize - don't do SET_USER_DACR unless we switch address spaces */
	SET_USER_DACR

	/* restore the user's banked SP, LR, CPSR */
	sub	tmp1,	sp,	#ARM_IPC_STACK_SIZE
	ldmia	tmp1,	{r12, sp}^
	nop
	ldr	lr,	[sp, #-4]
	msr	spsr,	r12

	movs	pc,	lr

check_other_tcb:
	/* use tmp1, tmp2, tmp3, tmp6 only */

	/* from_tid == 0?					   TEST13 */
	cmp	from_tid,   #0					/* TEST13 */
	beq	arm_slowpath					/* TEST13 */

	/* Calculate from_tcb					   CALC4 */
	mov	tmp1,	from_tid, LSL #(32 - VALID_THREADNO_BITS - L4_GLOBAL_VERSION_BITS)	/* CALC4 */
	mov	tmp1,	tmp1,	LSR #(32 - VALID_THREADNO_BITS)	/* CALC4 */
	mov	tmp1,	tmp1,	LSL #12				/* CALC4 */
	add	tmp1,	tmp1,	#0xe0000000			/* CALC4 */

	/* Check global ID */
	ldr	tmp3,	[tmp1, #OFS_TCB_MYSELF_GLOBAL]
	ldr	tmp6,	[tmp1, #OFS_TCB_THREAD_STATE]

	cmp	tmp3,	from_tid
	bne	arm_slowpath

	/*
	 * Check if the thread is polling us --- if so, go to slow path
	 */

	/* is_polling() */	
	cmp	tmp6,	#TSTATE_POLLING
	bne	continue_ipc	    /* from_tcb isn't polling */

	/* partner == current->global_id */
	ldr	tmp3,	[tmp1, #OFS_TCB_PARTNER]
	ldr	tmp6,	[current, #OFS_TCB_MYSELF_GLOBAL]
	cmp	tmp3,	tmp6
	beq	arm_slowpath

	/* partner == current->local_id */
	ldr	tmp6,	[current, #OFS_TCB_MYSELF_LOCAL]
	cmp	tmp3,	tmp6
	beq	continue_ipc

	b	arm_slowpath

fast_path_recover:
	mov	tmp1,	sp,	LSR #12
	mov	current,tmp1,	LSL #12
	add	sp,	current,#KTCB_SIZE
	sub	sp,	sp,	#ARM_IPC_STACK_SIZE

	/* Set the state to running */
	mov	tmp1,	#TSTATE_RUNNING
	str	tmp1,	[current, #OFS_TCB_THREAD_STATE]

	/* Load result */
	ldr	r0,	[current, #OFS_TCB_PARTNER]

	b	ipc_syscall_return

arm_slowpath:
	mov	r12,	#0xff000000
	ldr	r12,	[r12]
	add	r12,	r12,	#64
	stmia	r12,	{r3-r7}

	/* Macro uses register ip/r12 */
	SET_KERNEL_DACR

	adr	lr,	ipc_syscall_return
	ldr	pc,	arm_sys_ipc

arm_non_ipc_syscall:
#endif
        bcc     arm_prefetch_abort_exception

	/* svc SP should point to current thread's stack in KTCB - this is
	* given as it is an invariant that the banked svc SP is to the
	* currently executing thread's KTCB stack
	*/

	/* Test to see if it is a syscall */
	and	lr,	lr,	#0x000000ff
	cmp	lr,	#SYSCALL_limit
	bhi	arm_non_l4_syscall

	SET_KERNEL_DACR

	/* It is a syscall, so save the user's banked SP and LR as well as
	 * CPSR
	 */

	ldr	r12,	tmp_spsr_abt
	stmdb	sp,	{r12, sp, lr}^
	nop
	sub	sp,	sp,	#ARM_IPC_STACK_SIZE

	/* Calling registers:
	 *   r0, r1, r2, r3, r4, r5, r6, r7 : arguments 1 - 8
	 * Retuned registers:
	 *   r0, r1, r2, r3, r4, r5, r6     : returned 1 - 7
	 */
	mov	r12,	lr
	adr	lr,	syscall_return
	ldr	pc,	[pc, r12 /* , lsl #2*/]
	nop

arm_sys_ipc:
.word	sys_ipc
.word	sys_thread_switch
.word	sys_thread_control_exargs
.word	sys_exchange_registers_exargs
.word	sys_schedule_exargs 
.word	sys_unmap 
.word	sys_space_control
.word	syscall_return /* sys_processor_control */
.word	sys_memory_control_exargs 
.word   sys_clock
.word   sys_ipc /* lipc */

#define SYS_EXARGS(name, lastreg, numregs)	\
name##_exargs:					\
	stmdb	sp!,	{r4##lastreg};		\
	ldr	r12,	=name##;		\
	mov	lr,	pc;			\
	mov	pc,	r12;			\
						\
	add	sp,	sp,     #(numregs * 4);	\
	b	syscall_return;			

SYS_EXARGS(sys_thread_control, , 1)
SYS_EXARGS(sys_exchange_registers,-r7,4)
SYS_EXARGS(sys_schedule, , 1)
SYS_EXARGS(sys_memory_control, , 1)

arm_domain_ptr:
	.word	domain_dirty
arm_current_ptr:
	.word	current_domain
arm_pid_ptr:
	.word	current_pid
#ifdef CONFIG_ENABLE_FASS
arm_fass_ptr:
	.word	arm_fass
arm_utcb_dirty_ptr:
	.word	utcb_dirty
#endif

.global ipc_syscall_return
ipc_syscall_return:
#ifdef CONFIG_IPC_FASTPATH

	SET_USER_DACR

	mov	r12,	#0xff000000
	ldr	r12,	[r12]
	add	r12,	r12,	#64
	ldmia	r12,	{r3-r7}

	/* restore the user's banked SP, LR, CPSR */

	ldmia	sp,	{r12, sp}^
	nop
	add	sp,	sp,	#ARM_IPC_STACK_SIZE
	ldr	lr,	[sp, #-4]
	msr	spsr,	r12

	movs	pc,	lr
#endif

.global syscall_return
syscall_return:
	SET_USER_DACR
	/* restore the user's banked SP, LR, CPSR */

	ldmia	sp,	{r12, sp}^
	nop
	add	sp,	sp,	#ARM_IPC_STACK_SIZE
	ldr	lr,	[sp, #-4]
	msr	spsr,	r12

	movs	pc,	lr

arm_non_l4_syscall:
	SAVE_ALL_INT
	SAVE_CPSR_TMP
	SET_KERNEL_DACR

	/* Call C function arm_swi(arm_irq_context_t *) */
	mov	r0,	sp
	ldr	r1,	=arm_misc_l4_syscall
	mov	lr,	pc
	mov	pc,	r1

	SET_USER_DACR
	RESTORE_ALL

	ldr	lr,	[sp, #(-PT_SIZE + PT_LR)]

	movs	pc,	lr

arm_prefetch_abort_exception:
	/* Even if the fault came from the kernel, it won't be on the current
	 * stack as KTCBs are faulted on for allocation prior to the use of
	 * their stacks 
	 */

	SAVE_ALL_INT
	SAVE_CPSR_TMP
	SET_KERNEL_DACR

	/* Pointer to base of current arm_irq_context_t record */
	mov	r2,	sp

	/* Faulting address */
	ldr	r1,	[sp, #PT_PC]

	/* Assume it was a page fault for now */

	/* Fault status - not updated on prefetch abort */
	mov	r0,	#0

	mov	r3,	#0

	/* Process the page fault */
 
	ldr	r4,	=arm_page_fault
	adr	lr,	arm_abort_return
	mov	pc,	r4

arm_data_abort_exception:
	/* Save R13_abt, SPSR_abt */
	sub	lr,	lr,	#8
	str	lr,	tmp_r14_abt
	mrs	lr,	spsr
	str	lr,	tmp_spsr_abt

	/* Enter supervisor mode, IRQ/FIQ disabled */
	msr	cpsr_c,	#0x000000d3

	/* since SAVE_ALL_INT only does user's banked lr */
	str	lr,	[sp, #(-PT_SIZE + PT_KLR)]

	ldr	lr,	tmp_r14_abt

	/* Even if the fault came from the kernel, it won't be on the current
	 * stack as KTCBs are faulted on for allocation prior to the use of
	 * their stacks
	 */
	
	SAVE_ALL_INT
	SAVE_CPSR_TMP
	SET_KERNEL_DACR

	/* Pointer to base of current arm_irq_context_t record */
	mov	r2,	sp

	/* Fault status */
	mrc	p15, 0, r0, c5, c0, 0

	/* Faulting address */
	mrc	p15, 0, r1, c6, c0, 0

	mov	r3,	#1

	ldr	r4,	=arm_page_fault
	mov	lr,	pc
	mov	pc,	r4

.global arm_abort_return
arm_abort_return:
	SET_USER_DACR_K
	RESTORE_ALL

	str	lr,	tmp_r14_abt
	mrs	lr,	spsr
	str	lr,	tmp_spsr_abt

	ldr	lr,	[sp, #(-PT_SIZE + PT_KLR)]

	/* Enter abort mode, IRQ/FIQ disabled */
	msr	cpsr_c,	#0x000000d7

	/* Restore R13_abt, SPSR_abt */
	ldr	lr,	tmp_spsr_abt
	msr	spsr,	lr
	ldr	lr,	tmp_r14_abt

	movs	pc,	r14

arm_irq_exception:
	/* Save R13_abt, SPSR_abt */
	sub	lr,	lr,	#4
	str	lr,	tmp_r14_abt
	mrs	lr,	spsr
	str	lr,	tmp_spsr_abt

	/* Enter supervisor mode, IRQ/FIQ disabled */
	msr	cpsr_c,	#0x000000d3

	/* since SAVE_ALL_INT only does user's banked lr */
	str	lr,	[sp, #(-PT_SIZE + PT_KLR)]

	ldr	lr,	tmp_r14_abt

	SAVE_ALL_INT
	SAVE_CPSR_TMP
	SET_KERNEL_DACR

	mov	r0,	sp
       
	ldr	r4,	=arm_irq
	mov	lr,	pc
	mov	pc,	r4

	SET_USER_DACR_K
	RESTORE_ALL

	str	lr,	tmp_r14_abt
	mrs	lr,	spsr
	str	lr,	tmp_spsr_abt

	ldr	lr,	[sp, #(-PT_SIZE + PT_KLR)]

	/* Enter irq mode, IRQ/FIQ disabled */
	msr	cpsr_c, #0x000000d2

	/* Restore R13_abt, SPSR_abt */
	ldr	lr,	tmp_spsr_abt
	msr	spsr,	lr
	ldr	lr,	tmp_r14_abt

	movs	pc,	lr

.balign 32

.global utcb_dirty
utcb_dirty: 
	.word	0xdeadbeef

.global domain_dirty
domain_dirty:
	.word	0xdeadbeef

.global current_domain
current_domain:
	.word	0xdeadbeef

.global current_pid
current_pid:
	.word	0xdeadbeef

.balign 32

tmp_r14_abt:
	.word	0xdeadbeef
tmp_spsr_abt:
	.word	0xdeadbeef

.ltorg

.balign 4096

