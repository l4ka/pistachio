/****************************************************************************
 *                
 * Copyright (C) 2003, University of New South Wales
 *                
 * File path:	arch/sparc64/switch.S
 * Description:	SPARC64 switch_to and notify code.
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
 * $Id: switch.S,v 1.4 2004/06/28 07:04:31 philipd Exp $
 *
 ***************************************************************************/

#include INC_ARCH(asm.h)

#include <asmsyms.h>

.register	%g2,#ignore
.register	%g3,#ignore

BEGIN_PROC(sparc64_do_notify, ".text")
	call	%o2
	nop
	ldx	[ %sp + SWITCH_FRAME_O7 + STACK_BIAS_64BIT ], %o7
	ldx	[ %sp + SWITCH_FRAME_I6 + STACK_BIAS_64BIT ], %i6
	ldx	[ %sp + SWITCH_FRAME_I7 + STACK_BIAS_64BIT ], %i7
	ldx	[ %sp + SWITCH_FRAME_O0 + STACK_BIAS_64BIT ], %o0
	ldx	[ %sp + SWITCH_FRAME_O1 + STACK_BIAS_64BIT ], %o1
	ldx	[ %sp + SWITCH_FRAME_O2 + STACK_BIAS_64BIT ], %o2
	ldx	[ %sp + SWITCH_FRAME_Y + STACK_BIAS_64BIT ], %l0
	wr	%l0, %y
	add	%sp, SWITCH_FRAME_SIZE, %sp
	jmp	%o7
	nop

/**
 * sparc64_initial_to_user: Starts a user thread.
 */
BEGIN_PROC(sparc64_initial_to_user, ".text")
	wr	%g0, %y
	clr	%g1
	clr	%g2
	clr	%g3
	clr	%g4
	clr	%g5
	clr	%g6
	clr	%o0
	clr	%o1
	clr	%o2
	clr	%o3
	clr	%o4
	clr	%o5
	clr	%o7
	clr	%l0
	clr	%l1
	clr	%l2
	clr	%l3
	clr	%l4
	clr	%l5
	clr	%l6
	clr	%l7
	clr	%i0
	clr	%i1
	clr	%i2
	clr	%i3
	clr	%i4
	clr	%i5
	clr	%i6
	clr	%i7
	ldx	[ %sp + TRAP_FRAME_O6 + STACK_BIAS_64BIT ], %sp
	retry /* jump to TL[0].TPC */
