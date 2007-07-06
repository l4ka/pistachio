/*********************************************************************
 *                
 * Copyright (C) 2002-2006,  Karlsruhe University
 *                
 * File path:     mkasmsym.h
 * Description:   Macro for generating object file output that can be
 *                used for creating assembly compliant macros
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
 * $Id: mkasmsym.h,v 1.7 2006/11/23 20:49:38 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __MKASMSYM_H__
#define __MKASMSYM_H__

#define __ABSVALUE(x) ((u64_t) ((x) < 0 ? -(x) : (x)))
#define DS	__attribute__ ((section (".data")))

#define MKASMSYM(sym, value)						\
struct sym ##_sign_t { char x[32*((value) < 0 ? 1 : 2)]; };		\
struct sym ##_sign_t sym ##_sign DS;					\
struct sym ##_b0_t { char x[32*((__ABSVALUE(value) >> 0) & 0xff)]; };	\
struct sym ##_b0_t sym ##_b0 DS;					\
struct sym ##_b1_t { char x[32*((__ABSVALUE(value) >> 8) & 0xff)]; };	\
struct sym ##_b1_t sym ##_b1 DS;					\
struct sym ##_b2_t { char x[32*((__ABSVALUE(value) >> 16) & 0xff)]; };	\
struct sym ##_b2_t sym ##_b2 DS;					\
struct sym ##_b3_t { char x[32*((__ABSVALUE(value) >> 24) & 0xff)]; };	\
struct sym ##_b3_t sym ##_b3 DS;					\
struct sym ##_b4_t { char x[32*((__ABSVALUE(value) >> 32) & 0xff)]; };	\
struct sym ##_b4_t sym ##_b4 DS;					\
struct sym ##_b5_t { char x[32*((__ABSVALUE(value) >> 40) & 0xff)]; };	\
struct sym ##_b5_t sym ##_b5 DS;					\
struct sym ##_b6_t { char x[32*((__ABSVALUE(value) >> 48) & 0xff)]; };	\
struct sym ##_b6_t sym ##_b6 DS;					\
struct sym ##_b7_t { char x[32*((__ABSVALUE(value) >> 56) & 0xff)]; };	\
struct sym ##_b7_t sym ##_b7 DS

#endif /* !__MKASMSYM_H__ */
