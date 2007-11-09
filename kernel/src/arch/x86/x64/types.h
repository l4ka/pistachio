/*********************************************************************
 *                
 * Copyright (C) 2003-2004, 2007,  Karlsruhe University
 *                
 * File path:     arch/x86/x64/types.h
 * Description:   
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
 * $Id: types.h,v 1.5 2006/10/22 19:42:34 reichelt Exp $
 *                
 ********************************************************************/
#ifndef __ARCH__X86__X64__TYPES_H__
#define __ARCH__X86__X64__TYPES_H__


#if !defined(X64_32BIT_CODE)
/*
 * long, pointers are 64bit on x86_64
 * int is 32Bit on x86-64
 */
typedef unsigned long		u64_t;
typedef unsigned int		u32_t;
typedef unsigned short		u16_t;
typedef unsigned char		u8_t;

typedef signed long	        s64_t;
typedef signed int		s32_t;
typedef signed short		s16_t;
typedef signed char		s8_t;

/**
 *	word_t - 64 Bit
 */
typedef u64_t            	word_t;

/**
 *	64 Bit Macros
 */
#define __X86_64BIT_TYPED(x)	(x##UL)
#define X86_64BIT_TYPED(x)	__X86_64BIT_TYPED(x)
#define X86_64BIT_ZERO		X86_64BIT_TYPED(0)
#define X86_64BIT_ONE		X86_64BIT_TYPED(1)

#else
/* This is for the few startup files which are compiled as 32 bit code */

typedef unsigned int __attribute__((__mode__(__DI__))) u64_t;
typedef unsigned int		u32_t;
typedef unsigned short		u16_t;
typedef unsigned char		u8_t;

typedef int __attribute__((__mode__(__DI__))) s64_t;
typedef int			s32_t;
typedef short			s16_t;
typedef char			s8_t;

/**
 *	word_t - 32 Bit
 */
typedef u32_t            	word_t;

/**
 *	64 Bit 0
 */

/**
 *	64 Bit Macros
 */
#define __X86_64BIT_TYPED(x)	(x##ULL)
#define X86_64BIT_TYPED(x)	__X86_64BIT_TYPED(x)
#define X86_64BIT_ZERO		X86_64BIT_TYPED(0)
#define X86_64BIT_ONE		X86_64BIT_TYPED(1)

#endif /* !defined(X64_32BIT_CODE) */

#endif /* !__ARCH__X86__X64__TYPES_H__ */
