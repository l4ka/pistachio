/*********************************************************************
 *                
 * Copyright (C) 2002,  University of New South Wales
 *                
 * File path:     glue/v4-alpha/traps.S
 * Description:   Kernel entry points for various traps 
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
 * $Id: traps.S,v 1.11 2003/10/21 07:50:22 sjw Exp $
 *                
 ********************************************************************/

#include INC_ARCH(asm.h)

.set noat
	
/* Interrupt.  We just send this straight to C after saving registers */
BEGIN_PROC(__ent_interrupt)
#ifdef  CONFIG_NO_CLOCK_IN_INT
	ldq	$18, 0($30)
	cmpeq	$18, 4, $18
	bne	$18, 1f
#endif	
	SAVE_ALL	

	DISABLE_INT($3, $4)
	
	lda	$27, handle_IRQ
	jsr	$26, ($27), handle_IRQ
	
	RESTORE_ALL
1:	
	call_pal PAL_rti
END_PROC(__ent_interrupt)

BEGIN_PROC(__ent_arith)
	SAVE_ALL	

	DISABLE_INT($3, $4)
	
	lda	$27, handle_arith
	jsr	$26, ($27), handle_arith
	
	RESTORE_ALL
	call_pal PAL_rti
END_PROC(__ent_arith)

BEGIN_PROC(__ent_mm)		
	SAVE_ALL	
	
	DISABLE_INT($3, $4)
		
	/* We need to put the context ptr in a3 */
	lda	$19, INTERRUPT_STACK_SIZE($30)
	lda	$27, handle_mm
	jsr	$26, ($27), handle_mm

	RESTORE_ALL
	call_pal PAL_rti
END_PROC(__ent_mm)


BEGIN_PROC(__ent_if)
	SAVE_ALL	

	DISABLE_INT($3, $4)
	
	/* Need to put the ctx ptr in a3, and the stack in a4 */
	lda	$19, INTERRUPT_STACK_SIZE($30)
	mov	$30, $20

	lda	$27, handle_if
	jsr	$26, ($27), handle_if
	
	RESTORE_ALL
	call_pal PAL_rti
END_PROC(__ent_if)

	
BEGIN_PROC(__ent_una)
	SAVE_ALL	

	DISABLE_INT($3, $4)

	/* Need to put the ctx ptr in a3, and the stack in a4 */
	lda	$19, INTERRUPT_STACK_SIZE($30)
	mov	$30, $20

		
	lda	$27, handle_una
	jsr	$26, ($27), handle_una
	
	RESTORE_ALL
	call_pal PAL_rti
END_PROC(__ent_una)


BEGIN_PROC(__return_except)
	RESTORE_ALL
	call_pal PAL_rti
END_PROC(__return_except)
	
