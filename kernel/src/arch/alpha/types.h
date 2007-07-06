/*********************************************************************
 *                
 * Copyright (C) 2002,  University of New South Wales
 *                
 * File path:     arch/alpha/types.h
 * Created:       15/07/2002 20:02:58 by Simon Winwood (sjw)
 * Description:   types for alpha 
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
 * $Id: types.h,v 1.2 2003/09/24 19:04:25 skoglund Exp $
 *                
 ********************************************************************/

#ifndef __ARCH__ALPHA__TYPES_H__
#define __ARCH__ALPHA__TYPES_H__

#define L4_64BIT
#undef  L4_32BIT

typedef unsigned long           u64_t;
typedef unsigned int		u32_t;
typedef unsigned short		u16_t;
typedef unsigned char		u8_t;

typedef signed long             s64_t;
typedef signed int		s32_t;
typedef signed short		s16_t;
typedef signed char		s8_t;


/**
 * word_t: machine word wide unsigned int
 */
typedef u64_t			word_t;

#endif /* __ARCH__ALPHA__TYPES_H__ */
