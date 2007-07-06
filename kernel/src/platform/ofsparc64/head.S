/*********************************************************************
 *                
 * Copyright (C) 2003-2004, University of New South Wales
 *                
 * File path:    platform/ofsparc64/head.S
 * Description:  Kernel entry point for Open Firmware (OpenBoot)
 *               based SPARC v9 systems.
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
 * $Id: head.S,v 1.6 2004/07/01 04:10:51 philipd Exp $
 *                
 ********************************************************************/

#include INC_ARCH(asm.h)
#include INC_ARCH(asi.h)
#include INC_ARCH(registers.h)
	
/**************
* Entry point *
**************/

/**
 *  Assume that we have a one-to-one mapping of memory from 0x0 up.
 */
BEGIN_PROC(_start, .boot)

	/* Clean up the register file */
	flushw
	
	/* Fault in stack via firmware. */
	setx	_bootstack_bottom, %g6, %g7
	stx	%g0, [%g7]
	setx	_bootstack_top, %g6, %g7
	stx	%g0, [%g7]

	/* Take over from firmware. */
	mov	(PSTATE_TSO | PSTATE_PRIV), %g7
	wrpr	%g7, %pstate			! Set processor state.
	setx	ttable, %g6, %g7
	wrpr	%g7, %tba			! Set trap table base address.
	wrpr	0x1, %tl			! Set trap level (TL = 1).

	/* Setup register windows. */
	wrpr	0x0, %cwp			! First reg window is current.
	wrpr	(NWINDOWS - 2), %cansave	! All windows avaliable.
	wrpr	0x0, %canrestore		! Start of stack.
	wrpr	0x0, %otherwin			! No other address spaces.
	wrpr	(NWINDOWS - 2), %cleanwin	! All windows clean.
	wrpr	0x0, %wstate			! Use spill/fill handlers '0'.

	/* Setup boot stack. */
	setx	_bootstack_top, %g7, %sp		 
	sub	%sp, STACK_BIAS_64BIT, %sp	! Setup biased stack top.
	sub	%sp, STACK_FRAME_64BIT, %sp	! Allocate a frame for _start.
	mov	0, %fp				! Previous frame is invalid.
	
	/* Call platform init code. */
	call	ofsparc64_init
	 nop	

	/* Setup kernel stack. */
	setx	_init_stack_top, %g7, %sp	! Setup initial (pinned) stack 
	sub	%sp, STACK_BIAS_64BIT, %sp	! Setup biased stack top.
	sub	%sp, STACK_FRAME_64BIT, %sp	! Allocate a frame for _start.
	
	/* Start kernel - Long jump. */
	setx	startup_system, %g6, %g7
	call	%g7
	 nop	

	/* Shouldn't get here. */
	sir	0x0

/* END_PROC(_start) */

#warning awiggins (12-09-03): putc_boot needs to be removed from this file!
/**
 *  Startup debugging.
 */
BEGIN_PROC(putc_boot, .boot)
	setx	0x1FFF1400000, %g6, %g7	! Serial port base address
/* Check if TX buffer has space. */
1:	mov	0x60, %g6		! Channel B status register
	lduba	[%g7 + %g6] ASI_PHYS_BYPASS_EC_E_L, %g6
	btst	(1 << 6), %g6		! Test XFW bit 
	bz	1b
	 nop
/* Poll ready to give command. */
	btst	(1 << 2), %g6		! Test CEC bit
	bnz	1b
	 nop
/* Put charactor. */
	mov	0x40, %g6		! Channel B transmit FIFO
	stba	%o0, [%g7 + %g6] ASI_PHYS_BYPASS_EC_E_L
	mov	0x60, %g6		! Channel B control register
	mov	(1 << 3), %o0		! XF, ie transmit
	retl
	 stba	%o0, [%g7 + %g6] ASI_PHYS_BYPASS_EC_E_L

/* END_PROC(putc_boot) */

#define		INIT_STACK_SIZE		8192

/* Initial stack (separate from bootstack, which is used before
 * the kernel's pinned mapping is established) */
.section ".init.data"
	.align 16
_init_stack_bottom:
	.space	(INIT_STACK_SIZE)
_init_stack_top:

