/*********************************************************************
 *                
 * Copyright (C) 2002,  University of New South Wales
 *                
 * File path:     kdb/platform/dp264/dispatch.S
 * Description:   Dispatch routine.
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
 * $Id: dispatch.S,v 1.2 2003/09/24 19:06:19 skoglund Exp $
 *                
 ********************************************************************/

#include <l4/alpha/asm.h>
#include <l4/alpha/pal.h>
		
BEGIN_PROC(dispatch)
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

	mov	$20, $1         	/* HWRPB in a4 */	
	ldq	$2,0xc0($1)		/* crb offset */
	addq	$2,$1,$2		/* crb */
	ldq	$27,0($2)		/* dispatch procedure value */
	
	ldq	$2,8($27)		/* dispatch call address */
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
END_PROC(dispatch)
