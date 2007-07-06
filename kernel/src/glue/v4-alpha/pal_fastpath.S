/*********************************************************************
 *                
 * Copyright (C) 2004,  National ICT Australia (NICTA)
 *                
 * File path:     glue/v4-alpha/pal_fastpath.S
 * Description:   PAL mode IPC fastpath
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
 * $Id: pal_fastpath.S,v 1.3 2004/06/04 06:32:02 cvansch Exp $
 *                
 ********************************************************************/

#include <tcb_layout.h>	
#include <asmsyms.h>
#include <config.h>
#include INC_ARCH(asm.h)
#include INC_ARCH(pal.h)
#include INC_GLUE(config.h)
#include INC_GLUE(ipcregs.h)

#define ALIGN_FETCH_BLOCK(x) .align 4

/* The rest of L4 hardcode these pcb offset... evil.. */
#define OSF_PCB__KSP  0
#define OSF_PCB__USP  8
#define OSF_PCB__PTBR 16
#define OSF_PCB__CPC  24 // PCC
#define OSF_PCB__ASN  28
#define OSF_PCB__UNQ  32
#define OSF_PCB__FEN  40	
#define OSF_PCB__RSV  48

#define OSF_PCB__ASN__S		32
#define OSF_PCB__ASN__M		0xFF
#define OSF_PCB__FEN__S         0
#define OSF_PCB__FEN__M         0x1
#define OSF_PCB__PME__S         62
#define OSF_PCB__PME__M         0x1

#define OSF_P_MISC__PS__M       0xF

#define OSF_FRM__PS             0x0000
#define OSF_FRM__PC             0x0008
#define OSF_FRM__GP             0x0010

#define OSF_FRM__A0             0x0018
#define OSF_FRM__R16            0x0018

#define OSF_FRM__A1             0x0020
#define OSF_FRM__R17            0x0020

#define OSF_FRM__A2             0x0028
#define OSF_FRM__R18            0x0028

#define OSF_FRM__SIZE           0x0030
	
	
/* other defines used by this file */

/* '264 internal registers and offsets */
#define EV6__PS 	 	0x9
#define EV6__PAL_BASE		0x10
#define	EV6__CC			0xc0 
#define EV6__PROCESS_CONTEXT	0x5f     

#define EV6__PPCE__PPCE__S 	0x1        
#define EV6__FPE__FPE__S	0x2  
#define EV6__ASN__ASN__S	0x27 
	
#define EV6__DTB_ASN0		0x25    
#define EV6__DTB_ASN0__ASN__S	0x38    
#define EV6__DTB_ASN1		0xa5    

/* EV6 PAL temps */
#define PT__VPTB	0x0
#define PT__PTBR	0x8
#define PT__PCBB	0x10
#define PT__KSP		0x18
#define PT__USP		0x20
#define PT__KGP		0x28
/* ... */
#define PT__ENT_SYS	0x50
#define PT__STACK_PC	0x60
#define PT__IMPURE	0x88

/* PAL shadow registers */
#define p4     $4
#define p5     $5
#define p6     $6
#define p7     $7
#define p20    $20
#define p21    $21
#define p_temp $21
#define p22    $22
#define p_misc $22
#define p23    $23

#define r0 $0   /* v0 */
#define r16 $16 /* a0 */
#define r31 $31 /* zero */
#define r29 $29 /* gp */
#define r30 $30 /* sp */

#define asn_shift  (EV6__DTB_ASN0__ASN__S - EV6__ASN__ASN__S)
#define fpe_shift  (EV6__FPE__FPE__S - OSF_PCB__FEN__S)
#define ppce_shift (OSF_PCB__PME__S - EV6__PPCE__PPCE__S)
#define page_offset_size_bits 13

	
/* Pistachio IPC fastpath defines */
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
#define current_global	$0
#define dest_partner	$25
#define current_utcb	$26 /* This is relatively free :) */

		
#define T(a)

#define	tmp0		$19
#define	tmp1		$19	
#define	tmp2		$29 //20
#define	tmp3		$28 //21
#define	tmp4		$27 //22
#define	tmp5		$19
#define	tmp6		NOTUSED
#define	tmp7		NOTUSED
#define	tmp8		NOTUSED		
#define	tmp9		dest_partner //was $23
#define	tmp10		$24		
#define	tmp11		NOTUSED
#define	tmp12		$29 //20
#define	tmp13		$24
#define	tmp14		$29 //20
#define	tmp15		NOTUSED
#define	tmp16		$27 //22
#define	tmp17		$19
#define	tmp18		$29 //20
#define	tmp19		$28 //21
		

		
.global _start_pal_code
.global _end_pal_code
.global _start_pal_stub
.global _end_pal_stub
/*.section .pal_code*/
.text
pal_stub:	
_start_pal_stub:
	ldiq p4, 0x22001
	nop
	nop
	nop

	
	/* Swap context code from sdk */
	//rpcc $12

	hw_jmp (p4)
	br $31, .-4
