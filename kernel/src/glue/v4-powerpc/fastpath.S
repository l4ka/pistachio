/****************************************************************************
 *
 * Copyright (C) 2002-2003, Karlsruhe University
 *
 * File path:	glue/v4-powerpc/fastpath.S
 * Description:	IPC fastpath implementation
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
 * $Id: fastpath.S,v 1.17 2004/06/04 17:28:37 joshua Exp $
 *
 ***************************************************************************/
#include INC_GLUE(abi.h)

/**
 * tid_to_tcb: Converts a thread ID into a TCB offset within the KTCB area.
 * @param tcb Name of register to hold the output TCB offset.
 * @param tid Name of register holding the TID.
 */
.macro	tid_to_tcb tcb, tid
	rlwinm \tcb, \tid, 32 - (L4_GLOBAL_VERSION_BITS - KTCB_BITS), (L4_GLOBAL_VERSION_BITS - KTCB_BITS), 31 - KTCB_BITS
.endm

/**
 * stack_to_tcb: Converts the stack pointer into a TCB pointer.
 * @param tcb Name of register to hold the output TCB pointer.
 */
.macro	stack_to_tcb tcb
	extlwi	\tcb, %r1, 32-KTCB_BITS, 0
.endm

#define MR0	IPC_ABI_MR0
#define MR1	IPC_ABI_MR1
#define MR2	IPC_ABI_MR2
#define MR3	IPC_ABI_MR3
#define MR4	IPC_ABI_MR4
#define MR5	IPC_ABI_MR5
#define MR6	IPC_ABI_MR6
#define MR7	IPC_ABI_MR7
#define MR8	IPC_ABI_MR8
#define MR9	IPC_ABI_MR9

#define TO_TID		IPC_ABI_TO_TID
#define FROM_TID	IPC_ABI_FROM_TID
#define TIMEOUTS	IPC_ABI_TIMEOUTS

#define LOCAL_ID	%r2
#define KTCB_AREA	%r31
#define SUM		%r30
#define TO_TCB		%r29
#define CURRENT_TCB	%r28
#define MY_TID		%r27
#define PARTNER_TID	%r26
#define USER_SRR0	%r24
#define USER_SRR1	%r23
#define UTCB		%r22
#define TO_UTCB		SUM

#define TMP2		%r21
#define TMP1		%r20
#define TMP0		%r19

#define CMP_DEFAULT	cr0
#define CMP_SLOW_PATH	cr1
#define CMP_TMP0	cr2

#define CR_DEFAULT	4*CMP_DEFAULT+eq
#define CR_SLOW_PATH	4*CMP_SLOW_PATH+eq
#define CR_TMP0		4*CMP_TMP0+eq

#define MAX_UNTYPED	IPC_ABI_MR_TOT


	/********************************************************************
	 *
	 *  Fast Path Implementation
	 *
	 ********************************************************************/
KIP_SC_ENTRY(".sc_ipc", ipc)
	globalize_any_tid TO_TID
	globalize_any_tid FROM_TID
	syscall_entry

	 .globl _ipc_start
_ipc_start:
	/* Locate the current tcb. */
	stack_to_tcb CURRENT_TCB

#if defined(CONFIG_IPC_FASTPATH)
#warning "PowerPC fastpath is experimental!"

	/********************************************************************
	 *
	 *  Fast Path Tests
	 *
	 ********************************************************************/

	/* Look for a nil from-tid. */
	cmplwi	CMP_SLOW_PATH, FROM_TID, 0	/* Is from-tid equal to 0?	*/

#if 1	/* (Adds about 2 cycles.) */
	cmplwi	CMP_TMP0, TO_TID, 0		/* Is to-tid equal to 0? */
	cror	CR_DEFAULT, CR_SLOW_PATH, CR_TMP0
	beq	_sc_ipc_slowpath	/* Jump to slow path if from-tid or to-tid is 0. */
#endif

	/* Grab the KTCB area. */
