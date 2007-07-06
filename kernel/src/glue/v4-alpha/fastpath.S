/*********************************************************************
 *                
 * Copyright (C) 2002,  University of New South Wales
 *                
 * File path:     glue/v4-alpha/fastpath.S
 * Description:   IPC fastpath
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
 * $Id: fastpath.S,v 1.4 2004/04/06 07:33:23 benno Exp $
 *                
 ********************************************************************/

/*
 * The fast path is:	
 *   + both send and receive. 
 *   + All IDs are global.
 *   + No typed message registers 
 *   + Up to and including IPC_MAX_REGS untyped message registers.
 *   + Not propagating.
 *   + A receive is involved, the target thread must not be polling.  If the 
 *   target threadid is anythread, the send queue must be empty.
 *   + The receive timeout must be infinite. (may change)
 */

#include <tcb_layout.h>	
#include <asmsyms.h>
#include <config.h>
#include INC_ARCH(asm.h)
#include INC_ARCH(pal.h)
#include INC_GLUE(config.h)
#include INC_GLUE(ipcregs.h)
			
#if defined(CONFIG_PREEMPT_ASIDS)
#error ASID check in context switch code not done
#endif
	
#define IPC_MAX_REGS	8

/* Pistachio defines */
#define THREAD_STATE_RUNNABLE 2
#define THREAD_STATE_WAITING -1	
#define IPC_MR0_PROPAGATED (1 << 12)	

#if VALID_THREADNO_BITS != 16
#error I expect VALID_THREADNO_BITS to be 16!
#endif	
/* 
 * Convenience functions 
 */

/* Clobber lower 12 bits of stack to get TCB */
#define GET_CURRENT(reg)	lda	reg, ASM_KTCB_MASK ;  and	reg, $30, reg

/* */
#define TID_TO_TCB(tid, tcb, tmp)						\
	zapnot	tid, 0x30, tcb	;			/* Get threadno bits */	\
	lda	tmp, 0xe04	;						\
	sll	tmp, (64 - 12), tmp ;						\
	or	tmp, tcb, tcb	;						\
	sra	tcb, (32 - ASM_KTCB_BITS), tcb 

/* API defines */					
#define mr0		R_MR0
#define mr1		R_MR1
#define mr2		R_MR2
#define mr3		R_MR3
#define mr4		R_MR4
#define mr5		R_MR5
#define mr6		R_MR6
#define mr7		R_MR7
#define mr8		R_MR8

#define to_tid		R_TO_TID
#define from_tid	R_FROM_TID
#define timeout		R_TIMEOUT
	
/* Locals */	
#define current		$1
#define to_tcb		$2
#define from_tcb	$3
#define current_global	$4
#define dest_partner	$5
#define current_utcb	$6

		
#define T(a)

#define	tmp0		$19
#define	tmp1		$19	
#define	tmp2		$20
#define	tmp3		$21
#define	tmp4		$22
#define	tmp5		$19
#define	tmp6		NOTUSED
#define	tmp7		NOTUSED
#define	tmp8		NOTUSED		
#define	tmp9		$23
#define	tmp10		$24		
#define	tmp11		NOTUSED
#define	tmp12		$20
#define	tmp13		$24
#define	tmp14		$20
#define	tmp15		NOTUSED
#define	tmp16		$22
#define	tmp17		$19
#define	tmp18		$20
#define	tmp19		$21
		
/* Layout is:	
T(n)	inst	arg, arg, arg	
	
	where:	
	n is the thread number (used to keep track of what is happening --- purely for my convenience)	
	IBOX is the pipeline the instruction can execute within 

Note:	
	21164 can do 0-cycle latency for branches in parallel with logical or compare instructions.  This
	means that	
		cmpeq	a, b, c	;  beq c, label 
	
	is perfectly valid and will execute in parallel. 
*/
		
BEGIN_PROC(__ent_sys)
	DISABLE_INT($3, $4)
	nop
	bne	$0, __handle_non_ipc

#if defined(CONFIG_ALPHA_PAL_IPC_FASTPATH)
	br	$31, __ipc_goto_slowpath
#endif
#if ! defined(CONFIG_ALPHA_FASTPATH)
	br	$31, __ipc_goto_slowpath
#else		
	.align	4	
send_path:	
	/*
	 * Check conditions: infinite receive timeout, < IPC_MAX_REGS sent, no typed registers
	 */	

