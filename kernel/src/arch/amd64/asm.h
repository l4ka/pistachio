/*********************************************************************
 *                
 * Copyright (C) 2002-2004, 2006,  Karlsruhe University
 *                
 * File path:     arch/amd64/asm.h
 * Description:   Assembler specific macros
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
 * $Id: asm.h,v 1.5 2006/10/19 22:57:34 ud3 Exp $
 *                
 ********************************************************************/
#ifndef __ARCH__AMD64__ASM_H__
#define __ARCH__AMD64__ASM_H__

#define BEGIN_PROC(name)			\
.align 16;					\
.globl	name;					\
.type name,@function;				\
name:

#define END_PROC(name)

#define AMD64_64BIT_ZERO       0
#define AMD64_64BIT_ONE        1

#define BOOT_KCS	AMD64_KCS
#define BOOT_KDS	AMD64_KDS


#endif /* !__ARCH__AMD64__ASM_H__ */
