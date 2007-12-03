/*********************************************************************
 *                
 * Copyright (C) 1999-2004, 2007,  Karlsruhe University
 *                
 * File path:     crt0-amd64.S
 * Description:   Startup code for x86 family processors.
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
 * $Id: crt0-amd64.S,v 1.2 2006/10/19 22:57:41 ud3 Exp $
 *                
 ********************************************************************/
#include <config.h>

	.text
	.global _start
_start:

        /* We assume we are multibooted. Save the multiboot info
           structure pointer (ebx) and the flags (eax) to global
           variables. */
        movl    %ebx, grub_mbi_ptr
        movl    %eax, grub_mbi_flags
        movl    %eax, %esi
        movl    %ebx, %edi
	cli     /* disable int's */
	cld     /* clear direction flag */
	mov     %ds, %ax
	mov     %ax, %es
	mov     %ax, %fs
	mov     %ax, %gs
	mov     %ax, %ss

	/* load temporary stack pointer */
	lea     _mini_stack, %esp
	/* set up paging */
        call    loader
        
        /* We should never return from the loader */
1:      jmp     1b

        .align  16
	.space	8192
_mini_stack:


        .section .mbh

        jmp     _start
        .ascii  "KICKSTART"
	.align 16, 0x90
__mb_header:
	.long   0x1BADB002;
	.long   0x00000000;
	.long   - 0x00000000 - 0x1BADB002;
	.long   __mb_header;
	.long   _start;
	.long   _edata;
	.long   _end;
	.long   _start;
