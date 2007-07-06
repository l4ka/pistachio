/*********************************************************************
 *                
 * Copyright (C) 2002-2003,  University of New South Wales
 *                
 * File path:     glue/v4-mips64/syscalls.S
 * Created:       20/08/2002 by Carl van Schaik
 * Description:   Kernel entry points for syscalls
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
 * $Id: syscalls.S,v 1.24 2004/12/09 01:11:33 cvansch Exp $
 *                
 ********************************************************************/

#include INC_ARCH(asm.h)
#include INC_ARCH(regdef.h)
#include INC_GLUE(context.h)
#include INC_GLUE(syscalls.h)
#include <asmsyms.h>
#include <tcb_layout.h>

#define mr0	    v1
#define mr1	    s0
#define mr2	    s1
#define mr3	    s2
#define mr4	    s3
#define mr5	    s4
#define mr6	    s5
#define mr7	    s6
#define mr8	    s7

#define DECODE_SYSCALL_IPC		    \
1:;					    \
	.set push;			    \
	.set noreorder;			    \
	li	t4, SYSCALL_ipc;	    \
	bne	v0, t4, 1f;		    \
	lui	ra, %hi(_mips64_l4sysipc_return);		    \
	dli	t4, -4096;		    /* tcb mask */	    \
	and	t5, sp, t4;		    /* t5 = current tcb */  \
	ld	t5, OFS_TCB_UTCB(t5);	    /* t5 = current utcb */ \
	daddiu	ra, %lo(_mips64_l4sysipc_return);		    \
	sd	s8, PT_S8(sp);		    \
	sd	mr0, 128(t5);		    \
	sd	mr1, 136(t5);		    \
	sd	mr2, 144(t5);		    \
	sd	mr3, 152(t5);		    \
	sd	mr4, 160(t5);		    \
	sd	mr5, 168(t5);		    \
	sd	mr6, 176(t5);		    \
	sd	mr7, 184(t5);		    \
	j	sys_ipc;		    \
	sd	mr8, 192(t5);		    \
1:;	\
	lui	ra, %hi(_mips64_l4syscall_return);	\
	dsubu	t4, t4, 1;		    \
	.set pop;

#define DECODE_SYSCALL(name, vector)	    \
	.set push;			    \
	.set noreorder;			    \
1:	bne	v0, t4, 1f;		    \
	dsubu	t4, t4, 1;		    \
	j	sys_##name;		    \
	daddiu	ra, %lo(_mips64_l4syscall_return);   \
	.set pop;

#define SKIPDECODE_SYSCALL(name, vector)    \
	.set push;			    \
	.set noreorder;			    \
1:	bne	v0, t4, 1f;		    \
	dsubu	t4, t4, 1;		    \
	li	AT, L4_TRAP_KDEBUG;	    \
	dla	v0, 2f;			    \
	break;				    \
	.data;				    \
2:	.string "Unsupported SYSCALL";	    \
	.previous;			    \
	.set pop;


#if 0
#define	START_SYSCALL_COUNTER	    \
	mfc0	k0, CP0_COUNT;	    \
	nop;			    \
	nop;			    \
	mtc0	k0, CP0_ERROREPC

#define STOP_SYSCALL_COUNTER	    \
	nop;	\
	nop;	\
	nop;	\
	nop;	\
	nop;	\
	nop;	\
	mfc0	t2, CP0_COUNT;	    \
	mfc0	t3, CP0_ERROREPC;   \
	nop;	\
	nop;	\
	dsub	AT, t2, t3;	    \
	dsll	AT, AT, 1;

#if CONFIG_PLAT_ERPCN01
#define d0 k0
#define d1 k1
#define PRINT_SYSCALL_TIMES	\
	li	t2, 28;		\
1:;	\
	dsrlv	t3, AT, t2;	\
	andi	t3, 0xf;	\
	sub	t3, 10;		\
	bgez	t3, 2f;		\
	add	t3, '0'+10;	\
	b 3f;			\
2:;	\
	add	t3, 'a';	\
3:;	\
	dla     d0, propane_uart;   \
	ld	d0, 0(d0);	\
4:;	\
	lw      d1,8(d0);	\
	andi    d1,d1,0xf;	\
	sltiu   d1,d1,13;	\
	beqz    d1,4b;		\
	sw      t3,4(d0);	\
	sub	t2, 4;		\
	bgez	t2, 1b;		\
	\
	li	t3, '\r';	\
4:;	\
	lw      d1,8(d0);	\
	andi    d1,d1,0xf;	\
	sltiu   d1,d1,13;	\
	beqz    d1,4b;		\
	sw      t3,4(d0);	\
	li	t3, '\n';	\
4:;	\
	lw      d1,8(d0);	\
	andi    d1,d1,0xf;	\
	sltiu   d1,d1,13;	\
	beqz    d1,4b;		\
	sw      t3,4(d0)

#endif

#if CONFIG_PLAT_U4600
#define PRINT_SYSCALL_TIMES	\
	SAVE_ALL_INT		\
	\
	li	t2, 60;		\
1:;	\
	dsrlv	a0, AT, t2;	\
	andi	a0, 0xf;	\
	sub	a0, 10;		\
	bgez	a0, 2f;		\
	add	a0, '0'+10;	\
	b 3f;			\
2:;	\
	add	a0, 'a';	\
3:;	\
	jal	_Z11putc_serialc;\
	sub	t2, 4;		\
	bgez	t2, 1b;		\
	\
	li	a0, '\n';	\
	jal	_Z11putc_serialc;\
	\
	RESTORE_ALL
#endif

#else
#define	START_SYSCALL_COUNTER
#define STOP_SYSCALL_COUNTER
#define PRINT_SYSCALL_TIMES
#endif

	.set noat
	.set noreorder