T(0)	zapnot	to_tid, 0x30, to_tcb			/* E0 */	/* Get threadno bits T(0) */	
T(0)	lda	tmp0, 0xe04				/* E0 or E1 */	
						
T(0)	sll	tmp0, (64 - 12), tmp0			/* E0 */							
T(3)	and	mr0, 0x3f, tmp3				/* E0 or E1 */	/* we support < IPC_MAX_REGS T(3) */

T(6)	lda	current, ASM_KTCB_MASK			/* E0 or E1 */	/* get current T(6) */	
T(0)	or	tmp0, to_tcb, to_tcb			/* E0 or E1 */							

T(0)	sra	to_tcb, (32 - ASM_KTCB_BITS), to_tcb	/* E0 */ 	/* END T(0) */	
T(6)	and	current, $30, current			/* E0 or E1 */	/* END T(6) */

T(7)	ldq	dest_partner, OFS_TCB_PARTNER(to_tcb)	/* E0 or E1 */	/* Check partner T(7) <- T(0) */
T(8)	ldq	current_global, OFS_TCB_MYSELF_GLOBAL(current)	/* E0 or E1 */	/* Get current_global T(8) <- T(6) */

T(1)	zapnot	timeout, 0x3, tmp1			/* E0 */	/* infinite timeout T(1) */			
T(2)	ldiq	tmp2, (0x3f << 6) | IPC_MR0_PROPAGATED	/* E0 or E1 */	/* num_typed is bits 6 - 11 (6 bits) in mr0 T(2) */
	
T(10)	cmpeq	current_global, dest_partner, tmp10	/* E0 or E1 */  /* partner check T(10) <- (T(8), T(7)) */
T(9)	addq	dest_partner, 1, tmp9			/* E0 or E1 */	/* is_anythread == -1 T(9) <- T(7) */	

T(11)	cmovne	tmp10, 0, tmp9				/* E0 or E1 */  /* tmp9 == 0 or tmp10 != 0 -> fast_path T(11) <- (T(9), T(10)) */
	nop								/* Hopefully this will be squashed by the cmove */
	
T(13)	ldq	tmp13, OFS_TCB_SEND_HEAD(current)	/* E0 or E1 */	/* Require send_head to be empty T(13) <- T(6) */	
T(1)	bne	tmp1, __ipc_goto_slowpath		/* E1 */	/* From timeout END T(1) */

T(4)	ldq	tmp4, OFS_TCB_MYSELF_GLOBAL(to_tcb)	/* E0 or E1 */	/* Check global ID T(4) <- T(0) */		
T(5)	ldl	tmp5, OFS_TCB_THREAD_STATE(to_tcb)	/* E0 or E1 */	/* Check partner is waiting T(5) <- T(0) */

T(2)	and	tmp2, mr0, tmp2				/* E0 or E1 */	
T(11)	bne	tmp9, __ipc_goto_slowpath		/* E1 */	/* END T(9) END T(10) END T(11) */
	
T(3)	cmple	tmp3, IPC_MAX_REGS, tmp3		/* E0 or E1 */	
T(3)	beq	tmp3, __ipc_goto_slowpath		/* E1 */	/* From IPC_MAX_REGS END T(3) */

T(4)	cmpeq	tmp4, to_tid, tmp4			/* E0 or E1 */
T(4)	beq	tmp4, __ipc_goto_slowpath		/* E1 */	/* from global ID check. END T(4) */
			
T(5)	addq	tmp5, 1, tmp5				/* E0 or E1 */
T(2)	bne	tmp2, __ipc_goto_slowpath		/* E1 */	/* From num_typed END T(2)*/

T(12)	addq	from_tid, 1, tmp12			/* E0 or E1 */	/* Check that receive phase blocks T(12) */
T(5)	bne	tmp5, __ipc_goto_slowpath		/* E1 */	/* From is_waiting check END T(5) */
	
	nop						/* This isn't such a bad place for this nop --- probably 50% chance here */
T(12)	bne	tmp12, check_other_tcb			/* E1 */	/* END T(12) */	

T(14)	ldiq	tmp14, THREAD_STATE_WAITING		/* E0 or E1 */	/* Set thread state to waiting T(14) */	
T(13)	bne	tmp13, __ipc_goto_slowpath		/* E1 */	/* END T(13) */

	/* FALLTHRU */
	
continue_ipc:
	/* This is the point of no return --- after this we cannot go to the slow path */	
	/* At this point, we have set up the sending thread's TCB state.  We now setup the 
	 * stack so that when we are next switched to we do the right thing (set state to running 
	 * and return partner) --- this only happens in the generic send case.
	 */	