_end_pal_stub:	

.align 4

_start_pal_code:
BEGIN_PROC(pal_ipc_fast_path)
	/* 1) Interrupts are off because we are in PAL.
	*  2) We should switch from PAL/user to PAL/kernel.
	*     This path assumes it is NOT called from kernel (easy enough to fix).
	*/

	/* Set up an exception frame -- returnable via RETSYS if needed */

        hw_stq/p r30, PT__USP(p_temp)          		// save user SP
	hw_stq/p p23, PT__STACK_PC(p_temp)              // store away nextpc
	and     p_misc, OSF_P_MISC__PS__M, p20         // save original ps
	br	p4, p_fastpath_k_cm
	/* Aligned */

p_fastpath_k_cm:
#define p_fastpath_k_offset (p_fastpath_k_done - p_fastpath_k_cm)	
	addq	p4, (p_fastpath_k_offset+1), p4
	hw_mtpr	$31, (EV6__PS << 8 | 0x10)          // (4,0L) switch to kern
	bsr	$31, .+4		            // push prediction stack
	hw_ret/stall (p4)			    // pop prediction stack
	.align 4
p_fastpath_k_done:

	/* We are now in PAL/kernel mode */
	hw_ldq/p r30, PT__KSP(p_temp)          		// get kernel SP
	bic     p_misc, OSF_P_MISC__PS__M, p_misc       // cm=0, ipl=0
	lda     r30, (0-OSF_FRM__SIZE)(r30)             // allocate stack space
 	stq     p20, OSF_FRM__PS(r30)                   // save original ps

	stq     r29, OSF_FRM__GP(r30)                   // save gp
	hw_ldq/p p20, PT__STACK_PC(p_temp)              // 1.39 get nextpc back
	stq     p20, OSF_FRM__PC(r30)                   // 1.39 save nextpc
	nop

	/* Check zero to_tid */
	beq	to_tid, __pal_goto_slowpath
	/*
	 * Check conditions: infinite receive timeout, < IPC_MAX_REGS sent, 
	 * no typed registers
	 */	
T(0)	zapnot	to_tid, 0x30, to_tcb			/* E0 */	/* Get threadno bits T(0) */	
T(0)	lda	tmp0, 0xe04				/* E0 or E1 */	
						
T(0)	sll	tmp0, (64 - 12), tmp0			/* E0 */							
T(3)	and	mr0, 0x3f, tmp3				/* E0 or E1 */	/* we support < IPC_MAX_REGS T(3) */

T(6)	lda	current, ASM_KTCB_MASK			/* E0 or E1 */	/* get current T(6) */	
T(0)	or	tmp0, to_tcb, to_tcb			/* E0 or E1 */							

T(0)	sra	to_tcb, (32 - ASM_KTCB_BITS), to_tcb	/* E0 */ 	/* END T(0) */	
T(6)	and	current, $30, current			/* E0 or E1 */	/* END T(6) */


T(7)	ldq	dest_partner, OFS_TCB_SPACE(to_tcb)	/* E0 or E1 */	/* doing irq thread go to slow path */
	beq	dest_partner, __pal_goto_slowpath
	
T(7)	ldq	dest_partner, OFS_TCB_PARTNER(to_tcb)	/* E0 or E1 */	/* Check partner T(7) <- T(0) */
T(8)	ldq	current_global, OFS_TCB_MYSELF_GLOBAL(current)	/* E0 or E1 */	/* Get current_global T(8) <- T(6) */
	

T(1)	zapnot	timeout, 0x3, tmp1			/* E0 */	/* infinite timeout T(1) */			
T(2)	ldiq	tmp2, (0x3f << 6) | IPC_MR0_PROPAGATED	/* E0 or E1 */	/* num_typed is bits 6 - 11 (6 bits) in mr0 T(2) */
	
T(10)	cmpeq	current_global, dest_partner, tmp10	/* E0 or E1 */  /* partner check T(10) <- (T(8), T(7)) */
T(9)	addq	dest_partner, 1, tmp9			/* E0 or E1 */	/* is_anythread == -1 T(9) <- T(7) */	

