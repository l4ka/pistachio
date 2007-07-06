/*********************************************************************
 *                
 * Copyright (C) 2002,  University of New South Wales
 *                
 * File path:     glue/v4-alpha/user.S
 * Description:   User syscall stubs 
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
 * $Id: user.S,v 1.6 2004/04/06 07:27:14 benno Exp $
 *                
 ********************************************************************/

#include INC_ARCH(asm.h)
#include INC_ARCH(pal.h)
#include INC_GLUE(syscalls.h)
#include <asmsyms.h>
#include INC_GLUE(ipcregs.h)

#define SYSCALL(name)				\
.section .user.##name, "ax";			\
BEGIN_PROC(user_##name);			\
    subq   $30, 8, $30;				\
    stq    $26, 0($30);				\
    lda    $0, SYSCALL_##name;			\
    call_pal  PAL_callsys;			\
    ldq    $26, 0($30);				\
    addq   $30, 8, $30;				\
    ret    $31, ($26);				\
END_PROC(user_##name);				\
.previous

#define SYSCALL_UNIMPLEMENTED(name)		\
.section .user.##name, "ax";			\
BEGIN_PROC(user_##name);			\
	lda	$0, 0;				\
	ret	$31, ($26);			\
END_PROC(user_##name);				\
.previous
	
/* This is special due to saving s regs. */
.section .user.ipc, "ax"
BEGIN_PROC(user_ipc)
	subq   $30, 8 * 8, $30 /* Make room for ra and s0 - s6 ($9 - $15) */

	stq    $9,   0($30)
	stq    $10,  8($30)
	stq    $11, 16($30)
	stq    $12, 24($30)
	stq    $13, 32($30)
	stq    $14, 40($30)
	stq    $15, 48($30)	
	stq    $26, 56($30)


/* API defines */					
#define mr0		R_MR0
#define mr1		R_MR1
#define mr2		R_MR2
#define mr3		R_MR3
#define mr4		R_MR4
#define mr5		R_MR5
#define mr6		R_MR6
#define mr7		R_MR7
#define mr8		R_MR8
	
#define current_utcb	$0
	
	/* Load MRs */
	call_pal PAL_rdunique
	ldq	mr0, UTCB_MR_OFFSET +  0(current_utcb)
	ldq	mr1, UTCB_MR_OFFSET +  8(current_utcb)
	ldq	mr2, UTCB_MR_OFFSET + 16(current_utcb)
	ldq	mr3, UTCB_MR_OFFSET + 24(current_utcb)
	ldq	mr4, UTCB_MR_OFFSET + 32(current_utcb)
	ldq	mr5, UTCB_MR_OFFSET + 40(current_utcb)
	ldq	mr6, UTCB_MR_OFFSET + 48(current_utcb)
	ldq	mr7, UTCB_MR_OFFSET + 56(current_utcb)						
	ldq	mr8, UTCB_MR_OFFSET + 64(current_utcb)						
	
	mov	$31, $0
#if defined(CONFIG_ALPHA_PAL_IPC_FASTPATH)
	call_pal  0xb0
#else
	call_pal  PAL_callsys
#endif /* CONFIG_ALPHA_PAL_IPC_FASTPATH */

	mov	$0, $16
#if 1
	call_pal PAL_rdunique	
	stq	mr0, UTCB_MR_OFFSET +  0(current_utcb)
	stq	mr1, UTCB_MR_OFFSET +  8(current_utcb)
	stq	mr2, UTCB_MR_OFFSET + 16(current_utcb)
	stq	mr3, UTCB_MR_OFFSET + 24(current_utcb)
	stq	mr4, UTCB_MR_OFFSET + 32(current_utcb)
	stq	mr5, UTCB_MR_OFFSET + 40(current_utcb)
	stq	mr6, UTCB_MR_OFFSET + 48(current_utcb)
	stq	mr7, UTCB_MR_OFFSET + 56(current_utcb)						
	stq	mr8, UTCB_MR_OFFSET + 64(current_utcb)						
#endif
	mov	$16, $0
	
	ldq    $9,   0($30)
	ldq    $10,  8($30)
	ldq    $11, 16($30)
	ldq    $12, 24($30)
	ldq    $13, 32($30)
	ldq    $14, 40($30)
	ldq    $15, 48($30)	
	ldq    $26, 56($30)
	
	addq   $30, 8 * 8, $30				
	ret    $31, ($26)				
END_PROC(user_ipc)			
.previous

SYSCALL(thread_switch)
SYSCALL(thread_control)
SYSCALL(exchange_registers)
SYSCALL(schedule)
SYSCALL(unmap)
SYSCALL(space_control)
SYSCALL(processor_control)
SYSCALL(memory_control)
SYSCALL(system_clock)
	
SYSCALL_UNIMPLEMENTED(lipc)

	