#if (KTCB_AREA_START & 0x0000ffff)
# error "lower 16-bits of KTCB_AREA_START must be zero"
#endif
	lis	KTCB_AREA, KTCB_AREA_START @ha

	/* Locate the destination tcb.
	 */
	tid_to_tcb TO_TCB, TO_TID
	add	TO_TCB, TO_TCB, KTCB_AREA	/* Compute the tcb location. */

	/* Validate the to-tcb.  For this to work, we must have at 
	 * least one interrupt thread, to prevent the nil thread
	 * from matching the global ID of the first TCB
	 * If succeeds, we know that the to-tid is a valid global ID,
	 * and the to-tcb is a valid tcb, and the to-tid is
	 * not the nil-thread.
	 */
	lwz	TMP0, OFS_TCB_MYSELF_GLOBAL (TO_TCB)	/* Load to-tid. */
	xor	SUM, TMP0, TO_TID	/* Compare to tcb's global id to to_tid. */

	/* What is our tid? */
	lwz	MY_TID, OFS_TCB_MYSELF_GLOBAL (CURRENT_TCB)

	/* Look at everyone's resources. */
	lwz	TMP0, OFS_TCB_RESOURCE_BITS (CURRENT_TCB)
	lwz	TMP1, OFS_TCB_RESOURCE_BITS (TO_TCB)
	or	SUM, TMP0, SUM
	or	SUM, TMP1, SUM

	/* Inspect to-tcb's receive state. */
	lwz	TMP0, OFS_TCB_THREAD_STATE (TO_TCB)
	lwz	PARTNER_TID, OFS_TCB_PARTNER (TO_TCB)
	orc	SUM, SUM, TMP0	/* Ensure that the to-tcb is waiting forever.  This negates the thread state. */

	/* Is the to-tcb waiting for us?  Use condition
	 * register boolean algebra to figure this out.
	 */
	nand.	TMP2, PARTNER_TID, PARTNER_TID   /* Is to-tcb's partner the any-thread?  Sets CMP_DEFAULT. */
	cmplw	CMP_TMP0, PARTNER_TID, MY_TID	/* Is to-tcb's partner equal to us? */
	cror	CR_TMP0, CR_DEFAULT, CR_TMP0	/* Is to-tcb's partner any-thread or us? */
	crorc	CR_SLOW_PATH, CR_SLOW_PATH, CR_TMP0  /* Negate the result, and OR to the slow path CR. */

	/* Inspect the message tag for typed-items and message flags.
	 * Bits 15 through 6 should be zero for fast path.
	 */
#if 0
	extrwi.	TMP0, MR0, 10, 16
	crorc	CR_SLOW_PATH, CR_SLOW_PATH, CR_DEFAULT	/* Negate the result, and OR to the slow path CR. */
#else
	extrwi	TMP0, MR0, 10, 16
	or	SUM, TMP0, SUM
#endif

	/* We must have no receive time-out.
	 */
#if 0
	andi.	TMP1, TIMEOUTS, 0xffff
	crorc	CR_SLOW_PATH, CR_SLOW_PATH, CR_DEFAULT	/* Negate the result, and OR to the slow path CR. */
#else
	andi.	TMP1, TIMEOUTS, 0xffff
	or	SUM, TMP1, SUM
#endif

	/* The from-tcb must not be polling on us.
	 */
	lwz	TMP0, OFS_TCB_SEND_HEAD (CURRENT_TCB)
#if 1
	or	SUM, SUM, TMP0
#else
	cmplw	CMP_DEFAULT, TO_TID, FROM_TID	/* IPC call (to == from)? */
	cmplwi	CMP_TMP0, TMP0, 0		/* NULL send-head? */
	cror	CR_TMP0, CR_TMP0, CR_DEFAULT	/* IPC call OR NULL send-head? */
	crorc	CR_SLOW_PATH, CR_SLOW_PATH, CR_TMP0	/* Negate result, and OR to the slow path CR. */
#endif

	/* Prepare for message transfer.
	 */
	andi.	TMP0, MR0, 0x3f    /* Extract number of untyped message items. */
	cmpwi	CMP_DEFAULT, TMP0, MAX_UNTYPED   /* Are all untyped messages in the register file? */

	/* Consolidate test results, and branch to slow path
	 * if necessary.
	 */
	cmplwi	CMP_TMP0, SUM, 0	/* Did we pass all the fast-path tests? */
	crorc	CR_SLOW_PATH, CR_SLOW_PATH, CR_TMP0  /* Negate the result, and OR to the slow path CR. */
	bc	12 /* branch if true */, CR_SLOW_PATH, _sc_ipc_slowpath	

	/********************************************************************
	 *
	 *  Message transfer.  We may modify the target UTCB.
	 *
	 ********************************************************************/
	blt	2f	/* If everything is in the register file, skip copy. */

	/* Copy message registers between UTCBs.  Remember that the
	 * untyped message count excludes MR[0].
	 */
	lwz	UTCB, OFS_TCB_UTCB (CURRENT_TCB)  /* Locate the source UTCB. */
	lwz	TO_UTCB, OFS_TCB_UTCB (TO_TCB)    /* Locate the target UTCB. */

	addi	TMP0, TMP0, -(MAX_UNTYPED-1)	  /* Calculate loop count. */
	mtctr	TMP0				  /* Set loop count. */

	addi	UTCB, UTCB, MR_OFFSET(MAX_UNTYPED)-4		/* Start of first copy MR, -4. */
	addi	TO_UTCB, TO_UTCB, MR_OFFSET(MAX_UNTYPED)-4	/* Start of first copy MR, -4. */

	/* Copy loop. */