T(11)	cmovne	tmp10, 0, tmp9				/* E0 or E1 */  /* tmp9 == 0 or tmp10 != 0 -> fast_path T(11) <- (T(9), T(10)) */
	nop								/* Hopefully this will be squashed by the cmove */
	
T(13)	ldq	tmp13, OFS_TCB_SEND_HEAD(current)	/* E0 or E1 */	/* Require send_head to be empty T(13) <- T(6) */	
T(1)	bne	tmp1, __pal_goto_slowpath		/* E1 */	/* From timeout END T(1) */

T(4)	ldq	tmp4, OFS_TCB_MYSELF_GLOBAL(to_tcb)	/* E0 or E1 */	/* Check global ID T(4) <- T(0) */		
T(5)	ldl	tmp5, OFS_TCB_THREAD_STATE(to_tcb)	/* E0 or E1 */	/* Check partner is waiting T(5) <- T(0) */

T(2)	and	tmp2, mr0, tmp2				/* E0 or E1 */	
T(11)	bne	tmp9, __pal_goto_slowpath		/* E1 */	/* END T(9) END T(10) END T(11) */
	
T(3)	cmple	tmp3, IPC_MAX_REGS, tmp3		/* E0 or E1 */	
T(3)	beq	tmp3, __pal_goto_slowpath		/* E1 */	/* From IPC_MAX_REGS END T(3) */

T(4)	cmpeq	tmp4, to_tid, tmp4			/* E0 or E1 */
T(4)	beq	tmp4, __pal_goto_slowpath		/* E1 */	/* from global ID check. END T(4) */
			
T(5)	addq	tmp5, 1, tmp5				/* E0 or E1 */
T(2)	bne	tmp2, __pal_goto_slowpath		/* E1 */	/* From num_typed END T(2)*/

T(12)	addq	from_tid, 1, tmp12			/* E0 or E1 */	/* Check that receive phase blocks T(12) */
T(5)	bne	tmp5, __pal_goto_slowpath		/* E1 */	/* From is_waiting check END T(5) */
	/* Works to here */
	nop						/* This isn't such a bad place for this nop --- probably 50% chance here */
T(12)	bne	tmp12, check_other_tcb			/* E1 */	/* END T(12) */	

T(14)	ldiq	tmp14, THREAD_STATE_WAITING		/* E0 or E1 */	/* Set thread state to waiting T(14) */	
T(13)	bne	tmp13, __pal_goto_slowpath		/* E1 */	/* END T(13) */
	
	/* FALLTHRU */
	
continue_ipc:
	/* This is the point of no return --- after this we cannot go
	to the slow path */ 
	
	/* At this point, we have set up the sending thread's TCB
	state.  We now setup the stack so that when we are next
	switched to we do the right thing (set state to running and
	return partner) --- this only happens in the generic send
	case.  */

	/* Works to here */

T(17)	ldq	$16, OFS_TCB_ARCH(to_tcb)		/* E0 or E1 */	/* Gives the PA of the PCB T(17) END T(17) */	

#if 0 /* Evil shit */
T(16)	lda	tmp16, goto_finish			/* E0 or E1 */	/* turns into a ldq ... grumble  T(16) */
#endif
	
/* This block of code if fucked */

        lda tmp17, -0x400($31)
        br tmp16, next_foo
next_foo:
	lda tmp16, (goto_finish - next_foo)(tmp16)
	bic	tmp16, 0x1, tmp16
T(16)	subq	$30, ALPHA_SWITCH_STACK_SIZE, $30	/* E0 or E1 */	/* Save RA */
        sll tmp17, 32, tmp17
T(14)	stl	tmp14, OFS_TCB_THREAD_STATE(current)	/* E0 or E1 */	/* END T(14) */

	addq tmp16, tmp17, tmp16
	
T(15)	stq	from_tid, OFS_TCB_PARTNER(current)	/* E0 or E1 */	/* Set partner T(15) END T(15) */
T(16)	stq	tmp16, SWITCH_STACK_RA($30)		/* E0 or E1 */	/* END T(16) */
	/* t0, t8..t11, a0 trashed, v0 contains old ctx */
