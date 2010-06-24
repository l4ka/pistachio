/*********************************************************************
 *                
 * Copyright (C) 1999-2010,  Karlsruhe University
 * Copyright (C) 2008-2009,  Volkmar Uhlig, IBM Corporation
 *                
 * File path:     platform/ppc44x/startup.S
 * Description:   
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
 ********************************************************************/

#include INC_ARCH(ppc_registers.h)
#include INC_ARCH(msr.h)

	.section ".einit"
	.globl	_init_stack_bottom
	.globl	_init_stack_top

#define INIT_STACK_SIZE (4096)
_init_stack_bottom:
.space INIT_STACK_SIZE
_init_stack_top:


/*****************************************************************************
 *  Main entry point.
 */
	.section ".einit"
	.align	2
	.globl	_start
	.type	_start,@function
_start: 
	/* the 44x always has translation turned on.  We assume that
	the boot loader put sufficient number of TLB entries in so
		that we can boot-strap the kernel.  Interrupts are disabled. */

	/* set init MSR */
	lis	%r0, MSR_KERNEL_INIT@h
	ori	%r0, %r0,MSR_KERNEL_INIT@l
	mtmsr	%r0

	/* switch stack */
	lis	%r1, _init_stack_top@h
	ori	%r1, %r1,_init_stack_top@l

	mfspr	%r3, SPR_PIR
	cmpwi	%r3, 0
	bgt	wait_secondary

	bl	init_paging

	/* use relocated stack */
	addis	%r1, %r1, KERNEL_OFFSET@ha

	li	%r2, startup_system@l
	oris	%r2, %r2, startup_system@h
	mtctr	%r2
	bctr	
	
wait_secondary:
#ifdef CONFIG_SMP
	lis	%r2, secondary_release@ha
	la	%r2, secondary_release@l(%r2)
2:	dcbi	0,%r2
	lwz	%r0, 0(%r2)
	cmpw	%r0, %r3
	bne	2b

	bl	init_paging

	/* relocate stack */
	addis	%r1, %r1, KERNEL_OFFSET@ha

	li	%r2, startup_cpu@l
	oris	%r2, %r2, startup_cpu@h
	mfspr	%r3, SPR_PIR
	mtctr	%r2
	bctr
#else
	b	.
#endif
	.global secondary_release
secondary_release:
	.long	0
.end
