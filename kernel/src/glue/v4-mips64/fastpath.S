/*********************************************************************
 *                
 * Copyright (C) 2002-2004,  University of New South Wales
 *                
 * File path:     glue/v4-mips64/fastpath.S
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
 * $Id: fastpath.S,v 1.17 2004/12/01 23:53:19 cvansch Exp $
 *
 ********************************************************************/

#if CONFIG_IPC_FASTPATH

#include INC_ARCH(asm.h)
#include INC_ARCH(regdef.h)
#include INC_GLUE(context.h)
#include INC_GLUE(syscalls.h)
#include <asmsyms.h>
#include <tcb_layout.h>


#if 0
#define	START_SYSCALL_COUNTER	    \
	mfc0	k0, CP0_COUNT;	    \
	nop;			    \
	nop;			    \
	mtc0	k0, CP0_ERROREPC

#if CONFIG_PLAT_SB1
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
	dsub	AT, t2, t3
#else
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
#endif

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
#define CS1_BASE		0x1c800000
#define Z85230_BASE		(CS1_BASE | 0x30)
#define MPSC_BASE		Z85230_BASE
#define PHYS_TO_CKSEG1(n)	(CKSEG1 | (n))
#define PHYS_TO_K1(x)		PHYS_TO_CKSEG1(x)
#define U4600_PORT0		PHYS_TO_K1(MPSC_BASE+2)
#define zRR0_TXEMPTY		0004

/* print a single character to the serial port  (k1) */
#define U4600_PUTC					\
	/* wait for device to become ready */		\
1:	dla	k0, U4600_PORT0;			\
	lb	k0, 0(k0);				\
	andi	k0, zRR0_TXEMPTY;			\
	beqz	k0, 1b;					\
							\
	/* transmit by writing to the second byte */	\
	dla     k0, U4600_PORT0;			\
	sb      k1, 1(k0);				\
	sync


#define PRINT_SYSCALL_TIMES	\
	li	t2, 28;		\
1:;	\
	dsrlv	k1, AT, t2;	\
	andi	k1, 0xf;	\
	sub	k1, 10;		\
	bgez	k1, 2f;		\
	add	k1, '0'+10;	\
	b 3f;			\
2:;	\
	add	k1, 'a';	\
3:;	\
4:	dla	k0, U4600_PORT0;			\
	lb	k0, 0(k0);				\
	andi	k0, zRR0_TXEMPTY;			\
	beqz	k0, 4b;					\
							\
	/* transmit by writing to the second byte */	\
	dla     k0, U4600_PORT0;			\
	sb      k1, 1(k0);				\
	sync;			\
	sub	t2, 4;		\
	bgez	t2, 1b;		\
	\
	li	k1, '\n';	\
4:	dla	k0, U4600_PORT0;			\
	lb	k0, 0(k0);				\
	andi	k0, zRR0_TXEMPTY;			\
	beqz	k0, 4b;					\
							\
	/* transmit by writing to the second byte */	\
	dla     k0, U4600_PORT0;			\
	sb      k1, 1(k0);				\
	sync
#endif

#if CONFIG_PLAT_SB1
#define CKSEG1      0xffffffffa0000000
#define PHYS_TO_CKSEG1(n)     (CKSEG1 | (n))
#define DUART_PHYS		0x0010060000
#define DUART_PHYS_SIZE		0x100
#define DUART_STATUS            0x120
#define DUART_TX_HOLD		0x170
#define DUART_TX_RDY		(1<<2)

#define DUART_REG(chan,r)	(DUART_PHYS_SIZE*(chan) + (r))
#define DUART_REG_PHYS(chan,r)	(DUART_PHYS + DUART_REG(chan,r))

#define PRINT_SYSCALL_TIMES	\
	li	t2, 28;		\
1:;	\
	dsrlv	k1, AT, t2;	\
	andi	k1, 0xf;	\
	sub	k1, 10;		\
	bgez	k1, 2f;		\
	add	k1, '0'+10;	\
	b 3f;			\