T(17)	lda	tmp17, THREAD_STATE_RUNNABLE		/* E0 or E1 */	/*
									 * Make thread runnable */
		/* ALIGNED.. start pal stuff here */

	/* How is this different from current?? */
	hw_ldl/p p6, OSF_PCB__ASN(r16)		// get new cpc/asn
	hw_ldq/p tmp16, PT__PCBB(p_temp)		// get old PCBB
	hw_ldq/p p7, PT__USP(p_temp)		// get usp
	and	p6, OSF_PCB__ASN__M, p6		// clean ASN

	
	hw_stq/p r30, OSF_PCB__KSP(tmp16)		// store ksp
	hw_stq/p p7, OSF_PCB__USP(tmp16)		// store usp
	sll	p6, EV6__DTB_ASN0__ASN__S, p6	// ASN into mbox spot
	nop
	
	hw_ldq/p p5, OSF_PCB__FEN(r16)		// get new fen/pme
	hw_stq/p r16, PT__PCBB(p_temp)		// set new PCBB
	nop
	nop	
//
// Now write ASN to DTB_ASNx.
//
// Current state:
//	p5	fen/pme quadword
//	p6	cpc/asn quadword
//
//
// There must be a scoreboard bit -> register dependency chain to prevent
// hw_mtpr DTB_ASx from issuing while ANY of scoreboard bits <7:4> are set.
//
	hw_mfpr	p7, (EV6__PAL_BASE << 8 | 0xF0)	// (4-7,0L)
	xor	p7, p7, p7			// zap p7
	bis	p7, p6, p6			// force register dependency
	NOP					// force fetch block

	.align 4
	hw_mtpr	p6, (EV6__DTB_ASN0 << 8 | 0x10)		// (4,0L)
	hw_mtpr	p6, (EV6__DTB_ASN1 << 8 | 0x80)		// (7,1L)
	sll	p5, fpe_shift, p4		// get FEN into position
T(18)	lda	tmp18, ASM_KTCB_SIZE - ALPHA_CONTEXT_SIZE /* E0 or E1 */	/* Clean up stack T(18) */	


//
// Create Ibox Process Context IPR, filling in ASN, FPE, PPCE
//
// Current state:
//	p5	fen/pme quadword
//	p6	asn in DTB_ASNx position
//


	hw_ldq/p p7, OSF_PCB__PTBR(r16)		// get new PTBR
	srl	p6, (asn_shift), p6		// ASN back into ibox position
	and	p4, (1<<EV6__FPE__FPE__S), p4	// clean it
	bis	p6, p4, p6			// or in FEN enable


	srl	p5, ppce_shift, p4		// get pme into position
	and	p4, (1<<EV6__PPCE__PPCE__S), p4	// clean it
	bis	p6, p4, p6			// or in ppce bit
	hw_mtpr	p6, (EV6__PROCESS_CONTEXT << 8 | 0x10)	// (4,0L) write it

#if 0
	.align 4
	bis	r0, r0, r0			// force retire
	nop
	nop
	hw_mtpr p6, (EV6__PROCESS_CONTEXT << 8 | 0x10)
#endif
	//rpcc $12
	//.align 4


		/* May need alignment */
//
// Write new PTBR, get new USP and SP, save USP.
//
	
	hw_ldq/p p5, OSF_PCB__USP(r16)		// get new USP
	//hw_ldq/p r30, OSF_PCB__KSP(r16)		// get new KSP

	sll	p7, page_offset_size_bits, p7	// convert PFN
	hw_stq/p p7, PT__PTBR(p_temp)		// store new PTBR
	hw_stq/p p5, PT__USP(p_temp)		// save USP


	/* Swap context done */
T(17)	stl	tmp17, OFS_TCB_THREAD_STATE(to_tcb)	/* E0 or E1 */	/* END T(17) */
	/* ldiq is two instructions! */
T(19)	ldiq	tmp19, ~(0xe << 12)			/* E0 or E1 */	/* Clear receive flags in mr0 T(19) */
	
T(18)	addq	to_tcb, tmp18, $30			/* E0 or E1 */	/* END T(18) */
T(19)	and	mr0, tmp19, mr0				/* E0 or E1 */	/* END T(19) */

//	nop

      /* We made this look like a syscall, so now we return to user
       * using the RETSYS call path
       */
	//rpcc $13
	//.align 4

	call_pal	PAL_retsys			/* E1 */


	.align	4
	/* Check whether from_tid is polling */
check_other_tcb:	
	beq	from_tid, __pal_goto_slowpath
	/* works up to here */
	
	TID_TO_TCB(from_tid, from_tcb, tmp1)

