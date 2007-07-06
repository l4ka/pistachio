/*********************************************************************
 *                
 * Copyright (C) 2002-2004,  Karlsruhe University
 *                
 * File path:     platform/simics/startup32.S
 * Description:   startup file for pc99 platform
 *		  initializes segments, invokes init_paging and
 *		  startup_system
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
 * $Id: startup32.S,v 1.4 2006/10/19 22:57:40 ud3 Exp $ 
 *                
 ********************************************************************/

.section .init
	
	
.globl start
.align 4
start:
	/* Save the multiboot info structure pointer (ebx) to physical
	   address of kernel_arg. We assume that the kernel IS multibooted.
	   This value is later on passed to the root task */
	/* UD: Actually, we should save the mbi flags as well - just for the
	   case where we AREN'T multibooted. */
	cli	/* disable int's */
	cld	/* clear direction flag */
	mov	%ds, %ax
	mov	%ax, %es
	mov	%ax, %fs
	mov	%ax, %gs
	mov	%ax, %ss

	/* load temporary stack pointer */
	lea	_mini_stack-4, %esp
	/* set up paging */
	call	init_paging
	nop

	.align	4
	.space	1024
_mini_stack:
	.align	4
	.section .mb_header
_mb_header:
        .long   0x1BADB002			/* magic */
        .long   0x00000000			/* flags */
        .long   - 0x00000000 - 0x1BADB002	/* check */