2:;	\
	add	k1, 'a';	\
3:;	\
4:	dla	k0, PHYS_TO_CKSEG1(DUART_REG_PHYS(0,DUART_STATUS));			\
	ld	k0, 0(k0);				\
	andi	k0, DUART_TX_RDY;			\
	beqz	k0, 4b;					\
							\
	/* transmit by writing to the second byte */	\
	dla     k0, PHYS_TO_CKSEG1(DUART_REG_PHYS(0,DUART_TX_HOLD));			\
	sd      k1, 0(k0);				\
	sub	t2, 4;		\
	bgez	t2, 1b;		\
	\
	li	k1, '\r';	\
4:	dla	k0, PHYS_TO_CKSEG1(DUART_REG_PHYS(0,DUART_STATUS));			\
	ld	k0, 0(k0);				\
	andi	k0, DUART_TX_RDY;			\
	beqz	k0, 4b;					\
							\
	/* transmit by writing to the second byte */	\
	dla     k0, PHYS_TO_CKSEG1(DUART_REG_PHYS(0,DUART_TX_HOLD));			\
	sd      k1, 0(k0);	\
	li	k1, '\n';	\
4:	dla	k0, PHYS_TO_CKSEG1(DUART_REG_PHYS(0,DUART_STATUS));			\
	ld	k0, 0(k0);				\
	andi	k0, DUART_TX_RDY;			\
	beqz	k0, 4b;					\
							\
	/* transmit by writing to the second byte */	\
	dla     k0, PHYS_TO_CKSEG1(DUART_REG_PHYS(0,DUART_TX_HOLD));			\
	sd      k1, 0(k0)
#endif

#else
#define	START_SYSCALL_COUNTER
#define STOP_SYSCALL_COUNTER
#define PRINT_SYSCALL_TIMES
#endif

	.set noat
	.set noreorder
BEGIN_PROC(__mips64_interrupt_fp)
	START_SYSCALL_COUNTER
	mfc0    k1, CP0_CAUSE			/* get interrupt cause */
	li	k0, 8<<2
	andi    k1, k1, 0x7c
	bne	k0, k1, other_exception		/* If not SYSCALL, goto normal path */
	mfc0	k0, CP0_STATUS			/* get STATUS register */
	li	$1, SYSCALL_ipc
	bne	v0, $1, _goto_mips64_l4syscall	/* if not IPC goto normal syscall entry */
	lui     t5, %hi(K_STACK_BOTTOM)		/* Load kernel stack base address */

	j	_mips64_fastpath
	move	t7, k0

_goto_mips64_l4syscall:
	j _mips64_l4syscall
	nop
	
other_exception:
	dsll    k1, k1, 1
	lui	k0, %hi(exception_handlers)
	add	k0, k0, k1
	ld      k0, %lo(exception_handlers)(k0)
	jr      k0
	nop
END_PROC(__mips64_interrupt_fp)

/* Pistachio defines */

#define to_tid	    a0
#define from_tid    a1 
#define timeout	    a2
#define current	    a3
#define to_tcb	    t0
#define from_tcb    t1
#define to_state    t2
#define dest_partner	t3
#define current_global	v0

#define tmp0	    t4
#define tmp1	    t5
#define tmp2	    t6
#define tmp3	    t7
#define tmp4	    t8
#define tmp5	    t9
#define tmp6	    t3

#define mr0	    v1
#define mr1	    s0
#define mr2	    s1
#define mr3	    s2
#define mr4	    s3
#define mr5	    s4
#define mr6	    s5
#define mr7	    s6
#define mr8	    s7

	.set reorder