T(14)	ldiq	tmp14, THREAD_STATE_WAITING		/* E0 or E1 */	/* Set thread state to waiting T(14) */	
	
	/* Check global ID */
	/* works to here */
	ldq	tmp1, OFS_TCB_MYSELF_GLOBAL(from_tcb)
	cmpeq	tmp1, from_tid, tmp1
	beq	tmp1, __pal_goto_slowpath
	
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
	bne	tmp2, __pal_goto_slowpath	/* If the other thread is polling us, goto the slowpath */
	
	/* partner == current->local_id */
	ldq	tmp2, OFS_TCB_MYSELF_LOCAL(current)
	cmpeq	tmp1, tmp2, tmp2
	beq	tmp2, continue_ipc

	br	__pal_goto_slowpath

	/* goto_finish (__ipc_finish) must be executed in non-PAL/kernel mode. */
goto_finish:
	bsr	$31, __pal_ipc_finish
END_PROC(pal_ipc_fast_path)

BEGIN_PROC(__pal_ipc_finish)
	
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
	nop
	nop
	nop
END_PROC(__pal_ipc_finish)

#if 0
BEGIN_PROC(__pal_irq_handler)
	/* Unmask irq */
	/* Evil.. assuming pci for now */
	/* cached_irq_mask |= (1UL << (irq - BASE_PCI_IRQ));*/
	//ldq	tmp0, 0(cached_irq_mask)

	/* $19 == 0xfffffd01a0000200 == AS_KSEG_START + 0x101a0000200UL
		== TSUNAMI_IRQ_MASK */
	lda	$19, -0x2ff($31)
	lda	$29, 0xa($31)
	sll	$29, 28, $29
	lda	$27, 0x2($31)
	sll	$27, 8, $27
	addq	$27, $29, $29
	addq	$19, $29, $19


	ldq	$28, 0($19)
			
	srl	to_tcb, 32, $29
	/* 24 is BASE_PCI_IRQ */
	subl	$29, 24, $29
	lda	$24, 1
	sll	$24, $29, $24
	bis	$28, $24, $28

	
	stq	$28, 0($19)

	
/*                 0xfffffc0000000000	
/*	AS_KSEG_START + 0x101a0000200UL*/
/*                   fffffd01a0000200 */
	/* */
	
        lda tmp17, -0x400($31)
        br tmp16, next_irq_foo
next_irq_foo:
	lda tmp16, (goto_irq_finish - next_irq_foo)(tmp16)
	bic	tmp16, 0x1, tmp16
T(16)	subq	$30, ALPHA_SWITCH_STACK_SIZE, $30	/* E0 or E1 */	/* Save RA */
        sll tmp17, 32, tmp17
T(14)	stl	tmp14, OFS_TCB_THREAD_STATE(current)	/* E0 or E1 */	/* END T(14) */

	addq tmp16, tmp17, tmp16
	
T(15)	stq	from_tid, OFS_TCB_PARTNER(current)	/* E0 or E1 */	/* Set partner T(15) END T(15) */
T(16)	stq	tmp16, SWITCH_STACK_RA($30)		/* E0 or E1 */	/* END T(16) */

	/* Setup return stack */
//foo:	br	$31, foo
	/* Switch to next runnable */

	hw_ldq/p p23, PT__ENT_SYS(p_temp)               // get entry point
	hw_ldq/p r29, PT__KGP(p_temp)                   // get kgp

	mov	$31, $16
	lda	$0, 0x1($31) // XXX we should call __ipc_goto_slowpath directly!
	 
	hw_ret  (p23)				             // to os
	
goto_irq_finish:
	bsr	$31, __pal_ipc_finish
END_PROC(__pal_irq_handler)
#endif
			
BEGIN_PROC(__pal_goto_slowpath)
	/* handles getting out of PAL can calling IPC slow path */
	/* 1) currently in PAL/kernel mode: switch to kernel (non-PAL) mode,
	 *  branch to __ipc_goto_slowpath code (do this at non pal time)
	 * We need to make sure we save the right state to get back 
	 * (make it look like callsys).
	 */

	/* Same code CALLSYS does.. we should have done the other stuff earlier.. */
 
//foo:	br	$31, foo

	hw_ldq/p p23, PT__ENT_SYS(p_temp)               // get entry point
	hw_ldq/p r29, PT__KGP(p_temp)                   // get kgp

	mov	$31, $0		// XXX we should call __ipc_goto_slowpath directly!
	 
	hw_ret  (p23)				             // to os
END_PROC(__pal_goto_slowpath)	
		
_end_pal_code:

