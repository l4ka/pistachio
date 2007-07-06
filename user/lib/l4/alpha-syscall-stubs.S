/*********************************************************************
 *                
 * Copyright (C) 2002,  University of New South Wales
 *                
 * File path:     libl4/src/arch/alpha/exregs.S
 * Description:   asm implementation for exregs 
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
 * $Id: alpha-syscall-stubs.S,v 1.2 2003/09/24 19:06:28 skoglund Exp $
 *                
 ********************************************************************/

#include <l4/alpha/asm.h>

/*
 * L4_INLINE L4_ThreadId_t L4_ExchangeRegisters (
 *	          		      L4_ThreadId_t dest, -- r16
 *				      L4_Word_t control, -- r17
 *				      L4_Word_t sp, -- r18
 *				      L4_Word_t ip, -- r19
 *				      L4_Word_t flags, -- r20
 *				      L4_Word_t UserDefHandle, --r21
 *				      L4_ThreadId_t pager, -- 0 + FRAME(sp)
 *				      L4_Word_t *old_control, -- 8 + FRAME(sp)
 *				      L4_Word_t *old_sp, -- 16 + FRAME(sp)
 *				      L4_Word_t *old_ip, -- 24 + FRAME(sp)
 *				      L4_Word_t *old_flags, -- 32 + FRAME(sp)
 *				      L4_Word_t *old_UserDefHandle, 40 + FRAME(sp)
 *				      L4_ThreadId_t *old_pager 48 + FRAME(sp))
 */

/* Only need to save ra */
#define FRAME		8
#define IN_PAGER	(0  + FRAME)
#define OUT_CONTROL	(8  + FRAME)
#define OUT_SP		(16 + FRAME)
#define OUT_IP		(24 + FRAME)
#define OUT_FLAGS	(32 + FRAME)
#define OUT_UDH		(40 + FRAME)
#define OUT_PAGER	(48 + FRAME)
	
BEGIN_PROC(__alpha_L4_ExchangeRegisters)
	.prologue 1
	ldgp	$29, 0($27)
	subq	$30, FRAME, $30
	stq	$26, 0($30)

	ldq	$2, IN_PAGER($30)

	lda	$1, __L4_ExchangeRegisters
	ldq	$27, 0($1)

	jsr	$26, ($27)

	ldq	$1, OUT_CONTROL($30)
	stq	$16, 0($1)
	
	ldq	$1, OUT_SP($30)
	stq	$17, 0($1)

	
	ldq	$1, OUT_IP($30)
	stq	$18, 0($1)

	
	ldq	$1, OUT_FLAGS($30)
	stq	$19, 0($1)

	
	ldq	$1, OUT_UDH($30)
	stq	$20, 0($1)

	
	ldq	$1, OUT_PAGER($30)
	stq	$21, 0($1)

	ldq	$26, 0($30)
	addq	$30, FRAME, $30
	
	ret	$31, ($26)		
END_PROC(L4_ExchangeRegisters)