BEGIN_PROC(_mips64_fastpath)
	.set noat
	srl	t6, t7, 5		/* clear IE, EXL, ERL, KSU */
	move	t4, sp
	sll	t6, t6, 5
	mtc0	t6, CP0_STATUS		/* Enter kernel mode */

	ld	sp, %lo(K_STACK_BOTTOM)(t5)	/* Load saved stack */
	dmfc0   t6, CP0_EPC

	sd	ra, PT_RA-PT_SIZE(sp)	/* Save RA */
	sd	t7, PT_STATUS-PT_SIZE(sp) /* Save status */
	sd	t4, PT_SP-PT_SIZE(sp)	/* Save stack */

	daddu	t6, t6, 4		/* Calc New EPC */
	dsubu	sp, sp, PT_SIZE		/* New stack pointer */

	dli	tmp4, -4096		/* tcb mask */		/* CALC2 */
	sd	t6, PT_EPC(sp)		/* Save EPC */

/*** START FASTPATH ***/
send_path:
	.set	noreorder
	/* Look for a nil from-tid / to-tid. */			/* TEST0 */
	beqz	to_tid, _mips64_slowpath			/* TEST0 */
	and	current, sp, tmp4	/* get current tcb */	/* CALC2 */

	andi	tmp0, mr0, 0xffc0				/* TEST1 | Bits 15 through 6 should be zero for fast path in mr0 */

	dsrl    tmp5, to_tid, 32	/* calculate to_tcb */	/* CALC1 */
	li      to_tcb, 0x4					/* CALC1 */
	dsll    tmp5, 12					/* CALC1 */
	dsll    to_tcb, 60					/* CALC1 */
	/* Check that the receive timeout is infinite */	/* TEST3 | (lower 16 timeout bits == 0) */
	andi	tmp2, timeout, 0xffff				/* TEST3 */
	or      to_tcb, tmp5, to_tcb				/* CALC1 */
	/* We don't do typed words or propagation.*/		/* TEST1 */
	bnez	tmp0, _mips64_slowpath				/* TEST1 */

	/* Check if any resource bits are set */		/* TEST9 & TEST10 */
	ld	tmp3, OFS_TCB_RESOURCE_BITS(to_tcb)		/* TEST9 */
	ld	tmp0, OFS_TCB_RESOURCE_BITS(current)		/* TEST10 */

	bnez	tmp3, _mips64_slowpath				/* TEST9 */
	ld	tmp1, OFS_TCB_MYSELF_GLOBAL(to_tcb)		/* TEST4 */

	bnez	tmp0, _mips64_slowpath				/* TEST10 */
	ld	to_state, OFS_TCB_THREAD_STATE(to_tcb)		/* TEST5 */
        /*
	 * Check partner ID	CALC1, TEST4
	 */

	/* Check to_tcb->get_global_id == to_tid */		/* TEST4 */
	bne	tmp1, to_tid, _mips64_slowpath			/* TEST4 | May be an error, may be just a receive etc. */

	/*
	 * Check partner is waiting				   TEST5
	 */

	/* is_waiting */
#if ((TSTATE_WAITING_FOREVER != (0xffffffffffffffff)) && defined(__ASMSYMS_H__))  /* Don't let the preprocessor die due to no asmsyms */
# error "expecting thread_state_t::waiting_forever to be -1ull"
#endif
	dli	tmp3, -1					/* TEST5 & TEST6 & TEST7 *
	bnez	tmp2, _mips64_slowpath				/* TEST3 */
	ld	dest_partner, OFS_TCB_PARTNER(to_tcb)		/* TEST6 */

	bne	to_state, tmp3, _mips64_slowpath		/* TEST5 */
	ld	current_global, OFS_TCB_MYSELF_GLOBAL(current)	/* TEST6 */

	/* IS_WAITING_GLOBAL:   (tcb->get_partner() == current->get_global_id()) || tcb->get_partner().is_anythread() */
	/* is_anythread() */					/* TEST6 */

	beq	tmp3, dest_partner, 1f				/* TEST6 */
	lui	ra, %hi(ipc_finish)				/* RA CALC */

	/* tcb->get_partner() == current->get_global_id() */	/* TEST6 */
	bne	current_global, dest_partner, _mips64_slowpath	/* TEST6 */

#ifdef CONFIG_SMP
        /* check that both threads are on the same CPU */
