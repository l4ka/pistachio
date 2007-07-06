/****************************************************************************
 *
 * Copyright (C) 2002, Karlsruhe University
 *
 * File path:	glue/v4-powerpc/kip_sc.S
 * Description:	Defines the user space syscall entry points.  
 *		They are prefixes for the actual system calls.  The linker is
 *		responsible for aligning these entry points with their
 *		respective system calls.
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
 * $Id: kip_sc.S,v 1.30 2004/06/04 17:29:41 joshua Exp $
 *
 ***************************************************************************/

#include INC_ARCH(frame.h)
#include INC_GLUE(abi.h)

#include <tcb_layout.h>
#include <asmsyms.h>

#define SC_REG(x)	    (OFS_SYSCALL_##x + EABI_STACK_SIZE)
#define MR_OFFSET(n)	    (OFS_UTCB_MR + 4*(n))
#define USER_MR_OFFSET(n)   (MR_OFFSET(n) - OFS_UTCB_MR)
#define TCB_STATE_OFFSET(r) (TOTAL_TCB_SIZE-SYSCALL_FRAME_SIZE+OFS_SYSCALL_##r)

/**
 * syscall_entry: Switches from user-level to kernel-level for system
 *   calls, and spills relevant user state.
 */
.macro	syscall_entry
	/* Promote privilege level and switch to the kernel stack.	*/
	sc
	/* Save state. */
	mflr	%r27
	stw	%r29, SC_REG(R1)(%r1)	/* Preserve the user sp.	*/
	stw	%r30, SC_REG(R30)(%r1)	/* Preserve r30.		*/
	stw	%r31, SC_REG(R31)(%r1)	/* Preserve r31.		*/
	stw	%r28, SC_REG(SRR1)(%r1)	/* Preserve srr1.		*/
	stw	%r27, SC_REG(SRR0)(%r1)	/* Preserve return address.	*/
	stw	%r2,  SC_REG(R2)(%r1)	/* Preserve r2 (utcb pointer).	*/
.endm

.macro	stack_alloc size
	addi	%r1, %r1, -\size
.endm

/**
 * stack_store: Stores the contents of a register on the stack,
 *   and stores in the register the memory location of the
 *   value on the stack.
 */
.macro	stack_store reg, off
	stw	\reg, \off (%r1)
	addi	\reg, %r1, \off
.endm

/**
 * stack_store2: Stores the contents of a src register on the stack,
 *   and stores in a dst register the memory location of the value
 *   on the stack.
 */
.macro	stack_store2 dst, src, off
	stw	\src, \off (%r1)
	addi	\dst, %r1, \off
.endm

/**
 * grab_sym: Encodes a symbol in the instruction stream as
 *   two 16-bit immediates, and combines into a dst register.
 */
.macro	grab_sym dst, sym
	lis	\dst, \sym @ha
	la	\dst, \sym @l(\dst)
.endm

/**
 * stackify1: Moves a register to the stack, while recording the
 *   stack location in the register.  Allocates space on the stack.
 */
.macro	stackify1 reg1
#if defined(CONFIG_SYSV_ABI)
	stack_alloc	8
	stack_store	\reg1, 8
#endif
.endm

/**
 * stackify2: Moves two registers to the stack, while recording their
 *   stack locations in the registers.  Allocates space on the stack.
 */
.macro	stackify2 reg1, reg2
#if defined(CONFIG_SYSV_ABI)
	stack_alloc	8
	stack_store	\reg1, 8
	stack_store	\reg2, 12
#endif
.endm

/**
 * stackify3: Moves three registers to the stack, while recording their
 *   stack locations in the registers.  Allocates space on the stack.
 */
.macro	stackify3 reg1, reg2, reg3
#if defined(CONFIG_SYSV_ABI)
	stack_alloc	16
	stack_store	\reg1, 8
	stack_store	\reg2, 12
	stack_store	\reg3, 16
#endif
.endm

/**
 * stackify4: Moves four registers to the stack, while recording their
 *   stack locations in the registers.  Allocates space on the stack.
 */
.macro	stackify4 reg1, reg2, reg3, reg4
#if defined(CONFIG_SYSV_ABI)
	stack_alloc	16
	stack_store	\reg1, 8
	stack_store	\reg2, 12
	stack_store	\reg3, 16
	stack_store	\reg4, 20
#endif
.endm


#define KIP_SC_ENTRY(sect_name,label)	\
	.section sect_name ;		\
	.align	2 ;			\
	.globl	_sc_##label ;		\
	_sc_##label :


#define NILTHREAD	0
#define LOCAL_TID_MASK	0x3f


/**
 * globalize_tid: Detect whether a TID is a local TID, and convert to
 *   a global TID as necessary.  Can handle the nil-TID, but not the
 *   any-TID.
 * @param tid Name of register containing the TID.
 * @return The default condition register is set if the TID was a
 *   local TID.
 */
