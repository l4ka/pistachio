/*********************************************************************
 *                
 * Copyright (C) 2003-2004,  University of New South Wales
 *                
 * File path:     arch/sparc64/asm.h
 * Description:   Assembler defines/macros for sparc64. 
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
 * $Id: asm.h,v 1.4 2004/02/03 04:11:11 philipd Exp $
 *                
 ********************************************************************/

#ifndef __ARCH__SPARC64__ASM_H__
#define __ARCH__SPARC64__ASM_H__

/* SPARC 64-bit ABI dictates that the stack pointer is baised by 2047 bits */
#define STACK_BIAS_64BIT  2047

/* SPARC 64-bit compiler 64-bit frame size. 8 bytes * (8in + 8local + 6) */ 
#define STACK_FRAME_64BIT 176

#define DECLAR_STRING(name, sect, value) \
  	.section sect;                   \
	.globl name;                     \
name:	.string value

#define BEGIN_PROC(name, sect) \
  	.section sect, "x";    \
	.align 8;              \
	.globl name;           \
	.type name,@function;  \
name:

#define _INS_(x) #x
#define STR(x) _INS_(x)

#endif /* !__ARCH__SPARC64__ASM_H__ */
