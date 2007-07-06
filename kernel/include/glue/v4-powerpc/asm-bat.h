/****************************************************************************
 *
 * Copyright (C) 2002, Karlsruhe University
 *
 * File path:	glue/v4-powerpc/asm-bat.h
 * Description:	Kernel BAT initialization code, used when starting a cpu.
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
 * $Id$
 *
 ***************************************************************************/

#ifndef __GLUE__V4_POWERPC__ASM_BAT_H__
#define __GLUE__V4_POWERPC__ASM_BAT_H__

#include INC_ARCH(bat.h)
#include INC_GLUE(bat.h)

#if defined(CONFIG_SMP)
# define BAT_WIMG	(1 << BAT_M)
#else
# define BAT_WIMG	0
#endif

/**
 * hard_init_bats While in physical mode, initialize the BAT registers to
 *   support some basic kernel execution.
 * @param %r13 The KERNEL_OFFSET.
 */
.macro	hard_init_bats
	/* Build the lower code BAT. */
	grab_sym %r3, _start_kernel			/* Get the va of the kernel code start. */
	andis.	%r3, %r3, CODE_BAT_PAGE_MASK@ha		/* Align to CODE_BAT_PAGE_SIZE. */
	mr	%r4, %r3				/* Save for the upper BAT. */
	sub	%r3, %r3, %r13				/* Convert to a real address. */
	andis.	%r3, %r3, BAT_BLOCK_MASK@ha		/* Convert to a BAT. */
	ori	%r3, %r3, BAT_WIMG | (BAT_PP_READ_ONLY << BAT_PP)	/* Finish BAT construction. */
	mtspr	IBAT_LOWER_KERNEL, %r3			/* Install lower code bat. */

	/* Build the upper code BAT. */
	andis.	%r4, %r4, BAT_BLOCK_MASK@ha		/* Convert to a BAT. */
	ori	%r4, %r4, (CODE_BAT_BL << BAT_BL) | (1 << BAT_VS)
#if defined(CONFIG_PPC_BAT_SYSCALLS)
	ori	%r4, %r4, (1 << BAT_VP)			/* Expose kernel to user mode. */
#endif
	mtspr	IBAT_UPPER_KERNEL, %r4			/* Install upper code bat. */

	/* Build the lower data BAT. */
	grab_sym %r3, _start_data			/* Get the va of the kernel data start. */
	andis.	%r3, %r3, DATA_BAT_PAGE_MASK@ha		/* Align to DATA_BAT_PAGE_SIZE. */
	mr	%r4, %r3				/* Save for the upper BAT. */
	sub	%r3, %r3, %r13				/* Convert to a real address. */
	andis.	%r3, %r3, BAT_BLOCK_MASK@ha		/* Convert to a BAT. */
	ori	%r3, %r3, BAT_WIMG | (BAT_PP_READ_WRITE << BAT_PP)	/* Finish BAT construction. */
	mtspr	DBAT_LOWER_KERNEL, %r3			/* Install the lower data bat. */

	/* Build the upper data BAT. */
	andis.	%r4, %r4, BAT_BLOCK_MASK@ha		/* Convert to a BAT. */
	ori	%r4, %r4, (DATA_BAT_BL << BAT_BL) | (1 << BAT_VS)
	mtspr	DBAT_UPPER_KERNEL, %r4			/* Install the upper data bat. */

	/* Build the lower exception BAT (1:1 for 128k). */
	li	%r3, (BAT_PP_READ_ONLY << BAT_PP) | BAT_WIMG
	mtspr	IBAT_LOWER_EXCEPT, %r3

	/* Build the upper exception BAT (1:1 for 128k). */
	li	%r4, (BAT_BL_128K << BAT_BL) | (1 << BAT_VS)
	mtspr	IBAT_UPPER_EXCEPT, %r4

	/* Clear the remaining BAT registers to ensure that we won't have any
	 * aliases.
	 */
	li	%r0, 0
	mtspr	DBAT_LOWER_PGHASH, %r0
	mtspr	DBAT_UPPER_PGHASH, %r0
	mtspr	DBAT_LOWER_OPIC, %r0
	mtspr	DBAT_UPPER_OPIC, %r0
	mtspr	DBAT_LOWER_CPU, %r0
	mtspr	DBAT_UPPER_CPU, %r0

	mtspr	IBAT_LOWER_UNUSED0, %r0
	mtspr	IBAT_UPPER_UNUSED0, %r0
	mtspr	IBAT_LOWER_UNUSED1, %r0
	mtspr	IBAT_UPPER_UNUSED1, %r0
.endm

#endif	/* __GLUE__V4_POWERPC__ASM_BAT_H__ */