1:	lwzu	TMP0, 4 (UTCB)		/* Load MR, and update MR pointer. */
	stwu	TMP0, 4 (TO_UTCB)	/* Store MR, and update MR pointer. */
	bdnz	1b			/* Loop. */
	
2:

	/********************************************************************
	 *
	 *  Prepare to wait for message.  We now start to modify TCBs.
	 *
	 ********************************************************************/

	/* Set current TCB state to waiting for-ever. */
#if defined(TSTATE_WAITING_FOREVER) && (TSTATE_WAITING_FOREVER != 0xffffffff)
# error "Expected thread_state_t::waiting_forever to be -1"
#endif
	addi	TMP0, 0, -1	/* Create a -1 value, thread_state_t::waiting_forever */
	stw	TMP0, OFS_TCB_THREAD_STATE (CURRENT_TCB)
	stw	FROM_TID, OFS_TCB_PARTNER (CURRENT_TCB) /* Set partner to from-tid. */
	
	/* Create a thread-switch activation point.
	 */
	grab_sym TMP1, _sc_fast_ipc_return 
	stwu	TMP1, (OFS_TSWITCH_FRAME_IP-TSWITCH_FRAME_SIZE) (%r1) /* Store (with update) return address. */

	stw	%r1, OFS_TCB_STACK (CURRENT_TCB)	/* Set stack to current stack location. */

	/********************************************************************
	 *
	 *  Activate the to TCB.
	 *
	 ********************************************************************/
#define MY_ASID		%r26
#define ASID		%r25

	/* Preload the ASIDs. */
	lwz	ASID, OFS_TCB_PDIR_CACHE (TO_TCB)	/* ASID of to-tcb */
	lwz	MY_ASID, OFS_TCB_PDIR_CACHE (CURRENT_TCB)	/* ASID of current-tcb */

	/* Set the return value for the IPC call: use a local TID. 
	 * Do this before restoring the user's local ID.
	 */
	mr	FROM_TID, LOCAL_ID

	/* Save the tcb pointer in the appropriate sprg. */
	mtsprg	SPRG_CURRENT_TCB, TO_TCB

	/* Set the TCB state to running. */
	li	TMP0, TSTATE_RUNNING		/* thread_state_t::running */
	stw	TMP0, OFS_TCB_THREAD_STATE (TO_TCB)

	/* Are the ASIDs equal? */
	cmplw	CMP_DEFAULT, ASID, MY_ASID

	/* Restore the user's context. */
	lwz	USER_SRR0, TCB_STATE_OFFSET(SRR0) (TO_TCB)
        lwz     USER_SRR1, TCB_STATE_OFFSET(SRR1) (TO_TCB)
	lwz	%r31, TCB_STATE_OFFSET(R31)  (TO_TCB)
	lwz	%r30, TCB_STATE_OFFSET(R30)  (TO_TCB)
	lwz	%r1,  TCB_STATE_OFFSET(R1)   (TO_TCB)
	lwz	%r2,  TCB_STATE_OFFSET(R2)   (TO_TCB)

	/* Skip forward if ASIDs not equal. */
	bne	3f

	/* Return to user. */
	mtsrr0	USER_SRR0
	mtsrr1	USER_SRR1
	rfi


	/********************************************************************
	 *
	 *  Address space switch.
	 *
	 ********************************************************************/
3:
	/* Set the return value for the IPC call: use a global TID. */
	mr	FROM_TID, MY_TID

	/* Switch the address space.  We use the rfi for
	 * context synchronization, so no touching memory
	 * after this point!!!
	 */
#if defined(CONFIG_PPC_SEGMENT_LOOP)
	li	TMP1, 12	/* Init the loop count. */
	mtctr	TMP1		/* Load the loop count. */
	li	TMP1, 0		/* Init the segment register index. */
99:	mtsrin	ASID, TMP1	/* Set the segment register. */
	addi	ASID, ASID, 1	/* Increment the ASID. */
	extlwi	TMP1, ASID, 4, 28	/* Extract the segment register index. */
	bdnz	99b		/* Loop. */
#else
#define SET_SR( sr, asid )	mtsr sr , asid ; addi asid, asid, 1
	SET_SR( 0, ASID )
	SET_SR( 1, ASID )
	SET_SR( 2, ASID )
	SET_SR( 3, ASID )
	SET_SR( 4, ASID )
	SET_SR( 5, ASID )
	SET_SR( 6, ASID )
	SET_SR( 7, ASID )
	SET_SR( 8, ASID )
	SET_SR( 9, ASID )
	SET_SR( 10, ASID )
	mtsr 11, ASID
