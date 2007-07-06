/*********************************************************************
 *                
 * Copyright (C) 2002, 2003,  Karlsruhe University
 *                
 * File path:     ia64-syscall-stubs.S
 * Description:   IA64 system call library stubs
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
 * $Id: ia64-syscall-stubs.S,v 1.9 2005/05/16 07:37:56 cgray Exp $
 *                
 ********************************************************************/


#define DEFINE_SYSCALL(syscall)					\
								;\
	/* Stub for calling system call trampoline */		;\
	.align 16						;\
	.global __L4_##syscall					;\
	.proc	__L4_##syscall					;\
								;\
__L4_##syscall:							;\
	add	r27 = @gprel(__L4_##syscall##_ptr), gp ;;	;\
	ld8	r27 = [r27] ;;					;\
	mov	b6 = r27 ;;					;\
	br.sptk.few b6						;\
								;\
	.endp	__L4_##syscall					;\
								;\
	/* Pointer to system call function */			;\
	.data							;\
__L4_##syscall##_ptr:						;\
	data8	__L4_##syscall##_fixup				;\
	.previous						;\
								;\
	/* Fixuup stub */					;\
	.section .text.syscallfix, "ax", "progbits"		;\
	.align 16						;\
	.proc __L4_##syscall##_fixup				;\
								;\
__L4_##syscall##_fixup:						;\
	add	r27 = @ltoff(@fptr(__L4_##syscall)), gp ;;	;\
	ld8 r27 = [r27] ;;					;\
	ld8 r27 = [r27] ;;					;\
        mov	b6 = r27					;\
        br.sptk.few __L4_fixup_syscalls				;\
								;\
	.endp __L4_##syscall##_fixup				;\
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
DEFINE_SYSCALL (PAL_Call)
DEFINE_SYSCALL (SAL_Call)



#define  FIXUP_SYSCALL(syscall)					;\
	ld8	scval = [kipptr], 8				;\
	add	fptr = @ltoff(@fptr(__L4_##syscall)), gp	;\
	add	scptr = @gprel(__L4_##syscall##_ptr), gp ;;	;\
	ld8	fptr = [fptr]					;\
	cmp.gt	p6,p0 = scval, mb1 ;;				;\
(p6)	add	scval = kipbase, scval ;;			;\
	st8	[fptr] = scval					;\
	st8	[scptr] = scval

	//
	// Function for fixing all system call links.  Address of
	// invoking stub resides in b6.
	//

	.align	16
	.proc	__L4_fixup_syscalls
__L4_fixup_syscalls:

kipbase	=	r8
kipptr	=	r9
fptr	=	r10
scptr	=	r11
mb1	=	r27
scval	=	r26

	// Get Kernel Interface Page
{ .mlx
	break.m	0x1face
	movl	r0 = 0x0 ;;
}
	// Offset for system call links

	add	kipptr = 0x1a0, kipbase

	// Syscall offsets greater than 1MB are treated as absolute links

	mov	mb1 = (1 << 20) ;;

	// Calculate system call locations and store the results

	FIXUP_SYSCALL (SpaceControl)
	FIXUP_SYSCALL (ThreadControl)
	FIXUP_SYSCALL (ProcessorControl)
	FIXUP_SYSCALL (MemoryControl)
	FIXUP_SYSCALL (Ipc)
	FIXUP_SYSCALL (Lipc)
	FIXUP_SYSCALL (Unmap)
	FIXUP_SYSCALL (ExchangeRegisters)
	FIXUP_SYSCALL (SystemClock)
	FIXUP_SYSCALL (ThreadSwitch)
	FIXUP_SYSCALL (Schedule)

	add	kipptr = 0x28, kipptr ;;

	FIXUP_SYSCALL (PAL_Call)
	FIXUP_SYSCALL (SAL_Call)

	// Jump back to system call stub

	br.sptk.few b6

	.endp	__L4_fixup_syscalls
