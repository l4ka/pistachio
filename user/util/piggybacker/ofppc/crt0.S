/****************************************************************************
 *
 * Copyright (C) 2002-2003, Karlsruhe University
 *
 * File path:	piggybacker/ofppc/crt0.S
 * Description:	Jumps to C code for the loader.
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
 * $Id: crt0.S,v 1.5 2003/09/24 19:06:38 skoglund Exp $
 *
 ***************************************************************************/


/*****************************************************************************
 *  Define a stack.
 */
	.section ".bss"
	.globl	_init_stack_bottom
	.globl	_init_stack_top

#define INIT_STACK_SIZE (4096*3)
_init_stack_bottom:
.lcomm	init_stack, INIT_STACK_SIZE, 16
_init_stack_top:

/*****************************************************************************
 *  Main entry point.
 */
	.section ".text"
	.align	2
	.globl	_start
_start:
	/*  Use our local stack.  */
	lis	%r1, init_stack@ha
	la	%r1, init_stack@l(%r1)
	addi	%r1, %r1, INIT_STACK_SIZE-32

	/* initialize the system reserved register */
	li	%r2, 0
	/* point to 0 for the small data area */
	li	%r13, 0

	/*  Initialize .bss (which also zeros the stack).  */
#define BSS_START	__bss_start
#define BSS_END		_end
	lis	%r10, BSS_START@ha
	la	%r10, BSS_START@l(%r10)
	subi	%r10, %r10, 4
	lis	%r11, BSS_END@ha
	la	%r11, BSS_END@l(%r11)
	subi	%r11, %r11, 4
	li	%r12, 0
1:	cmp	0, %r10, %r11
	beq	2f
	stwu	%r12, 4(%r10)
	b	1b

	/*  Jump into C code.  */
2:
	bl	loader_main
	
3:	b	3b		/* we should never execute this line.  */

/*****************************************************************************
 *  Kernel hand-off point.
 *  void enter_kernel( r3, r4, r5, start_ip )
 */
	.section ".text"
	.align	2
	.globl	enter_kernel
enter_kernel:
	sync
	isync

	li	%r10, 0
	mtsrr1	%r10		// Clear the msr, and disable everything.
	mtsrr0	%r6		// Install the kernel's start ip address.
	rfi

/*****************************************************************************
 * OpenFirmware note section.
 */

#if 0
	.section ".note"
	.align	2
.note_section_header:
	.long	8		/* note name length (including null byte) */
	.long	24		/* note descriptor length */
	.long	0x1275		/* note type */
	.string	"PowerPC"
.note_descriptor:
	.long	0x0		/* real-mode (0 == false)	*/
	.long	0xffffffff	/* real-base (-1 == default)	*/
	.long	0xffffffff	/* real-size (-1 == default)	*/
	.long	0xffffffff	/* virt-base (-1 == default)	*/
	.long	0xffffffff	/* virt-size (-1 == default)	*/
	.long	0xffffffff	/* load-base (-1 == default)	*/

#endif

