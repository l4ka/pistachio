/****************************************************************************
 *
 * Copyright (C) 2002, Karlsruhe University
 *
 * File path:	platform/ofppc/startup.S
 * Description:	Kernel entry point, ie. the first instruction executed upon
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
 * $Id: startup.S,v 1.19 2003/11/01 17:33:41 joshua Exp $
 *
 ***************************************************************************/

#include INC_ARCH(msr.h)
#include INC_PLAT(ofppc.h)	/* psim constants */
#include INC_GLUE(offsets.h)	/* kernel offset */
#include INC_GLUE(asm.h)
#include INC_GLUE(asm-bat.h)

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
	.section ".init"
	.align	2
	.globl	_start
	.type	_start,@function
_start:
	/*  We could be running in real or virtual mode.  Lets switch
	 *  to real mode, map the kernel with the BAT registers, and 
	 *  enter virtual mode.
	 */  
	lis	%r13, KERNEL_OFFSET@ha		/* Get the kernel offset. */
	grab_sym %r11, _real_mode		/* Get the address of the bat init code. */
	sub	%r11, %r11, %r13			/* Create a real address. */
	grab_sym %r12, _virt_mode		/* Get the address of the return code. */

	/* Jump to real mode. */
	grab_sym %r14, MSR_REAL			/* Grab the MSR_REAL constant. */
	mtsrr0	%r11				/* Set the real address. */
	mtsrr1	%r14				/* Use an msr that disables virtual addressing. */
	rfi					/* Enable real mode, and jump below. */

_real_mode:

	/* Include the BAT initialization code.  
	 * It expects r13 to hold the KERNEL_OFFSET.
	 */
	hard_init_bats

	/* Map the cpu data area with a data BAT.  First build the lower BAT.
	 */
	grab_sym %r3, _cpu_phys			/* Get the physical address of the cpu area. */
	andis.	%r3, %r3, BAT_BLOCK_MASK@ha	/* Convert to a BAT. */
	ori	%r3, %r3, BAT_WIMG | (BAT_PP_READ_WRITE << BAT_PP)	/* Finish BAT construction. */
	mtspr	DBAT_LOWER_CPU, %r3		/* Install the lower cpu data bat. */

	/* Build the upper cpu data BAT.
	 */
	grab_sym %r4, KERNEL_CPU_OFFSET		/* Get the virtual address of the cpu area. */
	andis.	%r4, %r4, BAT_BLOCK_MASK@ha	/* Convert to a BAT. */
	ori	%r4, %r4, (BAT_BL_128K << BAT_BL) | (1 << BAT_VS)
	mtspr	DBAT_UPPER_CPU, %r4		/* Install the upper cpu data bat. */
	

	/* Jump to virtual mode. */
	mtsrr0	%r12			/* Jump to a valid virtual address. */
	grab_sym %r10, MSR_KERNEL_INIT  /* Grab the MSR_KERNEL_INIT constant. */
	mtsrr1	%r10			/* Install an initial MSR. */
	rfi

_virt_mode:

	/*  We have virtual addressing.
	 */

	/*  Use our local init/boot stack.  */
	grab_sym %r1, init_stack		/* Get a pointer to init_stack. */
	addi	%r1, %r1, INIT_STACK_SIZE-32	/* Position ourselves at the top of the stack. */

	/* initialize the system reserved register */
	li	%r2, 0
	/* point to 0 for the small data area */
	li	%r13, 0

	/*  Initialize .bss (which also zeros the stack).  */
#define BSS_START	_start_bss
#define BSS_END		_end_bss
	grab_sym %r10, BSS_START
	subi	%r10, %r10, 4
	grab_sym %r11, BSS_END
	subi	%r11, %r11, 4
	li	%r12, 0
1:	cmp	0, %r10, %r11
	beq	2f
	stwu	%r12, 4(%r10)
	b	1b

	/*  Jump into C code.  */
2:
	bl	l4_powerpc_init
	
3:	b	3b		/* we should never execute this line.  */
._start_end:
	.size	_start,._start_end-_start


/*****************************************************************************
 * OpenFirmware note section.
 */

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

