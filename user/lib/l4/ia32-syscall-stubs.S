/*********************************************************************
 *                
 * Copyright (C) 2002, 2003,  Karlsruhe University
 *                
 * File path:     ia32-syscall-stubs.S
 * Description:   Syscall gate jump table
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
 * $Id: ia32-syscall-stubs.S,v 1.6 2004/03/11 16:46:07 joshua Exp $
 *                
 ********************************************************************/

#define DEFINE_SYSCALL(syscall)				\
							;\
	.section .data.syscalls, "wax", "progbits"	;\
	.global	__L4_##syscall				;\
__L4_##syscall:						;\
	jmp	__L4_fixup_##syscall			;\
	.previous					;\
							;\
	.section .data.syscalls_copy, "wax", "progbits"	;\
	jmp	__L4_fixup_##syscall			;\
	.previous					;\
							;\
__L4_fixup_##syscall:					;\
	pusha						;\
	call	__L4_init_syscalls			;\
	popa						;\
	jmp	__L4_##syscall


	.section .data.syscalls, "wax", "progbits"
	.p2align 12
	.global __L4_syscalls_start
__L4_syscalls_start:
	.previous

	.section .data.syscalls_copy, "wax", "progbits"
	.global __L4_syscalls_copy_start
__L4_syscalls_copy_start:
	.previous

DEFINE_SYSCALL (Ipc)
DEFINE_SYSCALL (Lipc)
DEFINE_SYSCALL (ExchangeRegisters)
DEFINE_SYSCALL (ThreadControl)
DEFINE_SYSCALL (SystemClock)
DEFINE_SYSCALL (ThreadSwitch)
DEFINE_SYSCALL (Schedule)
DEFINE_SYSCALL (Unmap)
DEFINE_SYSCALL (SpaceControl)
DEFINE_SYSCALL (ProcessorControl)
DEFINE_SYSCALL (MemoryControl)

	.section .data.syscalls, "wax", "progbits"
	.global __L4_syscalls_end
__L4_syscalls_end:
	.p2align 12
	.previous

	.section .data.syscalls_copy, "wax", "progbits"
	.global __L4_syscalls_copy_end
__L4_syscalls_copy_end:
	.previous
