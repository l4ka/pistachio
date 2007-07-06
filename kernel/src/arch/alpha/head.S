/*********************************************************************
 *                
 * Copyright (C) 2002,  University of New South Wales
 *                
 * File path:     arch/alpha/head.S
 * Description:    The kernels entry point 
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
 * $Id: head.S,v 1.7 2003/09/24 19:05:25 skoglund Exp $
 *                
 ********************************************************************/

#include INC_ARCH(asm.h)
#include INC_ARCH(pal.h)

	.lcomm bootstrap_pcb, 64

/* This function expects the current PTBR in a0 ($16) */
/* sjw (04/08/2002): This is majorly hacky ;( */
BEGIN_PROC(_start)
	br	$27, 1f
1:	ldgp	$29, 0($27)

	lda	$1, bootstrap_pcb
	lda	$2, _bootstack_top 
	stq	$2,   0($1)    /* KSP */
	stq	$16, 16($1)    /* PTBR */
	
	extll	$1, 0, $16
	call_pal PAL_swpctx
	
	br	$27, 2f
2:	ldgp	$29, 0($27)
			
	lda	$27, startup_system
	jsr	$26, ($27)

	call_pal PAL_halt
END_PROC(_start)

#define INIT_HWRPB 0x10000000
	
BEGIN_PROC(fixup)
	.prologue 0
	subq	$30,80,$30
	stq	$26,0($30)
	stq	$29,8($30)

	stq	$8,16($30)	/* OpenVMS save regs */
	stq	$9,24($30)
	stq	$10,32($30)
	stq	$11,40($30)
	stq	$12,48($30)
	stq	$13,56($30)
	stq	$14,64($30)
	stq	$15,72($30)

	lda	$1, INIT_HWRPB
	ldq	$2,0xc0($1)		/* crb offset */
	addq	$2,$1,$2		/* crb */
	ldq	$27,16($2)		/* fixup procedure value */

	ldq	$2,8($27)		/* fixup call address */
	jsr	$26,($2)		/* call it (weird VMS call seq) */

	ldq	$26,0($30)
	ldq	$29,8($30)

	ldq	$8,16($30)
	ldq	$9,24($30)
	ldq	$10,32($30)
	ldq	$11,40($30)
	ldq	$12,48($30)
	ldq	$13,56($30)
	ldq	$14,64($30)
	ldq	$15,72($30)

	addq	$30,80,$30
	ret	$31,($26)
END_PROC(fixup)

	
