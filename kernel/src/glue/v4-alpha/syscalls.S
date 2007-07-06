/*********************************************************************
 *                
 * Copyright (C) 2002,  University of New South Wales
 *                
 * File path:     glue/v4-alpha/syscalls.S
 * Description:   Syscall stubs for alpha 
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
 * $Id: syscalls.S,v 1.6 2003/11/03 08:01:20 sjw Exp $
 *                
 ********************************************************************/

#include INC_ARCH(asm.h)
#include INC_GLUE(syscalls.h)
	
.set noat
	
#define DECODE_SYSCALL(name, vector, t0, t1, t2, jr)	\
	lda	t0, SYSCALL_##name ;			\
	lda	t1, sys_##name ;			\
	cmpeq	t0, vector, t2	; 			\
	cmovne	t2, t1, jr 

#define DECODE_SYSCALL_SPECIAL(name, vector, t0, t1, t2, jr)	\
	lda	t0, SYSCALL_##name ;			\
	lda	t1, __call_##name ;			\
	cmpeq	t0, vector, t2	; 			\
	cmovne	t2, t1, jr 	
			
/* Entry is fastpath.S */				
BEGIN_PROC(__handle_non_ipc)
//	DISABLE_INT($3, $4)
		
	lda	$27, __unknown_syscall			
	lda	$26, __finish_sys
	
	DECODE_SYSCALL(thread_switch, $0, $22, $23, $24, $27) /* 1 in, 0 out */
	DECODE_SYSCALL(thread_control, $0, $22, $23, $24, $27) /* 4 in, 1 out */ 
	DECODE_SYSCALL_SPECIAL(exchange_registers, $0, $22, $23, $24, $27) /* 7 in, 7 out */	
	DECODE_SYSCALL(schedule, $0, $22, $23, $24, $27) /* 5 in, 2 out */
	DECODE_SYSCALL(unmap, $0, $22, $23, $24, $27) /* 1 in, 0 out */
	DECODE_SYSCALL(space_control, $0, $22, $23, $24, $27) /* 5 in, 2 out */

#if 0
	DECODE_SYSCALL(processor_control, $0, $22, $23, $24, $27) /* 4 in, 1 out */
	DECODE_SYSCALL(memory_control, $0, $22, $23, $24, $27) /* 5 in, 0 out */ 
#endif
	DECODE_SYSCALL(system_clock, $0, $22, $23, $24, $27) /* 0 in, 1 out */
											
	DECODE_SYSCALL(wrperfmon, $0, $22, $23, $24, $27) /* 2 in, 1 out */
	DECODE_SYSCALL(read_idle, $0, $22, $23, $24, $27) /* 1 in, 1 out */

	DECODE_SYSCALL(halt, $0, $22, $23, $24, $27) /* 0 in, 0 out */ 	
	DECODE_SYSCALL(null, $0, $22, $23, $24, $27) /* 1 in, 0 out */
	
	jsr	$31, ($27)
END_PROC(__handle_non_ipc)

BEGIN_PROC(__call_exchange_registers)
	subq	$30, 8, $30
	stq	$2, 0($30)

	lda	$27, sys_exchange_registers
	jsr	$26, ($27)

	addq	$30, 8, $30
	call_pal	PAL_retsys
END_PROC(__call_exchange_registers)
	
BEGIN_PROC(__unknown_syscall)
	/* well, this shouldn't happen ... */
	ldiq	$16, 2
	lda	$17, 1f

	call_pal	PAL_gentrap

	call_pal	PAL_retsys
	
1:	.asciz	"Unknown syscall!"

END_PROC(__unknown_syscall)
	
BEGIN_PROC(__finish_sys)
	call_pal	PAL_retsys
END_PROC(__finish_sys)
	
/* Alpha specifics */
	
BEGIN_PROC(sys_null)
	call_pal	PAL_retsys
END_PROC(sys_null)

BEGIN_PROC(sys_wrperfmon)
	call_pal	PAL_wrperfmon
	call_pal	PAL_retsys
END_PROC(sys_wrperfmon)

	