/*********************************************************************
 *                
 * Copyright (C) 1999-2010,  Karlsruhe University
 *                
 * File path:     elf-loader/platform/srm/head.S
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
/*
 * arch/alpha/boot/head.S
 *
 * initial bootloader stuff..
 */

#include <l4/alpha/pal.h>
#include <l4/alpha/asm.h>
	.set noreorder

	.section .text.start


	.globl	_start
	.ent	_start
_start:
	.prologue 0
	bis	$31,$31,$31
	br	1f
	/* room for the initial PCB, which comes here */
	.quad	0,0,0,0,0,0,0,0
1:	br	$27,2f
2:	ldgp	$29,0($27)
	lda	$27,main
	jsr	$26,($27),main
	call_pal PAL_halt
	.end _start

	.previous

	
	.align 3
	.globl tbi
	.ent tbi
tbi:	
	.prologue 0
        call_pal PAL_tbi
        ret     ($26)
        .end tbi

	.align 3
	.globl switch_to_osf_pal
	.ent switch_to_osf_pal
switch_to_osf_pal:
	.prologue 0
	subq	$30,128,$30
	stq	$26,0($30)
	stq	$1,8($30)	# regs clobbered by swppal
	stq	$2,16($30)
	stq	$3,24($30)
	stq	$4,32($30)
	stq	$5,40($30)
	stq	$6,48($30)
	stq	$7,56($30)
	stq	$8,64($30)
	stq	$9,72($30)
	stq	$10,80($30)
	stq	$11,88($30)
	stq	$12,96($30)
	stq	$13,104($30)
	stq	$14,112($30)
	stq	$15,120($30)

	stq	$30,0($17)	/* save KSP in PCB (a1) */

	bis	$30,$30,$20	/* a4 = KSP */
	br	$17,__do_swppal

	ldq	$26,0($30)
	ldq	$1,8($30)
	ldq	$2,16($30)
	ldq	$3,24($30)
	ldq	$4,32($30)
	ldq	$5,40($30)
	ldq	$6,48($30)
	ldq	$7,56($30)
	ldq	$8,64($30)
	ldq	$9,72($30)
	ldq	$10,80($30)
	ldq	$11,88($30)
	ldq	$12,96($30)
	ldq	$13,104($30)
	ldq	$14,112($30)
	ldq	$15,120($30)
	addq	$30,128,$30
	ret ($26)

/* This is only used here */
#define PAL_swppal      10
__do_swppal:
	call_pal PAL_swppal
	.end	switch_to_osf_pal


	.align 3
	.globl imb
	.ent imb
imb:
	call_pal PAL_imb
	ret $31, ($26)
	.end imb

BEGIN_PROC(halt)
	call_pal PAL_halt
END_PROC(halt)