.macro	globalize_tid tid
	cmplwi	cr1, \tid, NILTHREAD		/* Compare TID to the nil TID. */
	andi.	%r11, \tid, LOCAL_TID_MASK	/* Is the TID a local TID?     */
	/* Negate NIL result and AND with local TID result. */
	crandc	4*cr0+eq, 4*cr0+eq, 4*cr1+eq
	bne	0f
	lwz	\tid, (OFS_UTCB_MY_GLOBAL_ID - OFS_UTCB_MR) (\tid)
0:
.endm

/**
 * globalize_any_tid: Detect whether a TID is a local TID, and convert to
 *   a global TID as necessary.  Can handle the nil-TID and the any-TID.
 * @param tid Name of register containing the TID.
 * @return The default condition register is set if the TID was a
 *   local TID.
 */
.macro	globalize_any_tid tid
	cmplwi	cr1, \tid, NILTHREAD		/* Compare TID to the nil TID. */
	andi.	%r11, \tid, LOCAL_TID_MASK	/* Is the TID a local TID?     */
	/* Negate NIL result and AND with local TID result. */
	crandc	4*cr1+eq, 4*cr0+eq, 4*cr1+eq
	/* OR a bunch of 1's with the TID, to see if it becomes the any TID. */
	/* Invert the result to see whether it equals 0. */
	li	%r11, LOCAL_TID_MASK & 0xffffffff
	nor.	%r11, %r11, \tid
	/* Negate the any thread result, and AND with the local TID result. */
	crandc	4*cr0+eq, 4*cr1+eq, 4*cr0+eq
	bne	0f
	lwz	\tid, (OFS_UTCB_MY_GLOBAL_ID - OFS_UTCB_MR) (\tid)
0:
.endm

.macro	spill_register_MRs_to_user_UTCB
	stw	IPC_ABI_MR0, USER_MR_OFFSET(0) (ABI_LOCAL_ID)
	stw	IPC_ABI_MR1, USER_MR_OFFSET(1) (ABI_LOCAL_ID)
	stw	IPC_ABI_MR2, USER_MR_OFFSET(2) (ABI_LOCAL_ID)
	stw	IPC_ABI_MR3, USER_MR_OFFSET(3) (ABI_LOCAL_ID)
	stw	IPC_ABI_MR4, USER_MR_OFFSET(4) (ABI_LOCAL_ID)
	stw	IPC_ABI_MR5, USER_MR_OFFSET(5) (ABI_LOCAL_ID)
	stw	IPC_ABI_MR6, USER_MR_OFFSET(6) (ABI_LOCAL_ID)
	stw	IPC_ABI_MR7, USER_MR_OFFSET(7) (ABI_LOCAL_ID)
	stw	IPC_ABI_MR8, USER_MR_OFFSET(8) (ABI_LOCAL_ID)
	stw	IPC_ABI_MR9, USER_MR_OFFSET(9) (ABI_LOCAL_ID)
.endm

/*****************************************************************************/

/* Define the IPC paths. */
#include "fastpath.S"

/*****************************************************************************/

KIP_SC_ENTRY(".sc_schedule", schedule)
	globalize_tid %r3
	syscall_entry
	stackify1 SCHEDULE_ABI_DEST

/*****************************************************************************/

KIP_SC_ENTRY(".sc_xchg_registers", xchg_registers)

	/* Convert the dest TID into a global TID.  But we must also
	 * tell the ExchangeRegisters system call whether the input TID
	 * was local, and thus we generate an extra parameter, is_local,
	 * for the ExchangeRegisters system call.
	 */
	globalize_tid EXREG_ABI_DEST
	mfcr	EXREG_ABI_IS_LOCAL    /* Copy the condition register into the is_local parameter. */
	extrwi	EXREG_ABI_IS_LOCAL, EXREG_ABI_IS_LOCAL, 1, 2    /* Isolate cr0[eq] as bit 0. */

	/* Does the control register mention the pager TID?
	 * If so, we convert it into a global TID.
	 */
	andi.	%r11, EXREG_ABI_CONTROL, 1 << EXREG_ABI_CONTROL_P    /* Check the p-bit of the control register. */
	beq	1f			/* If zero, skip the globalize. */
	globalize_tid EXREG_ABI_PAGER
1:

	syscall_entry
	stackify2 EXREG_ABI_DEST, EXREG_ABI_PAGER

/*****************************************************************************/

KIP_SC_ENTRY(".sc_unmap", unmap)

	/* Fixup the parameters for the C function calling ABI (we are still
	 * in user mode). */
	spill_register_MRs_to_user_UTCB
	mr	%r3, UNMAP_ABI_CONTROL

	/* Enter the kernel and handle the system call. */
	syscall_entry