1:	ld	tmp0, OFS_TCB_CPU(to_tcb)
	ld	tmp1, OFS_TCB_CPU(current)
	nop
	bne	tmp0, tmp1, _mips64_slowpath
	nop
#else
1:
#endif
	ld	tmp4, OFS_TCB_SPACE(to_tcb)			/* TEST11 - get: to_tcb->space */
	andi	tmp5, mr0, 0x3f					/* TEST2 */ 
	beqz	tmp4, _mips64_slowpath				/* TEST11 - Null space = interrupt thread */
	sub	tmp5, 8						/* TEST2 */

	/*
	 * Check that receive phase blocks			   TEST7
	 */
	beq	to_tid, from_tid, 2f				/* Call ? */
	daddiu	ra, %lo(ipc_finish)				/* RA CALC */

	ld	tmp6, OFS_TCB_SEND_HEAD(current)		/* TEST8 | Require send_head to be empty */
	nop
	bnez	tmp6, _mips64_slowpath				/* TEST8 */
	nop
	bne	tmp3, from_tid, check_other_tcb			/* TEST7 */
	nop
2:

// continue starts here
	sd	tmp3, OFS_TCB_THREAD_STATE(current)		/* STORE1 */

        /* FALLTHRU */
continue_ipc:
	/* This is the point of no return --- after this we cannot go to the slow path */

	sd	from_tid, OFS_TCB_PARTNER(current)
	blez	tmp5, switch_to					/* TEST2 */
	sd	s8, PT_S8(sp)

	ld	tmp0, OFS_TCB_UTCB(current)			/* tmp0 = current utcb */
	sll	tmp2, tmp5, 3					/* number to copy * 8 */
	ld	tmp1, OFS_TCB_UTCB(to_tcb)			/* tmp1 = to utcb */

	andi	tmp6, tmp5, 1
	daddu	tmp2, tmp2, tmp0

	beqz	tmp6, 10f
	ld	tmp3, 200(tmp0)
	daddiu	tmp0, 8
	daddiu	tmp1, 8
	beq	tmp0, tmp2, switch_to 
	sd	tmp3, 192(tmp1)
copy_loop:
	ld	tmp3, 200(tmp0)
10:
	ld	tmp6, 208(tmp0)
	daddiu	tmp1, 16
	daddiu	tmp0, 16
	sd	tmp3, 184(tmp1)
	bne	tmp0, tmp2, copy_loop
	sd	tmp6, 192(tmp1)
switch_to:

	/*
	 * Set thread state to waiting				   STORE1
	 */

/* mips switch_to */
	/* At this point, we have set up the sending thread's TCB state.  We now setup the
	 * stack so that when we are next switched to we do the right thing (set state to running
	 * and return partner) --- this only happens in the generic send case.
	 */
	dsubu	sp, sp,	MIPS64_SWITCH_STACK_SIZE
	lw	tmp1, SPACE_ASID_OFFSET(tmp4)	/* get: space->asid (assume no asid management) */

	sd	ra, 32(sp)			/* RA CALC */

	lui     tmp5, %hi(K_STACK_BOTTOM)	/* Load kernel stack base address */
	sd	sp, OFS_TCB_STACK(current)	/* Store current stack in old_stack */

	.set at
	dmtc0	tmp1, CP0_ENTRYHI	/* Set new ASID */
	daddiu	sp, to_tcb, 4096	/* STACK TOP CALC */
	dsll	tmp4, tmp4, 32
	sd	sp, %lo(K_STACK_BOTTOM)(tmp5)	/* Set current TCB */
	dli	tmp0, TSTATE_RUNNING

	dmtc0	tmp4, CP0_CONTEXT	/* Save current Page Table */

	/* Mark self as runnable */
	sd	tmp0, OFS_TCB_THREAD_STATE(to_tcb)

	/* Set return value to sender's global ID (already in v0)*/

	mfc0	t6, CP0_STATUS
	ld	t7, PT_SP-PT_SIZE(sp)		/* load stack */
	ori	t6, t6, ST_EXL		/* set Exception Level */
	ld	t0, OFS_TCB_MYSELF_LOCAL(to_tcb)    /* Load UTCB */

	/* Clean up mr0 (clear receive flags) */
	and	mr0, ~(0xe << 12)

	mtc0	t6, CP0_STATUS		/* to disable interrupts, we now can set EPC */
	ld	t4, PT_STATUS-PT_SIZE(sp)	/* load status */
	ld	t5, PT_EPC-PT_SIZE(sp)		/* load epc */
	ld	ra, PT_RA-PT_SIZE(sp)		/* load ra */

	.set	reorder

	STOP_SYSCALL_COUNTER
	PRINT_SYSCALL_TIMES

	dmtc0	t5, CP0_EPC		/* restore EPC */ 
	ld	s8, PT_S8-PT_SIZE(sp)		/* restore s8 */

	dli	t3, CONFIG_MIPS64_STATUS_MASK
	move	sp, t7			/* restore stack */
	and	t6, t3, t6		/* compute new status register */
	nor	t3, zero, t3
	and	t4, t3, t4
	or	t7, t6, t4		/*            " "              */
	mtc0	t7, CP0_STATUS		/* new status value */
	move	k0, t0			/* Load UTCB into k0 */
