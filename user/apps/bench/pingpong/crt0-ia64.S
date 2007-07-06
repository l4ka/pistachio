/*********************************************************************
 *                
 * Copyright (C) 2002, 2003,  Karlsruhe University
 *                
 * File path:     bench/pingpong/crt0-ia64.S
 * Description:   Startup code for ia64 processors
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
 * $Id: crt0-ia64.S,v 1.5 2003/09/24 19:05:53 skoglund Exp $
 *                
 ********************************************************************/
#include <config.h>

#define BEG_PROC(func)		\
	.global	func		;\
	.align	16		;\
	.proc	func		;\
func:

#define END_PROC(func)		\
	.endp	func

	.text
BEG_PROC (_start)
	movl	gp = __gp
	movl	r8 = main_stack_beg
	movl	sp = main_stack_end
	mov	ar.rsc = 0		// turn off RSE
	;;
	mov	ar.bspstore = r8
	;;
	mov	ar.rsc = 3		// turn RSE back on
	;;
	alloc	r8 = ar.pfs,0,0,0,0
	br.sptk.many main

	/* NOTREACHED */
END_PROC (_start)

	.bss
	.align	16
	.global main_stack_beg
	.global main_stack_end
main_stack_beg:	
	.space	8192-16
main_stack_end:	
	.space	16
	.previous


BEG_PROC (_start_pager)

	movl	gp = __gp
	movl	r8 = pager_stack
	mov	ar.rsc = 0
	;;
	mov	ar.bspstore = r8
	;;
	mov	ar.rsc = 3 + (3 << 2)
	;;
	alloc	r8 = ar.pfs,0,0,0,0
	br.sptk.many _Z5pagerv

END_PROC (_start_pager)



BEG_PROC (_start_ping_thread)

	movl	gp = __gp
	movl	r8 = ping_stack
	mov	ar.rsc = 0
	;;
	mov	ar.bspstore = r8
	;;
	mov	ar.rsc = 3 + (3 << 2)
	;;
	alloc	r8 = ar.pfs,0,0,0,0
	br.sptk.many _Z11ping_threadv

END_PROC (_start_ping_thread)



BEG_PROC (_start_pong_thread)

	movl	gp = __gp
	movl	r8 = pong_stack
	mov	ar.rsc = 0
	;;
	mov	ar.bspstore = r8
	;;
	mov	ar.rsc = 3 + (3 << 2)
	;;
	alloc	r8 = ar.pfs,0,0,0,0
	br.sptk.many _Z11pong_threadv

END_PROC (_start_ping_thread)
