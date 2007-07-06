/****************************************************************************
 *
 * Copyright (C) 2003, University of New South Wales
 *
 * File path:	platform/ofg5/head.S
 * Description:	Kernel entry point
 *		kernel boot.  Clears .bss, sets up the initial stack,
 *		and jumps into C code.
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
 * $Id: head.S,v 1.3 2005/01/21 05:22:31 cvansch Exp $
 *
 ***************************************************************************/

#include INC_ARCH(asm.h)
#include INC_ARCH(msr.h)
#include INC_ARCH(ppc64_registers.h)


#define INIT_STACK_SIZE	    (4096*4)

/* Initialisation stack */
	.section ".init.data"
	.align 3
init_stack:
	.space (INIT_STACK_SIZE)

/* _start - the Entry point of L4
 *   It is assumed that we have the same lower 48 bit alignment as the kernel
 *   image.
 */

	.section ".init"
	.align	2
	.globl	_start
	.type	_start,@function
_start:
/* From here, we are running 64-bit with virt-addressing (due to OpenFirmware).
 * But we are still using physical addresses! Beware
 */
	/* Zero the BSS */
	LD_ADDR(r10, _start_bss)
	LD_ADDR(r11, _end_bss)
	subi	r10,	r10,	4
	subi	r11,	r11,	4
	LD_CONST(r6, KERNEL_OFFSET)
	sub	r10,	r10,	r6
	sub	r11,	r11,	r6
	li	r12,	0
1:	cmp	0,	r10,	r11
	beq	2f
	stwu	r12,	4(r10)
	b	1b
2:

	/* Load the TOC for start_kernel */
	LD_ADDR(r10, start_kernel)
	sub	r10,	r10,	r6
	ld	r2,	8(r10)
	sub	r2,	r2,	r6

	/*  Use our local init/boot stack.  */
	LD_ADDR (r1, init_stack)			/* Get a pointer to init_stack. */
	sub	r1,	r1,	r6
	addi	r1,	r1,	INIT_STACK_SIZE-48	/* Position ourselves at the top of the stack. */

	/* Setup correct CPU state */
	li	r11,	0
	mfspr	r12,	SPR_HID4
	rldimi	r12,	r11,	40, 23	    /* clear bit 23 - rm_ci */
	rldimi	r12,	r11,	2, 61	    /* clear bit 61 - lg_pg_dis */
	sync
	mtspr	SPR_HID4,   r12
	isync
	sync
	mfspr	r12,	SPR_HID5
	rldimi	r12,	r11,	6, 56	    /* clear bits 56-57 - dcbz 64-bit */
	sync
	mtspr	SPR_HID5,   r12
	isync
	sync

	bl	.start_kernel

	.long	0x00000000