#endif

	/* Do not touch memory!!  The CPU context may be
	 * in a funny state.
	 */
	mtsrr0	USER_SRR0
	mtsrr1	USER_SRR1
	rfi


#endif	/* CONFIG_IPC_FASTPATH */


/**************************************************************/
/******* Fall-through if !defined(CONFIG_IPC_FASTPATH) ********/
/**************************************************************/


	/********************************************************************
	 *
	 *  IPC Slow Path
	 *
	 ********************************************************************/
_sc_ipc_slowpath:
	/* Get the UTCB location, relative to the kernel. */
	lwz	UTCB, OFS_TCB_UTCB (CURRENT_TCB)

	/* Spill message registers to the UTCB. */
	stw	MR0, MR_OFFSET(0) (UTCB)
	stw	MR1, MR_OFFSET(1) (UTCB)
	stw	MR2, MR_OFFSET(2) (UTCB)
	stw	MR3, MR_OFFSET(3) (UTCB)
	stw	MR4, MR_OFFSET(4) (UTCB)
	stw	MR5, MR_OFFSET(5) (UTCB)
	stw	MR6, MR_OFFSET(6) (UTCB)
	stw	MR7, MR_OFFSET(7) (UTCB)
	stw	MR8, MR_OFFSET(8) (UTCB)
	stw	MR9, MR_OFFSET(9) (UTCB)

	/* Setup parameters. */
#if defined(CONFIG_SYSV_ABI)
	stack_alloc  16
	stack_store2 %r3, TO_TID, 8
	stack_store2 %r4, FROM_TID, 12
	stack_store2 %r5, TIMEOUTS, 16
#else
	mr	%r3, TO_TID
	mr	%r4, FROM_TID
	mr	%r5, TIMEOUTS
#endif

	/* Set the link return to the IPC slow path state restore. */
	grab_sym TMP0, _sc_ipc_return
	mtlr	TMP0

	/* Fall-through to kernel C code (setup by the linker). */


	/********************************************************************
	 *
	 *  IPC Return
	 *
	 ********************************************************************/
	.section .text
	.align	2
	.globl	_sc_fast_ipc_return
	.globl	_sc_ipc_return

	/* When a slow-path IPC thread switches to someone that used a fast
	 * path, this is the entry point.  It has to clean-up.
	 */
_sc_fast_ipc_return:
	stack_to_tcb CURRENT_TCB	/* Locate the current TCB. */
	lwz	FROM_TID, OFS_TCB_PARTNER (CURRENT_TCB)	/* Load the partner TID from the TCB. */

	/* Set the TCB state to running. */
	li	TMP0, TSTATE_RUNNING		/* thread_state_t::running */
	stw	TMP0, OFS_TCB_THREAD_STATE (CURRENT_TCB)

	/* Slow-path return point.
	 */
_sc_ipc_return:
	/* Locate the current tcb. */
	stack_to_tcb CURRENT_TCB
	/* Get the UTCB location, relative to the kernel. */
	lwz	UTCB, OFS_TCB_UTCB (CURRENT_TCB)

	/* Load the message registers from the UTCB. */
	lwz	MR0, MR_OFFSET(0) (UTCB)
	lwz	MR1, MR_OFFSET(1) (UTCB)
	lwz	MR2, MR_OFFSET(2) (UTCB)
	lwz	MR3, MR_OFFSET(3) (UTCB)
	lwz	MR4, MR_OFFSET(4) (UTCB)
	lwz	MR5, MR_OFFSET(5) (UTCB)
	lwz	MR6, MR_OFFSET(6) (UTCB)
	lwz	MR7, MR_OFFSET(7) (UTCB)
	lwz	MR8, MR_OFFSET(8) (UTCB)
	lwz	MR9, MR_OFFSET(9) (UTCB)

	/* Restore the user's context. */
	lwz	TMP0, TCB_STATE_OFFSET(SRR0) (CURRENT_TCB)
	lwz	TMP1, TCB_STATE_OFFSET(SRR1) (CURRENT_TCB)
	lwz	%r31, TCB_STATE_OFFSET(R31)  (CURRENT_TCB)
	lwz	%r30, TCB_STATE_OFFSET(R30)  (CURRENT_TCB)
	lwz	%r1,  TCB_STATE_OFFSET(R1)   (CURRENT_TCB)
	lwz	%r2,  TCB_STATE_OFFSET(R2)   (CURRENT_TCB)

	mtsrr0	TMP0
	mtsrr1	TMP1
	rfi

