/*********************************************************************
 *
 * Copyright (C) 2003,  University of New South Wales
 *
 * File path:      contrib/elf-loader/platform/ofsparc64/head.S
 * Description:    The bootloader entry point for SPARC v9 OpenBoot
 *                 (Open Firmware) platforms.
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
 * $Id: head.S,v 1.2 2003/09/24 19:06:16 skoglund Exp $
 *
 ********************************************************************/

#include <l4/sparc64/asm.h>

/**********
* Defines *
**********/

/**
 *  Notes: On boot the register state is as follows:
 *		* o4 = Open Firmware client interface address.
 *		* o6 = Biased stack pointer (see below).
 *  The stack pointer is biased by (2047) on in the 64bit SPARC ABI.
 *
 *  Info from NetBSD's ofwboot for the sparc64 port.
 *
 *  A single space indent indicates an instruction in the branch delay slot.
 */

#define CC64FSZ 176 /* C compiler 64-bit frame size, 8 * (8in + 8local + 6) */ 

/**************
* Entry point *
**************/

BEGIN_PROC(_start, ".text")
	/* Test if we are using 64-bit SPARC ABI, stack should be biased. */
	btst	1, %sp		! Test if the stack is misalinged
	beq	1f
	 nop
	save	%sp, -CC64FSZ, %sp
	set	ofw_cihandle, %l0
	stx	%i4, [%l0] 	! Save client interface handle
	clr	%g4
	call	main
	 nop
1:
	set	0x1, %o0	! OFW_ERROR_NOSTART
	illtrap 0x1		! Platform does not appear to be 64-bit ABI

/* end _start */
	
/*********************
* Call Open Firmware *
*********************/

BEGIN_PROC(ofw_entry, ".text")
	.register %g2, #ignore
	.register %g3, #ignore
	sethi	%hi(ofw_cihandle), %o1
	ldx	[%o1 + %lo(ofw_cihandle)], %o4
	save	%sp, -CC64FSZ, %sp
	mov	%i0, %o0	
	mov	%g1, %l1
	mov	%g2, %l2
	mov	%g3, %l3
	mov	%g4, %l4
	mov	%g5, %l5
	mov	%g6, %l6
	mov	%g7, %l7
	rdpr	%pstate, %l0
	jmpl	%i4, %o7
	 wrpr	%g0, (0x0 | 0x4 | 0x2), %pstate	! PSTATE: TSO | PRIV | IE
	wrpr	%l0, %g0, %pstate
	mov	%l1, %g1
	mov	%l2, %g2
	mov	%l3, %g3
	mov	%l4, %g4
	mov	%l5, %g5
	mov	%l6, %g6
	mov	%l7, %g7
	ret
	 restore	%o0, %g0, %o0

/* end ofw_entry */
		
/********
* Panic *
********/

BEGIN_PROC(ofw_panic, ".text")
	illtrap 	/* Trap to the debugger if we can... */

/* end ofw_panic */