T(17)	ldq	$16, OFS_TCB_ARCH(to_tcb)		/* E0 or E1 */	/* Gives the PA of the PCB T(17) END T(17) */	
T(16)	lda	tmp16, goto_finish			/* E0 or E1 */	/* turns into a ldq ... grumble  T(16) */
	
T(14)	stl	tmp14, OFS_TCB_THREAD_STATE(current)	/* E0 or E1 */	/* END T(14) */
T(16)	subq	$30, ALPHA_SWITCH_STACK_SIZE, $30	/* E0 or E1 */	/* Save RA */

T(15)	stq	from_tid, OFS_TCB_PARTNER(current)	/* E0 or E1 */	/* Set partner T(15) END T(15) */
T(16)	stq	tmp16, SWITCH_STACK_RA($30)		/* E0 or E1 */	/* END T(16) */
	
	/* t0, t8..t11, a0 trashed, v0 contains old ctx */
T(17)	ldiq	tmp17, THREAD_STATE_RUNNABLE		/* E0 or E1 */	/* Make thread runnable */
	call_pal	PAL_swpctx			/* E1 */
	
	/* OK, we are now in to_tcb's context.  Clean up the stack and return */
T(18)	mov	current_global, $0			/* E0 or E1 */	/* return from TID */
T(17)	stl	tmp17, OFS_TCB_THREAD_STATE(to_tcb)	/* E0 or E1 */	/* END T(17) */

T(18)	ldiq	tmp18, ASM_KTCB_SIZE - ALPHA_CONTEXT_SIZE /* E0 or E1 */	/* Clean up stack T(18) */	
T(19)	ldiq	tmp19, ~(0xe << 12)			/* E0 or E1 */	/* Clear receive flags in mr0 T(19) */
	
T(18)	addq	to_tcb, tmp18, $30			/* E0 or E1 */	/* END T(18) */
T(19)	and	mr0, tmp19, mr0				/* E0 or E1 */	/* END T(19) */

	nop
	call_pal	PAL_retsys			/* E1 */
		
	.align	6	
	/* Check whether from_tid is polling */
check_other_tcb:	
	beq	from_tid, __ipc_goto_slowpath
	
	TID_TO_TCB(from_tid, from_tcb, tmp1)

T(14)	ldiq	tmp14, THREAD_STATE_WAITING		/* E0 or E1 */	/* Set thread state to waiting T(14) */	
	
	/* Check global ID */
	ldq	tmp1, OFS_TCB_MYSELF_GLOBAL(from_tcb)
	cmpeq	tmp1, from_tid, tmp1
	beq	tmp1, __ipc_goto_slowpath
	
	/* 
	 * Check if the thread is polling us --- if so, go to slow path
	 */

	/* is_polling():	thread_state == 5 << 1*/
	ldl	tmp1, OFS_TCB_THREAD_STATE(from_tcb)
	cmpeq	tmp1, 5 << 1, tmp1
	beq	tmp1, continue_ipc		/* from_tcb isn't polling */

	/* partner == current->global_id */
	ldq	tmp1, OFS_TCB_PARTNER(from_tcb)
	cmpeq	tmp1, current_global, tmp2
	bne	tmp2, __ipc_goto_slowpath	/* If the other thread is polling us, goto the slowpath */
	
	/* partner == current->local_id */
	ldq	tmp2, OFS_TCB_MYSELF_LOCAL(current)
	cmpeq	tmp1, tmp2, tmp2
	beq	tmp2, continue_ipc

	br	__ipc_goto_slowpath

goto_finish:
	bsr	$31, __ipc_finish
#endif /* not defined(CONFIG_ALPHA_FASTPATH) */		
END_PROC(__ent_sys)

/* This code handles the dest thread waiting on a local thread */
#if 0
	/* IS_WAITING_LOCAL:	(tcb->get_space() == current->get_space()) && (tcb->get_partner() == current->myself_local) || (tcb->get_partner().is_anylocalthread()) */
	/* check tcb->get_space() == current->get_space() */
	ldq	tmp1, OFS_TCB_SPACE(to_tcb)
	ldq	tmp2, OFS_TCB_SPACE(current)
	cmpeq	tmp1, tmp2, tmp1
	beq	tmp1, __ipc_goto_slowpath

	/* myself_local == utcb */
	cmpeq	current_utcb, dest_partner, tmp1
	bne	tmp1, do_send_phase

	/* is_anylocalthread() version == 0, id == -1 */
	dliq	tmp1, -1 << L4_LOCAL_ID_ZERO_BITS
	cmpeq	tmp1, dest_partner, tmp1
	beq	tmp1, __ipc_goto_slowpath
