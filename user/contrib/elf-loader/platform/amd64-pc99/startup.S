/*********************************************************************
 *                
 * Copyright (C) 2002-2003,  Karlsruhe University
 *                
 * File path:     startup.S
 * Description:   X86_64 Startup file
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
 * $Id: startup.S,v 1.2 2006/10/19 22:57:40 ud3 Exp $
 *                
 ********************************************************************/
#define __ASSEMBLY__
#include <multiboot.h>

#define ENTRY(name) \
	.globl name; \
	.align 4; \
	name##:

ENTRY(start)
ENTRY(_start)
	leal _mini_stack-4,%esp 
	push $0
	popf
	/* Push  magic */
	push %eax			
	/* Push  multiboot info*/
	push %ebx
	call startup
	
	.space 1024
_mini_stack:
	
.section .mb_header
.align	4	
_mb_header:	
        .long   MULTIBOOT_HEADER_MAGIC					/* magic */
        .long   MULTIBOOT_HEADER_FLAGS					/* no mb flags */
        .long   - (MULTIBOOT_HEADER_FLAGS + MULTIBOOT_HEADER_MAGIC)	/* checksum */