/*****************************************************************************/

KIP_SC_ENTRY(".sc_memory_ctrl", memory_ctrl)

	/* Fixup the parameters for the C function calling ABI (we are still
	 * in user mode). */
	spill_register_MRs_to_user_UTCB
	mr	%r3, MEM_ABI_CONTROL
	mr	%r4, MEM_ABI_ATTR0
	mr	%r5, MEM_ABI_ATTR1
	mr	%r6, MEM_ABI_ATTR2
	mr	%r7, MEM_ABI_ATTR3

	/* Enter the kernel and handle the system call. */
	syscall_entry

/*****************************************************************************/

KIP_SC_ENTRY(".sc_processor_ctrl", processor_ctrl)
	syscall_entry

/*****************************************************************************/

KIP_SC_ENTRY(".sc_thread_ctrl", thread_ctrl)
	globalize_tid TCTRL_ABI_DEST
	globalize_tid TCTRL_ABI_SPACE
	globalize_tid TCTRL_ABI_SCHEDULER
	globalize_tid TCTRL_ABI_PAGER

	syscall_entry
	stackify4 TCTRL_ABI_DEST, TCTRL_ABI_SPACE, TCTRL_ABI_SCHEDULER, TCTRL_ABI_PAGER

/*****************************************************************************/

KIP_SC_ENTRY(".sc_space_ctrl", space_ctrl)
	globalize_tid SPACE_ABI_SPACE
	globalize_any_tid SPACE_ABI_REDIRECTOR

	syscall_entry
	stackify4 SPACE_ABI_SPACE, SPACE_ABI_KIP, SPACE_ABI_UTCB, SPACE_ABI_REDIRECTOR

/*****************************************************************************/

	.section ".sc_system_clock"
	.align 2
	.global	_sc_system_clock
_sc_system_clock:
	/*********************************************************************
	 * Important: we never entered the kernel!!
	 * We are still executing in user mode.
	 *********************************************************************
	 * Note: for the 750, the time base is clocked at 1/4 the bus clock.
	 */

	/* Allocate stack space for a further function call, and store 
	 * the return address.
	 */
	mflr	%r11
	stwu	%r1, -8(%r1)
	stw	%r11, 12(%r1)

	/* Extract the current clock value.  Loop as necessary.
	 */
1:
	mftbu	%r3			/* Get upper time base.	*/
	mftbl	%r4			/* Get lower time base.	*/
	mftbu	%r11			/* Get upper time base.	*/
	cmpl	%cr0, %r3, %r11		/* Did we witness an overflow?	*/
	bne	1b			/* Try again if overflow.	*/

	/* Load the bus frequency from the kernel interface page.
	 */
	lwz	%r10, (OFS_UTCB_PROCESSOR_NO - OFS_UTCB_MR) (%r2)	/* Get the CPU number. */
	grab_sym %r9, processor_descriptors
	mulli	%r11, %r10, PROCDESC_SIZE	/* Convert the CPU number into an procdesc offset. */
	add	%r9, %r9, %r11	/* Add the offset to the procdesc array. */
	lwz	%r7, OFS_PROCDESC_EXTERNAL_FREQ (%r9)	/* Load the bus KHz. */

	/* Convert from KHz to 1/4 MHz.  PowerPC ISA voodoo follows. */
	lis	%r8, 0x1062
	ori	%r5, %r8, 19923
	mulhwu	%r9, %r7, %r5
	srwi	%r6, %r9, 8	/* The lower word of the denominator. */
	li	%r5, 0		/* The upper word of the denominator is 0. */

	/* Perform the division. */
	bl	__udivdi3

	/* Restore state and return.  %r3 and %r4 already contain the results.
	 */
	lwz	%r11, 12(%r1)	/* Load the old LR value.	*/
	addi	%r1, %r1, 8	/* Restore the stack.		*/
	mtlr	%r11		/* Restore the LR.		*/
	blr			/* Return.			*/

/*****************************************************************************/

KIP_SC_ENTRY(".sc_perf", perf)
	syscall_entry
	lwz	%r11, SC_REG(SRR0) (%r1)
	lwz	%r12, SC_REG(SRR1) (%r1)
	lwz	%r31, SC_REG(R31)  (%r1)
	lwz	%r30, SC_REG(R30)  (%r1)
	lwz	%r2,  SC_REG(R2)   (%r1)
	lwz	%r1,  SC_REG(R1)   (%r1)
	mtsrr0	%r11
	mtsrr1	%r12
	rfi

/*****************************************************************************/

KIP_SC_ENTRY(".sc_thread_switch", thread_switch)
	globalize_tid TSWITCH_ABI_DEST
	syscall_entry
	stackify1 TSWITCH_ABI_DEST

