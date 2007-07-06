/*********************************************************************
 *                
 * Copyright (C) 2002, 2003,  Karlsruhe University
 *                
 * File path:     arch/ia64/asm.h
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
 * $Id: asm.h,v 1.5 2003/09/24 19:04:28 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __ARCH__IA64__ASM_H__
#define __ARCH__IA64__ASM_H__

#if defined(ASSEMBLY)

#define BEG_PROC(func)			 \
	.globl	func			;\
	.align	16			;\
	.proc	func			;\
func:


#define END_PROC(func)			\
	.endp	func


#if 0
#define ITANIUM_A_STEP_BSW	nop.i 0x0 ;; nop.i 0x0 ;; nop.i 0x0 ;;
#else
#define ITANIUM_A_STEP_BSW
#endif

#define enter_kdebug(text)			\
{ .mlx						;\
	break.m 0x3				;\
	movl	r0 = 1f ;;			;\
}						;\
	.rodata					;\
1:	stringz text				;\
	.previous


#endif /* ASSEMBLY */

#endif /* !__ARCH__IA64__ASM_H__ */
