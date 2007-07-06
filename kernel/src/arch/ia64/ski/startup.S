/*********************************************************************
 *                
 * Copyright (C) 2002, 2003,  Karlsruhe University
 *                
 * File path:     arch/ia64/ski/startup.S
 * Description:   Itanium startup code
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
 * $Id: startup.S,v 1.5 2003/09/24 19:05:29 skoglund Exp $
 *                
 ********************************************************************/
#include INC_ARCH(asm.h)
#include INC_ARCH(psr.h)

	.section .init.data

	.globl	_initial_register_stack
	.globl	_initial_stack

	.align	8192
_initial_register_stack:
	.space	8192-16
_initial_stack:
	.space	16
	.text

BEG_PROC(_start)

	movl	r9 = ia64_vector_table
	;;
	mov	cr.iva = r9
	movl	gp = __gp
	movl	r9 = _initial_register_stack
	movl	sp = _initial_stack
	movl	r10 = CONFIG_IA64_PHYSMEM_OFFSET
	mov	ar.rsc = 0
	;;
	dep	r9 = 0, r9, 61, 3	// translate to phys addr
	dep	sp = 0, sp, 61, 3
	dep	gp = 0, gp, 61, 3
	;;
	sub	r9 = r9, r10
	sub	sp = sp, r10
	sub	gp = gp, r10
	;;
	mov	ar.bspstore = r9
	;;
	loadrs				// invalidate regs
	;;
	mov	ar.rsc = 3		// turn rse back on
	mov	ar.fpsr = 0x3f		// disable fp traps
	;;
	alloc	r9 = ar.pfs,0,0,1,0
	ssm	psr.ic
	bsw.1
	;;
	srlz.i

	br.call.sptk.many rp = init_efi_ski	// returns bootinfo structure
 	
	mov	out0 = r8
	br.call.sptk.many rp = startup_system
	/* NOTREACHED */

END_PROC(_start)
