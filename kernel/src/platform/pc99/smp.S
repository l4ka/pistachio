/*********************************************************************
 *                
 * Copyright (C) 2002-2003, 2006,  Karlsruhe University
 *                
 * File path:     platform/pc99/smp.S
 * Description:   startup of application processors for SMP systems
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
 * $Id: smp.S,v 1.5 2006/09/25 13:55:01 stoess Exp $
 *                
 ********************************************************************/

#include INC_ARCH(asm.h)
#include INC_GLUE(config.h)

.section .init.smp, "ax"
.code16

BEGIN_PROC(_start_ap)	
_base = .
	/* disable ints */
	cli

	/* equal code and data segment */
	mov	%cs, %ax
	mov	%ax, %ds

	/* setup a boot gdt, (ptr - _base) for .code16 relocation */
	lgdt	_smp_gdt - _base

	/* enable protected mode */
	xor	%ax, %ax
	inc	%ax
	lmsw	%ax
	jmp	1f
1:
	nop
	nop
	
	ljmpl	$BOOT_KCS, $_start_ap_32
	
	/* does not return */
END_PROC(_start_ap)

.code32
.align	16
_smp_gdt:
	.word	3*8	/* this GDT has 3 entries: NULL, KCS, KDS */
	.long	smp_boot_gdt - KERNEL_OFFSET
	.word	0	
	

//.section .init
.code32
BEGIN_PROC(_start_ap_32)
	mov	$BOOT_KDS, %ax
	mov	%ax, %ds
	mov	%ax, %ss
	mov	%ax, %es
	mov	%ax, %fs
	mov	%ax, %gs
	jmp	1f
1:	

	/* load temporary stack pointer */
	lea	_mini_stack-4, %esp
	/* indicate the init_paging was called by an AP (only needed for AMD64 */
	pushl	$1
	/* set up paging */	
	call	init_paging
	/* clean up stack. AMD64 implementation will not return */
	add	$4, %esp
	nop

	/* still running in 1:1 mapped area
	   jump to the top level init function */ 
	jmp	startup_processor
END_PROC(_start_ap_32)

        .align  16
	.space	1024
_mini_stack:
