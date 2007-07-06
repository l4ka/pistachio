/*********************************************************************
 *                
 * Copyright (C) 2002,  University of New South Wales
 *                
 * File path:     arch/alpha/switch.S
 * Description:   Thread switch asm stubs 
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
 * $Id: switch.S,v 1.5 2003/09/24 19:05:25 skoglund Exp $
 *                
 ********************************************************************/

#include INC_ARCH(asm.h)
#include INC_ARCH(pal.h)

#define ALPHA_SWITCH_STACK_SIZE (8*8)
	
/* sjw (29/07/2002): If this changes, change thread.h as well! */
#define SAVE_SWITCH_STACK					\
	/* Need to save S registers (r9 .. r15) and ra (r26) */	\
	subq	$30, ALPHA_SWITCH_STACK_SIZE, $30;		\
	stq	$9, 0($30);					\
	stq	$10, 8($30);					\
	stq	$11, 16($30);					\
	stq	$12, 24($30);					\
	stq	$13, 32($30);					\
	stq	$14, 40($30);					\
	stq	$15, 48($30);					\
	stq	$26, 56($30)

#define RESTORE_SWITCH_STACK				\
	ldq	$9, 0($30);				\
	ldq	$10, 8($30);				\
	ldq	$11, 16($30);				\
	ldq	$12, 24($30);				\
	ldq	$13, 32($30);				\
	ldq	$14, 40($30);				\
	ldq	$15, 48($30);				\
	ldq	$26, 56($30);				\
	addq	$30, ALPHA_SWITCH_STACK_SIZE, $30
	
BEGIN_PROC(alpha_switch_to)
	SAVE_SWITCH_STACK	
	
	call_pal PAL_swpctx
	unop		/* Why have this here? */

	RESTORE_SWITCH_STACK
	ret	$31, ($26), 1
END_PROC(alpha_switch_to)

	
/* So r14 is the func, r15 is the arg.  We can't trash $0 either */ 
BEGIN_PROC(alpha_return_from_notify)
	mov	$0, $9		/* Save the pcb in an s reg. */
	mov	$15, $27	
	mov	$14, $16
	mov	$13, $17
	
	jsr	$26, ($27), 0
	
	mov	$9, $0
	RESTORE_SWITCH_STACK
	ret	$31, ($26)		
END_PROC(alpha_return_from_notify)

/* Just return from the interrupt */
BEGIN_PROC(alpha_return_to_user)
	call_pal	PAL_rti
	unop
	unop
	unop
END_PROC(alpha_return_to_user)