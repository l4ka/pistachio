/*********************************************************************
 *                
 * Copyright (C) 2002-2003,  Karlsruhe University
 *                
 * File path:     arch/amd64/traps.h
 * Description:   AMD64 exceptions
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
 * $Id: traps.h,v 1.3 2006/10/19 22:57:34 ud3 Exp $ 
 *                
 ********************************************************************/
#ifndef __ARCH__AMD64__TRAPS_H__
#define __ARCH__AMD64__TRAPS_H__

/* 
 * defined AMD64 exceptions
 * for details see: AMD x86-64  Architecture Programmer's 
 * Manual, Volume II, Chapter 8, Interrupt and Exception Handling
 * 
 */


#define AMD64_EXC_DIVIDE_ERROR		0
#define AMD64_EXC_DEBUG			1
#define AMD64_EXC_NMI			2
#define AMD64_EXC_BREAKPOINT		3
#define AMD64_EXC_OVERFLOW		4
#define AMD64_EXC_BOUNDRANGE		5
#define AMD64_EXC_INVALIDOPCODE		6
#define AMD64_EXC_NOMATH_COPROC		7
#define AMD64_EXC_DOUBLEFAULT		8
#define AMD64_EXC_COPSEG_OVERRUN	9
#define AMD64_EXC_INVALID_TSS		10
#define AMD64_EXC_SEGMENT_NOT_PRESENT	11
#define AMD64_EXC_STACKSEG_FAULT	12
#define AMD64_EXC_GENERAL_PROTECTION	13
#define AMD64_EXC_PAGEFAULT		14
#define AMD64_EXC_RESERVED		15
#define AMD64_EXC_FPU_FAULT		16
#define AMD64_EXC_ALIGNEMENT_CHECK	17
#define AMD64_EXC_MACHINE_CHECK		18
#define AMD64_EXC_SIMD_FAULT		19

/* Intel reserved exceptions */
#define AMD64_EXC_RESERVED_FIRST	20
#define AMD64_EXC_RESERVED_LAST		31



#endif /* !__ARCH__AMD64__TRAPS_H__ */
