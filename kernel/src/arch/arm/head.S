/*********************************************************************
 *
 * Copyright (C) 2003-2004,  National ICT Australia (NICTA)
 *
 * File path:      arch/arm/head.S
 * Description:    The kernels entry point
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
 * $Id: head.S,v 1.9 2004/12/02 22:02:16 cvansch Exp $
 *
 ********************************************************************/

#include INC_ARCH(asm.h)
#include INC_CPU(syscon.h)

/* This is the entry point of the L4Ka::Pistachio ARM architecture.
 * The boot loader must call _start with the processor in privileged
 * mode and translation disabled.
 */
	.section .init.head
BEGIN_PROC(_start)
	/* Setup status register for supervisor mode, interrupts disabled */
	msr	cpsr,	#0xd3

	/* Setup the CP15 control register to a known initial state */
	ldr	r0,	=C15_CONTROL_INIT
	mcr	CP15, 0, r0, C15_control, c0

	/* Setup boot stack (physical address) */
	ldr	sp,	_kernel_init_stack

	/* Jump	to startup_system */
	bl	startup_system

	/* NOT REACHED */
1:
	b	1b
 
_kernel_init_stack:
	.word	_bootstack_top_phys
