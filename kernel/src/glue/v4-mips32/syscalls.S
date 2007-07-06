/*********************************************************************
 *                
 * Copyright (C) 2006,  Karlsruhe University
 *                
 * File path:     glue/v4-mips32/syscalls.S
 * Description:   System call entry points for MIPS32
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
 * $Id: syscalls.S,v 1.1 2006/02/23 21:07:46 ud3 Exp $
 *                
 ********************************************************************/

#include INC_ARCH(asm.h)
#include INC_ARCH(regdef.h)
#include INC_GLUE(syscalls.h)
#include INC_GLUE(context.h)
#include <tcb_layout.h>


#define mr0	    s0
#define mr1	    s1
#define mr2	    s2
#define mr3	    s3
#define mr4	    s4
#define mr5	    s5
#define mr6	    s6
#define mr7	    s7


#define DECODE_SYSCALL_IPC									\
1:;												\
	.set push;										\
	.set noreorder;										\
	li	t4, SYSCALL_ipc;								\
	bne	v0, t4, 1f;									\
	lui	ra, %hi(mips32_l4sysipc_return);                                                \
	li	t4, -4096;					/* tcb mask */			\
	and	t5, sp, t4;					/* t5 = current tcb */          \
	lw	t5, OFS_TCB_UTCB(t5);		/* t5 = current utcb */                         \
	addiu	ra, %lo(mips32_l4sysipc_return);                                                \
	sw	s8, PT_S8(sp);									\
	sw	mr0, 64(t5);									\
	sw	mr1, 68(t5); 									\
	sw	mr2, 72(t5); 									\
	sw	mr3, 76(t5); 									\
	sw	mr4, 80(t5); 									\
	sw	mr5, 84(t5); 									\
	sw	mr6, 88(t5); 									\
	sw	mr7, 92(t5); 									\
	j	sys_ipc;									\
	subu sp, sp, 0x10;									\
1:;												\
	lui	ra, %hi(mips32_l4syscall_return);                                               \
	subu	t4, t4, 1;									\
	.set reorder;										\
	.set pop; 






#define DECODE_SYSCALL(name, vector)                    \
	.set push;					\
	.set noreorder;					\
1:	bne	v0, t4, 1f;				\
	subu	t4, t4, 1;				\
	addiu	ra, %lo(mips32_l4syscall_return);       \
	j	sys_##name;				\
	subu sp, sp, 0x10;				\
	.set pop;					\
	.set reorder;

#define DECODE_SYSCALL_FIVE_IN(name)                            \
	.set push;						\
	.set noreorder;						\
1:	bne	v0, t4, 1f;					\
	subu	t4, t4, 1;					\
	addiu	ra, %lo(mips32_l4syscall_five_in_return);	\
								\
	subu sp, sp, 0x14; /* Space for 5 in parameters */	\
	sw s0, 0x10(sp); /* pass arg no 5 on stack */		\
								\
	j	sys_##name;					\
	nop;							\
	.set reorder;						\
	.set pop; 


#define DECODE_SYSCALL_EIGHT_IN(name)				\
	.set push;						\
	.set noreorder;						\
1:	bne	v0, t4, 1f;					\
	subu	t4, t4, 1;					\
	addiu	ra, %lo(mips32_l4syscall_eight_in_return);	\
								\
	subu sp, sp, 0x20; /* Space for 8 in parameters */	\
	sw	s0, 0x10(sp); /* pass arg no 5 on stack */	\
	sw	s1, 0x14(sp); /* pass arg no 6 on stack */	\
	sw	s2, 0x18(sp); /* pass arg no 7 on stack */	\
	sw	s3, 0x1C(sp); /* pass arg no 8 on stack */	\
								\
	j	sys_##name;					\
	nop;							\
	.set reorder;						\
	.set pop;






#define SKIPDECODE_SYSCALL(name, vector)	\
	.set push;				\
	.set noreorder;				\
1:	bne	v0, t4, 1f;			\
	subu	t4, t4, 1;			\
	li	AT, L4_TRAP_KDEBUG;		\
	la	v0, 2f;				\
	break;					\
	.data;					\
2:	.string "Unsupported SYSCALL";		\
	.previous;				\
	.set pop;				\
	.set reorder;