#endif

	
/* 
 * SLOW PATH:	
 * This is the return path for the thread.  We got here via the generic path, so we 
 * need to load stuff out of MRs and set up the state that the receiving thread expects. 
 */

BEGIN_PROC(__ipc_finish)
#if 0
	subq	$30, 8*2, $30
	stq	$16, 0($30)
	stq	$17, 8($30)
	
	ldiq	$16, 2
	lda	$17, 1f
	call_pal	PAL_gentrap
	br	$31, 2f
	
1:	.asciz	"Slowpath (Finish)"
	.align	4

2:		

	ldq	$16, 0($30)
	ldq	$17, 8($30)
	addq	$30, 8*2, $30
#endif
	
	GET_CURRENT(current)
	
	/* first the state */
	ldiq	tmp1, THREAD_STATE_RUNNABLE
	stl	tmp1, OFS_TCB_THREAD_STATE(current)
	ldq	$0, OFS_TCB_PARTNER(current)

	/* Now the message contents */
	/* OPT:	only load as many MRs as there have been sent */
	ldq	current_utcb, OFS_TCB_UTCB(current)
	ldq	mr0, UTCB_MR_OFFSET +  0(current_utcb)
	ldq	mr1, UTCB_MR_OFFSET +  8(current_utcb)
	ldq	mr2, UTCB_MR_OFFSET + 16(current_utcb)
	ldq	mr3, UTCB_MR_OFFSET + 24(current_utcb)
	ldq	mr4, UTCB_MR_OFFSET + 32(current_utcb)
	ldq	mr5, UTCB_MR_OFFSET + 40(current_utcb)
	ldq	mr6, UTCB_MR_OFFSET + 48(current_utcb)
	ldq	mr7, UTCB_MR_OFFSET + 56(current_utcb)
	ldq	mr8, UTCB_MR_OFFSET + 64(current_utcb)							
	
	call_pal	PAL_retsys	
END_PROC(__ipc_finish)

/*
 * SLOW PATH:	
 * We need to store all the MRs into the UTCB and call the regular IPC function 
 *
 */

BEGIN_PROC(__ipc_goto_slowpath)
#if 0
	subq	$30, 8*2, $30
	stq	$16, 0($30)
	stq	$17, 8($30)
	
	ldiq	$16, 2
	lda	$17, 1f
	call_pal	PAL_gentrap
	br	$31, 2f
	
1:	.asciz	"Slowpath"
	.align	4

2:		

	ldq	$16, 0($30)
	ldq	$17, 8($30)
	addq	$30, 8*2, $30
#endif
		
	GET_CURRENT(current)
	
	/* Now the message contents */
	/* OPT:	only store as many MRs as there have been sent */
	ldq	current_utcb, OFS_TCB_UTCB(current)
	stq	mr0, UTCB_MR_OFFSET +  0(current_utcb)
	stq	mr1, UTCB_MR_OFFSET +  8(current_utcb)
	stq	mr2, UTCB_MR_OFFSET + 16(current_utcb)
	stq	mr3, UTCB_MR_OFFSET + 24(current_utcb)
	stq	mr4, UTCB_MR_OFFSET + 32(current_utcb)
	stq	mr5, UTCB_MR_OFFSET + 40(current_utcb)
	stq	mr6, UTCB_MR_OFFSET + 48(current_utcb)
	stq	mr7, UTCB_MR_OFFSET + 56(current_utcb)
	stq	mr8, UTCB_MR_OFFSET + 64(current_utcb)							

	lda	$27, sys_ipc
	lda	$26, __ipc_finish

	jsr	$31, ($27)


END_PROC(__ipc_goto_slowpath)

BEGIN_PROC(__ipc_error)
#if 1
	subq	$30, 8*2, $30
	stq	$16, 0($30)
	stq	$17, 8($30)
	
	ldiq	$16, 2
	lda	$17, 1f
	call_pal	PAL_gentrap
	br	$31, 2f
	
1:	.asciz	"IPC Error"
	.align	4

2:		

	ldq	$16, 0($30)
	ldq	$17, 8($30)
	addq	$30, 8*2, $30
#endif	
	halt
END_PROC(__ipc_error)
