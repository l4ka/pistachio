/****************************************************************************
 *
 * Copyright (C) 2002, Karlsruhe University
 *
 * File path:	kdb/platform/ofppc/stack.S
 * Description:	Switches to Open Firmware's original address space,
 *		switches to a large stack, invokes a function,
 *		and then restores the kernel's execution environtment.
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
 * $Id: stack.S,v 1.5 2003/09/24 19:05:20 skoglund Exp $
 *
 ***************************************************************************/

#if defined(CONFIG_KDB_CONS_OF1275)

#include INC_ARCH(ibm750.h)
#include INC_ARCH(msr.h)
#include INC_ARCH(page.h)
#include INC_GLUE(offsets.h)

/*
 * extern "C" word_t kdb_switch_space( word_t htab_loc, word_t *segments,
 *			old_htab_loc, word_t *old_segments,
 * 			word_t stack, word_t (*func)(void *), void *param )
 */

#define GRAB_SYM(dst,sym)	\
	lis	dst, sym @ha ;	\
	la	dst, sym @l(dst) ;

#define INSTALL_SR(dst,src,which)	\
	lwz	dst, (which * 4)(src) ;	\
	mtsr	which, dst ;

#define ENTER_PHYSICAL(junk1,junk2,sym)					\
	lis	junk1, KERNEL_OFFSET@ha	; /* Get the kernel offset. */	\
	GRAB_SYM(junk2, sym) ;						\
	sub	junk2, junk2, junk1 ; /* Convert to a physical address. */ \
	li	junk1, 0 ; 	/* Create a null msr. */		\
	mtsrr0	junk2 ;		/* Execute at a real address. */	\
	mtsrr1	junk1 ;	/* Disables virtual addressing. */		\
	rfi ;

#define ENTER_VIRTUAL(junk,msr,sym)					\
	GRAB_SYM(junk, sym) ;						\
	mtsrr0	junk ;		/* Execute at a virtual address. */	\
	mtsrr1	msr ;		/* Enables virtual addressing. */	\
	rfi ;


#define FLUSH_TLB(junk1,junk2)	\
	GRAB_SYM(junk2, POWERPC_TLBIE_MAX); \
	li	junk1, 0 ;	\
2:	tlbie	junk1 ;		\
	addi	junk1, junk1, POWERPC_TLBIE_INC ;	\
	cmpw	0, junk1, junk2 ;	\
	blt	2b ;		\
	tlbsync ;


	.section ".text"
	.align	2
	.globl	kdb_switch_space
	.type	kdb_switch_space,@function
kdb_switch_space:

#define PARAM_HTAB		%r3
#define PARAM_SEGMENTS		%r4
#define PARAM_OLD_HTAB		%r5
#define PARAM_OLD_SEGMENTS	%r6
#define PARAM_STACK		%r7
#define PARAM_FUNC		%r8
#define PARAM_FUNC_ARG		%r9

#define OLD_MSR			%r4

#define STACK_SAVE_LR		8
#define STACK_SAVE_OLD_SP	12
#define STACK_SAVE_MSR		16
#define STACK_SAVE_CR		20
#define STACK_SAVE_HTAB		24
#define STACK_SAVE_SEGMENTS	28

	/* Stabilize CPU context. */
	isync

	/* Set-up the stack.
	 */
	mfcr	%r11
	mr	%r10, %r1	/* Grab the old stack. */

	cmpwi	0, PARAM_STACK, 0	/* Is the new stack NULL? */
	bne	1f		/* Jump if not NULL. */
	mr	PARAM_STACK, %r1	/* If NULL, reuse the current stack. */
1:	addi	%r1, PARAM_STACK, -32	/* Install the new stack. */
	stw	%r10, STACK_SAVE_OLD_SP(%r1)	/* Save the old stack. */

	/* Save other stuff.
	 */
	mflr	%r0		/* Grab the lr. */
	stw	%r0, STACK_SAVE_LR(%r1)	/* Save lr. */

	mfmsr	%r0		/* Read the msr. */
	stw	%r0, STACK_SAVE_MSR(%r1)	/* Save the msr. */

	stw	%r11, STACK_SAVE_CR(%r1)	/* Save cr. */

	/* Save parameters on the stack. */
	stw	PARAM_OLD_HTAB, STACK_SAVE_HTAB(%r1)
	stw	PARAM_OLD_SEGMENTS, STACK_SAVE_SEGMENTS(%r1)

	/* Jump to physical mode.
	 */
	ENTER_PHYSICAL(%r11, %r12, .phys_activate)

