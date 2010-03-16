/*********************************************************************
 *                
 * Copyright (C) 1999-2010,  Karlsruhe University
 * Copyright (C) 2008-2009,  Volkmar Uhlig, IBM Corporation
 *                
 * File path:     crt0-powerpc.S
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

	.section ".text"
	.align	2
	.globl	_start
_start:
	mfspr	%r24, 0x11e /* bookE processor identifier */
	cmpwi	%r24, 0
	bgt	spin_secondary
	
	/* Install our stack. */
	lis	%r1, _mini_stack@ha
	la	%r1, _mini_stack@l(%r1)
	addi	%r1, %r1, -32

	/* Point to 0 for the small data area. */
	li	%r13, 0

	/* Jump into C code. */
	bl	__loader

1:	b	1b	/* Spin. */

	.global entry_secondary
entry_secondary: .long -1
spin_secondary:
	bl	1f
1:
	mflr	%r1
	subi	%r1,%r1,8
2:
	dcbi	0,%r1
	lwz	%r2, 0(%r1)
	addi	%r3,%r2,1
	cmpwi	%r3,0
	beq	2b
	iccci	0, 0
	mtlr	%r2
	blr

/****************************************************************************/

	.section ".bss"
        .align  16
	.space	4096
_mini_stack:
