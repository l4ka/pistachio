/*********************************************************************
 *                
 * Copyright (C) 2003,  Karlsruhe University
 *                
 * File path:     l4test/ia64/l4fix.S
 * Description:   Assembly stubs for ia64
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
 * $Id: l4fix.S,v 1.4 2003/09/24 19:06:00 skoglund Exp $
 *                
 ********************************************************************/

	.file	"l4fix.S"
	.text


/* where we ex-reg things, so they work on ia64 */
	.align 16
	.global _exreg_target
	.align	16
	.proc	_exreg_target
_exreg_target:
	/* load GP and RSE spots */
	movl	gp = __gp

	/* Load the jump address and reg-stack address from the stack */
	ld8     r8 = [sp], 8
		;; 
	ld8	r9 = [sp], 8
		;; 
	mov	b0 = r9
		;; 

	/* setup the RSE */
	mov	ar.rsc = 0
	;;
	mov	ar.bspstore = r8
	;;
	mov	ar.rsc = 3 + (3 << 2)
	;;
	alloc	r8 = ar.pfs,0,0,0,0
	br.sptk.many b0
	.endp	_exreg_target
