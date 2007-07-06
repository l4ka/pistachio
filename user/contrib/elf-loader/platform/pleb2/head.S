/*********************************************************************
 *
 * Copyright (C) 2003,  University of New South Wales
 *
 * File path:      contrib/elf-loader/platform/pleb/head.S
 * Description:    The bootloader entry point
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
 * $Id: head.S,v 1.1 2004/12/02 22:06:38 cvansch Exp $
 *
 ********************************************************************/

#include <l4/arm/asm.h>

.section .text.start

BEGIN_PROC(_start)
        ldr     sp,     1f

        bl      main
1:      .word   _bootstack_top
END_PROC(_start)

BEGIN_PROC(print_byte)
	mov	r1,	#0x40000000
	add	r1,	r1, #0x00100000
	ldr	r2,	[r1, #0x14]	/* LSR */
	ands	r2,	r2,	#0x40	/* TEMPT */
	beq	print_byte

	str	r0,	[r1, #0x00]	/* THR */

        mov     pc,     lr
END_PROC(print_byte)

BEGIN_PROC(__div0)
1:
        b	1b
END_PROC(__div0)

.align
_bootstack:
        .space  1024
_bootstack_top:

