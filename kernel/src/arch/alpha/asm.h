/*********************************************************************
 *                
 * Copyright (C) 2002-2003,  University of New South Wales
 *                
 * File path:     arch/alpha/asm.h
 * Created:       23/07/2002 17:48:20 by Simon Winwood (sjw)
 * Description:   Assembler macros etc. 
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
 * $Id: asm.h,v 1.7 2004/06/04 02:14:22 cvansch Exp $
 *                
 ********************************************************************/

#ifndef __ARCH__ALPHA__ASM_H__
#define __ARCH__ALPHA__ASM_H__

#include INC_ARCH(pal.h)
	
#define BEGIN_PROC(name)			\
    .global name;				\
    .align 3;					\
    .ent name;					\
name:

#define END_PROC(name)				\
    .end name


/* This defines the normal saved register layout.
 *
 * regs 9-15 preserved by C code
 * regs 16-18 saved by PAL-code
 * regs 29-30 saved and set up by PAL-code
 */

#define INTERRUPT_STACK_SIZE  (8 * 26)

#define SAVE_ALL				\
	subq	$30, INTERRUPT_STACK_SIZE, $30;	\
        stq     $0, 0($30);			\
        stq     $1, 8($30);			\
        stq     $2, 16($30);			\
        stq     $3, 24($30);			\
        stq     $4, 32($30);			\
        stq     $5, 40($30);			\
        stq     $6, 48($30);			\
        stq     $7, 56($30);			\
        stq     $8, 64($30);			\
        stq     $9, 72($30);			\
        stq     $10, 80($30);			\
        stq     $11, 88($30);			\
        stq     $12, 96($30);			\
        stq     $13, 104($30);			\
        stq     $14, 112($30);			\
        stq     $15, 120($30);			\
        stq     $19, 128($30);			\
        stq     $20, 136($30);			\
        stq     $21, 144($30);			\
        stq     $22, 152($30);			\
        stq     $23, 160($30);			\
        stq     $24, 168($30);			\
        stq     $25, 176($30);			\
        stq     $26, 184($30);			\
        stq     $27, 192($30);			\
        stq     $28, 200($30);
	
#define RESTORE_ALL				\
        ldq     $0, 0($30);			\
        ldq     $1, 8($30);			\
        ldq     $2, 16($30);			\
        ldq     $3, 24($30);			\
        ldq     $4, 32($30);			\
        ldq     $5, 40($30);			\
        ldq     $6, 48($30);			\
        ldq     $7, 56($30);			\
        ldq     $8, 64($30);			\
        ldq     $9, 72($30);			\
        ldq     $10, 80($30);			\
        ldq     $11, 88($30);			\
        ldq     $12, 96($30);			\
        ldq     $13, 104($30);			\
        ldq     $14, 112($30);			\
        ldq     $15, 120($30);			\
        ldq     $19, 128($30);			\
        ldq     $20, 136($30);			\
        ldq     $21, 144($30);			\
        ldq     $22, 152($30);			\
        ldq     $23, 160($30);			\
        ldq     $24, 168($30);			\
        ldq     $25, 176($30);			\
        ldq     $26, 184($30);			\
        ldq     $27, 192($30);			\
        ldq     $28, 200($30);			\
	addq	$30, INTERRUPT_STACK_SIZE, $30

/* Use 6 rather than 7 here as we want correctable errors */
#define DISABLE_INT(tmp1, tmp2) 		\
	mov	$16, tmp1;			\
	mov	$0, tmp2;			\
	lda	$16, 6;				\
	call_pal	PAL_swpipl;		\
	mov	tmp1, $16;			\
	mov	tmp2, $0

#define DISABLE_INT_NOSAVE			\
	lda	$16, 6;				\
	call_pal	PAL_swpipl

#endif /* __ARCH__ALPHA__ASM_H__ */