.phys_activate:

	/* Switch to the new target address space.
	 */
	FLUSH_TLB( %r11, %r12 )

	mtsdr1	PARAM_HTAB	/* Install the new page hash. */

	/* Convert the segment's pointer into a physical address.
	 */
	lis %r0, KERNEL_OFFSET@ha	/* Get the kernel offset. */
	sub PARAM_SEGMENTS, PARAM_SEGMENTS, %r0

	INSTALL_SR( %r0, PARAM_SEGMENTS, 0 )
	INSTALL_SR( %r0, PARAM_SEGMENTS, 1 )
	INSTALL_SR( %r0, PARAM_SEGMENTS, 2 )
	INSTALL_SR( %r0, PARAM_SEGMENTS, 3 )
	INSTALL_SR( %r0, PARAM_SEGMENTS, 4 )
	INSTALL_SR( %r0, PARAM_SEGMENTS, 5 )
	INSTALL_SR( %r0, PARAM_SEGMENTS, 6 )
	INSTALL_SR( %r0, PARAM_SEGMENTS, 7 )
	INSTALL_SR( %r0, PARAM_SEGMENTS, 8 )
	INSTALL_SR( %r0, PARAM_SEGMENTS, 9 )
	INSTALL_SR( %r0, PARAM_SEGMENTS, 10 )
	INSTALL_SR( %r0, PARAM_SEGMENTS, 11 )
	INSTALL_SR( %r0, PARAM_SEGMENTS, 12 )
	INSTALL_SR( %r0, PARAM_SEGMENTS, 13 )
	INSTALL_SR( %r0, PARAM_SEGMENTS, 14 )
	INSTALL_SR( %r0, PARAM_SEGMENTS, 15 )

	/* Jump to virtual mode.
	 */
	GRAB_SYM( %r0, MSR_KERNEL )
	ENTER_VIRTUAL( %r11, %r0, .virt_new_space )
	
.virt_new_space:

	/* Execute the target function in our new space.
	 */
	mtctr	PARAM_FUNC	/* Prepare to jump to the function. */
	mr	%r3, PARAM_FUNC_ARG	/* Setup the function parameter. */
	bctrl
	/* Be sure to return r3 to our caller! */

	/* Load some of the saved values.
	 */
	lwz	OLD_MSR, STACK_SAVE_MSR(%r1)	/* Load the original msr. */
	lwz	PARAM_OLD_HTAB, STACK_SAVE_HTAB(%r1)
	lwz	PARAM_OLD_SEGMENTS, STACK_SAVE_SEGMENTS(%r1)

	/* Jump to physical mode.
	 */
	ENTER_PHYSICAL( %r11, %r12, .phys_restore )

.phys_restore:

	/* Restore our original address space.
	 */
	FLUSH_TLB( %r11, %r12 )

	mtsdr1	PARAM_OLD_HTAB	/* Install the new page hash. */

	/* Convert the segment's pointer into a physical address.
	 */
	lis %r0, KERNEL_OFFSET@ha	/* Get the kernel offset. */
	sub PARAM_OLD_SEGMENTS, PARAM_OLD_SEGMENTS, %r0

	INSTALL_SR( %r0, PARAM_OLD_SEGMENTS, 0 )
	INSTALL_SR( %r0, PARAM_OLD_SEGMENTS, 1 )
	INSTALL_SR( %r0, PARAM_OLD_SEGMENTS, 2 )
	INSTALL_SR( %r0, PARAM_OLD_SEGMENTS, 3 )
	INSTALL_SR( %r0, PARAM_OLD_SEGMENTS, 4 )
	INSTALL_SR( %r0, PARAM_OLD_SEGMENTS, 5 )
	INSTALL_SR( %r0, PARAM_OLD_SEGMENTS, 6 )
	INSTALL_SR( %r0, PARAM_OLD_SEGMENTS, 7 )
	INSTALL_SR( %r0, PARAM_OLD_SEGMENTS, 8 )
	INSTALL_SR( %r0, PARAM_OLD_SEGMENTS, 9 )
	INSTALL_SR( %r0, PARAM_OLD_SEGMENTS, 10 )
	INSTALL_SR( %r0, PARAM_OLD_SEGMENTS, 11 )
	INSTALL_SR( %r0, PARAM_OLD_SEGMENTS, 12 )
	INSTALL_SR( %r0, PARAM_OLD_SEGMENTS, 13 )
	INSTALL_SR( %r0, PARAM_OLD_SEGMENTS, 14 )
	INSTALL_SR( %r0, PARAM_OLD_SEGMENTS, 15 )

	ENTER_VIRTUAL( %r12, OLD_MSR, .virt_old_space )

.virt_old_space:
	lwz	%r0,  STACK_SAVE_LR(%r1)	/* Extract lr. */
	lwz	%r10, STACK_SAVE_OLD_SP(%r1)	/* Extract the old sp. */
	lwz	%r11, STACK_SAVE_CR(%r1)	/* Extract the old cr. */
	mr	%r1,  %r10			/* Restore the old stack. */
	mtlr	%r0
	mtcr	%r11
	blr

#endif	/* CONFIG_KDB_CONS_OF1275 */