BEGIN_PROC(mips32_l4syscall)
	
	/* Trashed registers:
	 *   t4, t5, t6, t7
	 * Calling Registers:
	 *   v0 : syscall number
	 *   a0, a1, a2, a3 : arguments 1 - 4
	 *   s0, s1, s2, s3 : arguments 5 - 8
	 * Returned Registers:
	 *   v0	    : returned 1
	 *   a0..a3 : returned 2 - 5
	 *   t0..t1 : returned 6 - 7
	 */
	
	.set noat
	bgez	v0, mips32_syscall_exception	/* Not an L4 system call? */
	lui	t5, %hi(K_STACK_BOTTOM)		/* Load kernel stack base address */
	
	mfc0	t7, CP0_STATUS			/* get STATUS register */

	move	t4, sp				/* Old stack in t4 */

	srl	t6, t7, 5			/* clear IE, EXL, ERL, UM */
	sll	t6, t6, 5
	mtc0	t6, CP0_STATUS			/* Enter kernel mode */
	andi	t6, t7, 0x10			/* Isolate UM bit */

	.set noreorder
	beq	t6, zero, 9f			/* Branch if from KERNEL mode */
	mfc0	t6, CP0_EPC			/* Branch delay */
	.set reorder

	lw	sp, %lo(K_STACK_BOTTOM)(t5)	/* Load saved stack */

9:
	addu	t6, t6, 4			/* Calc New EPC */
	subu	sp, sp, PT_SIZE			/* New stack pointer */
	sw	ra, PT_RA(sp)			/* Save RA */
	sw	t7, PT_STATUS(sp)		/* Save status */
	sw	t4, PT_SP(sp)			/* Save stack */
	sw	t6, PT_EPC(sp)			/* Save EPC */

	move    t6, t4				/* top of user stack in t6 */
	li	t4, SYSCALL_ipc

	
	DECODE_SYSCALL_IPC							/* 3 in, 1 out */
	DECODE_SYSCALL(thread_switch, SYSCALL_thread_switch)			/* 1 in, 0 out */
	DECODE_SYSCALL_FIVE_IN(thread_control)					/* 5 in, 1 out */
	DECODE_SYSCALL_EIGHT_IN(exchange_registers)				/* 8 in, 7 out */
	DECODE_SYSCALL_FIVE_IN(schedule)					/* 5 in, 2 out */
	DECODE_SYSCALL(unmap, SYSCALL_unmap)					/* 1 in, 0 out */
	DECODE_SYSCALL_FIVE_IN(space_control)					/* 5 in, 2 out */
	SKIPDECODE_SYSCALL(processor_control,SYSCALL_processor_control)
	SKIPDECODE_SYSCALL(memory_control,SYSCALL_memory_control)		/* 3 in, 1 out */
	DECODE_SYSCALL(clock,SYSCALL_system_clock)				/* 0 in, 1 out */
	nop

1:
	li	AT, L4_TRAP_KDEBUG
	la	v0, 2f
	break
	.data
2:	.string "Unknown SYSCALL"
	.previous
	.set at
END_PROC(mips32_l4syscall)

BEGIN_PROC(mips32_l4sysipc_return)
	.set noat
	addiu	sp, sp, 0x10
	li	t4, -4096			/* tcb mask */
	and	t5, sp, t4			/* t5 = current tcb */
	lw	t5, OFS_TCB_UTCB(t5)            /* t5 = current utcb */
	lw	s8, PT_S8(sp)
	lw	mr0, 64(t5)
	lw	mr1, 68(t5)
	lw	mr2, 72(t5)
	lw	mr3, 76(t5)
	lw	mr4, 80(t5)
	lw	mr5, 84(t5)
	lw	mr6, 88(t5)
	lw	mr7, 92(t5)
	b	17f	
mips32_l4syscall_eight_in_return:
	addiu	sp, sp, 0xc
mips32_l4syscall_five_in_return:
	addiu	sp, sp, 0x4
mips32_l4syscall_return:
	addiu	sp, sp, 0x10
17:	mfc0	t6, CP0_STATUS
	lw	t7, PT_SP(sp)                   /* load stack */
	ori	t6, t6, 0x2			/* set Exception Level */
	mtc0	t6, CP0_STATUS                  /* to disable interrupts, we now can set EPC */
	lw	t4, PT_STATUS(sp)               /* load status */
	lw	t5, PT_EPC(sp)                  /* load epc */
	lw	ra, PT_RA(sp)                   /* load ra */

#STOP_SYSCALL_COUNTER
#PRINT_SYSCALL_TIMES

	mtc0	t5, CP0_EPC			/* restore EPC */ 
	li	t3, 0x0fffffe0			/* CONFIG_MIPS32_STATUS_MASK */
	srl	t5, sp, 12
	move	sp, t7				/* restore stack */
	and	t6, t3, t6			/* compute new status register */
	nor	t3, zero, t3
	and	t4, t3, t4
	or	t7, t6, t4			/*            " "              */
	mtc0	t7, CP0_STATUS			/* new status value */
	sll	t5, t5, 12			/* Get TCB pointer */
	lw	k0, OFS_TCB_MYSELF_LOCAL(t5)	/* Load UTCB into k0 */
	nop
	eret
	.set at
END_PROC(mips32_l4sysipc_return)




BEGIN_PROC(mips32_syscall_exception)
	
	SAVE_ALL_INT
	move	a0, sp
	jal	syscall_exception
	j	mips32_restore_user
	nop

END_PROC(mips32_syscall_exception)