BEGIN_PROC(__mips64_interrupt)
	mfc0    k1, CP0_CAUSE
	lui	k0, %hi(exception_handlers)
	andi    k1, k1, 0x7c
	dsll    k1, k1, 1
	add	k0, k0, k1
	ld      k0, %lo(exception_handlers)(k0)
	jr      k0
	nop
END_PROC(__mips64_interrupt)

	.set reorder
BEGIN_PROC(_mips64_l4syscall)
	START_SYSCALL_COUNTER
	/* Trashed registers:
	 *   t4, t5, t6, t7
	 * Calling Registers:
	 *   v0 : syscall number
	 *   a0, a1, a2, a3 : arguments 1 - 4
	 *   t0, t1, t2, t3 : arguments 5 - 8
	 * Returned Registers:
	 *   v0	    : returned 1
	 *   a0..a3 : returned 2 - 5
	 *   t0..t1 : returned 6 - 7
	 */
	.set noat
	bgez	v0, _mips64_syscall_exception	/* Not an L4 system call? */
	mfc0	t7, CP0_STATUS		/* get STATUS register */
	lui	t5, %hi(K_STACK_BOTTOM)	/* Load kernel stack base address */
	move	t4, sp			/* Old stack in t4 */

	srl	t6, t7, 5		/* clear IE, EXL, ERL, KSU */
	sll	t6, t6, 5
	mtc0	t6, CP0_STATUS		/* Enter kernel mode */
	andi	t6, t7, ST_KSU		/* Isolate KSU bits */

	.set noreorder
	beq	t6, zero, 9f		/* Branch if from KERNEL mode */
	dmfc0	t6, CP0_EPC		/* Branch delay */
	.set reorder

	ld	sp, %lo(K_STACK_BOTTOM)(t5)	/* Load saved stack */

9:
	daddu	t6, t6, 4		/* Calc New EPC */
	dsubu	sp, sp, PT_SIZE		/* New stack pointer */
	sd	ra, PT_RA(sp)		/* Save RA */
	sd	t7, PT_STATUS(sp)	/* Save status */
	sd	t4, PT_SP(sp)		/* Save stack */
	sd	t6, PT_EPC(sp)		/* Save EPC */

	/* XXX decode must be in this order ! */
	DECODE_SYSCALL_IPC					/* 3 in, 1 out */
	DECODE_SYSCALL(thread_switch, SYSCALL_thread_switch)	/* 1 in, 0 out */
	DECODE_SYSCALL(thread_control, SYSCALL_thread_control)	/* 5 in, 1 out */
	DECODE_SYSCALL(exchange_registers, SYSCALL_exchange_registers)	/* 8 in, 7 out */
	DECODE_SYSCALL(schedule, SYSCALL_schedule)		/* 5 in, 2 out */
	DECODE_SYSCALL(unmap, SYSCALL_unmap)			/* 1 in, 0 out */
	DECODE_SYSCALL(space_control, SYSCALL_space_control)	/* 5 in, 2 out */
	SKIPDECODE_SYSCALL(processor_control,SYSCALL_processor_control)
	DECODE_SYSCALL(memory_control,SYSCALL_memory_control)   /* 3 in, 1 out */
	DECODE_SYSCALL(clock,SYSCALL_system_clock)		/* 0 in, 1 out */
	nop

1:
	li	AT, L4_TRAP_KDEBUG
	dla	v0, 2f
	break
	.data
2:	.string "Unknown SYSCALL"
	.previous
	.set at
END_PROC(_mips64_l4syscall)

BEGIN_PROC(_mips64_l4sysipc_return)
	dli	t4, -4096		    /* tcb mask */
	and	t5, sp, t4		    /* t5 = current tcb */
	ld	t5, OFS_TCB_UTCB(t5)	    /* t5 = current utcb */
	ld	s8, PT_S8(sp)
	ld	mr0, 128(t5)
	ld	mr1, 136(t5)
	ld	mr2, 144(t5)
	ld	mr3, 152(t5)
	ld	mr4, 160(t5)
	ld	mr5, 168(t5)
	ld	mr6, 176(t5)
	ld	mr7, 184(t5)
	ld	mr8, 192(t5)

_mips64_l4syscall_return:
	mfc0	t6, CP0_STATUS
	ld	t7, PT_SP(sp)		/* load stack */
	ori	t6, t6, ST_EXL		/* set Exception Level */
	mtc0	t6, CP0_STATUS		/* to disable interrupts, we now can set EPC */
	ld	t4, PT_STATUS(sp)	/* load status */
	ld	t5, PT_EPC(sp)		/* load epc */
	ld	ra, PT_RA(sp)		/* load ra */

	STOP_SYSCALL_COUNTER
	PRINT_SYSCALL_TIMES

	dmtc0	t5, CP0_EPC		/* restore EPC */ 
	li	t3, CONFIG_MIPS64_STATUS_MASK
	dsrl	t5, sp, 12
	move	sp, t7			/* restore stack */
	and	t6, t3, t6		/* compute new status register */
	nor	t3, zero, t3
	and	t4, t3, t4
	or	t7, t6, t4		/*            " "              */
	mtc0	t7, CP0_STATUS		/* new status value */
	dsll	t5, t5, 12		/* Get TCB pointer */
	ld	k0, OFS_TCB_MYSELF_LOCAL(t5)	/* Load UTCB into k0 */
nop
	eret

.set at
END_PROC(_mips64_l4sysipc_return)

	.set reorder
BEGIN_PROC(_mips64_syscall_exception)
	SAVE_ALL_INT

	move	a0, sp
	jal	syscall_exception

	j	_mips64_restore_user
END_PROC(_mips64_syscall_exception)