nop
	eret

	.set	reorder
ipc_finish:	/* Return Address */
	dli	tmp0, -4096			/* tcb mask */
	dli	tmp1, TSTATE_RUNNING
	and	current, sp, tmp0		/* t5 = current tcb */

	daddu	sp, current, 4096-PT_SIZE

        /* first the state */
	sd	tmp1, OFS_TCB_THREAD_STATE(current)

	ld	v0, OFS_TCB_PARTNER(current)

	j	_mips64_l4sysipc_return

check_other_tcb:
	.set	noreorder
	beqz	from_tid, _mips64_slowpath
        dsrl    tmp0, from_tid, 32                /* calculate from_tcb */
	li      from_tcb, 0x4
	dsll    tmp0, 12
	dsll    from_tcb, 60
	or      from_tcb, tmp0, from_tcb

	/* Check global ID */
	ld	tmp0, OFS_TCB_MYSELF_GLOBAL(from_tcb)
	ld	tmp1, OFS_TCB_THREAD_STATE(from_tcb)
	bne	tmp0, from_tid, _mips64_slowpath

	/*
	 * Check if the thread is polling us --- if so, go to slow path
	 */

	/* is_polling() */
	li	tmp2, TSTATE_POLLING
	bnel	tmp1, tmp2, continue_ipc	/* from_tcb isn't polling */
	sd	tmp3, OFS_TCB_THREAD_STATE(current)		/* STORE1 */ /* from continue ipc */
	
	/* partner == current->global_id */
	ld	tmp1, OFS_TCB_PARTNER(from_tcb)
	beq	tmp1, current_global, _mips64_slowpath	/* If the other thread is polling us, goto the slowpath */

	/* partner == current->local_id */
	ld	tmp2, OFS_TCB_MYSELF_LOCAL(current)
	bnel	tmp1, tmp2, continue_ipc
	sd	tmp3, OFS_TCB_THREAD_STATE(current)		/* STORE1 */ /* from continue ipc */

	j	_mips64_slowpath
	nop

END_PROC(_mips64_fastpath)

	.set reorder
BEGIN_PROC(_mips64_slowpath)
	lui	ra, %hi(_mips64_l4sysipc_return)
	ld	t5, OFS_TCB_UTCB(current)
	daddiu	ra, %lo(_mips64_l4sysipc_return)
	sd	s8, PT_S8(sp)
	sd	mr0, 128(t5)
	sd	mr1, 136(t5)
	sd	mr2, 144(t5)
	sd	mr3, 152(t5)
	sd	mr4, 160(t5)
	sd	mr5, 168(t5)
	sd	mr6, 176(t5)
	sd	mr7, 184(t5)
	sd	mr8, 192(t5)
	j	sys_ipc
END_PROC(_mips64_slowpath)

#endif /* CONFIG_IPC_FASTPATH */

