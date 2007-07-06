/*********************************************************************
 *                
 * Copyright (C) 2002-2004,  University of New South Wales
 *                
 * File path:	  arch/mips64/cache.h
 * Description:   Functions which manipulate the MIPS cache
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
 * $Id: cache.h,v 1.6 2004/06/04 02:14:25 cvansch Exp $
 *                
 ********************************************************************/

#ifndef __ARCH__MIPS64__CACHE_H__
#define __ARCH__MIPS64__CACHE_H__

#ifndef ASSEMBLER

#include INC_ARCH(pgent.h)

class cache_t
{
public:
    static inline void init_cpu(void);
    static inline void flush_cache_all(void);
    static inline void flush_cache_l1(void);
    static inline void flush_cache_range(unsigned long start, unsigned long end);
    static inline void flush_icache_range(unsigned long start, unsigned long end);
    static inline void flush_cache_page(unsigned long page, pgent_t::pgsize_e pgsize);
    static inline void flush_icache_page(unsigned long page, pgent_t::pgsize_e pgsize);

//  void clear_page
//  void copy_page
};

#endif	/* ASSEMBLER */

#endif	/* __ARCH__MIPS64__CACHE_H__ */
