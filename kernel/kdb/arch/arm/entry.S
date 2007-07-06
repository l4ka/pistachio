/*********************************************************************
 *                
 * Copyright (C) 2002,  Karlsruhe University
 *                
 * File path:     kdb/arch/arm/entry.S
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
 * $Id: entry.S,v 1.8 2004/09/07 20:12:40 cvansch Exp $
 *                
 ********************************************************************/

#include INC_ARCH(thread.h)

/**********************************************************************
	kdebug entry
 **********************************************************************/
	

	.section .kdebug
3:	.word	_Z12kdebug_entryPv

	.globl	kern_kdebug_entry
kern_kdebug_entry:
	stmdb	sp, {r0-r14}^
	nop
	sub	sp, sp, #PT_SIZE
	str	lr, [sp, #PT_PC]
	mrs	lr, cpsr
	str	lr, [sp, #PT_KLR]
	mrs	lr, spsr
	str	lr, [sp, #PT_CPSR]

	adr	r0, _kdebug_str
        bl      printf

	mov	r0, sp
	
	/* use kdebug's own stack */
	ldr	sp, _kdebug_stack
	stmdb	sp!, {r0}
	adr	lr, 1f
	ldr	pc, 3b
1:
	ldr	sp, _kdebug_stack
	ldr	sp, [sp, #-4]

	ldr	lr, [sp, #PT_KLR]
	msr	cpsr, lr
	ldr	lr, [sp, #PT_CPSR]
	msr	spsr, lr
	add	sp, sp, #PT_SIZE
	ldmdb	sp, {r0-r14}^
	nop
	ldr	lr, [sp, #PT_PC-PT_SIZE]
	mov	pc, lr

	.globl	_kdebug_stack
_kdebug_stack:
	.word	_kdebug_stack_top

_kdebug_str:
	.string	"--- KD# %s ---\n" 

	/* alignment so that stack looks like a tcb. NB space_t field should be NULL */
	.align	12
	.globl	_kdebug_stack_bottom
_kdebug_stack_bottom:
	.space	1024
_kdebug_stack_top:

